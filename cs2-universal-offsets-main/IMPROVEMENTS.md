# cs2-sdk — improvements & next steps

Short living note for where the dumper is and where it should go. Keep it tight.

## Recently done
- **Callable interfaces** (`interface_classes.hpp`): real pure-virtual structs in
  vtable-slot order — call methods like normal virtuals, no slot indices. Known
  slots are typed/named via a curated table; the rest are `method_<N>`
  placeholders. `ifc::detail::vfunc(inst, &T::M)` resolves an address for hooking.
- **Manual interfaces**: hand-declared wrappers for classes that aren't registered
  interfaces (e.g. `CEnginePVSManager`) via `manual_classes()` in
  `output/interface_classes.rs`.
- **Automatic forward declarations**: `output/mod.rs::emit_auto_forward_decls`
  scans every runtime `SCHEMA_FIELD` type (incl. nested template args) and
  forward-declares anything undefined. No hand-maintained patch list.
- **Runtime/SDK split**: implementation helpers live under `include/impl/`
  (`entity_system.hpp`), using `offsets::` constants + typed structs instead of
  baked literals.
- **Editor-module exclusion**: `amalgamation.rs::EDITOR_MODULES` keeps tool-only
  schemas out of `cs2.hpp` (they carry editor type graphs + Windows-macro enum
  collisions). Headers still emitted to `schemas/`.
- **Typed protobuf fields**: messages → `T*`, strings → `pb::string_t*`, repeated
  → `pb::RepeatedField/RepeatedPtrField<T>`, all with exact padding +
  `static_assert(offsetof/sizeof)`.

## Next steps (high value first)
1. **Schema inheritance** — `Class::parent_name` is captured but unused. Emit
   `struct Derived : Base` so base fields are reachable and layout matches the
   game (removes duplicated fields, enables proper member access across the
   hierarchy).
2. **RTTI-class vtable dumping** — walk vtables for RTTI-named classes, not just
   registered interfaces. Auto-generates wrappers like `CEnginePVSManager` and
   retires the `manual_classes()` curation.
3. **Typed global accessors** — for riprel-resolved global instances (e.g.
   `offsets::engine2::PVSManager`) emit `ifc::engine2::CEnginePVSManager*
   PVSManager(base)` so consumers don't hand-write the cast+offset.
4. **SDK compile smoke-test in CI** — a TU that just `#include <cs2.hpp>` and
   builds, so a bad dump can never ship headers that don't compile (this would
   have caught the broken schema/include-path issues immediately).
5. **Generalize macro-collision handling** — `#pragma push_macro/undef/pop_macro`
   around schema includes for Windows-macro enum names (RELATIVE/ABSOLUTE/…), so
   even editor modules compile if someone includes them directly.

## Protobuf follow-ups
- `oneof` members: emit as a union instead of first-wins offset collapse.
- `map<k,v>` fields: detect + type (currently a repeated entry message).
- Typed enums for scalar enum fields (use the schema/proto enum, not `int32_t`).

## Known limitations (inherent, not bugs)
- Non-schema engine structs (e.g. `CEntityIdentity`) have no `CSchemaSystem`
  binding — only forward-declared; the entity-list walk in `impl/entity_system.hpp`
  keeps a few documented raw offsets out of necessity.
- The forward-decl list and protobuf structs only refresh on a live re-dump.
