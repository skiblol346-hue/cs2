//! Read-only walker for the tier0 **CCvar** convar / concommand registry.
//!
//! The registry is normally enumerated through virtual methods
//! (`GetFirstConVar` / `GetNextConVar` / `GetConVarData`), which a memory-only
//! reader cannot call. Instead we walk the underlying storage directly, using
//! offsets reversed from `tier0.dll` (build 14169). The `g_pCVar` global that
//! anchors the walk is resolved via the `pCvarRegistry` RIP-relative pattern.
//!
//! ```text
//! *g_pCVar  ->  CCvar instance:
//!   +0x50  convar linked-list array base   (stride 16)
//!   +0x58  convar list head index (u16)
//!          elem = { ConVarInfo* @+0, u16 prev @+8, u16 next @+10 }
//!   +0x108 concommand array base           (stride 56)
//!   +0x110 concommand head index (u16)
//!          elem = { name @+0, desc @+8, u32 flags @+0x10, u16 prev @+0x30, next @+0x32 }
//!
//! ConVarInfo (the convar data object):
//!   +0x00 name (char*)   +0x20 description (char*)
//!   +0x28 type (u16 EConVarType)   +0x30 flags (u32)   +0x58 value union
//! ```
//!
//! Enumeration follows the head→next index chain (the exact list the game's own
//! `cvarlist` walks), so every registered convar/command is captured regardless
//! of hidden/dev flags. All reads are best-effort — a bad pointer yields an
//! empty field, never a panic.

use anyhow::{Result, bail};
use memflow::prelude::v1::*;
use serde::Serialize;

/// Sentinel index terminating a CUtlLinkedList chain.
const LIST_END: u16 = 0xFFFF;
/// Hard cap on chain length — guards against a corrupt `next` pointer looping
/// forever. Comfortably above CS2's real convar count (~5k).
const MAX_WALK: usize = 200_000;

// --- CCvar instance offsets ------------------------------------------------
const OFF_CV_BASE: u64 = 0x50; // convar list array base ptr
const OFF_CV_HEAD: u64 = 0x58; // convar list head index (u16)
const CV_STRIDE: u64 = 16;
const CV_ELEM_NEXT: u64 = 10; // next index within a convar list element

const OFF_CC_BASE: u64 = 0x108; // concommand array base ptr
const OFF_CC_HEAD: u64 = 0x110; // concommand array head index (u16)
const CC_STRIDE: u64 = 56;
const CC_ELEM_NEXT: u64 = 0x32; // next index within a concommand element

// --- ConVarInfo_t field offsets --------------------------------------------
const CVI_NAME: u64 = 0x00;
const CVI_DESC: u64 = 0x20;
const CVI_TYPE: u64 = 0x28;
const CVI_FLAGS: u64 = 0x30;
const CVI_VALUE: u64 = 0x58;

// --- ConCommand element field offsets --------------------------------------
const CC_NAME: u64 = 0x00;
const CC_DESC: u64 = 0x08;
const CC_FLAGS: u64 = 0x10;

/// FCVAR bit -> canonical `FCVAR_*` short name. Table + masks read live from
/// tier0's own flag-name table (build 14169); names normalised to the
/// well-known Source SDK constants rather than the terse console tokens.
const FCVAR_FLAGS: &[(u32, &str)] = &[
    (0x00000001, "FCVAR_LINKED_CONCOMMAND"),
    (0x00000002, "FCVAR_DEVELOPMENTONLY"),
    (0x00000004, "FCVAR_GAMEDLL"),
    (0x00000008, "FCVAR_CLIENTDLL"),
    (0x00000020, "FCVAR_PROTECTED"),
    (0x00000040, "FCVAR_SPONLY"),
    (0x00000080, "FCVAR_ARCHIVE"),
    (0x00000100, "FCVAR_NOTIFY"),
    (0x00000200, "FCVAR_USERINFO"),
    (0x00000800, "FCVAR_UNLOGGED"),
    (0x00002000, "FCVAR_REPLICATED"),
    (0x00004000, "FCVAR_CHEAT"),
    (0x00008000, "FCVAR_PER_USER"),
    (0x00010000, "FCVAR_DEMO"),
    (0x00020000, "FCVAR_DONTRECORD"),
    (0x00080000, "FCVAR_RELEASE"),
    (0x00100000, "FCVAR_MENUBAR_ITEM"),
    (0x00800000, "FCVAR_VCONSOLE_FUZZY_MATCHING"),
    (0x01000000, "FCVAR_SERVER_CAN_EXECUTE"),
    (0x04000000, "FCVAR_SERVER_CANNOT_QUERY"),
    (0x08000000, "FCVAR_VCONSOLE_SET_FOCUS"),
    (0x10000000, "FCVAR_CLIENTCMD_CAN_EXECUTE"),
    (0x20000000, "FCVAR_EXECUTE_PER_TICK"),
];

/// One registered convar with its live value decoded to a string.
#[derive(Debug, Serialize)]
pub struct ConVar {
    pub name: String,
    pub value: String,
    pub type_id: u16,
    pub type_name: &'static str,
    pub flags: u32,
    pub flag_names: Vec<&'static str>,
    pub description: String,
    /// VA of the `ConVarInfo_t` object (handy for external tooling).
    pub address: u64,
}

/// One registered console command (no value; name/flags/help only).
#[derive(Debug, Serialize)]
pub struct ConCommand {
    pub name: String,
    pub flags: u32,
    pub flag_names: Vec<&'static str>,
    pub description: String,
    pub address: u64,
}

#[derive(Debug, Default, Serialize)]
pub struct ConVarsDump {
    pub convars: Vec<ConVar>,
    pub commands: Vec<ConCommand>,
}

/// Walk the registry. `registry_global_va` is the resolved address of the
/// `g_pCVar` global (a `CCvar*`), i.e. the VA the `pCvarRegistry` pattern
/// resolves to.
pub fn walk<P: Process + MemoryView>(
    process: &mut P,
    registry_global_va: u64,
) -> Result<ConVarsDump> {
    let inst = rd_u64(process, registry_global_va);
    if inst == 0 {
        bail!("g_pCVar is null (0x{:X})", registry_global_va);
    }

    let mut dump = ConVarsDump::default();

    // --- convars: head->next chain over the +0x50 array (stride 16) ---------
    let cv_base = rd_u64(process, inst + OFF_CV_BASE);
    if cv_base != 0 {
        let mut idx = rd_u16(process, inst + OFF_CV_HEAD);
        let mut seen = 0usize;
        while idx != LIST_END && seen < MAX_WALK {
            seen += 1;
            let elem = cv_base + CV_STRIDE * idx as u64;
            let data = rd_u64(process, elem);
            let next = rd_u16(process, elem + CV_ELEM_NEXT);
            if data != 0
                && let Ok(cv) = read_convar(process, data)
            {
                dump.convars.push(cv);
            }
            if next == idx {
                break; // self-loop guard
            }
            idx = next;
        }
    }

    // --- concommands: head->next chain over the +0x108 array (stride 56) -----
    let cc_base = rd_u64(process, inst + OFF_CC_BASE);
    if cc_base != 0 {
        let mut idx = rd_u16(process, inst + OFF_CC_HEAD);
        let mut seen = 0usize;
        while idx != LIST_END && seen < MAX_WALK {
            seen += 1;
            let elem = cc_base + CC_STRIDE * idx as u64;
            let next = rd_u16(process, elem + CC_ELEM_NEXT);
            if let Ok(cc) = read_concommand(process, elem) {
                dump.commands.push(cc);
            }
            if next == idx {
                break;
            }
            idx = next;
        }
    }

    dump.convars.sort_by(|a, b| a.name.cmp(&b.name));
    dump.commands.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(dump)
}

fn read_convar<P: MemoryView>(process: &mut P, data_va: u64) -> Result<ConVar> {
    let name = rd_cstr_at(process, data_va + CVI_NAME);
    if name.is_empty() {
        bail!("empty convar name");
    }
    let description = rd_cstr_at(process, data_va + CVI_DESC);
    let type_id = rd_u16(process, data_va + CVI_TYPE);
    let flags = rd_u32(process, data_va + CVI_FLAGS);
    Ok(ConVar {
        name,
        value: decode_value(process, data_va + CVI_VALUE, type_id),
        type_id,
        type_name: type_name(type_id),
        flags,
        flag_names: decode_flags(flags),
        description,
        address: data_va,
    })
}

fn read_concommand<P: MemoryView>(process: &mut P, elem_va: u64) -> Result<ConCommand> {
    let name = rd_cstr_at(process, elem_va + CC_NAME);
    if name.is_empty() {
        bail!("empty concommand name");
    }
    let description = rd_cstr_at(process, elem_va + CC_DESC);
    let flags = rd_u32(process, elem_va + CC_FLAGS);
    Ok(ConCommand {
        name,
        flags,
        flag_names: decode_flags(flags),
        description,
        address: elem_va,
    })
}

/// EConVarType (CS2) -> readable type name.
fn type_name(t: u16) -> &'static str {
    match t {
        0 => "bool",
        1 => "int16",
        2 => "uint16",
        3 => "int32",
        4 => "uint32",
        5 => "int64",
        6 => "uint64",
        7 => "float32",
        8 => "float64",
        9 => "string",
        10 => "color",
        11 => "vector2",
        12 => "vector3",
        13 => "vector4",
        14 => "qangle",
        _ => "unknown",
    }
}

/// Decode the value union at `va` per EConVarType. Best-effort — unreadable
/// memory or an unknown type yields a raw hex fallback / empty string.
fn decode_value<P: MemoryView>(process: &mut P, va: u64, type_id: u16) -> String {
    let a = Address::from(va);
    match type_id {
        0 => {
            if process.read::<u8>(a).data_part().unwrap_or(0) != 0 {
                "true".into()
            } else {
                "false".into()
            }
        }
        1 => num(process.read::<i16>(a).data_part()),
        2 => num(process.read::<u16>(a).data_part()),
        3 => num(process.read::<i32>(a).data_part()),
        4 => num(process.read::<u32>(a).data_part()),
        5 => num(process.read::<i64>(a).data_part()),
        6 => num(process.read::<u64>(a).data_part()),
        7 => process
            .read::<f32>(a)
            .data_part()
            .map(fmt_f32)
            .unwrap_or_default(),
        8 => process
            .read::<f64>(a)
            .data_part()
            .map(fmt_f)
            .unwrap_or_default(),
        9 => rd_cstr_at(process, va), // string: char* at +0x58
        10 => {
            let c = rd_u32(process, va);
            format!("{} {} {} {}", c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF)
        }
        11 => read_vec(process, va, 2),
        12 | 14 => read_vec(process, va, 3),
        13 => read_vec(process, va, 4),
        _ => format!("0x{:016X}", rd_u64(process, va)),
    }
}

fn read_vec<P: MemoryView>(process: &mut P, va: u64, n: u64) -> String {
    (0..n)
        .map(|i| {
            process
                .read::<f32>(Address::from(va + 4 * i))
                .data_part()
                .map(fmt_f32)
                .unwrap_or_default()
        })
        .collect::<Vec<_>>()
        .join(" ")
}

fn decode_flags(flags: u32) -> Vec<&'static str> {
    FCVAR_FLAGS
        .iter()
        .filter(|(m, _)| flags & m != 0)
        .map(|(_, n)| *n)
        .collect()
}

// --- small read helpers (best-effort, never panic) -------------------------

fn rd_u64<P: MemoryView>(process: &mut P, va: u64) -> u64 {
    process.read::<u64>(Address::from(va)).data_part().unwrap_or(0)
}

fn rd_u32<P: MemoryView>(process: &mut P, va: u64) -> u32 {
    process.read::<u32>(Address::from(va)).data_part().unwrap_or(0)
}

fn rd_u16<P: MemoryView>(process: &mut P, va: u64) -> u16 {
    process.read::<u16>(Address::from(va)).data_part().unwrap_or(LIST_END)
}

fn rd_cstr<P: MemoryView>(process: &mut P, ptr: u64) -> String {
    if ptr == 0 {
        return String::new();
    }
    process
        .read_utf8_lossy(Address::from(ptr), 256)
        .data_part()
        .unwrap_or_default()
}

/// Read a `char*` field at `ptr_field_va`, then the C-string it points to.
fn rd_cstr_at<P: MemoryView>(process: &mut P, ptr_field_va: u64) -> String {
    let ptr = rd_u64(process, ptr_field_va);
    rd_cstr(process, ptr)
}

fn num<T: std::fmt::Display, E>(r: Result<T, E>) -> String {
    r.map(|v| v.to_string()).unwrap_or_default()
}

/// `%g`-ish float formatting: Rust's default already drops trailing zeros
/// (`1.0` -> `"1"`, `0.5` -> `"0.5"`), which matches the engine's cvarlist.
fn fmt_f(v: f64) -> String {
    if v == 0.0 { "0".into() } else { format!("{v}") }
}

/// Format an `f32` directly (NOT widened to f64 first) so we get the shortest
/// round-trippable decimal — `1.92f32` prints `"1.92"`, not `"1.9199999570846558"`.
fn fmt_f32(v: f32) -> String {
    if v == 0.0 { "0".into() } else { format!("{v}") }
}
