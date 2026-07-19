use std::collections::BTreeMap;

use super::ButtonMap;

pub fn render_hpp(buttons: &ButtonMap) -> String {
    let mut s = String::from("#pragma once\n\n#include <cstddef>\n#include <cstdint>\n\nnamespace button {\n");
    for (name, value) in buttons {
        s.push_str(&format!("    constexpr std::ptrdiff_t {} = {:#X};\n", name, value));
    }
    s.push_str("}\n");
    s
}

pub fn render_json(buttons: &ButtonMap) -> String {
    let map: BTreeMap<_, _> = buttons.iter().collect();
    let content = BTreeMap::from([("client.dll", map)]);
    serde_json::to_string_pretty(&content).unwrap_or_else(|_| "{}".into())
}
