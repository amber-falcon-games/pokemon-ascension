#include "global.h"
#include "test/test.h"
#include "battle_lab.h"
#include "constants/battle.h"

// Pokémon Ascension F020 (D028): the queued follow-up's cancel rules.
static struct AscQueueCheck Base(void)
{
    struct AscQueueCheck c = {.samePokemon = TRUE, .moveUsable = TRUE, .targetAlive = TRUE, .sameTarget = TRUE,
                              .hpBefore = 100, .hpNow = 100, .maxHp = 100};
    return c;
}

TEST("Queue (F020): a queued follow-up still runs when nothing changed, or after a small hit")
{
    struct AscQueueCheck c = Base();
    EXPECT(BattleLab_QueueStillValid(&c));
    c.hpNow = 80;   // lost 20%
    EXPECT(BattleLab_QueueStillValid(&c));
    c.hpNow = 120;  // healed
    EXPECT(BattleLab_QueueStillValid(&c));
}

TEST("Queue (F020): cancelled by a 25% hit, a new status or confusion, Exhaustion, or the target fainting or leaving")
{
    struct AscQueueCheck c;
    c = Base(); c.hpNow = 75;                      EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.statusNow = STATUS1_POISON;      EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.statusBefore = c.statusNow = STATUS1_POISON;   EXPECT(BattleLab_QueueStillValid(&c));   // not new
    c = Base(); c.confusedNow = TRUE;              EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.exhausted = TRUE;                EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.targetAlive = FALSE;             EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.sameTarget = FALSE;              EXPECT(!BattleLab_QueueStillValid(&c));
    c = Base(); c.samePokemon = FALSE;             EXPECT(!BattleLab_QueueStillValid(&c));   // switched out
    c = Base(); c.moveUsable = FALSE;              EXPECT(!BattleLab_QueueStillValid(&c));   // disabled, taunted, too costly
    c = Base(); c.selfTarget = TRUE; c.targetAlive = FALSE; EXPECT(BattleLab_QueueStillValid(&c));
}
