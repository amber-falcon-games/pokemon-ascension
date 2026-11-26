# Decision log

Confirmed design directions (D001–D009 recorded by the owner at bootstrap, 2026-11-25; options and rationale were not captured at the time):
- D001: Pokémon Ascension; Emerald-style ROM hack; Emerald Expansion proposed, version unselected.
- D002: Always runnable after baseline; incremental features and owner acceptance.
- D003: Battle Lab mandatory after scaffolding/baseline and before combat changes; configurable real battles.
- D004: Initiative/time/action-resource dynamic combat, manual commands, no autobattler or grid positioning.
- D005: All learned moves via layered menus; optional move info and known-species Pokédex.
- D006: Defensive use of moves, conditional reactions, traps, tactical switching and elemental interactions.
- D007: Permanent Mega progression; automatic mid-battle level evolution; explore contextual triggers.
- D008: Authentic Emerald pixel art, mobile-friendly minimal clutter; trainer/boss pacing longer than wild; discourage routine OHKO/sweeps.
- D009: Branching move mastery/evolution is a direction to prototype, not a fixed design.

Decided:
- D010 (raised 2026-11-25, F001) — upstream baseline and Git integration method. Options A–D in docs/features/F001-upstream-integration.md. Recommendation: A — pin `expansion/1.17.2` and merge upstream history with `--allow-unrelated-histories`, keeping our docs. Owner response 2026-11-26: **approved A**. Implication: future upstream updates are deliberate `git merge` of a release tag on a feature branch.

To be raised at F015 (not decided): how to recover save space for an expanded move library — see SAVE_COMPATIBILITY.md options S1–S5.

Open: timeline wait/active, energy/cooldown formulas, reaction ordering, move tree replacement/retention, type immunity override rules, XP timing, save migration and other GAME_DESIGN.md items. Assign numbered decision IDs as needed, recording date, options, recommendation, owner response and implications. No presumed approval.

