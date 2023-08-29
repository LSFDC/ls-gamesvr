
#include "stdafx.h"
#include "ioBingo.h"

#include "../EtcHelpFunc.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "ioBingoManager.h"
#include "../DataBase/LogDBClient.h"
#include "../MainProcess.h"

ioBingo::ioBingo() : m_bChangeNumber( FALSE ), m_bChangePresent( FALSE ), m_bSendPresentState( FALSE )
{
	Init();
}

ioBingo::~ioBingo()
{
	Destroy();
}

void ioBingo::Init()
{
	ZeroMemory( m_BingoBoard, sizeof( m_BingoBoard ) );	
	ZeroMemory( m_present, sizeof( m_present ) );
}

void ioBingo::Destroy()
{
	ZeroMemory( m_BingoBoard, sizeof( m_BingoBoard ) );	
	ZeroMemory( m_present, sizeof( m_present ) );	
}

void ioBingo::Initialize( User *pUser )
{
	SetUser( pUser );
}

bool ioBingo::DBtoNewIndex( DWORD dwIndex )
{
	return true;
}

void ioBingo::DBtoData( CQueryResultData *query_data )
{
	//! DB Table 2���� select �ϹǷ� _Number _Present �� ����.
	return;
}

void ioBingo::DBtoData_Number( CQueryResultData *query_data )
{
	// Set : ��� ������ �ٷ�
	int iArrayIdx = 0;
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			query_data->GetValue( m_BingoBoard[ i ][ j ].m_Number, sizeof( BYTE ) );

			if( m_BingoBoard[ i ][ j ].m_Number == 0 )
				return;

			m_BingoBoard[ i ][ j ].m_BingoDuumyCode = g_BingoMgr.GetBingoDuumyCode( iArrayIdx++ );
		}
	}
}

void ioBingo::DBtoData_Present( CQueryResultData *query_data )
{
	// Set : ��� ������ �ٷ�
	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		query_data->GetValue( m_present[ i ], sizeof( BYTE ) );

		if( m_present[ i ] == 0 )
			return;
	}

	User* pUser = GetUser();
	if( !pUser )
	{
		LOG.PrintTimeAndLog(0, "%s- pUser == NULL !!", __FUNCTION__ );
		return;
	}

	SP2Packet kPacket( STPK_BINGO_DATA );
	PACKET_GUARD_VOID( kPacket.Write( g_BingoMgr.GetBingoType() ) );	
	FillBingoData( kPacket, RESTART_NO_ROILLING );
	pUser->SendMessage( kPacket );
}

void ioBingo::SaveData()
{
	// Get : User
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	// Check : Change Number
	if( GetChangeNumber() == TRUE )
	{
		BYTE ArrayNumber[MAX][MAX] = { 0, };
		GetBingoNumberData( ArrayNumber );

		g_DBClient.OnUpdateBingoNumber( pUser->GetUserDBAgentID(),
										pUser->GetAgentThreadID(),
										pUser->GetUserIndex(),
										ArrayNumber );

		SetChangeNumber( FALSE );
	}

	// Check : Change Present
	if( GetChangePresent() == TRUE )
	{
		g_DBClient.OnUpdateBingoPresent( pUser->GetUserDBAgentID(),
										pUser->GetAgentThreadID(),
										pUser->GetUserIndex(),
										m_present );

		SetChangePresent( FALSE );
	}
}

//! ���� 25 + ���� 12 + �ú��� 1
void ioBingo::FillMoveData( SP2Packet &rkPacket )
{
	// ����
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			rkPacket << m_BingoBoard[ i ][ j ].m_Number;		
		}
	}

	// ����
	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		rkPacket << m_present[ i ];
	}
}

void ioBingo::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode /*= false*/ )
{
	// ����
	int iArrayIdx = 0;
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			m_BingoBoard[ i ][ j ].Clear();

			rkPacket >> m_BingoBoard[ i ][ j ].m_Number;			
			m_BingoBoard[ i ][ j ].m_BingoDuumyCode = g_BingoMgr.GetBingoDuumyCode( iArrayIdx++ );
		}
	}

	// ����
	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		rkPacket >> m_present[ i ];
	}
}

void ioBingo::InitAll()
{
	// Member Init
	Init();

	// Number
	InitBingoNumber();

	// Present
	InitBingoPresent();
}

void ioBingo::InitBingoNumber()
{
	// Declare : Get Max Number
	const int tableValue = g_BingoMgr.GetBingoMaxNumber();

	// Reserve
	vector< BYTE > tempBingo;
	tempBingo.reserve( tableValue );

	// push_back
	for( int i = 0 ; i < tableValue ; ++i )
	{
		tempBingo.push_back( i + 1 );
	}
	
	 // shuffle
	if( g_BingoMgr.GetBingoType() == BT_RAND )	
		random_shuffle( tempBingo.begin(), tempBingo.end() );
	
	// Declare
	int iArrayIdx = 0;
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			m_BingoBoard[ i ][ j ].m_Number = tempBingo[iArrayIdx];
			m_BingoBoard[ i ][ j ].m_BingoDuumyCode = g_BingoMgr.GetBingoDuumyCode( iArrayIdx );
			iArrayIdx++;
		}
	}

	SetChangeNumber( TRUE );
}

void ioBingo::InitBingoPresent()
{
	// Declare)
	vector< stRewardData > tempBingo;

	// Set
	g_BingoMgr.GetBingoReward( tempBingo );

	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		m_present[ i ] = tempBingo[ i ].index;
	}

	SetChangePresent( TRUE );
}

void ioBingo::ResetBingoPresent()
{
	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		if( m_present[ i ] > FLAG_VALUE )
		{
			// ���� ������ �ٽ� ���� �� �ְ�..
			m_present[ i ] -= FLAG_VALUE;

			// Set : ���� �ߴٰ�
			SetChangePresent( TRUE );
		}
	}
}

void ioBingo::GetBingoNumberData( BYTE (*pArray)[ MAX ] )
{
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			pArray[ i ][ j ] = m_BingoBoard[ i ][ j ].m_Number;
		}
	}
}

void ioBingo::GetBingoPresentData( BYTE* pArray )
{
	for( int i = 0 ; i < PRESENT_COUNT ; ++i )
	{
		*( pArray + i ) = m_present[ i ];
	}
}

void ioBingo::ChoiceNumber( int selectNumber, int iType /*= 0*/ )
{
	// Get User
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	// declare
	BOOL bHitState = FALSE;

	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			// find
			if( m_BingoBoard[ i ][ j ].m_Number == selectNumber )
			{
				// +100
				m_BingoBoard[ i ][ j ].m_Number  += FLAG_VALUE;

				// save flag
				SetChangeNumber( TRUE );

				// for log
				bHitState = TRUE;

				break;
			}
		}
	}

	// Check : ��÷ ��ȣ
	if( bHitState == TRUE )
	{
		// ���� üũ.
		CheckBingo();

		// ���� ������ �ƴٸ�..
		if( GetSendPresentState() == TRUE )
		{
			// �ѹ���.
			pUser->SendPresentMemory();

			// �ٽ� FALSE��..
			SetSendPresentState( FALSE );
		}

		// LOG
		g_LogDBClient.OnInsertBingoChoiceNumber( pUser->GetUserIndex(), pUser->GetPublicID(), iType, selectNumber, LOG_NUMBER_HIT );
	}
	else
	{
		// Log
		g_LogDBClient.OnInsertBingoChoiceNumber( pUser->GetUserIndex(), pUser->GetPublicID(), iType, selectNumber, LOG_NUMBER_MISS );
	}
}

int ioBingo::FreeChoiceNumber()
{
	// Declare)
	vector< BYTE > tempNumber;
	
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			// find
			if( m_BingoBoard[ i ][ j ].m_Number > FLAG_VALUE )
				continue;

			tempNumber.push_back( m_BingoBoard[ i ][ j ].m_Number );
		}
	}

	// random_shuffle
	random_shuffle( tempNumber.begin(), tempNumber.end() );

	// choice
	int selectNumber = tempNumber[ 0 ];

	// ���� ��ȣ ����.
	ChoiceNumber( selectNumber, LOG_FREE_CHOICE );

	return selectNumber;
}

bool ioBingo::RandomShuffleNumber()
{
	if( g_BingoMgr.GetBingoType() != BT_RAND )
	{		
		LOG.PrintTimeAndLog(0, "%s - wrong bingo type - %d", __FUNCTION__, g_BingoMgr.GetBingoType() );
		return false;
	}

	// ���� �ƴ� ����.
	vector< BYTE > vecNoneBingo;

	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			// �� üũ
			if( m_present[ i + 1 ] > FLAG_VALUE )
				continue;

			// �� üũ
			if( m_present[ j + PRESENT_POS_6 + 1 ] > FLAG_VALUE )
				continue;

			// �� üũ
			if( i == j )
			{
				if( m_present[ PRESENT_POS_0 ] > FLAG_VALUE )
					continue;
			}

			// �� üũ
			if( i + j == MAX - 1 )
			{
				if( m_present[ PRESENT_POS_6 ] > FLAG_VALUE )
					continue;
			}

			// push
			vecNoneBingo.push_back( m_BingoBoard[ i ][ j ].m_Number );
			m_BingoBoard[ i ][ j ].Clear();
		}
	}

	// random_shuffle
	random_shuffle( vecNoneBingo.begin(), vecNoneBingo.end() );

	int iArrayIdx = 0;
	for( int i = 0 ; i < MAX ; ++i )
	{
		for( int j = 0 ; j < MAX ; ++j )
		{
			if( m_BingoBoard[ i ][ j ].m_Number != 0 )
				continue;

			// 0�� �༮���� ã�Ƽ� ����
			m_BingoBoard[ i ][ j ].m_Number = vecNoneBingo[ iArrayIdx ];
			m_BingoBoard[ i ][ j ].m_BingoDuumyCode = g_BingoMgr.GetBingoDuumyCode( iArrayIdx );
			iArrayIdx++;
		}
	}

	SetChangeNumber( TRUE );

	CheckBingo();

	// ���� ������ �ƴٸ�
	if( GetSendPresentState() == TRUE )
	{
		// Get User
		User* pUser = GetUser();
		if( pUser == NULL )
			return false;

		// �ѹ���
		pUser->SendPresentMemory();

		// �ٽ� FALSE��
		SetSendPresentState( FALSE );
	}

	return true;
}

void ioBingo::RandomShufflePresent()
{
	// Declare)
	vector< stRewardData > tempBingo;

	// Get : 13��
	g_BingoMgr.GetBingoReward( tempBingo );
	
	// Set : �ú��� ���� ����.
	m_present[ ALL_BINGO ] = tempBingo[ tempBingo.size() - 1 ].index;

	// vector ����.
	for( int i = 0 ; i < ALL_BINGO ; ++i )
	{
		if( m_present[ i ] > FLAG_VALUE )
		{
			tempBingo.erase( remove( tempBingo.begin(), tempBingo.end(), m_present[ i ] - FLAG_VALUE ), tempBingo.end() );
		}
	}

	int count = 0;

	// ����12
	for( int i = 0 ; i < ALL_BINGO ; ++i )
	{
		if( m_present[ i ] > FLAG_VALUE )
			continue;

		// Set : ���� 12
		m_present[ i ] = tempBingo[ count++ ].index;
	}
	
	// Set
	SetChangePresent( TRUE );
}

void ioBingo::CheckBingo()
{
	int bingo = 0;

	// ����, ����, ��, ��
	int flag_x = 0, flag_y = 0, flag_cross = 0, flag_across = 0;

	for( int i = 0 ; i < MAX ; ++i )
	{
		flag_x = flag_y = 0;

		for( int j = 0 ; j < MAX ; ++j )
		{
			// find
			if( m_BingoBoard[ i ][ j ].m_Number > FLAG_VALUE )
			{
				++flag_x;

				if( flag_x == MAX )
				{
					++bingo;

					// ����
					CheckPresent( i + PRESENT_POS_0 + 1 );
				}
			}

			// find
			if( m_BingoBoard[ j ][ i ].m_Number > FLAG_VALUE )
			{
				++flag_y;

				if( flag_y == MAX )
				{
					++bingo;

					// ����
					CheckPresent( i + PRESENT_POS_6 + 1 );
				}
			}
		}

		if( m_BingoBoard[ i ][ i ].m_Number > FLAG_VALUE )
			++flag_cross;
		if( m_BingoBoard[ MAX - 1 - i ][ i ].m_Number > FLAG_VALUE )
			++flag_across;
	}

	if( flag_cross == MAX )
	{
		++bingo;

		// ����
		CheckPresent( PRESENT_POS_0 );
	}
	if( flag_across == MAX )
	{
		++bingo;

		// ����
		CheckPresent( PRESENT_POS_6 );
	}

	if( bingo == ALL_BINGO )
	{
		CheckPresent( PRESENT_POS_12 );
	}
}

void ioBingo::CheckPresent( const int index )
{
	// Check : �̹� ���� ���޵Ǿ�����..
	if( m_present[ index ] > FLAG_VALUE )
		return;

	// Declare)
	stRewardData presentInfo;

	// index�� ���� ���� ������ ���;���.
	if( index != PRESENT_POS_12 )
	{
		// ���� 12
		presentInfo = g_BingoMgr.GetBingoPresentInfo( m_present[ index ] );
	}
	else
	{
		// �ú��� 1
		presentInfo = g_BingoMgr.GetAllBingoPresentInfo( m_present[ index ] );
	}

	// Check
	if( presentInfo.index == 0 )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ERROR] Bingo presentInfo.index == 0, index : %d", m_present[ index ] );
		return;
	}

	// Send Present
	SendPresent( presentInfo );

	// Set Flag
	m_present[ index ] += FLAG_VALUE;

	// ���� ���� ����
	SetChangePresent( TRUE );
}

void ioBingo::SendPresent( stRewardData& presentInfo )
{
	// Get User
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	// ��������.
	CTimeSpan cPresentGapTime( g_BingoMgr.GetPeriod(), 0, 0, 0 );
	CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;

	// To Memory
	pUser->AddPresentMemory( g_BingoMgr.GetSendID(), presentInfo.type, presentInfo.value1,
		presentInfo.value2, 0, 0, g_BingoMgr.GetMent(), kPresentTime, g_BingoMgr.GetState() );
			
	// LogDB
	g_LogDBClient.OnInsertPresent( 0, g_BingoMgr.GetSendID(), g_App.GetPublicIP().c_str(), pUser->GetUserIndex(), presentInfo.type,
		presentInfo.value1, presentInfo.value2, 0, 0, LogDBClient::PST_RECIEVE, "Bingo" );

	// Set Flag
	SetSendPresentState( TRUE );
}


void ioBingo::OnBingoStart( User* pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog(0, "%s- pUser == NULL !!", __FUNCTION__ );
		return;
	}

	InitAll();

	BYTE ArrayNumber[MAX][MAX] = { 0, };
	GetBingoNumberData( ArrayNumber );
	g_DBClient.OnInsertBingoNumber( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), ArrayNumber );
	g_DBClient.OnInsertBingoPresent( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), m_present );

	SP2Packet kPacket( STPK_BINGO_DATA );
	PACKET_GUARD_VOID( kPacket.Write( g_BingoMgr.GetBingoType() ) );
	FillBingoData( kPacket, RESTART_ROLLING );
	pUser->SendMessage( kPacket );
}


void ioBingo::OnBingoNumberInitialize( User* pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog(0, "%s- pUser == NULL !!", __FUNCTION__ );
		return;
	}
	
	InitBingoNumber();
	ResetBingoPresent();

	SP2Packet kPacket( STPK_BINGO_NUMBER_INIT );
	PACKET_GUARD_VOID( kPacket.Write( g_BingoMgr.GetBingoType() ) );
	FillBingoData( kPacket, RESTART_ROLLING );
	pUser->SendMessage( kPacket );
}

void ioBingo::OnBingoALLInitialize( User* pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog(0, "%s- pUser == NULL !!", __FUNCTION__ );
		return;
	}

	InitAll();

	SP2Packet kPacket( STPK_BINGO_DATA );
	PACKET_GUARD_VOID( kPacket.Write( g_BingoMgr.GetBingoType() ) );
	FillBingoData( kPacket, RESTART_ROLLING );
	pUser->SendMessage( kPacket );
}

void ioBingo::FillBingoData( SP2Packet& rkPacket, const int iRollingType )
{
	PACKET_GUARD_VOID( rkPacket.Write( iRollingType ) );

	for( int i = 0 ; i < ioBingo::MAX ; ++i )
	{
		for( int j = 0 ; j < ioBingo::MAX ; ++j )
		{
			PACKET_GUARD_VOID( rkPacket.Write( m_BingoBoard[ i ][ j ].m_Number ) );
			PACKET_GUARD_VOID( rkPacket.Write( m_BingoBoard[ i ][ j ].m_BingoDuumyCode ) );
		}
	}

	for( int i = 0 ; i < ioBingo::PRESENT_COUNT ; ++i )
	{
		PACKET_GUARD_VOID( rkPacket.Write( m_present[ i ] ) );
	}
}


void ioBingo::SendBingoType( User* pUser )
{
	SP2Packet kPacket( STPK_BINGO_TYPE );
	PACKET_GUARD_VOID( kPacket.Write( g_BingoMgr.GetBingoType() ) );	
	pUser->SendMessage( kPacket );
}