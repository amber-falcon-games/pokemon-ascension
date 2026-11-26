# Project status

Last updated: 2026-11-26 (Claude session: PR #5 merged; F007 started).
Current phase: 0 — scaffolding and foundation.
Current feature: F007 Battle Lab entry (implementation next). F002–F005 DONE (owner verified on Mac 2026-11-26). F006: owner smoke test + `release.sh 0.1.0` pending.
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
- `main` at `879f7eb` (PR #5): release pipeline + macOS GNU make 4.x fix. Owner's first Mac build failed because macOS make 3.81 silently ignores header-generation failures; fixed, awaiting owner retry.
- Owner dashboard: Ascension Mission Control artifact (https://claude.ai/artifact/TBcNe5AUVsVQBbduRuj2Wv). Update it with ArtifactData at each step (collections data/ascension/{features,actions,activity,state}); run `date -u` before writing timestamps.
- Owner's Mac session can be messaged (owner granted trusted-device access); it cannot message back, so the owner relays its output.
- Exact next action: implement F007 per docs/features/F007-battle-lab-entry.md (title L+SELECT entry, Lab shell menu, LAB-001 debug battle returning to the Lab, release exclusion check), on `claude/sharp-euler-yfhti2`.
