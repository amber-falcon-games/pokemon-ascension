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
// Talent trees (F015, D027): generated ROM tables, see tools/ascension/talent_trees.py.
#define TALENT_MAX_CHOICES 4
struct TalentNode { u8 level; u8 count; u16 moves[TALENT_MAX_CHOICES]; };
struct TalentBranch { u8 nodeCount; const struct TalentNode *nodes; };
struct TalentTree { u8 branchCount; const struct TalentBranch *branches; };
extern const struct TalentTree gTalentTrees[];
extern const u16 *const gTalentGlobalChains[];

struct Pokemon;
struct BattlePokemon;
// The move a tree branch gives at a level (MOVE_NONE before its first node), with the choice made
// at branch points (index into the node's options).
u16 Ascension_TalentBranchMove(u32 species, u32 branch, u32 level, u32 choice);
u32 Ascension_TalentBranchCount(u32 species);
// The option chosen at a branch's branch points (0-3), saved with the Pokémon (MON_DATA_TALENT_CHOICES).
u32 Ascension_GetTalentChoice(struct Pokemon *mon, u32 branch);
void Ascension_SetTalentChoice(struct Pokemon *mon, u32 branch, u32 choice);
u32 Ascension_FillExtraBattleMoves(struct Pokemon *mon, enum Move moves[MAX_BATTLE_MOVES], u8 pp[MAX_BATTLE_MOVES]);

#endif // GUARD_ASCENSION_MOVES_H
