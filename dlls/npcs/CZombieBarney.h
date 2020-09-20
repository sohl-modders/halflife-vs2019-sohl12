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

#ifndef CZOMBIEBARNEY_H
#define CZOMBIEBARNEY_H

#ifndef CZOMBIE_H
#include "CZombie.h"
#endif

//=========================================================
// Class definition of CZombieBarney
//=========================================================
class CZombieBarney : public CZombie
{
public:
	void Spawn() override;
	void Precache() override;
};

#endif // CZOMBIEBARNEY_H
