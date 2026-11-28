#include "global.h"
#include "test/test.h"
#include "battle_lab.h"

// Pokémon Ascension F021: the AI's Boost rule.
static struct AscAiBoostCheck Base(void)
{
    struct AscAiBoostCheck c = {.damaging = TRUE, .energy = 200, .maxEnergy = 200, .cost = 20,
                                .damage = 50, .targetHp = 100, .nextAct = 2000, .nextActSpeed = 1600, .targetNextAct = 1500};
    return c;
}

TEST("AI Boost (F021): Power when x1.3 turns a hit into a knockout")
{
    struct AscAiBoostCheck c = Base();
    c.damage = 80;                      // 80 < 100, 104 >= 100
    EXPECT_EQ(BattleLab_AiBoostChoice(&c), LAB_BOOST_POWER);
    c.damage = 120;                     // already a knockout: no need
    c.nextAct = 1400;                   // and it acts first anyway
    EXPECT_EQ(BattleLab_AiBoostChoice(&c), LAB_BOOST_NONE);
}

TEST("AI Boost (F021): Speed when it lets the AI act before its target, and keeps a 50% reserve")
{
    struct AscAiBoostCheck c = Base();
    c.nextActSpeed = 1400;              // 2000 > 1500, 1400 <= 1500
    EXPECT_EQ(BattleLab_AiBoostChoice(&c), LAB_BOOST_SPEED);
    c.energy = 120;                     // 120 < 30 + 100: too little left after paying
    EXPECT_EQ(BattleLab_AiBoostChoice(&c), LAB_BOOST_NONE);
    c = Base(); c.damaging = FALSE; c.nextActSpeed = 1400;
    EXPECT_EQ(BattleLab_AiBoostChoice(&c), LAB_BOOST_NONE);   // status moves are not boosted
}
