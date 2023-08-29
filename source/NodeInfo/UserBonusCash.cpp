#include "stdafx.h"
#include "BonusCashInfo.h"
#include "BonusCashManager.h"
#include "User.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "../QueryData/QueryResultData.h"
#include "UserBonusCash.h"

UserBonusCash::UserBonusCash()
{
	Init(NULL);
}

UserBonusCash::~UserBonusCash()
{
	Destroy();
}

void UserBonusCash::Init(User* pUser)
{
	m_pUser	= pUser;

	int iSize	= m_vUserBonusCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
			delete pInfo;
	}

	iSize	= m_vExpiredBonusCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vExpiredBonusCashTable[i];
		if( pInfo )
			delete pInfo;
	}

	m_vUserBonusCashTable.clear();
	m_vExpiredBonusCashTable. clear();
}

void UserBonusCash::Destroy()
{
	int iSize	= m_vUserBonusCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
			delete pInfo;
	}

	iSize	= m_vExpiredBonusCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vExpiredBonusCashTable[i];
		if( pInfo )
			delete pInfo;
	}
}

void UserBonusCash::DBToExpiredData(CQueryResultData *query_data)
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "UserBonusCash::DBToExpiredData() User NULL!!"); 
		return;
	}

	while( query_data->IsExist() )
	{	
		DWORD dwTableIndex	= 0;
		int iTotal			= 0;
		int iRemaining		= 0;
		DBTIMESTAMP ExpirationDate;

		PACKET_GUARD_BREAK( query_data->GetValue( dwTableIndex, sizeof(dwTableIndex) ) );			// ���̺� �ε���
		PACKET_GUARD_BREAK( query_data->GetValue( iTotal, sizeof(iTotal) ) );						// �� ���ʽ� ĳ��
		PACKET_GUARD_BREAK( query_data->GetValue( iRemaining, sizeof(iRemaining) ) );				// �ܾ�
		PACKET_GUARD_VOID( query_data->GetValue( (char*)&ExpirationDate, sizeof(DBTIMESTAMP) ) );	// ����Ⱓ

		BonusCashInfo* pInfo = CreateCashData();
		if( pInfo )
		{
			pInfo->Create(dwTableIndex, iTotal, iRemaining, ExpirationDate);
			m_vExpiredBonusCashTable.push_back(pInfo);
		}
	}

	int iSize	= m_vExpiredBonusCashTable.size();

	SP2Packet kPacket(STPK_BONUS_CASH_INFO);
	PACKET_GUARD_VOID( kPacket.Write(BONUS_CASH_EXPIRATION) );
	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo = m_vExpiredBonusCashTable[i];
		if( pInfo )
		{
			PACKET_GUARD_VOID( kPacket.Write(pInfo->GetIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(pInfo->GetTotalAmount()) );
			PACKET_GUARD_VOID( kPacket.Write(pInfo->GetRemainingAmount()) );
			PACKET_GUARD_VOID( kPacket.Write(pInfo->GetExpirationDate()) );
		}
		else
		{
			PACKET_GUARD_VOID( kPacket.Write(0) );
			PACKET_GUARD_VOID( kPacket.Write(0) );
			PACKET_GUARD_VOID( kPacket.Write(0) );
			PACKET_GUARD_VOID( kPacket.Write(0) );
		}
	}

	m_pUser->SendMessage(kPacket);
}

void UserBonusCash::DBToData(CQueryResultData *query_data, int dwSelectType)
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "UserBonusCash::DBToData() User NULL!!"); 
		return;
	}
	m_vUserBonusCashTable.clear();

	while( query_data->IsExist() )
	{
	
		DWORD dwTableIndex	= 0;
		int iTotal			= 0;
		int iRemaining		= 0;
		DBTIMESTAMP ExpirationDate;

		PACKET_GUARD_BREAK( query_data->GetValue( dwTableIndex, sizeof(dwTableIndex) ) );			// ���̺� �ε���
		PACKET_GUARD_BREAK( query_data->GetValue( iTotal, sizeof(iTotal) ) );						// �� ���ʽ� ĳ��
		PACKET_GUARD_BREAK( query_data->GetValue( iRemaining, sizeof(iRemaining) ) );				// �ܾ�
		PACKET_GUARD_VOID( query_data->GetValue( (char*)&ExpirationDate, sizeof(DBTIMESTAMP) ) );	// ����Ⱓ

		BonusCashInfo* pInfo = CreateCashData();
		if( pInfo )
		{
			pInfo->Create(dwTableIndex, iTotal, iRemaining, ExpirationDate);
			m_vUserBonusCashTable.push_back(pInfo);			//���ʽ� ĳ�� �ε��� ���� ����
		}
	}
	
	int iSize	= m_vUserBonusCashTable.size();

	if( dwSelectType == GET_BONUS_CASH_PERIOD )
	{
		SP2Packet kPacket(STPK_BONUS_CASH_INFO);
		PACKET_GUARD_VOID( kPacket.Write(BONUS_CASH_ALIVE) );
		PACKET_GUARD_VOID( kPacket.Write(iSize) );

		for( int i = 0; i < iSize; i++ )
		{
			BonusCashInfo* pInfo = m_vUserBonusCashTable[i];
			if( pInfo )
			{
				PACKET_GUARD_VOID( kPacket.Write(pInfo->GetIndex()) );
				PACKET_GUARD_VOID( kPacket.Write(pInfo->GetTotalAmount()) );
				PACKET_GUARD_VOID( kPacket.Write(pInfo->GetRemainingAmount()) );
				PACKET_GUARD_VOID( kPacket.Write(pInfo->GetExpirationDate()) );
			}
			else
			{
				PACKET_GUARD_VOID( kPacket.Write(0) );
				PACKET_GUARD_VOID( kPacket.Write(0) );
				PACKET_GUARD_VOID( kPacket.Write(0) );
				PACKET_GUARD_VOID( kPacket.Write(0) );
			}
		}

		m_pUser->SendMessage(kPacket);
	}
}

BonusCashInfo* UserBonusCash::CreateCashData()
{
	BonusCashInfo* pInfo	= new BonusCashInfo;
	return pInfo;
}

void UserBonusCash::FillMoveData(SP2Packet& kPacket)
{
	int iSize = m_vUserBonusCashTable.size();

	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
			pInfo->FillCashInfo(kPacket);
	}

	iSize = m_vExpiredBonusCashTable.size();

	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vExpiredBonusCashTable[i];
		if( pInfo )
			pInfo->FillCashInfo(kPacket);
	}
}

void UserBonusCash::ApplyMoveData(SP2Packet& kPacket)
{
	int iSize	= 0;

	PACKET_GUARD_VOID( kPacket.Read(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo = CreateCashData();
		if( pInfo )
		{
			pInfo->ApplyCashInfo(kPacket);
			m_vUserBonusCashTable.push_back(pInfo);
		}
	}

	PACKET_GUARD_VOID( kPacket.Read(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo = CreateCashData();
		if( pInfo )
		{
			pInfo->ApplyCashInfo(kPacket);
			m_vExpiredBonusCashTable.push_back(pInfo);
		}
	}
}

BOOL UserBonusCash::AddBonusCash(int iVal, int iExpirationDay)
{
	if( !m_pUser )
		return FALSE;

	CTimeSpan cGap(iExpirationDay, 0, 0, 0);
	CTime cEndDate = CTime::GetCurrentTime() - cGap;

	//DB�� Insert ��û.
	g_DBClient.OnInsertBonusCash(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), iVal, cEndDate);
	return TRUE;
}

void UserBonusCash::AddCashTable(BonusCashInfo* pInfo)
{
	m_vUserBonusCashTable.push_back(pInfo);

	std::sort(m_vUserBonusCashTable.begin(), m_vUserBonusCashTable.end());
}

BOOL UserBonusCash::ResultAddBonusCash(int iIndex, int iVal, DWORD dwExpirationDate)
{
	//Insert ���� �� ó��. (m_vUserBonusCashTable push_back �� ���� �ǽ�)
	BonusCashInfo* pInfo = CreateCashData();
	if( pInfo )
	{
		pInfo->Create(iIndex, iVal, iVal, dwExpirationDate);

		AddCashTable(pInfo);
		return TRUE;
	}

	return FALSE;
}

int	UserBonusCash::GetAvailableBonusCash()
{
	int iSize		= m_vUserBonusCashTable.size();
	int iTotalVal	= 0;

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
		{
			if( !pInfo->IsAvailable() )	// flag , �Ⱓ���Ῡ��, �����ܾ� > 0 
				continue;

			iTotalVal += pInfo->GetRemainingAmount();		//HRYOON BONUS CASH ��� ���� �� ���ʽ� ĳ�� 
		}
	}

	return iTotalVal;
}

//�޸� �󿡼� ���ʽ� ĳ���� ���Ű� �������� ���� üũ��, ���� ���ɽÿ� DB�� ��û ����
BOOL UserBonusCash::SpendBonusCash(const DWORD dwIndex, const int iVal, int iBuyItemType, int iBuyValue1, int iBuyValue2)
{
	if( !m_pUser )
		return FALSE;

	int iSize		= m_vUserBonusCashTable.size();

	if( 0 == iSize )
	{
		
		return FALSE;
	}

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
		{
			if( pInfo->GetIndex() != dwIndex )
				continue;

			//���ʽ� ĳ���� ���� ���
			if( pInfo->GetRemainingAmount() < iVal )
			{
				return FALSE;
			}

			BonusCashUpdateType eType	= BUT_CHANGE;

			if( pInfo->GetRemainingAmount() == iVal )
				eType	= BUT_END;
				
			
			/*g_DBClient.OnUpdateBonusCash(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), pInfo->GetIndex(), pInfo->GetRemainingAmount() - iVal, eType, 
								iBuyItemType, iBuyValue1, iBuyValue2);
*/
			g_DBClient.OnUpdateBonusCash(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), pInfo->GetIndex(), iVal, eType, 
				iBuyItemType, iBuyValue1, iBuyValue2, m_pUser->GetBillingGUID());
			return TRUE;
		}
	}


	return FALSE;
}

BOOL UserBonusCash::GetMoneyForConsume(IntOfTwoVec& vVal, const int iSpendVal)
{
	int iSize		= m_vUserBonusCashTable.size();
	int iRemainVal	= iSpendVal;	// ��� �� ���ʽ� ĳ��
	IntOfTwo stInfo;

	if( 0 == iSize || iRemainVal <= 0 )
		return FALSE;

	if( GetAvailableBonusCash() < iSpendVal )
		return FALSE;

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
		{
			if( !pInfo->IsAvailable() )
				continue;

			//HRYOON BONUS CASH �� ���� ���ʽ� ĳ�� �ܾ����� ���� ���ɿ��� 
			// vVal -> ���ʽ� ĳ�� �ε��� & ���ݾ�
			if( pInfo->GetRemainingAmount() >= iRemainVal )
			{
				stInfo.value1 = pInfo->GetIndex();
				stInfo.value2 = iRemainVal;		//��� �ݾ�
				vVal.push_back(stInfo);

				pInfo->SetFlag(FALSE);			//HRYOON BONUS CASH �÷��� ����, ����� ���ʽ� ĳ���� ����
				return TRUE;
			}
			else
			{
				//HRYOON BONUS CASH �� �ε������� �Ϻθ� �Һ��� ���(�ݺ�)
				stInfo.value1 = pInfo->GetIndex();
				stInfo.value2 = pInfo->GetRemainingAmount();		//���ݾ�
				vVal.push_back(stInfo);

				iRemainVal -= pInfo->GetRemainingAmount();			//���� ���ʽ� ĳ�� ���� ���� �ݾ�
				pInfo->SetFlag(FALSE);	//HRYOON BONUS CASH �÷��� ����
			}
		}
	}

	return TRUE;
}

BOOL UserBonusCash::ResultSpendBonusCash(BonusCashUpdateType eType, const int iStatus, const int iIndex, const int iVal, const int iUsedAmount, const int iType, const int iValue1, const int iValue2, const ioHashString &szBillingGUID)
{
	//Update ���� ��� ó��.�޸𸮿� �ִ� �� ������
	CASHTABLE_ITER it	= m_vUserBonusCashTable.begin();

	
	while( it != m_vUserBonusCashTable.end() )
	{
		BonusCashInfo* pInfo	= *it;
		if( pInfo )
		{
			if( pInfo->GetIndex() == iIndex )
			{
				pInfo->UpdateRemainingAmount(iVal);
				pInfo->SetFlag(TRUE);
				
				//�ش��ε����� �ܾ��� �����ؼ� ���� ���� �� ��� 
				if( iStatus > 0)
				{
					g_LogDBClient.OnInsertUsageOfBonusCash( m_pUser, pInfo->GetIndex(), -iUsedAmount, iType, iValue1, iValue2, szBillingGUID, iStatus);
				}
				else
				{
					g_LogDBClient.OnInsertUsageOfBonusCash( m_pUser, pInfo->GetIndex(), iUsedAmount, iType, iValue1, iValue2, szBillingGUID, iStatus);
				}
				//�ܾ��� 0���� �Դٸ� �� �ε����� ����
				if( iVal == 0 )
				{
					delete pInfo;
					m_vUserBonusCashTable.erase(it);
				}
				/*switch( eType )
				{
				case BUT_END:
					{
						delete pInfo;
						m_vUserBonusCashTable.erase(it);
					}
					break;
				}*/
				return TRUE;
			}
			else
				it++;
			
		}
	}
	return FALSE;
}

void UserBonusCash::CheckExpiredBonusCash()
{
	if( !m_pUser )
		return;

	static CASHTABLE vExpiredInfo;
	vExpiredInfo.clear();
	CASHTABLE_ITER it	= m_vUserBonusCashTable.begin();

	while( it != m_vUserBonusCashTable.end() )
	{
		BonusCashInfo* pInfo	= *it;
		if( pInfo )
		{
			if( pInfo->IsExpired() )
			{
				vExpiredInfo.push_back(pInfo);

				delete pInfo;
				it	= m_vUserBonusCashTable.erase(it);
			}
			else
			{
				it++;
			}
		}

	}

	int iSize = vExpiredInfo.size();

	if( 0 == iSize )
		return;

	SP2Packet kPacket(STPK_EXPIRED_BONUS_CASH);
	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= vExpiredInfo[i];

		if( pInfo)
		{
			PACKET_GUARD_VOID( kPacket.Write(pInfo->GetIndex()) );
		}
		else
		{
			PACKET_GUARD_VOID( kPacket.Write(0) );
		}
	}

	m_pUser->SendMessage(kPacket);
}

void UserBonusCash::RelieveFlag(const DWORD dwIndex)
{
	int iSize		= m_vUserBonusCashTable.size();

	for( int i = 0; i < iSize; i++ )
	{
		BonusCashInfo* pInfo	= m_vUserBonusCashTable[i];
		if( pInfo )
		{
			if( pInfo->GetIndex() != dwIndex )
				continue;

			pInfo->SetFlag(TRUE);
			return;
		}
	}
}