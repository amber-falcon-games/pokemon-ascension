# Project status

Last updated: 2026-11-26 12:59 UTC (Claude session: F007b part 1, PR #8).
Current phase: 0.2.0 Battle Lab track (F007–F013), with F007b dev checkpoints.
Current feature: F007b dev checkpoints. Part 1 (CP-START, CP-BALLS, CP-GYM1) is in PR #8 on `feature/F007b-dev-checkpoints`; part 2 (CP-GYM2–8 and automated tests) is next.
Done: F000–F005b. F006: the owner ran the smoke test. Release 0.1.0 has **not** been published: there is no tag or GitHub release, and `release.sh` was probably left waiting at a prompt. F007 is merged (PR #7) and waits for the owner's Battle Lab playtest.
Next eligible feature: F007b part 2. Then F008, the Battle Lab team builder.
Last known-good playable build: `main` @ 841465262e (dev). There is no tagged release yet.
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: CI runs on each PR. F007b was built locally (`make -j8` exit 0); the checkpoints were checked headlessly with gbashot screenshots.
Local Mac environment: verified by the owner (setup, play, 16 tests passed).
Pending owner decisions: none.
Owner actions: (1) finish `./scripts/release.sh` for 0.1.0; (2) playtest the Battle Lab (`./scripts/play.sh --lab`); (3) optionally, try the checkpoints on the PR #8 branch.

## Session handoff
- Branches (D021): each feature is on its own `feature/F0xx-*` branch and they stack. `feature/F007b-dev-checkpoints` also carries the session-branch commits: gbashot, D021 and the Done-button convention.
- Checkpoints: `data/scripts/ascension_checkpoints.inc` (shared steps IntroDone → StarterToPokeBalls → PokeBallsToGym1) and the menu in `src/debug.c` (`sDebugMenu_Actions_Checkpoints`). For each gym, add a `<prev>To<GymN>` step traced from `data/maps/*/scripts.inc`, then a menu row.
- Screenshots: `tools/ascension/shots/` (gbashot, built with libmgba-dev). Screenshots go only to the dashboard, never into the repo.
- Owner dashboard: https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv (ArtifactData collections data/ascension/{features,actions,activity,state,shots}). Run `date -u` before writing timestamps. Check whether the owner pressed Done (`status: done`, `done_by: owner`). Set state to idle at the end of a turn.
- Exact next action: once PR #8 CI is green, merge it (dev-only tooling). Then F007b part 2: GYM2 Brawly (Makuhita 19) through GYM8 Juan (Kingdra 46). The team grows by one per gym, up to 6, built on the Mudkip line.
