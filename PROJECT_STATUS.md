# Project status

Last updated: 2026-11-26 (Claude session: F004a in progress).
Current phase: 0 — scaffolding and foundation.
Current feature: F004a — CI speed-up (ACTIVE on `claude/sharp-euler-yfhti2`). F000, F001 DONE.
F000: DONE (owner accepted 2026-11-26).
Next eligible feature: F002 Mac setup/doctor scripts (needs the owner's Mac), then the approved order in docs/ROADMAP.md.
Last known-good playable build: none (pre-F006 bootstrap exception).
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: on the merged tree (`bfc0553a56`, Linux container) `make -j4` exit 0 and `make check -j4` exit 0: 5,429 passed / 5,876 total, 0 unexpected failures; ROM byte-identical to pristine upstream. Details in docs/features/F001-upstream-integration.md.
Local Mac environment: unverified. Linux container can build and run tests; Mac scripts (F002, F005) and playtests need the owner's Mac.
Pending owner decisions: none blocking. Repository is public (D013, done by owner). Approved order and continuous work: D018.
Progress by feature status: DONE 2 (F000, F001); TODO 34 (F002–F035).

## Session handoff
- `main` at `e7e19d1`: upstream 1.17.2 + project docs; builds and passes upstream tests (Linux). Not yet booted in an emulator.
- F004a work on `claude/sharp-euler-yfhti2`: new `.github/workflows/ci.yml` (PR-only; docs-only skip; changed-tests-only; full suite sharded 4 ways), `nightly-build.yml` / `nightly-tests.yml` (run only if `main` changed; carry previous result forward; seed caches on main), `.github/actions/setup` and `.github/ci/*.sh` (incremental build cache via timestamps), sharding support in `tools/mgba-rom-test-hydra/main.c` (`TEST_SHARD_COUNT`/`TEST_SHARD_INDEX`). Upstream `build.yml` removed (keep deleted on upstream merges).
- Exact next action: verify cache scripts locally, open the F004a PR, measure CI timings for docs-only / test-only / full, merge, then dispatch both nightly workflows once to seed caches and badges. Then F002.
- Owner preferences: see docs/DEVELOPMENT_WORKFLOW.md (nudge on decisions, builds ready, playable features, required testing).
