#include "global.h"
#include "test/battle.h"
#include "battle_lab.h"

// Pokémon Ascension F019: a campaign battle with the overhaul on keeps the energy spent.
SINGLE_BATTLE_TEST("Energy: a campaign battle saves the energy your Pokémon spent")
{
    GIVEN {
        gAscensionCombatTestOverride = TRUE;
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Tackle!");
    } THEN {
        EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_ASC_FATIGUE), 0);
        EXPECT_LT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_ASC_FATIGUE), 30);
    }
}

SINGLE_BATTLE_TEST("Energy items (F019): a Max Elixir in a campaign battle restores energy, not PP")
{
    u32 fatigue = 60;   // the battle starts at 40% energy
    GIVEN {
        gAscensionCombatTestOverride = TRUE;
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_ASC_FATIGUE, &fatigue);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_MAX_ELIXIR, partyIndex: 0); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet's energy was restored!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_ASC_FATIGUE), 0);
    }
}
