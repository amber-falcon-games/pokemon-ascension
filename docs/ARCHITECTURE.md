# Architecture — provisional

Proposed upstream: https://github.com/rh-hideout/pokeemerald-expansion. No selected version or source audit yet. F001 must assess stable releases, upstream history/licences, build dependencies, tests, debugging, ROM/EWRAM/IWRAM/save budgets, battle scripts, four-move assumptions, AI and UI. Record actual measurements and file references. Propose integration strategy preserving this scaffold and upstream attribution; do not flatten or replace history without approval.

Prefer incremental adapters and data tables over a wholesale rewrite. Logical boundaries (adapt to upstream): player commands/AI -> scheduler -> reaction/interaction/resource/resolution -> authoritative battle state -> UI/animation. Reuse production battle code in Lab; isolate Lab setup/save and debug-only entry. Seeded randomness, bounded event chains, explicit event ordering and state invalidation. Existing mechanics remain available until replacements are proven. Measure ROM size, memory, stack, frame responsiveness, AI time and save capacity per milestone. Document schema and migration before changing persistent structures. Escalate if limits cannot support the intended gameplay.

No implementation details in this file are verified yet. Update after source review and spikes.

