#include "stdafx.h"
#include "MissionManager.h"
#include "MissionData.h"
#include "User.h"
#include "../EtcHelpFunc.h"
#include "ioMission.h"
#include "Room.h"
#include "ioQuestManager.h"
#include "../DataBase/LogDBClient.h"

template<> MissionManager *Singleton< MissionManager >::ms_Singleton = 0;

MissionManager::MissionManager()
{
	Init();
}

MissionManager::~MissionManager()
{
	Destroy();
}

MissionManager& MissionManager::GetSingleton()
{
	return Singleton< MissionManager >::GetSingleton();
}

void MissionManager::Init()
{
	m_dwTimerCheckTime	= 0;

	m_vActiveDate.clear();
	m_vNextActiveDate.clear();
	m_vActiveIndex.clear();
	m_vMaxIndex.clear();
	m_vRelMissionIndex.clear();
	m_vResetHour.clear();
	m_vBaseTime.clear();

	for( int i = 0; i <= MT_MONTHLY; i++ )
	{
		m_vActiveDate.push_back(0);
		m_vNextActiveDate.push_back(0);
		m_vActiveIndex.push_back(0);
		m_vMaxIndex.push_back(0);
		m_vRelMissionIndex.push_back(0);
		m_vResetHour.push_back(0);
		m_vBaseTime.push_back(0);
	}
}

void MissionManager::DeleteDailyMissionTable()
{
	mCurMissionTable::iterator it = m_mDailyMissionTable.begin();

	for(	; it != m_mDailyMissionTable.end(); it++ )
	{
		Mission* pMission = it->second;
		if( pMission )
			delete pMission;
	}

	m_mDailyMissionTable.clear();
}

void MissionManager::DeleteWeeklyMissionTable()
{
	mCurMissionTable::iterator it = m_mWeeklyMissionTable.begin();

	for(	; it != m_mWeeklyMissionTable.end(); it++ )
	{
		Mission* pMission = it->second;
		if( pMission )
			delete pMission;
	}

	m_mWeeklyMissionTable.clear();
}

void MissionManager::DeleteMonthlyMissionTable()
{
	mCurMissionTable::iterator it = m_mMonthlyMissionTable.begin();

	for(	; it != m_mMonthlyMissionTable.end(); it++ )
	{
		Mission* pMission = it->second;
		if( pMission )
			delete pMission;
	}

	m_mMonthlyMissionTable.clear();
}

void MissionManager::Destroy()
{
	DeleteDailyMissionTable();
	DeleteWeeklyMissionTable();
	DeleteMonthlyMissionTable();
}

void MissionManager::ProcessMission()
{
	if( TIMEGETTIME() - m_dwTimerCheckTime < (60 * 1000) ) 
		return;

	m_dwTimerCheckTime = TIMEGETTIME();

	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 초 이상 걸리면로그 남김

	CTime kCurTime  = CTime::GetCurrentTime();
	DWORD dwCurTime = (DWORD)kCurTime.GetTime();
	DWORD dwNextDate = 0;
	static IntVec vResetTypeVec;
	vResetTypeVec.clear();

	for( int i = 0; i < MT_MONTHLY + 1; i++ )
	{
		dwNextDate = GetNextMissionDate(i);
		if( dwNextDate != 0 )
		{
			if( dwCurTime >= dwNextDate )
			{
				//초기화
				ChangeActiveMissionDate(i);
				ChangeActiveMissionData(i);
				vResetTypeVec.push_back(i);
			}
		}
	}

	if( !vResetTypeVec.empty() )
		g_UserNodeManager.UserNode_MissionTargetTypeReset(vResetTypeVec);
}

void MissionManager::LoadMissionTypeIniFile(ioINILoader& kLoader, int iType)
{
	switch( iType )
	{
	case MT_DAILY:
		kLoader.LoadFile( "config/sp2_daily_mission.ini" );
		break;
	case MT_WEEKLY:
		kLoader.LoadFile( "config/sp2_weekly_mission.ini" );
		break;
	case MT_MONTHLY:
		kLoader.LoadFile( "config/sp2_monthly_mission.ini" );
		break;
	}
}

void MissionManager::ReloadMissionTypeIniFile(ioINILoader& kLoader, int iType)
{
	switch( iType )
	{
	case MT_DAILY:
		kLoader.ReloadFile( "config/sp2_daily_mission.ini" );
		break;
	case MT_WEEKLY:
		kLoader.ReloadFile( "config/sp2_weekly_mission.ini" );
		break;
	case MT_MONTHLY:
		kLoader.ReloadFile( "config/sp2_monthly_mission.ini" );
		break;
	}
}

DWORD MissionManager::GetStartDateFromINI(ioINILoader& kLoader, const int iIndex)
{
	char szKey[MAX_PATH]="";

	kLoader.SetTitle("common");
	int iResetHour = kLoader.LoadInt("reset_hour", 0);

	StringCbPrintf( szKey, sizeof( szKey ), "date%d", iIndex );
	kLoader.SetTitle( szKey );
	int iStartDate		= kLoader.LoadInt( "start_date", 0 );
	if( 0 == iStartDate )
		return 0;

	int iYear			= (iStartDate / 10000) + 2000;
	int iMonth			= (iStartDate % 10000 ) / 100;
	int iDay			= (iStartDate % 10000 ) % 100;

	CTime cDate = CTime( Help::GetSafeValueForCTimeConstructor( iYear, iMonth, iDay, iResetHour, 0, 0 ) );
	return cDate.GetTime();
}

BOOL MissionManager::IsPrevMissionData(int iType, int iIndex)
{
	//
	ioINILoader kLoader;
	LoadMissionTypeIniFile(kLoader, iType);
	DWORD dwStartDate = GetStartDateFromINI(kLoader, iIndex);

	CTime cCurTime = CTime::GetCurrentTime();
	CTime cStartTime(dwStartDate);

	switch(iType)
	{
	case MT_DAILY:
		{
			CTimeSpan cSpan = cCurTime - cStartTime;
			if( cSpan.GetDays() > 0 )
				return TRUE;

			return FALSE;
		}
	case MT_WEEKLY:
		{
			char szKey[MAX_PATH] = "";
			kLoader.SetTitle("common");
			int iResetHour = kLoader.LoadInt("reset_hour", 0);
			BOOL bTest = kLoader.LoadBool("test", 0);

			StringCbPrintf( szKey, sizeof( szKey ), "date%d", iIndex );
			kLoader.SetTitle( szKey );
			int iEndDate		= kLoader.LoadInt( "end_date", 0 );

			int iYear			= (iEndDate / 10000) + 2000;
			int iMonth			= (iEndDate % 10000 ) / 100;
			int iDay			= (iEndDate % 10000 ) % 100;
			int iMinute			= 0;

			if( bTest )
			{
				iResetHour = kLoader.LoadInt("end_hour", iResetHour);
				iMinute	   = kLoader.LoadInt("end_minute", 0);
			}

			CTime cEndDate = CTime( Help::GetSafeValueForCTimeConstructor( iYear, iMonth, iDay, iResetHour, iMinute, 0 ) );
			
			if( cEndDate < cCurTime)
				return TRUE;

			return FALSE;
		}
	case MT_MONTHLY:
		{
			if(  cCurTime.GetYear() != cStartTime.GetYear() )
				return TRUE;

			if( cCurTime.GetYear() == cStartTime.GetYear() && cCurTime.GetMonth() > cStartTime.GetMonth() )
				return TRUE;

			return FALSE;

		}
	}
	return TRUE;
}

BOOL MissionManager::LoadINI(BOOL bReload)
{
	Init();

	char szKey[MAX_PATH]="";

	//daily Mission Read
	int iType	= 0;
	for(	; iType <= MT_MONTHLY; iType++ )
	{
		BOOL bFlag = FALSE;

		ioINILoader kLoader;
		if( bReload )
		{
			ReloadMissionTypeIniFile(kLoader, iType );
			switch( iType )
			{
			case MT_DAILY:
				DeleteDailyMissionTable();
				break;
			case MT_WEEKLY:
				DeleteWeeklyMissionTable();
				break;
			case MT_MONTHLY:
				DeleteMonthlyMissionTable();
				break;
			}
		}
		else
			LoadMissionTypeIniFile(kLoader, iType);

		kLoader.SetTitle( "common" );
		int		iResetHour	= kLoader.LoadInt( "reset_hour", 0 );	
		BOOL	bClose		= kLoader.LoadBool( "close", 0 );
#ifdef ROTATION_MISSION
		m_vResetHour[iType] = iResetHour;
		int		iMissionDate= GetCurrentMissionNumber( iType , kLoader.LoadInt( "basedate", 0 ) , kLoader.LoadInt( "RotationValue", 0 ) );
		if( 0 >= iMissionDate )
		{
			LOG.PrintTimeAndLog(0, "%s - ERROR MissionDate - please check the basedate", __FUNCTION__);
			iMissionDate = 0;
		}
#endif
		if( bClose )
		{
			DeleteDailyMissionTable();
			DeleteWeeklyMissionTable();
			DeleteMonthlyMissionTable();

			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] mission is closed");
			return TRUE;
		}
#ifdef ROTATION_MISSION
		m_vNextActiveDate[iType] = GetNextActiveDate( iType, iResetHour );
		m_vActiveIndex[iType]	 = iMissionDate;

		SetDateFromINI( kLoader, m_vActiveIndex[iType] );
		DWORD dwStartDate = GetStartDate( iType , iResetHour );
		m_vActiveDate[iType] = dwStartDate;
#else
		int iDate = 1;
		for(	; iDate < 1000; iDate++ )
		{
			DWORD dwStartDate = GetStartDateFromINI(kLoader, iDate);
			if( 0 == dwStartDate )
				break;

			CTime cCurDate = CTime::GetCurrentTime();
			DWORD dwCurDate = cCurDate.GetTime();

			if( dwStartDate < dwCurDate )
				continue;

			if( dwCurDate < dwStartDate && FALSE == bFlag )
			{
				m_vNextActiveDate[iType] = dwStartDate;
				m_vActiveIndex[iType] = iDate - 1;
				bFlag = TRUE;
			}
		}

		m_vMaxIndex[iType] = iDate - 1;
		if( 0 == m_vActiveIndex[iType] )
		{
			//첫 인덱스, 마지막 인덱스 시간을 비교.
			if( GetMaxIndex(iType) != 1 && !IsPrevMissionData(iType, 1) )
				continue;

			if( IsPrevMissionData(iType, m_vMaxIndex[iType]) )
				continue;

			int iIndex = m_vMaxIndex[iType];
			m_vActiveIndex[iType] = iIndex;

			ioINILoader kLoader;
			LoadMissionTypeIniFile(kLoader, iType);
			DWORD dwStartDate = GetStartDateFromINI(kLoader, iIndex);
			m_vActiveDate[iType] = dwStartDate;
		}

		//현재 동작중인 미션으로 셋팅.
		DWORD dwStartDate = GetStartDateFromINI(kLoader, m_vActiveIndex[iType]);
		CTime cCurdate = CTime::GetCurrentTime();
		DWORD dwCurDate = cCurdate.GetTime();
		DWORD dwEndDate = 0;

		m_vActiveDate[iType] = dwStartDate;
#endif
		for( int i = 0; i < 1000; i++ )
		{
			char szValues[MAX_PATH] = "";

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_code", i+1 );
			int iMissionCode	= kLoader.LoadInt(szKey, 0);
			if( 0 == iMissionCode )
				break;

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_type", i+1 );
			int iMissionClass	= kLoader.LoadInt(szKey, 0);

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_values", i+1 );
			kLoader.LoadString(szKey, "", szValues, MAX_PATH );

			//value 파싱
			IntVec vValues;
			Help::TokenizeToINT(szValues, ".", vValues);

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_present", i+1 );
			int iPresent		= kLoader.LoadInt(szKey, 0);

			Mission* pMission = new Mission;
			if( !pMission )
				continue;

			if( !pMission->Create(iMissionCode, (MissionClasses)iMissionClass, (MissionTypes)iType, vValues, iPresent) )
				return FALSE;

			switch( iType )
			{
			case MT_DAILY:
				m_mDailyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
				break;
			case MT_WEEKLY:
				m_mWeeklyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
				break;
			case MT_MONTHLY:
				m_mMonthlyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
				break;
			}
		}
	}
	
	//Active mission code
	mCurMissionTable::iterator it;
	for( it = m_mDailyMissionTable.begin() ; it != m_mDailyMissionTable.end(); it++ )
	{
		Mission *pMission = it->second;
		if( pMission )
			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] daily active mission code : %d", pMission->GetMissionCode());
	}

	for( it = m_mWeeklyMissionTable.begin(); it != m_mWeeklyMissionTable.end(); it++ )
	{
		Mission *pMission = it->second;
		if( pMission )
			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] weekly active mission code : %d", pMission->GetMissionCode());
	}

	for( it = m_mMonthlyMissionTable.begin(); it != m_mMonthlyMissionTable.end(); it++ )
	{
		Mission *pMission = it->second;
		if( pMission )
			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] monthly active mission code : %d", pMission->GetMissionCode());
	}

	return TRUE;
}

DWORD MissionManager::GetNextMissionDate(const int iMissionType)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return 0;
	}
	return m_vNextActiveDate[iMissionType];
}

DWORD MissionManager::GetNextMissionDate( const int iMissionType, int iIndex )
{
	if( m_vRelMissionIndex[iMissionType] == 0 )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] Can't Found Mission Date : [type:%d]", iMissionType);
		return -1;
	}
	if( m_vResetHour[iMissionType] == 0 )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] Can't Found Reset Hour : [type:%d]", iMissionType);
		return -1;
	}
	
	CTime cBaseTime = m_vBaseTime[iMissionType];
	m_vRelMissionIndex[iMissionType] += 1;
	CTimeSpan cSpanTime = NULL;
	int iLeftYear = 0;
	int iRestMonth = 0;
	switch( iMissionType )
	{
	case MT_DAILY:
		cSpanTime = CTimeSpan( m_vRelMissionIndex[iMissionType], 0, 0, 0 );
		break;
	case MT_WEEKLY:
		cSpanTime = CTimeSpan( m_vRelMissionIndex[iMissionType] * 7, 0, 0, 0 );
		break;
	case MT_MONTHLY:
		if( (cBaseTime.GetMonth() + iIndex ) > 12)
		{
			iLeftYear = ( cBaseTime.GetMonth() + iIndex ) / 12;
			iRestMonth = ( cBaseTime.GetMonth() + iIndex ) % 12 + 1; //next month time
			cBaseTime = CTime( Help::GetSafeValueForCTimeConstructor( cBaseTime.GetYear() + iLeftYear , iRestMonth, 0, 0, 0, 0 ) );
		}
		else
		{
			cBaseTime = CTime( Help::GetSafeValueForCTimeConstructor( cBaseTime.GetYear(), iIndex + 2, cBaseTime.GetDay(), 0, 0, 0 ) );
		}
		break;
	}
	if( cSpanTime != NULL )
		cBaseTime += cSpanTime;

	cBaseTime += CTimeSpan( 0, m_vResetHour[iMissionType], 0, 0 );
	return cBaseTime.GetTime();
}

DWORD MissionManager::GetActiveMissionDate(const int iMissionType)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return -1;
	}

	return m_vActiveDate[iMissionType];
}

int MissionManager::GetActiveIndex(const int iMissionType)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return -1;
	}

	return m_vActiveIndex[iMissionType];
}

void MissionManager::SetActiveMissionDate(const int iMissionType, DWORD dwDate)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return;
	}

	m_vActiveDate[iMissionType] = dwDate;
}

void MissionManager::SetNextMissionDate(const int iMissionType, DWORD dwDate)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return;
	}

	m_vNextActiveDate[iMissionType] = dwDate;
}

void MissionManager::SetActiveIndex(const int iMissionType, int iIndex)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return;
	}

	m_vActiveIndex[iMissionType] = iIndex;
}

int MissionManager::GetMaxIndex(const int iMissionType)
{
	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return 0;
	}

	return m_vMaxIndex[iMissionType];
}

void MissionManager::ChangeActiveMissionDate(const int iType)
{
	if( !COMPARE(iType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iType);
		return;
	}

	DWORD dwNextDate = GetNextMissionDate(iType);
	if( 0 == dwNextDate )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] next date is not exist : [type:%d activeIndex:%d]", iType, m_vActiveIndex[iType]);

		//ini에서 로드 실시.
		int iActiveIndex = GetActiveIndex(iType);
		if( iActiveIndex < 0 )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warining][missionmgr] current index is not exist : [type:%d ]", iType);
			return;
		}

		ioINILoader kLoader;
		LoadMissionTypeIniFile(kLoader, iType);
		dwNextDate = GetStartDateFromINI(kLoader, iActiveIndex + 1);
		if( 0 == dwNextDate )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[error][missionmgr] next date is not exist : [type:%d activeIndex:%d]", iType, m_vActiveIndex[iType]);
			return;
		}
	}
	
	int iNextIndex = GetActiveIndex(iType) + 1;
	if( iNextIndex > GetMaxIndex(iType) )
	{
#ifdef ROTATION_MISSION
		iNextIndex = (m_vRelMissionIndex[iType] % m_vMaxIndex[iType]) + 1;
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] next index is overflow : [type:%d nextIndex:%d]", iType, iNextIndex);
#else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] next index is overflow : [type:%d nextIndex:%d]", iType, iNextIndex);
		return;
#endif
	}

	ioINILoader kLoader;
	LoadMissionTypeIniFile(kLoader, iType);
	SetActiveIndex(iType, iNextIndex);
	SetActiveMissionDate(iType, dwNextDate);

#ifdef ROTATION_MISSION
	dwNextDate = GetNextMissionDate( iType, m_vRelMissionIndex[iType] );
#else
	dwNextDate = GetStartDateFromINI(kLoader, iNextIndex + 1);
#endif

	if( 0 == dwNextDate )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] next date is not exist : [type:%d activeIndex:%d]", iType, m_vActiveIndex[iType]);
		SetNextMissionDate(iType, 0);
		return;
	}
	SetNextMissionDate(iType, dwNextDate);
}

BOOL MissionManager::IsAlive(const DWORD dwCode)
{
	if( GetActiveMission(dwCode) )
		return TRUE;

	return FALSE;
}

void MissionManager::DoTrigger(const MissionClasses eMissionClass, User* pUser, DWORDVec& vValues, BOOL bMacro)
{
	if( !pUser )
		return;

	ioMission* pUserMission = pUser->GetUserMission();
	if( !pUserMission )
		return;

	pUserMission->TriggerMission(eMissionClass, vValues, bMacro);

	//일일,주간 미션과 연관된 미션들 체크.
	for( int i = 0; i <= MT_MONTHLY; i++ )
	{	
		int iCompletedCnt = pUserMission->GetCompletedTypeCount(i);
		if( iCompletedCnt <= 0 )
			continue;

		static DWORDVec vCompleteVec;
		vCompleteVec.clear();
		vCompleteVec.push_back(iCompletedCnt);

		if( MT_DAILY == i )
		{
			pUserMission->TriggerMission(MISSION_CLASS_DAILY_COMPLETE, vCompleteVec);
			pUserMission->TriggerMission(MISSION_CLASS_DAILY_ALL_CLEAR, vCompleteVec);
		}
		else if( MT_WEEKLY == i )
		{
			pUserMission->TriggerMission(MISSION_CLASS_WEEKLY_COMPLETE, vCompleteVec);
			pUserMission->TriggerMission(MISSION_CLASS_WEEKLY_ALL_CLEAR, vCompleteVec);
		}
		else
			pUserMission->TriggerMission(MISSION_CLASS_MONTHLY_ALL_CLEAR, vCompleteVec);	

		pUserMission->InitCompletedTypeCount(i);
	}
}

BOOL MissionManager::IsComplete(MissionData* pMissionData)
{
	if( !pMissionData )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warining][missionmgr] missiondata is null");
		return FALSE;
	}

	Mission* pMission = GetActiveMission(pMissionData->GetCode());
	if( pMission )
	{
		return pMission->IsComplete(pMissionData->GetValue());
	}

	return FALSE;
}

Mission* MissionManager::GetActiveMission(const DWORD dwCode)
{
	mCurMissionTable::iterator it = m_mDailyMissionTable.find(dwCode);
	if( it != m_mDailyMissionTable.end() )
		return it->second;

	it = m_mWeeklyMissionTable.find(dwCode);
	if( it != m_mWeeklyMissionTable.end() )
		return it->second;

	it = m_mMonthlyMissionTable.find(dwCode);
	if( it != m_mMonthlyMissionTable.end() )
		return it->second;

	return NULL;
}

DWORD MissionManager::GetPresentID(DWORD dwCode)
{
	Mission* pMission = GetActiveMission(dwCode);
	if( pMission )
		return pMission->GetMissionPresent();

	return 0;
}

void MissionManager::SendReward(User* pUser, const DWORD dwMissionCode)
{
	if( !pUser )
		return;

	Mission* pMission = GetActiveMission(dwMissionCode);
	if( pMission )
	{
		bool bDirect = g_QuestMgr.SendRewardPresent(pUser, pMission->GetMissionPresent() );
		if( !bDirect )
			pUser->SendPresentMemory();

		//LogDB insert
		int iParam1 = 0, iParam2 = 0, iParam3 = 0, iParam4 = 0;
		g_QuestMgr.GetRewardPresent(pMission->GetMissionPresent(), iParam1, iParam2, iParam3, iParam4);

		g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_COMPLETE_COMPENSATION, pUser, 0, pMission->GetMissionCode(), (int)(pMission->GetMissionType()) + 1, iParam1, iParam2, iParam3, iParam4, NULL );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] mission is not exist, present is not recved : [missioncode:%d]", dwMissionCode );
	}
	
}

void MissionManager::GetResetMissionType(const DWORD dwDate, IntVec& vMissionType)
{
	bool bResetFlag[MT_MONTHLY+1] = {false, false, false};
	DWORD dwActiveDate = 0;

	for( int i = 0; i <= MT_MONTHLY; i++ )
	{
		dwActiveDate = GetActiveMissionDate(i);

#ifdef ROTATION_MISSION
		//로그시간과 엑티브 시간체크!
		//다음날부터 리셋타임 전에 테스트 할 경우 미션 초기화 막음.
		CTime TimeGetDay = m_vActiveDate[i];
		int iActiveDay = TimeGetDay.GetDay();
		TimeGetDay = dwDate;
		int iLogDay = TimeGetDay.GetDay();
		if( iActiveDay == iLogDay )
			continue;
#endif
		if( 0 == m_vActiveDate[i] || dwDate < m_vActiveDate[i] )
		{
			bResetFlag[i] = true;
		}
	}

	vMissionType.push_back(bResetFlag[MT_DAILY]);
	vMissionType.push_back(bResetFlag[MT_WEEKLY]);
	vMissionType.push_back(bResetFlag[MT_MONTHLY]);
}

void MissionManager::TurnCurDataIntoNextData(ioMission* pUserMission, int iResetType)
{
	if( !pUserMission )
		return;

	if( !COMPARE(iResetType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iResetType);
		return;
	}

	//pUserMission->InitMissionType(iResetType);
	pUserMission->DeleteMissionData(iResetType);
	FillActiveMissionData( pUserMission, iResetType);
}

void MissionManager::FillActiveMissionData(ioMission* pUserMission, const int iMissionType)
{
	if( !pUserMission )
		return;

	if( !COMPARE(iMissionType, MT_DAILY, MT_MONTHLY+1) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] non exist missiontype : [type:%d]", iMissionType);
		return;
	}

	mCurMissionTable::iterator it;
	switch( iMissionType )
	{
	case MT_DAILY:
		{
			it = m_mDailyMissionTable.begin();
			for( ; it != m_mDailyMissionTable.end(); it++ )
			{
				Mission* pMission = it->second;
				if( pMission )
					pUserMission->InsertMission(pMission->GetMissionCode(), MS_PROGRESS, MT_DAILY, 0 );
			}
			break;
		}
	case MT_WEEKLY:
		{
			it = m_mWeeklyMissionTable.begin();
			for( ; it != m_mWeeklyMissionTable.end(); it++ )
			{
				Mission* pMission = it->second;
				if( pMission )
					pUserMission->InsertMission(pMission->GetMissionCode(), MS_PROGRESS, MT_WEEKLY, 0 );
			}
			break;
		}
	case  MT_MONTHLY:
		{
			it = m_mMonthlyMissionTable.begin();
			for( ; it != m_mMonthlyMissionTable.end(); it++ )
			{
				Mission* pMission = it->second;
				if( pMission )
					pUserMission->InsertMission(pMission->GetMissionCode(), MS_PROGRESS, MT_MONTHLY, 0 );
			}
			break;
		}
	}
}

void MissionManager::FillAllActiveMissionData(ioMission* pUserMission)
{
	if( !pUserMission )
		return;

	FillActiveMissionData(pUserMission, MT_DAILY);
	FillActiveMissionData(pUserMission, MT_WEEKLY);
	FillActiveMissionData(pUserMission, MT_MONTHLY);
}

void MissionManager::ChangeActiveMissionData(const int iType)
{
	DWORD dwActiveDate = 0;

	switch(iType)
	{
	case MT_DAILY:
		{
			DeleteDailyMissionTable();
			break;
		}
	case MT_WEEKLY:
		{
			DeleteWeeklyMissionTable();
			break;
		}
	case MT_MONTHLY:
		{
			DeleteMonthlyMissionTable();
			break;
		}
	}

	CreateActiveMissionTable(iType);
}

void MissionManager::CreateActiveMissionTable(const int iType)
{
	ioINILoader kLoader;
	LoadMissionTypeIniFile(kLoader, iType);
	DWORD dwActiveDate = GetActiveMissionDate(iType);
	int iIndex = GetActiveIndex(iType);
#ifdef ROTATION_MISSION
	if( iIndex <= 0 )
#else
	if( iIndex <= 0 || iIndex > GetMaxIndex(iType))
#endif
		return;

	if( dwActiveDate != 0 )
	{
		char szKey[MAX_PATH];
		char szValues[MAX_PATH] = "";
		StringCbPrintf( szKey, sizeof( szKey ), "date%d", iIndex );
		kLoader.SetTitle(szKey);
		for( int i = 0; i < 1000; i++ )
		{

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_code", i+1 );
			int iMissionCode	= kLoader.LoadInt(szKey, 0);
			if( 0 == iMissionCode )
				break;

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_type", i+1 );
			int iMissionClass	= kLoader.LoadInt(szKey, 0);

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_values", i+1 );
			kLoader.LoadString(szKey, "", szValues, MAX_PATH );

			//value 파싱
			IntVec vValues;
			Help::TokenizeToINT(szValues, ".", vValues);

			StringCbPrintf( szKey, sizeof( szKey ), "mission%d_present", i+1 );
			int iPresent		= kLoader.LoadInt(szKey, 0);

			Mission* pMission = new Mission;
			if( !pMission )
				break;

			if( !pMission->Create(iMissionCode, (MissionClasses)iMissionClass, (MissionTypes)iType, vValues, iPresent) )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][missionmgr] mission value is invalid : [code:%d]", iMissionCode);
				continue;
			}

			switch( iType )
			{
			case MT_DAILY:
				m_mDailyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
				break;
			case MT_WEEKLY:
				m_mWeeklyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
				break;
			case MT_MONTHLY:
				m_mMonthlyMissionTable.insert(make_pair(pMission->GetMissionCode(), pMission));
			}
		}

		//동작 중인 미션 로그!
		mCurMissionTable::iterator it;
		for( it = m_mDailyMissionTable.begin() ; it != m_mDailyMissionTable.end(); it++ )
		{
			Mission *pMission = it->second;
			if( pMission )
				LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] daily active mission code : %d", pMission->GetMissionCode());
		}

		for( it = m_mWeeklyMissionTable.begin(); it != m_mWeeklyMissionTable.end(); it++ )
		{
			Mission *pMission = it->second;
			if( pMission )
				LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] weekly active mission code : %d", pMission->GetMissionCode());
		}

		for( it = m_mMonthlyMissionTable.begin(); it != m_mMonthlyMissionTable.end(); it++ )
		{
			Mission *pMission = it->second;
			if( pMission )
				LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[info][missionmgr] monthly active mission code : %d", pMission->GetMissionCode());
		}
	}
}

BOOL MissionManager::IsTimeMission(const DWORD dwCode)
{
	Mission* pMission = GetActiveMission(dwCode);
	if( pMission )
	{
		int iMissionClass = pMission->GetMissionClass();

		switch( iMissionClass )
		{
		case MISSION_CLASS_LOGINTIME_CHECK:
			return TRUE;
		}
	}

	return FALSE;
}

int MissionManager::GetMissionClass(const DWORD dwCode)
{
	Mission* pMission = GetActiveMission(dwCode);
	if( pMission )
		return pMission->GetMissionClass();

	return MISSION_CLASS_NONE;
}

void MissionManager::TestChangeNextDate( SP2Packet& kPacket )
{
	int iMacroType = 0;
	int iType = 0;
	int iMinutes = 0;
	int iNextTab = 0;

	PACKET_GUARD_VOID( kPacket.Read(iMacroType) );
	PACKET_GUARD_VOID( kPacket.Read(iType) );

	if( !COMPARE(iType, 0, MT_MONTHLY + 1) )
		return;

	if( 1 == iMacroType )
	{
		PACKET_GUARD_VOID( kPacket.Read(iMinutes) );
		
		DWORD dwActiveDate = GetActiveMissionDate(iType);
		DWORD dwNextDate  = GetNextMissionDate(iType);

		if( dwActiveDate != 0 && dwNextDate != 0 )
		{
	
			CTime cNextDate = CTime::GetCurrentTime();
			CTimeSpan cSpan(0,0,iMinutes,0);
			cNextDate += cSpan;

			DWORD dwNextDate = cNextDate.GetTime();
			SetNextMissionDate(iType, dwNextDate);
		}
	}
	else if( 2 == iMacroType )
	{
		PACKET_GUARD_VOID( kPacket.Read(iNextTab) );
		PACKET_GUARD_VOID( kPacket.Read(iMinutes) );

		int iActiveIndex = GetActiveIndex(iType);
		if( iActiveIndex != 0 )
		{
			if( 0 == iNextTab )
				return;

			iActiveIndex += iNextTab;
			if( GetMaxIndex(iType) < iActiveIndex )
				return;

			SetActiveIndex(iType, iActiveIndex-1);

			CTime cNextDate = CTime::GetCurrentTime();
			CTimeSpan cSpan(0,0,iMinutes,0);
			cNextDate += cSpan;

			DWORD dwNextDate = cNextDate.GetTime();
			SetNextMissionDate(iType, dwNextDate);
		}

	}
}

DWORD MissionManager::GetMostRapidNextActiveDate()
{
	DWORD dwNextActiveDate = GetNextMissionDate(MT_DAILY);
	if( 0 == dwNextActiveDate )
	{
		DWORD dwNextWeekly = GetNextMissionDate(MT_WEEKLY);
		DWORD dwNextMonthly = GetNextMissionDate(MT_MONTHLY);
		
		if( dwNextWeekly != 0 && 0 == dwNextMonthly )
			dwNextActiveDate = dwNextWeekly;
		else if( dwNextMonthly != 0 && 0 == dwNextWeekly )
			dwNextActiveDate = dwNextMonthly;
		else if( dwNextMonthly != 0 && dwNextWeekly != 0 )
		{
			dwNextActiveDate = min(dwNextWeekly, dwNextMonthly);
		}
	}

	return dwNextActiveDate;
}

int MissionManager::GetCurrentMissionNumber( IN int iType, IN int iBaseDate, IN int iRotation )
{
	if( 0 == iBaseDate )
	{
		LOG.PrintTimeAndLog( 0 , "%s - Base Time is Empty", __FUNCTION__ );
		return 0;
	}
	int iYear	= (iBaseDate / 10000) + 2000;
	int iDay	= (iBaseDate % 10000 ) % 100;
	int iMonth	= (iBaseDate % 10000 ) / 100;

	if( iYear >= 3000 )
	{
		//The upper date limit is 12/31/3000. The lower limit is 1/1/1970 12:00:00 AM GMT.
		LOG.PrintTimeAndLog( 0 , "%s - basetime is error. time check please.( year : %d, month : %d, day : %d )", __FUNCTION__ , iYear, iMonth, iDay);
		return 0;
	}
	//base 날짜, 월요일(1), 화요일(2)
	CTime cBaseTime = CTime(iYear,iMonth,iDay,0,0,0);
	CTime cCurTime = CTime::GetCurrentTime();
	if( cCurTime.GetHour() < 5 )
	{
		cCurTime -= CTimeSpan( 1, 0, 0, 0 );
	}
	CTimeSpan cTmSpan = cCurTime - cBaseTime;
	m_vBaseTime[iType] = cBaseTime.GetTime();
	m_vMaxIndex[iType] = iRotation;

	switch( iType )
	{
		case MT_DAILY :
			m_vRelMissionIndex[iType] = cTmSpan.GetDays()  + 1;
			return ( cTmSpan.GetDays() % iRotation ) + 1;
			break;
		case MT_WEEKLY : 
			m_vRelMissionIndex[iType] = ( cTmSpan.GetDays() / 7 ) + 1;
			return (( cTmSpan.GetDays() / 7 ) % iRotation ) + 1 ;
			break;
		case MT_MONTHLY :
			iMonth = 0;
			iMonth = cCurTime.GetMonth();
			iMonth = cBaseTime.GetMonth();
			if( cBaseTime.GetYear() != cCurTime.GetYear() )
				iMonth = ( cCurTime.GetYear() - cBaseTime.GetYear() ) * 12;
			m_vRelMissionIndex[iType] =  iMonth + cCurTime.GetMonth() - 1;
			if( ( m_vRelMissionIndex[iType] % iRotation  ) == 0 ) 
				return iRotation;
			return ( m_vRelMissionIndex[iType] % iRotation  );
			break;
	}
	return 0;
}

//오늘 날짜를 기준으로 체크
DWORD MissionManager::GetStartDate( IN int iType, IN int iResetHour )
{
	CTime cCurrentTime	= CTime::GetCurrentTime();
	CTime cStartTime	= CTime::GetCurrentTime();
	CTimeSpan cSpanTime = NULL;
	int iWeekDay = 0;
	int iDay	 = 0;
	int iBaseWeekDay = 0;
	int iTomorrow = 0 ;
	switch( iType )
	{
	case MT_DAILY :
		if( iResetHour > cCurrentTime.GetHour() )
			iTomorrow = 1;
		cSpanTime = CTimeSpan(iTomorrow, cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() );
		break;
	case MT_WEEKLY : 
		cStartTime = m_vBaseTime[iType];
		iBaseWeekDay = cStartTime.GetDayOfWeek(); //1: 일요일, 2: 월요일
		iWeekDay = cCurrentTime.GetDayOfWeek();		

		if (iBaseWeekDay > iWeekDay )
			iWeekDay = 7 - ( iBaseWeekDay - iWeekDay );
		else if(iWeekDay == iBaseWeekDay && cCurrentTime.GetHour() < iResetHour )
			iWeekDay = 7;
		else if(iWeekDay == iBaseWeekDay /*&& cCurrentTime.GetHour() < iResetHour*/ )
			iWeekDay = 0;
		else
			iWeekDay = iWeekDay - iBaseWeekDay;

		cSpanTime = CTimeSpan( iWeekDay, cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() );
		break;
	case MT_MONTHLY :
		iDay = cCurrentTime.GetDay();
		if( iDay == 1 && iResetHour < cCurrentTime.GetHour()) //이번 달 시간
			break;
		else if( iDay == 1 && iResetHour > cCurrentTime.GetHour() ) //한달 전 시간
		{
			cSpanTime = CTimeSpan( iDay , cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() ); //하루전에 엑티브
		}
		else //이번달 경과 시간
			cSpanTime = CTimeSpan( iDay - 1, cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() );
		break;
	}
	if( cSpanTime != NULL )
	{
 		if( iType == 1 && iTomorrow == 1 )
 			cStartTime = cCurrentTime + cSpanTime;
		else
			cStartTime = cCurrentTime - cSpanTime;
		cStartTime += CTimeSpan( 0, iResetHour, 0, 0 );
	}
	return cStartTime.GetTime();
}

DWORD MissionManager::GetNextActiveDate( IN int iType, IN int iResetHour )
{
	CTime	cCurrentTime	= CTime::GetCurrentTime();
	int		iTomorrow = 0;
	int		iBaseWeekDay = 0;
	int		iWeekDay = 0;
	int		iCurrHour = 0;
	CTime	cBaseTime =  NULL;
	CTime		cActiveTime	= CTime::GetCurrentTime();
	CTimeSpan	cSpanTime = NULL;
	switch( iType )
	{
	case MT_DAILY:
		if( cCurrentTime.GetHour() >= iResetHour )
		{
			iTomorrow = 1;
		}
		cCurrentTime -= CTimeSpan( 0, cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() );
		cSpanTime = CTimeSpan( iTomorrow ,iResetHour ,0 , 0 );
		break;
	case MT_WEEKLY:
		iCurrHour = cCurrentTime.GetHour();
		cCurrentTime -= CTimeSpan( 0, cCurrentTime.GetHour(), cCurrentTime.GetMinute(), cCurrentTime.GetSecond() ); //오늘 0시 0분
		cBaseTime = m_vBaseTime[iType];
		iBaseWeekDay = cBaseTime.GetDayOfWeek(); //기준일 요일 체크, 1: 일요일, 2: 월요일
		iWeekDay = cCurrentTime.GetDayOfWeek(); //오늘날 요일 체크
		if (iBaseWeekDay > iWeekDay ) //다음 일 : 2 
			iWeekDay = iBaseWeekDay - iWeekDay ;
		else if(iWeekDay == iBaseWeekDay && iCurrHour >= iResetHour )
			iWeekDay = 7;
		else if(iWeekDay == iBaseWeekDay && iCurrHour < iResetHour )
			iWeekDay = 0;
		else //기준일 보다 빠른 경우
		{
			iWeekDay = 7 - ( iWeekDay - iBaseWeekDay );
		}
		cSpanTime = CTimeSpan( iWeekDay, iResetHour, 0, 0 );
		break;
	case MT_MONTHLY:
		if(cCurrentTime.GetHour() < iResetHour && cCurrentTime.GetDay() == 1 )
			iTomorrow = 0;
		else
			iTomorrow = 1;

		if( iTomorrow == 0 )
			return CTime( Help::GetSafeValueForCTimeConstructor( cCurrentTime.GetYear(), cCurrentTime.GetMonth(), 0, iResetHour, 0, 0 ) ).GetTime(); //한달 지남 문제 있음
		else
			return CTime( Help::GetSafeValueForCTimeConstructor( cCurrentTime.GetYear(), cCurrentTime.GetMonth() + 1, 0, iResetHour, 0, 0 ) ).GetTime(); //한달 지남
		break;
	}
	if( cSpanTime != NULL )
		cActiveTime = cCurrentTime + cSpanTime;
	return cActiveTime.GetTime();
}

void MissionManager::SetDateFromINI( ioINILoader& kLoader, const int iIndex )
{
	char szKey[MAX_PATH]="";
	StringCbPrintf( szKey, sizeof( szKey ), "date%d", iIndex );
	kLoader.SetTitle( szKey );
}
