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

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"

extern DLL_GLOBAL short g_sModelIndexNullModel; //null model index
extern DLL_GLOBAL short g_sModelIndexErrorModel; //error model index
extern DLL_GLOBAL short g_sModelIndexNullSprite; //null sprite index
extern DLL_GLOBAL short g_sModelIndexErrorSprite; //error sprite index
extern DLL_GLOBAL short g_sSoundIndexNullSound; //null sound index

unsigned short CBaseEntityAbstract::PrecacheEvent(int type, const char* psz)
{
	byte* data = LOAD_FILE_FOR_ME((char*)psz, NULL);
	if (data)
	{
		FREE_FILE(data);
		return PRECACHE_EVENT(type, psz);
	}

	ALERT(at_console, "Warning: event \"%s\" not found!\n", psz);
	return PRECACHE_EVENT(type, "events/null.sc");
}

int CBaseEntityAbstract::PrecacheModel(const char* const pszModelName)
{
	if (!pszModelName || !*pszModelName)
	{
		ALERT(at_console, "Warning: modelname not specified\n");
		return g_sModelIndexNullModel; //set null model
	}
	//no need to precacahe brush
	if (pszModelName[0] == '*')
		return 0;

	//verify file exists
	byte* data = LOAD_FILE_FOR_ME(pszModelName, NULL);
	if (data)
	{
		FREE_FILE(data);
		return PRECACHE_MODEL(pszModelName);
	}

	char* ext = UTIL_FileExtension(const_cast<char*>(pszModelName));

	if (FStrEq(ext, "mdl"))
	{
		//this is model
		ALERT(at_console, "Warning: model \"%s\" not found!\n", pszModelName);
		return g_sModelIndexErrorModel;
	}

	if (FStrEq(ext, "spr"))
	{
		//this is sprite
		ALERT(at_console, "Warning: sprite \"%s\" not found!\n", pszModelName);
		return g_sModelIndexErrorSprite;
	}

	//unknown format
	ALERT(at_console, "Warning: invalid name \"%s\"!\n", pszModelName);
	return g_sModelIndexNullModel; //set null model
}

int CBaseEntityAbstract::PrecacheSound(const char* const pszSoundName)
{
	if (!pszSoundName || !*pszSoundName)
		return g_sSoundIndexNullSound; //set null sound

	char path[256];	//g-cont.
	const char* sound = pszSoundName; //sounds from model events can contains a symbol '*'.
				//remove this for sucessfully loading a sound	
	if (sound[0] == '*')
		sound++;	//only for fake path, engine needs this prefix!
	sprintf(path, "sound/%s", sound);

	//verify file exists
	byte* data = LOAD_FILE_FOR_ME(path, NULL);
	if (data)
	{
		FREE_FILE(data);
		return PRECACHE_SOUND(pszSoundName);
	}

	char* ext = UTIL_FileExtension(const_cast<char*>(pszSoundName));
	if (FStrEq(ext, "wav"))
	{
		//this is sound
		ALERT(at_console, "Warning: sound \"%s\" not found!\n", pszSoundName);
		return g_sSoundIndexNullSound; //set null sound
	}

	//unknown format
	ALERT(at_console, "Warning: invalid name \"%s\"!\n", pszSoundName);
	return g_sSoundIndexNullSound; //set null sound
}

void CBaseEntityAbstract::SetModel(const char* const pszModelName)
{
	if (!pszModelName || !(*pszModelName))
	{
		SET_MODEL(edict(), "models/null.mdl");
		return;
	}

	//is this brush model?
	if (pszModelName[0] == '*')
	{
		SET_MODEL(edict(), pszModelName);
		return;
	}

	//verify file exists
	byte* data = LOAD_FILE_FOR_ME(const_cast<char*>(pszModelName), NULL);
	if (data)
	{
		FREE_FILE(data);
		SET_MODEL(edict(), pszModelName);
		return;
	}

	char* ext = UTIL_FileExtension(const_cast<char*>(pszModelName));
	if (FStrEq(ext, "mdl"))
	{
		//this is model
		SET_MODEL(edict(), "models/error.mdl");
	}
	else if (FStrEq(ext, "spr"))
	{
		//this is sprite
		SET_MODEL(edict(), "sprites/error.spr");
	}
	else
	{
		//set null model
		SET_MODEL(edict(), "models/null.mdl");
	}

	SET_MODEL(edict(), pszModelName);
}
