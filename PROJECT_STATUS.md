# Project status

Last updated: 2026-11-26 (Claude session: F001 upstream import).
Current phase: 0 — scaffolding and foundation.
Current feature: F001 — DONE. Upstream 1.17.2 merged to `main` via PR #1 (merge commit `02efe96`, history preserved).
F000: DONE (owner accepted 2026-11-26).
Next eligible feature: F004a CI speed/badges (D014), after the owner makes the repository public (D013). Then F002 Mac setup/doctor scripts (needs the owner's Mac).
Last known-good playable build: none (pre-F006 bootstrap exception).
Current engine version: pokeemerald-expansion `expansion/1.17.2` (commit 79dd4369) on `main`.
Automated tests: on the merged tree (`bfc0553a56`, Linux container) `make -j4` exit 0 and `make check -j4` exit 0: 5,429 passed / 5,876 total, 0 unexpected failures; ROM byte-identical to pristine upstream. Details in docs/features/F001-upstream-integration.md.
Local Mac environment: unverified. Linux container can build and run tests; Mac scripts (F002, F005) and playtests need the owner's Mac.
Pending owner decisions: none blocking. Recorded D011–D015 (feature cuts, CI, public repo, CI-first order, move-system direction). Waiting on owner: switch repository visibility to public. Owner reviewing the cut-candidate feature list (SAVE_COMPATIBILITY.md); move-storage design deferred to F015.
Progress by feature status: DONE 2 (F000, F001); TODO 34 (F002–F035).

## Session handoff
- `main` at `02efe96`: upstream 1.17.2 plus project docs; builds and passes upstream tests (Linux). Not yet booted in an emulator (F005/F006).
- Next action: F002: Mac setup/doctor scripts. Needs the owner's Mac for verification and permission for privileged installs (devkitPro pacman uses sudo).
- Local only: `rhh` remote for future upstream updates (`git fetch --no-tags rhh refs/tags/expansion/X.Y.Z:...` then merge on a feature branch).
- Note: upstream CI (`build.yml`) will run on PRs, five jobs including FireRed/LeafGreen builds and the full test suite. Trimming it is F004.
- Key risks: save headroom (move library F015, options in SAVE_COMPATIBILITY.md), scheduler depth (F017).
