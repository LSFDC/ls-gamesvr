#include "stdafx.h"
#include "../EtcHelpFunc.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"

//#include "..\Window.h"
#include "../MainProcess.h"

#include "User.h"
#include "ioPresentHelper.h"

#include ".\ioeventusernode.h"

extern CLog EventLOG;

EventUserNode::EventUserNode()
{
	Clear();
}

EventUserNode::~EventUserNode()
{
	m_ValueVec.clear();
	m_BackupVec.clear();
}

void EventUserNode::SetSize( int iSize )
{
	m_ValueVec.clear();
	m_BackupVec.clear();

	for (int i = 0; i < iSize ; i++)
	{
		int iValue = 0;
		m_ValueVec.push_back( iValue );
		m_BackupVec.push_back( iValue );
	}
}

int EventUserNode::GetValue( int iArray )
{
	if( !COMPARE(iArray, 0, (int) m_ValueVec.size()) )
		return 0;

	return m_ValueVec[iArray];
}

void EventUserNode::SetValue( int iArray, int iValue )
{
	if( COMPARE( iArray, 0, (int) m_ValueVec.size() ) )
		m_ValueVec[iArray] = iValue;
}

EventType EventUserNode::GetType() const
{
	return m_eEventType;
}


void EventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue(0), GetValue(1) );
		Backup();
	}
}

void EventUserNode::UpdateFirst( User *pUser )
{

}

void EventUserNode::Backup()
{
	m_bMustSave = false;
	int iSize = m_ValueVec.size();
	for (int i = 0; i < iSize ; i++)
	{
		m_BackupVec[i] = m_ValueVec[i];
	}
}
bool EventUserNode::IsChange()
{
	if( m_bMustSave )
		return true;

	int iSize = m_ValueVec.size();
	for (int i = 0; i < iSize ; i++)
	{
		if( m_BackupVec[i] != m_ValueVec[i] )
			return true;
	}

	return false;
}

void EventUserNode::SetIndex( DWORD dwIndex )
{
	m_dwIndex = dwIndex;
}

void EventUserNode::FillMoveData( SP2Packet &rkPacket, bool bLog )
{
	rkPacket << m_dwIndex;
	if( bLog )
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "FillMoveData : Index: %d", m_dwIndex );
	int iSize = m_ValueVec.size();
	rkPacket << iSize;
	for (int i = 0; i < iSize ; i++)
	{
		rkPacket <<  m_ValueVec[i];
		if( bLog )
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "FillMoveData : Vaule%d : %d", i+1, m_ValueVec[i] );
	}

	bool bChange = IsChange();
	rkPacket << bChange;
	if( bLog )
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "FillMoveData : Change : %d", bChange );
}

void EventUserNode::ApplyMoveData( SP2Packet &rkPacket )
{
	int iEventType = 0;
	rkPacket >> m_dwIndex;
	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ApplyMoveData : Index: %d", m_dwIndex );
	int iSize = 0;
	rkPacket >> iSize;
	for (int i = 0; i < iSize ; i++)
	{
		int iValue = 0;
		rkPacket >>  iValue;
		SetValue( i, iValue );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ApplyMoveData : Vaule%d : %d", i+1, iValue );
	}

	bool bChange = false;
	rkPacket >> bChange;
	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ApplyMoveData : Change : %d", bChange );
	if( !bChange )
		Backup();
	else
		m_bMustSave = true;
}

int EventUserNode::GetFillMoveDataSize() 
{
	SP2Packet kPacket;
	FillMoveData( kPacket , false );
	return kPacket.GetBufferSize();
}

int EventUserNode::GetAddSize()
{
	return 0;
}

void EventUserNode::Init()
{

}

void EventUserNode::Process( User *pUser )
{

}

void EventUserNode::ProcessTime( User *pUser )
{

}

int EventUserNode::GetSize() const
{
	return m_ValueVec.size();
}

void EventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;
	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( 0 ), GetValue( 1 ), (int)GetType() );
}

void EventUserNode::SendData( User *pUser )
{

}

bool EventUserNode::IsEmptyValue()
{
	return false;
}

void EventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{

}

void EventUserNode::Clear()
{
	m_dwIndex    = 0;
	m_ValueVec.clear();
	m_BackupVec.clear();
	m_bMustSave  = false;
	m_eEventType = EVT_NONE;
	m_eModeCategory = MC_DEFAULT;
}

void EventUserNode::SetType( EventType eEventType )
{
	m_eEventType = eEventType;
}
//----------------------------------------------------------------------------------------------------------------------------------
ProposalEventUserNode::ProposalEventUserNode()
{
}

ProposalEventUserNode::~ProposalEventUserNode()
{

}

bool ProposalEventUserNode::IsGiveChar( const User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return false;

	if( GetValue( VA_PROPOSAL_CNT ) >= g_EventMgr.GetValue( GetType(), EA_MAX_PROPOSAL_CHAR ) )
	{
		if( GetValue( VA_GAVE_CHAR ) < g_EventMgr.GetValue( GetType(), EA_MAX_CHAR_DAY ) )
			return true;
	}

	return false;
}

void ProposalEventUserNode::SetValueGiveChar()
{
	int iCharCnt = GetValue( VA_GAVE_CHAR );
	iCharCnt++;
	SetValue( VA_ADD_GAVE_CHAR_CNT, GetValue( VA_ADD_GAVE_CHAR_CNT )+1 ); 
	int iProCnt = 0;

	if( iCharCnt >= g_EventMgr.GetValue( GetType(), EA_MAX_CHAR_DAY ) )
	{
		SetValue( VA_ADD_PROPOSAL_CNT, -GetValue( VA_PROPOSAL_CNT ) );
		SetValue( VA_PROPOSAL_CNT, 0 );
	}
	else
	{
		SetValue( VA_ADD_PROPOSAL_CNT,  -g_EventMgr.GetValue( GetType(), EA_MAX_PROPOSAL_CHAR ) );

		iProCnt = GetValue( VA_PROPOSAL_CNT );
		iProCnt -= g_EventMgr.GetValue( GetType(), EA_MAX_PROPOSAL_CHAR );
		if( iProCnt < 0 )
			iProCnt = 0;
		SetValue( VA_PROPOSAL_CNT , iProCnt );
	}

	SetValue(VA_GAVE_CHAR, iCharCnt );
	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "Set Event Value : Type %d : %d : %d", (int) GetType(), iProCnt, GetValue( VA_PROPOSAL_CNT ) );
}

int ProposalEventUserNode::GetAddSize()
{
	return ADD_PROPOSAL_SIZE;
}

void ProposalEventUserNode::Init()
{
	SetValue( VA_ADD_PROPOSAL_CNT, 0 );
	SetValue( VA_ADD_GAVE_CHAR_CNT, 0 );
}

void ProposalEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue(VA_ADD_PROPOSAL_CNT), GetValue(VA_ADD_GAVE_CHAR_CNT) );
		SetValue( VA_ADD_PROPOSAL_CNT, 0 );
		SetValue( VA_ADD_GAVE_CHAR_CNT, 0 );
		Backup();
	}
}

void ProposalEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;
	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_ADD_PROPOSAL_CNT ), GetValue( VA_ADD_GAVE_CHAR_CNT ), (int)GetType() );
}

//---------------------------------------------------------------------------------------------------------------------------------
CoinEventUserNode::CoinEventUserNode()
{

}

CoinEventUserNode::~CoinEventUserNode()
{

}

void CoinEventUserNode::Process( User *pUser ) 
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CoinEventUserNode::Process : pUser == NULL" );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	int iCurPlaySec = GetValue( VA_PLAY_SEC ) + ( MAX_EVENT_CHECK_MS/1000 );
	SetValue( VA_PLAY_SEC, iCurPlaySec );

	if( GetValue( VA_PLAY_SEC ) >= g_EventMgr.GetValue( GetType(), EA_MAX_PLAY_SEC ) )
	{
		int iType = 0;
		int iCoin = 0;
		SYSTEMTIME st;
		GetLocalTime(&st);
		if( st.wHour == g_EventMgr.GetValue( GetType(), EA_BURNING_HOUR_1 ) ||
			st.wHour == g_EventMgr.GetValue( GetType(), EA_BURNING_HOUR_2 ) )
		{
			iCoin = g_EventMgr.GetValue( GetType(), EA_BURNING_GIVE_CNT );
			iType = EA_BURNING_GIVE_CNT;
		}
		else
		{
			iCoin = g_EventMgr.GetValue( GetType(), EA_NORMAL_GIVE_CNT );
			iType = EA_NORMAL_GIVE_CNT;
		}

		SetValue( VA_PLAY_SEC, 0 );
		SetValue( VA_ADD_COIN_CNT, GetValue( VA_ADD_COIN_CNT ) + iCoin );

		int iTotalCoin = GetValue( VA_COIN_CNT );
		iTotalCoin += iCoin;
		SetValue( VA_COIN_CNT, iTotalCoin );

		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s Add Coin : %d:%d:%d",pUser->GetPublicID().c_str(), GetValue( VA_ADD_COIN_CNT ), iCoin, iTotalCoin );

		g_DBClient.OnInsertEventLog( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), iCoin );
		Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );

		// send
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int) GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << iType;
		kPacket << iCoin;
		kPacket << iTotalCoin;
		pUser->SendMessage( kPacket );
	}
}

void CoinEventUserNode::Init()
{
	SetValue( VA_ADD_COIN_CNT, 0 );
	SetValue( VA_PLAY_SEC, 0 );
}

void CoinEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		// game_event_save�� ����Ǿ����Ƿ� coinevent�� ����Ϸ��� ��쾾���� ��û�ؼ� game_event_save�� �ٽ� �����ؾ� ��.
		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue(VA_ADD_COIN_CNT), GetValue(VA_PLAY_SEC) ); // value1�� add += , value2�� set =
		SetValue( VA_ADD_COIN_CNT , 0 );
		Backup();
	}
}

void CoinEventUserNode::SetGradeUpCoin( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CoinEventUserNode::SetGradeUpCoin : pUser == NULL" );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	int iCoin = g_EventMgr.GetValue( GetType(), EA_GRADEUP_GIVE_CNT );
	SetValue( VA_ADD_COIN_CNT, GetValue( VA_ADD_COIN_CNT ) +  iCoin );

	int iTotalCoin = GetValue( VA_COIN_CNT );
	iTotalCoin += iCoin;
	SetValue( VA_COIN_CNT, iTotalCoin );
	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s Add Coin : %d:%d:%d",pUser->GetPublicID().c_str(), GetValue( VA_ADD_COIN_CNT ), iCoin ,iTotalCoin );

	g_DBClient.OnInsertEventLog( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), iCoin );
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int) GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EA_GRADEUP_GIVE_CNT;
	kPacket << iCoin;
	kPacket << iTotalCoin;
	pUser->SendMessage( kPacket );
}

int CoinEventUserNode::GetAddSize()
{
	return ADD_COIN_SIZE;
}

void CoinEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;
	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_ADD_COIN_CNT ), GetValue( VA_PLAY_SEC ), (int)GetType() );
}

//----------------------------------------------------------------------------------------------------------------------------
ExpEventUserNode::ExpEventUserNode()
{

}

ExpEventUserNode::~ExpEventUserNode()
{

}

void ExpEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void ExpEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool ExpEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool ExpEventUserNode::IsEventTime( const User *pUser, ModeCategory eModeCategory )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType(), eModeCategory ) )
	{
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );

	enum { MAX_CHECK = 2, MAX_VALUE = 3,};
	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_EXP_FIRST_EVENT_WEEK_ON_OFF,  EA_EXP_FIRST_START_TIME,  EA_EXP_FIRST_END_TIME,  
		                                         EA_EXP_SECOND_EVENT_WEEK_ON_OFF, EA_EXP_SECOND_START_TIME, EA_EXP_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0], eModeCategory ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1], eModeCategory ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2], eModeCategory ) ) )
				return true;
		}	
	}

	return false;
}

float ExpEventUserNode::GetEventPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory )
{
	if( !IsEventTime( pUser, eModeCategory ) )
	{
		return 0.0f;
	}

	float fEventBonus = ( (float) g_EventMgr.GetValue( GetType(), EA_EXP_PER, eModeCategory ) / 100.0f ); // 20 -> 0.20
	return fEventBonus;
}

bool ExpEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_EXP_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_EXP_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}

	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------

PesoEventUserNode::PesoEventUserNode()
{
}

PesoEventUserNode::~PesoEventUserNode()
{

}

void PesoEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void PesoEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool PesoEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool PesoEventUserNode::IsPesoTime( const User *pUser, ModeCategory eModeCategory )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType(), eModeCategory ) )
	{
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );

	enum { MAX_CHECK = 2, MAX_VALUE = 3,};
	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_FIRST_EVENT_WEEK_ON_OFF,  EA_FIRST_START_TIME,  EA_FIRST_END_TIME,  
		                                         EA_SECOND_EVENT_WEEK_ON_OFF, EA_SECOND_START_TIME, EA_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0], eModeCategory ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1], eModeCategory ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2], eModeCategory ) ) )
				return true;
		}	
	}

	return false;
}

float PesoEventUserNode::GetPesoPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory )
{
	if( !IsPesoTime( pUser, eModeCategory ) )
	{
		return 0.0f;
	}

	float fEventBonus = ( (float) g_EventMgr.GetValue( GetType(), EA_PESO_PER, eModeCategory ) / 100.0f ); // 20 -> 0.20

// �̹� �̺�Ʈ���� PC�� ���ʽ� ���� ����
// 	if( fPCRoomBonus != 0.0f )
// 	{
// 		float fReturnBonus = fEventBonus - fPCRoomBonus;
// 		if( fReturnBonus < 0.0f )
// 			fReturnBonus = 0.0f;
// 		return fReturnBonus;
// 	}

	return fEventBonus;
}

bool PesoEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}
	
	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------

FishingEventUserNode::FishingEventUserNode()
{
}

FishingEventUserNode::~FishingEventUserNode()
{

}

void FishingEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void FishingEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool FishingEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool FishingEventUserNode::IsEventTime( const User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );

	enum { MAX_CHECK = 2, MAX_VALUE = 3,};
	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_FS_FIRST_EVENT_WEEK_ON_OFF,  EA_FS_FIRST_START_TIME,  EA_FS_FIRST_END_TIME,  
		EA_FS_SECOND_EVENT_WEEK_ON_OFF, EA_FS_SECOND_START_TIME, EA_FS_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0] ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1] ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2] ) ) )
				return true;
		}	
	}

	return false;
}

bool FishingEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_FS_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_FS_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}

	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------
PlayTimeEventUserNode::PlayTimeEventUserNode()
{

}

PlayTimeEventUserNode::~PlayTimeEventUserNode()
{

}

void PlayTimeEventUserNode::Process( User *pUser ) 
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser is NULL", __FUNCTION__);
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	int iCurPlaySec = GetValue( VA_PLAYTIME_PLAY_SEC ) + ( MAX_EVENT_CHECK_MS/1000 );
	SetValue( VA_PLAYTIME_PLAY_SEC, iCurPlaySec );

	enum { MAX_LOOP = 5, };
	for (int i = 0; i < MAX_LOOP ; i++)
	{
		if( iCurPlaySec >= g_EventMgr.GetValue( GetType(), EA_PASS_PLAYTIME_5 - i ) )
		{
			if( IsGiftData( (GiftType)(GT_5_SOLDIER - i), GS_NONE ) )
			{
				SetGiftData( (GiftType)(GT_5_SOLDIER - i), GS_NOTICE );

				// send
				SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
				kPacket << (int) GetType();
				kPacket << EVENT_DATA_UPDATE_OK;
				kPacket << EVENT_DATA_UPDATE_PLAYTIME_NOTICE;
				kPacket << iCurPlaySec;
				kPacket << GetValue( VA_PLAYTIME_GET_GIFT );
				pUser->SendMessage( kPacket );

				// medal
				if( (GT_5_SOLDIER-i) == GT_5_SOLDIER )
				{
					enum 
					{
						JOKER_100HOUR_EVENT_MEDAL_INDEX = 160,
					};
				}

				EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ERROR !!!! Set Event Value (Process): ID:%s : Type %d : %d : %d", pUser->GetPublicID().c_str(), (int) GetType(), iCurPlaySec, GetValue( VA_PLAYTIME_GET_GIFT ) );
				break;
			}
		}
	}
}

void PlayTimeEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue(VA_PLAYTIME_PLAY_SEC), GetValue(VA_PLAYTIME_GET_GIFT) ); 
		Backup();
	}
}

void PlayTimeEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;
	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_PLAYTIME_PLAY_SEC ), GetValue( VA_PLAYTIME_GET_GIFT ), (int)GetType() );
}

bool PlayTimeEventUserNode::IsGift( GiftType eType, User *pUser, bool bPeso )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return false;

	if( bPeso )
	{
		if( eType == GT_3_SOLDIER || 
			eType == GT_4_SOLDIER || 
			eType == GT_5_SOLDIER )
		{
			return false;
		}
	}
	else
	{
		if( eType == GT_1_PESO || 
			eType == GT_2_PESO )
		{
			return false;
		}
	}

	int iArray = 0;
	if( eType == GT_1_PESO )
	{
		iArray = EA_PASS_PLAYTIME_1;
	}
	else if( eType == GT_2_PESO )
	{
		iArray = EA_PASS_PLAYTIME_2;
	}
	else if( eType == GT_3_SOLDIER )
	{
		iArray = EA_PASS_PLAYTIME_3;
	}
	else if( eType == GT_4_SOLDIER )
	{
		iArray = EA_PASS_PLAYTIME_4;
	}
	else if( eType == GT_5_SOLDIER )
	{
		iArray = EA_PASS_PLAYTIME_5;
	}
	else
	{
	
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "PlayTimeEventUserNode::IsGift : Error Type : %s : %d", pUser->GetPublicID().c_str(), (int) eType );
		return false;
	}


	if( !IsGiftData( eType, GS_NOTICE ) )
		return false;

	return true;
}

void PlayTimeEventUserNode::SetGift( GiftType eType, User *pUser, bool bPeso )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, " Set Gift pUser is NULL." );
		return;
	}

	SetGiftData( eType, GS_USED );
	
	int iPeso = 0;
	if( bPeso )
	{
		if( eType == GT_1_PESO )
		{
			iPeso = g_EventMgr.GetValue( GetType(), EA_GIFT_1 );
		}
		else if( eType == GT_2_PESO )
		{
			iPeso = g_EventMgr.GetValue( GetType(), EA_GIFT_2 );
		}

		pUser->AddMoney( iPeso );
		pUser->SaveUserData();
		g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, pUser, 0, 0, PRESENT_PESO, LogDBClient::PT_EVENT_PESO, iPeso, 0, 0, NULL);
	}

	bool bOffMouseBusy = false;
	if( bPeso )
		bOffMouseBusy = true;
	
	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int) GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_USED;
	kPacket << pUser->GetMoney();
	kPacket << GetValue( VA_PLAYTIME_GET_GIFT );
	kPacket << bOffMouseBusy; 
	kPacket << (int) eType;
	pUser->SendMessage( kPacket );

	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "Set Event Value : ID:%s : Type %d : %d : %d :%d : %I64d", pUser->GetPublicID().c_str(), (int) GetType(), GetValue( VA_PLAYTIME_PLAY_SEC), GetValue( VA_PLAYTIME_GET_GIFT ), iPeso, pUser->GetMoney() );
}

bool PlayTimeEventUserNode::IsGiftData( GiftType eType, GiftState eGiftState )
{
	if( eType == GT_1_PESO )
	{
		if( ( GetValue( VA_PLAYTIME_GET_GIFT ) % 10 ) == (int)eGiftState ) // XXXX[X]
			return true;
	}
	else if( eType == GT_2_PESO )
	{
		if( ( ( GetValue( VA_PLAYTIME_GET_GIFT ) % 100 ) / 10 ) == (int)eGiftState ) // XXX[X]X
			return true;
	}
	else if( eType == GT_3_SOLDIER )
	{
		if( ( ( GetValue( VA_PLAYTIME_GET_GIFT ) % 1000 ) / 100 ) == (int)eGiftState ) // XX[X]XX
			return true;
	}
	else if( eType == GT_4_SOLDIER )
	{
		if( ( ( GetValue( VA_PLAYTIME_GET_GIFT ) % 10000 ) / 1000 ) == (int)eGiftState ) // X[X]XXX
			return true;
	}
	else if( eType == GT_5_SOLDIER )
	{
		if( ( GetValue( VA_PLAYTIME_GET_GIFT ) / 10000 ) == (int)eGiftState ) // [X]XXXX
			return true;
	}
	
	return false;
}

void PlayTimeEventUserNode::SetGiftData( GiftType eGiftType , GiftState eGiftState )
{
	if( eGiftType == GT_1_PESO )
	{
		int iValue = GetValue( VA_PLAYTIME_GET_GIFT );
		int iPreValue = iValue % 10;
		iValue -= iPreValue; // �������� �����Ѵ�.
		iValue += (int)eGiftState; // XXXX[X]
		SetValue( VA_PLAYTIME_GET_GIFT, iValue );
	}
	else if( eGiftType == GT_2_PESO )
	{
		int iValue = GetValue( VA_PLAYTIME_GET_GIFT );
		int iPreValue = ( (iValue % 100 ) / 10 );
		iValue -= ( 10 * iPreValue ); // �������� �����Ѵ�.
		iValue += ( 10 * (int)eGiftState ); // XXX[X]X
		SetValue( VA_PLAYTIME_GET_GIFT, iValue );
	}
	else if( eGiftType == GT_3_SOLDIER )
	{
		int iValue = GetValue( VA_PLAYTIME_GET_GIFT );
		int iPreValue = ( ( iValue % 1000 ) / 100 );
		iValue -= ( 100 * iPreValue ); // �������� �����Ѵ�.
		iValue += ( 100 * (int)eGiftState ); // XX[X]XX
		SetValue( VA_PLAYTIME_GET_GIFT, iValue );
	}
	else if( eGiftType == GT_4_SOLDIER )
	{
		int iValue = GetValue( VA_PLAYTIME_GET_GIFT );
		int iPreValue = ( ( iValue % 10000 ) / 1000 );
		iValue -= ( 1000 * iPreValue ); // �������� �����Ѵ�.
		iValue += ( 1000 * (int)eGiftState ); // X[X]XXX
		SetValue( VA_PLAYTIME_GET_GIFT, iValue );
	}
	else if( eGiftType == GT_5_SOLDIER )
	{
		int iValue = GetValue( VA_PLAYTIME_GET_GIFT );
		int iPreValue = ( iValue / 10000 );
		iValue -= ( 10000 * iPreValue ); // �������� �����Ѵ�.
		iValue += ( 10000 * (int)eGiftState ); // [X]XXXX
		SetValue( VA_PLAYTIME_GET_GIFT, iValue );
	}
}

int PlayTimeEventUserNode::GetLimitSecond( GiftType eType )
{
	if( eType == GT_3_SOLDIER )
	{
		return g_EventMgr.GetValue( GetType(), EA_GIFT_3 );
	}
	else if( eType == GT_4_SOLDIER )
	{
		return g_EventMgr.GetValue( GetType(), EA_GIFT_4 );
	}
	else if( eType == GT_5_SOLDIER )
	{
		return g_EventMgr.GetValue( GetType(), EA_GIFT_5 );
	}

	return 0;
}

void PlayTimeEventUserNode::SendData( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser is NULL", __FUNCTION__);
		return;
	}

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int) GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_SECOND;
	kPacket << GetValue( VA_PLAYTIME_PLAY_SEC );
	kPacket << GetValue( VA_PLAYTIME_GET_GIFT );
	pUser->SendMessage( kPacket );
}

void PlayTimeEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	int iGiftType = 0;
	rkPacket >> iGiftType;

	if( !IsGift( (PlayTimeEventUserNode::GiftType) iGiftType, pUser, true ) )
	{
		SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
		kReturn << GetType();
		kReturn << EVENT_DATA_UPDATE_FAIL;
		pUser->SendMessage( kReturn );
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Error IsGift :%s : %d ", __FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );
		return;
	}

	SetGift( (PlayTimeEventUserNode::GiftType) iGiftType, pUser, true );
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}
//---------------------------------------------------------------------------------------------------------------------------------
ChildrenDayEventUserNode::ChildrenDayEventUserNode()
{
}

ChildrenDayEventUserNode::~ChildrenDayEventUserNode()
{
}

void ChildrenDayEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void ChildrenDayEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

bool ChildrenDayEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}
//---------------------------------------------------------------------------------------------------------------------------------
PesoBonusEventUserNode::PesoBonusEventUserNode()
{
}

PesoBonusEventUserNode::~PesoBonusEventUserNode()
{
}

void PesoBonusEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void PesoBonusEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

bool PesoBonusEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

void PesoBonusEventUserNode::SetPesoBonus( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "PesoBonusEventUserNode::SetPesoBonus : pUser == NULL" );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;
	
	__int64 iPrevPeso = pUser->GetMoney();
	int iBonusPeso = g_EventMgr.GetValue( GetType(), EA_BONUS_PESO );
	pUser->AddMoney( iBonusPeso );
	g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, pUser, 0, 0, LogDBClient::LET_ETC, LogDBClient::PT_EVENT_PESO, PRESENT_PESO, 0, iBonusPeso, NULL);
	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "PesoBonusEventUserNode::SetPesoBonus : %s - %I64d - > %I64d", pUser->GetPublicID().c_str(), iPrevPeso, pUser->GetMoney() );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << pUser->GetMoney();
	pUser->SendMessage( kPacket );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BuyCharNoLevelLimitEventUserNode::BuyCharNoLevelLimitEventUserNode()
{

}

BuyCharNoLevelLimitEventUserNode::~BuyCharNoLevelLimitEventUserNode()
{

}

void BuyCharNoLevelLimitEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void BuyCharNoLevelLimitEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool BuyCharNoLevelLimitEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool BuyCharNoLevelLimitEventUserNode::IsNoLevelLimit( const User *pUser, bool bCharCreate, int iMsgType, int iCharLimitSecond, int iCharPeriodType )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return false;

	if( bCharCreate )
	{
		if( iMsgType != CHAR_CREATE_NORMAL )
			return false;
	}
	else
	{
		if( iMsgType != CHAR_EXTEND_NORMAL )
			return false;
	}

	if( iCharPeriodType != CPT_TIME )
		return false;

	if( iCharLimitSecond != g_EventMgr.GetValue( GetType(), EA_CHAR_LIMIT_SECOND ) )
		return false;

	EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s Buy no level char :%d:%s [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), bCharCreate, iCharLimitSecond );
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------
GradeUpEventUserNode::GradeUpEventUserNode()
{

}

GradeUpEventUserNode::~GradeUpEventUserNode()
{

}

void GradeUpEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void GradeUpEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool GradeUpEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

void GradeUpEventUserNode::SetGift( User *pUser, int iGradeLevel )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	enum { MAX_LOOP = 4, };
	int iGradeArray[MAX_LOOP]={ g_EventMgr.GetValue( GetType(), EA_GRADEUP_1_GRADE) , 
	                            g_EventMgr.GetValue( GetType(), EA_GRADEUP_2_GRADE) ,
	                            g_EventMgr.GetValue( GetType(), EA_GRADEUP_3_GRADE) , 
	                            g_EventMgr.GetValue( GetType(), EA_GRADEUP_4_GRADE)  };

	int iPesoArray[MAX_LOOP]={ g_EventMgr.GetValue( GetType(), EA_GRADEUP_1_PESO) , 
		                       g_EventMgr.GetValue( GetType(), EA_GRADEUP_2_PESO) ,
		                       g_EventMgr.GetValue( GetType(), EA_GRADEUP_3_PESO) , 
		                       g_EventMgr.GetValue( GetType(), EA_GRADEUP_4_PESO)  };

	for (int i = 0; i < MAX_LOOP ; i++)
	{
		if( iGradeLevel == iGradeArray[i] )
		{
			if( iPesoArray[i] > 0 )
			{
				__int64 iPreMoney = pUser->GetMoney();
				pUser->AddMoney( iPesoArray[i] );
				g_LogDBClient.OnInsertPeso( pUser, iPesoArray[i], LogDBClient::PT_MGAME_OPEN );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, pUser, 0, 0, LogDBClient::LET_ETC, LogDBClient::PT_MGAME_OPEN, PRESENT_PESO, 0, iPesoArray[i], NULL);
				EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s AddMoney : %d)%s %d Peso / %I64d PreMoney / %I64d Money", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPesoArray[i], iPreMoney, pUser->GetMoney() );
			}
			return;
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------
PCRoomEventUserNode::PCRoomEventUserNode()
{
}

PCRoomEventUserNode::~PCRoomEventUserNode()
{
}

void PCRoomEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void PCRoomEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

bool PCRoomEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool PCRoomEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeType )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_PCROOM_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_PCROOM_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}
	
	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeType )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}

bool PCRoomEventUserNode::IsEventTime(  const User *pUser, ModeCategory eModeType )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );
	
	enum { MAX_CHECK = 2, MAX_VALUE = 3,};

	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_PCROOM_FIRST_EVENT_WEEK_ON_OFF,  EA_PCROOM_FIRST_START_TIME,  EA_PCROOM_FIRST_END_TIME,  
		                                         EA_PCROOM_SECOND_EVENT_WEEK_ON_OFF, EA_PCROOM_SECOND_START_TIME, EA_PCROOM_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0], eModeType ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1], eModeType ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2], eModeType ) ) )
				return true;
		}	
	}

	return false;
}

void PCRoomEventUserNode::SetPesoAndExpBonus( const User *pUser, float& fPesoBonus, float& fExpBonus, ModeCategory eModeType )
{
	if( !IsEventTime( pUser, eModeType ) )
	{
		fPesoBonus = Help::GetPCRoomBonusPeso();
		fExpBonus = Help::GetPCRoomBonusExp();
	}
	else
	{
		fPesoBonus = ( (float) g_EventMgr.GetValue( GetType(), EA_PCROOM_PESO_BONUS_PER, eModeType ) / 100.0f ); // 20 -> 0.20
		fExpBonus = ( (float) g_EventMgr.GetValue( GetType(), EA_PCROOM_EXP_BONUS_PER, eModeType ) / 100.0f );
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
ChanceMortmainCharEventUserNode::ChanceMortmainCharEventUserNode()
{

}

ChanceMortmainCharEventUserNode::~ChanceMortmainCharEventUserNode()
{

}

void ChanceMortmainCharEventUserNode::Init()
{
	SetValue( VA_CMC_MAGIC_CODE, 0 );
	SetValue( VA_CMC_MORTMAIN_CNT, 0 );
}

void ChanceMortmainCharEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_CMC_MAGIC_CODE ), GetValue( VA_CMC_MORTMAIN_CNT ) ); 
		Backup();
	}
}

void ChanceMortmainCharEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_CMC_MAGIC_CODE ), GetValue( VA_CMC_MORTMAIN_CNT ), (int)GetType() );
}

void ChanceMortmainCharEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	int iValue2 = GetValue( VA_CMC_MORTMAIN_CNT );
	int iBonusType = 0;
	int iValue1 = 0;
	rkPacket >> iValue1;
	if( iValue1 == -1 )         // �Ϲ� ����
	{
		if( GetValue( VA_CMC_MAGIC_CODE ) == -1 || GetValue( VA_CMC_MAGIC_CODE ) == -3 )
		{
			if( GetValue( VA_CMC_MAGIC_CODE ) == -3 )
				SetValue( VA_CMC_MAGIC_CODE, -2 );
			else
				SetValue( VA_CMC_MAGIC_CODE, 0 );

			iBonusType = pUser->ChanceMortmainCharEventLotto( iValue2 );
		}
		else
		{
			SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
			kReturn << GetType();
			kReturn << EVENT_DATA_UPDATE_FAIL;
			pUser->SendMessage( kReturn );			
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ChanceMortmainCharEventUserNode::OnRecievePacket. %s : %d : %d", pUser->GetPublicID().c_str(), iValue1, GetValue( VA_CMC_MAGIC_CODE ) );
			return;
		}
	}
	else if( iValue1 == -2 )    // �Ǿ��� ����
	{
		if( GetValue( VA_CMC_MAGIC_CODE ) == -2 || GetValue( VA_CMC_MAGIC_CODE ) == -3 )
		{
			if( GetValue( VA_CMC_MAGIC_CODE ) == -3 )
				SetValue( VA_CMC_MAGIC_CODE, -1 );
			else
				SetValue( VA_CMC_MAGIC_CODE, 0 );

			iBonusType = pUser->ChanceMortmainCharEventLotto( iValue2 );
		}
		else
		{
			SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
			kReturn << GetType();
			kReturn << EVENT_DATA_UPDATE_FAIL;
			pUser->SendMessage( kReturn );
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ChanceMortmainCharEventUserNode::OnRecievePacket. %s : %d : %d", pUser->GetPublicID().c_str(), iValue1, GetValue( VA_CMC_MAGIC_CODE ) );
			return;
		}
	}
	SetValue( VA_CMC_MORTMAIN_CNT, iValue2 );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_USED;
	kPacket << GetValue( VA_CMC_MAGIC_CODE );
	kPacket << GetValue( VA_CMC_MORTMAIN_CNT );
	kPacket << iBonusType;
	kPacket << pUser->GetMoney();
	pUser->SendMessage( kPacket );
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}

void ChanceMortmainCharEventUserNode::UpdatePlayTime( User *pUser, DWORD dwPlayTime )
{
	if( !pUser ) return;
	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;
	if( GetValue( VA_CMC_MAGIC_CODE ) < 0 ) return;        //  ������ ������� �ʾҴ�.
    
	int iCurTime = GetValue( VA_CMC_MAGIC_CODE ) + (int)dwPlayTime;
	if( iCurTime >= g_EventMgr.GetValue( GetType(), EA_CHANCE_MORTMAIN_CHAR_TIME ) )
	{
		if( pUser->IsPCRoomAuthority() )  //�Ǿ��� ����
		{
			SetValue( VA_CMC_MAGIC_CODE, -3 );
		}
		else	// �Ϲ� ���� 
		{
			SetValue( VA_CMC_MAGIC_CODE, -1 );
		}
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ChanceMortmainCharEventUserNode::UpdatePlayTime ADD Lotto : %s(%d) : %d = %d",
									 pUser->GetPublicID().c_str(), (int)pUser->IsPCRoomAuthority(), iCurTime, GetValue( VA_CMC_MAGIC_CODE ) );
	}	
	else
	{
		// �ð� ����
		SetValue( VA_CMC_MAGIC_CODE, iCurTime );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ChanceMortmainCharEventUserNode::UpdatePlayTime ADD Time : %s : %d",
									 pUser->GetPublicID().c_str(), GetValue( VA_CMC_MAGIC_CODE ) );
	}

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_SECOND;
	kPacket << GetValue( VA_CMC_MAGIC_CODE );
	kPacket << GetValue( VA_CMC_MORTMAIN_CNT );
	pUser->SendMessage( kPacket );
}
//---------------------------------------------------------------------------------------------------------------------------------
OneDayGoldItemEvent::OneDayGoldItemEvent()
{

}

OneDayGoldItemEvent::~OneDayGoldItemEvent()
{

}

void OneDayGoldItemEvent::Init()
{
	SetValue( VA_GOLD_ITEM_RECV_DATE, 0 );
}

void OneDayGoldItemEvent::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_GOLD_ITEM_RECV_DATE ), 0 ); 
		Backup();
	}
}

void OneDayGoldItemEvent::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_GOLD_ITEM_RECV_DATE ), 0, (int)GetType() );
}

int OneDayGoldItemEvent::GetEventCurrentDate()
{
	CTimeSpan cGapTime( 0, g_EventMgr.GetValue( GetType(), EA_GOLD_ITEM_INIT_DATE_HOUR ), 0, 0 );
	CTime cCurTime = CTime::GetCurrentTime() - cGapTime;
	int iCurDate = ( cCurTime.GetMonth() * 100 ) + cCurTime.GetDay();
	return iCurDate;
}

void OneDayGoldItemEvent::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	int iCurDate = GetEventCurrentDate();
	int iPrevDate= GetValue( VA_GOLD_ITEM_RECV_DATE );      // ������ ���� ��:��
	if( iCurDate != iPrevDate )
	{		
		// ���� ����
		ioHashString szSendID;
		bool  bAlarm = false;
		short iPresentType = 0;
		int   iPresentValue1 = 0;
		int   iPresentValue2 = 0;
		int   iPresentValue3 = 0;
		int   iPresentValue4 = 0;
		if( g_PresentHelper.GetOneDayEventPresent( szSendID, iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4, bAlarm ) != -1 )
		{
			// �ð� ������Ʈ
			SetValue( VA_GOLD_ITEM_RECV_DATE, iCurDate );
			// ���� Insert
			CTimeSpan cPresentGapTime( g_EventMgr.GetValue( GetType(), EA_GOLD_ITEM_PRESENT_LIMIT ), 0, 0, 0  );
			CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
			g_DBClient.OnInsertPresentData( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), szSendID, pUser->GetPublicID(), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4,
											(short)g_EventMgr.GetValue( GetType(), EA_GOLD_ITEM_PRESENT_MENT ), kPresentTime, (short)g_EventMgr.GetValue( GetType(), EA_GOLD_ITEM_PRESENT_ALARM ) );

			g_LogDBClient.OnInsertPresent( 0, szSendID, g_App.GetPublicIP().c_str(), pUser->GetUserIndex(), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4, LogDBClient::PST_RECIEVE, "Event:12" );
			
			pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

			// �������� �˸�
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_OK;
			kPacket << EVENT_DATA_UPDATE_ONEDAY_USE;
			kPacket << GetValue( VA_GOLD_ITEM_RECV_DATE );
			kPacket << iPresentType << iPresentValue1 << iPresentValue2 << iPresentValue3 << iPresentValue4 << bAlarm;
			pUser->SendMessage( kPacket );
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OneDayGoldItemEvent::OnRecievePacket %s : %d - > %d : (%d:%d:%d:%d:%d)", pUser->GetPublicID().c_str(), iPrevDate, GetValue( VA_GOLD_ITEM_RECV_DATE ), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4 );	
		}		
		else
		{
			// ���� �� ���� ��Ȳ�̴�. ��� �� �ٽ� �õ� �˸�
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_OK;
			kPacket << EVENT_DATA_UPDATE_ONEDAY_USE_FAILED;
			pUser->SendMessage( kPacket );
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OneDayGoldItemEvent::OnRecievePacket %s : %d - > %d : �߻��� �� ���� ���� �߻�!!!", pUser->GetPublicID().c_str(), iPrevDate, GetValue( VA_GOLD_ITEM_RECV_DATE ) );	
		}
	}
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}

void OneDayGoldItemEvent::CheckGoldItemDate( User *pUser )
{
	if( !pUser ) return;
	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) ) return;
	
	int iCurDate = GetEventCurrentDate();;
	int iPrevDate= GetValue( VA_GOLD_ITEM_RECV_DATE );      // ������ ���� ��:��
	if( iCurDate != iPrevDate )
	{
		// �ð� ������Ʈ�� ��� �������� ���� �� �Ѵ�.
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_ONEDAY_CHANGE;
		pUser->SendMessage( kPacket );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OneDayGoldItemEvent::CheckGoldItemDate Check Date: %s : %d - > %d", pUser->GetPublicID().c_str(), iPrevDate, iCurDate );	
	}		
}
//---------------------------------------------------------------------------------------------------------------------------------
DormancyUserEvent::DormancyUserEvent()
{
}

DormancyUserEvent::~DormancyUserEvent()
{
}

void DormancyUserEvent::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void DormancyUserEvent::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

bool DormancyUserEvent::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool DormancyUserEvent::CheckDormancyDateToPresent( User *pUser, CTime &rkEntryTime, CTime &rkConnectTime )
{
	if( !pUser ) return false;
	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) ) return false;

	DWORD dwCheckTime = g_EventMgr.GetValue( GetType(), EA_DORMANCY_USER_LIMIT_DATE );
	CTime cCheckTime( Help::GetSafeValueForCTimeConstructor( dwCheckTime / 10000, (dwCheckTime % 10000) / 100, dwCheckTime % 100, 0, 0, 0 ) );
	if( rkEntryTime < cCheckTime && rkConnectTime < cCheckTime )
	{
		// ���� Insert : �α��� �������� �����ϹǷ� Select�� ���� �ʰ� insert���Ѵ�.
		CTimeSpan cPresentGapTime( g_EventMgr.GetValue( GetType(), EA_DORMANCY_USER_PRESENT_LIMIT ), 0, 0, 0  );
		CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
		int iMaxPresent    = g_PresentHelper.GetMaxDormancyUserPresent();
		for(int i = 0;i < iMaxPresent;i++)
		{
			ioHashString szSendID;
			short iPresentType = 0;
			int   iPresentValue1 = 0;
			int   iPresentValue2 = 0;
			int   iPresentValue3 = 0;
			int   iPresentValue4 = 0;
			if( g_PresentHelper.GetDormancyUserPresent( i, szSendID, iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4 ) != -1 )
			{
				g_DBClient.OnInsertPresentData( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), szSendID, pUser->GetPublicID(), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4,
												(short)g_EventMgr.GetValue( GetType(), EA_DORMANCY_USER_PRESENT_MENT ), kPresentTime, 
												(short)g_EventMgr.GetValue( GetType(), EA_DORMANCY_USER_PRESENT_ALARM ) );

				g_LogDBClient.OnInsertPresent( 0, szSendID, g_App.GetPublicIP().c_str(), pUser->GetUserIndex(), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4, LogDBClient::PST_RECIEVE, "Event:13" );
				EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OneDayGoldItemEvent::CheckDormancyDateToPresent %s : (%d-%d) : (%d:%d:%d:%d:%d)", pUser->GetPublicID().c_str(), 
											 Help::ConvertCTimeToYearMonthDay( rkEntryTime ), Help::ConvertCTimeToYearMonthDay( rkConnectTime ), iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4 );	
			}
		}		
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------------------
PlayTimePresentEventUserNode::PlayTimePresentEventUserNode()
{

}

PlayTimePresentEventUserNode::~PlayTimePresentEventUserNode()
{

}

void PlayTimePresentEventUserNode::Init()
{
	SetValue( VA_PLAYTIME_PRESENT_TIME_CNT, 0 );
}

void PlayTimePresentEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ), 0 ); 
		Backup();
	}
}

void PlayTimePresentEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ), 0, (int)GetType() );
}

void PlayTimePresentEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
		kReturn << GetType();
		kReturn << EVENT_DATA_UPDATE_FAIL;
		pUser->SendMessage( kReturn );			
		return;
	}

	if( GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) != CHANCE_GET_PRESENT )
	{
		SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
		kReturn << (int)GetType();
		kReturn << EVENT_DATA_UPDATE_FAIL;
		pUser->SendMessage( kReturn );			
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Wrong value :[%s:%d] %d." , __FUNCTION__ , pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) );
		return;
	}

	SetValue( VA_PLAYTIME_PRESENT_TIME_CNT, 0 );
	g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType() );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_USED;
	pUser->SendMessage( kPacket );
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}

void PlayTimePresentEventUserNode::UpdatePlayTime( User *pUser, DWORD dwPlayTime )
{
	if( !pUser ) 
		return;
	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;
	if( GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) == CHANCE_GET_PRESENT )  //  ������ ������� �ʾҴ�.
		return;       

	int iCurTime = GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) + (int)dwPlayTime;
	if( iCurTime >= g_EventMgr.GetValue( GetType(), EA_PLAYTIME_PRESENT_TIME ) )
	{
		SetValue( VA_PLAYTIME_PRESENT_TIME_CNT, CHANCE_GET_PRESENT );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s ADD Chance : [%s:%d] : %d = %d", __FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), iCurTime, GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) );
	}	
	else
	{
		// �ð� ����
		SetValue( VA_PLAYTIME_PRESENT_TIME_CNT, iCurTime );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s ADD Time : [%s:%d] : %d",  __FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_PLAYTIME_PRESENT_TIME_CNT ) );
	}

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_PLAYTIME_SECOND;
	kPacket << GetValue( VA_PLAYTIME_PRESENT_TIME_CNT );
	pUser->SendMessage( kPacket );
}

//-----------------------------------------------------------------------------------------------------------------------------------
ChristmasEventUserNode::ChristmasEventUserNode()
{

}

ChristmasEventUserNode::~ChristmasEventUserNode()
{

}

void ChristmasEventUserNode::Init()
{
	SetValue( VA_CHRISTMAS_GET_GIFT_DATE, 0 );
}

void ChristmasEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_CHRISTMAS_GET_GIFT_DATE ), 0 ); 
		Backup();
	}
}

void ChristmasEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_CHRISTMAS_GET_GIFT_DATE ), 0, (int)GetType() );
}

void ChristmasEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
		kReturn << GetType();
		kReturn << EVENT_DATA_UPDATE_FAIL;
		pUser->SendMessage( kReturn );			
		return;
	}

	// ������ 1���� �ְ� �̹� ������ �޾Ҵٸ�
	if( g_EventMgr.GetValue( GetType(), EA_CHRISTMAS_IS_ONLY_ONE_GIFT ) == 1 && GetValue( VA_CHRISTMAS_GET_GIFT_DATE ) != 0 )
	{
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_CHRISTMAS_WARNING;
		pUser->SendMessage( kPacket );
		return;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);

	DWORD dwCurDate = ( st.wYear * 10000 ) + ( st.wMonth * 100 ) + st.wDay;
	if( dwCurDate == GetValue( VA_CHRISTMAS_GET_GIFT_DATE ) )
	{
		SP2Packet kReturn( STPK_EVENT_DATA_UPDATE );
		kReturn << (int)GetType();
		kReturn << EVENT_DATA_UPDATE_OK;
		kReturn << EVENT_DATA_UPDATE_CHRISTMAS_WARNING;
		pUser->SendMessage( kReturn );			
		return;
	}

	SetValue( VA_CHRISTMAS_GET_GIFT_DATE, dwCurDate );
	g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType() );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	kPacket << EVENT_DATA_UPDATE_CHRISTMAS_GET_GIFT;
	kPacket << dwCurDate;
	pUser->SendMessage( kPacket );
	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}
//----------------------------------------------------------------------------------------------------------------------------
BuyItemEventUserNode::BuyItemEventUserNode()
{
}

BuyItemEventUserNode::~BuyItemEventUserNode()
{

}

void BuyItemEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void BuyItemEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool BuyItemEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

void BuyItemEventUserNode::SendBuyPresent( User *pUser, bool bPeso, short iBuyType, int iBuyValue1, int iBuyValue2, int iBonusMilesage )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL", __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( bPeso )
	{
		if( g_EventMgr.GetValue( GetType(), EA_BUY_TYPE ) == 1 ) // 1:ĳ��
			return;
	}
	else
	{
		if( g_EventMgr.GetValue( GetType(), EA_BUY_TYPE ) == 0 ) // 0:���
			return;
	}

	g_PresentHelper.SendBuyPresent( pUser, GetType(), iBuyType, iBuyValue1, iBuyValue2, iBonusMilesage );

}

bool BuyItemEventUserNode::InsertUserPresentByBuyPresent( User *pUser, bool bPeso, DWORD dwRecvUserIndex, short iPresentType, int iBuyValue1, int iBuyValue2, bool bPresentEvent /*false*/ ) 
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return true;

	if( bPeso )
	{
		if( g_EventMgr.GetValue( GetType(), EA_BUY_TYPE ) == 1 ) // 1:ĳ��
			return true;
	}
	else
	{
		if( g_EventMgr.GetValue( GetType(), EA_BUY_TYPE ) == 0 ) // 0:���
			return true;
	}

	if( !g_PresentHelper.InsertUserPresentByBuyPresent( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), GetType(), pUser->GetUserIndex(), pUser->GetPublicID(), pUser->GetPublicIP(), dwRecvUserIndex, iPresentType, iBuyValue1, iBuyValue2, bPresentEvent ) )
		return false;

	return true;
}
//----------------------------------------------------------------------------------------------------------------------------
ExerciseSoldierEventUserNode::ExerciseSoldierEventUserNode()
{

}

ExerciseSoldierEventUserNode::~ExerciseSoldierEventUserNode()
{

}

void ExerciseSoldierEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void ExerciseSoldierEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool ExerciseSoldierEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}
//-----------------------------------------------------------------------------------------------------------------------------------
ConnectionTimeEventUserNode::ConnectionTimeEventUserNode()
{

}

ConnectionTimeEventUserNode::~ConnectionTimeEventUserNode()
{

}

void ConnectionTimeEventUserNode::Init()
{
	SetValue( VA_CT_GET_CHANCE_DATE, 0 );
	SetValue( VA_CT_IS_CHANCE, 1 ); // ������ 1�� �ְ� ����
}

void ConnectionTimeEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_CT_GET_CHANCE_DATE ), GetValue( VA_CT_IS_CHANCE ) ); 
		Backup();
	}
}

void ConnectionTimeEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_CT_GET_CHANCE_DATE ), GetValue( VA_CT_IS_CHANCE ), (int)GetType() );
}

void ConnectionTimeEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_EXCEPTION; 
		kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CT_IS_CHANCE );
		pUser->SendMessage( kPacket );	
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) event is over. ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
		return;
	}

	bool bUseChance = false;
	rkPacket >> bUseChance;

	if( bUseChance )
	{
		if( GetValue( VA_CT_IS_CHANCE ) == 0 )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_NO_CHANCE;
			kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CT_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Chance. ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
			return;
		}
		// ������Ʈ
		SetValue( VA_CT_GET_CHANCE_DATE, GetNextCheckTime() );
		SetValue( VA_CT_IS_CHANCE, 0 );

		// ���� Insert
		int iPresentReceiveType = 0;
		if( pUser->GetGradeLevel() <= g_EventMgr.GetValue( GetType(), EA_CT_PRESENT_LOW_USER_LEVEL ) )
			iPresentReceiveType = 1;
		else 
			iPresentReceiveType = 2;
		g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType(), iPresentReceiveType );

		// �������� �˸�
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_OK;
		kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CT_IS_CHANCE );
		pUser->SendMessage( kPacket );

		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Present %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CT_GET_CHANCE_DATE ) );	
	}
	else
	{
		if( GetValue( VA_CT_IS_CHANCE ) == 1 )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_EXCEPTION; 
			kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CT_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Chance 1. ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
			return;
		}

		CTime kCurTime = CTime::GetCurrentTime();
		int iCurCheckTime = GetCheckTime( kCurTime.GetMonth(), kCurTime.GetDay(), kCurTime.GetHour(), kCurTime.GetMinute() );
		if( iCurCheckTime < GetValue( VA_CT_GET_CHANCE_DATE ) )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_WANT_TIME;
			kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CT_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Time. %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CT_GET_CHANCE_DATE ) );	
			return;
		}

		SetValue( VA_CT_IS_CHANCE, 1 );

		// �������� �˸�
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_OK;
		kPacket << GetValue( VA_CT_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CT_IS_CHANCE );
		pUser->SendMessage( kPacket );

		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Chance %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CT_GET_CHANCE_DATE ) );	
	}

	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}

int ConnectionTimeEventUserNode::GetNextCheckTime()
{
	CTimeSpan kGapTime( 0, 0, g_EventMgr.GetValue( GetType(), EA_CT_CHANCE_TIME ), 0 );
	CTime kNextTime = CTime::GetCurrentTime() + kGapTime;
	int iTime = GetCheckTime(kNextTime.GetMonth(), kNextTime.GetDay(), kNextTime.GetHour(), kNextTime.GetMinute() );
	return iTime;
}

int ConnectionTimeEventUserNode::GetCheckTime( int iMonth, int iDays, int iHours, int iMins )
{
	int iTime = (iMonth * 1000000) + (iDays * 10000) + (iHours * 100) + iMins;
	return iTime;
}

void ConnectionTimeEventUserNode::UpdateFirst( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( GetValue( VA_CT_IS_CHANCE ) == 1 )
		return;

	CTime kCurTime = CTime::GetCurrentTime();
	int iCurCheckTime = GetCheckTime( kCurTime.GetMonth(), kCurTime.GetDay(), kCurTime.GetHour(), kCurTime.GetMinute() );
	if( iCurCheckTime >= GetValue( VA_CT_GET_CHANCE_DATE ) )
	{
		SetValue( VA_CT_IS_CHANCE, 1 );
		Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Chance %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CT_GET_CHANCE_DATE ) );	
	}
}
//----------------------------------------------------------------------------------------------------------------------------
OneDayGiftEventUserNode::OneDayGiftEventUserNode()
{
}

OneDayGiftEventUserNode::~OneDayGiftEventUserNode()
{

}

void OneDayGiftEventUserNode::Init()
{
	SetValue( VA_OG_GET_GIFT_DATE, 0 );
}

void OneDayGiftEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_OG_GET_GIFT_DATE ), 0 ); 
		Backup();
	}
}

void OneDayGiftEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_OG_GET_GIFT_DATE ), 0, (int)GetType() );
}

void OneDayGiftEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s %d:%s event is over." , __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
		return;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);

	DWORD dwCurDate = ( st.wYear * 10000 ) + ( st.wMonth * 100 ) + st.wDay;

	if( g_EventMgr.GetValue( GetType(), EA_OD_IS_EVERYDAY_GIFT ) == 1 )
	{
		if( GetValue( VA_OG_GET_GIFT_DATE ) == dwCurDate )
			return;

		if( g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType() ) )
			SetValue( VA_OG_GET_GIFT_DATE, dwCurDate );
	}
	else // �̺�Ʈ �Ⱓ�� 1���� ����
	{
		int iEventStartDate = g_EventMgr.GetStartDate( GetType() );
		int iEventEndDate   = g_EventMgr.GetEndDate( GetType() );

		int iUserGiftDate   = GetValue( VA_OG_GET_GIFT_DATE );

		if( iUserGiftDate >= iEventStartDate )
			return;

		if( g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType() ) )
			SetValue( VA_OG_GET_GIFT_DATE, dwCurDate );
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
GradePresentEventUserNode::GradePresentEventUserNode()
{

}

GradePresentEventUserNode::~GradePresentEventUserNode()
{

}

void GradePresentEventUserNode::Init()
{
	SetValue( VA_GP_CAN_RECEIVE_GIFT, 0 );
}

void GradePresentEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void GradePresentEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

void GradePresentEventUserNode::SetGift( User *pUser, int iGradeLevel )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( GetValue( VA_GP_CAN_RECEIVE_GIFT ) == 0 )
		return;

	if( iGradeLevel != g_EventMgr.GetValue( GetType(), EA_GP_GRADE_OF_PRESENT ) )
		return;


	g_PresentHelper.SendEventPresent( pUser, (DWORD)GetType() );
}

void GradePresentEventUserNode::SetCanReceiveGift( CTime &rkEntryTime )
{
	int iEntryDate = ( rkEntryTime.GetYear()*10000 ) + ( rkEntryTime.GetMonth()*100 ) + rkEntryTime.GetDay();

	if( COMPARE( iEntryDate, g_EventMgr.GetValue( GetType(), EA_GP_START_ENTRY_DATE ), g_EventMgr.GetValue( GetType(), EA_GP_END_ENTRY_DATE ) + 1 ) )
		SetValue( VA_GP_CAN_RECEIVE_GIFT, 1 );
	else
		SetValue( VA_GP_CAN_RECEIVE_GIFT, 0 );
}

//-----------------------------------------------------------------------------------------------------------------------------------
ConnectionTimeSelectGiftEventUserNode::ConnectionTimeSelectGiftEventUserNode()
{

}

ConnectionTimeSelectGiftEventUserNode::~ConnectionTimeSelectGiftEventUserNode()
{

}

void ConnectionTimeSelectGiftEventUserNode::Init()
{
	SetValue( VA_CTSG_GET_CHANCE_DATE, 0 );
	SetValue( VA_CTSG_IS_CHANCE, 1 ); // ������ 1�� �ְ� ����
}

void ConnectionTimeSelectGiftEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_CTSG_GET_CHANCE_DATE ), GetValue( VA_CTSG_IS_CHANCE ) ); 
		Backup();
	}
}

void ConnectionTimeSelectGiftEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_CTSG_GET_CHANCE_DATE ), GetValue( VA_CTSG_IS_CHANCE ), (int)GetType() );
}

void ConnectionTimeSelectGiftEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_EXCEPTION; 
		kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CTSG_IS_CHANCE );
		pUser->SendMessage( kPacket );	
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) event is over.",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
		return;
	}

	int iPresentReceiveType = 0;
	rkPacket >> iPresentReceiveType;

	if( iPresentReceiveType != -1 )
	{
		if( GetValue( VA_CTSG_IS_CHANCE ) == 0 )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_NO_CHANCE;
			kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CTSG_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Chance. ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
			return;
		}
		// ������Ʈ
		SetValue( VA_CTSG_GET_CHANCE_DATE, GetNextCheckTime() );
		SetValue( VA_CTSG_IS_CHANCE, 0 );

		// ���� Insert
		if( !g_PresentHelper.SendEventPresent( pUser, (DWORD) GetType(), iPresentReceiveType ) )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_EXCEPTION; 
			kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CTSG_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) It have not a present.(%d) ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), iPresentReceiveType );	
			return;
		}

		// �������� �˸�
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_OK;
		kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CTSG_IS_CHANCE );
		pUser->SendMessage( kPacket );

		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Present %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CTSG_GET_CHANCE_DATE ) );	
	}
	else
	{
		if( GetValue( VA_CTSG_IS_CHANCE ) == 1 )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << (int)GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_EXCEPTION; 
			kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CTSG_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Chance 1. ",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex() );	
			return;
		}

		CTime kCurTime = CTime::GetCurrentTime();
		int iCurCheckTime = GetCheckTime( kCurTime.GetMonth(), kCurTime.GetDay(), kCurTime.GetHour(), kCurTime.GetMinute() );
		if( iCurCheckTime < GetValue( VA_CTSG_GET_CHANCE_DATE ) )
		{
			SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
			kPacket << GetType();
			kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_WANT_TIME;
			kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
			kPacket << GetValue( VA_CTSG_IS_CHANCE );
			pUser->SendMessage( kPacket );	
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s (%s:%d) Error Time. %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CTSG_GET_CHANCE_DATE ) );	
			return;
		}

		SetValue( VA_CTSG_IS_CHANCE, 1 );

		// �������� �˸�
		SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
		kPacket << (int)GetType();
		kPacket << EVENT_DATA_UPDATE_OK;
		kPacket << EVENT_DATA_UPDATE_CONNECTION_TIME_SELECT_GIFT_OK;
		kPacket << GetValue( VA_CTSG_GET_CHANCE_DATE );
		kPacket << GetValue( VA_CTSG_IS_CHANCE );
		pUser->SendMessage( kPacket );

		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Chance %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CTSG_GET_CHANCE_DATE ) );	
	}

	Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
}

int ConnectionTimeSelectGiftEventUserNode::GetNextCheckTime()
{
	CTimeSpan kGapTime( 0, 0, g_EventMgr.GetValue( GetType(), EA_CTSG_CHANCE_TIME ), 0 );
	CTime kNextTime = CTime::GetCurrentTime() + kGapTime;
	int iTime = GetCheckTime(kNextTime.GetMonth(), kNextTime.GetDay(), kNextTime.GetHour(), kNextTime.GetMinute() );
	return iTime;
}

int ConnectionTimeSelectGiftEventUserNode::GetCheckTime( int iMonth, int iDays, int iHours, int iMins )
{
	int iTime = (iMonth * 1000000) + (iDays * 10000) + (iHours * 100) + iMins;
	return iTime;
}

void ConnectionTimeSelectGiftEventUserNode::UpdateFirst( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( GetValue( VA_CTSG_IS_CHANCE ) == 1 )
		return;

	CTime kCurTime = CTime::GetCurrentTime();
	int iCurCheckTime = GetCheckTime( kCurTime.GetMonth(), kCurTime.GetDay(), kCurTime.GetHour(), kCurTime.GetMinute() );
	if( iCurCheckTime >= GetValue( VA_CTSG_GET_CHANCE_DATE ) )
	{
		SetValue( VA_CTSG_IS_CHANCE, 1 );
		Save( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID() );
		EventLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%s:%d) Get Chance %d",__FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetUserIndex(), GetValue( VA_CTSG_GET_CHANCE_DATE ) );	
	}
}
//------------------------------------------------------------------------------------------------------------------------------------
EntryEventUserNode::EntryEventUserNode()
{

}

EntryEventUserNode::~EntryEventUserNode()
{

}

void EntryEventUserNode::Init()
{
	SetValue( VA_E_GET_GIFT, 0 );
	SetValue( VA_E_CAN_RECEIVE_GIFT, 0 ); // �޸� ����
}

void EntryEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_E_GET_GIFT ), 0 ); 
		Backup();
	}
}

void EntryEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_E_GET_GIFT ), 0, (int)GetType() );
}

int EntryEventUserNode::GetAddSize()
{
	return ADD_ENTRY_SIZE;
}

void EntryEventUserNode::SetCanReceiveGift( CTime &rkEntryTime )
{
	int iEntryDate = ( rkEntryTime.GetYear()*10000 ) + ( rkEntryTime.GetMonth()*100 ) + rkEntryTime.GetDay();

	if( !COMPARE( iEntryDate, g_EventMgr.GetValue( GetType(), EA_E_START_ENTRY_DATE ), g_EventMgr.GetValue( GetType(), EA_E_END_ENTRY_DATE ) + 1 ) )
		return;

	SetValue( VA_E_CAN_RECEIVE_GIFT, 1 );
}

void EntryEventUserNode::SetGift( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( GetValue( VA_E_CAN_RECEIVE_GIFT ) == 0 )
		return;

	if( GetValue( VA_E_GET_GIFT ) == 1 )
		return;
	
	if( g_PresentHelper.SendEventPresent( pUser, (DWORD)GetType() ) )
		SetValue( VA_E_GET_GIFT, 1 );
}

//----------------------------------------------------------------------------------------------------------------------------
LadderPointEventUserNode::LadderPointEventUserNode()
{

}

LadderPointEventUserNode::~LadderPointEventUserNode()
{

}

//------------------------------------------------------------------------------------------------------------------------------------
EntryAfterEventUserNode::EntryAfterEventUserNode()
{

}

EntryAfterEventUserNode::~EntryAfterEventUserNode()
{

}

void EntryAfterEventUserNode::Init()
{
	SetValue( VA_EA_GET_GIFT, 0 );
	SetValue( VA_EA_CAN_RECEIVE_GIFT, 0 ); // �޸� ����
}

void EntryAfterEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_EA_GET_GIFT ), 0 ); 
		Backup();
	}
}

void EntryAfterEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_EA_GET_GIFT ), 0, (int)GetType() );
}

int EntryAfterEventUserNode::GetAddSize()
{
	return ADD_ENTRY_AFTER_SIZE;
}

void EntryAfterEventUserNode::SetCanReceiveGift( CTime &rkEntryTime, int iUserState )
{
	int iEntryDate = ( (rkEntryTime.GetYear()%100)*1000000 ) + ( rkEntryTime.GetMonth()*10000 ) + (rkEntryTime.GetDay()*100 ) + rkEntryTime.GetHour();

	if( iEntryDate > g_EventMgr.GetValue( GetType(), EA_EA_GIFT_ENTRY_DATE_AFTER )  )
		return;

	if( iUserState != US_TUTORIAL_CLEAR )
	{
		SetValue( VA_EA_GET_GIFT, 1 ); // ����Ʈ�� ������ �ް� �ǹǷ� �̺�Ʈ���� �޾Ҵٰ� ó��
		return;
	}

	SetValue( VA_EA_CAN_RECEIVE_GIFT, 1 );
}

void EntryAfterEventUserNode::SetGift( User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL." , __FUNCTION__ );
		return;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( GetValue( VA_EA_CAN_RECEIVE_GIFT ) == 0 )
		return;

	if( GetValue( VA_EA_GET_GIFT ) == 1 )
		return;

	if( g_PresentHelper.SendEventPresent( pUser, (DWORD)GetType() ) )
		SetValue( VA_EA_GET_GIFT, 1 );
}
//----------------------------------------------------------------------------------------------------------------------------
ConnectAndPlayTimeEventUserNode::ConnectAndPlayTimeEventUserNode()
{
}

ConnectAndPlayTimeEventUserNode::~ConnectAndPlayTimeEventUserNode()
{

}

void ConnectAndPlayTimeEventUserNode::Init()
{
	SetValue( VA_CAP_CONNECT_RESET_DATE, 0 );
	SetValue( VA_CAP_POINT_AND_SEC, 0 );
}

void ConnectAndPlayTimeEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	if( IsChange() )
	{
		if( m_dwIndex == 0 ) return;
		if( m_ValueVec.empty() ) return;

		g_DBClient.OnUpdateEventData( dwAgentID, dwThreadID, m_dwIndex, GetValue( VA_CAP_CONNECT_RESET_DATE ), GetValue( VA_CAP_POINT_AND_SEC ) ); 
		Backup();
	}
}

void ConnectAndPlayTimeEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	if( m_dwIndex != 0 )
		return;

	g_DBClient.OnInsertEventData( dwAgentID, dwThreadID, szUserGUID, dwUserIndex, GetValue( VA_CAP_CONNECT_RESET_DATE ), GetValue( VA_CAP_POINT_AND_SEC ), (int)GetType() );
}

void ConnectAndPlayTimeEventUserNode::ProcessTime( User *pUser )
{
	if( !pUser )
		return;

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( GetResetDate() != GetCurrentDate() )
	{
		int iNewConnectResetDate = GetConnectResetDate( GetConnectDate(), GetCurrentDate()  );
		SetValue( VA_CAP_CONNECT_RESET_DATE, iNewConnectResetDate );
		SetValue( VA_CAP_POINT_AND_SEC, 0 );
	}

	int iPoint = GetPoint();
	if( iPoint >= g_EventMgr.GetValue( GetType(), EA_CAP_MAX_POINT ) )
		return;

	int iSec = GetSec() + MAX_EVENT_TIME_CHECK_SEC;
	if( iSec >= 10000 )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s %d:%s error play sec [%d:%d]." , __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str()
			                                                                          , GetValue( VA_CAP_CONNECT_RESET_DATE ) , GetValue( VA_CAP_POINT_AND_SEC ));
		return;
	}

	// sec update
	if( iSec < g_EventMgr.GetValue( GetType(), EA_CAP_MAX_SEC ) )
	{
		int iNewPointAndSec = GetPointAndSec( GetPoint(), iSec );
		SetValue( VA_CAP_POINT_AND_SEC, iNewPointAndSec );	
		return;
	}

	// db insert
	iPoint++;
	int iNewPointAndSec = GetPointAndSec( iPoint, 0 );  // 0�� Sec �ʱ�ȭ
	SetValue( VA_CAP_POINT_AND_SEC, iNewPointAndSec );	
	g_DBClient.OnInsertEventMarble( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), pUser->GetPublicID(), (int)pUser->GetChannelingType() );

	// send
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int) GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	pUser->SendMessage( kPacket );

	EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s %d:%s time get [%d:%d]." , __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str()
		                                                                    , GetValue( VA_CAP_CONNECT_RESET_DATE ) , GetValue( VA_CAP_POINT_AND_SEC ) );
}

void ConnectAndPlayTimeEventUserNode::OnRecievePacket( User *pUser, SP2Packet &rkPacket )
{
	if( !pUser )
		return;

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
		return;

	if( GetConnectDate() == GetCurrentDate() )
		return;

	g_DBClient.OnInsertEventMarble( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), pUser->GetPublicID(), (int)pUser->GetChannelingType() );
	int iNewConnectResetDate = GetConnectResetDate( GetCurrentDate(), GetResetDate() );
	SetValue( VA_CAP_CONNECT_RESET_DATE, iNewConnectResetDate );

	// �������� �˸�
	SP2Packet kPacket( STPK_EVENT_DATA_UPDATE );
	kPacket << (int)GetType();
	kPacket << EVENT_DATA_UPDATE_OK;
	pUser->SendMessage( kPacket );

	EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s %d:%s connect get [%d:%d]." , __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str()
		                                                                       , GetValue( VA_CAP_CONNECT_RESET_DATE ) , GetValue( VA_CAP_POINT_AND_SEC ) );
}


int ConnectAndPlayTimeEventUserNode::GetPoint()
{
	// [0000]0000
	return ( GetValue( VA_CAP_POINT_AND_SEC )/10000 );
}

int ConnectAndPlayTimeEventUserNode::GetSec()
{
	// 0000[0000]
	return GetValue( VA_CAP_POINT_AND_SEC )%10000;
}

int ConnectAndPlayTimeEventUserNode::GetPointAndSec( int iPoint, int iSec )
{
	return (iPoint*10000) + iSec;
}

int ConnectAndPlayTimeEventUserNode::GetConnectDate()
{
	// [0000]0000
	return ( GetValue( VA_CAP_CONNECT_RESET_DATE)/10000 );
}

int ConnectAndPlayTimeEventUserNode::GetResetDate()
{
	// 0000[0000]
	return GetValue( VA_CAP_CONNECT_RESET_DATE )%10000;
}

int ConnectAndPlayTimeEventUserNode::GetConnectResetDate( int iConnectDate, int iResetDate )
{
	return (iConnectDate*10000) + iResetDate;
}

int ConnectAndPlayTimeEventUserNode::GetCurrentDate()
{
	return ( g_EventMgr.GetCurrentDate()%10000 ); // mmdd
}

//----------------------------------------------------------------------------------------------------------------------------
PlazaMonsterEventUserNode::PlazaMonsterEventUserNode()
{
}

PlazaMonsterEventUserNode::~PlazaMonsterEventUserNode()
{
}

void PlazaMonsterEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void PlazaMonsterEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool PlazaMonsterEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool PlazaMonsterEventUserNode::IsEventTime( User* pUser )
{
	// Check : IsAlive
	if( ! g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		return false;
	}

	// Check : Day & Time
	if( g_EventMgr.IsEventTime( GetType() ) == true )
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------------
ModeBonusEventUserNode::ModeBonusEventUserNode()
{
}

ModeBonusEventUserNode::~ModeBonusEventUserNode()
{
}

void ModeBonusEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void ModeBonusEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool ModeBonusEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool ModeBonusEventUserNode::IsEventTime( const User *pUser, ModeCategory eModeCategory )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType(), eModeCategory ) )
	{
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );

	enum { MAX_CHECK = 2, MAX_VALUE = 3,};
	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_MB_EXP_FIRST_EVENT_WEEK_ON_OFF,  EA_MB_EXP_FIRST_START_TIME,  EA_MB_EXP_FIRST_END_TIME,  
		                                         EA_MB_EXP_SECOND_EVENT_WEEK_ON_OFF, EA_MB_EXP_SECOND_START_TIME, EA_MB_EXP_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0], eModeCategory ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1], eModeCategory ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2], eModeCategory ) ) )
				return true;
		}	
	}

	return false;
}

float ModeBonusEventUserNode::GetEventPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory )
{
	if( !IsEventTime( pUser, eModeCategory ) )
	{
		return 0.0f;
	}

	float fEventBonus = ( (float) g_EventMgr.GetValue( GetType(), EA_MB_EXP_PER, eModeCategory ) / 100.0f ); // 20 -> 0.20
	return fEventBonus;
}

bool ModeBonusEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_MB_EXP_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_MB_EXP_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}

	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray, eModeCategory )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}

bool ModeBonusEventUserNode::IsEventMode( const int iMode, ModeCategory eModeCategory )
{
	if( eModeCategory == MC_SHUFFLE )
		return true;

	enum { EVENT_MODE_OFF = 1, EVENT_MODE_ON = 2, };

	int iOnOff = 0;
	int iValueArray = 0;

	switch( iMode )
	{
	case MT_SYMBOL:
	case MT_UNDERWEAR:
	case MT_CBT:
	case MT_CATCH:
	case MT_KING:
	case MT_TRAINING:
	case MT_SURVIVAL:
	case MT_TEAM_SURVIVAL:
	case MT_DOBULE_CROWN:
	case MT_BOSS:
	case MT_MONSTER_SURVIVAL:
	case MT_FOOTBALL:
	case MT_TEAM_SURVIVAL_AI:
		iValueArray = EA_MB_LIST1;
		break;

	case MT_HEROMATCH:
	case MT_GANGSI:
	case MT_DUNGEON_A:
	case MT_HEADQUARTERS:
	case MT_CATCH_RUNNINGMAN:
	case MT_FIGHT_CLUB:
	case MT_TOWER_DEFENSE:
	case MT_DARK_XMAS:
	case MT_FIRE_TEMPLE:
	case MT_RAID:
		iValueArray = EA_MB_LIST2;
		break;
	}

	switch( iMode )
	{
	case MT_SYMBOL:
	case MT_HEROMATCH:
		iOnOff = g_EventMgr.GetValue( GetType(), iValueArray ) / 100000000;
		break;

	case MT_UNDERWEAR:
	case MT_CBT:
	case MT_CATCH:
	case MT_GANGSI:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10000000 ) % 10 );
		break;
	case MT_KING:
	case MT_DUNGEON_A:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000000 ) % 10 );
		break;
	case MT_TRAINING:
	case MT_HEADQUARTERS:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100000 ) % 10 );
		break;
	case MT_SURVIVAL:
	case MT_CATCH_RUNNINGMAN:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10000 ) % 10 );
		break;
	case MT_TEAM_SURVIVAL:
	case MT_DOBULE_CROWN:
	case MT_FIGHT_CLUB:
	case MT_TEAM_SURVIVAL_AI:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000 ) % 10 );
		break;
	case MT_BOSS:
	case MT_TOWER_DEFENSE:
	case MT_RAID:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100 ) % 10 );
		break;
	case MT_MONSTER_SURVIVAL:
	case MT_DARK_XMAS:
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10 ) % 10 );
		break;
	case MT_FOOTBALL:
	case MT_FIRE_TEMPLE:
		iOnOff = g_EventMgr.GetValue( GetType(), iValueArray ) % 10;
		break;
	}

	if( EVENT_MODE_ON == iOnOff )
		return true;

	return false;
}

//---------------------------------------------------------------------------------
MonsterDungeonEventUserNode::MonsterDungeonEventUserNode()
{
}

MonsterDungeonEventUserNode::~MonsterDungeonEventUserNode()
{

}

void MonsterDungeonEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ������ ���� ����.
}

void MonsterDungeonEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// insert�� ���� ����.
}

bool MonsterDungeonEventUserNode::IsEmptyValue()
{
	return true; // ����ϴ� value�� ����.
}

bool MonsterDungeonEventUserNode::IsEventTime( const User *pUser )
{
	if( !pUser )
	{
		EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( !g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		return false;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );

	enum { MAX_CHECK = 2, MAX_VALUE = 3,};
	int iValueArrayList[MAX_CHECK][MAX_VALUE]= { EA_MD_FIRST_EVENT_WEEK_ON_OFF,  EA_MD_FIRST_START_TIME,  EA_MD_FIRST_END_TIME,  
		                                         EA_MD_SECOND_EVENT_WEEK_ON_OFF, EA_MD_SECOND_START_TIME, EA_MD_SECOND_END_TIME 	};

	for (int i = 0; i < MAX_CHECK ; i++)
	{
		if( IsEventWeek( st.wDayOfWeek, iValueArrayList[i][0] ) )
		{
			if( COMPARE( st.wHour, g_EventMgr.GetValue( GetType(), iValueArrayList[i][1] ), g_EventMgr.GetValue( GetType(), iValueArrayList[i][2] ) ) )
				return true;
		}	
	}

	return false;
}

int MonsterDungeonEventUserNode::GetAddCount( const User *pUser  )
{

	if( !IsEventTime( pUser ) )
	{
		return 0;
	}

	return g_EventMgr.GetValue( GetType(), EA_MD_ADD_CARD_COUNT );

}

bool MonsterDungeonEventUserNode::IsEventWeek( WORD wDayOfWeek, int iValueArray )
{
	// wDayOfWeek : ( 0:��, 1:��, 2:ȭ, 3:��, 4:��, 5:��, 6:�� )
	// 1211111    : �Ͽ�ȭ������� / �����Ͽ� �̺�Ʈ ����
	enum { EVENT_WEEK_OFF = 1, EVENT_WEEK_ON = 2, };

	if( iValueArray != EA_MD_FIRST_EVENT_WEEK_ON_OFF && 
		iValueArray != EA_MD_SECOND_EVENT_WEEK_ON_OFF )
	{
		return false;
	}

	int iOnOff = 0;
	if( wDayOfWeek == 0 )
	{
		// [1]211111 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000000 );
	}
	else if( wDayOfWeek == 1 )
	{
		// 1[2]11111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100000 ) % 10 );
	}
	else if( wDayOfWeek == 2 )
	{
		// 12[1]1111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10000 ) % 10 );
	}
	else if( wDayOfWeek == 3 )
	{
		// 121[1]111 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 1000 ) % 10 );
	}
	else if( wDayOfWeek == 4 )
	{
		// 1211[1]11 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 100 ) % 10 );
	}
	else if( wDayOfWeek == 5 )
	{
		// 12111[1]1 
		iOnOff = ( ( g_EventMgr.GetValue( GetType(), iValueArray ) / 10 ) % 10 );
	}
	else if( wDayOfWeek == 6 )
	{
		// 121111[1] 
		iOnOff = ( g_EventMgr.GetValue( GetType(), iValueArray )% 10 );
	}
	else
		return false;

	if( EVENT_WEEK_ON == iOnOff )
		return true;

	return false;
}



//---------------------------------------------------------------------------------------------------------------------------------
FreeDayEventUserNode::FreeDayEventUserNode()
{
}

FreeDayEventUserNode::~FreeDayEventUserNode()
{
}

void FreeDayEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void FreeDayEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

void FreeDayEventUserNode::ProcessTime( User *pUser )
{
	if( !pUser )
		return;

	if( g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		if( pUser->GetPCRoomNumber() == 0 )
		{
			pUser->SendFreeDayEvent(FREEDAY_EVENT_CODE);		// Event �� ���� ���� + 0x00010000 �� �� �ش�.
			LOG.PrintTimeAndLog(0, "[%s]Select All Hero Event On", pUser->GetPrivateID().c_str());
		}
	}
	else
	{
		if( pUser->GetPCRoomNumber() == FREEDAY_EVENT_CODE )
		{
			pUser->SendFreeDayEvent(0);
			LOG.PrintTimeAndLog(0, "[%s] Select All Hero Event Off", pUser->GetPrivateID().c_str());
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------------------
HeroExpBoostEventUserNode::HeroExpBoostEventUserNode()
{
}

HeroExpBoostEventUserNode::~HeroExpBoostEventUserNode()
{
}

void HeroExpBoostEventUserNode::Save( const DWORD dwAgentID, const DWORD dwThreadID )
{
	// ���� �ʿ� ����
}

void HeroExpBoostEventUserNode::InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex )
{
	// DB ������
}

void HeroExpBoostEventUserNode::ProcessTime( User *pUser )
{
	if( !pUser )
		return;

	int nExpBonus = 0;
	if( g_EventMgr.IsAlive( GetType(), pUser->GetChannelingType() ) )
	{
		nExpBonus = g_EventMgr.GetValue( GetType(), 0 );
		pUser->SetExpBonusEventValue(nExpBonus);
	}
	else
	{	
		pUser->SetExpBonusEventValue(nExpBonus);
	}
}