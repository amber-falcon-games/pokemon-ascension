#include "global.h"
#include "test/test.h"
#include "ascension_moves.h"
#include "move.h"
#include "pokemon.h"
#include "constants/moves.h"
#include "constants/species.h"

// Pokémon Ascension F014: battle move list slots 4+ (until F015's move library).

TEST("Move list: slots 5-8 are the most recent level-up moves not already known")
{
    struct Pokemon mon;
    enum Move moves[MAX_BATTLE_MOVES] = {MOVE_THUNDERBOLT, MOVE_QUICK_ATTACK, MOVE_NONE, MOVE_NONE};
    u8 pp[MAX_BATTLE_MOVES] = {0};
    u32 count, i, j;

    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    gAscensionMoveListTestOverride = TRUE;
    moves[2] = MOVE_NONE; moves[3] = MOVE_NONE;
    count = Ascension_FillExtraBattleMoves(&mon, moves, pp);
    gAscensionMoveListTestOverride = FALSE;

    EXPECT_GT(count, MAX_MON_MOVES);
    EXPECT_LE(count, MAX_BATTLE_MOVES);
    for (i = MAX_MON_MOVES; i < count; i++)
    {
        EXPECT_NE(moves[i], MOVE_NONE);
        EXPECT_EQ(pp[i], GetMovePP(moves[i]));
        for (j = 0; j < i; j++)
            EXPECT_NE(moves[i], moves[j]);   // no duplicates
    }
}

TEST("Move list: off means slots 5-8 stay empty")
{
    struct Pokemon mon;
    enum Move moves[MAX_BATTLE_MOVES] = {MOVE_TACKLE};
    u8 pp[MAX_BATTLE_MOVES] = {0};
    u32 i;

    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
        moves[i] = MOVE_SURF;   // garbage that must be cleared
    EXPECT_EQ(Ascension_FillExtraBattleMoves(&mon, moves, pp), MAX_MON_MOVES);
    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
        EXPECT_EQ(moves[i], MOVE_NONE);
}
