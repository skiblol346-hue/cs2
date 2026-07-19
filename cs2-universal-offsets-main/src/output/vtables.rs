//! `vtables.json` emitter.
//!
//! Straight `serde_json` dump of [`crate::analysis::VTableMap`] after the
//! Pattern name-recovery pass — `module -> interface -> { vtable_rva,
//! vtable_module, rtti_class, methods: [{ index, module, rva, name }] }`.
//! This is the machine-readable counterpart to `interfaces.hpp`; the web
//! site's Vtables tab reads this file directly.

use anyhow::Result;
use serde_json::json;

use crate::analysis::VTableMap;

pub fn render_json(map: &VTableMap) -> Result<String> {
    let modules: serde_json::Map<String, serde_json::Value> = map
        .iter()
        .map(|(module, ifaces)| {
            let ifaces_json: serde_json::Map<String, serde_json::Value> = ifaces
                .iter()
                .map(|(iface_name, info)| {
                    let methods: Vec<serde_json::Value> = info
                        .methods
                        .iter()
                        .enumerate()
                        .map(|(index, m)| {
                            json!({
                                "index": index,
                                "module": m.module,
                                "rva": m.rva,
                                "name": m.name,
                            })
                        })
                        .collect();
                    (
                        iface_name.clone(),
                        json!({
                            "vtable_rva": info.vtable_rva,
                            "vtable_module": info.vtable_module,
                            "rtti_class": info.rtti_class,
                            "methods": methods,
                        }),
                    )
                })
                .collect();
            (module.clone(), serde_json::Value::Object(ifaces_json))
        })
        .collect();

    let root = json!({
        "note": "Primary vtable only (*(void**)instance). Classes reached via secondary vtables (multiple inheritance) are not walked and may be incomplete.",
        "modules": serde_json::Value::Object(modules),
    });

    Ok(serde_json::to_string_pretty(&root)?)
}
