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

- D011 (2026-11-26) — optional Emerald features. Owner direction; implementation scheduled per item, each change behind its own spec and save-compatibility note (nothing removed yet).
  - Cut: Recorded Battle, Secret Bases and decorations, Mystery Gift / Mystery Event / e-Reader, record mixing, TV shows / PokéNews / Gabby & Ty, Contests (incl. Pokéblocks, contest stats and ribbons), Mail, Union Room, link battle records, Match Call (rematches move to an in-world mechanism such as the upstream VS Seeker), redundant Pokédex seen-flag copies.
  - Also cut (owner, later on 2026-11-26): Trainer Hill, Mauville Old Man / Lilycove Lady / Dewford Trends / Walda phrase, link-cable minigames (Berry Crush, Pokémon Jump, Dodrio Berry Picking).
  - Keep: Hall of Fame, Battle Frontier (for now), fusion storage (official fusions; curated custom fusions are a future idea), Daycare/breeding, roaming legendaries (to be expanded with quests).
  - PC boxes: reduce from 14 to 10 (300 slots); can grow again later if space allows.
  - Replacement concept: in-game "mystery gift" style events (hidden eggs, gift NPCs, special battles); design later.
- D012 (2026-11-26) — CI direction for F004: run build and tests on pull requests only, not on pushes to `main`; add build/test status badges to README; cut CI time (caching, drop unneeded jobs). Owner refinement: nightly status run on `main` only if `main` changed that day, run asynchronously; badges in README; selective builds/tests so unchanged work is not redone. Caching and CI speed come before further feature work (see D014).
- D013 (2026-11-26) — repository becomes public. Rationale: noncommercial fan hack, free unlimited Actions minutes and faster public runners. Constraints unchanged: no ROMs, saves or secrets committed; releases are `.bps` patches only. Owner changes the visibility in GitHub settings.
- D014 (2026-11-26) — roadmap order: CI speed-up and badges (F004a) are pulled ahead of F002, since they need no Mac. `scripts/test.sh` (F004b) stays after F003.
- D015 (2026-11-26) — move system direction (supersedes the technique-slot suggestion, which owner rejected because it removes role choice): multi-layer talent tree per evolution line; Final Fantasy-style categorised move list showing more than four moves, with tree upgrades replacing earlier moves. Cost analysis recorded in GAME_DESIGN.md; formulas, trees and PP/energy handling remain open for F014–F015 and F027–F028.
- D016 (2026-11-26) — PP is replaced by energy. Battle order is determined by energy, recovery time (where a move has one) and Speed. Exact costs, regeneration, recovery durations, Speed scaling and anti-stall rules remain open (F017 spike, F018–F019) and need owner approval.
- D017 (raised 2026-11-26, pending) — release process. Recommendation: deliberate releases. Owner runs `scripts/release.sh X.Y.Z` (or a manual GitHub trigger), which builds `make release`, produces a `.bps` patch against the owner's own Emerald dump, tags `vX.Y.Z` and publishes a GitHub Release with patch and changelog. Not on every merge. The base ROM never leaves the owner's machine, so patch generation runs locally, not in public CI. Semantic versions tied to milestones (0.1.0 at F006).

To be raised at F015 (not decided): how to recover save space for an expanded move library — see SAVE_COMPATIBILITY.md options S1–S5.

Open: timeline wait/active, energy/cooldown formulas, reaction ordering, move tree replacement/retention, type immunity override rules, XP timing, save migration and other GAME_DESIGN.md items. Assign numbered decision IDs as needed, recording date, options, recommendation, owner response and implications. No presumed approval.

