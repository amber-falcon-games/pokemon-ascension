# Project status

Last updated: 2026-11-28 07:58 UTC (Claude session: F019 built (campaign combat, saved energy, energy items, summary cost); F020 needs owner decisions).
Current phase: Combat track (after the 0.2.0 Battle Lab foundation). D003 gate passed: the Lab is accepted.
Current feature: F019 built (`feature/F019-energy`, PR #23 draft, stacked on F018): campaign battles use Dual bars on the timeline (ASCENSION_COMBAT, dev builds); energy kept between battles, walking recovery, full heals, PP items as energy items, summary shows energy cost. F018 built (`feature/F018-timeline`, PR #22, stacked on F016): end-of-turn effects on 1,000-unit beats, recharge waits for the battler's own action, turn-order strip. F016 built (`feature/F016-move-info`, PR #21, stacked on F015 PR #20): talent page on a second L in the move info box; in-battle Pokédex panel on L in the action menu. F015 move library (`feature/F015-move-library`, PR #20, stacked on F014 PR #19). Done: generator, checker, ROM tables, battle list, choices, TM rules (D027), storage, party-menu TM flow with an 8-move replace picker, level-up growth and branch-point prompts (battle and Rare Candy), evolution keeps branch slots (generator plus runtime remap), docs. Lab rule Moves: Set/Tree (LAB-014). Left before release: the old-save migration for contest fields.
Done: F000–F013, F007b and F017 (spike). Releases: v0.1.0 (c204b79399) and v0.2.0 (3faa92d298, Battle Lab foundation; player build unchanged from 0.1.0).
Next eligible feature: F019 (energy replaces PP; the overhaul reaches the campaign). Order: F014 → F015 → F016 → F018 → F019 → F020 → F021; F005c only when space is needed (D027-3).
Last known-good playable build: v0.2.0 (3faa92d298), owner smoke-tested and published.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. `make check TESTS=test/ascension/checkpoints.c`: 10/10 PASS locally; every checkpoint was screenshot-checked with gbashot; release build has no checkpoint or Lab symbols.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: D028 (F020 queue/cancel/intervene), asked 2026-11-28.
Owner actions: none now. The owner tests the whole overhaul together once it is built (D026); subjective features stay on stacked branches until then.

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc`: menu entry `Ascension_Checkpoint_<CP>` = `Ascension_CPState_<CP>` (story steps `Ascension_Story_GymNToGymN+1` in order + `Ascension_CPTeam_<CP>`) + warp. Menu rows in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`); tests in `test/ascension/checkpoints.c`. Dev builds only (`#if !defined(RELEASE)` in `data/event_scripts.s`).
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- F017 tooling: battle log lines "LAB clock", "LAB energy", "LAB ai" (timer 3, 4389 ticks = 1 frame). Headless comparison ROMs/scripts were scratch only.
- Known F017 findings: end-of-turn effects tick per action (re-time in F018); AI never boosts; recharge passes while sitting out; LAB-011 balance (Slaking one-shots Ninjask).
- Exact next action: F020 is blocked on owner decision D028 (queue length, interruption, intervene, cancel cost; GAME_DESIGN.md open questions). Meanwhile: summary screen showing all 8 talent moves (follow-up), and F021 groundwork that doesn't depend on F020 (AI uses Boost; AI time).
- F015 checks: tests 18 in test/ascension/move_list.c plus test/ascension/talent_growth_battle.c. Headless flows used a local build that adds Rare Candy to a checkpoint (never commit it). Battle tests that turn the move library on set gAscensionMoveListTestOverride; the battle test runner resets it after each test.
- F015 tools: `python3 tools/ascension/talent_trees.py generate|check`; tests in test/ascension/test_talent_trees.py.
