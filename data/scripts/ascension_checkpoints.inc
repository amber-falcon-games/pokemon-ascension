@ Pokémon Ascension dev checkpoints (F007b, decision D019). Dev builds only: reached from the
@ debug menu (R + Start → Checkpoints…), which is compiled out of release builds.
@ Each checkpoint sets the story state a normal playthrough has at that point. Every block names
@ the upstream map script it mirrors, so it can be re-checked when upstream changes.
@
@ Layout: Ascension_Checkpoint_<CP> (menu entry) = Ascension_CPState_<CP> (state only, used by
@ test/ascension/checkpoints.c) + warp. A state = the story steps in order + that checkpoint's
@ team (Ascension_CPTeam_<CP>), so teams never stack. The debug action resets the game with
@ NewGameInitData first, so a checkpoint replaces the current state.
@ Teams (D019): Mudkip line, Hoenn catches available by then, ace at the leader's ace level,
@ others 1–3 below. Edit the CPTeam blocks to taste.

@ CP-START: intro done, before Route 101. Littleroot's north exit; walk north for Birch's rescue.
Ascension_Checkpoint_Start::
	lockall
	call Ascension_CPState_Start
	warp MAP_LITTLEROOT_TOWN, 11, 3
	waitstate
	end

Ascension_CPState_Start::
	call Ascension_Checkpoint_IntroDone
	return

@ CP-BALLS: Mudkip chosen, rival beaten on Route 103, Pokédex and 5 Poké Balls. Outside Birch's lab.
Ascension_Checkpoint_PokeBalls::
	lockall
	call Ascension_CPState_PokeBalls
	warp MAP_LITTLEROOT_TOWN, 7, 17
	waitstate
	end

Ascension_CPState_PokeBalls::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_CPTeam_PokeBalls
	return

@ CP-GYM1: outside Rustboro Gym, before Roxanne (Nosepass 15).
Ascension_Checkpoint_Gym1::
	lockall
	call Ascension_CPState_Gym1
	warp MAP_RUSTBORO_CITY, 27, 20
	waitstate
	end

Ascension_CPState_Gym1::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_CPTeam_Gym1
	return

@ CP-GYM2: outside Dewford Gym, before Brawly (Makuhita 19).
Ascension_Checkpoint_Gym2::
	lockall
	call Ascension_CPState_Gym2
	warp MAP_DEWFORD_TOWN, 8, 18
	waitstate
	end

Ascension_CPState_Gym2::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_CPTeam_Gym2
	return

@ CP-GYM3: outside Mauville Gym, before Wattson (Manectric 24).
Ascension_Checkpoint_Gym3::
	lockall
	call Ascension_CPState_Gym3
	warp MAP_MAUVILLE_CITY, 8, 6
	waitstate
	end

Ascension_CPState_Gym3::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_CPTeam_Gym3
	return

@ CP-GYM4: outside Lavaridge Gym, before Flannery (Torkoal 29).
Ascension_Checkpoint_Gym4::
	lockall
	call Ascension_CPState_Gym4
	warp MAP_LAVARIDGE_TOWN, 5, 16
	waitstate
	end

Ascension_CPState_Gym4::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_Story_Gym3ToGym4
	call Ascension_CPTeam_Gym4
	return

@ CP-GYM5: outside Petalburg Gym, before Norman (Slaking 31).
Ascension_Checkpoint_Gym5::
	lockall
	call Ascension_CPState_Gym5
	warp MAP_PETALBURG_CITY, 15, 9
	waitstate
	end

Ascension_CPState_Gym5::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_Story_Gym3ToGym4
	call Ascension_Story_Gym4ToGym5
	call Ascension_CPTeam_Gym5
	return

@ CP-GYM6: outside Fortree Gym, before Winona (Altaria 33).
Ascension_Checkpoint_Gym6::
	lockall
	call Ascension_CPState_Gym6
	warp MAP_FORTREE_CITY, 22, 12
	waitstate
	end

Ascension_CPState_Gym6::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_Story_Gym3ToGym4
	call Ascension_Story_Gym4ToGym5
	call Ascension_Story_Gym5ToGym6
	call Ascension_CPTeam_Gym6
	return

@ CP-GYM7: outside Mossdeep Gym, before Tate & Liza (Lunatone/Solrock 42).
Ascension_Checkpoint_Gym7::
	lockall
	call Ascension_CPState_Gym7
	warp MAP_MOSSDEEP_CITY, 38, 10
	waitstate
	end

Ascension_CPState_Gym7::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_Story_Gym3ToGym4
	call Ascension_Story_Gym4ToGym5
	call Ascension_Story_Gym5ToGym6
	call Ascension_Story_Gym6ToGym7
	call Ascension_CPTeam_Gym7
	return

@ CP-GYM8: outside Sootopolis Gym, before Juan (Kingdra 46).
Ascension_Checkpoint_Gym8::
	lockall
	call Ascension_CPState_Gym8
	warp MAP_SOOTOPOLIS_CITY, 31, 33
	waitstate
	end

Ascension_CPState_Gym8::
	call Ascension_Checkpoint_IntroDone
	call Ascension_Checkpoint_StarterToPokeBalls
	call Ascension_Checkpoint_PokeBallsToGym1
	call Ascension_Story_Gym1ToGym2
	call Ascension_Story_Gym2ToGym3
	call Ascension_Story_Gym3ToGym4
	call Ascension_Story_Gym4ToGym5
	call Ascension_Story_Gym5ToGym6
	call Ascension_Story_Gym6ToGym7
	call Ascension_Story_Gym7ToGym8
	call Ascension_CPTeam_Gym8
	return

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
	@ Mom's Running Shoes scene on leaving Littleroot (LittlerootTown_EventScript_SetReceivedRunningShoes).
	setflag FLAG_RECEIVED_RUNNING_SHOES
	setflag FLAG_SYS_B_DASH
	setvar VAR_LITTLEROOT_TOWN_STATE, 4
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
	additem ITEM_GREAT_BALL, 1
	return

@ RustboroCity_Gym (Roxanne), RustboroCity stolen-goods scene, RusturfTunnel grunt, Devon Corp 3F
@ (PokéNav, Letter), Route 104 rival, Route104_MrBrineysHouse + Route104 sail to Dewford.
Ascension_Story_Gym1ToGym2::
	setflag FLAG_DEFEATED_RUSTBORO_GYM
	setflag FLAG_BADGE01_GET
	settrainerflag TRAINER_ROXANNE_1
	setflag FLAG_RECEIVED_TM_ROCK_TOMB
	additem ITEM_TM_ROCK_TOMB
	setflag FLAG_RECEIVED_HM_CUT
	additem ITEM_HM_CUT
	addvar VAR_PETALBURG_GYM_STATE, 1
	setflag FLAG_RETURNED_DEVON_GOODS
	setflag FLAG_RECOVERED_DEVON_GOODS
	setflag FLAG_HIDE_RUSTBORO_CITY_DEVON_EMPLOYEE_1
	setflag FLAG_HIDE_RUSTBORO_CITY_AQUA_GRUNT
	settrainerflag TRAINER_GRUNT_RUSTURF_TUNNEL
	setvar VAR_RUSTURF_TUNNEL_STATE, 3
	setflag FLAG_HIDE_RUSTURF_TUNNEL_AQUA_GRUNT
	setflag FLAG_HIDE_RUSTURF_TUNNEL_PEEKO
	setflag FLAG_HIDE_ROUTE_116_MR_BRINEY
	setvar VAR_ROUTE116_STATE, 2
	additem ITEM_DEVON_PARTS
	setflag FLAG_SYS_POKENAV_GET
	setflag FLAG_RECEIVED_POKENAV
	setflag FLAG_HIDE_RUSTBORO_CITY_POKEMON_SCHOOL_SCOTT
	setflag FLAG_HIDE_ROUTE_116_WANDAS_BOYFRIEND
	clearflag FLAG_HIDE_RUSTURF_TUNNEL_WANDAS_BOYFRIEND
	clearflag FLAG_HIDE_RUSTURF_TUNNEL_WANDA
	setvar VAR_DEVON_CORP_3F_STATE, 1
	additem ITEM_LETTER
	setflag FLAG_HIDE_RUSTBORO_CITY_SCIENTIST
	setflag FLAG_HAS_MATCH_CALL
	setflag FLAG_ADDED_MATCH_CALL_TO_POKENAV
	setflag FLAG_MET_RIVAL_RUSTBORO
	setflag FLAG_DEFEATED_RIVAL_ROUTE_104
	setflag FLAG_HIDE_RUSTBORO_CITY_RIVAL
	setflag FLAG_HIDE_ROUTE_104_RIVAL
	setvar VAR_RUSTBORO_CITY_STATE, 8
	setvar VAR_ROUTE104_STATE, 2
	@ Briney sailed you to Dewford and waits there
	setflag FLAG_MR_BRINEY_SAILING_INTRO
	setvar VAR_BRINEY_HOUSE_STATE, 1
	setflag FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY
	setflag FLAG_HIDE_BRINEYS_HOUSE_PEEKO
	setflag FLAG_HIDE_ROUTE_104_MR_BRINEY
	setflag FLAG_HIDE_ROUTE_104_MR_BRINEY_BOAT
	clearflag FLAG_HIDE_MR_BRINEY_DEWFORD_TOWN
	clearflag FLAG_HIDE_MR_BRINEY_BOAT_DEWFORD_TOWN
	setvar VAR_BRINEY_LOCATION, 2
	setvar VAR_BOARD_BRINEY_BOAT_STATE, 0
	setflag FLAG_VISITED_DEWFORD_TOWN
	return

@ DewfordTown_Gym (Brawly), GraniteCave_StevensRoom (Letter, Flash), DewfordTown sail to Slateport,
@ OceanicMuseum_2F (Devon Goods to Stern), Route110 rival, MauvilleCity Wally battle.
Ascension_Story_Gym2ToGym3::
	setflag FLAG_DEFEATED_DEWFORD_GYM
	setflag FLAG_BADGE02_GET
	settrainerflag TRAINER_BRAWLY_1
	setflag FLAG_RECEIVED_TM_BULK_UP
	additem ITEM_TM_BULK_UP
	addvar VAR_PETALBURG_GYM_STATE, 1
	setflag FLAG_DELIVERED_STEVEN_LETTER
	removeitem ITEM_LETTER
	setflag FLAG_REGISTERED_STEVEN_POKENAV
	additem ITEM_TM_STEEL_WING
	setflag FLAG_RECEIVED_HM_FLASH
	additem ITEM_HM_FLASH
	@ Briney is now at Route 109 (Slateport side)
	setflag FLAG_HIDE_MR_BRINEY_DEWFORD_TOWN
	setflag FLAG_HIDE_MR_BRINEY_BOAT_DEWFORD_TOWN
	clearflag FLAG_HIDE_ROUTE_109_MR_BRINEY
	clearflag FLAG_HIDE_ROUTE_109_MR_BRINEY_BOAT
	setvar VAR_BRINEY_LOCATION, 3
	setflag FLAG_VISITED_SLATEPORT_CITY
	setvar VAR_SLATEPORT_MUSEUM_1F_STATE, 1
	settrainerflag TRAINER_GRUNT_MUSEUM_1
	settrainerflag TRAINER_GRUNT_MUSEUM_2
	setflag FLAG_HIDE_SLATEPORT_CITY_OCEANIC_MUSEUM_AQUA_GRUNTS
	removeitem ITEM_DEVON_PARTS
	setflag FLAG_HIDE_ROUTE_110_TEAM_AQUA
	setvar VAR_REGISTER_BIRCH_STATE, 1
	setflag FLAG_DELIVERED_DEVON_GOODS
	clearflag FLAG_HIDE_ROUTE_116_DEVON_EMPLOYEE
	setflag FLAG_HIDE_RUSTBORO_CITY_DEVON_CORP_3F_EMPLOYEE
	setvar VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE, 1
	setvar VAR_ROUTE110_STATE, 1
	setflag FLAG_HIDE_ROUTE_110_RIVAL
	setflag FLAG_HIDE_ROUTE_110_RIVAL_ON_BIKE
	setflag FLAG_VISITED_MAUVILLE_CITY
	settrainerflag TRAINER_WALLY_MAUVILLE
	setflag FLAG_DEFEATED_WALLY_MAUVILLE
	setflag FLAG_HIDE_MAUVILLE_CITY_WALLY
	setflag FLAG_HIDE_MAUVILLE_CITY_WALLYS_UNCLE
	clearflag FLAG_HIDE_VERDANTURF_TOWN_WANDAS_HOUSE_WALLY
	clearflag FLAG_HIDE_VERDANTURF_TOWN_WANDAS_HOUSE_WALLYS_UNCLE
	@ MauvilleCity_BikeShop: Mach Bike
	setflag FLAG_RECEIVED_BIKE
	additem ITEM_MACH_BIKE
	return

@ MauvilleCity_Gym (Wattson), Rock Smash (Mauville), MeteorFalls_1F_1R (Magma takes the
@ Meteorite; Route 112 cable car opens), MtChimney (Maxie), JaggedPass down to Lavaridge.
Ascension_Story_Gym3ToGym4::
	setflag FLAG_DEFEATED_MAUVILLE_GYM
	setflag FLAG_BADGE03_GET
	settrainerflag TRAINER_WATTSON_1
	setflag FLAG_RECEIVED_TM_SHOCK_WAVE
	additem ITEM_TM_SHOCK_WAVE
	addvar VAR_PETALBURG_GYM_STATE, 1
	setvar VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE, 3
	clearflag FLAG_HIDE_VERDANTURF_TOWN_SCOTT
	setflag FLAG_RECEIVED_HM_ROCK_SMASH
	additem ITEM_HM_ROCK_SMASH
	setflag FLAG_RECEIVED_HM_STRENGTH
	additem ITEM_HM_STRENGTH
	setflag FLAG_VISITED_FALLARBOR_TOWN
	setflag FLAG_HIDE_ROUTE_112_TEAM_MAGMA
	setflag FLAG_MET_ARCHIE_METEOR_FALLS
	setflag FLAG_HIDE_FALLARBOR_TOWN_BATTLE_TENT_SCOTT
	setvar VAR_METEOR_FALLS_STATE, 1
	setflag FLAG_MET_PROF_COZMO
	settrainerflag TRAINER_TABITHA_MT_CHIMNEY
	settrainerflag TRAINER_MAXIE_MT_CHIMNEY
	setflag FLAG_EVIL_LEADER_PLEASE_STOP
	setflag FLAG_HIDE_MT_CHIMNEY_TEAM_MAGMA
	setflag FLAG_HIDE_MT_CHIMNEY_TEAM_AQUA
	setflag FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY
	clearflag FLAG_HIDE_FALLARBOR_HOUSE_PROF_COZMO
	setflag FLAG_HIDE_METEOR_FALLS_1F_1R_COZMO
	clearflag FLAG_HIDE_MT_CHIMNEY_LAVA_COOKIE_LADY
	setflag FLAG_RECEIVED_METEORITE
	additem ITEM_METEORITE
	setflag FLAG_VISITED_LAVARIDGE_TOWN
	return

@ LavaridgeTown_Gym_1F (Flannery), LavaridgeTown rival gives the Go-Goggles; back to Petalburg,
@ where the Gym opens for Norman (Common_EventScript_ReadyPetalburgGymForBattle).
Ascension_Story_Gym4ToGym5::
	setflag FLAG_DEFEATED_LAVARIDGE_GYM
	setflag FLAG_BADGE04_GET
	settrainerflag TRAINER_FLANNERY_1
	setflag FLAG_RECEIVED_TM_OVERHEAT
	additem ITEM_TM_OVERHEAT
	addvar VAR_PETALBURG_GYM_STATE, 1
	setflag FLAG_HIDE_VERDANTURF_TOWN_WANDAS_HOUSE_WALLY
	setflag FLAG_RECEIVED_GO_GOGGLES
	additem ITEM_GO_GOGGLES
	setvar VAR_LAVARIDGE_TOWN_STATE, 2
	setflag FLAG_HIDE_LAVARIDGE_TOWN_RIVAL
	setflag FLAG_HIDE_LAVARIDGE_TOWN_RIVAL_ON_BIKE
	clearflag FLAG_HIDE_PETALBURG_GYM_GREETER
	setflag FLAG_PETALBURG_MART_EXPANDED_ITEMS
	return

@ PetalburgCity_Gym (Norman), PetalburgCity_WallysHouse (Surf), Route118 Steven,
@ Route119_WeatherInstitute_2F (Shelly), Route119 rival (Fly), Route120 Steven (Devon Scope),
@ FortreeCity Kecleon moved from the Gym.
Ascension_Story_Gym5ToGym6::
	setflag FLAG_DEFEATED_PETALBURG_GYM
	setflag FLAG_BADGE05_GET
	settrainerflag TRAINER_NORMAN_1
	setvar VAR_PETALBURG_GYM_STATE, 7
	setflag FLAG_RECEIVED_TM_FACADE
	additem ITEM_TM_FACADE
	setflag FLAG_HIDE_MAUVILLE_GYM_WATTSON
	clearflag FLAG_HIDE_MAUVILLE_CITY_WATTSON
	clearflag FLAG_HIDE_DEWFORD_HALL_SLUDGE_BOMB_MAN
	setflag FLAG_RECEIVED_HM_SURF
	additem ITEM_HM_SURF
	setvar VAR_PETALBURG_CITY_STATE, 5
	setflag FLAG_HIDE_PETALBURG_CITY_WALLYS_DAD
	setvar VAR_ROUTE118_STATE, 1
	setflag FLAG_HIDE_ROUTE_118_STEVEN
	settrainerflag TRAINER_SHELLY_WEATHER_INSTITUTE
	setflag FLAG_HIDE_ROUTE_119_TEAM_AQUA
	setflag FLAG_HIDE_WEATHER_INSTITUTE_2F_AQUA_GRUNT_M
	setflag FLAG_HIDE_WEATHER_INSTITUTE_2F_WORKERS
	clearflag FLAG_HIDE_WEATHER_INSTITUTE_1F_WORKERS
	setvar VAR_WEATHER_INSTITUTE_STATE, 2
	setflag FLAG_RECEIVED_HM_FLY
	additem ITEM_HM_FLY
	setvar VAR_ROUTE119_STATE, 1
	setflag FLAG_HIDE_ROUTE_119_RIVAL
	setflag FLAG_HIDE_ROUTE_119_RIVAL_ON_BIKE
	setflag FLAG_VISITED_FORTREE_CITY
	setflag FLAG_RECEIVED_DEVON_SCOPE
	additem ITEM_DEVON_SCOPE
	setflag FLAG_HIDE_ROUTE_120_STEVEN
	setflag FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE
	setflag FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE_SHADOW
	setflag FLAG_HIDE_FORTREE_CITY_KECLEON
	setflag FLAG_KECLEON_FLED_FORTREE
	return

@ FortreeCity_Gym (Winona), LilycoveCity rival, MtPyre_Summit (orbs taken, Magma Emblem),
@ JaggedPass + MagmaHideout_4F (Maxie), SlateportCity_Harbor (Aqua takes the submarine),
@ AquaHideout_B2F (Lilycove grunts leave: Route 124 opens), Mossdeep.
Ascension_Story_Gym6ToGym7::
	setflag FLAG_DEFEATED_FORTREE_GYM
	setflag FLAG_BADGE06_GET
	settrainerflag TRAINER_WINONA_1
	setflag FLAG_RECEIVED_TM_AERIAL_ACE
	additem ITEM_TM_AERIAL_ACE
	setflag FLAG_VISITED_LILYCOVE_CITY
	setflag FLAG_MET_RIVAL_LILYCOVE
	setvar VAR_MT_PYRE_STATE, 1
	setflag FLAG_HIDE_MT_PYRE_SUMMIT_ARCHIE
	setflag FLAG_HIDE_MT_PYRE_SUMMIT_TEAM_AQUA
	setflag FLAG_RECEIVED_RED_OR_BLUE_ORB
	setflag FLAG_HIDE_JAGGED_PASS_MAGMA_GUARD
	additem ITEM_MAGMA_EMBLEM
	setvar VAR_JAGGED_PASS_STATE, 2
	setflag FLAG_BEAT_MAGMA_GRUNT_JAGGED_PASS
	settrainerflag TRAINER_GRUNT_JAGGED_PASS
	settrainerflag TRAINER_TABITHA_MAGMA_HIDEOUT
	settrainerflag TRAINER_MAXIE_MAGMA_HIDEOUT
	setflag FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT
	setflag FLAG_HIDE_MAGMA_HIDEOUT_GRUNTS
	@ Slateport: Stern's harbor scene done
	setvar VAR_SLATEPORT_CITY_STATE, 2
	setflag FLAG_HIDE_SLATEPORT_CITY_CAPTAIN_STERN
	setvar VAR_SLATEPORT_HARBOR_STATE, 2
	setflag FLAG_MET_TEAM_AQUA_HARBOR
	setflag FLAG_HIDE_LILYCOVE_MOTEL_SCOTT
	setflag FLAG_HIDE_SLATEPORT_CITY_HARBOR_AQUA_GRUNT
	setflag FLAG_HIDE_SLATEPORT_CITY_HARBOR_ARCHIE
	setflag FLAG_HIDE_SLATEPORT_CITY_HARBOR_SUBMARINE_SHADOW
	setflag FLAG_HIDE_AQUA_HIDEOUT_1F_GRUNT_1_BLOCKING_ENTRANCE
	setflag FLAG_HIDE_AQUA_HIDEOUT_1F_GRUNT_2_BLOCKING_ENTRANCE
	settrainerflag TRAINER_MATT
	setflag FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE
	setflag FLAG_HIDE_LILYCOVE_CITY_AQUA_GRUNTS
	setflag FLAG_VISITED_MOSSDEEP_CITY
	return

@ MossdeepCity_Gym (Tate & Liza), MossdeepCity_SpaceCenter_2F (Maxie + Tabitha with Steven),
@ MossdeepCity_StevensHouse (Dive), SeafloorCavern_Room9 (Archie, Kyogre), Route128,
@ CaveOfOrigin_B1F + SkyPillar (Wallace, Rayquaza), SootopolisCity Rayquaza scene and Waterfall.
@ Ends with the weather calm, Archie and Maxie gone and the Gym door open.
Ascension_Story_Gym7ToGym8::
	setflag FLAG_DEFEATED_MOSSDEEP_GYM
	setflag FLAG_BADGE07_GET
	settrainerflag TRAINER_TATE_AND_LIZA_1
	setflag FLAG_RECEIVED_TM_CALM_MIND
	additem ITEM_TM_CALM_MIND
	setflag FLAG_HIDE_AQUA_HIDEOUT_GRUNTS
	setvar VAR_MOSSDEEP_CITY_STATE, 3
	setvar VAR_MOSSDEEP_SPACE_CENTER_STATE, 3
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_TEAM_MAGMA
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_TEAM_MAGMA
	setflag FLAG_HIDE_MOSSDEEP_CITY_TEAM_MAGMA
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_1F_STEVEN
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_2F_STEVEN
	setflag FLAG_DEFEATED_MAGMA_SPACE_CENTER
	setflag FLAG_HIDE_MOSSDEEP_CITY_SPACE_CENTER_MAGMA_NOTE
	settrainerflag TRAINER_MAXIE_MOSSDEEP
	settrainerflag TRAINER_TABITHA_MOSSDEEP
	setflag FLAG_RECEIVED_HM_DIVE
	additem ITEM_HM_DIVE
	setflag FLAG_OMIT_DIVE_FROM_STEVEN_LETTER
	setflag FLAG_HIDE_MOSSDEEP_CITY_SCOTT
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ENTRANCE_AQUA_GRUNT
	setflag FLAG_HIDE_MOSSDEEP_CITY_STEVENS_HOUSE_STEVEN
	setvar VAR_STEVENS_HOUSE_STATE, 2
	settrainerflag TRAINER_ARCHIE
	setflag FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN
	setvar VAR_SEAFLOOR_CAVERN_STATE, 1
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ROOM_9_ARCHIE
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ROOM_9_MAXIE
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ROOM_9_MAGMA_GRUNTS
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_ROOM_9_KYOGRE
	setflag FLAG_HIDE_SEAFLOOR_CAVERN_AQUA_GRUNTS
	setvar VAR_ROUTE128_STATE, 2
	setflag FLAG_HIDE_ROUTE_128_STEVEN
	setflag FLAG_HIDE_ROUTE_128_ARCHIE
	setflag FLAG_HIDE_ROUTE_128_MAXIE
	setflag FLAG_VISITED_SOOTOPOLIS_CITY
	setflag FLAG_STEVEN_GUIDES_TO_CAVE_OF_ORIGIN
	setflag FLAG_WALLACE_GOES_TO_SKY_PILLAR
	setflag FLAG_HIDE_SKY_PILLAR_WALLACE
	setvar VAR_SKY_PILLAR_RAYQUAZA_CRY_DONE, 1
	@ Rayquaza calmed Groudon and Kyogre (SootopolisCity_EventScript_RayquazaSceneFromPokeCenter)
	setvar VAR_SOOTOPOLIS_CITY_STATE, 5
	setvar VAR_SKY_PILLAR_STATE, 3
	clearflag FLAG_SYS_WEATHER_CTRL
	clearflag FLAG_LEGENDARIES_IN_SOOTOPOLIS
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_RAYQUAZA
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_GROUDON
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_KYOGRE
	clearflag FLAG_HIDE_SOOTOPOLIS_CITY_STEVEN
	clearflag FLAG_HIDE_SOOTOPOLIS_CITY_RESIDENTS
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_MAN_1
	clearflag FLAG_HIDE_SOOTOPOLIS_CITY_WALLACE
	setflag FLAG_MET_MAXIE_SOOTOPOLIS
	setflag FLAG_MET_ARCHIE_SOOTOPOLIS
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_MAXIE
	setflag FLAG_HIDE_SOOTOPOLIS_CITY_ARCHIE
	setflag FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE
	clearflag FLAG_HIDE_MT_PYRE_SUMMIT_MAXIE
	clearflag FLAG_HIDE_MT_PYRE_SUMMIT_ARCHIE
	setvar VAR_MT_PYRE_STATE, 2
	setflag FLAG_RECEIVED_HM_WATERFALL
	additem ITEM_HM_WATERFALL
	setvar VAR_SOOTOPOLIS_WALLACE_STATE, 1
	return

@ ---------------------------------------------------------------- teams and supplies
@ Money: start is ₽3,000; each block adds roughly gym prizes + trainer winnings so far.
Ascension_CPTeam_PokeBalls::
	givemon SPECIES_MUDKIP, 7
	return

Ascension_CPTeam_Gym1::
	givemon SPECIES_MUDKIP, 15
	givemon SPECIES_TAILLOW, 13
	additem ITEM_POTION, 3
	additem ITEM_ANTIDOTE, 1
	addmoney 2400
	return

Ascension_CPTeam_Gym2::
	givemon SPECIES_MARSHTOMP, 19
	givemon SPECIES_TAILLOW, 18
	givemon SPECIES_GEODUDE, 17
	additem ITEM_POTION, 5
	additem ITEM_ANTIDOTE, 2
	additem ITEM_POKE_BALL, 5
	addmoney 4000
	return

Ascension_CPTeam_Gym3::
	givemon SPECIES_MARSHTOMP, 24
	givemon SPECIES_SWELLOW, 23
	givemon SPECIES_GEODUDE, 22
	givemon SPECIES_KIRLIA, 22
	additem ITEM_SUPER_POTION, 4
	additem ITEM_PARALYZE_HEAL, 2
	additem ITEM_GREAT_BALL, 5
	addmoney 7000
	return

Ascension_CPTeam_Gym4::
	givemon SPECIES_MARSHTOMP, 29
	givemon SPECIES_SWELLOW, 28
	givemon SPECIES_GRAVELER, 27
	givemon SPECIES_KIRLIA, 27
	givemon SPECIES_MACHOP, 27
	additem ITEM_SUPER_POTION, 6
	additem ITEM_BURN_HEAL, 3
	additem ITEM_GREAT_BALL, 5
	addmoney 11000
	return

Ascension_CPTeam_Gym5::
	givemon SPECIES_MARSHTOMP, 31
	givemon SPECIES_SWELLOW, 30
	givemon SPECIES_MACHOKE, 30
	givemon SPECIES_GRAVELER, 29
	givemon SPECIES_KIRLIA, 29
	givemon SPECIES_NUMEL, 28
	additem ITEM_SUPER_POTION, 8
	additem ITEM_FULL_HEAL, 2
	additem ITEM_GREAT_BALL, 5
	addmoney 15000
	return

Ascension_CPTeam_Gym6::
	givemon SPECIES_MARSHTOMP, 33
	givemon SPECIES_SWELLOW, 32
	givemon SPECIES_MACHOKE, 32
	givemon SPECIES_GRAVELER, 31
	givemon SPECIES_GARDEVOIR, 31
	givemon SPECIES_NUMEL, 31
	additem ITEM_HYPER_POTION, 4
	additem ITEM_FULL_HEAL, 3
	additem ITEM_ULTRA_BALL, 5
	addmoney 20000
	return

Ascension_CPTeam_Gym7::
	givemon SPECIES_SWAMPERT, 42
	givemon SPECIES_SWELLOW, 40
	givemon SPECIES_MACHOKE, 40
	givemon SPECIES_GRAVELER, 39
	givemon SPECIES_GARDEVOIR, 40
	givemon SPECIES_CAMERUPT, 40
	additem ITEM_HYPER_POTION, 6
	additem ITEM_FULL_HEAL, 4
	additem ITEM_REVIVE, 2
	additem ITEM_ULTRA_BALL, 10
	addmoney 28000
	return

Ascension_CPTeam_Gym8::
	givemon SPECIES_SWAMPERT, 46
	givemon SPECIES_SWELLOW, 44
	givemon SPECIES_MACHOKE, 44
	givemon SPECIES_GRAVELER, 43
	givemon SPECIES_GARDEVOIR, 45
	givemon SPECIES_CAMERUPT, 44
	additem ITEM_HYPER_POTION, 8
	additem ITEM_FULL_HEAL, 5
	additem ITEM_REVIVE, 4
	additem ITEM_ULTRA_BALL, 10
	addmoney 36000
	return
