#include "stdafx.h"
#include "User.h"
#include "../QueryData/QueryResultData.h"
#include "../EtcHelpFunc.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "TimeCashManager.h"
#include "UserTimeCashTable.h"

UserTimeCashTable::UserTimeCashTable()
{
	Init(NULL);
}

UserTimeCashTable::~UserTimeCashTable()
{
}

void UserTimeCashTable::Init(User* pUser)
{
	m_vTimeCashTable.clear();

	m_pUser				= pUser;
	m_bActive			= FALSE;
}

void UserTimeCashTable::Destroy()
{
}

void UserTimeCashTable::DBToData(CQueryResultData *query_data)
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s User NULL!!", __FUNCTION__ ); 
		return;
	}

	//������ �ְ�.
	DWORD dwItemCode	= 0;
	DBTIMESTAMP RecentlyRewardDate;
	DBTIMESTAMP EndDate;

	if( query_data->IsExist() )
	{
		UserCashTable stInfo;

		PACKET_GUARD_VOID( query_data->GetValue( dwItemCode, sizeof(DWORD) ) );
		PACKET_GUARD_VOID( query_data->GetValue( (char*)&EndDate, sizeof(DBTIMESTAMP) ) );
		PACKET_GUARD_VOID( query_data->GetValue( (char*)&RecentlyRewardDate, sizeof(DBTIMESTAMP) ) );
		
		DWORD dwRecentlyDate	= Help::ConvertDBTIMESTAMPToDWORD(RecentlyRewardDate);
		DWORD dwEndDate			= Help::ConvertDBTIMESTAMPToDWORD(EndDate);

		stInfo.m_dwItemCode		= dwItemCode;
		stInfo.m_dwReceiveDate	= dwRecentlyDate;
		stInfo.m_dwEndDate		= dwEndDate;

		m_vTimeCashTable.push_back(stInfo);
	}

	//���� üũ
	int iSize = m_vTimeCashTable.size();
	
	for( int i = 0; i < iSize; i++ )
	{
		//�Ⱓ�� ������?
		if( IsExpired(m_vTimeCashTable[i].m_dwEndDate) )
		{
			//Update ó��.
			SQLUpdateReceiveDate(m_vTimeCashTable[i].m_dwItemCode, FALSE);
			continue;
		}

		if( !IsActive() )
		{
			CheckUpdateInfo();
		}
	}
}

void UserTimeCashTable::CheckUpdateInfo()
{
	int iSize = m_vTimeCashTable.size();
	
	for( int i = 0; i < iSize; i++ )
	{
		if( IsRenewalDate(m_vTimeCashTable[i].m_dwItemCode) )
		{
			SQLUpdateReceiveDate(m_vTimeCashTable[i].m_dwItemCode, TRUE);
		}
		else
		{
			SetActive(TRUE);
		}
	}
}

void UserTimeCashTable::FillMoveData(SP2Packet& kPacket)
{
	int iSize = m_vTimeCashTable.size();

	PACKET_GUARD_VOID(kPacket.Write(m_bActive));
	PACKET_GUARD_VOID(kPacket.Write(iSize));

	for( int i = 0; i < iSize; i++ )
	{
		PACKET_GUARD_VOID(kPacket.Write(m_vTimeCashTable[i].m_dwItemCode));
		PACKET_GUARD_VOID(kPacket.Write(m_vTimeCashTable[i].m_dwReceiveDate));
		PACKET_GUARD_VOID(kPacket.Write(m_vTimeCashTable[i].m_dwEndDate));
	}
}

void UserTimeCashTable::ApplyMoveData(SP2Packet& kPacket)
{
	int iSize = 0;

	PACKET_GUARD_VOID(kPacket.Read(m_bActive));

	PACKET_GUARD_VOID(kPacket.Read(iSize));

	for( int i = 0; i < iSize; i++ )
	{
		UserCashTable sInfo;

		PACKET_GUARD_VOID(kPacket.Read(sInfo.m_dwItemCode));
		PACKET_GUARD_VOID(kPacket.Read(sInfo.m_dwReceiveDate));
		PACKET_GUARD_VOID(kPacket.Read(sInfo.m_dwEndDate));
	}
}

void UserTimeCashTable::SQLUpdateReceiveDate(const DWORD dwItemCode, BOOL bRequestCash)
{
	if( !m_pUser )
		return;

	if( bRequestCash )
	{
		if( m_pUser->IsBillingWait() )
		{
			//LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][timecash]SQLUpdateReceiveDate Billing Wait : [%d]", m_pUser->GetUserIndex() );
			return;
		}

		char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
		Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
		m_pUser->SetBillingGUID(szTempGUID);
	}
	
	g_DBClient.OnUpdateTimeCashTable(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), m_pUser->GetBillingGUID(), dwItemCode);
}

void UserTimeCashTable::ResultSQLInsertCashTable(CQueryResultData *query_data)
{
	if( !query_data )
		return;

	DWORD dwCode	= 0;
	DWORD dwEndDate	= 0;
	DBTIMESTAMP RecentlyRewardDate;
	char szGUID[USER_GUID_NUM_PLUS_ONE]="";

	PACKET_GUARD_VOID( query_data->GetValue( szGUID, USER_GUID_NUM_PLUS_ONE ) );
	PACKET_GUARD_VOID( query_data->GetValue( dwCode, sizeof(dwCode) ) );
	PACKET_GUARD_VOID( query_data->GetValue( dwEndDate, sizeof(dwEndDate) ) );

	PACKET_GUARD_VOID( query_data->GetValue( (char*)&RecentlyRewardDate, sizeof(DBTIMESTAMP) ) );
	DWORD dwRecentlyDate	= Help::ConvertDBTIMESTAMPToDWORD(RecentlyRewardDate);

	UserCashTable stInfo;

	stInfo.m_dwItemCode		= dwCode;
	stInfo.m_dwReceiveDate	= dwRecentlyDate;
	stInfo.m_dwEndDate		= dwEndDate;

	m_vTimeCashTable.push_back(stInfo);

	if( !IsActive() )
		SetActive(TRUE);

	//billing�� ���ʽ� ĳ�� ���� ��û.
	ioHashString szBillingGUID	= szGUID;
	m_pUser->RequestTimeCash(dwCode, szBillingGUID, TRUE);
}

void UserTimeCashTable::DeleteExpiredData(const DWORD dwCode)
{
	vTimeCashTable_iter it	= m_vTimeCashTable.begin();

	for( ; it != m_vTimeCashTable.end(); it++ )
	{
		UserCashTable& stInfo	= (*it);
		if( stInfo.m_dwItemCode == dwCode )
		{
			m_vTimeCashTable.erase(it);
			return;
		}
	}
}

void UserTimeCashTable::DisposeUpdateReceiveData(const DWORD dwCode, const int iResult, const DWORD dwReceiveDate, ioHashString& szBillingGUID)
{
	if( !m_pUser )
		return;

	if( !IsActive() )
		SetActive(TRUE);

	switch( iResult )
	{
	case UTCT_SUCCESS:
		{
			if( !UpdateCashTable(dwCode, dwReceiveDate) )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][timecash] time table not exist this code : [%d][%d]", m_pUser->GetUserIndex(), dwCode); 
				return;
			}

			//billing�� ���ʽ� ĳ�� ���� ��û.
			//ioHashString szBillingGUID	= szGUID;
			m_pUser->RequestTimeCash(dwCode, szBillingGUID, FALSE);
		}
		break;

	case UTCT_EXPIRE:
		{
			DeleteExpiredData(dwCode);

			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][timecash] time cash item is expired : [%d][%d]", m_pUser->GetUserIndex(), dwCode); 
			SP2Packet kPacket(STPK_TIME_CASH_INFO);
			PACKET_GUARD_VOID( kPacket.Write(TIME_CASH_ITEM_EXPIRE) );
			PACKET_GUARD_VOID( kPacket.Write(dwCode) );
			m_pUser->SendMessage(kPacket);
		}
		break;

	case UTCT_FAIL:
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][timecash]invalid time cash perform : [%d][%d]", m_pUser->GetUserIndex(), dwCode); 
		}
	}
}

BOOL UserTimeCashTable::UpdateCashTable(const DWORD dwCode, const DWORD dwReceiveDate)
{
	int iSize = m_vTimeCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		if( dwCode == m_vTimeCashTable[i].m_dwItemCode )
		{
			m_vTimeCashTable[i].m_dwReceiveDate	= dwReceiveDate;
			return TRUE;
		}
	}

	return FALSE;
}

void UserTimeCashTable::GetCashTable(const DWORD dwCode, UserCashTable& stInfo)
{
	int iSize = m_vTimeCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		if( dwCode == m_vTimeCashTable[i].m_dwItemCode )
		{
			stInfo	= m_vTimeCashTable[i];
		}
	}
}

BOOL UserTimeCashTable::IsRenewalDate(const DWORD dwItemCode)
{
	UserCashTable stInfo;
	GetCashTable(dwItemCode, stInfo);

	if( 0 == stInfo.m_dwItemCode )
		return FALSE;

	if( 0 == stInfo.m_dwReceiveDate )
		return FALSE;

	CTime cCurTime = CTime::GetCurrentTime();

	DWORD dwTodayStandardDate = g_TimeCashMgr.GetStandardDate(cCurTime.GetTime());
	DWORD dwPrevStandardDate = g_TimeCashMgr.GetStandardDate(stInfo.m_dwReceiveDate);

	if( stInfo.m_dwEndDate < cCurTime.GetTime() )
		return FALSE;

	if( dwTodayStandardDate > dwPrevStandardDate )
		return TRUE;

	return FALSE;
}

BOOL UserTimeCashTable::IsExpired(const DWORD dwEndDate)
{
	if( !m_pUser )
		return TRUE;

	if( 0 == dwEndDate )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][timecash]user end date is zero : [%d]", m_pUser->GetUserIndex() ); 
		return TRUE;
	}

	CTime cCurTime = CTime::GetCurrentTime();
	CTime cEndTime(dwEndDate);

	if( cEndTime > cCurTime )
		return FALSE;

	return TRUE;
}