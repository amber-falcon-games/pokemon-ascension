# Project status

Last updated: 2026-11-26 (Claude session: F005b done; F006 next).
Current phase: 0 — scaffolding and foundation.
Current feature: F006 baseline release 0.1.0 — tooling done; awaiting owner Mac run, smoke test and first release. F000, F001, F004a, F005b DONE; F002–F005 awaiting owner Mac run.
F000: DONE (owner accepted 2026-11-26).
Next eligible feature: F007 Battle Lab entry (can start while the owner verifies 0.1.0).
Last known-good playable build: none (pre-F006 bootstrap exception).
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: on the merged tree (`bfc0553a56`, Linux container) `make -j4` exit 0 and `make check -j4` exit 0: 5,429 passed / 5,876 total, 0 unexpected failures; ROM byte-identical to pristine upstream. Details in docs/features/F001-upstream-integration.md.
Local Mac environment: unverified. Linux container can build and run tests; Mac scripts (F002, F005) and playtests need the owner's Mac.
Pending owner decisions: none. OWNER ACTION NEEDED: run the Mac steps below and paste the output back.
Progress by feature status: DONE 2 (F000, F001); TODO 34 (F002–F035).

## Owner action (Mac)
```bash
git clone https://github.com/amber-falcon-games/pokemon-ascension.git   # skip if already cloned
cd pokemon-ascension && git checkout main && git pull
./scripts/setup.sh
./scripts/play.sh
./scripts/test.sh test/battle/move_effect/dragon_darts.c
```
Expected: setup ends with "Ready to build."; play prints the SHA-1 and opens mGBA at the Emerald title screen; the test prints 16 passed. Paste the output back.
Then, for release 0.1.0: run the smoke test in docs/features/F006-baseline-release.md, put your Emerald dump at `.local/base/emerald.gba`, and run `./scripts/release.sh 0.1.0`.

## Session handoff
- `main`: upstream 1.17.2 + docs + F004a CI (PR #2, `520ffeb`). Nightly build/tests dispatched once to seed caches and badges.
- Scripts for F002–F005 on `claude/sharp-euler-yfhti2` (PR pending/merged — check `git log main`).
- Exact next action: owner's Mac run (above). In parallel: F005b lean cuts — enable upstream FREE_* switches for cut features, I_VS_SEEKER_CHARGING, reduce TOTAL_BOXES_COUNT to 10; spec first. Deeper removals (Secret Bases, TV, Contests, Mail, flavour NPCs, recorded-battle sector) are split into F005c before F015, when the space is needed.
- Owner preferences: docs/DEVELOPMENT_WORKFLOW.md.
