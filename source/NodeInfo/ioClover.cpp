
#include "stdafx.h"
#include "ioClover.h"

#include "../EtcHelpFunc.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"

ioClover::ioClover()
{
	Init();
}

ioClover::~ioClover()
{
	Destroy();
}

void ioClover::Init()
{
	SetChangeData( false );
	m_iCloverCount = 0;
	m_iLastChargeTime = 0;
	m_sRemainTime = 0;
	m_dwCloverCheckTime = 0;
}

void ioClover::Destroy()
{
}

void ioClover::SetCloverCount( const int iCount )
{
	m_iCloverCount = iCount;
	SetChangeData( true );
}

void ioClover::SetLastChargeTime( const int iChargeTime )
{
	m_iLastChargeTime = iChargeTime;
	SetChangeData( true );
}

void ioClover::SetMaxRemainTime()
{
	m_sRemainTime = static_cast< short >( Help::GetCloverChargeTimeMinute() );
	m_dwCloverCheckTime = TIMEGETTIME();

	SetChangeData( true );
}

void ioClover::IncreaseCloverCount( const int iCount )
{
	int iResultCount = GetCloverCount() + iCount;

	if( iResultCount >= Help::GetCloverMaxCount() )
		SetCloverCount( Help::GetCloverMaxCount() );
	else
		SetCloverCount( iResultCount );
}

void ioClover::DecreaseCloverCount( const int iCount )
{
	if( GetCloverCount() >= Help::GetCloverMaxCount() )
	{
		// �� �������� �Ҹ��ϸ� �ð� �帧.
		SetMaxRemainTime();
	}

	int iResultCount = GetCloverCount() - iCount;

	if( iResultCount <= 0 )
		SetCloverCount( 0 );
	else
		SetCloverCount( iResultCount );
}

void ioClover::Initialize( User *pUser )
{
	SetUser( pUser );
}

bool ioClover::DBtoNewIndex( DWORD dwIndex )
{
	return true;
}

void ioClover::DBtoData( CQueryResultData *query_data )
{
	User* pUser = GetUser();
	if( pUser == NULL )
		return;

	// Set : ��� ������ �ٷ�
	PACKET_GUARD_VOID( query_data->GetValue( m_iCloverCount, sizeof( int ) ) );
	PACKET_GUARD_VOID( query_data->GetValue( m_iLastChargeTime, sizeof( int ) ) );
	PACKET_GUARD_VOID( query_data->GetValue( m_sRemainTime, sizeof( short ) ) );
	
	// Check : DB�� üũ.
	CheckCloverCount();
	CheckCloverRemainTime();

	// ����ð�
	CTime current_time = CTime::GetCurrentTime();
	
	if( GetLastChargeTime() == 0 )
	{
		// 0 �ϰ��, 30�� �� ���� ��������. �ʱ� Ŭ�ι����� ini ����.
		UpdateCloverInfo( CLOVER_TYPE_REFILL, Help::GetCloverMaxCount(), current_time );

		// LOGDB : Clover Charge
		g_LogDBClient.OnInsertCloverInfo( pUser->GetUserIndex(), 0, ioClover::CLOVER_TYPE_REFILL, GetCloverCount() );
	}
	else
	{
		// �׽�Ʈ �ƴҶ�...
		if( Help::GetCloverRefillTest() == 0 )
		{
			//------------------------------
			// ������ �����ð��� ��¥ ��.
			//------------------------------
			
			CTimeSpan GapTime( 0, Help::GetCloverRefill_Hour(), Help::GetCloverRefill_Min(), 0 );

			// ���� �ð����� gap �ð���ŭ -
			CTime compareTime = CTime::GetCurrentTime() - GapTime;

			// int������ ���ϰ� DB���� ��.
			int compareDay = ConvertYYMMDDToDate( compareTime.GetYear(), compareTime.GetMonth(), compareTime.GetDay() );

			// ��, �� ���� ����Ϸ� ��.
			int lastChargeTime	= m_iLastChargeTime / DATE_DAY_VALUE;

			// ���� ������ ����.
			if( lastChargeTime != compareDay )
			{
				// ����.
				UpdateCloverInfo( CLOVER_TYPE_REFILL, Help::GetCloverMaxCount(), compareTime );

				// LOGDB : Clover Charge
				g_LogDBClient.OnInsertCloverInfo( pUser->GetUserIndex(), 0, ioClover::CLOVER_TYPE_REFILL, GetCloverCount() );
			}
		}
		else
		{
			// ���� �׽�Ʈ...
			// ������ �����ð� �� �α��� �ð� ��.

			// �α׾ƿ� �ð�.
			CTime LogoutTime = ConvertNumberToCTime( m_iLastChargeTime );

			// ����ð�
			SYSTEMTIME st;
			GetLocalTime( &st );

			if( LogoutTime.GetHour() != st.wHour )
			{
				// ����
				UpdateCloverInfo( CLOVER_TYPE_REFILL, Help::GetCloverMaxCount(), current_time );

				// LOGDB : Clover Charge
				g_LogDBClient.OnInsertCloverInfo( pUser->GetUserIndex(), 0, ioClover::CLOVER_TYPE_REFILL, GetCloverCount() );
			}
		}
	}

	m_dwCloverCheckTime = TIMEGETTIME();
	
	// �������� Ŭ�ι� ���� Send.
	pUser->GiftCloverInfo( ioClover::GIFT_CLOVER_LOGIN );
}

void ioClover::SaveData()
{
	if( GetChangeData() == false )
		return;

	User* pUser = GetUser();
	if( pUser == NULL )
		return;
	
	g_DBClient.OnUpdateCloverInfo( pUser->GetUserDBAgentID(),
								pUser->GetAgentThreadID(),
								pUser->GetUserIndex(),
								GetCloverCount(),
								GetLastChargeTime(),
								GetCloverRemainTime() );

	SetChangeData( false );
}

void ioClover::FillMoveData( SP2Packet &rkPacket )
{
	rkPacket << GetCloverCount() << GetLastChargeTime() << GetCloverRemainTime();
}

void ioClover::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode /*= false*/ )
{
	rkPacket >> m_iCloverCount >> m_iLastChargeTime >> m_sRemainTime;

	m_dwCloverCheckTime = TIMEGETTIME();
}

const int ioClover::GetCloverCount()
{
	CheckCloverCount();

	return m_iCloverCount;
}

const short ioClover::GetCloverRemainTime()
{
	short remainTime = max( 0, m_sRemainTime - ( ( TIMEGETTIME() - m_dwCloverCheckTime ) / 1000 ) );
	if( remainTime < 0 )
		return MAGIC_TIME;

	return remainTime;
}

CTime ioClover::ConvertNumberToCTime( const int iDateTime )
{
	int year	= iDateTime / DATE_YEAR_VALUE;
	int month	= ( iDateTime % DATE_YEAR_VALUE ) / DATE_MONTH_VALUE;
	int day		= ( iDateTime % DATE_MONTH_VALUE ) / DATE_DAY_VALUE;
	int hour	= ( iDateTime % DATE_DAY_VALUE ) / DATE_HOUR_VALUE;
	int min		= iDateTime % DATE_HOUR_VALUE;

	CTime time( Help::GetSafeValueForCTimeConstructor( year + DEFAULT_YEAR, month, day, hour, min, 0 ) );
	return time;
}

const int ioClover::ConvertYYMMDDToDate( const WORD wYear, const WORD wMonth, const WORD wDay )
{
	// ( year * 10000 ) + ( month * 100 ) + day
	int iReturnDate = ((wYear - DEFAULT_YEAR) * DATE_DAY_VALUE) + (wMonth * DATE_HOUR_VALUE) + wDay;
	return iReturnDate;
}

BOOL ioClover::IsValidCharge( CTime& CurrentTime )
{
	// RemainTime �߰��� RemainTime���� üũ.
	if( GetCloverRemainTime() - MAGIC_TIME * 60 > 0 )
		return FALSE;

	return TRUE;
}

void ioClover::IsSaveRemainTime()
{
	short sRemainTime = GetCloverRemainTime();

	if( sRemainTime != m_sRemainTime )
		SetChangeData( true );
}

void ioClover::UpdateCloverInfo( const int iType, const int iCount, CTime& CurrentTime )
{
	switch( iType )
	{
	case CLOVER_TYPE_CHARGE:
		UpdateCloverCharge( iCount );
		break;
	case CLOVER_TYPE_REFILL:
		UpdateCloverRefill( iCount, CurrentTime );
		break;
	case CLOVER_TYPE_SEND:
		DecreaseCloverCount( iCount );
		break;
	case CLOVER_TYPE_RECEIVE:
		IncreaseCloverCount( iCount );
		break;
	}
}

void ioClover::UpdateCloverCharge( const int iCount )
{
	// Count
	IncreaseCloverCount( iCount );

	// Remain Time
	SetMaxRemainTime();
}

void ioClover::UpdateCloverRefill( const int iCount, CTime& CurrentTime )
{
	// Count
	SetCloverCount( iCount );

	// �ð� ����.
	SetConvertDate( CurrentTime );

	// Remain Time
	SetMaxRemainTime();
}

void ioClover::SetConvertDate( CTime& CurrentTime )
{
	// �ð� ����.
	DWORD dwCurrentTime = Help::ConvertYYMMDDHHMMToDate( CurrentTime.GetYear(),
														CurrentTime.GetMonth(),
														CurrentTime.GetDay(),
														0,		// hour
														0 );	// min

	// time
	SetLastChargeTime( static_cast< int >( dwCurrentTime ) );
}

void ioClover::CheckCloverCount()
{
	// min
	if( m_iCloverCount < 0 )
		m_iCloverCount = 0;

	// max
	if( m_iCloverCount > Help::GetCloverMaxCount() )
		m_iCloverCount = Help::GetCloverMaxCount();
}

void ioClover::CheckCloverRemainTime()
{
	if( m_sRemainTime < 0 )
		m_sRemainTime = MAGIC_TIME;

	if( m_sRemainTime > Help::GetCloverChargeTimeMinute() )
		m_sRemainTime = Help::GetCloverChargeTimeMinute();
}
