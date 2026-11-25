# Development workflow

1. Read current status, roadmap, decisions and git state. Pick next eligible feature.
2. Write feature spec before coding, identify decisions and dependencies. Request meaningful input only.
3. Create focused feature branch from verified baseline. Implement small increments with flags where needed.
4. Build and test; record exact commands/results, relevant memory/ROM budget, save compatibility and skips.
5. Provide runnable local build and Lab scenario (once available). Subjective gameplay requires owner approval before merge.
6. PR includes scope, test evidence, manual steps, rollback and compatibility. Merge only when gates satisfied; retain last stable version.
7. Update status, roadmap, decisions and changelog. Tag verified milestones. Record next action and blockers even on interrupted sessions.

## User request format
DECISION REQUIRED / ASSET REQUIRED / PLAYTEST REQUIRED / PERMISSION REQUIRED; stable ID; context; why needed; options (where applicable); recommendation; expected input; what happens next. For playtests include branch/commit, exact launch command, controls, scenario, steps, expected results, feedback question and how to report bugs. Avoid bundling unrelated decisions.

## Definition of done
Acceptance criteria met; actual successful build and relevant automated tests; regression and resource checks documented; manual instructions and Lab scenario where applicable; required owner acceptance; save compatibility and rollback documented; clean focused commit/PR and status updated. No fabricated results. Before F006 document bootstrap exceptions; after F006 main remains playable. If blocked, preserve stable state and report.

Use GitHub issues/milestones optionally as mirrors; roadmap/status are canonical. Do not expose ROMs, saves or secrets in CI/release artifacts. Request approval for privileged installs and significant architecture/gameplay choices.

