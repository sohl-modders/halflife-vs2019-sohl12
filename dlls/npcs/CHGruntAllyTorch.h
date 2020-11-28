/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef CHGRUNTALLYTORCH_H
#define CHGRUNTALLYTORCH_H

#ifndef SCHEDULE_H
#include "schedule.h"
#endif
#include "CSquadTalkMonster.h"

#define	TORCH_DEAGLE_CLIP_SIZE			8 // how many bullets in a clip?
#define GRUNT_VOL						0.35		// volume of grunt sounds
#define GRUNT_ATTN						ATTN_NORM	// attenutation of grunt sentences
#define HGRUNT_LIMP_HEALTH				20
#define HGRUNT_DMG_HEADSHOT				( DMG_BULLET | DMG_CLUB )	// damage types that can kill a grunt with a single headshot.
#define HGRUNT_NUM_HEADS				2 // how many grunt heads are there? 
#define HGRUNT_MINIMUM_HEADSHOT_DAMAGE	15 // must do at least this much damage in one shot to head to score a headshot kill
#define	TORCH_SENTENCE_VOLUME			(float)0.35 // volume of grunt sentences
#define TORCH_BEAM_SPRITE "sprites/xbeam3.spr"

//=========================================================
// monster-specific DEFINE's
//=========================================================
namespace TorchAllyBodygroup
{
	enum TorchAllyBodygroup
	{
		Head = 1,
		Weapons = 2
	};
}

namespace TorchAllyWeapon
{
	enum TorchAllyWeapon
	{
		DesertEagle = 0,
		Torch,
		None
	};
}

namespace TorchAllyWeaponFlag
{
	enum TorchAllyWeaponFlag
	{
		DesertEagle = 1 << 0,
		Torch = 1 << 1,
		HandGrenade = 1 << 2,
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		TORCH_AE_RELOAD		( 2 )
#define		TORCH_AE_KICK			( 3 )
#define		TORCH_AE_SHOOT			( 4 )
#define		TORCH_AE_GREN_TOSS		( 7 )
#define		TORCH_AE_GREN_DROP		( 9 )
#define		TORCH_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		TORCH_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define TORCH_AE_HOLSTER_TORCH		17
#define TORCH_AE_HOLSTER_GUN		18
#define TORCH_AE_HOLSTER_BOTH		19
#define TORCH_AE_ACTIVATE_TORCH		20
#define TORCH_AE_DEACTIVATE_TORCH	21

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_TORCH_ALLY_SUPPRESS = LAST_TALKMONSTER_SCHEDULE + 1,
	SCHED_TORCH_ALLY_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_TORCH_ALLY_COVER_AND_RELOAD,
	SCHED_TORCH_ALLY_SWEEP,
	SCHED_TORCH_ALLY_FOUND_ENEMY,
	SCHED_TORCH_ALLY_REPEL,
	SCHED_TORCH_ALLY_REPEL_ATTACK,
	SCHED_TORCH_ALLY_REPEL_LAND,
	SCHED_TORCH_ALLY_WAIT_FACE_ENEMY,
	SCHED_TORCH_ALLY_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_TORCH_ALLY_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_TORCH_ALLY_FACE_TOSS_DIR = LAST_TALKMONSTER_TASK + 1,
	TASK_TORCH_ALLY_SPEAK_SENTENCE,
	TASK_TORCH_ALLY_CHECK_FIRE,
};

//=========================================================
// Class definition of CHGrunt
//=========================================================
class CHGruntAllyTorch : public CSquadTalkMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	int ISoundMask(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL FCanCheckAttacks(void);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	void CheckAmmo(void);
	void SetActivity(Activity NewActivity);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	void DeathSound(void);
	void PainSound(void);
	void IdleSound(void);
	Vector GetGunPosition(void);
	void Shoot(void);
	void PrescheduleThink(void);
	void GibMonster(void);
	void SpeakSentence(void);

	int	Save(CSave& save);
	int Restore(CRestore& restore);

	CBaseEntity* Kick(void);
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	BOOL FOkToSpeak(void);
	void JustSpoke(void);

	int ObjectCaps() override;

	void TalkInit();

	void AlertSound() override;

	void DeclineFollowing() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void MonsterThink() override;

	MONSTERSTATE GetIdealState()
	{
		return CSquadTalkMonster::GetIdealState();
	}

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;

	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector m_vecTossVelocity;

	BOOL m_fThrowGrenade;
	BOOL m_fStanding;
	BOOL m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int m_cClipSize;

	BOOL m_fUseTorch;
	EHANDLE m_hNewTargetEnt;
	int m_iBlackOrWhite;
	BOOL m_fGunHolstered;
	BOOL m_fTorchHolstered;
	BOOL m_fTorchActive;

	CBeam* m_pTorchBeam;

	float m_flLastShot;

	int m_iSentence;

	static const char* pTorchSentences[];
};


#endif // CHGRUNTALLYTORCH_H
