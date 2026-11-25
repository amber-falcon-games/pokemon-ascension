# Project status

Last updated: 2026-11-25 (Claude session: F000 review and F001 audit).
Current phase: 0 — scaffolding and foundation.
Current feature: F001 — upstream audit complete; import BLOCKED on owner decision D010.
F000: AWAITING_ACCEPTANCE — review and amendments in docs/features/F000-scaffold-review.md.
Next eligible feature: F001 import (after D010), then F002 Mac setup/doctor scripts.
Last known-good playable build: none (pre-F006 bootstrap exception).
Current engine version: none imported; recommended pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369, 2026-10-31).
Automated tests: upstream 1.17.2 built (`make -j4`, exit 0) and `make check` run in a Linux cloud container, outside this repository; results in docs/features/F001-upstream-integration.md. No project tests exist yet.
Local Mac environment: unverified. Linux container can build and run tests; Mac scripts (F002, F005) and playtests need the owner's Mac.
Pending owner decisions: D010 — upstream baseline and integration method (recommendation: option A).
Progress by feature status: AWAITING_ACCEPTANCE 1 (F000); BLOCKED 1 (F001); TODO 34 (F002–F035); DONE 0.

## Session handoff
- Branch: `claude/sharp-euler-yfhti2` (documentation only). `main` unchanged at `39bf610`.
- Exact next action once D010 is approved: create `feature/F001-upstream-import` from `main`, `git fetch https://github.com/rh-hideout/pokeemerald-expansion expansion/1.17.2`, merge with `--allow-unrelated-histories`, resolve README/.gitignore/.github collisions as specified in the F001 spec, run `make` and `make check`, open a PR with evidence.
- Blockers: D010 owner decision; F000 owner acceptance.
- Key risks surfaced: save headroom (move library F015), four-move assumption (F014/F015), scheduler depth (F017). None block the foundation phase.
