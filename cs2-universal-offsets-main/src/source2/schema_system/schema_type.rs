use memflow::prelude::v1::*;

/// Memory layout for CS2's `SchemaType` — only `name` is read by the analysis.
/// The remaining fields (type_category, atomic_category, value union) are padding.
#[derive(Clone, Copy)]
#[repr(C)]
pub struct SchemaType {
    _pad0: [u8; 0x8],
    pub name: Pointer64<ReprCString>, // 0x0008
}

unsafe impl Pod for SchemaType {}
