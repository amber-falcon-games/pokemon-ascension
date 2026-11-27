# Project status

Last updated: 2026-11-27 22:00 UTC (Claude session: F015 core done: trees in game, choices, TM rules, storage).
Current phase: Combat track (after the 0.2.0 Battle Lab foundation). D003 gate passed: the Lab is accepted.
Current feature: F015 move library (`feature/F015-move-library`, PR #20, stacked on F014 PR #19). Done: generator, checker, ROM tables, battle list from the tree, choices saved (spare bits), TM rules from D027 (Ascension_TeachMove), TM progress stored in the contest fields. Left: hook TMs and tutors into the party menu (use Ascension_TeachMove and show the refusal text); level-up growth message and choice prompt; Lab tree toggle; docs (SAVE_COMPATIBILITY, CHANGELOG).
Done: F000–F013, F007b and F017 (spike). Releases: v0.1.0 (c204b79399) and v0.2.0 (3faa92d298, Battle Lab foundation; player build unchanged from 0.1.0).
Next eligible feature: F005c (deep cuts; frees the save space F015 needs), then F014 categorised move menu. Order: F005c → F014 → F015 → F016 → F018 → F019 → F020 → F021.
Last known-good playable build: v0.2.0 (3faa92d298), owner smoke-tested and published.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. `make check TESTS=test/ascension/checkpoints.c`: 10/10 PASS locally; every checkpoint was screenshot-checked with gbashot; release build has no checkpoint or Lab symbols.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: none blocking (D027 answered F015). The owner may veto the contest cut that F015 needs for joined-branch storage.
Owner actions: none now. The owner tests the whole overhaul together once it is built (D026); subjective features stay on stacked branches until then.

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc`: menu entry `Ascension_Checkpoint_<CP>` = `Ascension_CPState_<CP>` (story steps `Ascension_Story_GymNToGymN+1` in order + `Ascension_CPTeam_<CP>`) + warp. Menu rows in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`); tests in `test/ascension/checkpoints.c`. Dev builds only (`#if !defined(RELEASE)` in `data/event_scripts.s`).
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- F017 tooling: battle log lines "LAB clock", "LAB energy", "LAB ai" (timer 3, 4389 ticks = 1 frame). Headless comparison ROMs/scripts were scratch only.
- Known F017 findings: end-of-turn effects tick per action (re-time in F018); AI never boosts; recharge passes while sitting out; LAB-011 balance (Slaking one-shots Ninjask).
- Exact next action: hook F015 into the UI. In party_menu.c, the TM/HM use path calls Ascension_TeachMove when ASCENSION_MOVE_LIST is on (messages: jumped, joined, grown past, no room). Then the level-up growth message and the branch-point prompt (evolution/level-up flow). Then F016.
- F015 tools: `python3 tools/ascension/talent_trees.py generate|check`; tests in test/ascension/test_talent_trees.py.
