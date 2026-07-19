//! Read-only walker for the client.dll **CGameEventManager** — enumerates every
//! registered (legacy Source1) game event with its numeric id and typed field
//! schema, exactly as parsed from the `resource/*.gameevents` files.
//!
//! Layout reversed from client.dll (build 14169), anchored on the
//! `pGameEventManager` global (a `CGameEventManager*`):
//! ```text
//! *g_pGameEventManager  ->  CGameEventManager:
//!   +0x08  event count (u32)
//!   +0x10  descriptor array base ptr        (stride 88, indexed by event id)
//!   +0xC4  info-vector size/flags (u32, &0x7FFFFFFF)
//!   +0xC8  info-vector base ptr             (stride 32: name char* @+16, id u32 @+24)
//!
//! CGameEventDescriptor (88 bytes):
//!   +0x08  id (u32)   +0x0C  name-index (u32, into info-vector)
//!   +0x28  local flag (u8)
//!   +0x38  key count (u32)   +0x40  keys array base ptr   (key stride 72)
//!
//! Key (72 bytes):  +0x08 name char*   +0x38 description char*   +0x40 type (u32)
//! ```
//! Type ids (from the manager's own string->int table): 1 string, 2 float,
//! 3 int32(long), 4 int16(short), 5 byte, 6 bool, 7 uint64, 8 player_pawn,
//! 9 player_controller, 0 local.

use anyhow::{Result, bail};
use memflow::prelude::v1::*;
use serde::Serialize;

const MAX_EVENTS: u32 = 4096;
const MAX_KEYS: u32 = 512;

const M_COUNT: u64 = 0x08;
const M_DESC_BASE: u64 = 0x10;
const M_INFO_FLAGS: u64 = 0xC4;
const M_INFO_BASE: u64 = 0xC8;

const DESC_STRIDE: u64 = 88;
const D_ID: u64 = 0x08;
const D_NAME_IDX: u64 = 0x0C;
const D_LOCAL: u64 = 0x28;
const D_KEY_COUNT: u64 = 0x38;
const D_KEYS_BASE: u64 = 0x40;

const INFO_STRIDE: u64 = 32;
const INFO_NAME: u64 = 16;

const KEY_STRIDE: u64 = 72;
const K_NAME: u64 = 0x08;
const K_DESC: u64 = 0x38;
const K_TYPE: u64 = 0x40;

#[derive(Debug, Serialize)]
pub struct GameEventField {
    pub name: String,
    #[serde(rename = "type")]
    pub type_name: &'static str,
    #[serde(skip_serializing_if = "String::is_empty")]
    pub description: String,
}

#[derive(Debug, Serialize)]
pub struct GameEvent {
    pub name: String,
    pub id: u32,
    pub local: bool,
    pub fields: Vec<GameEventField>,
}

pub fn walk<P: Process + MemoryView>(
    process: &mut P,
    manager_global_va: u64,
) -> Result<Vec<GameEvent>> {
    let manager = rd_u64(process, manager_global_va);
    if manager == 0 {
        bail!("g_pGameEventManager is null");
    }

    let count = rd_u32(process, manager + M_COUNT);
    let desc_base = rd_u64(process, manager + M_DESC_BASE);
    let info_size = rd_u32(process, manager + M_INFO_FLAGS) & 0x7FFF_FFFF;
    let info_base = if info_size != 0 {
        rd_u64(process, manager + M_INFO_BASE)
    } else {
        0
    };
    if desc_base == 0 || count == 0 || count > MAX_EVENTS {
        bail!("game event manager looks unpopulated (count={count})");
    }

    let mut events = Vec::new();
    for id in 0..count {
        let desc = desc_base + DESC_STRIDE * id as u64;
        // Event name via the info-vector, indexed by the descriptor's name-index.
        let name_idx = rd_u32(process, desc + D_NAME_IDX);
        let name = if info_base != 0 && name_idx < info_size {
            let np = rd_u64(process, info_base + INFO_STRIDE * name_idx as u64 + INFO_NAME);
            rd_cstr(process, np)
        } else {
            String::new()
        };
        if name.is_empty() {
            continue;
        }

        let local = rd_u8(process, desc + D_LOCAL) != 0;
        let key_count = rd_u32(process, desc + D_KEY_COUNT);
        let keys_base = rd_u64(process, desc + D_KEYS_BASE);

        let mut fields = Vec::new();
        if keys_base != 0 && key_count <= MAX_KEYS {
            for k in 0..key_count {
                let key = keys_base + KEY_STRIDE * k as u64;
                let kname_ptr = rd_u64(process, key + K_NAME);
                let kname = rd_cstr(process, kname_ptr);
                if kname.is_empty() {
                    continue;
                }
                let ktype = rd_u32(process, key + K_TYPE);
                let kdesc_ptr = rd_u64(process, key + K_DESC);
                fields.push(GameEventField {
                    name: kname,
                    type_name: type_name(ktype),
                    description: rd_cstr(process, kdesc_ptr),
                });
            }
        }

        events.push(GameEvent {
            name,
            id: rd_u32(process, desc + D_ID),
            local,
            fields,
        });
    }

    events.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(events)
}

fn type_name(t: u32) -> &'static str {
    match t {
        0 => "local",
        1 => "string",
        2 => "float",
        3 => "int32",
        4 => "int16",
        5 => "byte",
        6 => "bool",
        7 => "uint64",
        8 => "player_pawn",
        9 => "player_controller",
        _ => "unknown",
    }
}

fn rd_u64<P: MemoryView>(process: &mut P, va: u64) -> u64 {
    process.read::<u64>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_u32<P: MemoryView>(process: &mut P, va: u64) -> u32 {
    process.read::<u32>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_u8<P: MemoryView>(process: &mut P, va: u64) -> u8 {
    process.read::<u8>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_cstr<P: MemoryView>(process: &mut P, ptr: u64) -> String {
    if ptr == 0 {
        return String::new();
    }
    process.read_utf8_lossy(Address::from(ptr), 128).data_part().unwrap_or_default()
}
