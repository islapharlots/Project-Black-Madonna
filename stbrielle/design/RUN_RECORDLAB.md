# Running the St. Brielle Record Laboratory

## What this test proves

The two tiny maps verify the campaign feature that matters most before full level production:

1. Map 1 sets `sb_test_door = 1`.
2. A blue confirmation light turns on.
3. Walking to the far end loads Map 2.
4. Map 2 checks `sb_test_door`.
5. If persistent state survived the transition, the green confirmation light turns on.

The maps are intentionally ugly greybox laboratories.

## Build

Open:

`solution/monstergame.sln`

Build the **stbrielle** project for x64.

The project target now writes:

`stbrielle.exe`

to the repository root, matching the existing output layout.

## Development content dependency

At this stage the project still uses the inherited `base` game data as a development dependency for generic engine/game declarations such as the player, triggers, and existing interaction infrastructure.

Do **not** redistribute commercial Skin Deep assets with St. Brielle.

The goal is to replace inherited content progressively with original St. Brielle data until the project no longer requires the commercial data layer.

## Launch

From the repository root run:

`run_stbrielle_recordlab.bat`

It mounts:

`+set fs_game stbrielle`

then compiles both laboratory maps with `dmap` and launches the first one.

## Manual console flow

You can also launch St. Brielle normally and run:

`dmap sb_dev_recordlab`

`dmap sb_dev_recordlab02`

`devmap sb_dev_recordlab`

Useful Record commands:

`sb_recordDump`

`sb_recordGet sb_test_door`

`sb_recordSet sb_test_door 1`

`sb_recordClear sb_test_door`

## Expected map behavior

### Map 1

Walk forward through the first invisible trigger volume.

Console should print a Record mutation for:

`sb_test_door`

The previously disabled blue light should activate.

Continue to the far end of the room to transition to:

`sb_dev_recordlab02`

### Map 2

Walk forward through its first trigger.

The Record check should report TRUE and activate the green light.

If the check reports FALSE, dump the Record and verify whether persistent level info survived the transition.

## Failure modes to inspect first

- **Unknown entityDef target_stbrielle_record** — verify `fs_game` is `stbrielle` and that `stbrielle/def/stbrielle_targets.def` was discovered.
- **Map not found** — run `dmap` on each source map and inspect the console for compile errors.
- **Player/entity defs missing** — the temporary inherited base data layer is not present or not being found.
- **No confirmation light** — use `sb_recordGet sb_test_door`, then trigger/list entities to separate Record-state issues from light activation issues.
