# Testing and Mac quickstart

Status: planned, not verified. Claude must inspect Apple Silicon environment, official upstream macOS instructions, installed Xcode command-line tools, Homebrew dependencies, devkitARM and mGBA. Pin compatible versions; request privileged changes. Document exact setup after testing from fresh checkout. Do not assume native compatibility or hardcode user paths.

Planned scripts (not yet implemented):
- ./scripts/setup.sh — idempotent setup; prerequisites and clear failures.
- ./scripts/doctor.sh — architecture, toolchain/version and emulator checks.
- ./scripts/build.sh — reproducible compile; print output path; no stale ROM on failure.
- ./scripts/test.sh — upstream automated tests plus project regression; nonzero on failures; distinguish skips.
- ./scripts/play.sh — successful build then open mGBA; document controls and save location.
- ./scripts/play.sh --lab — debug build directly to Battle Lab.
- ./scripts/play.sh --scenario LAB-001 — select reproducible scenario; verify supported mechanism.
- ./scripts/status.sh — current feature, next task, blockers and last known good version.

After F006 run baseline boot/new game/battle/save/load smoke tests. After F013 verify Lab team configuration, start, reset, save/load scenarios, no campaign mutation and release exclusion. Add feature-specific tests for command validity, RNG determinism, AI parity, resource bounds, invalidation, reaction ordering, loop termination, switch and evolution state. Record exact build hash, test commands, results, skipped tests, resource deltas and manual acceptance. Keep debug and campaign saves isolated. CI should run build/test without public ROM artifacts. Use upstream tooling first; confirm actual workflows before claiming functionality.

