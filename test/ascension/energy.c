#include "global.h"
#include "test/test.h"
#include "ascension_moves.h"
#include "battle_lab.h"
#include "pokemon.h"
#include "constants/species.h"

// Pokémon Ascension F019: energy kept between battles (MON_DATA_ASC_FATIGUE).

TEST("Energy: fatigue is stored with the Pokémon and survives boxing; it is not a ribbon")
{
    struct Pokemon mon, back;
    u32 fatigue = 73;
    CreateMon(&mon, SPECIES_MUDKIP, 20, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ASC_FATIGUE), 0);      // new Pokémon are full
    SetMonData(&mon, MON_DATA_ASC_FATIGUE, &fatigue);
    BoxMonToMon(&mon.box, &back);
    EXPECT_EQ(GetMonData(&back, MON_DATA_ASC_FATIGUE), 73);
    gAscensionCombatTestOverride = TRUE;
    EXPECT_EQ(GetMonData(&back, MON_DATA_RIBBON_COUNT), 0);
    EXPECT_EQ(GetMonData(&back, MON_DATA_RIBBONS), 0);
    gAscensionCombatTestOverride = FALSE;
}

TEST("Energy: walking recovers 1% every 16 steps, up to the health cap; full heals restore it")
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u32 i, fatigue = 50, hp;
    ZeroPlayerPartyMons();
    CreateMon(mon, SPECIES_MUDKIP, 20, 0, OTID_STRUCT_PRESET(0));
    CalculateMonStats(mon);
    hp = GetMonData(mon, MON_DATA_MAX_HP);
    EXPECT_GT(hp, 0);
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_ASC_FATIGUE, &fatigue);
    gAscensionCombatTestOverride = TRUE;
    for (i = 0; i < 16 * 3; i++)
        BattleLab_OnPlayerStep();
    EXPECT_EQ(GetMonData(mon, MON_DATA_ASC_FATIGUE), 47);
    hp = GetMonData(mon, MON_DATA_MAX_HP) / 2;                // half health: energy caps at 75%
    SetMonData(mon, MON_DATA_HP, &hp);
    fatigue = 26;
    SetMonData(mon, MON_DATA_ASC_FATIGUE, &fatigue);
    for (i = 0; i < 16 * 5; i++)
        BattleLab_OnPlayerStep();
    EXPECT_EQ(GetMonData(mon, MON_DATA_ASC_FATIGUE), 25);
    HealPokemon(mon);
    EXPECT_EQ(GetMonData(mon, MON_DATA_ASC_FATIGUE), 0);
    gAscensionCombatTestOverride = FALSE;
    ZeroPlayerPartyMons();
}
