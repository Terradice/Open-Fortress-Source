// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: General definitions
// Author(s): ficool2, Fenteale, KaydemonLP, Ms
//

#include "cbase.h"
#include "of_shareddefs.h"

const char *g_aWeaponModePrefix[] =
{
	"", // Primary parse doesnt use any prefixes
	"secondary_", // Primary parse doesnt use any prefixes
};

uint g_aWeaponDamageTypes[] =
{
	// explanation on how these values were found: https://gist.github.com/CherrySodaPop/f5038de894389d58ebc4ffc1a8bcf5eb - cherry

	// OFTODO: there might be some econ weapons here and cut weapons, they are only for refrence
	//			to make sure the other weapon ids are correct, remove them in the future! - cherry

	DMG_GENERIC,							// WEAPON_NONE
						
	DMG_CLUB,								// OF_WEAPON_BAT
	DMG_CLUB,								// OF_WEAPON_BAT_WOOD - econ, only for refrence!
	DMG_CLUB,								// OF_WEAPON_BOTTLE
	DMG_CLUB,								// OF_WEAPON_FIREAXE
	DMG_CLUB,								// OF_WEAPON_CLUB
	DMG_CLUB,								// OF_WEAPON_CROWBAR
	DMG_SLASH,								// OF_WEAPON_KNIFE
	DMG_CLUB,								// OF_WEAPON_FISTS
	DMG_CLUB,								// OF_WEAPON_SHOVEL
	DMG_CLUB,								// OF_WEAPON_WRENCH
	DMG_SLASH,								// OF_WEAPON_BONESAW
	DMG_BUCKSHOT | DMG_USEDISTANCEMOD,		// OF_WEAPON_SHOTGUN
	DMG_BUCKSHOT | DMG_USEDISTANCEMOD,		// OF_WEAPON_SHOTGUN_SOLDIER
	DMG_BUCKSHOT | DMG_USEDISTANCEMOD,		// OF_WEAPON_SHOTGUN_HWG
	DMG_BUCKSHOT | DMG_USEDISTANCEMOD,		// OF_WEAPON_SHOTGUN_PYRO
	DMG_BUCKSHOT | DMG_USEDISTANCEMOD,		// OF_WEAPON_SCATTERGUN
	DMG_BULLET | DMG_USE_HITLOCATIONS,		// OF_WEAPON_SNIPERRIFLE
	DMG_BULLET | DMG_USE_HITLOCATIONS,		// OF_WEAPON_MINIGUN
	DMG_BULLET | DMG_USEDISTANCEMOD,		// OF_WEAPON_SMG
	DMG_BULLET | DMG_PREVENT_PHYSICS_FORCE | DMG_NOCLOSEDISTANCEMOD | DMG_USEDISTANCEMOD,	// OF_WEAPON_SYRINGEGUN_MEDIC
	DMG_BULLET | DMG_PREVENT_PHYSICS_FORCE | DMG_PARALYZE | DMG_USEDISTANCEMOD,				// OF_WEAPON_TRANQ
	DMG_BLAST | DMG_HALF_FALLOFF | DMG_USEDISTANCEMOD,										// OF_WEAPON_ROCKETLAUNCHER
	DMG_BLAST | DMG_HALF_FALLOFF | DMG_USEDISTANCEMOD,										// OF_WEAPON_GRENADELAUNCHER
	DMG_BLAST | DMG_NOCLOSEDISTANCEMOD | DMG_HALF_FALLOFF,									// OF_WEAPON_PIPEBOMBLAUNCHER
	DMG_IGNITE | DMG_PREVENT_PHYSICS_FORCE,	// OF_WEAPON_FLAMETHROWER
	DMG_BLAST | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_NORMAL
	DMG_SONIC | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_CONCUSSION
	DMG_BULLET | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_NAIL
	DMG_BLAST | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_MIRV
	DMG_BLAST | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_MIRV_DEMOMAN
	DMG_BURN | DMG_RADIUS_MAX,				// OF_WEAPON_GRENADE_NAPALM
	DMG_NOCLOSEDISTANCEMOD | DMG_HALF_FALLOFF,					// OF_WEAPON_GRENADE_GAS
	DMG_BLAST | DMG_PREVENT_PHYSICS_FORCE | DMG_HALF_FALLOFF,	// OF_WEAPON_GRENADE_EMP
	DMG_GENERIC,												// OF_WEAPON_GRENADE_CALTROP
	DMG_BLAST | DMG_NOCLOSEDISTANCEMOD | DMG_HALF_FALLOFF,		// OF_WEAPON_GRENADE_PIPEBOMB
	DMG_GENERIC,							// OF_WEAPON_GRENADE_SMOKE_BOMB
	DMG_GENERIC,							// OF_WEAPON_GRENADE_HEAL
	DMG_CLUB,								// OF_WEAPON_GRENADE_STUNBALL
	DMG_CLUB,								// OF_WEAPON_GRENADE_JAR
	DMG_CLUB,								// OF_WEAPON_GRENADE_JAR_MILK
	DMG_BULLET | DMG_USEDISTANCEMOD,		// OF_WEAPON_PISTOL
	DMG_BULLET | DMG_USEDISTANCEMOD,		// OF_WEAPON_PISTOL_SCOUT
	DMG_BULLET | DMG_USEDISTANCEMOD,		// OF_WEAPON_REVOLVER
	DMG_BULLET | DMG_PREVENT_PHYSICS_FORCE | DMG_NOCLOSEDISTANCEMOD | DMG_USEDISTANCEMOD,	// OF_WEAPON_NAILGUN
	DMG_BULLET,								// OF_WEAPON_PDA
	DMG_BULLET,								// OF_WEAPON_PDA_ENGINEER_BUILD
	DMG_BULLET,								// OF_WEAPON_PDA_ENGINEER_DESTROY
	DMG_BULLET,								// OF_WEAPON_PDA_SPY
	DMG_BULLET,								// OF_WEAPON_BUILDER
	DMG_BULLET,								// OF_WEAPON_MEDIGUN
	DMG_BLAST,								// OF_WEAPON_GRENADE_MIRVBOMB
	DMG_BLAST | DMG_RADIUS_MAX | DMG_IGNITE,// OF_WEAPON_FLAMETHROWER_ROCKET
	DMG_BLAST | DMG_HALF_FALLOFF,			// OF_WEAPON_GRENADE_DEMOMAN
	DMG_BULLET,								// OF_WEAPON_SENTRY_BULLET
	DMG_BLAST,								// OF_WEAPON_SENTRY_ROCKET
	DMG_GENERIC,							// OF_WEAPON_DISPENSER
	DMG_GENERIC,							// OF_WEAPON_INVIS

	DMG_GENERIC,							// WEAPON_OFTODO
	DMG_GENERIC,							// WEAPON_MAX
};

const char *g_aProjectileTypeNames[] =
{
	"projectile_none",
	"projectile_bullet",
	"projectile_rocket",
	"projectile_pipe",
	"projectile_pipe_remote",
	"projectile_syringe",
	"projectile_flare",
	"projectile_jar",
	"projectile_arrow",
	"projectile_flamerocket",
	"projectile_jar_milk",
	"projectile_healing_bolt",
	"projectile_energy_ball",
	"projectile_energy_ring",
	"projectile_stickyjumper",
	"projectile_cleaver",
	"projectile_unknown1",
	"projectile_cannonball",
	"projectile_ranger",
	"projectile_arrow_festive",
	"projectile_unknown2",
	"projectile_unknown3",
	"projectile_jarate_festive",
	"projectile_crossbow_festive",
	"projectile_jarate_bread",
	"projectile_madmilk_bread",
	"tf_projectile_grapplinghook",
	"tf_projectile_balloffire",
	"projectile_jar_gas",
};

const char *g_aAmmoNames[] =
{
	"DUMMY AMMO", // This ammo is skipped
	"TF_AMMO_PRIMARY",
	"TF_AMMO_SECONDARY",
	"TF_AMMO_METAL",
	"TF_AMMO_GRENADES1",
	"TF_AMMO_GRENADES2",
	"TF_AMMO_GRENADES3"
};

const char *g_aTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Red",
	"Blue",
	"Mercenary"
};

const char* s_ValveMaps[][3] =
{
	"ctf_2fort", "2Fort", "#Gametype_CTF",
};

const char* s_CommunityMaps[][3] =
{
	"pl_borneo", "Borneo", "#Gametype_Escort",
};