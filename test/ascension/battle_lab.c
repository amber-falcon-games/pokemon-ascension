#include "global.h"
#include "battle_lab.h"
#include "config/ascension.h"
#include "test/test.h"

// Pokémon Ascension F007: LAB-001 scenario data (docs/BATTLE_LAB_SCENARIOS.md).
TEST("Battle Lab LAB-001 builds the documented parties")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_SetupScenarioParties();

    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), SPECIES_SWAMPERT);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1), MOVE_WATERFALL);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_NONE);

    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), SPECIES_BLAZIKEN);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_SPECIES), SPECIES_NONE);
}
