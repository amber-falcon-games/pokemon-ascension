#include "global.h"
#include "ascension_moves.h"
#include "move.h"
#include "pokemon.h"
#include "constants/moves.h"

#if TESTING
EWRAM_DATA bool8 gAscensionMoveListTestOverride = FALSE;
#endif

// Fills moves[4..] (and their PP) and returns how many moves the battler has in total.
u32 Ascension_FillExtraBattleMoves(struct Pokemon *mon, enum Move moves[MAX_BATTLE_MOVES], u8 pp[MAX_BATTLE_MOVES])
{
    u32 count = MAX_MON_MOVES, i, j, level;
    const struct LevelUpMove *learnset;
    s32 last;

    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
    {
        moves[i] = MOVE_NONE;
        pp[i] = 0;
    }
    if (!ASCENSION_MOVE_LIST)
        return MAX_MON_MOVES;

    learnset = GetSpeciesLevelUpLearnset(GetMonData(mon, MON_DATA_SPECIES));
    level = GetMonData(mon, MON_DATA_LEVEL);
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
