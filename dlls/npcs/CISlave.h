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
#ifndef CISLAVE_H
#define CISLAVE_H

#ifndef CSQUADMONSTER_H
#include "CSquadMonster.h"
#endif

#define		ISLAVE_AE_CLAW		( 1 )
#define		ISLAVE_AE_CLAWRAKE	( 2 )
#define		ISLAVE_AE_ZAP_POWERUP	( 3 )
#define		ISLAVE_AE_ZAP_SHOOT		( 4 )
#define		ISLAVE_AE_ZAP_DONE		( 5 )

#define		ISLAVE_MAX_BEAMS	8

class CISlave : public CSquadMonster
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int ISoundMask(void) override;
	int Classify(void) override;
	int IRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, Vector& vecLocation);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
	                 int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void DeathSound(void) override;
	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule(void) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();
	void ArmBeam(int side);
	void WackBeam(int side, CBaseEntity* pEntity);
	void ZapBeam(int side);
	void BeamGlow(void);

	int m_iBravery;

	CBeam* m_pBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int m_voicePitch;

	EHANDLE m_hDead;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};

#endif
