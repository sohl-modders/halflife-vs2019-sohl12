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
#ifndef CALLYMONSTER_H
#define CALLYMONSTER_H

#ifndef TALKMONSTER_H
#include "talkmonster.h"
#endif

/**
*	@brief Base class for Opposing force allies
*	A link to CTalkMonster
*/
class CAllyMonster : public CTalkMonster
{
public:
	static const char* m_szFriends[TLK_CFRIENDS];		// array of friend names
	static float g_talkWaitTime;
};

#endif //CALLYMONSTER_H
