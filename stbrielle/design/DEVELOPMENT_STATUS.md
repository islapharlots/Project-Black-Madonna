# Development status — 2026-10-29

## Assessment

The foundation has a distinctive playable idea: a municipal employee investigates a city whose official records change reality. The next priority is to make that idea readable, reliable, and expressive within one small mission.

The reviewed foundation commit was `8f74b9c` (`Finish Annex handoff into St. Brielle`). Its exterior stopped at an arrival notice. The README and vertical-slice document lagged behind the implemented Annex sequence.

## What exists in source

| Area | Current implementation | Remaining gap |
| --- | --- | --- |
| Campaign memory | `sb_record.cpp`, map targets, save serialization in `Game_local.cpp` | Native save/load regression playtest |
| Annex | Work order → breaker → Room 214 → ledger → correction → phone → exit review/return → filed person → departure | Timing, navigation, original environmental detail, alternate solutions |
| First hostile | Moves while unobserved; sustained observation withdraws it; contact affects Record | Not a full pathfinding/search AI; no combat alternatives |
| Player | Movement, collision trace interaction, civic notice HUD | Minimal controller bypasses much of the inherited game loop; defensive tools and richer interaction need deliberate integration |
| Exterior | Street geometry, arrival consequence, new evidence/certification/dispatch loop | Windows rendering/collision/playtime verification; eventual hospital transition |
| Four Cycles | Persistent values and target directors | Authored playable variations across meaningful spaces |
| Build | VS2022 project, dependency/asset preparation, two-map launcher | Reproducible clean Windows build and automated native smoke checks |

The original design allows leaving Room 214 absent. The current Annex completion check requires its ledger, making Room 214 mandatory. Keep that distinction explicit; an alternate Annex path is future work.

## This implementation pass

- Adds north-side municipal occupancy evidence and south-side witness testimony.
- Requires both accounts before either certification can be filed.
- Makes the first signature binding. Repeat reading and repeated dispatch collection do not accumulate rewards or replace the outcome.
- Accepting the city stores public hospital intake; retaining testimony stores service intake and reveals a second figure on the street. The certification desk light also changes.
- Keeps assignment guidance visible after timed cards close.
- Moves the current slice's completion flag from arrival to dispatch collection.
- Requires a clear collision trace before the filed person counts as observed; looking toward it through a wall no longer withdraws it.
- Prevents an older notice's timeout from closing a newer notice.
- Repairs three inverted X brush intervals in the original street and makes the street facades/tower solid.

No hospital map or new combat system is implied by the dispatch assignment.

## Verification

**Passed:** 12 portable tests for map structure, material/entity references, positive brush bounds, evidence order, denied early actions, exclusive outcomes, repeated use, Record roundtrip/map revisit, and invariants across all reachable target-graph states. `git diff --check` also passed.

**Not verified here:** Visual Studio compilation, `dmap2`, graphics/audio, actual collision/navigation, native save/load, and a timed playthrough. The available Linux compiler encounters inherited platform/header and build assumptions before it can validate these engine translation units. The Record roundtrip test is a graph-level simulation, not a native savegame test.

Use the Windows checklist in [VERTICAL_SLICE.md](VERTICAL_SLICE.md) before merging this pass as runtime-verified work. There is no compiled binary in this changeset.

## Next milestones, in order

### 1. Lock the prologue

Build and run on Windows from a clean checkout. Play both street outcomes. Save/reload before and during the hostile encounter and at every street decision boundary. Establish a reliable start/continue/reset flow and a useful save interaction. Give the observation mechanic an environmental clue, then measure whether a first-time player understands it without explanation.

Exit criterion: both full paths complete reliably, the Record survives a real save and transition, and the player always understands the next actionable task.

### 2. St. Mercuria: one complete mission

Build a compact receiving wing instead of the whole hospital. The street certification selects public admission or a service approach. Both routes reach the same central contradiction: the ledger says zero transfers, but six patients have been moved.

Use three connected spaces: intake, service/power, and an observation ward. Give one threat actual search, loss, and reacquisition behavior. Provide one defensive tool and three solutions: confront, distract/bypass, or isolate it with powered doors. A power decision should change both traversal and which patient record can be verified.

Exit criterion: a complete 15–20 minute target experience with distinct approaches, one consequential record decision, a reliable endpoint, and no dependence on commercial Skin Deep assets. Target duration must be measured, not inferred from code.

### 3. Production-quality slice

Replace essential greybox assets with coherent municipal/gothic architecture, readable signage, sound cues, and restrained environmental animation. Profile frame time and loading. Improve remapping, subtitles/text legibility, pause behavior, and save compatibility. Move level-specific sequence logic out of the shared Awareness director as the campaign grows.

Only expand to WSTB and larger districts after this slice survives repeated outside playtests.
