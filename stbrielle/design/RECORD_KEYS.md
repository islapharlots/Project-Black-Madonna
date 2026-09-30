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
