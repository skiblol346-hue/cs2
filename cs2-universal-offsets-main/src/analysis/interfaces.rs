use std::collections::BTreeMap;

use anyhow::Result;
use log::debug;
use memflow::prelude::v1::*;
use pelite::pe64::exports::Export;
use pelite::pe64::{Pe, PeView};

use crate::memory::address;
use crate::source2::InterfaceReg;

#[derive(Clone, Copy, Debug)]
pub struct InterfaceEntry {
    /// RVA the create_fn factory resolves to. For `lea`-style factories
    /// this is the singleton's `this` pointer; for `mov`-style factories
    /// this is the address of a global pointer holding the singleton.
    pub rva: umem,
    /// `true` when the factory uses `mov rax, [rip+disp]; ret` (resolved
    /// address holds a pointer to the singleton — one extra deref needed).
    /// `false` for the `lea rax, [rip+disp]; ret` shape (resolved address
    /// IS the singleton).
    pub needs_deref: bool,
}

pub type InterfaceMap = BTreeMap<String, BTreeMap<String, InterfaceEntry>>;

pub fn interfaces<P: Process + MemoryView>(process: &mut P) -> Result<InterfaceMap> {
    process
        .module_list()?
        .iter()
        .filter_map(|module| {
            let buf = process
                .read_raw(module.base, module.size as _)
                .data_part()
                .ok()?;

            let view = PeView::from_bytes(&buf).ok()?;

            let ci_export = view
                .exports()
                .ok()?
                .by()
                .ok()?
                .name("CreateInterface")
                .ok()?;

            match ci_export {
                Export::Symbol(symbol) => {
                    let list_ptr = address::resolve_rip(process, module.base + symbol).ok()?;
                    let list_head = process.read_addr64(list_ptr).data_part().ok()?;

                    return read_interfaces(process, module, list_head)
                        .ok()
                        .filter(|ifaces| !ifaces.is_empty())
                        .map(|ifaces| Ok((module.name.to_string(), ifaces)));
                }
                _ => None,
            }
        })
        .collect()
}

fn read_interfaces(
    mem: &mut impl MemoryView,
    module: &ModuleInfo,
    list_head: Address,
) -> Result<BTreeMap<String, InterfaceEntry>> {
    let mut result = BTreeMap::new();

    let mut reg_ptr = Pointer64::<InterfaceReg>::from(list_head);

    while !reg_ptr.is_null() {
        let reg = mem.read_ptr(reg_ptr).data_part()?;
        let name = mem.read_utf8_lossy(reg.name.address(), 128).data_part()?;

        let create_fn_va = reg.create_fn.address();
        let instance_addr = address::resolve_rip(mem, create_fn_va)?;

        // Read the opcode byte at create_fn+1 to determine whether the
        // factory is `lea rax, [rip+disp]; ret` (no deref) or
        // `mov rax, [rip+disp]; ret` (needs deref). Both share REX.W (0x48)
        // at +0 and ModR/M 0x05 at +2; the discriminator is byte +1.
        let opcode = mem.read::<u8>(create_fn_va + 1).data_part().unwrap_or(0);
        let needs_deref = opcode == 0x8B; // 0x8D = lea, 0x8B = mov

        if let Some(instance_rva) = instance_addr.to_umem().checked_sub(module.base.to_umem()) {
            debug!(
                "found \"{}\" at {:#X} ({} + {:#X}){}",
                name,
                instance_addr.to_umem(),
                module.name,
                instance_rva,
                if needs_deref { " [deref]" } else { "" },
            );

            result.insert(name, InterfaceEntry { rva: instance_rva, needs_deref });
        }

        reg_ptr = reg.next;
    }

    Ok(result)
}
