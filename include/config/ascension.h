#ifndef GUARD_CONFIG_ASCENSION_H
#define GUARD_CONFIG_ASCENSION_H

// Pokémon Ascension project switches.

// Battle Lab (F007+): developer sandbox opened with L + SELECT on the title screen.
// Disabled automatically in `make release` builds, so players never see it.
#define ASCENSION_BATTLE_LAB    DISABLED_ON_RELEASE

// Move list (F014): battlers carry up to MAX_BATTLE_MOVES moves; until F015, slots 4+ are filled from
// the species' level-up learnset. Off in release builds until the combat overhaul ships.
// Off in the upstream test build (its tests define exactly 4 moves); F014's own tests turn it on
// at run time with gAscensionMoveListTestOverride.
#if TESTING
#define ASCENSION_MOVE_LIST     (gAscensionMoveListTestOverride)
#else
#define ASCENSION_MOVE_LIST     DISABLED_ON_RELEASE
#endif

// Combat overhaul in the campaign (F019): every eligible campaign battle uses Dual bars (health and
// energy) on the timeline. Off in release builds until the overhaul ships; off in the upstream
// test build, where tests turn it on at run time with gAscensionCombatTestOverride.
#if TESTING
#define ASCENSION_COMBAT        (gAscensionCombatTestOverride)
#else
#define ASCENSION_COMBAT        DISABLED_ON_RELEASE
#endif

// The contest data the overhaul reuses (F015 talent data, F019 energy) is not shown as ribbons.
#define ASCENSION_CONTEST_FIELDS_REUSED (ASCENSION_MOVE_LIST || ASCENSION_COMBAT)

#endif // GUARD_CONFIG_ASCENSION_H
