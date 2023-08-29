// HeroRankManager.cpp: implementation of the HeroRankManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "../EtcHelpFunc.h"
#include "../ioEtcLogManager.h"

#include "UserNodeManager.h"
#include "HeroRankManager.h"
#include "LevelMatchManager.h"

#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "../MainServerNode/MainServerNode.h"

//#include "../Window.h"
#include "../MainProcess.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../Local/ioLocalParent.h"
#include "../Local/ioLocalManager.h"
#include <strsafe.h>

extern CLog EventLOG;
HeroRankManager *HeroRankManager::sg_Instance = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HeroRankManager::HeroRankManager()
{
	m_dwCurrentTime = 0;
	m_dwTop100SyncDate = 0;
	m_dwUserDataSyncDate = 0;
}

HeroRankManager::~HeroRankManager()
{
	m_HeroRankList.clear();
}

HeroRankManager &HeroRankManager::GetInstance()
{
	if( !sg_Instance )
		sg_Instance = new HeroRankManager;
	return *sg_Instance;
}

void HeroRankManager::ReleaseInstance()
{
	SAFEDELETE( sg_Instance );
}

void HeroRankManager::DBtoData( CQueryResultData *query_data )
{
	// ���� �������� Ȯ��
	int iStartRank = 0;
	query_data->GetValue( iStartRank, sizeof(int) );

	if( iStartRank == 1 )       // 1�� ���� ������ ������ ���� ���� ����
		m_HeroRankList.clear();
	
	LOOP_GUARD();
	while( query_data->IsExist() )
	{	
		HeroRankData kRankData;
		kRankData.m_iRank = iStartRank++;
		query_data->GetValue( kRankData.m_dwUserIndex, sizeof(DWORD) );    //���� �ε���
		query_data->GetValue( kRankData.m_iGradeLevel, sizeof(int) );	   //���� ���
		query_data->GetValue( kRankData.m_szPublicID, ID_NUM_PLUS_ONE );   //���� �г���
		query_data->GetValue( kRankData.m_iHeroTitle, sizeof(int) );       //���� Īȣ
		query_data->GetValue( kRankData.m_iHeroWin, sizeof(int) );		   //������ ��
		query_data->GetValue( kRankData.m_iHeroLose, sizeof(int) );        //������ ��
		query_data->GetValue( kRankData.m_iCampPos, sizeof(int) );         //���� ���� Ÿ��
		query_data->GetValue( kRankData.m_iHeroExpert, sizeof(int) );      //���� ������ ����ġ
		
		kRankData.m_bLogIn = g_UserNodeManager.IsPublicIDConnectUser( kRankData.m_dwUserIndex );
		m_HeroRankList.push_back( kRankData );

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "HeroRankManager::DBtoData %d�� (%s:%d:%d:%d:%d:%d:%d)", iStartRank - 1, 
								kRankData.m_szPublicID.c_str(), kRankData.m_iGradeLevel, kRankData.m_iHeroTitle,
								kRankData.m_iHeroWin, kRankData.m_iHeroLose, kRankData.m_iCampPos, kRankData.m_iHeroExpert );
	}
	LOOP_GUARD_CLEAR();
}

void HeroRankManager::UpdateProcess()
{
	if( TIMEGETTIME() - m_dwCurrentTime < 60000 ) return;

	m_dwCurrentTime = TIMEGETTIME();

	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����
	//
	CTime kCurTime = CTime::GetCurrentTime();
	{   // Top 100 ��������
		CTimeSpan kMinusTime( 0, Help::GetHeroTop100SyncHour(), 0, 0 );
		CTime kCheckTime = kCurTime - kMinusTime;
		DWORD dwCheckDate = Help::ConvertCTimeToYearMonthDay( kCheckTime );
		if( dwCheckDate != m_dwTop100SyncDate )
			OnSelectHeroRank( dwCheckDate );
	}

	{   // �α������� ������ ��ŷ - Īȣ - �ֱ� 6���� ��������
		CTimeSpan kMinusTime( 0, Help::GetHeroUserDataSyncHour(), 0, 0 );
		CTime kCheckTime = kCurTime - kMinusTime;
		DWORD dwCheckDate = Help::ConvertCTimeToYearMonthDay( kCheckTime );
		if( dwCheckDate != m_dwUserDataSyncDate )
		{
			m_dwUserDataSyncDate = dwCheckDate;
			g_UserNodeManager.UserNode_AllHeroDataSync();
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "HeroRankManager::UserDataSyncDate : %d", m_dwUserDataSyncDate );
		}
	}
}

void HeroRankManager::OnSelectHeroRank( DWORD dwUpdateDate /* = 0  */ )
{
	g_DBClient.OnSelectHeroTop100Data( 1, 100 );	

	// ���� ���� �ð� -
	if( dwUpdateDate == 0 )
	{
		CTime kCurTime = CTime::GetCurrentTime();
		CTimeSpan kMinusTime( 0, Help::GetHeroTop100SyncHour(), 0, 0 );
		CTime kCheckTime = kCurTime - kMinusTime;
		m_dwTop100SyncDate = Help::ConvertCTimeToYearMonthDay( kCheckTime );
	}
	else
	{
		m_dwTop100SyncDate = dwUpdateDate;
	}	
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "HeroRankManager::OnSelectHeroRank : %d", m_dwTop100SyncDate );
}

void HeroRankManager::CheckLogIn( DWORD dwUserIndex, const ioHashString &rkPublicID )
{
	vHeroRankData::iterator iter = m_HeroRankList.begin();
	for(;iter != m_HeroRankList.end();iter++)
	{
		HeroRankData &rkData = *iter;
		if( rkData.m_dwUserIndex == dwUserIndex )
		{
			rkData.m_bLogIn = true;
			rkData.m_szPublicID = rkPublicID;
			return;
		}
	}
}

void HeroRankManager::CheckLogOut( DWORD dwUserIndex )
{
	vHeroRankData::iterator iter = m_HeroRankList.begin();
	for(;iter != m_HeroRankList.end();iter++)
	{
		HeroRankData &rkData = *iter;
		if( rkData.m_dwUserIndex == dwUserIndex )
		{
			rkData.m_bLogIn = false;
			return;
		}
	}
}

void HeroRankManager::CheckCamp( DWORD dwUserIndex, int iCampPos )
{
	vHeroRankData::iterator iter = m_HeroRankList.begin();
	for(;iter != m_HeroRankList.end();iter++)
	{
		HeroRankData &rkData = *iter;
		if( rkData.m_dwUserIndex == dwUserIndex )
		{
			rkData.m_iCampPos = iCampPos;
			return;
		}
	}
}

void HeroRankManager::SendCurTop100Data( User *pUser, int iCurPage, int iMaxCount )
{
	if( pUser == NULL ) return;

	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	int iMaxList = m_HeroRankList.size();
	if( iMaxList == 0 )
	{
		SP2Packet kPacket( STPK_HERO_TOP100_DATA );
		kPacket << 0 << 0;
		pUser->SendMessage( kPacket );
		return;
	}

	int iStartPos = iCurPage * iMaxCount;
	int iCurSize  = max( 0, min( iMaxList - iStartPos, iMaxCount ) );

	SP2Packet kPacket( STPK_HERO_TOP100_DATA );
	kPacket << iMaxList << iCurSize;
	for(int i = iStartPos; i < iStartPos + iCurSize;i++)
	{
		HeroRankData &rkData = m_HeroRankList[i];
		kPacket << rkData.m_iRank << rkData.m_iGradeLevel << rkData.m_szPublicID << rkData.m_iHeroTitle
				<< rkData.m_iHeroWin << rkData.m_iHeroLose << rkData.m_iCampPos << rkData.m_bLogIn;
	}
	pUser->SendMessage( kPacket );
}

int HeroRankManager::GetTop100UserExpert( int iRank )
{
	vHeroRankData::iterator iter = m_HeroRankList.begin();
	for(;iter != m_HeroRankList.end();iter++)
	{
		HeroRankData &rkData = *iter;
		if( rkData.m_iRank == iRank )
			return max( rkData.m_iHeroExpert, g_LevelMatchMgr.GetRoomEnterLevelMax() * 10 );
	}

	return g_LevelMatchMgr.GetRoomEnterLevelMax() * 10;
}