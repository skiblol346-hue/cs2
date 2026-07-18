#pragma once

#include <map>

// @source: master/game/shared/cstrike15/gametypes.h
#pragma region valve_gametypes

enum EGameType : int
{
	GAMETYPE_UNKNOWN = -1,
	GAMETYPE_CLASSIC,
	GAMETYPE_GUNGAME,
	GAMETYPE_TRAINING,
	GAMETYPE_CUSTOM,
	GAMETYPE_COOPERATIVE,
	GAMETYPE_SKIRMISH,
	GAMETYPE_FREEFORALL
};

enum EGameMode : int
{
	GAMEMODE_UNKNOWN = -1,

	// GAMETYPE_CLASSIC
	GAMEMODE_CLASSIC_CASUAL = 0,
	GAMEMODE_CLASSIC_COMPETITIVE,
	GAMEMODE_CLASSIC_SCRIM_COMPETITIVE2V2,
	GAMEMODE_CLASSIC_SCRIM_COMPETITIVE5V5,

	// GAMETYPE_GUNGAME
	GAMEMODE_GUNGAME_PROGRESSIVE = 0,
	GAMEMODE_GUNGAME_BOMB,
	GAMEMODE_GUNGAME_DEATHMATCH,

	// GAMETYPE_TRAINING
	GAMEMODE_TRAINING_DEFAULT = 0,

	// GAMETYPE_CUSTOM
	GAMEMODE_CUSTOM_DEFAULT = 0,

	// GAMETYPE_COOPERATIVE
	GAMEMODE_COOPERATIVE_DEFAULT = 0,
	GAMEMODE_COOPERATIVE_MISSION,

	// GAMETYPE_SKIRMISH
	GAMEMODE_SKIRMISH_DEFAULT = 0,

	// GAMETYPE_FREEFORALL
	GAMEMODE_FREEFORALL_SURVIVAL = 0
};

#pragma endregion

enum ELifeState : int
{
	LIFE_ALIVE = 0,
	LIFE_DYING,
	LIFE_DEAD,
	LIFE_RESPAWNABLE,
	LIFE_DISCARDBODY
};

enum EFlags : int
{
	FL_ONGROUND = ( 1 << 0 ), // entity is at rest / on the ground
	FL_DUCKING = ( 1 << 1 ), // player is fully crouched/uncrouched
	FL_ANIMDUCKING = ( 1 << 2 ), // player is in the process of crouching or uncrouching but could be in transition
	FL_WATERJUMP = ( 1 << 3 ), // player is jumping out of water
	FL_ONTRAIN = ( 1 << 4 ), // player is controlling a train, so movement commands should be ignored on client during prediction
	FL_INRAIN = ( 1 << 5 ), // entity is standing in rain
	FL_FROZEN = ( 1 << 6 ), // player is frozen for 3rd-person camera
	FL_ATCONTROLS = ( 1 << 7 ), // player can't move, but keeps key inputs for controlling another entity
	FL_CLIENT = ( 1 << 8 ), // entity is a client (player)
	FL_FAKECLIENT = ( 1 << 9 ), // entity is a fake client, simulated server side; don't send network messages to them
	FL_INWATER = ( 1 << 10 ), // entity is in water
	FL_FLY = ( 1 << 11 ),
	FL_SWIM = ( 1 << 12 ),
	FL_CONVEYOR = ( 1 << 13 ),
	FL_NPC = ( 1 << 14 ),
	FL_GODMODE = ( 1 << 15 ),
	FL_NOTARGET = ( 1 << 16 ),
	FL_AIMTARGET = ( 1 << 17 ),
	FL_PARTIALGROUND = ( 1 << 18 ), // entity is standing on a place where not all corners are valid
	FL_STATICPROP = ( 1 << 19 ), // entity is a static property
	FL_GRAPHED = ( 1 << 20 ),
	FL_GRENADE = ( 1 << 21 ),
	FL_STEPMOVEMENT = ( 1 << 22 ),
	FL_DONTTOUCH = ( 1 << 23 ),
	FL_BASEVELOCITY = ( 1 << 24 ), // entity have applied base velocity this frame
	FL_WORLDBRUSH = ( 1 << 25 ), // entity is not moveable/removeable brush (part of the world, but represented as an entity for transparency or something)
	FL_OBJECT = ( 1 << 26 ),
	FL_KILLME = ( 1 << 27 ), // entity is marked for death and will be freed by the game
	FL_ONFIRE = ( 1 << 28 ),
	FL_DISSOLVING = ( 1 << 29 ),
	FL_TRANSRAGDOLL = ( 1 << 30 ), // entity is turning into client-side ragdoll
	FL_UNBLOCKABLE_BY_PLAYER = ( 1 << 31 )
};

enum EEFlags : int
{
	EFL_KILLME = ( 1 << 0 ),
	EFL_DORMANT = ( 1 << 1 ),
	EFL_NOCLIP_ACTIVE = ( 1 << 2 ),
	EFL_SETTING_UP_BONES = ( 1 << 3 ),
	EFL_KEEP_ON_RECREATE_ENTITIES = ( 1 << 4 ),
	EFL_DIRTY_SHADOWUPDATE = ( 1 << 5 ),
	EFL_NOTIFY = ( 1 << 6 ),
	EFL_FORCE_CHECK_TRANSMIT = ( 1 << 7 ),
	EFL_BOT_FROZEN = ( 1 << 8 ),
	EFL_SERVER_ONLY = ( 1 << 9 ),
	EFL_NO_AUTO_EDICT_ATTACH = ( 1 << 10 ),
	EFL_DIRTY_ABSTRANSFORM = ( 1 << 11 ),
	EFL_DIRTY_ABSVELOCITY = ( 1 << 12 ),
	EFL_DIRTY_ABSANGVELOCITY = ( 1 << 13 ),
	EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS = ( 1 << 14 ),
	EFL_DIRTY_SPATIAL_PARTITION = ( 1 << 15 ),
	EFL_HAS_PLAYER_CHILD = ( 1 << 16 ),
	EFL_IN_SKYBOX = ( 1 << 17 ),
	EFL_USE_PARTITION_WHEN_NOT_SOLID = ( 1 << 18 ),
	EFL_TOUCHING_FLUID = ( 1 << 19 ),
	EFL_IS_BEING_LIFTED_BY_BARNACLE = ( 1 << 20 ),
	EFL_NO_ROTORWASH_PUSH = ( 1 << 21 ),
	EFL_NO_THINK_FUNCTION = ( 1 << 22 ),
	EFL_NO_GAME_PHYSICS_SIMULATION = ( 1 << 23 ),
	EFL_CHECK_UNTOUCH = ( 1 << 24 ),
	EFL_DONTBLOCKLOS = ( 1 << 25 ),
	EFL_DONTWALKON = ( 1 << 26 ),
	EFL_NO_DISSOLVE = ( 1 << 27 ),
	EFL_NO_MEGAPHYSCANNON_RAGDOLL = ( 1 << 28 ),
	EFL_NO_WATER_VELOCITY_CHANGE = ( 1 << 29 ),
	EFL_NO_PHYSCANNON_INTERACTION = ( 1 << 30 ),
	EFL_NO_DAMAGE_FORCES = ( 1 << 31 )
};

enum EMoveType : std::uint8_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_OBSOLETE,
	MOVETYPE_WALK,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_OBSERVER,
	MOVETYPE_LADDER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST,
	MOVETYPE_INVALID,
	MOVETYPE_MAX_BITS = 5
};

// identifies how submerged in water a player is
enum : int
{
	WL_NOTINWATER = 0,
	WL_FEET,
	WL_WAIST,
	WL_EYES
};

enum ETeamID : int
{
	TEAM_UNK,
	TEAM_SPECTATOR,
	TEAM_TT,
	TEAM_CT
};

using ItemDefinitionIndex_t = std::uint16_t;
enum EItemDefinitionIndexes : ItemDefinitionIndex_t
{
	WEAPON_NONE,
	WEAPON_DESERT_EAGLE,
	WEAPON_DUAL_BERETTAS,
	WEAPON_FIVE_SEVEN,
	WEAPON_GLOCK_18,
	WEAPON_AK_47 = 7,
	WEAPON_AUG,
	WEAPON_AWP,
	WEAPON_FAMAS,
	WEAPON_G3SG1,
	WEAPON_GALIL_AR = 13,
	WEAPON_M249,
	WEAPON_M4A4 = 16,
	WEAPON_MAC_10,
	WEAPON_P90 = 19,
	WEAPON_REPULSOR_DEVICE,
	WEAPON_MP5_SD = 23,
	WEAPON_UMP_45,
	WEAPON_XM1014,
	WEAPON_PP_BIZON,
	WEAPON_MAG_7,
	WEAPON_NEGEV,
	WEAPON_SAWED_OFF,
	WEAPON_TEC_9,
	WEAPON_ZEUS_X27,
	WEAPON_P2000,
	WEAPON_MP7,
	WEAPON_MP9,
	WEAPON_NOVA,
	WEAPON_P250,
	WEAPON_RIOT_SHIELD,
	WEAPON_SCAR_20,
	WEAPON_SG_553,
	WEAPON_SSG_08,
	WEAPON_KNIFE0,
	WEAPON_KNIFE1,
	WEAPON_FLASHBANG,
	WEAPON_HIGH_EXPLOSIVE_GRENADE,
	WEAPON_SMOKE_GRENADE,
	WEAPON_MOLOTOV,
	WEAPON_DECOY_GRENADE,
	WEAPON_INCENDIARY_GRENADE,
	WEAPON_C4_EXPLOSIVE,
	WEAPON_KEVLAR_VEST,
	WEAPON_KEVLAR_and_HELMET,
	WEAPON_HEAVY_ASSAULT_SUIT,
	WEAPON_NO_LOCALIZED_NAME0 = 54,
	WEAPON_DEFUSE_KIT,
	WEAPON_RESCUE_KIT,
	WEAPON_MEDI_SHOT,
	WEAPON_MUSIC_KIT,
	WEAPON_KNIFE2,
	WEAPON_M4A1_S,
	WEAPON_USP_S,
	WEAPON_TRADE_UP_CONTRACT,
	WEAPON_CZ75_AUTO,
	WEAPON_R8_REVOLVER,
	WEAPON_TACTICAL_AWARENESS_GRENADE = 68,
	WEAPON_BARE_HANDS,
	WEAPON_BREACH_CHARGE,
	WEAPON_TABLET = 72,
	WEAPON_KNIFE3 = 74,
	WEAPON_AXE,
	WEAPON_HAMMER,
	WEAPON_WRENCH = 78,
	WEAPON_SPECTRAL_SHIV = 80,
	WEAPON_FIRE_BOMB,
	WEAPON_DIVERSION_DEVICE,
	WEAPON_FRAG_GRENADE,
	WEAPON_SNOWBALL,
	WEAPON_BUMP_MINE,
	WEAPON_BAYONET = 500,
	WEAPON_CLASSIC_KNIFE = 503,
	WEAPON_FLIP_KNIFE = 505,
	WEAPON_GUT_KNIFE,
	WEAPON_KARAMBIT,
	WEAPON_M9_BAYONET,
	WEAPON_HUNTSMAN_KNIFE,
	WEAPON_FALCHION_KNIFE = 512,
	WEAPON_BOWIE_KNIFE = 514,
	WEAPON_BUTTERFLY_KNIFE,
	WEAPON_SHADOW_DAGGERS,
	WEAPON_PARACORD_KNIFE,
	WEAPON_SURVIVAL_KNIFE,
	WEAPON_URSUS_KNIFE,
	WEAPON_NAVAJA_KNIFE,
	WEAPON_NOMAD_KNIFE,
	WEAPON_STILETTO_KNIFE,
	WEAPON_TALON_KNIFE,
	WEAPON_SKELETON_KNIFE = 525,
};

enum EWeaponType : std::uint32_t
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL = 1,
	WEAPONTYPE_SUBMACHINEGUN = 2,
	WEAPONTYPE_RIFLE = 3,
	WEAPONTYPE_SHOTGUN = 4,
	WEAPONTYPE_SNIPER_RIFLE = 5,
	WEAPONTYPE_MACHINEGUN = 6,
	WEAPONTYPE_C4 = 7,
	WEAPONTYPE_TASER = 8,
	WEAPONTYPE_GRENADE = 9,
	WEAPONTYPE_EQUIPMENT = 10,
	WEAPONTYPE_STACKABLEITEM = 11,
	WEAPONTYPE_FISTS = 12,
	WEAPONTYPE_BREACHCHARGE = 13,
	WEAPONTYPE_BUMPMINE = 14,
	WEAPONTYPE_TABLET = 15,
	WEAPONTYPE_MELEE = 16,
	WEAPONTYPE_SHIELD = 17,
	WEAPONTYPE_ZONE_REPULSOR = 18,
	WEAPONTYPE_UNKNOWN = 19
};

enum ObserverMode_t : int
{
	OBS_MODE_NONE = 0U,
	OBS_MODE_FIXED = 1U,
	OBS_MODE_IN_EYE = 2U,
	OBS_MODE_CHASE = 3U,
	OBS_MODE_ROAMING = 4U,
	OBS_MODE_DIRECTED = 5U,
	NUM_OBSERVER_MODES = 6U,
};

enum Bones : int
{ // Total: 104
	pelvis = 0, // Parent (null)
	spine_0 = 1, // Parent pelvis
	spine_1 = 2, // Parent spine_0
	spine_2 = 3, // Parent spine_1
	spine_3 = 4, // Parent spine_2
	clavicle_l = 5, // Parent spine_3
	clavicle_r = 6, // Parent spine_3
	arm_upper_l = 7, // Parent clavicle_l
	arm_upper_r = 8, // Parent clavicle_r
	arm_lower_l = 9, // Parent arm_upper_l
	arm_lower_r = 10, // Parent arm_upper_r
	hand_l = 11, // Parent arm_lower_l
	hand_r = 12, // Parent arm_lower_r
	finger_middle_meta_l = 13, // Parent hand_l
	finger_pinky_meta_l = 14, // Parent hand_l
	finger_index_meta_l = 15, // Parent hand_l
	finger_ring_meta_l = 16, // Parent hand_l
	finger_middle_meta_r = 17, // Parent hand_r
	finger_pinky_meta_r = 18, // Parent hand_r
	finger_index_meta_r = 19, // Parent hand_r
	finger_ring_meta_r = 20, // Parent hand_r
	chesthier_offset = 21, // Parent spine_2
	leg_upper_l = 22, // Parent pelvis
	leg_upper_r = 23, // Parent pelvis
	neck_0 = 24, // Parent spine_3
	finger_middle_0_l = 25, // Parent finger_middle_meta_l
	finger_pinky_0_l = 26, // Parent finger_pinky_meta_l
	finger_index_0_l = 27, // Parent finger_index_meta_l
	finger_thumb_0_l = 28, // Parent hand_l
	finger_ring_0_l = 29, // Parent finger_ring_meta_l
	finger_middle_0_r = 30, // Parent finger_middle_meta_r
	finger_pinky_0_r = 31, // Parent finger_pinky_meta_r
	finger_index_0_r = 32, // Parent finger_index_meta_r
	finger_thumb_0_r = 33, // Parent hand_r
	finger_ring_0_r = 34, // Parent finger_ring_meta_r
	weaponhier_jnt = 35, // Parent chesthier_offset
	leg_lower_l = 36, // Parent leg_upper_l
	leg_lower_r = 37, // Parent leg_upper_r
	root_motion = 38, // Parent (null)
	head_0 = 39, // Parent neck_0
	finger_middle_1_l = 40, // Parent finger_middle_0_l
	finger_pinky_1_l = 41, // Parent finger_pinky_0_l
	finger_index_1_l = 42, // Parent finger_index_0_l
	finger_thumb_1_l = 43, // Parent finger_thumb_0_l
	finger_ring_1_l = 44, // Parent finger_ring_0_l
	finger_middle_1_r = 45, // Parent finger_middle_0_r
	finger_pinky_1_r = 46, // Parent finger_pinky_0_r
	finger_index_1_r = 47, // Parent finger_index_0_r
	finger_thumb_1_r = 48, // Parent finger_thumb_0_r
	finger_ring_1_r = 49, // Parent finger_ring_0_r
	weaponhier_r_iktarget = 50, // Parent weaponhier_jnt
	pectaim_l = 51, // Parent spine_2
	pectaim_r = 52, // Parent spine_2
	ankle_l = 53, // Parent leg_lower_l
	ankle_r = 54, // Parent leg_lower_r
	leg_l_offset = 55, // Parent root_motion
	leg_r_offset = 56, // Parent root_motion
	eyeball_l = 57, // Parent head_0
	eyeball_r = 58, // Parent head_0
	eye_target = 59, // Parent head_0
	head_0_twist = 60, // Parent neck_0
	finger_middle_2_l = 61, // Parent finger_middle_1_l
	finger_pinky_2_l = 62, // Parent finger_pinky_1_l
	finger_index_2_l = 63, // Parent finger_index_1_l
	finger_thumb_2_l = 64, // Parent finger_thumb_1_l
	finger_ring_2_l = 65, // Parent finger_ring_1_l
	weapon_hand_l = 66, // Parent hand_l
	arm_lower_l_twist = 67, // Parent arm_lower_l
	arm_lower_l_twist1 = 68, // Parent arm_lower_l
	arm_upper_l_twist1 = 69, // Parent arm_upper_l
	arm_upper_l_twist = 70, // Parent arm_upper_l
	pect_l_aimat = 71, // Parent arm_upper_l
	scapula_l = 72, // Parent clavicle_l
	finger_middle_2_r = 73, // Parent finger_middle_1_r
	finger_pinky_2_r = 74, // Parent finger_pinky_1_r
	finger_index_2_r = 75, // Parent finger_index_1_r
	finger_thumb_2_r = 76, // Parent finger_thumb_1_r
	finger_ring_2_r = 77, // Parent finger_ring_1_r
	weapon_hand_r = 78, // Parent hand_r
	arm_lower_r_twist = 79, // Parent arm_lower_r
	arm_lower_r_twist1 = 80, // Parent arm_lower_r
	arm_upper_r_twist1 = 81, // Parent arm_upper_r
	arm_upper_r_twist = 82, // Parent arm_upper_r
	pect_r_aimat = 83, // Parent arm_upper_r
	scapula_r = 84, // Parent clavicle_r
	jiggle_primary = 85, // Parent spine_3
	pect_l_aimup = 86, // Parent spine_3
	pect_r_aimup = 87, // Parent spine_3
	scap_aimup = 88, // Parent spine_3
	weaponhier_l_iktarget = 89, // Parent weaponhier_r_iktarget
	pecttrans_l = 90, // Parent pectaim_l
	pecttrans_r = 91, // Parent pectaim_r
	scap_r_aimat = 92, // Parent spine_2
	scap_l_aimat = 93, // Parent spine_2
	pect_l_ptbase = 94, // Parent spine_2
	pect_r_ptbase = 95, // Parent spine_2
	ball_l = 96, // Parent ankle_l
	leg_upper_l_twist = 97, // Parent leg_upper_l
	leg_upper_l_twist1 = 98, // Parent leg_upper_l
	ball_r = 99, // Parent ankle_r
	leg_upper_r_twist = 100, // Parent leg_upper_r
	leg_upper_r_twist1 = 101, // Parent leg_upper_r
	leg_l_iktarget = 102, // Parent leg_l_offset
	leg_r_iktarget = 103, // Parent leg_r_offset
};

enum EHitboxes : int
{
	/* HEAD */
	HITBOX_HEAD,
	HITBOX_NECK,
	/* BODY */
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_CHEST,
	HITBOX_LOWER_CHEST,
	HITBOX_UPPER_CHEST,
	/* LEGS */
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	/* ARMS */
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX
};