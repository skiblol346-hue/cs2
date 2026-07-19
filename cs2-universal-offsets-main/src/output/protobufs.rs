//! Protobuf message-layout emitter.
//!
//! Emits real, drop-in C++ structs (one per protobuf message) recovered from
//! libprotobuf's reflection tables (see `analysis::protobufs`). Each struct is
//! `#pragma pack(1)` with explicit padding so member offsets are exact by
//! construction, plus `static_assert`s on `sizeof`/`offsetof`. Cast a live
//! message pointer to it and read/write fields directly; has-bit i is tested as
//! `*(uint32_t*)((char*)msg + kHasBits) & (1u << i)`.
//!
//! Non-scalar fields are typed (not raw byte slots): singular messages are
//! `Type*`, strings/bytes are `pb::string_t*`, and repeated fields are
//! `pb::RepeatedField<T>` / `pb::RepeatedPtrField<T>` — while the explicit
//! padding + `static_assert`s keep every offset byte-exact.

use std::collections::{BTreeMap, BTreeSet};
use std::fmt::Write;

use crate::analysis::{ProtoField, ProtoMessage, ProtobufMap};

fn module_ns(module: &str) -> String {
    sanitize(module.trim_end_matches(".dll"))
}

const CPP_KEYWORDS: &[&str] = &[
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
    "case", "catch", "char", "char16_t", "char32_t", "class", "compl", "const", "constexpr",
    "const_cast", "continue", "decltype", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
    "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "register",
    "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static",
    "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local",
    "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
];

fn sanitize(raw: &str) -> String {
    let mut s = String::with_capacity(raw.len());
    for c in raw.chars() {
        if c.is_ascii_alphanumeric() || c == '_' {
            s.push(c);
        } else {
            s.push('_');
        }
    }
    if s.is_empty() {
        s.push('_');
    }
    if s.chars().next().map(|c| c.is_ascii_digit()).unwrap_or(false) {
        s.insert(0, '_');
    }
    if CPP_KEYWORDS.contains(&s.as_str()) {
        s.push('_');
    }
    s
}

/// `flattened message name -> module namespace`, so a field's proto type_name
/// can be resolved to the emitted `pb::<module>::<Name>` struct.
struct Registry {
    by_mod: BTreeMap<String, BTreeSet<String>>,
    global: BTreeMap<String, String>,
}

impl Registry {
    fn build(map: &ProtobufMap) -> Self {
        let mut by_mod: BTreeMap<String, BTreeSet<String>> = BTreeMap::new();
        let mut global: BTreeMap<String, String> = BTreeMap::new();
        for (module, msgs) in map {
            let ns = module_ns(module);
            for m in msgs {
                if m.size == 0 {
                    continue; // not emitted → don't resolve pointers to it
                }
                let flat = sanitize(&m.name);
                by_mod.entry(ns.clone()).or_default().insert(flat.clone());
                global.entry(flat).or_insert_with(|| ns.clone());
            }
        }
        Registry { by_mod, global }
    }

    /// Resolve a proto type_name (`.pkg.Outer.Inner`) to `pb::<mod>::<Name>`,
    /// preferring the referencing message's own module. `None` if unknown.
    fn resolve(&self, type_name: &str, cur_ns: &str) -> Option<String> {
        let flat = sanitize(&type_name.trim_start_matches('.').replace('.', "_"));
        if self.by_mod.get(cur_ns).is_some_and(|s| s.contains(&flat)) {
            return Some(format!("pb::{cur_ns}::{flat}"));
        }
        self.global.get(&flat).map(|m| format!("pb::{m}::{flat}"))
    }
}

/// Scalar C++ type + byte size for a singular (non-repeated) field, else None.
fn scalar(f: &ProtoField) -> Option<(&'static str, u32)> {
    if f.label == 3 {
        return None; // repeated
    }
    Some(match f.ty {
        1 => ("double", 8),
        2 => ("float", 4),
        3 | 16 | 18 => ("int64_t", 8),
        4 | 6 => ("uint64_t", 8),
        5 | 15 | 17 => ("int32_t", 4),
        7 | 13 => ("uint32_t", 4),
        8 => ("bool", 1),
        14 => ("int32_t", 4), // enum
        _ => return None,     // 9 string, 10/11 message, 12 bytes
    })
}

/// Element C++ type for a repeated field's `RepeatedField`/`RepeatedPtrField`.
/// Returns (type, uses_ptr_field) — message/string use RepeatedPtrField.
fn repeated_elem(f: &ProtoField, reg: &Registry, cur_ns: &str) -> (String, bool) {
    match f.ty {
        1 => ("double".into(), false),
        2 => ("float".into(), false),
        3 | 16 | 18 => ("int64_t".into(), false),
        4 | 6 => ("uint64_t".into(), false),
        5 | 15 | 17 => ("int32_t".into(), false),
        7 | 13 => ("uint32_t".into(), false),
        8 => ("bool".into(), false),
        14 => ("int32_t".into(), false), // enum
        9 | 12 => ("pb::string_t".into(), true), // string/bytes
        10 | 11 => (reg.resolve(&f.type_name, cur_ns).unwrap_or_else(|| "void".into()), true),
        _ => ("void".into(), true),
    }
}

/// Human-readable proto type for the trailing comment.
fn proto_ty(f: &ProtoField) -> String {
    let base = match f.ty {
        1 => "double", 2 => "float", 3 => "int64", 4 => "uint64", 5 => "int32",
        6 => "fixed64", 7 => "fixed32", 8 => "bool", 9 => "string", 10 => "group",
        11 => "message", 12 => "bytes", 13 => "uint32", 14 => "enum", 15 => "sfixed32",
        16 => "sfixed64", 17 => "sint32", 18 => "sint64", _ => "?",
    };
    let rep = if f.label == 3 { "repeated " } else { "" };
    let tn = if f.type_name.is_empty() {
        String::new()
    } else {
        format!(" {}", f.type_name.trim_start_matches('.'))
    };
    format!("{rep}{base}{tn}")
}

/// The C++ member declaration (type only, no name) and its byte size for a
/// field, given the slot available before the next field. `None` → emit a raw
/// byte slot (slot too small for the natural type).
fn member_type(f: &ProtoField, reg: &Registry, cur_ns: &str, slot: u32) -> Option<(String, u32)> {
    if f.label == 3 {
        // repeated: RepeatedField<T> (scalar) / RepeatedPtrField<T> (msg/string), 0x18.
        if slot >= 0x18 {
            let (elem, ptr) = repeated_elem(f, reg, cur_ns);
            let cont = if ptr { "RepeatedPtrField" } else { "RepeatedField" };
            return Some((format!("pb::{cont}<{elem}>"), 0x18));
        }
        return None;
    }
    if let Some((ty, sz)) = scalar(f) {
        return (sz <= slot).then(|| (ty.to_string(), sz));
    }
    match f.ty {
        9 | 12 => (slot >= 8).then(|| ("pb::string_t*".to_string(), 8)), // string/bytes
        10 | 11 => (slot >= 8).then(|| {
            let t = reg.resolve(&f.type_name, cur_ns).unwrap_or_else(|| "void".into());
            (format!("{t}*"), 8)
        }),
        _ => None,
    }
}

fn struct_block(m: &ProtoMessage, cur_ns: &str, reg: &Registry) -> String {
    let name = sanitize(&m.name);
    let size = m.size;
    let mut s = String::new();

    let hb = m
        .has_bits_offset
        .map(|o| format!("_has_bits_ @ {o:#x}"))
        .unwrap_or_else(|| "no _has_bits_".to_string());

    // Fields in memory order; drop ones at/after object end, collapse oneof
    // members that share an offset (keep the first).
    let mut fields: Vec<&ProtoField> = m.fields.iter().filter(|f| f.offset < size).collect();
    fields.sort_by_key(|f| f.offset);
    fields.dedup_by_key(|f| f.offset);

    writeln!(s, "#pragma pack(push, 1)").ok();
    writeln!(s, "struct {name} {{ // sizeof {size:#x}, {hb}").ok();

    let mut asserts: Vec<String> = Vec::new();

    if fields.is_empty() {
        if size > 0 {
            writeln!(s, "    uint8_t _data[{size:#x}];").ok();
        }
    } else {
        let mut cursor: u32 = 0;
        for (i, f) in fields.iter().enumerate() {
            let off = f.offset;
            if off < cursor {
                continue;
            }
            if off > cursor {
                writeln!(s, "    uint8_t _pad_{:x}[{:#x}];", cursor, off - cursor).ok();
                cursor = off;
            }
            let next = fields.get(i + 1).map(|n| n.offset).unwrap_or(size);
            let slot = next.saturating_sub(off);
            if slot == 0 {
                continue;
            }
            let fname = sanitize(&f.name);
            let hbit = f
                .has_bit
                .map(|b| format!("has-bit {b}"))
                .unwrap_or_else(|| "no has-bit".to_string());
            match member_type(f, reg, cur_ns, slot) {
                Some((ty, sz)) => {
                    writeln!(s, "    {ty} {fname}; // #{} {}, {}", f.number, proto_ty(f), hbit).ok();
                    if slot > sz {
                        writeln!(s, "    uint8_t _pad_{:x}[{:#x}];", off + sz, slot - sz).ok();
                    }
                }
                None => {
                    writeln!(
                        s,
                        "    uint8_t {fname}[{slot:#x}]; // #{} {}, {}",
                        f.number, proto_ty(f), hbit
                    )
                    .ok();
                }
            }
            asserts.push(format!("static_assert(offsetof({name}, {fname}) == {off:#x});"));
            cursor = off + slot;
        }
        if cursor < size {
            writeln!(s, "    uint8_t _pad_{:x}[{:#x}];", cursor, size - cursor).ok();
        }
    }

    writeln!(s, "    static constexpr std::ptrdiff_t kSizeOf = {size:#x};").ok();
    if let Some(o) = m.has_bits_offset {
        writeln!(s, "    static constexpr std::ptrdiff_t kHasBits = {o:#x};").ok();
    }
    writeln!(s, "}};").ok();
    writeln!(s, "#pragma pack(pop)").ok();
    writeln!(s, "static_assert(sizeof({name}) == {size:#x});").ok();
    for a in asserts {
        writeln!(s, "{a}").ok();
    }
    s.push('\n');
    s
}

/// libprotobuf container shims (x64 layout) used by typed repeated fields.
fn base_decls() -> &'static str {
    "namespace pb {\n\
    \x20   // std::string the message owns (ArenaStringPtr points at one). Opaque.\n\
    \x20   struct string_t;\n\
    \x20   // libprotobuf RepeatedField<T> / RepeatedPtrField<T> (x64 = 0x18).\n\
    \x20   template <class T> struct RepeatedField    { void* arena; int current_size; int total_size; T* elements; };\n\
    \x20   template <class T> struct RepeatedPtrField { void* arena; int current_size; int total_size; void* rep; };\n\
    \x20   static_assert(sizeof(RepeatedField<int>) == 0x18 && sizeof(RepeatedPtrField<int>) == 0x18);\n\
    } // namespace pb\n\n"
}

pub fn render_hpp(map: &ProtobufMap, build_number: Option<u32>) -> String {
    let reg = Registry::build(map);

    let mut s = String::new();
    s.push_str("// Generated by cs2-sdk - https://cs2-sdk.com\n");
    s.push_str("// Real protobuf message structs (offsets recovered from libprotobuf\n");
    s.push_str("// reflection tables). Each is #pragma pack(1) with exact padding;\n");
    s.push_str("// cast a live message pointer and read fields directly. has-bit i:\n");
    s.push_str("//   *(uint32_t*)((char*)msg + Msg::kHasBits) & (1u << i)\n");
    s.push_str("#pragma once\n\n#include <cstddef>\n#include <cstdint>\n\n");
    if let Some(bn) = build_number {
        writeln!(s, "namespace pb {{ inline constexpr std::uint32_t CS2_BUILD = {bn}; }}\n").ok();
    }
    s.push_str(base_decls());

    // Forward-declare every message struct so cross-module / forward field
    // pointers resolve regardless of emission order.
    for (module, messages) in map {
        if messages.is_empty() {
            continue;
        }
        let ns = module_ns(module);
        let mut seen = BTreeSet::new();
        let mut fwd = String::new();
        for m in messages {
            if m.size == 0 {
                continue;
            }
            let n = sanitize(&m.name);
            if seen.insert(n.clone()) {
                writeln!(fwd, "    struct {n};").ok();
            }
        }
        if !fwd.is_empty() {
            writeln!(s, "namespace pb::{ns} {{").ok();
            s.push_str(&fwd);
            writeln!(s, "}} // namespace pb::{ns}\n").ok();
        }
    }

    for (module, messages) in map {
        if messages.is_empty() {
            continue;
        }
        let ns = module_ns(module);
        writeln!(s, "namespace pb::{} {{", ns).ok();
        let mut seen = BTreeSet::new();
        for m in messages {
            if m.size == 0 || !seen.insert(sanitize(&m.name)) {
                continue;
            }
            s.push_str(&struct_block(m, &ns, &reg));
        }
        writeln!(s, "}} // namespace pb::{}\n", ns).ok();
    }
    s
}

pub fn render_json(map: &ProtobufMap) -> String {
    let mut s = String::new();
    s.push_str("{\n");
    let modules: Vec<_> = map.iter().filter(|(_, m)| !m.is_empty()).collect();
    for (mi, (module, messages)) in modules.iter().enumerate() {
        writeln!(s, "  \"{}\": {{", module).ok();
        for (msgi, m) in messages.iter().enumerate() {
            writeln!(s, "    \"{}\": {{", m.name).ok();
            writeln!(s, "      \"size\": {},", m.size).ok();
            writeln!(
                s,
                "      \"has_bits_offset\": {},",
                m.has_bits_offset.map(|o| o as i64).unwrap_or(-1)
            )
            .ok();
            s.push_str("      \"fields\": {\n");
            for (fi, f) in m.fields.iter().enumerate() {
                let comma = if fi + 1 < m.fields.len() { "," } else { "" };
                writeln!(
                    s,
                    "        \"{}\": {{ \"offset\": {}, \"number\": {}, \"has_bit\": {}, \"type\": {}, \"label\": {} }}{}",
                    f.name,
                    f.offset,
                    f.number,
                    f.has_bit.map(|b| b as i64).unwrap_or(-1),
                    f.ty,
                    f.label,
                    comma,
                )
                .ok();
            }
            s.push_str("      }\n");
            let comma = if msgi + 1 < messages.len() { "," } else { "" };
            writeln!(s, "    }}{}", comma).ok();
        }
        let comma = if mi + 1 < modules.len() { "," } else { "" };
        writeln!(s, "  }}{}", comma).ok();
    }
    s.push_str("}\n");
    s
}
