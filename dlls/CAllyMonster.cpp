/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "talkmonster.h"
#include "CAllyMonster.h"

// time delay until it's ok to speak: used so that two NPCs don't talk at once
float CAllyMonster::g_talkWaitTime = 0;

// array of friend names
const char* CAllyMonster::m_szFriends[TLK_CFRIENDS] =
{
	"monster_human_grunt_ally",
	"monster_human_medic_ally",
	"monster_human_torch_ally",
};