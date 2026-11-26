# Architecture — provisional

Proposed upstream: https://github.com/rh-hideout/pokeemerald-expansion. No selected version or source audit yet. F001 must assess stable releases, upstream history/licences, build dependencies, tests, debugging, ROM/EWRAM/IWRAM/save budgets, battle scripts, four-move assumptions, AI and UI. Record actual measurements and file references. Propose integration strategy preserving this scaffold and upstream attribution; do not flatten or replace history without approval.

Prefer incremental adapters and data tables over a wholesale rewrite. Logical boundaries (adapt to upstream): player commands/AI -> scheduler -> reaction/interaction/resource/resolution -> authoritative battle state -> UI/animation. Reuse production battle code in Lab; isolate Lab setup/save and debug-only entry. Seeded randomness, bounded event chains, explicit event ordering and state invalidation. Existing mechanics remain available until replacements are proven. Measure ROM size, memory, stack, frame responsiveness, AI time and save capacity per milestone. Document schema and migration before changing persistent structures. Escalate if limits cannot support the intended gameplay.

## Measured upstream facts (F001 audit, expansion/1.17.2, 2026-11-25)
Built in a Linux container (arm-none-eabi-gcc 13.2.1); not yet imported or verified on Mac. Details and file references: docs/features/F001-upstream-integration.md.
- EWRAM 226,416 / 262,144 B (35,728 free, including a 115,968 B heap used for battle structures); IWRAM 28,376 / 32,768 B (4,392 free); ROM 26.75 / 32 MB (6.8 MB free).
- `MAX_MON_MOVES` = 4, referenced 484 times in 59 files; four moves stored per Pokémon in party and boxes.
- Turn-based battle core: battle_script_commands.c (12.4k lines), battle_util.c (11.3k), AI ≈ 17.4k, battle scripts 6.5k.
- Declarative battle test DSL with ≈5,000 battle/AI tests run headless in bundled mGBA.
- Debug builds include overworld debug menu, in-battle debug menu and text-format debug trainers; release builds disable them.

Everything else in this file remains provisional until import and spikes.

