#include "global.h"
#include "ascension_moves.h"
#include "battle_lab.h"
#include "move.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "data/ascension/talent_trees.h"

#if TESTING
EWRAM_DATA bool8 gAscensionMoveListTestOverride = FALSE;
#endif

u32 Ascension_GetTalentChoice(struct Pokemon *mon, u32 branch)
{
    return (GetMonData(mon, MON_DATA_TALENT_CHOICES) >> (2 * branch)) & 3;
}

void Ascension_SetTalentChoice(struct Pokemon *mon, u32 branch, u32 choice)
{
    u32 choices = GetMonData(mon, MON_DATA_TALENT_CHOICES);
    choices = (choices & ~(3u << (2 * branch))) | ((choice & 3) << (2 * branch));
    SetMonData(mon, MON_DATA_TALENT_CHOICES, &choices);
}

u32 Ascension_TalentBranchCount(u32 species)
{
    return species < NUM_SPECIES ? gTalentTrees[species].branchCount : 0;
}

u16 Ascension_TalentBranchMove(u32 species, u32 branch, u32 level, u32 choice)
{
    const struct TalentBranch *b;
    s32 i;
    if (branch >= Ascension_TalentBranchCount(species))
        return MOVE_NONE;
    b = &gTalentTrees[species].branches[branch];
    for (i = b->nodeCount - 1; i >= 0; i--)
        if (b->nodes[i].level <= level)
            return b->nodes[i].moves[choice % b->nodes[i].count];
    return MOVE_NONE;
}

// Builds the battle move list (and PP). With a talent tree, every branch contributes its current
// move (D027: no custom slots), and the saved 4 move slots are refreshed as a cache of the first
// four so menus and upstream code stay in step. Without one, F014's rule: slots 4+ get the most
// recent level-up moves not already known. Returns the number of moves.
u32 Ascension_FillExtraBattleMoves(struct Pokemon *mon, enum Move moves[MAX_BATTLE_MOVES], u8 pp[MAX_BATTLE_MOVES])
{
    u32 count = MAX_MON_MOVES, i, j, level, species;
    const struct LevelUpMove *learnset;
    s32 last;

    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
    {
        moves[i] = MOVE_NONE;
        pp[i] = 0;
    }
    if (!ASCENSION_MOVE_LIST)
        return MAX_MON_MOVES;

    species = GetMonData(mon, MON_DATA_SPECIES);
    level = GetMonData(mon, MON_DATA_LEVEL);
    // Lab battles keep the scenario's hand-set moves (plus F014 extras) so scenarios test what they set.
    if (Ascension_TalentBranchCount(species) > 0 && !BattleLab_IsLabBattle())
    {
        count = 0;
        for (i = 0; i < Ascension_TalentBranchCount(species) && count < MAX_BATTLE_MOVES; i++)
        {
            u16 move = Ascension_TalentBranchMove(species, i, level, Ascension_GetTalentChoice(mon, i));
            if (move != MOVE_NONE)
                moves[count++] = move;
        }
        for (i = 0; i < MAX_BATTLE_MOVES; i++)
        {
            if (i >= count)
                moves[i] = MOVE_NONE;
            pp[i] = moves[i] != MOVE_NONE ? GetMovePP(moves[i]) : 0;
        }
        for (i = 0; i < MAX_MON_MOVES; i++)   // keep the saved slots as a cache of the first four
        {
            u32 move = moves[i], movePp = pp[i];
            SetMonData(mon, MON_DATA_MOVE1 + i, &move);
            SetMonData(mon, MON_DATA_PP1 + i, &movePp);
        }
        return count;
    }

    learnset = GetSpeciesLevelUpLearnset(species);
    for (last = 0; learnset[last].move != LEVEL_UP_MOVE_END && learnset[last].level <= level; last++)
        ;
    // Most recent first: walk back from the highest level the Pokémon has reached.
    for (last = last - 1; last >= 0 && count < MAX_BATTLE_MOVES; last--)
    {
        enum Move move = learnset[last].move;
        bool32 known = (move == MOVE_NONE);
        for (j = 0; j < count && !known; j++)
            if (moves[j] == move)
                known = TRUE;
        if (known)
            continue;
        moves[count] = move;
        pp[count] = GetMovePP(move);
        count++;
    }
    return count;
}
