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

#ifndef CRANDOMSPEAKER_H
#define CRANDOMSPEAKER_H

//=========================================================
// Class definition of CRandomSpeaker
//=========================================================
class CRandomSpeaker : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

private:
	int random = 0;
	float volume = 0;
	const char* rsnoise = nullptr;
	float wait = 0;
};

#endif // CRANDOMSPEAKER_H
