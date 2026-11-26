#include "global.h"
#include "battle_lab.h"
#include "config/ascension.h"
#include "battle.h"
#include "constants/characters.h"
#include "random.h"
#include "test/test.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"

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

// ---------------------------------------------------------------- F009: details and rules

TEST("Battle Lab slot defaults copy the scenario's moves, IVs and item")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].moves[0], MOVE_WATERFALL);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].ivs, 31);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].hpPercent, 100);
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].status, LAB_STATUS_NONE);
}

TEST("Battle Lab builds a changed slot with every chosen detail")
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 maxHp;
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetSlotMove(LAB_SIDE_OPPONENT, 0, 0, MOVE_FLAMETHROWER);
    BattleLab_SetSlotMove(LAB_SIDE_OPPONENT, 0, 3, MOVE_NONE);
    BattleLab_SetSlotItem(LAB_SIDE_OPPONENT, 0, ITEM_LEFTOVERS);
    BattleLab_SetSlotAbility(LAB_SIDE_OPPONENT, 0, 2);
    BattleLab_SetSlotNature(LAB_SIDE_OPPONENT, 0, NATURE_ADAMANT);
    BattleLab_SetSlotIvs(LAB_SIDE_OPPONENT, 0, 0);
    BattleLab_SetSlotHpPercent(LAB_SIDE_OPPONENT, 0, 50);
    BattleLab_SetSlotStatus(LAB_SIDE_OPPONENT, 0, LAB_STATUS_BURN);
    BattleLab_SetupScenarioParties();

    EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIES), SPECIES_BLAZIKEN);
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE1), MOVE_FLAMETHROWER);
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE2), MOVE_SKY_UPPERCUT);     // other scenario moves kept
    EXPECT_EQ(GetMonData(mon, MON_DATA_MOVE4), MOVE_NONE);
    EXPECT_GT(GetMonData(mon, MON_DATA_PP1), 0);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(GetMonData(mon, MON_DATA_ABILITY_NUM), 2);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HIDDEN_NATURE), NATURE_ADAMANT);   // what stats and the summary use
    EXPECT_EQ(GetMonData(mon, MON_DATA_ATK_IV), 0);
    maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HP), maxHp / 2);
    EXPECT_EQ(GetMonData(mon, MON_DATA_STATUS), STATUS1_BURN);
}

TEST("Battle Lab choosing a new species refreshes its moves and ability")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetSlotSpecies(LAB_SIDE_PLAYER, 0, SPECIES_PIKACHU);
    EXPECT_NE(gBattleLabTeams[LAB_SIDE_PLAYER][0].moves[0], MOVE_WATERFALL);
    EXPECT(BattleLab_SpeciesCanLearn(SPECIES_PIKACHU, gBattleLabTeams[LAB_SIDE_PLAYER][0].moves[0]));
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].abilitySlot, 0);
    EXPECT(!BattleLab_SpeciesCanLearn(SPECIES_PIKACHU, MOVE_WATERFALL));
}

TEST("Battle Lab move and item lists skip unusable entries")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT_EQ(BattleLab_StepMove(MOVE_NONE, 1), MOVE_POUND);
    EXPECT_EQ(BattleLab_StepMove(MOVE_POUND, -1), MOVE_NONE);
    EXPECT(!BattleLab_IsMoveSelectable(MOVES_COUNT));   // Z-Moves and Max Moves
    EXPECT(BattleLab_IsItemSelectable(ITEM_LEFTOVERS));
    EXPECT(!BattleLab_IsItemSelectable(ITEM_MACH_BIKE));   // key item
    EXPECT(!BattleLab_IsItemSelectable(ITEM_NONE));
    EXPECT_EQ(BattleLab_FirstItemForLetter(11) != ITEM_NONE, TRUE);
}

TEST("Battle Lab rules: AI flags, weather and terrain, doubles validation")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_ResetRules();
    EXPECT(!BattleLab_IsEdited());
    EXPECT_EQ(BattleLab_OpponentAiFlags(), AI_FLAG_BASIC_TRAINER);

    gBattleLabRules.ai = LAB_AI_SMART;
    EXPECT_EQ(BattleLab_OpponentAiFlags(), AI_FLAG_SMART_TRAINER);
    gBattleLabRules.weather = LAB_WEATHER_RAIN;
    gBattleLabRules.terrain = LAB_TERRAIN_ELECTRIC;
    BattleLab_ApplyStartingStatuses();
    EXPECT(gStartingStatuses.weatherRain);
    EXPECT(gStartingStatuses.electricTerrain);
    EXPECT(!gStartingStatuses.weatherSun);
    EXPECT(BattleLab_IsEdited());

    EXPECT(BattleLab_ValidateStart() == NULL);
    gBattleLabRules.format = LAB_FORMAT_TRAINER_DOUBLE;
    EXPECT(BattleLab_ValidateStart() != NULL);          // 1 v 1 can't play doubles
    BattleLab_SetSlotSpecies(LAB_SIDE_PLAYER, 1, SPECIES_PIKACHU);
    BattleLab_SetSlotSpecies(LAB_SIDE_OPPONENT, 1, SPECIES_ONIX);
    EXPECT(BattleLab_ValidateStart() == NULL);
    BattleLab_ResetRules();
    memset(&gStartingStatuses, 0, sizeof(gStartingStatuses));
}

TEST("Battle Lab fixed seed repeats the RNG sequence")
{
    u32 a, b, c;
    ASSUME(ASCENSION_BATTLE_LAB);
    SeedRng(1234);
    a = Random32();
    b = Random32();
    SeedRng(1234);
    EXPECT_EQ(Random32(), a);
    c = Random32();
    EXPECT_EQ(c, b);
}

// ---------------------------------------------------------------- F010: battle loop

TEST("Battle Lab result text for each battle outcome")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_WON, FALSE, FALSE), LAB_RESULT_WON);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_LOST, FALSE, FALSE), LAB_RESULT_LOST);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_RAN, FALSE, FALSE), LAB_RESULT_RAN);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_CAUGHT, FALSE, FALSE), LAB_RESULT_CAUGHT);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_DREW, TRUE, FALSE), LAB_RESULT_QUIT);
    EXPECT_EQ(BattleLab_ResultFromOutcome(B_OUTCOME_DREW, FALSE, TRUE), LAB_RESULT_RESTARTED);
    EXPECT_EQ(BattleLab_ResultName(LAB_RESULT_WON)[0], CHAR_W);
}

TEST("Battle Lab restart and quit are refused outside Lab battles")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT(!BattleLab_RequestEnd(TRUE));
    EXPECT(!BattleLab_RequestEnd(FALSE));
}

TEST("Battle Lab fixed seed makes party building repeatable (personality, gender)")
{
    u32 first;
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetSlotSpecies(LAB_SIDE_OPPONENT, 0, SPECIES_NIDORINO);
    SeedRng(4321);
    BattleLab_SetupScenarioParties();
    first = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PERSONALITY);
    SeedRng(4321);
    BattleLab_SetupScenarioParties();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PERSONALITY), first);
}

TEST("Battle Lab wild battles keep only the opponent's first active Pokémon")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_ResetTeams();
    BattleLab_SetSlotSpecies(LAB_SIDE_OPPONENT, 1, SPECIES_ONIX);
    BattleLab_SetupScenarioParties();
    EXPECT_EQ(gPartiesCount[B_TRAINER_OPPONENT_A], 2);
    BattleLab_TrimToWildOpponent();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), SPECIES_BLAZIKEN);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT_EQ(gPartiesCount[B_TRAINER_OPPONENT_A], 1);
    EXPECT(!BattleLab_IsLabBattle());   // only set while a Lab battle runs
}

TEST("Battle Lab turn count shows turns played (counter is zero-based)")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT_EQ(BattleLab_TurnsPlayed(0), 1);
    EXPECT_EQ(BattleLab_TurnsPlayed(4), 5);
    EXPECT_EQ(BattleLab_TurnsPlayed(255), 255);
}

// ---------------------------------------------------------------- F011: scenarios

TEST("Battle Lab plain dev ROM has only the built-in scenario and does not auto-open")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    EXPECT_EQ(BattleLab_BlockScenarioCount(), 0);     // the block is all 0xFF until play.sh injects it
    EXPECT_EQ(BattleLab_ScenarioCount(), 1);
    EXPECT_EQ(BattleLab_AutoOpenScenario(), -1);
    EXPECT(!BattleLab_ShouldAutoOpen());
}

TEST("Battle Lab selecting the built-in scenario resets teams and rules to LAB-001")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_SelectScenario(0);
    BattleLab_SetSlotSpecies(LAB_SIDE_PLAYER, 0, SPECIES_PIKACHU);
    gBattleLabRules.weather = LAB_WEATHER_RAIN;
    EXPECT(BattleLab_IsEdited());
    EXPECT(BattleLab_SlotEdited(LAB_SIDE_PLAYER, 0));
    EXPECT(!BattleLab_SlotEdited(LAB_SIDE_OPPONENT, 0));
    EXPECT(BattleLab_SelectScenario(0));
    EXPECT_EQ(gBattleLabTeams[LAB_SIDE_PLAYER][0].species, SPECIES_SWAMPERT);
    EXPECT_EQ(gBattleLabRules.weather, LAB_WEATHER_NONE);
    EXPECT(!BattleLab_IsEdited());
    EXPECT(!BattleLab_SelectScenario(1));             // no injected scenarios in the test ROM
}

// ---------------------------------------------------------------- F012: determinism and diagnostics

TEST("Battle Lab 'use last seed' needs a battle first, then fixes that seed")
{
    ASSUME(ASCENSION_BATTLE_LAB);
    BattleLab_SelectScenario(0);
    if (!BattleLab_HasLastSeed())
        EXPECT(!BattleLab_UseLastSeed());
    BattleLab_LogTurnEnd();                 // outside a Lab battle: does nothing
    EXPECT(!gBattleLabRules.fixedSeed);
}

TEST("Battle Lab stamina energy costs (D023) and no timeline outside Lab battles")
{
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_TACKLE, 600), 36);        // power 40 -> 6%
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_HYDRO_PUMP, 600), 108);   // power 110 -> 18%
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_GROWL, 600), 30);         // status -> 5%
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_PROTECT, 600), 0);        // Protect is free
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_RECOVER, 600), 0);        // healing is free
    EXPECT_EQ(BattleLab_EnergyCost(MOVE_TACKLE, 5), 1);           // never below 1
    EXPECT(!BattleLab_StaminaMode());
    EXPECT(!BattleLab_TimelineSkips(0));
    EXPECT(!BattleLab_IsExhausted(0));
}
