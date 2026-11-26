@ Pokémon Ascension dev checkpoints (F007b, decision D019). Dev builds only: reached from the
@ debug menu (R + Start → Checkpoints…), which is compiled out of release builds.
@ Each checkpoint sets the story state a normal playthrough has at that point. Every block names
@ the upstream map script it mirrors, so it can be re-checked when upstream changes.

@ ---------------------------------------------------------------- CP-START
@ Intro done (truck, Mom, clock, Petalburg Gym broadcast, rival's house visited), before Route 101.
@ You stand at Littleroot's north exit; walk north onto Route 101 to start Birch's rescue.
Ascension_Checkpoint_Start::
	lockall
	call Ascension_Checkpoint_IntroDone
	warp MAP_LITTLEROOT_TOWN, 11, 3
	waitstate
	end

@ ---------------------------------------------------------------- CP-BALLS
@ Starter chosen (Mudkip, as in D019), Birch rescued, rival beaten on Route 103, Pokédex and five
@ Poké Balls received: catching is unlocked. You stand outside Birch's lab; leaving town plays
@ Mom's Running Shoes scene as in a normal run.
Ascension_Checkpoint_PokeBalls::
	lockall
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	givemon SPECIES_MUDKIP, 7
	warp MAP_LITTLEROOT_TOWN, 7, 17
	waitstate
	end

@ CP-GYM1: outside Rustboro Gym, before Roxanne (Nosepass Lv15). Team of 2, ace Lv15.
Ascension_Checkpoint_Gym1::
	lockall
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	warp MAP_RUSTBORO_CITY, 27, 20
	waitstate
	end

@ ---------------------------------------------------------------- shared steps
Ascension_Checkpoint_IntroDone::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, Ascension_Checkpoint_IntroMale
	call_if_eq VAR_RESULT, FEMALE, Ascension_Checkpoint_IntroFemale
	@ LittlerootTown: Mom shows you into the house
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setflag FLAG_VISITED_LITTLEROOT_TOWN
	@ players_house.inc: clock set, movers' Vigoroth gone, Petalburg Gym broadcast watched
	setflag FLAG_SET_WALL_CLOCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2
	setflag FLAG_SYS_TV_HOME
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
	@ Rival's house: met rival's mom, met rival upstairs
	setflag FLAG_MET_RIVAL_MOM
	setvar VAR_LITTLEROOT_RIVAL_STATE, 3
	setvar VAR_LITTLEROOT_TOWN_STATE, 1
	return

@ InsideOfTruck_EventScript_SetIntroFlagsMale + LittlerootTown truck + MaysHouse rival visit
Ascension_Checkpoint_IntroMale::
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MAY
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM
	return

@ InsideOfTruck_EventScript_SetIntroFlagsFemale + LittlerootTown truck + BrendansHouse rival visit
Ascension_Checkpoint_IntroFemale::
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_BRENDAN
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	return

Ascension_Checkpoint_StarterToPokeBalls::
	@ Route101: Birch rescued with the starter; rival leaves the bedroom
	setflag FLAG_SYS_POKEMON_GET
	setflag FLAG_RESCUED_BIRCH
	setflag FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	setflag FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG
	setvar VAR_ROUTE101_STATE, 3
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	setvar VAR_STARTER_MON, 2
	@ ProfessorBirchsLab: agreed to go and see the rival
	clearflag FLAG_HIDE_ROUTE_101_BOY
	@ Route103: rival battle won; OldaleTown visited on the way
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_RIVAL
	setflag FLAG_DEFEATED_RIVAL_ROUTE103
	setvar VAR_OLDALE_RIVAL_STATE, 1
	clearflag FLAG_HIDE_OLDALE_TOWN_RIVAL
	setflag FLAG_VISITED_OLDALE_TOWN
	@ ProfessorBirchsLab_EventScript_GivePokedex: Pokédex and five Poké Balls
	setflag FLAG_SYS_POKEDEX_GET
	special SetUnlockedPokedexFlags
	setflag FLAG_RECEIVED_POKEDEX_FROM_BIRCH
	setvar VAR_CABLE_CLUB_TUTORIAL_STATE, 1
	additem ITEM_POKE_BALL, 5
	setvar VAR_BIRCH_LAB_STATE, 5
	setflag FLAG_ADVENTURE_STARTED
	setvar VAR_OLDALE_TOWN_STATE, 1
	setvar VAR_LITTLEROOT_RIVAL_STATE, 4
	setvar VAR_LITTLEROOT_TOWN_STATE, 3
	return

@ Oldale rival sent on, Wally's catching tutorial in Petalburg (PetalburgCity_Gym,
@ PetalburgCity), Team Aqua in Petalburg Woods, arrival in Rustboro.
Ascension_Checkpoint_PokeBallsToGym1::
	setvar VAR_OLDALE_RIVAL_STATE, 2
	setflag FLAG_HIDE_OLDALE_TOWN_RIVAL
	setflag FLAG_RECEIVED_POTION_OLDALE
	setflag FLAG_VISITED_PETALBURG_CITY
	setflag FLAG_HIDE_PETALBURG_CITY_WALLYS_MOM
	setflag FLAG_HIDE_PETALBURG_CITY_WALLY
	setflag FLAG_HIDE_PETALBURG_GYM_WALLY
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_RIVAL
	special InitBirchState
	setvar VAR_PETALBURG_CITY_STATE, 3
	setvar VAR_PETALBURG_GYM_STATE, 2
	setvar VAR_PETALBURG_WOODS_STATE, 1
	setflag FLAG_HIDE_PETALBURG_WOODS_AQUA_GRUNT
	setflag FLAG_HIDE_PETALBURG_WOODS_DEVON_EMPLOYEE
	settrainerflag TRAINER_GRUNT_PETALBURG_WOODS
	setflag FLAG_VISITED_RUSTBORO_CITY
	@ Team: Mudkip ace at Roxanne's ace level, plus a Route 104 catch two levels below.
	givemon SPECIES_MUDKIP, 15
	givemon SPECIES_TAILLOW, 13
	additem ITEM_GREAT_BALL, 1
	additem ITEM_POTION, 3
	additem ITEM_ANTIDOTE, 1
	addmoney 2400
	return
