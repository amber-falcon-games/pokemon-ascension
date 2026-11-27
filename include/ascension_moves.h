#ifndef GUARD_ASCENSION_MOVES_H
#define GUARD_ASCENSION_MOVES_H

#include "global.h"
#include "config/ascension.h"

// Pokémon Ascension F014: the battle move list (MAX_BATTLE_MOVES). Slots 0-3 are the saved moves;
// with ASCENSION_MOVE_LIST on, slots 4+ are filled when the Pokémon enters battle (until F015's
// move library: the species' most recent level-up moves it doesn't already know).
#if TESTING
extern bool8 gAscensionMoveListTestOverride;
#endif
struct Pokemon;
struct BattlePokemon;
u32 Ascension_FillExtraBattleMoves(struct Pokemon *mon, enum Move moves[MAX_BATTLE_MOVES], u8 pp[MAX_BATTLE_MOVES]);

#endif // GUARD_ASCENSION_MOVES_H
