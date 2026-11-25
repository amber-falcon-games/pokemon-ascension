# Battle system — design brief

## Command and action model
Preserve Emerald battle presentation, concise menus, one-handed mobile emulator usability. Layer command categories (attack, support, defense, special are examples, not final taxonomy); all learned/current moves accessible. Optional detail shows type, effect, requirements and costs; do not flood main screen with hidden stats or forecasts. In-battle Pokédex exposes known species information without revealing unknown discoveries.

Build a deterministic initiative/action scheduler incrementally. Separate player intent and AI choice from resolution, animations and UI as feasible within upstream architecture. Action duration, recovery, priority, energy and cooldowns are candidate mechanics; actual rules require approval. Player may queue commands and intervene; no autonomous attacks without player intent. Provide cancel and invalidation rules for KO, switching and interrupted actions. Enemy AI obeys same mechanics and resource rules.

## Reactions and switching
Moves may be used as offensive or defensive actions: e.g. Iron Tail intercepts an incoming projectile. Conditional reactions/traps are configured by player and triggered by relevant events. Avoid unbounded chains; define ordering, eligibility, costs and interruption. Switching may be a reactive intercept, pivot opportunity or free in specific situations, not universally free. Provide visible counterplay and prevent endless switches.

## Elemental interactions
Use data-driven, testable conditions and effect rules where practical. Example: dry Ground-type Onix immune to Electric; water applies a temporary wet condition; wet changes electrical interaction; expiry/removal restores immunity. Establish counterplay. Treat exact damage and duration as open. Avoid one bespoke engine path per move pairing. Consider environmental hazards and move interactions when adding rules.

## Pacing and progression
Wild battles faster than trainer/boss encounters, but no automatic level-based sweep design. Ordinary attacks should rarely OHKO; exceptions and engineered setups possible. Prefer balance rules over arbitrary HP floors unless approved. Move mastery may replace or branch techniques, with examples in GAME_DESIGN.md. Support automatic mid-battle evolution when level threshold reached and eligible, subject to decisions on XP timing and state migration; contextual evolution later. Permanent Mega evolution is a persistent branch/form, with eligibility and mutually exclusive paths to be designed.

## First vertical slice
Use Lab scenarios for dry/wet Onix, defensive Iron Tail, trap reaction, and reactive switching. Demonstrate manual control, readable state, same AI rules, clear counterplay, no infinite reaction loops. Integrate only after foundation gates and incremental feature acceptance.

