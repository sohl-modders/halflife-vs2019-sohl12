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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include <time.h>
#include "shake.h"
#include "decals.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "movewith.h"

static unsigned int glSeed = 0;

unsigned int seed_table[256] =
{
	28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
	27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
	26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
	10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
	10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
	18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
	18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
	28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
	31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
	21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
	617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
	18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
	12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
	9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
	29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
	25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};

int g_groupmask = 0;
int g_groupop = 0;

float UTIL_WeaponTimeBase()
{
#if defined( CLIENT_WEAPONS )
	return 0.0;
#else
	return gpGlobals->time;
#endif
}

unsigned int U_Random()
{
	glSeed *= 69069;
	glSeed += seed_table[glSeed & 0xff];
	return (++glSeed & 0x0fffffff);
}

void U_Srand(unsigned int seed)
{
	glSeed = seed_table[seed & 0xff];
}

//LRC
char* COM_FileExtension(char* in)
{
	static char exten[8];
	int i;

	while (*in && *in != '.')
		in++;

	if (!*in)
		return "";

	in++;
	for (i = 0; i < 7 && *in; i++, in++)
		exten[i] = *in;

	exten[i] = 0;
	return exten;
}

/*
=====================
UTIL_SharedRandomLong
=====================
*/
int UTIL_SharedRandomLong(unsigned int seed, int low, int high)
{
	unsigned int range = high - low + 1;
	U_Srand((unsigned int)(high + low + seed));
	if (range != 1)
	{
		int rnum = U_Random();
		int offset = rnum % range;
		return (low + offset);
	}

	return low;
}

/*
=====================
UTIL_SharedRandomFloat
=====================
*/
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high)
{
	unsigned int range = high - low;
	U_Srand((unsigned int)seed + *(unsigned int*)&low + *(unsigned int*)&high);

	U_Random();
	U_Random();

	if (range)
	{
		int tensixrand = U_Random() & 0xFFFFu;
		float offset = float(tensixrand) / 0x10000u;
		return (low + offset * range);
	}

	return low;
}

void UTIL_ParametricRocket(entvars_t* pev, Vector vecOrigin, Vector vecAngles, edict_t* owner)
{
	pev->startpos = vecOrigin;
	// Trace out line to end pos
	TraceResult tr;
	UTIL_MakeVectors(vecAngles);
	UTIL_TraceLine(pev->startpos, pev->startpos + gpGlobals->v_forward * 8192, ignore_monsters, owner, &tr);
	pev->endpos = tr.vecEndPos;

	// Now compute how long it will take based on current velocity
	Vector vecTravel = pev->endpos - pev->startpos;
	float travelTime = 0.0f;
	if (pev->velocity.Length() > 0.0f)
	{
		travelTime = vecTravel.Length() / pev->velocity.Length();
	}

	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + travelTime;
}

// Normal overrides
void UTIL_SetGroupTrace(int groupmask, int op)
{
	g_groupmask = groupmask;
	g_groupop = op;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

void UTIL_UnsetGroupTrace(void)
{
	g_groupmask = 0;
	g_groupop = 0;

	ENGINE_SETGROUPMASK(0, 0);
}

// Smart version, it'll clean itself up when it pops off stack
UTIL_GroupTrace::UTIL_GroupTrace(int groupmask, int op)
{
	m_oldgroupmask = g_groupmask;
	m_oldgroupop = g_groupop;

	g_groupmask = groupmask;
	g_groupop = op;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

UTIL_GroupTrace::~UTIL_GroupTrace(void)
{
	g_groupmask = m_oldgroupmask;
	g_groupop = m_oldgroupop;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

#ifdef	DEBUG
edict_t* DBG_EntOfVars(const entvars_t* pev)
{
	if (pev->pContainingEntity != NULL)
		return pev->pContainingEntity;

	ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
	edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
	if (pent == NULL)
		ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");

	((entvars_t*)pev)->pContainingEntity = pent;
	return pent;
}
#endif //DEBUG


#ifdef	DEBUG
void
DBG_AssertFunction(
	BOOL		fExpr,
	const char* szExpr,
	const char* szFile,
	int			szLine,
	const char* szMessage)
{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != NULL)
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		sprintf(szOut, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_console, szOut);
}
#endif	// DEBUG

BOOL UTIL_GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{
	return g_pGameRules->GetNextBestWeapon(pPlayer, pCurrentWeapon);
}

// ripped this out of the engine
float	UTIL_AngleMod(float a)
{
	a = fmod(a, 360.0f);
	if (a < 0)
		a += 360.0f;

	return a;
}

float UTIL_AngleDiff(float destAngle, float srcAngle)
{
	float delta = destAngle - srcAngle;
	if (destAngle > srcAngle)
	{
		if (delta >= 180.0f)
			delta -= 360.0f;
	}
	else
	{
		if (delta <= -180.0f)
			delta += 360.0f;
	}

	return delta;
}

Vector UTIL_VecToAngles(const Vector& vec)
{
	float rgflVecOut[3];
	VEC_TO_ANGLES(vec, rgflVecOut);
	return Vector(rgflVecOut);
}

//LRC - pass in a normalised axis vector and a number of degrees, and this returns the corresponding
// angles value for an entity.
#ifdef WIN32
inline Vector UTIL_AxisRotationToAngles(const Vector& vecAxis, float flDegs)
#else
// GCC/G++ handle the "inline" keyword differently than MSVC - Shepard
static inline Vector UTIL_AxisRotationToAngles(const Vector& vecAxis, float flDegs)
#endif
{
	Vector vecTemp = UTIL_AxisRotationToVec(vecAxis, flDegs);
	float rgflVecOut[3];
	//ugh, mathsy.
	rgflVecOut[0] = asin(vecTemp.z) * (-180.0 / M_PI);
	rgflVecOut[1] = acos(vecTemp.x) * (180.0 / M_PI);

	if (vecTemp.y < 0)
		rgflVecOut[1] = -rgflVecOut[1];

	rgflVecOut[2] = 0; //for now
	return Vector(rgflVecOut);
}

//LRC - as above, but returns the position of point 1 0 0 under the given rotation
Vector UTIL_AxisRotationToVec(const Vector& vecAxis, float flDegs)
{
	float rgflVecOut[3];
	float flRads = flDegs * (M_PI / 180.0);
	float c = cos(flRads);
	float s = sin(flRads);
	float v = vecAxis.x * (1 - c);

	//ugh, more maths. Thank goodness for internet geometry sites...
	rgflVecOut[0] = vecAxis.x * v + c;
	rgflVecOut[1] = vecAxis.y * v + vecAxis.z * s;
	rgflVecOut[2] = vecAxis.z * v - vecAxis.y * s;
	return Vector(rgflVecOut);
}

//	float UTIL_MoveToOrigin( edict_t *pent, const Vector vecGoal, float flDist, int iMoveType )
void UTIL_MoveToOrigin(edict_t* pent, const Vector& vecGoal, float flDist, int iMoveType)
{
	float rgfl[3];
	vecGoal.CopyToArray(rgfl);
	//	return MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
	MOVE_TO_ORIGIN(pent, rgfl, flDist, iMoveType);
}

int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask)
{
	edict_t* pEdict = INDEXENT(1);
	int count = 0;

	if (!pEdict)
		return count;

	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		if (flagMask && !(pEdict->v.flags & flagMask))	// Does it meet the criteria?
			continue;

		if (mins.x > pEdict->v.absmax.x ||
			mins.y > pEdict->v.absmax.y ||
			mins.z > pEdict->v.absmax.z ||
			maxs.x < pEdict->v.absmin.x ||
			maxs.y < pEdict->v.absmin.y ||
			maxs.z < pEdict->v.absmin.z)
			continue;

		CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		pList[count++] = pEntity;

		if (count >= listMax)
			return count;
	}

	return count;
}

int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius)
{
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);

	int count = 0;
	float radiusSquared = radius * radius;

	if (!pEdict)
		return count;

	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		float delta = center.x - pEdict->v.origin.x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if (delta > radiusSquared)
			continue;
		float distance = delta;

		// Now Y
		delta = center.y - pEdict->v.origin.y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		// Now Z
		delta = center.z - (pEdict->v.absmin.z + pEdict->v.absmax.z) * 0.5;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		pList[count] = pEntity;
		count++;

		if (count >= listMax)
			return count;
	}

	return count;
}

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = nullptr;

	pentEntity = FIND_ENTITY_IN_SPHERE(pentEntity, vecCenter, flRadius);
	if (!FNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);

	return nullptr;
}

CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	for (;;)
	{
		// Don't change this to use UTIL_FindEntityByString!
		pentEntity = FIND_ENTITY_BY_STRING(pentEntity, szKeyword, szValue);

		// if pentEntity (the edict) is null, we're at the end of the entities. Give up.
		if (FNullEnt(pentEntity))
			return NULL;

		// ...but if only pEntity (the classptr) is null, we've just got one dud, so we try again.
		CBaseEntity* pEntity = CBaseEntity::Instance(pentEntity);
		if (pEntity)
			return pEntity;
	}
}

CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "classname", szName);
}

#define MAX_ALIASNAME_LEN 80

//LRC - things get messed up if aliases change in the middle of an entity traversal.
// so instead, they record what changes should be made, and wait until this function gets
// called.
void UTIL_FlushAliases()
{
	//	ALERT(at_console, "Flushing alias list\n");
	if (!g_pWorld)
	{
		ALERT(at_debug, "FlushAliases has no AliasList!\n");
		return;
	}

	while (g_pWorld->m_pFirstAlias)
	{
		if (g_pWorld->m_pFirstAlias->m_iLFlags & LF_ALIASLIST)
		{
			//			ALERT(at_console, "call FlushChanges for %s \"%s\"\n", STRING(g_pWorld->m_pFirstAlias->pev->classname), STRING(g_pWorld->m_pFirstAlias->pev->targetname));
			g_pWorld->m_pFirstAlias->FlushChanges();
			g_pWorld->m_pFirstAlias->m_iLFlags &= ~LF_ALIASLIST;
		}

		g_pWorld->m_pFirstAlias = g_pWorld->m_pFirstAlias->m_pNextAlias;
	}
}

void UTIL_AddToAliasList(CBaseAlias* pAlias)
{
	if (!g_pWorld)
	{
		ALERT(at_debug, "AddToAliasList has no AliasList!\n");
		return;
	}

	pAlias->m_iLFlags |= LF_ALIASLIST;

	//	ALERT(at_console, "Adding %s \"%s\" to alias list\n", STRING(pAlias->pev->classname), STRING(pAlias->pev->targetname));
	if (g_pWorld->m_pFirstAlias == NULL)
	{
		g_pWorld->m_pFirstAlias = pAlias;
		pAlias->m_pNextAlias = NULL;
	}
	else if (g_pWorld->m_pFirstAlias == pAlias)
	{
		// already in the list
		return;
	}
	else
	{
		CBaseAlias* pCurrent = g_pWorld->m_pFirstAlias;
		while (pCurrent->m_pNextAlias != NULL)
		{
			if (pCurrent->m_pNextAlias == pAlias)
			{
				// already in the list
				return;
			}

			pCurrent = pCurrent->m_pNextAlias;
		}

		pCurrent->m_pNextAlias = pAlias;
		pAlias->m_pNextAlias = NULL;
	}
}

// for every alias which has the given name, find the earliest entity which any of them refers to
// and which is later than pStartEntity.
CBaseEntity* UTIL_FollowAliasReference(CBaseEntity* pStartEntity, const char* szValue)
{
	CBaseEntity* pBestEntity = NULL; // the entity we're currently planning to return.
	int iBestOffset = -1; // the offset of that entity.

	CBaseEntity* pEntity = UTIL_FindEntityByTargetname(NULL, szValue);

	while (pEntity)
	{
		if (pEntity->IsAlias())
		{
			CBaseEntity* pTempEntity = ((CBaseAlias*)pEntity)->FollowAlias(pStartEntity);
			if (pTempEntity)
			{
				// We've found an entity; only use it if its offset is lower than the offset we've currently got.
				int iTempOffset = OFFSET(pTempEntity->pev);
				if (iBestOffset == -1 || iTempOffset < iBestOffset)
				{
					iBestOffset = iTempOffset;
					pBestEntity = pTempEntity;
				}
			}
		}
		pEntity = UTIL_FindEntityByTargetname(pEntity, szValue);
	}

	return pBestEntity;
}

// for every info_group which has the given groupname, find the earliest entity which is referred to by its member
// with the given membername and which is later than pStartEntity.
CBaseEntity* UTIL_FollowGroupReference(CBaseEntity* pStartEntity, char* szGroupName, char* szMemberName)
{
	CBaseEntity* pBestEntity = NULL; // the entity we're currently planning to return.
	int iBestOffset = -1; // the offset of that entity.
	CBaseEntity* pTempEntity;
	char szBuf[MAX_ALIASNAME_LEN];
	char* szThisMember = szMemberName;
	char* szTail = NULL;

	// find the first '.' in the membername and if there is one, split the string at that point.
	for (int i = 0; szMemberName[i]; i++)
	{
		if (szMemberName[i] == '.')
		{
			// recursive member-reference
			// FIXME: we should probably check that i < MAX_ALIASNAME_LEN.
			strncpy(szBuf, szMemberName, i);
			szBuf[i] = 0;
			szTail = &(szMemberName[i + 1]);
			szThisMember = szBuf;
			break;
		}
	}

	CBaseEntity* pEntity = UTIL_FindEntityByTargetname(NULL, szGroupName);
	while (pEntity)
	{
		if (FStrEq(STRING(pEntity->pev->classname), "info_group"))
		{
			int iszMemberValue = ((CInfoGroup*)pEntity)->GetMember(szThisMember);
			//			ALERT(at_console,"survived getMember\n");
			//			return NULL;
			if (!FStringNull(iszMemberValue))
			{
				if (szTail) // do we have more references to follow?
					pTempEntity = UTIL_FollowGroupReference(pStartEntity, (char*)STRING(iszMemberValue), szTail);
				else
					pTempEntity = UTIL_FindEntityByTargetname(pStartEntity, STRING(iszMemberValue));

				if (pTempEntity)
				{
					int iTempOffset = OFFSET(pTempEntity->pev);
					if (iBestOffset == -1 || iTempOffset < iBestOffset)
					{
						iBestOffset = iTempOffset;
						pBestEntity = pTempEntity;
					}
				}
			}
		}
		pEntity = UTIL_FindEntityByTargetname(pEntity, szGroupName);
	}

	if (pBestEntity)
	{
		//		ALERT(at_console,"\"%s\".\"%s\" returns %s\n",szGroupName,szMemberName,STRING(pBestEntity->pev->targetname));
		return pBestEntity;
	}

	return NULL;
}

// Returns the first entity which szName refers to and which is after pStartEntity.
CBaseEntity* UTIL_FollowReference(CBaseEntity* pStartEntity, const char* szName)
{
	char szRoot[MAX_ALIASNAME_LEN + 1]; // allow room for null-terminator
	char* szMember;
	int i;
	CBaseEntity* pResult;

	if (!szName || szName[0] == 0) return NULL;

	// reference through an info_group?
	for (i = 0; szName[i]; i++)
	{
		if (szName[i] == '.')
		{
			// yes, it looks like a reference through an info_group...
			// FIXME: we should probably check that i < MAX_ALIASNAME_LEN.
			strncpy(szRoot, szName, i);
			szRoot[i] = 0;
			szMember = (char*)&szName[i + 1];
			//ALERT(at_console,"Following reference- group %s with member %s\n",szRoot,szMember);
			pResult = UTIL_FollowGroupReference(pStartEntity, szRoot, szMember);
			//			if (pResult)
			//				ALERT(at_console,"\"%s\".\"%s\" = %s\n",szRoot,szMember,STRING(pResult->pev->targetname));
			return pResult;
		}
	}
	// reference through an info_alias?
	if (szName[0] == '*')
	{
		if (FStrEq(szName, "*player"))
		{
			CBaseEntity* pPlayer = UTIL_FindEntityByClassname(NULL, "player");
			if (pPlayer && (pStartEntity == NULL || pPlayer->eoffset() > pStartEntity->eoffset()))
				return pPlayer;

			return NULL;
		}
		//ALERT(at_console,"Following alias %s\n",szName+1);
		pResult = UTIL_FollowAliasReference(pStartEntity, szName + 1);
		//		if (pResult)
		//			ALERT(at_console,"alias \"%s\" = %s\n",szName+1,STRING(pResult->pev->targetname));
		return pResult;
	}
	// not a reference
//	ALERT(at_console,"%s is not a reference\n",szName);
	return NULL;
}

CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName)
{
	CBaseEntity* pFound = UTIL_FollowReference(pStartEntity, szName);
	if (pFound)
		return pFound;

	return UTIL_FindEntityByString(pStartEntity, "targetname", szName);
}

CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName, CBaseEntity* pActivator)
{
	if (FStrEq(szName, "*locus"))
	{
		if (pActivator && (pStartEntity == NULL || pActivator->eoffset() > pStartEntity->eoffset()))
			return pActivator;

		return NULL;
	}

	return UTIL_FindEntityByTargetname(pStartEntity, szName);
}

CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "target", szName);
}

CBaseEntity* UTIL_FindEntityGeneric(const char* szWhatever, Vector& vecSrc, float flRadius)
{
	CBaseEntity* pEntity = UTIL_FindEntityByTargetname(NULL, szWhatever);
	if (pEntity)
		return pEntity;

	CBaseEntity* pSearch = NULL;
	float flMaxDist2 = flRadius * flRadius;
	while ((pSearch = UTIL_FindEntityByClassname(pSearch, szWhatever)) != NULL)
	{
		float flDist2 = (pSearch->pev->origin - vecSrc).Length();
		flDist2 = flDist2 * flDist2;
		if (flMaxDist2 > flDist2)
		{
			pEntity = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
CBasePlayer* UTIL_PlayerByIndex(int playerIndex)
{
	CBasePlayer* pPlayer = nullptr;

	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		edict_t* pPlayerEdict = INDEXENT(playerIndex);
		if (pPlayerEdict && !pPlayerEdict->free)
			pPlayer = dynamic_cast<CBasePlayer*>(CBaseEntity::Instance(pPlayerEdict));
	}

	return pPlayer;
}

void UTIL_MakeVectors(const Vector& vecAngles)
{
	MAKE_VECTORS(vecAngles);
}

void UTIL_MakeAimVectors(const Vector& vecAngles)
{
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	MAKE_VECTORS(rgflVec);
}

#define SWAP(a,b,temp)	((temp)=(a),(a)=(b),(b)=(temp))

void UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv)
{
	MAKE_VECTORS(vec);

	float tmp;
	pgv->v_right = pgv->v_right * -1;

	SWAP(pgv->v_forward.y, pgv->v_right.x, tmp);
	SWAP(pgv->v_forward.z, pgv->v_up.x, tmp);
	SWAP(pgv->v_right.z, pgv->v_up.y, tmp);
}


void UTIL_EmitAmbientSound(edict_t* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch)
{
	float rgfl[3];
	vecOrigin.CopyToArray(rgfl);

	if (samp && *samp == '!')
	{
		char name[32];
		if (SENTENCEG_Lookup(samp, name) >= 0)
			EMIT_AMBIENT_SOUND(entity, rgfl, name, vol, attenuation, fFlags, pitch);
	}
	else
		EMIT_AMBIENT_SOUND(entity, rgfl, samp, vol, attenuation, fFlags, pitch);
}

static unsigned short FixedUnsigned16(float value, float scale)
{
	int output;

	output = value * scale;
	if (output < 0)
		output = 0;
	if (output > 0xFFFF)
		output = 0xFFFF;

	return (unsigned short)output;
}

static short FixedSigned16(float value, float scale)
{
	int output;

	output = value * scale;

	if (output > 32767)
		output = 32767;

	if (output < -32768)
		output = -32768;

	return (short)output;
}

// Shake the screen of all clients within radius
// radius == 0, shake all clients
// UNDONE: Allow caller to shake clients not ONGROUND?
// UNDONE: Fix falloff model (disabled)?
// UNDONE: Affect user controls?
//LRC UNDONE: Work during trigger_camera?
void UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius)
{
	int			i;
	float		localAmplitude;
	ScreenShake	shake;

	shake.duration = FixedUnsigned16(duration, 1 << 12);		// 4.12 fixed
	shake.frequency = FixedUnsigned16(frequency, 1 << 8);	// 8.8 fixed

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer || !(pPlayer->pev->flags & FL_ONGROUND))	// Don't shake if not onground
			continue;

		localAmplitude = 0;

		if (radius <= 0)
			localAmplitude = amplitude;
		else
		{
			Vector delta = center - pPlayer->pev->origin;
			float distance = delta.Length();

			// Had to get rid of this falloff - it didn't work well
			if (distance < radius)
				localAmplitude = amplitude;//radius - distance;
		}
		if (localAmplitude)
		{
			shake.amplitude = FixedUnsigned16(localAmplitude, 1 << 12);		// 4.12 fixed

			MESSAGE_BEGIN(MSG_ONE, gmsgShake, NULL, pPlayer->edict());		// use the magic #1 for "one client"

			WRITE_SHORT(shake.amplitude);				// shake amount
			WRITE_SHORT(shake.duration);				// shake lasts this long
			WRITE_SHORT(shake.frequency);				// shake noise frequency

			MESSAGE_END();
		}
	}
}

void UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration)
{
	UTIL_ScreenShake(center, amplitude, frequency, duration, 0);
}

void UTIL_ScreenFadeBuild(ScreenFade& fade, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	fade.duration = FixedUnsigned16(fadeTime, 1 << 12);		// 4.12 fixed
	fade.holdTime = FixedUnsigned16(fadeHold, 1 << 12);		// 4.12 fixed
	fade.r = (int)color.x;
	fade.g = (int)color.y;
	fade.b = (int)color.z;
	fade.a = alpha;
	fade.fadeFlags = flags;
}

void UTIL_ScreenFadeWrite(const ScreenFade& fade, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, pEntity->edict());		// use the magic #1 for "one client"

	WRITE_SHORT(fade.duration);		// fade lasts this long
	WRITE_SHORT(fade.holdTime);		// fade lasts this long
	WRITE_SHORT(fade.fadeFlags);		// fade type (in / out)
	WRITE_BYTE(fade.r);				// fade red
	WRITE_BYTE(fade.g);				// fade green
	WRITE_BYTE(fade.b);				// fade blue
	WRITE_BYTE(fade.a);				// fade blue

	MESSAGE_END();
}

static int CalculateFadeAlpha(const Vector& fadeSource, CBaseEntity* pEntity, int baseAlpha)
{
	UTIL_MakeVectors(pEntity->pev->v_angle);
	const Vector a = gpGlobals->v_forward;
	const Vector b = (fadeSource - (pEntity->pev->origin + pEntity->pev->view_ofs)).Normalize();
	const float dot = DotProduct(a, b);
	if (dot >= 0)
		return (int)(baseAlpha * min(dot + 0.134, 1.0));

	return 0;
}

void UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	ScreenFade	fade;

	UTIL_ScreenFadeBuild(fade, color, fadeTime, fadeHold, alpha, flags);

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		UTIL_ScreenFadeWrite(fade, pPlayer);
	}
}

void UTIL_ScreenFadeAll(const Vector& fadeSource, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			UTIL_ScreenFade(fadeSource, pPlayer, color, fadeTime, fadeHold, alpha, flags);
		}
	}
}

void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	ScreenFade	fade;

	UTIL_ScreenFadeBuild(fade, color, fadeTime, fadeHold, alpha, flags);
	UTIL_ScreenFadeWrite(fade, pEntity);
}

void UTIL_ScreenFade(const Vector& fadeSource, CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	alpha = CalculateFadeAlpha(fadeSource, pEntity, alpha);
	if (alpha > 0)
		UTIL_ScreenFade(pEntity, color, fadeTime, fadeHold, alpha, flags);
}

void UTIL_HudMessage(CBaseEntity* pEntity, const hudtextparms_t& textparms, const char* pMessage)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pEntity->edict());
	WRITE_BYTE(TE_TEXTMESSAGE);
	WRITE_BYTE(textparms.channel & 0xFF);

	WRITE_SHORT(FixedSigned16(textparms.x, 1 << 13));
	WRITE_SHORT(FixedSigned16(textparms.y, 1 << 13));
	WRITE_BYTE(textparms.effect);

	WRITE_BYTE(textparms.r1);
	WRITE_BYTE(textparms.g1);
	WRITE_BYTE(textparms.b1);
	WRITE_BYTE(textparms.a1);

	WRITE_BYTE(textparms.r2);
	WRITE_BYTE(textparms.g2);
	WRITE_BYTE(textparms.b2);
	WRITE_BYTE(textparms.a2);

	WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.holdTime, 1 << 8));

	if (textparms.effect == 2)
		WRITE_SHORT(FixedUnsigned16(textparms.fxTime, 1 << 8));

	if (strlen(pMessage) < 512)
	{
		WRITE_STRING(pMessage);
	}
	else
	{
		char tmp[512];
		strncpy(tmp, pMessage, 511);
		tmp[511] = 0;
		WRITE_STRING(tmp);
	}
	MESSAGE_END();
}

void UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
			UTIL_HudMessage(pPlayer, textparms, pMessage);
	}
}

extern int gmsgTextMsg, gmsgSayText;
void UTIL_ClientPrintAll(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgTextMsg);
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);

	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);

	MESSAGE_END();
}

void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgTextMsg, NULL, client);
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);

	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);

	MESSAGE_END();
}

void UTIL_SayText(const char* pText, CBaseEntity* pEntity)
{
	if (!pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity->edict());
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}

void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}

char* UTIL_dtos(const int iValue)
{
	//This buffer size calculation determines the number of characters needed for an int, plus a null terminator.
	//See http://stackoverflow.com/questions/3919995/determining-sprintf-buffer-size-whats-the-standard/3920025#3920025
	//The old buffer size used by the SDK functions was 8.
	static char szBuffers[NUM_STATIC_BUFFERS][(((sizeof(int) * CHAR_BIT) / 3) + 3) + 1];
	static size_t uiBufferIndex = 0;
	uiBufferIndex = (uiBufferIndex + 1) % NUM_STATIC_BUFFERS;
	snprintf(szBuffers[uiBufferIndex], sizeof(szBuffers[uiBufferIndex]), "%d", iValue);
	return szBuffers[uiBufferIndex];
}

void UTIL_ShowMessage(const char* pString, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgHudText, NULL, pEntity->edict());
	WRITE_STRING(pString);
	MESSAGE_END();
}

void UTIL_ShowMessageAll(const char* pString)
{
	// loop through all players

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
			UTIL_ShowMessage(pString, pPlayer);
	}
}

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_LINE(vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass ? 0x100 : 0), pentIgnore, ptr);
}

void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_LINE(vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr);
}

void UTIL_TraceHull(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_HULL(vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr);
}

void UTIL_TraceModel(const Vector& vecStart, const Vector& vecEnd, int hullNumber, edict_t* pentModel, TraceResult* ptr)
{
	g_engfuncs.pfnTraceModel(vecStart, vecEnd, hullNumber, pentModel, ptr);
}

TraceResult UTIL_GetGlobalTrace()
{
	TraceResult tr;

	tr.fAllSolid = gpGlobals->trace_allsolid;
	tr.fStartSolid = gpGlobals->trace_startsolid;
	tr.fInOpen = gpGlobals->trace_inopen;
	tr.fInWater = gpGlobals->trace_inwater;
	tr.flFraction = gpGlobals->trace_fraction;
	tr.flPlaneDist = gpGlobals->trace_plane_dist;
	tr.pHit = gpGlobals->trace_ent;
	tr.vecEndPos = gpGlobals->trace_endpos;
	tr.vecPlaneNormal = gpGlobals->trace_plane_normal;
	tr.iHitgroup = gpGlobals->trace_hitgroup;
	return tr;
}

void UTIL_SetSize(entvars_t* pev, const Vector& vecMin, const Vector& vecMax)
{
	SET_SIZE(ENT(pev), vecMin, vecMax);
}

float UTIL_VecToYaw(const Vector& vec)
{
	return VEC_TO_YAW(vec);
}

void UTIL_SetEdictOrigin(edict_t* pEdict, const Vector& vecOrigin)
{
	if (pEdict)
		SET_ORIGIN(pEdict, vecOrigin);
}

// 'links' the entity into the world
void UTIL_SetOrigin(CBaseEntity* pEntity, const Vector& vecOrigin)
{
	UTIL_SetOrigin(pEntity->pev, vecOrigin);
}

void UTIL_SetOrigin(entvars_t* pev, const Vector& vecOrigin)
{
	edict_t* ent = ENT(pev);
	if (ent)
		SET_ORIGIN(ent, vecOrigin);
}

void UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, ULONG ulColor, ULONG ulCount)
{
	PARTICLE_EFFECT(vecOrigin, vecDirection, (float)ulColor, (float)ulCount);
}

float UTIL_Approach(float target, float value, float speed)
{
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_ApproachAngle(float target, float value, float speed)
{
	target = UTIL_AngleMod(target);
	value = UTIL_AngleMod(value);

	float delta = target - value;

	// Speed is assumed to be positive
	if (speed < 0.0f)
		speed = -speed;

	if (delta < -180.0f)
		delta += 360.0f;
	else if (delta > 180.0f)
		delta -= 360.0f;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

float UTIL_AngleDistance(float next, float cur)
{
	float delta = next - cur;

	if (delta < -180.0f)
		delta += 360.0f;

	else if (delta > 180.0f)
		delta -= 360.0f;

	return delta;
}

float UTIL_SplineFraction(float value, float scale)
{
	float valueSquared = value * scale;
	return 3.0f * (valueSquared * valueSquared) - (valueSquared * valueSquared * valueSquared + valueSquared * valueSquared * valueSquared);
}

char* UTIL_VarArgs(const char* format, ...)
{
	static char szBuffers[NUM_STATIC_BUFFERS][1024];
	static size_t uiBufferIndex = 0;

	va_list	argptr;

	uiBufferIndex = (uiBufferIndex + 1) % NUM_STATIC_BUFFERS;

	va_start(argptr, format);
	vsprintf(szBuffers[uiBufferIndex], format, argptr);
	va_end(argptr);

	return szBuffers[uiBufferIndex];
}

Vector UTIL_GetAimVector(edict_t* pent, float flSpeed)
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}

BOOL UTIL_IsMasterTriggered(string_t iszMaster, CBaseEntity* pActivator)
{
	int i, j, found = false;
	char szBuf[80];
	int reverse = false;

	if (iszMaster)
	{
		//		ALERT(at_console, "IsMasterTriggered(%s, %s \"%s\")\n", STRING(iszMaster), STRING(pActivator->pev->classname), STRING(pActivator->pev->targetname));
		const char* szMaster = STRING(iszMaster);
		if (szMaster[0] == '~') //inverse master
		{
			reverse = true;
			szMaster++;
		}

		CBaseEntity* pMaster = UTIL_FindEntityByTargetname(NULL, szMaster);
		if (!pMaster)
		{
			for (i = 0; szMaster[i]; i++)
			{
				if (szMaster[i] == '(')
				{
					for (j = i + 1; szMaster[j]; j++)
					{
						if (szMaster[j] == ')')
						{
							strncpy(szBuf, szMaster + i + 1, (j - i) - 1);
							szBuf[(j - i) - 1] = 0;
							pActivator = UTIL_FindEntityByTargetname(NULL, szBuf);
							found = true;
							break;
						}
					}

					if (!found) // no ) found
					{
						ALERT(at_error, "Missing ')' in master \"%s\"\n", szMaster);
						return FALSE;
					}
					break;
				}
			}

			if (!found) // no ( found
			{
				ALERT(at_debug, "Master \"%s\" not found!\n", szMaster);
				return TRUE;
			}

			strncpy(szBuf, szMaster, i);
			szBuf[i] = 0;
			pMaster = UTIL_FindEntityByTargetname(NULL, szBuf);
		}

		if (pMaster)
		{
			if (reverse)
				return (pMaster->GetState(pActivator) != STATE_ON);

			return (pMaster->GetState(pActivator) == STATE_ON);
		}
	}

	// if the entity has no master (or the master is missing), just say yes.
	return TRUE;
}

bool UTIL_TargetnameIsActivator(const char* targetName)
{
	return targetName && (FStrEq(targetName, "*locus") || FStrEq(targetName, "!activator"));
}

bool UTIL_TargetnameIsActivator(string_t targetName)
{
	if (FStringNull(targetName))
		return false;

	return UTIL_TargetnameIsActivator(STRING(targetName));
}

BOOL UTIL_ShouldShowBlood(int color)
{
	if (color != DONT_BLEED)
	{
		if (color == BLOOD_COLOR_RED)
		{
			if (CVAR_GET_FLOAT("violence_hblood") != 0)
				return TRUE;
		}
		else
		{
			if (CVAR_GET_FLOAT("violence_ablood") != 0)
				return TRUE;
		}
	}
	return FALSE;
}

int UTIL_PointContents(const Vector& vec)
{
	return POINT_CONTENTS(vec);
}

void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x);
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);
	WRITE_BYTE(V_min(amount, 255));
	MESSAGE_END();
}

void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (color == DONT_BLEED || amount == 0)
		return;

	if (g_pGameRules->IsMultiplayer())
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if (amount > 255)
		amount = 255;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSPRITE);
	WRITE_COORD(origin.x);								// pos
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(g_sModelIndexBloodSpray);				// initial sprite model
	WRITE_SHORT(g_sModelIndexBloodDrop);				// droplet sprite models
	WRITE_BYTE(color);								// color index into host_basepal
	WRITE_BYTE(V_min(V_max(3, amount / 10), 16));		// size
	MESSAGE_END();
}

Vector UTIL_RandomBloodVector(void)
{
	Vector direction;

	direction.x = RANDOM_FLOAT(-1, 1);
	direction.y = RANDOM_FLOAT(-1, 1);
	direction.z = RANDOM_FLOAT(0, 1);

	return direction;
}

void UTIL_BloodDecalTrace(TraceResult* pTrace, int bloodColor)
{
	if (UTIL_ShouldShowBlood(bloodColor))
	{
		if (bloodColor == BLOOD_COLOR_RED)
			UTIL_DecalTrace(pTrace, DECAL_BLOOD1 + RANDOM_LONG(0, 5));
		else
			UTIL_DecalTrace(pTrace, DECAL_YBLOOD1 + RANDOM_LONG(0, 5));
	}
}

void UTIL_DecalTrace(TraceResult* pTrace, int decalNumber)
{
	short entityIndex;

	if (decalNumber < 0)
		return;

	int index = gDecals[decalNumber].index;

	if (index < 0)
		return;

	if (pTrace->flFraction == 1.0)
		return;

	// Only decal BSP models
	if (pTrace->pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(pTrace->pHit);
		if (pEntity && !pEntity->IsBSPModel())
			return;
		entityIndex = ENTINDEX(pTrace->pHit);
	}
	else
		entityIndex = 0;

	int message = TE_DECAL;
	if (entityIndex != 0)
	{
		if (index > 255)
		{
			message = TE_DECALHIGH;
			index -= 256;
		}
	}
	else
	{
		message = TE_WORLDDECAL;
		if (index > 255)
		{
			message = TE_WORLDDECALHIGH;
			index -= 256;
		}
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(message);
	WRITE_COORD(pTrace->vecEndPos.x);
	WRITE_COORD(pTrace->vecEndPos.y);
	WRITE_COORD(pTrace->vecEndPos.z);
	WRITE_BYTE(index);
	if (entityIndex)
		WRITE_SHORT(entityIndex);
	MESSAGE_END();
}

/*
==============
UTIL_PlayerDecalTrace

A player is trying to apply his custom decal for the spray can.
Tell connected clients to display it, or use the default spray can decal
if the custom can't be loaded.
==============
*/
void UTIL_PlayerDecalTrace(TraceResult* pTrace, int playernum, int decalNumber, BOOL bIsCustom)
{
	int index;

	if (!bIsCustom)
	{
		if (decalNumber < 0)
			return;

		index = gDecals[decalNumber].index;
		if (index < 0)
			return;
	}
	else
		index = decalNumber;

	if (pTrace->flFraction != 1.0f)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_PLAYERDECAL);
		WRITE_BYTE(playernum);
		WRITE_COORD(pTrace->vecEndPos.x);
		WRITE_COORD(pTrace->vecEndPos.y);
		WRITE_COORD(pTrace->vecEndPos.z);
		WRITE_SHORT(int(ENTINDEX(pTrace->pHit)));
		WRITE_BYTE(index);
		MESSAGE_END();
	}
}

void UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber)
{
	if (decalNumber < 0)
		return;

	int index = gDecals[decalNumber].index;
	if (index < 0)
		return;

	if (pTrace->flFraction == 1.0)
		return;

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pTrace->vecEndPos);
	WRITE_BYTE(TE_GUNSHOTDECAL);
	WRITE_COORD(pTrace->vecEndPos.x);
	WRITE_COORD(pTrace->vecEndPos.y);
	WRITE_COORD(pTrace->vecEndPos.z);
	WRITE_SHORT((short)ENTINDEX(pTrace->pHit));
	WRITE_BYTE(index);
	MESSAGE_END();
}

void UTIL_Sparks(const Vector& position)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_SPARKS);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	MESSAGE_END();
}

void UTIL_Ricochet(const Vector& position, float scale)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_ARMOR_RICOCHET);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_BYTE((int)(scale * 10));
	MESSAGE_END();
}

BOOL UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2)
{
	// Everyone matches unless it's teamplay
	if (!g_pGameRules->IsTeamplay())
		return TRUE;

	// Both on a team?
	if (*pTeamName1 != 0 && *pTeamName2 != 0)
	{
		if (!stricmp(pTeamName1, pTeamName2))	// Same Team?
			return TRUE;
	}

	return FALSE;
}

//LRC - moved here from barney.cpp
bool UTIL_IsFacing(entvars_t* pevTest, const Vector& reference)
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward;
	Vector angle = pevTest->v_angle;
	angle.x = 0;

	UTIL_MakeVectorsPrivate(angle, forward, nullptr, nullptr);
	// He's facing me, he meant it
	if (DotProduct(forward, vecDir) > 0.96)	// +/- 15 degrees or so
		return true;

	return false;
}

//LRC - randomized vectors of the form "0 0 0 .. 1 0 0"
void UTIL_StringToVector(float* pVector, const char* pString)
{
	char* pstr, * pfront, tempString[128];
	int	j;
	float pAltVec[3];

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < 3; j++)			// lifted from pr_edict.c
	{
		pVector[j] = atof(pfront);

		while (*pstr && *pstr != ' ') pstr++;
		if (!*pstr) break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j + 1; j < 3; j++)
			pVector[j] = 0;
	}
	else if (*pstr == '.')
	{
		pstr++;
		if (*pstr != '.') return;
		pstr++;
		if (*pstr != ' ') return;

		UTIL_StringToVector(pAltVec, pstr);

		pVector[0] = RANDOM_FLOAT(pVector[0], pAltVec[0]);
		pVector[1] = RANDOM_FLOAT(pVector[1], pAltVec[1]);
		pVector[2] = RANDOM_FLOAT(pVector[2], pAltVec[2]);
	}
}

//LRC - randomized vectors of the form "0 0 0 .. 1 0 0"
void UTIL_StringToRandomVector(float* pVector, const char* pString)
{
	char* pstr, * pfront, tempString[128];
	int	j;
	float pAltVec[3];

	strcpy(tempString, pString);
	pstr = pfront = tempString;

	for (j = 0; j < 3; j++)			// lifted from pr_edict.c
	{
		pVector[j] = atof(pfront);

		while (*pstr && *pstr != ' ') pstr++;
		if (!*pstr) break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j + 1; j < 3; j++)
			pVector[j] = 0;
	}
	else if (*pstr == '.')
	{
		pstr++;
		if (*pstr != '.') return;
		pstr++;
		if (*pstr != ' ') return;

		UTIL_StringToVector(pAltVec, pstr);

		pVector[0] = RANDOM_FLOAT(pVector[0], pAltVec[0]);
		pVector[1] = RANDOM_FLOAT(pVector[1], pAltVec[1]);
		pVector[2] = RANDOM_FLOAT(pVector[2], pAltVec[2]);
	}
}


void UTIL_StringToIntArray(int* pVector, int count, const char* pString)
{
	char* pfront, tempString[128];
	int	j;

	strcpy(tempString, pString);
	char* pstr = pfront = tempString;

	for (j = 0; j < count; j++)			// lifted from pr_edict.c
	{
		pVector[j] = atoi(pfront);

		while (*pstr && *pstr != ' ')
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}

	for (j++; j < count; j++)
	{
		pVector[j] = 0;
	}
}

Vector UTIL_ClampVectorToBox(const Vector& input, const Vector& clampSize)
{
	Vector sourceVector = input;

	if (sourceVector.x > clampSize.x)
		sourceVector.x -= clampSize.x;
	else if (sourceVector.x < -clampSize.x)
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if (sourceVector.y > clampSize.y)
		sourceVector.y -= clampSize.y;
	else if (sourceVector.y < -clampSize.y)
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;

	if (sourceVector.z > clampSize.z)
		sourceVector.z -= clampSize.z;
	else if (sourceVector.z < -clampSize.z)
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector.Normalize();
}


float UTIL_WaterLevel(const Vector& position, float minz, float maxz)
{
	Vector midUp = position;
	midUp.z = minz;

	if (UTIL_PointContents(midUp) != CONTENTS_WATER)
		return minz;

	midUp.z = maxz;
	if (UTIL_PointContents(midUp) == CONTENTS_WATER)
		return maxz;

	float diff = maxz - minz;
	while (diff > 1.0)
	{
		midUp.z = minz + diff / 2.0;
		if (UTIL_PointContents(midUp) == CONTENTS_WATER)
		{
			minz = midUp.z;
		}
		else
		{
			maxz = midUp.z;
		}
		diff = maxz - minz;
	}

	return midUp.z;
}


extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model

void UTIL_Bubbles(Vector mins, Vector maxs, int count)
{
	Vector mid = (mins + maxs) * 0.5f;
	float flHeight = UTIL_WaterLevel(mid, mid.z, mid.z + 1024.0f) - mins.z;

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, mid);
	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(mins.x);	// mins
	WRITE_COORD(mins.y);
	WRITE_COORD(mins.z);
	WRITE_COORD(maxs.x);	// maxz
	WRITE_COORD(maxs.y);
	WRITE_COORD(maxs.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}

void UTIL_BubbleTrail(Vector from, Vector to, int count)
{
	float flHeight = UTIL_WaterLevel(from, from.z, from.z + 256);
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel(to, to.z, to.z + 256);
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255)
		count = 255;

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(from.x);	// mins
	WRITE_COORD(from.y);
	WRITE_COORD(from.z);
	WRITE_COORD(to.x);	// maxz
	WRITE_COORD(to.y);
	WRITE_COORD(to.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}

void UTIL_Remove(CBaseEntity* pEntity)
{
	if (!pEntity)
		return;

	if (pEntity->pev == VARS(eoNullEntity) || pEntity->IsPlayer() || 
		(pEntity->pev->flags & FL_KILLME) == FL_KILLME)
		return;

	pEntity->UpdateOnRemove();
	pEntity->pev->solid = SOLID_NOT;
	pEntity->pev->flags |= FL_KILLME;
	pEntity->pev->targetname = 0;
}

BOOL UTIL_IsValidEntity(edict_t* pent)
{
	if (!pent || pent->free || (pent->v.flags & FL_KILLME))
		return FALSE;
	return TRUE;
}

void UTIL_PrecacheOther(const char* szClassname)
{
	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));
	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOther\n");
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::Instance(VARS(pent));
	if (pEntity)
		pEntity->Precache();

	REMOVE_ENTITY(pent);
}

//=========================================================
// UTIL_LogPrintf - Prints a logged message to console.
// Preceded by LOG: ( timestamp ) < message >
//=========================================================
void UTIL_LogPrintf(const char* fmt, ...)
{
	va_list			argptr;
	static char		string[1024];

	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	// Print to server console
	ALERT(at_logged, "%s", string);
}

void UTIL_ServerPrintf(const char* const pszFormat, ...)
{
	va_list			argptr;
	static char		string[1024];

	va_start(argptr, pszFormat);
	vsprintf(string, pszFormat, argptr);
	va_end(argptr);

	// Print to server console
	g_engfuncs.pfnServerPrint(string);
}

void UTIL_Log(const char* fmt, ...)
{
	va_list ap;
	static char string[1024];

	va_start(ap, fmt);
	vsnprintf(string, sizeof(string), fmt, ap);
	va_end(ap);

	if (strlen(string) < sizeof(string) - 2)
		strcat(string, "\n");
	else
		string[strlen(string) - 1] = '\n';

	FILE* fp = fopen("spirit.log", "at");
	if (fp)
	{
		fprintf(fp, "%s", string);
		fclose(fp);
	}
}

//=========================================================
// UTIL_DotPoints - returns the dot product of a line from
// src to check and vecdir.
//=========================================================
float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir)
{
	Vector2D vec2LOS = (vecCheck - vecSrc).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct(vec2LOS, (vecDir.Make2D()));
}


//=========================================================
// UTIL_StripToken - for redundant keynames
//=========================================================
void UTIL_StripToken(const char* pKey, char* pDest)
{
	int i = 0;

	while (pKey[i] && pKey[i] != '#')
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
}

void Cvar_DirectSet(cvar_t* pCvar, const float flValue)
{
	g_engfuncs.pfnCvar_DirectSet(pCvar, UTIL_VarArgs("%f", flValue));
}

char* GetStringForUseType(USE_TYPE useType)
{
	switch (useType)
	{
	case USE_ON: return "USE_ON";
	case USE_OFF: return "USE_OFF";
	case USE_TOGGLE: return "USE_TOGGLE";
	case USE_KILL: return "USE_KILL";
	case USE_NOT: return "USE_NOT";
	default:
		return "USE_UNKNOWN!?";
	}
}

char* GetStringForState(STATE state)
{
	switch (state)
	{
	case STATE_ON: return "ON";
	case STATE_OFF: return "OFF";
	case STATE_TURN_ON: return "TURN ON";
	case STATE_TURN_OFF: return "TURN OFF";
	case STATE_IN_USE: return "IN USE";
	default:
		return "STATE_UNKNOWN!?";
	}
}

void UTIL_MuzzleLight(Vector vecSrc, float flRadius, byte r, byte g, byte b, float flTime, float flDecay)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecSrc.x);	// X
	WRITE_COORD(vecSrc.y);	// Y
	WRITE_COORD(vecSrc.z);	// Z
	WRITE_BYTE(flRadius * 0.1f);	// radius * 0.1
	WRITE_BYTE(r);		// r
	WRITE_BYTE(g);		// g
	WRITE_BYTE(b);		// b
	WRITE_BYTE(flTime * 10.0f);	// time * 10
	WRITE_BYTE(flDecay * 0.1f);	// decay * 0.1
	MESSAGE_END();
}