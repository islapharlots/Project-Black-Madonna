# St. Brielle game content

This directory is the beginning of the **standalone St. Brielle game data layer**.

The Skin Deep repository provides engine and gameplay source, but its shipped commercial assets are not part of the GPL source release. St. Brielle therefore uses its own maps, definitions, GUIs, materials, models, textures, audio, and narrative data.

## Working title

**ST. BRIELLE: THE INTERVAL**

## Initial content layout

- `def/` — St. Brielle entity declarations
- `maps/` — original St. Brielle map sources and compiled maps
- `guis/` — HUD, terminals, menus, civic interfaces
- `materials/` — original material declarations
- `models/` — original models and animations
- `sound/` — original sound shaders and audio
- `script/` — map/game scripts
- `strings/` — localization and narrative text

Only the `def/` seed exists yet. The other folders will be added as the vertical slice grows.

## First engine feature: The Record

`idStBrielleRecord` stores campaign facts in the engine's persistent level dictionary. This lets one map permanently alter what later maps believe is true.

Example keys:

- `sb_black_madonna_awareness`
- `sb_room_214_exists`
- `sb_vargas_alive`
- `sb_wstb_knows_player`
- `sb_cycle`
- `sb_record_stability`

The mapper-facing target entities are:

- `target_stbrielle_record` — set/add/toggle/clear a persistent fact.
- `target_stbrielle_recordcheck` — conditionally fire map targets based on a persistent fact.

This is the foundation for alternate rooms, changed dialogue, manifestations, persistent casualties, optional discoveries, and branching mission outcomes.
