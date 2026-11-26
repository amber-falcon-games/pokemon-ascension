#include "global.h"
#include "battle_lab.h"
#include "config/ascension.h"
#include "test/test.h"

// Pokémon Ascension F007/F008: Battle Lab scenario data and team builder model.

static u32 PartySpecies(u32 trainer, u32 i)
{
    return GetMonData(&gParties[trainer][i], MON_DATA_SPECIES);
}

TEST("Battle Lab LAB-001 builds the documented parties")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetupScenarioParties();

    EXPECT_EQ(PartySpecies(B_TRAINER_PLAYER, 0), SPECIES_SWAMPERT);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1), MOVE_WATERFALL);
    EXPECT_EQ(PartySpecies(B_TRAINER_PLAYER, 1), SPECIES_NONE);

    EXPECT_EQ(PartySpecies(B_TRAINER_OPPONENT_A, 0), SPECIES_BLAZIKEN);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(PartySpecies(B_TRAINER_OPPONENT_A, 1), SPECIES_NONE);
}

TEST("Battle Lab team defaults equal LAB-001 and are not marked edited")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].species, SPECIES_SWAMPERT);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_OPPONENT][0].species, SPECIES_BLAZIKEN);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][1].species, SPECIES_NONE);
    EXPECT_EQ(BattleLab_ActiveCount(LAB_SIDE_PLAYER), 1);
    EXPECT_EQ(BattleLab_ActiveCount(LAB_SIDE_OPPONENT), 1);
    EXPECT(!BattleLab_IsEdited());
}

TEST("Battle Lab species stepping follows dex order, skips battle-only forms and wraps")
{
    u32 i, species;
    ASSUME(ASCENSION_BATTLE_LAB);

    EXPECT_EQ(BattleLab_StepSpecies(SPECIES_NONE, 1), SPECIES_BULBASAUR);
    EXPECT_EQ(BattleLab_StepSpecies(SPECIES_BULBASAUR, 1), SPECIES_IVYSAUR);
    EXPECT_EQ(BattleLab_StepSpecies(SPECIES_IVYSAUR, -1), SPECIES_BULBASAUR);
    // Wraps from the first to the last selectable species and back.
    species = BattleLab_StepSpecies(SPECIES_BULBASAUR, -1);
    EXPECT_NE(species, SPECIES_BULBASAUR);
    EXPECT_EQ(BattleLab_StepSpecies(species, 1), SPECIES_BULBASAUR);
    // Every stop is selectable: walk 60 steps from Venusaur (whose Mega must be skipped).
    species = SPECIES_VENUSAUR;
    for (i = 0; i < 60; i++)
    {
        species = BattleLab_StepSpecies(species, 1);
        EXPECT(BattleLab_IsSpeciesSelectable(species));
    }
#ifdef SPECIES_VENUSAUR_MEGA
    EXPECT(!BattleLab_IsSpeciesSelectable(SPECIES_VENUSAUR_MEGA));
#endif
    EXPECT(!BattleLab_IsSpeciesSelectable(SPECIES_NONE));
    EXPECT(!BattleLab_IsSpeciesSelectable(SPECIES_EGG));
}

TEST("Battle Lab letter jump picks the lowest dex number with that first letter")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT_EQ(BattleLab_FirstSpeciesForLetter(0), SPECIES_ARBOK);       // A: #24
    EXPECT_EQ(BattleLab_FirstSpeciesForLetter(1), SPECIES_BULBASAUR);   // B: #1
    EXPECT_EQ(BattleLab_FirstSpeciesForLetter(7), SPECIES_HAUNTER);     // H: #93
}

TEST("Battle Lab level is clamped to 1-100 and marks the slot edited")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetSlotLevel(LAB_SIDE_PLAYER, 0, 0);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].level, 1);
    BattleLab_SetSlotLevel(LAB_SIDE_PLAYER, 0, 250);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].level, MAX_LEVEL);
    EXPECT(BattleLab_IsEdited());
}

TEST("Battle Lab builds parties from active slots in slot order")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    // Player: Swampert (unchanged), Pikachu 12 (switched off), Gengar 40.
    BattleLab_SetSlotSpecies(LAB_SIDE_PLAYER, 1, SPECIES_PIKACHU);
    BattleLab_SetSlotLevel(LAB_SIDE_PLAYER, 1, 12);
    BattleLab_SetSlotActive(LAB_SIDE_PLAYER, 1, FALSE);
    BattleLab_SetSlotSpecies(LAB_SIDE_PLAYER, 2, SPECIES_GENGAR);
    BattleLab_SetSlotLevel(LAB_SIDE_PLAYER, 2, 40);
    // Opponent: Blaziken switched off, six-member team otherwise.
    BattleLab_SetSlotActive(LAB_SIDE_OPPONENT, 0, FALSE);
    BattleLab_SetSlotSpecies(LAB_SIDE_OPPONENT, 1, SPECIES_ONIX);
    BattleLab_SetSlotSpecies(LAB_SIDE_OPPONENT, 5, SPECIES_SNORLAX);
    BattleLab_SetupScenarioParties();

    EXPECT_EQ(PartySpecies(B_TRAINER_PLAYER, 0), SPECIES_SWAMPERT);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1), MOVE_WATERFALL);   // scenario moves kept
    EXPECT_EQ(PartySpecies(B_TRAINER_PLAYER, 1), SPECIES_GENGAR);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_LEVEL), 40);
    EXPECT_NE(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_MOVE1), MOVE_NONE);       // level-up moves
    EXPECT_EQ(PartySpecies(B_TRAINER_PLAYER, 2), SPECIES_NONE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_PLAYER], 2);

    EXPECT_EQ(PartySpecies(B_TRAINER_OPPONENT_A, 0), SPECIES_ONIX);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), LAB_DEFAULT_LEVEL);
    EXPECT_EQ(PartySpecies(B_TRAINER_OPPONENT_A, 1), SPECIES_SNORLAX);
    EXPECT_EQ(PartySpecies(B_TRAINER_OPPONENT_A, 2), SPECIES_NONE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_OPPONENT_A], 2);
}

TEST("Battle Lab reports a side with nobody in battle; reset and clear work")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_ClearSlot(LAB_SIDE_OPPONENT, 0);
    EXPECT_EQ(BattleLab_ActiveCount(LAB_SIDE_OPPONENT), 0);
    EXPECT(BattleLab_IsEdited());
    BattleLab_ResetSlot(LAB_SIDE_OPPONENT, 0);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_OPPONENT][0].species, SPECIES_BLAZIKEN);
    EXPECT_EQ(BattleLab_ActiveCount(LAB_SIDE_OPPONENT), 1);
    EXPECT(!BattleLab_IsEdited());
}
