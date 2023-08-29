#pragma once
#include "MissionTriggerBase.h"

class MissionTriggerModePlay : public MissionTriggerBase
{
	//vValues 0 : �� �÷��� Ÿ�� 1 : ���� ��� ( 0:�������, 1:�Ϲ����� 2:���� 3:������ ���) 2: ���� ��� (������, ��Ż....)
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
	//vValues 0 = ������ Ÿ�� 1 = �ڵ� 2 = ����
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
	virtual BOOL IsComplete(const DWORD dwValue);
};

class MissionTriggerJudge : public MissionTriggerBase
{
	//vValues 0 = ���� ���� ( 0 == ��� ����, 1 == ����, 2 == ���� �϶���)
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerAdd : public MissionTriggerBase
{
	//vValues 0 ���� ���ϱ⸸ �ϴ� Ʈ����
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};

class MissionTriggerLoginTimeCheck : public MissionTriggerBase
{
	//vValues 0 = �ش� �ð� 
public:
	virtual BOOL DoTrigger(MissionData* pMissionData, DWORDVec& vValues);
};