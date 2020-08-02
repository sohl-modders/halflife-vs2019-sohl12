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
#ifndef CCONTROLLER_H
#define CCONTROLLER_H

#ifndef CSQUADMONSTER_H
#include "CSquadMonster.h"
#endif

#define	CONTROLLER_AE_HEAD_OPEN		1
#define	CONTROLLER_AE_BALL_SHOOT	2
#define	CONTROLLER_AE_SMALL_SHOOT	3
#define CONTROLLER_AE_POWERUP_FULL	4
#define CONTROLLER_AE_POWERUP_HALF	5

#define CONTROLLER_FLINCH_DELAY			2		// at most one flinch every n secs

class CController : public CSquadMonster
{
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int Classify(void) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void RunAI(void) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override; // balls
	BOOL CheckRangeAttack2(float flDot, float flDist) override; // head
	BOOL CheckMeleeAttack1(float flDot, float flDist) override; // block, throw
	Schedule_t* GetSchedule(void) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	CUSTOM_SCHEDULES;

	void Stop(void) override;
	void Move(float flInterval) override;
	int CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist) override;
	void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void SetActivity(Activity NewActivity) override;
	BOOL ShouldAdvanceRoute(float flWaypointDist) override;
	int LookupFloat();

	float m_flNextFlinch;

	float m_flShootTime;
	float m_flShootEnd;

	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;
	void AttackSound(void);
	void DeathSound(void) override;

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void GibMonster(void) override;

	CSprite* m_pBall[2]; // hand balls
	int m_iBall[2]; // how bright it should be
	float m_iBallTime[2]; // when it should be that color
	int m_iBallCurrent[2]; // current brightness

	Vector m_vecEstVelocity;

	Vector m_velocity;
	int m_fInCombat;
};

#endif
