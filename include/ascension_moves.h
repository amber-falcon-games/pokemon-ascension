#ifndef GUARD_ASCENSION_MOVES_H
#define GUARD_ASCENSION_MOVES_H

#include "global.h"
#include "config/ascension.h"

// Pokémon Ascension F014: the battle move list (MAX_BATTLE_MOVES). Slots 0-3 are the saved moves;
// with ASCENSION_MOVE_LIST on, slots 4+ are filled when the Pokémon enters battle (until F015's
// move library: the species' most recent level-up moves it doesn't already know).
#if TESTING
extern bool8 gAscensionMoveListTestOverride;
extern bool8 gAscensionCombatTestOverride;   // ASCENSION_COMBAT in tests (battle_lab.c)
#endif
// Talent trees (F015, D027): generated ROM tables, see tools/ascension/talent_trees.py.
#define TALENT_MAX_CHOICES 4
struct TalentNode { u8 level; u8 count; u16 moves[TALENT_MAX_CHOICES]; };
struct TalentBranch { u8 nodeCount; const struct TalentNode *nodes; };
struct TalentTree { u8 branchCount; const struct TalentBranch *branches; };
extern const struct TalentTree gTalentTrees[];
struct TalentGlobalMove { u16 move; u8 level; };   // level: the move's typical (median) learn level
extern const struct TalentGlobalMove *const gTalentGlobalChains[];

// Teaching a move (TM, tutor, egg) per D027.
enum TalentTeachResult
{
    TALENT_TEACH_JUMPED,        // in its own tree, ahead: the branch jumps to it
    TALENT_TEACH_JOINED,        // not in its own tree: it joins the global chain at that move
    TALENT_TEACH_ALREADY_KNOWN, // it is the branch's current move
    TALENT_TEACH_GROWN_PAST,    // in its own tree but already grown past it: refused
    TALENT_TEACH_NO_ROOM,       // would join a chain but has no free join slot
    TALENT_TEACH_CANNOT,        // move is in no tree or chain
    TALENT_TEACH_NEEDS_SLOT,    // would join a chain but the list is full: ask which move it replaces
};
#define TALENT_MAX_JOINS 2
#define TALENT_SLOT_ASK 0xFF
// replaceSlot: when joining with a full list, the source (from BuildTalentList) of the move the
// joined chain replaces; TALENT_SLOT_ASK changes nothing and returns TALENT_TEACH_NEEDS_SLOT.
enum TalentTeachResult Ascension_TeachMove(struct Pokemon *mon, u16 move, u32 replaceSlot);
// The battle move list. sources (optional, MAX_BATTLE_MOVES): where each move comes from:
// 0-7 a tree branch (or a join replacing it), 8+k joined chain k; TALENT_SLOT_ASK for empty places.
u32 Ascension_BuildTalentList(struct Pokemon *mon, u16 moves[MAX_BATTLE_MOVES], u8 *sources);
void Ascension_RefreshMoveCache(struct Pokemon *mon);
// TRUE when this Pokémon's moves come from its talent tree (move library on, has a tree, not a Lab battle).
bool32 Ascension_UsesTalentTree(struct Pokemon *mon);
// Call right after an evolution sets the new species: carries TM jumps and choices over by move.
void Ascension_OnEvolve(struct Pokemon *mon, u32 oldSpecies);
// F016: "Talent branch 2.\nGrows into X at Lv. 39." (or a branch point / TM path / "Fully grown.").
// FALSE (empty text) when the move is not from this Pokémon's talent tree.
bool32 Ascension_DescribeTalentMove(struct Pokemon *mon, u16 move, u8 *dest);
// Level-up growth (spec item 6). Start with *iter = 0 and call until FALSE: each TRUE is one branch
// (0-7) or joined chain (8+k) whose move changes on reaching `level`. count > 1 is a branch point:
// the player picks one of options[] (Ascension_SetTalentChoice) and its moves follow that pick.
struct TalentGrowth { u8 source; u8 count; u16 from; u16 options[TALENT_MAX_CHOICES]; };
bool32 Ascension_NextTalentGrowth(struct Pokemon *mon, u32 level, u32 *iter, struct TalentGrowth *out);

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
