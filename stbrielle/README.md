# ST. BRIELLE: THE INTERVAL — game data

The standalone game data lives here and mounts with `fs_game stbrielle`.

## Current source

- **The Record:** persistent `sb_*` campaign facts, serialized in engine saves and retained across normal map transitions.
- **Awareness:** hidden systemic state interpreted by map-specific reactions.
- **Cycles:** Ink, Brass, Water, and Code directors.
- **Municipal Annex:** work order, Room 214, ledger, altered photograph and telephone, departure contradiction, filed-person encounter, and street transition.
- **Public Way:** two conflicting evidence cards, a binding certification, visible consequences, and a St. Mercuria dispatch receipt.
- **Record labs:** two small maps for state/transition experiments.

The original hospital chapter, navigated hostile AI, defensive tool, and release-quality art/audio are not implemented. See [development status](design/DEVELOPMENT_STATUS.md) for the distinction between code present and verified behavior.

## Run

Build the x64 **stbrielle** project in `solution/monstergame.sln`, then run `run_stbrielle_annex.bat` from the repository root. The launcher prepares generated content and compiles both playable map sources. Rebuild the executable after C++ changes.

- WASD / mouse: move and look
- E: interact
- Space / Ctrl: jump / crouch
- Shift: alternate movement speed

The street's evidence, instructions, certification receipts, and dispatch are repeatable reads. Both seals require both pieces of evidence. Once signed, the certification cannot be replaced during that campaign.

The dispatch receipt marks the current slice's endpoint. It prepares a hospital entry route in the Record; it does not load an unfinished hospital map.

## Content layout

- `def/`: entity declarations
- `maps/`: original map sources (compiled by the launcher)
- `guis/`: HUD, notices, and municipal interfaces
- `materials/`, `skins/`: original material definitions
- `sound/`: sound declarations; asset preparation scripts live in `../tools/`
- `script/`: standalone bootstrap scripts
- `design/`: current status, game bible, Record contracts, and playtest instructions

## Record and directors

`target_stbrielle_record` changes a fact; `target_stbrielle_recordcheck` conditionally activates map targets. Use `sb_<scope>_<fact>` keys. See [Record keys](design/RECORD_KEYS.md).

Awareness uses `sb_awareness`: Dormant 0, Noticed 1–9, Observing 10–24, Correcting 25–49, Rupture 50–100. The general tier values are hidden from players.

`sb_cycle` uses 0 Ink, 1 Brass, 2 Water, 3 Code. The city exterior currently has Water-era art; it does not yet rebuild its geometry for every Cycle.

## Developer tools

- `sb_recordDump`
- `sb_recordGet <key>`
- `sb_recordSet <key> <value>`
- `sb_recordAdd <key> <integer>`
- `sb_recordClear [key]`
- `sb_awareness`
- `sb_cycle [ink|brass|water|code]`

`sb_recordClear` without a key clears `sb_*` facts only. Do this only when intentionally starting a fresh test campaign. Development `map`/`devmap` commands are not a promise of a clean Record.

The older Record-lab launcher and [lab instructions](design/RUN_RECORDLAB.md) remain available for isolated engine experiments.
