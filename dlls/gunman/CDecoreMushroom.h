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

#ifndef CDECOREMUSHROOM_H
#define CDECOREMUSHROOM_H

#ifndef CDECORE_H
#include "CDecore.h"
#endif

class CDecoreMushroom : public CDecore
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iPose;

	static const char* m_szPoses[3];
};

// #################################
// Alias from CDecoreMushroom
// #################################
class CDecoreMushroom2 : public CDecoreMushroom
{
public:
	void Spawn() override
	{
		pev->model = MAKE_STRING("models/mushroom2.mdl"); //Set Model
		CDecoreMushroom::Spawn();
	}
};

LINK_ENTITY_TO_CLASS(decore_mushroom2, CDecoreMushroom2);

#endif	// CDECOREMUSHROOM_H
