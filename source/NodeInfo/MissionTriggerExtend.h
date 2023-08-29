#pragma once
#include "MissionTriggerBase.h"

class MissionTriggerModePlay : public MissionTriggerBase
{
	//vValues 0 : 총 플레이 타임 1 : 상위 모드 ( 0:모두적용, 1:일반전투 2:진영 3:오늘의 모드) 2: 하위 모드 (팀데스, 포탈....)
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerMonsterKill : public MissionTriggerBase
{
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerAllClear : public MissionTriggerBase
{
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerItem : public MissionTriggerBase
{
	//vValues 0 = 아이템 타입 1 = 코드 2 = 수량
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
	virtual BOOL IsComplete(const DWORD dwValue);
};

class MissionTriggerJudge : public MissionTriggerBase
{
	//vValues 0 = 성공 여부 ( 0 == 모두 가능, 1 == 성공, 2 == 실패 일때만)
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerAdd : public MissionTriggerBase
{
	//vValues 0 값을 더하기만 하는 트리거
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerLoginTimeCheck : public MissionTriggerBase
{
	//vValues 0 = 해당 시간 
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};