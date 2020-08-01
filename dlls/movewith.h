extern void CheckDesiredList(void);
extern void CheckAssistList(void);

extern void UTIL_DesiredAction(CBaseEntity* pEnt);
extern void UTIL_DesiredThink(CBaseEntity* pEnt);
extern void UTIL_DesiredInfo(CBaseEntity* pEnt);
extern void UTIL_DesiredPostAssist(CBaseEntity* pEnt);
extern void UTIL_AddToAssistList(CBaseEntity* pEnt);
extern void UTIL_MarkForAssist(CBaseEntity* pEnt, BOOL correctSpeed);
extern void UTIL_AssignOrigin(CBaseEntity* pEntity, const Vector vecOrigin);
extern void UTIL_AssignOrigin(CBaseEntity* pEntity, const Vector vecOrigin, BOOL bInitiator);
extern void UTIL_SetVelocity(CBaseEntity* pEnt, const Vector vecSet);
extern void UTIL_AssignAngles(CBaseEntity* pEntity, const Vector vecAngles);
extern void UTIL_AssignAngles(CBaseEntity* pEntity, const Vector vecAngles, BOOL bInitiator);
extern void UTIL_SetAvelocity(CBaseEntity* pEnt, const Vector vecSet);
extern void UTIL_SetMoveWithVelocity(CBaseEntity* pEnt, const Vector vecSet, int loopbreaker);
extern void UTIL_SetMoveWithAvelocity(CBaseEntity* pEnt, const Vector vecSet, int loopbreaker);
extern void UTIL_MergePos(CBaseEntity* pEnt, int loopbreaker = 100);