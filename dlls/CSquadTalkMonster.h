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
#ifndef CSQUADTALKMONSTER_H
#define CSQUADTALKMONSTER_H

#ifndef MONSTERS_H
#include "monsters.h"
#endif

#ifndef SQUADMONSTER_H
#include "squadmonster.h"
#endif

#ifndef CALLYMONSTER_H
#include "CAllyMonster.h"
#endif

//=========================================================
// CSquadTalkMonster - for any monster that forms squads.
//=========================================================
class CSquadTalkMonster : public CAllyMonster
{
public:
	// squad leader info
	EHANDLE m_hSquadLeader; // who is my leader
	EHANDLE m_hSquadMember[MAX_SQUAD_MEMBERS - 1]; // valid only for leader
	int m_afSquadSlots;
	float m_flLastEnemySightTime; // last time anyone in the squad saw the enemy
	BOOL m_fEnemyEluded;

	EHANDLE m_hWaitMedic;
	float m_flMedicWaitTime;
	float m_flLastHitByPlayer;
	int m_iPlayerHits;

	// squad member info
	int m_iMySlot; // this is the behaviour slot that the monster currently holds in the squad. 

	int CheckEnemy(CBaseEntity* pEnemy);
	void StartMonster();
	void VacateSlot();
	void ScheduleChange();
	void Killed(entvars_t* pevAttacker, int iGib);
	BOOL OccupySlot(int iDesiredSlot);
	BOOL NoFriendlyFire(BOOL playerAlly);

	// squad functions still left in base class
	CSquadTalkMonster* MySquadLeader()
	{
		auto* const pSquadLeader = reinterpret_cast<CSquadTalkMonster*>(static_cast<CBaseEntity*>(m_hSquadLeader));
		if (pSquadLeader != nullptr)
			return pSquadLeader;
		
		return this;
	}

	CSquadTalkMonster* MySquadMember(int i)
	{
		if (i >= MAX_SQUAD_MEMBERS - 1)
			return this;

		return (CSquadTalkMonster*)static_cast<CBaseEntity*>(m_hSquadMember[i]);
	}

	int InSquad() { return m_hSquadLeader != NULL; }
	int IsLeader() { return m_hSquadLeader == this; }
	
	int SquadRecruit(int searchRadius, int maxMembers);
	int SquadCount();
	void SquadRemove(CSquadTalkMonster* pRemove);
	BOOL SquadAdd(CSquadTalkMonster* pAdd);
	void SquadMakeEnemy(CBaseEntity* pEnemy);
	void SquadPasteEnemyInfo();
	void SquadCopyEnemyInfo();
	BOOL SquadEnemySplit();
	BOOL SquadMemberInRange(const Vector& vecLocation, float flDist);

	virtual CSquadTalkMonster* MySquadTalkMonsterPointer() { return this; }

	static TYPEDESCRIPTION m_SaveData[];

	int Save(CSave& save);
	int Restore(CRestore& restore);

	BOOL FValidateCover(const Vector& vecCoverLocation);

	MONSTERSTATE GetIdealState();
	Schedule_t* GetScheduleOfType(int iType);

	void EXPORT FollowerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	CSquadTalkMonster* MySquadMedic();

	CSquadTalkMonster* FindSquadMedic(int searchRadius);

	BOOL HealMe(CSquadTalkMonster* pTarget);
};

#endif
