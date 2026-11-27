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

#endif // GUARD_CONFIG_ASCENSION_H
