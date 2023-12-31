#include "stdafx.h"
#include "MissionTriggerExtend.h"
#include "MissionData.h"

BOOL MissionTriggerModePlay::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	if( GetValue(1) > vValues[0] )
		return FALSE;

	if( 0 == GetValue(2) || ( GetValue(2) == vValues[1] && GetValue(3) == vValues[2] ) || ( GetValue(2) == vValues[1] && 0 == GetValue(3) ) )
	{
		pMissionData->AddValue(1);
		return TRUE;
	}
	
	return FALSE;
}

BOOL MissionTriggerMonsterKill::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	//0 : 목표 달성도
	if( GetValue(1) != vValues[0] )
		return FALSE;

	pMissionData->AddValue(1);
	return TRUE;
}

BOOL MissionTriggerAllClear::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	//목표 달성
	pMissionData->AddValue(1);
	return TRUE;
}

BOOL MissionTriggerItem::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	//0 : type, 1 : code, 2 : 사용 수
	if( GetValue(0) == vValues[0] && GetValue(1) == vValues[1] )
	{
		pMissionData->AddValue(vValues[2]);
		return TRUE;
	}

	return FALSE;
}

BOOL MissionTriggerItem::IsComplete(const DWORD dwValue)
{
	if( dwValue >= GetValue(2) )
		return TRUE;

	return FALSE;
}

BOOL MissionTriggerJudge::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	//0:2가지 선택지 중에 선택값
	if( GetValue(1) == 0 || GetValue(1) == vValues[0] )
	{
		//성공, 실패 상관 없음
		if( (int)vValues.size() > 1 )
			pMissionData->AddValue(vValues[1]);
		else
			pMissionData->AddValue(1);

		return TRUE;
	}

	return FALSE;
}


BOOL MissionTriggerAdd::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	//0 : 목표 달성도
	pMissionData->AddValue(vValues[0]);
	return TRUE;
}

BOOL MissionTriggerLoginTimeCheck::DoTrigger(MissionData* pMissionData, DWORDVec& vValues)
{
	if( !pMissionData )
		return FALSE;

	CTime cCurTime = CTime::GetCurrentTime();
	CTime cLoginTime(vValues[0]);

	CTimeSpan cGap = cCurTime - cLoginTime;
	DWORD dwGapTime = cGap.GetTotalMinutes();

	if( GetValue(0) <= dwGapTime )
	{
		pMissionData->SetValue(GetValue(0));
		return TRUE;
	}
		
	return FALSE;
}