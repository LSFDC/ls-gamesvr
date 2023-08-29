
#include "stdafx.h"
#include "TrainingModeRoulette.h"

#include "../MainProcess.h"
#include "../DataBase/LogDBClient.h"

CEventRoulette::CEventRoulette()
{
	Init();
}

CEventRoulette::~CEventRoulette()
{
	Destroy();
}

void CEventRoulette::Init()
{
	m_iBoardingTime = 0;
	m_iSpinTime = 0;
	m_iCoinCount = 0;
	m_szSendID.Clear();
	m_vecGroupRange.clear();
	m_vecGroupAngle.clear();

	InitMember();

	m_Random.Randomize();
}

void CEventRoulette::InitMember()
{
	m_bStart = FALSE;
	m_dwStartTime = 0;
	m_dwJoinEndTime = 0;
	m_dwEndTime = 0;
	m_iAngle = -1;
	m_kResultPresent.Init();
	m_mapBoardingUser.clear();
}

void CEventRoulette::Destroy()
{
	Init();
}

void CEventRoulette::SetSpinTime( const int iSpinTime )
{
	m_iSpinTime = iSpinTime;
}

void CEventRoulette::SetBoardingTime( const int iBoardingTime )
{
	m_iBoardingTime = iBoardingTime;
}

void CEventRoulette::SetCoinCount( const int iCoinCount )
{
	m_iCoinCount = iCoinCount;
}

void CEventRoulette::SetSendID( const char* szSendID )
{
	m_szSendID = szSendID;
}

void CEventRoulette::InsertGroupRange( GROUPRANGE& rGroupRange )
{
	m_vecGroupRange.push_back( rGroupRange );
}

int CEventRoulette::GetGroupRangePosition( const int iRoomUserCount )
{
	int Pos = 0;
	int GroupRange = static_cast< int >( m_vecGroupRange.size() );

	for( Pos ; Pos < GroupRange ; ++Pos )
	{
		if( m_vecGroupRange[ Pos ].m_min <= iRoomUserCount && iRoomUserCount <= m_vecGroupRange[ Pos ].m_max )
			break;
	}

	return Pos;
}

void CEventRoulette::InsertAngleData( std::vector< ANGLEDATA >& rAngleData )
{
	m_vecGroupAngle.push_back( rAngleData );
}

const int CEventRoulette::GetNewAngle( const int iPos )
{
	// Declare) angel
	int angle = -1;

	// ��÷ Ȯ�� ���ϰ�.
	int state = m_Random.Random( 0, 10000 );

	std::vector< ANGLEDATA >::iterator iter		= m_vecGroupAngle[ iPos ].begin();
	std::vector< ANGLEDATA >::iterator iterEnd	= m_vecGroupAngle[ iPos ].end();

	while( iter != iterEnd )
	{
		// ��÷ Ȯ���� ���� ���� üũ.
		if( (*iter).m_maxRealPercent < state )
		{
			++iter;
			continue;
		}

		// ��÷ Ȯ���� ���� ���� üũ�� angle�� ���ؼ�.
		angle = m_Random.Random( (*iter).m_angle_min, (*iter).m_angle_max );

		// Set : Present
		m_kResultPresent = (*iter);
		break;
	}

	// set
	SetAngle( angle );
	
	// return
	return angle;
}

void CEventRoulette::SetRouletteStart( User* pUser )
{
	InitMember();

	SetState( TRUE );
	m_mapBoardingUser.insert( std::map< int, User* >::value_type( pUser->GetUserIndex(), pUser ) );
	SetStartTime( TIMEGETTIME() );
	SetJoinEndTime( GetStartTime() + GetBoardingTime() * 1000 );
	SetEndTime( GetStartTime() + GetSpinTime() * 1000 );
}

BOOL CEventRoulette::SetRouletteJoin( User* pUser )
{
	// Check : ���ΰ��� �ð�
	if( GetJoinEndTime() < TIMEGETTIME() )
	{
		// ž�½ð� ����.
		SP2Packet kPacket( STPK_ROULETTE_START );
		kPacket << static_cast< int >( ROULETTE_JOIN_TIME_OVER );
		pUser->SendMessage( kPacket );
		return FALSE;
	}

	// Check : ������ �ִ����� üũ����. ( �ι�° �Ķ���� TRUE )
	if( pUser->UseRouletteCoin( GetUseCoinCount(), TRUE ) == false )
	{
		// ���� ����.
		SP2Packet kPacket( STPK_ROULETTE_START );
		kPacket << static_cast< int >( ROULETTE_START_NOT_ENOUGH_COIN );
		pUser->SendMessage( kPacket );
		return FALSE;
	}

	// Check : �̹� ��ϵǾ� �ִµ� ž���� �Ǹ� �ȵǹǷ�..
	std::map< int, User* >::iterator	iter = m_mapBoardingUser.find( pUser->GetUserIndex() );
	if( iter != m_mapBoardingUser.end() )
	{
		// �̹� ����
		SP2Packet kPacket( STPK_ROULETTE_START );
		kPacket << static_cast< int >( ROULETTE_JOIN_ALREADY );
		pUser->SendMessage( kPacket );
		return FALSE;
	}

	// insert
	m_mapBoardingUser.insert( std::map< int, User* >::value_type( pUser->GetUserIndex(), pUser ) );

	// return
	return TRUE;
}

void CEventRoulette::SetRouletteEnd( User* pSend )
{
	// Check : ���� �ð�
	if( GetEndTime() > TIMEGETTIME() + 10000 )	// 10 ��
	{
		LOG.PrintTimeAndLog( 0, "Not End Time... %u > %u", GetEndTime(), TIMEGETTIME() + 10000 );

		// PacketSend : ���ᶧ���� ���� �ð� ����.
		SP2Packet kPacket( STPK_ROULETTE_EXCEPTION );
		kPacket << GetEndTime() - TIMEGETTIME();
		pSend->SendMessage( kPacket );
		return;
	}

	// Get : ���� Data
	ANGLEDATA& presentInfo = GetPresentInfo();

	// ������ ���鼭 ���� ����. / ���� ����.
	std::map< int, User* >::iterator	iter	= m_mapBoardingUser.begin();
	std::map< int, User* >::iterator	iterEnd	= m_mapBoardingUser.end();

	while( iter != iterEnd )
	{
		if( ( (*iter).second )->UseRouletteCoin( GetUseCoinCount(), FALSE ) == true )
		{
			// ��������.
			CTimeSpan cPresentGapTime( presentInfo.m_angle_PresentPeriod, 0, 0, 0 );
			CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;

			// To Memory
			( (*iter).second )->AddPresentMemory( GetSendID(), presentInfo.m_angle_PresentType, presentInfo.m_angle_PresentValue1,
				presentInfo.m_angle_PresentValue2, 0, 0, presentInfo.m_angle_PresentMent, kPresentTime, presentInfo.m_angle_PresentState );
			
			// LogDB
			g_LogDBClient.OnInsertPresent( 0, GetSendID(), g_App.GetPublicIP().c_str(), ( (*iter).second )->GetUserIndex(), presentInfo.m_angle_PresentType,
				presentInfo.m_angle_PresentValue1, presentInfo.m_angle_PresentValue2, 0, 0, LogDBClient::PST_RECIEVE, "Roulette" );

			// �ѹ���.
			( (*iter).second )->SendPresentMemory();
		}
		else
		{
			// Log.
			LOG.PrintTimeAndLog( 0, "%s : coin not enough.", __FUNCTION__ );
		}

		++iter;
	}

	// ������ �ʱ�ȭ.
	InitMember();
}

const int CEventRoulette::GetBoardingMemberCount()
{
	return static_cast< int >( m_mapBoardingUser.size() );
}

void CEventRoulette::GetBoardingMember( std::vector< User* >& rMember )
{
	std::map< int, User* >::iterator	iter	= m_mapBoardingUser.begin();
	std::map< int, User* >::iterator	iterEnd	= m_mapBoardingUser.end();

	while( iter != iterEnd )
	{
		rMember.push_back( (*iter).second );

		++iter;
	}
}

BOOL CEventRoulette::GetRouletteMaster( ioHashString& name )
{
	if( m_mapBoardingUser.empty() )
		return FALSE;

	// Name
	std::map< int, User* >::iterator	iter	= m_mapBoardingUser.begin();
	name = ( (*iter).second )->GetPublicID();
	return TRUE;
}

BOOL CEventRoulette::RemoveUser( User* pUser )
{
	if( m_mapBoardingUser.empty() )
		return FALSE;

	// find
	std::map< int, User* >::iterator	iter = m_mapBoardingUser.find( pUser->GetUserIndex() );
	if( iter != m_mapBoardingUser.end() )
	{
		m_mapBoardingUser.erase( iter );
	}

	// ���� �� �ƹ��� ���ٸ�.
	if( m_mapBoardingUser.empty() )
	{
		InitMember();

		return TRUE;
	}

	return FALSE;
}

void CEventRoulette::ModeInfo( SP2Packet& rkPacket )
{
	//	- �귿����(T/F) / �������� �����ð� / �ο��� / �귿�����ο� �̸� / T / angle

	BOOL bState = GetState();
	if( bState )
	{
		// declare)
		DWORD dwRemainTime = GetEndTime() - TIMEGETTIME();
		int iBoardingUserCount = GetBoardingMemberCount();

		rkPacket << true;	// �귿 ����.
		rkPacket << dwRemainTime;
		rkPacket << iBoardingUserCount;

		// Declare)
		std::vector< User* > vecData;
		GetBoardingMember( vecData );

		std::vector< User* >::iterator	iter	= vecData.begin();
		std::vector< User* >::iterator	iterEnd	= vecData.end();
		while( iter != iterEnd )
		{
			rkPacket << (*iter)->GetPublicID();

			++iter;
		}

		//
		int angle = GetAngle();
		if( angle == -1 )
		{
			rkPacket << false;
		}
		else
		{
			rkPacket << true;
			rkPacket << angle;
		}
	}
	else
	{
		rkPacket << false;
	}
}

