# St. Brielle Record Key Conventions

All game-specific persistent campaign keys should begin with `sb_`.

## Global keys

- `sb_campaign_version` — persistent-state schema version.
- `sb_cycle` — current dominant Cycle: 0 Ink, 1 Brass, 2 Water, 3 Code.
- `sb_awareness` — general systemic awareness value, 0–100.
- `sb_record_stability` — coarse continuity-health value, 0–100.

## Naming rules

Use:

`sb_<scope>_<fact>`

Examples:

- `sb_annex_complete`
- `sb_mercuria_generator_online`
- `sb_wstb_knows_player`
- `sb_miramar_producer_seen`

Avoid generic keys such as `doorOpen` or `missionDone`.

## Boolean values

Store booleans as 0/1.

## Counters

Counters should describe what is being counted:

- `sb_awareness`
- `sb_named_bullets_fired`
- `sb_archive_records_altered`

## Identity and continuity

Facts that can become disputed should be represented as explicit Record keys instead of hidden map-local script variables when future chapters may care about them.

Examples:

- whether a room exists;
- whether a person is recorded alive;
- whether an institution acknowledges the player;
- whether a prior conversation officially occurred;
- which Cycle version of a location was left dominant.

## Debug commands

The source branch exposes:

- `sb_recordDump`
- `sb_recordGet <key>`
- `sb_recordSet <key> <value>`
- `sb_recordAdd <key> <integer>`
- `sb_recordClear [key]`

Calling `sb_recordClear` with no key removes only `sb_*` state and leaves unrelated legacy persistent data alone.

## Public Way investigation contract

| Key | Values | Meaning |
| --- | --- | --- |
| `sb_city_arrived` | 0/1 | Exterior arrival trigger reached; does not imply investigation completion |
| `sb_city_roster_read` | 0/1 | Municipal occupancy roster inspected |
| `sb_city_testimony_read` | 0/1 | Witness slip inspected |
| `sb_city_certification` | absent/0, 1, 2 | Unsigned, accept official city account, retain witness account |
| `sb_mercuria_intake_route` | `public`, `service` | Hospital approach prepared by certification; no hospital map yet |
| `sb_city_dispatch_received` | 0/1 | Assignment receipt collected |
| `sb_city_investigation_complete` | 0/1 | Street investigation finished through dispatch |
| `sb_vertical_slice_complete` | 0/1 | Current slice endpoint reached |

Certification requires both evidence flags and may be written only while unsigned.
The map guards each signing path; HUD prompts are presentation, not authorization.
Evidence and dispatch writes are idempotent `set` operations. Do not add repeatable
Awareness rewards to those reads. Both branches preserve Annex contact/stability.

The previous exterior set `sb_vertical_slice_complete` on arrival. Existing saves
with that legacy flag are not erased. New progression uses the specific city keys;
the legacy flag alone must not authorize certification or hospital entry.
