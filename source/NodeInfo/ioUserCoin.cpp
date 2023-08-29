
#include "stdafx.h"
#include "ioUserCoin.h"

#include "../MainProcess.h"
#include "../EtcHelpFunc.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"

ioUserCoin::ioUserCoin() : m_DBGapTime(0)
{
	Init();
}

ioUserCoin::~ioUserCoin()
{
	Destroy();
}

void ioUserCoin::Init()
{
	m_bNeedResult = false;
	m_DBGapTime = 0;
}

void ioUserCoin::Destroy()
{
}

void ioUserCoin::UpdateCoinData( USER_COIN_TYPE eType )
{

	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	BYTE byType = (BYTE)eType;


	UserCoinInfo * pCoinInfo = FindCoinInfo(eType);

	if(pCoinInfo)
	{
		g_DBClient.OnUpdateUserCoin( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), byType);
		// �ϴ��� �����ð����� ������Ʈ �ص�
		pCoinInfo->m_lastUpdateTIme = CTime::GetCurrentTime();
	}
	else
	{
		g_DBClient.OnInsertUserCoin( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), byType);
		UserCoinInfo info;
		// �ϴ��� �����ð����� ������Ʈ �ص�
		info.m_lastUpdateTIme = CTime::GetCurrentTime();
		info.m_eType = eType;
		m_vCoinList.push_back(info);
	}
	m_bNeedResult = true;
}

void ioUserCoin::UpdateCoinTime( USER_COIN_TYPE eType, CTime & saveTime )
{
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	BYTE byType = (BYTE)eType;


	UserCoinInfo * pCoinInfo = FindCoinInfo(eType);

	if(pCoinInfo)
	{
		// �ѽð� �̻� ���� ���� db�����ð� �����صα�.
		if(pCoinInfo->m_lastUpdateTIme.GetHour() != saveTime.GetHour())
		{
			CString strServerTime;
			strServerTime = pCoinInfo->m_lastUpdateTIme.Format("%c");
			CString strDBTime;
			strDBTime = saveTime.Format("%c");

			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "Raid : %s, Time is different !! : server : %s, DB : %s ", __FUNCTION__, strServerTime.GetString(), strDBTime.GetString());
			m_DBGapTime = pCoinInfo->m_lastUpdateTIme - saveTime;

		}
		pCoinInfo->m_lastUpdateTIme = saveTime;
	}
	else
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "Raid : %s, Empty Type !! type : %d ", __FUNCTION__, byType);
	}
	m_bNeedResult = false;
}

bool ioUserCoin::GetLastCoinTime( CTime & outCoinTime, USER_COIN_TYPE eType )
{
	UserCoinInfo * pCoinInfo = FindCoinInfo(eType);
	if(pCoinInfo)
	{
		outCoinTime = pCoinInfo->m_lastUpdateTIme;

		//db �����ð� ���ؼ� ����
		if(m_DBGapTime != 0)
			outCoinTime += m_DBGapTime;

		return true;
	}
	else
	{
		return false;
	}
}

UserCoinInfo * ioUserCoin::FindCoinInfo( USER_COIN_TYPE eType )
{
	auto _funcFind = [&] (UserCoinInfo & info){
		return info.m_eType == eType;
	};

	auto it = std::find_if(m_vCoinList.begin(), m_vCoinList.end(), _funcFind);
	if(it != m_vCoinList.end())
	{
		UserCoinInfo & rkInfo = (*it);
		return &rkInfo;
	}
	else
	{
		return NULL;
	}
}

void ioUserCoin::Initialize( User *pUser )
{
	SetUser( pUser );
	m_vCoinList.clear();
}

bool ioUserCoin::DBtoNewIndex( DWORD dwIndex )
{
	return true;
}

void ioUserCoin::DBtoData( CQueryResultData *query_data )
{
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	// ���� �ϸ� �ȵ�
	// ���� ���³༮�� ������� 
	//m_vCoinList.clear();

	// ���� ���� ���� �ϳ��� ������ 
	if(query_data->IsExist() == false)
	{
		// ���̵� ���� ����
		UpdateCoinData(USER_COIN_RAID);
	}

	LOOP_GUARD();
	while( query_data->IsExist() )
	{
		DBTIMESTAMP updateTime;
		BYTE byCoinType = 0;
		PACKET_GUARD_VOID( query_data->GetValue( (char*)&updateTime, sizeof(DBTIMESTAMP) ) );
		PACKET_GUARD_VOID( query_data->GetValue( byCoinType, sizeof(byCoinType) ) );

		CTime lastUpdateTime(Help::GetSafeValueForCTimeConstructor(updateTime.year, updateTime.month, updateTime.day, 
			updateTime.hour, updateTime.minute, updateTime.second));


		// ���� ���̹Ƿ� ���� �־���.
		UserCoinInfo info;
		info.m_eType = (USER_COIN_TYPE)byCoinType;
		info.m_lastUpdateTIme = lastUpdateTime;

		// ���Ϳ� �ֱ�.
		m_vCoinList.push_back(info);
	}
	LOOP_GUARD_CLEAR();


}

void ioUserCoin::SaveData()
{
	// ������.
}

void ioUserCoin::FillMoveData( SP2Packet &rkPacket )
{
	DWORD coinSize = m_vCoinList.size();

	PACKET_GUARD_VOID( rkPacket.Write(coinSize) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bNeedResult) );
	for(DWORD i = 0; i < coinSize; ++i)
	{
		BYTE byType = m_vCoinList[i].m_eType;
		time_t  lastTime = m_vCoinList[i].m_lastUpdateTIme.GetTime();

		PACKET_GUARD_VOID( rkPacket.Write(byType) );
		PACKET_GUARD_VOID( rkPacket.Write(lastTime) );
	}
}

void ioUserCoin::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode /*= false*/ )
{
	DWORD coinSize = 0;
	PACKET_GUARD_VOID( rkPacket.Read(coinSize) );
	PACKET_GUARD_VOID( rkPacket.Read(m_bNeedResult) );
	for(DWORD i = 0; i < coinSize; ++i)
	{
		UserCoinInfo info;
		BYTE byType;
		time_t  lastTime;
		PACKET_GUARD_VOID( rkPacket.Read(byType) );
		PACKET_GUARD_VOID( rkPacket.Read(lastTime) );

		info.m_eType = (USER_COIN_TYPE)byType;
		info.m_lastUpdateTIme = CTime(lastTime);

		m_vCoinList.push_back(info);
	}

	if(m_bNeedResult)
	{
		if(m_pUser)
			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "UserCoin %s, Moving on DB NeedResult Time %d", __FUNCTION__,  m_pUser->GetUserIndex());
		else
			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "UserCoin %s, Moving on DB NeedResult Time __", __FUNCTION__);

	}

}

