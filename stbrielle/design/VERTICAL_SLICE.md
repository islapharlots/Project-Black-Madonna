# Playable slice — Municipal Annex and Public Way

## Purpose

Prove the central loop: investigate → survive → decide what becomes official → carry the consequence onward. The finished prologue targets approximately 20–30 minutes; the current greybox duration has not been measured in this pass.

## Current flow

1. Read the work order in the Continuity Office.
2. Power **ARCHIVE AUXILIARY / 214**. The blank wall gives way to Room 214.
3. Read the occupant ledger and leave the room. The Annex corrects itself.
4. Answer the reception telephone.
5. Review the impossible departure, then file the authorized return.
6. Encounter the filed person. It advances while unobserved. Hold a clear view long enough to withdraw it. Contact is nonlethal but leaves a persistent exception.
7. Use **LEAVE** at the released exit. The engine transitions to `sb01_cityglimpse` and retains the Record.
8. Read the street instruction plate, north occupancy roster, and south witness slip.
9. At the eastern desk, choose the green or red seal. Both require both pieces of evidence.
10. Collect the central dispatch receipt. This is the current playable endpoint.

## Street certification

| Seal | Account filed | Immediate response | Persistent hospital setup |
| --- | --- | --- | --- |
| Green — accept city | Official zero-transfer roster; witness omitted | Green desk light; no retained witness figure | Public intake; credentials accepted |
| Red — retain witness | Six patient transfers; testimony attached | Red desk light; second figure appears | Service intake; exception attached |

The first certification wins. All documents and receipts can be reread. The hospital assignment is stored, but the hospital map is not implemented yet.

## Launch

Rebuild the x64 **stbrielle** project in `solution/monstergame.sln`, then run:

```text
run_stbrielle_annex.bat
```

The launcher prepares generated assets, compiles `sb00_annex` and `sb01_cityglimpse`, verifies their `.proc`/`.cm` files, and launches the Annex. Compilation logs are `annex_compile.log` and `cityglimpse_compile.log`; the runtime log is `annex_runtime.log`.

## Windows verification checklist

These checks remain required; the portable target-graph tests do not perform them.

### Build and full path

- Build the changed C++ successfully. Do not test with an older executable.
- Run the launcher; confirm both maps compile without leaks or invalid brushes.
- Start a fresh test Record and follow the complete Annex sequence.
- Confirm work order/ledger text, phone, interaction prompts, and hostile sound are readable/audible.
- Look toward the hostile through a wall: it must not count as observed. Give it a clear sustained view: it should withdraw. Break line of sight halfway through: observation must restart.
- Complete one path without contact and another after contact. Confirm the arrival receipt retains the appropriate exception/count/stability.

### Street interaction and presentation

- Inspect all three evidence/instruction plates and the two seals. Confirm every plate is reachable with E and no facade can be walked through.
- Try both seals before reading anything, then after only one evidence card. Confirm informative denial and no certification.
- Read both evidence cards in each order. Confirm the assignment prompt advances.
- Use green: verify public intake, green light, and no witness figure. Try red afterward: verify the original certification remains.
- In a separate fresh test, use red: verify service intake, red light, and visible witness figure. Try green afterward: verify the original certification remains.
- Open one card and then another before the first timeout. The newer card must remain for its own duration.
- Collect and reread dispatch. Confirm completion flags are set, the ending is explicit, and there is no attempt to load a nonexistent hospital map.

### Save/load and persistence

- Save/reload before the Room 214 correction, during the hostile encounter, after reading only one street clue, after certification, and after dispatch.
- Verify the correct prompts, light, witness visibility, Record values, and encounter state resume.
- Reload the street with the existing Record. Confirm the same choice is retained and evidence/receipts remain readable.
- Check `sb_city_certification`, `sb_mercuria_intake_route`, `sb_city_dispatch_received`, and `sb_vertical_slice_complete` in the developer console.

For a deliberately fresh dev test, use `sb_recordClear` before loading the test map. This clears campaign facts; do not do it when testing continuation of an existing save.

## Remaining original slice requirements

A proper hostile AI that searches/loses/reacquires the player, at least one defensive tool, multiple encounter solutions, save stations, alternate Annex completion without Room 214, and a production art/audio pass remain future work. The street's new choice is a first test of player agency, not completion of those systems.
