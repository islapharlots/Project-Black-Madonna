# ST. BRIELLE: THE INTERVAL

This directory is the beginning of the **standalone St. Brielle game data layer**.

The project is being developed as a first-person horror / immersive-sim FPS using the Skin Deep / idTech 4 source foundation. The existing commercial Skin Deep content is not part of this repository's GPL source release, so all distributable St. Brielle maps, art, audio, models, textures, narrative data, and UI must be original.

## Current foundation

The `st-brielle-foundation` branch currently contains:

- St. Brielle runtime branding and `stbrielle.exe` build target.
- A separate `stbrielle/` game directory mounted with `fs_game stbrielle`.
- **The Record** — persistent campaign facts stored as `sb_*` keys.
- Mapper targets for changing and checking Record facts.
- **Awareness** — hidden 0–100 systemic state with Dormant, Noticed, Observing, Correcting, and Rupture tiers.
- **Awareness Director** — lets each map react differently when an Awareness tier becomes active.
- **Cycle Director** — lets each map react to Ink, Brass, Water, or Code becoming the dominant Cycle.
- Developer console commands for inspecting persistent state.
- Two original greybox Record laboratory maps.
- A Windows launcher that compiles and starts the laboratory.

## Development content layout

- `def/` — St. Brielle entity declarations
- `maps/` — original St. Brielle map sources and compiled maps
- `guis/` — HUD, terminals, menus, civic interfaces
- `materials/` — original material declarations
- `models/` — original models and animations
- `sound/` — original sound shaders and audio
- `script/` — map/game scripts
- `strings/` — localization and narrative text

Some folders will appear only when the first vertical-slice assets are created.

## The Record

`idStBrielleRecord` stores campaign facts in the engine's persistent level dictionary. One map can permanently alter what later maps believe is true.

Example keys:

- `sb_black_madonna_awareness`
- `sb_room_214_exists`
- `sb_vargas_alive`
- `sb_wstb_knows_player`
- `sb_cycle`
- `sb_record_stability`

Mapper-facing targets:

- `target_stbrielle_record` — set/add/toggle/clear a persistent fact.
- `target_stbrielle_recordcheck` — conditionally fire map targets based on a persistent fact.

## Awareness

The canonical system key is:

`sb_awareness`

Tiers:

- **0 — Dormant:** 0
- **1 — Noticed:** 1–9
- **2 — Observing:** 10–24
- **3 — Correcting:** 25–49
- **4 — Rupture:** 50–100

The player is not intended to see these numbers. Maps interpret them through `stbrielle_awareness_director`.

## Cycles

`sb_cycle` uses:

- **0 — Ink**
- **1 — Brass**
- **2 — Water**
- **3 — Code**

Maps interpret the current Cycle through `stbrielle_cycle_director`.

## Developer commands

- `sb_recordDump`
- `sb_recordGet <key>`
- `sb_recordSet <key> <value>`
- `sb_recordAdd <key> <integer>`
- `sb_recordClear [key]`
- `sb_awareness`
- `sb_cycle [ink|brass|water|code]`

## First runnable proof

See:

`design/RUN_RECORDLAB.md`

and run:

`run_stbrielle_recordlab.bat`

after building the x64 **stbrielle** target in Visual Studio 2022.

## Next production target

Once the Record lab compiles and behaves correctly on the local Windows build, development moves to the first playable vertical slice:

`sb00_annex` — **Municipal Annex**

That map will turn these engine primitives into actual St. Brielle horror: the impossible work order, Room 214, environmental interaction, a first hostile encounter, and the first persistent consequence.
