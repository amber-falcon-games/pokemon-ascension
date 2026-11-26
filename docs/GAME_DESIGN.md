# Game design

## Confirmed direction
- One active Pokémon per side normally, teams of up to six as in classic play; tactical switching, interception and pivot opportunities. No positioning grid.
- Initiative timeline and action-resource/time costs; dynamic queued commands with manual intervention, not autonomous battling.
- All currently learned moves available in layered, categorised Final Fantasy-like menus blended with Emerald UI. Move info on demand; in-battle Pokédex for known Pokémon.
- Offensive moves can be defensive where sensible; conditional reactions and traps; creative elemental interactions.
- Keep default status complexity close to classic initially; expand only where interactions justify it.
- Mid-battle automatic level-triggered evolution; explore contextual triggers such as protecting the trainer. Permanent Mega Evolutions outside battle once unlocked; may be mutually exclusive with other branches.
- Trainer is player, not a separate resource/skill tree. Authentic GBA look, balance spectacle and clarity. No excessive clutter.
- Longer trainer/boss fights, faster wild fights, reduced grinding. Avoid routine OHKO and sweeps unless rare special actions or deliberate setups eliminate counterplay.

## Open design questions (do not treat as approved formulas)
- Timeline pause/advance while selecting; reaction timing and whether preselected reactions auto-resolve; queue length, cancellation and interruption.
- Exact energy model, cooldowns, recovery, speed scaling, resource regeneration, defense costs and anti-loop rules.
- How much elemental sandbox freedom; conditions, terrain, interactions, duration, counterplay and discovery. Example dry Onix immune to Electric, wet Onix temporarily conductive.
- Move evolution/mastery: Ember may branch into Flamethrower or Fire Spin; replacement vs retention, respec, requirements, mastery progression. Concept accepted for investigation, exact trees not approved.
- Expanded status stacking, damage pacing formulas, XP timing, mid-battle evolution triggers and stat recalculation, permanent Mega unlock requirements and branches.
- Additional species/evolutions, story and world details later.

## Owner input 2026-11-26 (direction, not final design)
- Branching move progression via a talent tree. Proposed storage model: moves are derived from species line + level + the player's stored branch choices (e.g. Ember replaced by Flamethrower at level 20, then a level-35 choice between Fire Spin and Flame Wheel; only the choice is stored). Details belong to F015 and F027–F028.
- Battle Lab is debug-only and excluded from release builds (already required by BATTLE_LAB.md).
- Cut-candidate review done; see DECISIONS.md D011.
- Energy replaces PP; turn order from energy, recovery time and Speed (D016). Formulas open.
- Move system (D015): multi-layer talent tree plus a categorised move list larger than four. Feasibility notes (2026-11-26):
  - Save: moves are derived from line + level + choices. 8 branch points with up to 4 options each = 16 bits, which fits in the 22 spare bits per Pokémon. Cutting contests adds 63 bits. Reducing boxes to 10 frees 9,600 B, about 30 B per stored Pokémon for an extension table (e.g. TM/tutor moves).
  - PP: boxed Pokémon have PP restored, so only the party needs per-move PP for the longer list (small). Alternatively PP gives way to the planned energy/cooldown model.
  - Engine: in-battle moves grow from 4 to N (e.g. 12): under 1 KB RAM. The real cost is refactoring code that assumes four moves (484 references) and AI time, which grows roughly linearly with move count. This is the main engineering risk of F014–F015, measured in the F017 spike.
  - Roster: all Gen 1–9 species are already enabled in the current 1.17.2 build (1,573 species/form entries, 1,025 National Dex) with 6.8 MB ROM free; species ID field allows 2,047 entries.

See BATTLE_SYSTEM.md for implementation-facing design and DECISIONS.md for decision state.

