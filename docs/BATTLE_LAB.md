# Battle Lab — mandatory foundation

A developer-only playable sandbox, not an automated test suite substitute. Must be implemented after scaffolding and playable baseline (F006) and accepted at F013 before gameplay overhauls. Reuse upstream debug tools where useful but provide a cohesive user-facing setup/launch loop. Use actual production battle engine, not a mock.

## Foundation requirements
- F007: dedicated debug-build entry from boot/title or equivalent direct launch; no campaign progression or save required. Return to Lab; release builds exclude or disable it.
- F008: configure both teams (1–6 slots), species/forms, levels and active members; searchable/scrollable controls suited to GBA. Validate legal values; clearly distinguish intentional debug overrides.
- F009: configure moves, abilities, held items, HP, status, stats or underlying values, weather/terrain, wild/trainer format and opponent AI. Expose seed where practical. Initially support only upstream mechanics; add new fields with each feature.
- F010: launch real playable battle, restart identical initial configuration, edit and relaunch, exit back to Lab. Battle rewards, XP and state must not mutate campaign save.
- F011: save/load named reusable scenarios; propose a feasible versioned format within save constraints. Provide repository-defined scenario presets via build-time or local tooling. No dependence on emulator save states for canonical scenarios. Document import/export limitations and schema migration.
- F012: inspect battle state, toggle experimental mechanics and debug controls where available; deterministic seed, concise diagnostics, safe reset. Add host-side logs if feasible; no gameplay reliance on debugger.
- F013: prove direct boot, custom setup, play, reset, persistence, isolation and release exclusion with automated checks and owner manual acceptance. Measure memory/ROM overhead.

## Interface
Authentic retro menus with tabs or nested panels: Player Team | Opponent Team | Rules | Scenario | Start Battle. Do not require exact screen arrangement without prototyping at 240x160. Show current scenario, validation errors and clear controls. Allow defaults for rapid setup. Include a one-command launch via scripts/play.sh --lab; --scenario ID selects a defined scenario through a verified feasible method. If direct selection is technically blocked, propose a documented alternative before claiming support.

## Scenario catalogue
LAB-001 baseline 1v1; LAB-002 dry/wet Onix conductivity; LAB-003 defensive Iron Tail; LAB-004 reactive switch; LAB-005 trap trigger; LAB-006 branching move mastery; LAB-007 mid-battle evolution; LAB-008 permanent Mega; LAB-009 six-member trainer battle; LAB-010 overlevelled wild encounter. Only LAB-001 is required at foundation; remaining scenarios activate when mechanics exist, never mark them passed in advance.

## Ongoing contract
Every new combat feature adds appropriate Lab controls, at least one reproducible scenario, regression tests and manual instructions. Maintain a registry linking feature IDs, scenario IDs, configuration, expected results and supported versions. Lab changes must not compromise campaign or release builds.

