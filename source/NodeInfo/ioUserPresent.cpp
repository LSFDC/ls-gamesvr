
#include "stdafx.h"

//#include "../Window.h"
#include "../MainProcess.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"

#include "../EtcHelpFunc.h"

#include "Room.h"
#include "ioItemInfoManager.h"
#include "ioDecorationPrice.h"
#include "ioEtcItemManager.h"
#include "ioExtraItemInfoManager.h"
#include "ioItemInitializeControl.h"
#include "User.h"

#include "ioUserPresent.h"
#include "iomedaliteminfomanager.h"
#include "ioAlchemicMgr.h"
#include "ioPetInfoManager.h"
#include "CostumeManager.h"
#include "AccessoryManager.h"
#include "ioPresentHelper.h"

#include <strsafe.h>
#include "../Local/ioLocalParent.h"

ioUserPresent::ioUserPresent()
{
	Initialize( NULL );
}

ioUserPresent::~ioUserPresent()
{
	m_vPresentList.clear();
}

void ioUserPresent::Initialize( User *pUser )
{
	m_pUser = pUser;
	m_vPresentList.clear();
	m_vTempMemoryData.clear();
	m_dwLastDBIndex = m_dwLastSlotIndex = 0;
}

ioUserPresent::PresentData &ioUserPresent::GetPresentData( DWORD dwIndex, DWORD dwSlotIndex )
{
	int iSize = m_vPresentList.size();
	for(int i = 0;i < iSize;i++)
	{
		PresentData &rkData = m_vPresentList[i];
		if( rkData.m_dwIndex != dwIndex ) continue;
		if( rkData.m_dwSlotIndex != dwSlotIndex ) continue;

		return rkData;
	}

	static PresentData kTemp;
	return kTemp;
}

bool ioUserPresent::DeletePresentData( DWORD dwIndex, DWORD dwSlotIndex, DeletePresentType eDeletePresentType )
{
	int iSize = m_vPresentList.size();
	for(int i = 0;i < iSize;i++)
	{
		PresentData &rkData = m_vPresentList[i];
		if( rkData.m_dwIndex != dwIndex ) continue;
		if( rkData.m_dwSlotIndex != dwSlotIndex ) continue;

		DWORD dwAgentServerID = 0;
		DWORD dwAggentThreadID= 0;
		if( m_pUser )
		{
			dwAgentServerID = m_pUser->GetUserDBAgentID();
			dwAggentThreadID= m_pUser->GetAgentThreadID();
			char szNote[MAX_PATH]="";
			if( eDeletePresentType == DPT_TIMEOVER )
				StringCbPrintf( szNote, sizeof( szNote ), "TimeOver Index:%d", dwIndex );
			else if( eDeletePresentType == DPT_RECV )
				StringCbPrintf( szNote, sizeof( szNote ), "Recv Index:%d", dwIndex );
			else if( eDeletePresentType == DPT_SELL )
				StringCbPrintf( szNote, sizeof( szNote ), "Sell Index:%d", dwIndex );
			else
				StringCbPrintf( szNote, sizeof( szNote ), "Index:%d", dwIndex );

			g_LogDBClient.OnInsertPresent( rkData.m_dwSendUserIndex, rkData.m_szSendID, m_pUser->GetPublicIP(), m_pUser->GetUserIndex(), rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, rkData.m_iPresentValue3, rkData.m_iPresentValue4, LogDBClient::PST_DEL, szNote );
		}

		m_vPresentList.erase( m_vPresentList.begin() + i );
		if( dwIndex != PRESENT_INDEX_MEMORY )
		{
			g_DBClient.OnUpdatePresentData( dwAgentServerID, dwAggentThreadID, dwIndex, PRESENT_STATE_DELETE );
			g_DBClient.OnDeletePresent( dwAgentServerID, dwAggentThreadID, dwIndex);
		}
		return true;
	}
	return false;
}

bool ioUserPresent::ComparePresentData( ioUserPresent::PresentData &kData )
{
	int iSize = m_vPresentList.size();
	for(int i = 0;i < iSize;i++)
	{
		PresentData &rkData = m_vPresentList[i];
		if( rkData.m_dwIndex == kData.m_dwIndex )
			return true;
	}
	return false;
}

void ioUserPresent::DBtoPresentData( CQueryResultData *query_data )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::DBtoPresentData() User NULL!!"); 
		return;
	}

	int iDeletePresentCnt = 0;

	// ���� ���� �߰�.
	LOOP_GUARD();
	vPresentData vNewPresent;
	while( query_data->IsExist() )
	{		
		PresentData kData;
		query_data->GetValue( kData.m_dwIndex, sizeof( DWORD ) );
		query_data->GetValue( kData.m_dwSendUserIndex, sizeof( DWORD ) );
		query_data->GetValue( kData.m_szSendID, ID_NUM_PLUS_ONE );
		query_data->GetValue( kData.m_iPresentType, sizeof( short ) );
		query_data->GetValue( kData.m_iPresentValue1, sizeof( int ) );
		query_data->GetValue( kData.m_iPresentValue2, sizeof( int ) );
		query_data->GetValue( kData.m_iPresentValue3, sizeof( int ) );
		query_data->GetValue( kData.m_iPresentValue4, sizeof( int ) );
		query_data->GetValue( kData.m_iPresentMent, sizeof( short ) );
		query_data->GetValue( kData.m_iPresentState, sizeof( short ) );

		DBTIMESTAMP DTS;
		query_data->GetValue( (char*)&DTS, sizeof(DBTIMESTAMP) );
		CTime LimitTime( Help::GetSafeValueForCTimeConstructor( DTS.year, DTS.month, DTS.day, DTS.hour, DTS.minute, DTS.second ) );		
		if( !ComparePresentData( kData ) )
		{
			if( CTime::GetCurrentTime() >= LimitTime )
			{
				if( kData.m_iPresentState != PRESENT_STATE_DELETE )   // �̹� ������ ������ �� ������Ʈ �� �ʿ䰡 ����.
				{
					char szNote[MAX_PATH]="";
					StringCbPrintf( szNote, sizeof( szNote ), "TimeOver Index:%d", kData.m_dwIndex );
					g_LogDBClient.OnInsertPresent( kData.m_dwSendUserIndex, kData.m_szSendID, m_pUser->GetPublicIP(), m_pUser->GetUserIndex(), kData.m_iPresentType, kData.m_iPresentValue1, kData.m_iPresentValue2, kData.m_iPresentValue3, kData.m_iPresentValue4, LogDBClient::PST_DEL, szNote );
					// �Ⱓ�� �������Ƿ� DB���� �����ϰ� �������� �˸�
					g_DBClient.OnUpdatePresentData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_dwIndex, PRESENT_STATE_DELETE );
					g_DBClient.OnDeletePresent( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_dwIndex );
				}
				kData.m_dwLimitDate = 0;
				if( g_ItemInitControl.IsDeletePresentCheck( kData.m_iPresentType, kData.m_iPresentValue1 ) )
					kData.m_iPresentState = PRESENT_STATE_DELETE;     // ������ ������ ó���Ͽ� �˸� �˾��� Ȱ��ȭ���� �ʵ��� �Ѵ�.
			}
			else
			{
				// �Ⱓ �߰�
				kData.m_dwLimitDate = Help::ConvertCTimeToDate( LimitTime );
			}

			m_dwLastDBIndex = max( m_dwLastDBIndex, kData.m_dwIndex );    // ������ DB �ε���
			kData.m_dwSlotIndex = GetLastPresentSlotIndex();			  // ���� �ε��� ����

			if( kData.m_iPresentState == PRESENT_STATE_DELETE )
			{
				iDeletePresentCnt++;
				continue; // ���� ó���� ������ �н�. ���� ���� �ϰ� ������
			}
#ifdef SRC_OVERSEAS
			if( kData.m_iPresentType == PRESENT_PESO )
			{
				if( g_UserNodeManager.IsDeveloper( kData.m_szSendID.c_str() ))
				{
					if(  Help::GetLimitPeso() != 0 && kData.m_iPresentValue1 >= Help::GetLimitPeso() )
					{
						g_DBClient.OnUpdatePresentData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_dwIndex, PRESENT_STATE_DELETE );
						g_DBClient.OnDeletePresent( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_dwIndex );
						g_LogDBClient.OnInsertPeso( m_pUser, kData.m_iPresentValue1, LogDBClient::PT_DEL_CHEAT_PESO );	
						LOG.PrintTimeAndLog( 0, "%s - PESO DATE IS TOO HIGH TT^TT;; id/peso %s/%d", __FUNCTION__, m_pUser->GetPublicID().c_str() ,kData.m_iPresentValue1 );
					
						continue;
					}

				}
			}
#endif
			vNewPresent.push_back( kData );
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "DBtoPresentData �̹� ���� ������ �� �޾��� : %s - %d", m_pUser->GetPublicID().c_str(), kData.m_dwIndex );
		}
	}	
	LOOP_GUARD_CLEAR();

	if( vNewPresent.empty() )
	{
		SP2Packet kPacket( STPK_PRESENT_DATA );
		kPacket << 0 << iDeletePresentCnt;
		m_pUser->SendMessage( kPacket );
		return;
	}

	// ���� ���� ���� �� ����
	SP2Packet kPacket( STPK_PRESENT_DATA );
	kPacket << (int)vNewPresent.size() << iDeletePresentCnt;
	vPresentData::iterator iter = vNewPresent.begin();
	for(;iter != vNewPresent.end();iter++)
	{
		PresentData &kData = *iter;		
		kPacket << kData.m_dwIndex << kData.m_dwSlotIndex << kData.m_szSendID << kData.m_iPresentType << kData.m_iPresentMent
			<< kData.m_iPresentState << kData.m_iPresentValue1 << kData.m_iPresentValue2 << kData.m_iPresentValue3 << kData.m_iPresentValue4 << kData.m_dwLimitDate;
#ifdef SRC_NA
		if( PRESENT_SOLDIER == kData.m_iPresentType )
		{
			if( Help::GetLimitHeroNumber() > 0 && Help::GetLimitHeroNumber() < kData.m_iPresentValue1 )
			{
				LOG.PrintTimeAndLog( 0 , "%s - ID : %s, LimitHerUse: %d", __FUNCTION__, m_pUser->GetPublicID().c_str() , kData.m_iPresentValue1 );
				g_LogDBClient.OnInsertGameLogInfo( 40001, m_pUser, 0, kData.m_iPresentValue1, LogDBClient::OVS_PRESENT_CHAR, 0, 0, 0, 0, NULL );
			}
		}
#endif
		if( kData.m_dwLimitDate != 0 ) // ������ ������ �н�
		{
			if( kData.m_iPresentState == PRESENT_STATE_NEW && kData.m_dwIndex != PRESENT_INDEX_MEMORY )
			{
				// �ϴ� �ѹ� �������� �������� �������� �����Ƿ� �ٷ� ������Ʈ�Ѵ�.
				kData.m_iPresentState = PRESENT_STATE_NORMAL;
				g_DBClient.OnUpdatePresentData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_dwIndex, kData.m_iPresentState );
			}

			// �������� ���� ������ ����
			m_vPresentList.push_back( kData );			
		}		
	}
	m_pUser->SendMessage( kPacket );
	//
	vNewPresent.clear();
}

void ioUserPresent::CheckPresentLimitDate()
{
	if( m_pUser == NULL ) return;

	vPresentData vDelPresent;
	vPresentData::iterator iter = m_vPresentList.begin();
	for(;iter != m_vPresentList.end();iter++)
	{
		PresentData &kData = *iter;			
		SHORT iYear   = (SHORT)( 2000 + ( kData.m_dwLimitDate / 100000000 ) );
		SHORT iMonth  = (SHORT)( ( kData.m_dwLimitDate % 100000000 ) / 1000000 );
		SHORT iDay    = (SHORT)( ( kData.m_dwLimitDate % 1000000 ) / 10000 );
		SHORT iHour   = (SHORT)( ( kData.m_dwLimitDate % 10000 ) / 100 );
		SHORT iMinute = (SHORT)( kData.m_dwLimitDate % 100 );
		CTime cLimitDate( Help::GetSafeValueForCTimeConstructor( iYear, iMonth, iDay, iHour, iMinute, 0 ) );
		if( CTime::GetCurrentTime() >= cLimitDate )
		{
			kData.m_dwLimitDate = 0;
			if( g_ItemInitControl.IsDeletePresentCheck( kData.m_iPresentType, kData.m_iPresentValue1 ) )
				kData.m_iPresentState = PRESENT_STATE_DELETE;     // ������ ������ ó���Ͽ� �˸� �˾��� Ȱ��ȭ���� �ʵ��� �Ѵ�.
			vDelPresent.push_back( kData );
		}
	}

	if( vDelPresent.empty() )
		return;

	// ������ ���� �������� ����
	SP2Packet kPacket( STPK_PRESENT_DATA );
	kPacket << (int)vDelPresent.size() << 0;
	for(iter = vDelPresent.begin();iter != vDelPresent.end();iter++)
	{
		PresentData &kData = *iter;		
		kPacket << kData.m_dwIndex << kData.m_dwSlotIndex << kData.m_szSendID << kData.m_iPresentType << kData.m_iPresentMent
			<< kData.m_iPresentState << kData.m_iPresentValue1 << kData.m_iPresentValue2 << kData.m_iPresentValue3 << kData.m_iPresentValue4 << kData.m_dwLimitDate;
		DeletePresentData( kData.m_dwIndex, kData.m_dwSlotIndex, DPT_TIMEOVER );		
	}
	m_pUser->SendMessage( kPacket );
	vDelPresent.clear();
}

void ioUserPresent::AllDeleteData()
{
	m_vPresentList.clear();
}

DWORD ioUserPresent::GetLastPresentDBIndex()
{
	return m_dwLastDBIndex;
}

DWORD ioUserPresent::GetLastPresentSlotIndex()
{
	m_dwLastSlotIndex++;
	return m_dwLastSlotIndex;
}

bool ioUserPresent::PresentRecv( IN SP2Packet &rkRecvPacket, OUT SP2Packet &rkPacket )
{
 	DWORD dwIndex = 0, dwSlotIndex = 0;

	PACKET_GUARD_bool( rkRecvPacket.Read(dwIndex) );
	PACKET_GUARD_bool( rkRecvPacket.Read(dwSlotIndex) );

	if( !m_pUser )
	{
		rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
		return true;
	}

	PresentData &rkData = GetPresentData( dwIndex, dwSlotIndex );
	if( rkData.m_iPresentType == 0 || rkData.m_iPresentState == PRESENT_STATE_DELETE )
	{
		rkPacket << PRESENT_RECV_NONE_INDEX << dwIndex << dwSlotIndex;
		return true;
	}

	switch( rkData.m_iPresentType )
	{
	case PRESENT_SOLDIER:
		{
			int iResult = m_pUser->SetPresentChar( rkData.m_iPresentValue1, rkData.m_iPresentValue2 );
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );				 
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;			
		}
		return true;
	case PRESENT_DECORATION:
		{
			ITEMSLOT kSlot;
			kSlot.m_item_type = rkData.m_iPresentValue1;
			kSlot.m_item_code = rkData.m_iPresentValue2;
			ioInventory *pInventory = m_pUser->GetInventory();
			if( pInventory->IsSlotItem( kSlot ) )       // �̹� �ִ� ġ�� ����				
			{
				rkPacket << PRESENT_RECV_ALREADY_DECO << dwIndex << dwSlotIndex;
			}
			else 
			{
				DWORD dwDecoSlotIndex = 0;
				int   iSlotArray  = 0;
				pInventory->AddSlotItem( kSlot, false, 0, LogDBClient::DT_PRESENT, dwDecoSlotIndex, iSlotArray );
				if( dwDecoSlotIndex != 0 && dwDecoSlotIndex != ioInventory::NEW_INDEX )
				{
					char szItemIndex[MAX_PATH]="";
					StringCbPrintf( szItemIndex, sizeof( szItemIndex ), "%u-%u", dwDecoSlotIndex, iSlotArray+1 ); // db field�� 1���� �̹Ƿ� +1
					g_LogDBClient.OnInsertDeco( m_pUser, kSlot.m_item_type, kSlot.m_item_code, 0, szItemIndex, LogDBClient::DT_PRESENT );
				}

				// ���� ġ���� �����ϸ� ���� ġ�� �������� �����Ѵ�.
				if( kSlot.m_item_type % 1000 == UID_KINDRED )
				{
					CHARACTER kCharInfo;
					kCharInfo.m_class_type = kSlot.m_item_type/100000;
					kCharInfo.m_sex        = 2; // 1����, 2����
					kCharInfo.m_face       = g_DecorationPrice.GetDefaultDecoCode( 1, UID_FACE, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_FACE, kCharInfo.m_class_type ); // 0����, 1����
					kCharInfo.m_hair       = g_DecorationPrice.GetDefaultDecoCode( 1, UID_HAIR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_HAIR, kCharInfo.m_class_type );
					kCharInfo.m_skin_color = g_DecorationPrice.GetDefaultDecoCode( 1, UID_SKIN_COLOR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_SKIN_COLOR, kCharInfo.m_class_type );
					kCharInfo.m_hair_color = g_DecorationPrice.GetDefaultDecoCode( 1, UID_HAIR_COLOR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_HAIR_COLOR, kCharInfo.m_class_type );	
					kCharInfo.m_underwear  = g_DecorationPrice.GetDefaultDecoCode( 1, UID_UNDERWEAR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_UNDERWEAR, kCharInfo.m_class_type );	
					m_pUser->SetDefaultDecoItem( kCharInfo );
				}
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );

				rkPacket << PRESENT_RECV_OK << dwIndex << dwSlotIndex << kSlot.m_item_type << kSlot.m_item_code;

				ioUserGrowthLevel *pGrowthLevel = m_pUser->GetUserGrowthLevel();
				if( pGrowthLevel )
					pGrowthLevel->AddCharGrowthPointByDecoWoman( kSlot.m_item_type, kSlot.m_item_code );
			}			
		}
		return true;
	case PRESENT_ETC_ITEM:
		{	
			int iResult = m_pUser->SetPresentEtcItem( rkData.m_iPresentValue1, rkData.m_iPresentValue2 );
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );		
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;	
		}
		return true;
	case PRESENT_PESO:
		{
			__int64 iPrevMoney = m_pUser->GetMoney();
			m_pUser->AddMoney( rkData.m_iPresentValue1 );
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_RECV_PRESENT, PRESENT_PESO, 0, rkData.m_iPresentValue1, NULL);

			rkPacket << PRESENT_RECV_OK << dwIndex << dwSlotIndex << m_pUser->GetMoney();
			g_LogDBClient.OnInsertPeso( m_pUser, rkData.m_iPresentValue1, LogDBClient::PT_RECV_PRESENT );		
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );
		}
		return true;
	case PRESENT_EXTRAITEM:
		{
			int iResult = m_pUser->SetPresentExtraItem( rkData.m_iPresentValue1,																	 // ��� �ڵ�
				( rkData.m_iPresentValue2 % PRESENT_EXTRAITEM_DIVISION_1 ) / PRESENT_EXTRAITEM_DIVISION_2,   // ��� ���尪
				rkData.m_iPresentValue2 % PRESENT_EXTRAITEM_DIVISION_2,										 // ��� �Ⱓ
				rkData.m_iPresentValue2 / PRESENT_EXTRAITEM_DIVISION_1,	    								 // ���� ���� Ÿ��
				rkData.m_iPresentValue3, rkData.m_iPresentValue4,											 // ��� ��Ų��
				rkData.m_iPresentMent );																	 // ��� �Ⱓ
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );		
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;	
		}
		return true;
	case PRESENT_EXTRAITEM_BOX:
		{
			int  iPesoArray = ( ( rkData.m_iPresentValue2/100 )%100 ) - 1;
			bool bCash = false;
			if( ( rkData.m_iPresentValue2%100 ) == 1 )
				bCash = true;
			int iResult = m_pUser->SetPresentExtraItemBox( rkData.m_iPresentValue1, iPesoArray, bCash );	// ��񺸱��� ��ȣ
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );		
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;	
		}
		return true;
	case PRESENT_RANDOM_DECO:
		{
			int iSelectClassType = 0;
			rkRecvPacket >> iSelectClassType;

			// �����뺴 üũ
			if( !m_pUser->IsClassTypeExceptExercise( iSelectClassType ) )
			{
				rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv None Char1 : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
				return true;
			}

			int iCharArray = m_pUser->GetCharArrayByClass( iSelectClassType );
			ioCharacter *rkChar = m_pUser->GetCharacter( iCharArray );
			if( rkChar == NULL )
			{
				rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv None Char2 : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
				return true;
			}

			if( !rkChar->IsActive() )
			{
				rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv None Char3 : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
				return true;
			}
			//

			// ���õ� �뺴�� ġ������ ����!
			int iItemType = iSelectClassType * 100000 + ( rkData.m_iPresentValue1 % 100000 ); // m_iPresentValue1�� class Ÿ���� �� �� �����Ƿ� % 100000 �����Ѵ�. 
			int iItemCode = rkData.m_iPresentValue2;

			ITEMSLOT kSlot;
			kSlot.m_item_type = iItemType;
			kSlot.m_item_code = iItemCode;


			ioInventory *pInventory = m_pUser->GetInventory();
			if( pInventory->IsSlotItem( kSlot ) )       // �̹� �ִ� ġ�� ����				
			{
				rkPacket << PRESENT_RECV_ALREADY_DECO << dwIndex << dwSlotIndex;
				return true;
			}
			else 
			{
				// �� ���� ����...
				int iItemSexType = (rkData.m_iPresentValue1 % 100000) / 1000;
				if( iItemSexType == 1 )		// ��ĳ�� ���̸� ��ĳ �ִ��� üũ �ʿ�
				{
					ITEMSLOT kSexSlot;
					const int iSexType = 0;
					kSexSlot.m_item_type = ( iSelectClassType*100000 ) + ( iSexType * 1000 ) + UID_KINDRED; // �ش� ������ ���� ġ��
					kSexSlot.m_item_code = RDT_HUMAN_WOMAN;

					if( !pInventory->IsSlotItem( kSexSlot ) )
					{
						rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv None Sex : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
						return true;
					}
				}

				// ���� ġ���� �ٷ� ����. �������� ���� ������ ������ �� ������ �������� ����.
				bool bEqual = false;
				if( (kSlot.m_item_type % 1000 == UID_KINDRED) || (rkChar->GetCharInfo().m_sex - 1) == iItemSexType )
					bEqual = true;

				// ���� ��ü
				if( !bEqual )
				{
					ITEMSLOT kTempSlot;
					if( iItemSexType == 1 )
					{
						kTempSlot.m_item_type = ( iSelectClassType*100000 ) + UID_KINDRED;
						kTempSlot.m_item_code = RDT_HUMAN_WOMAN;
					}
					else
					{
						kTempSlot.m_item_type = ( iSelectClassType*100000 ) + UID_KINDRED;
						kTempSlot.m_item_code = RDT_HUMAN_MAN;
					}

					if( !pInventory->IsSlotItem( kTempSlot ) )
					{
						rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
						return true;
					}

					if( rkChar->SetCharDecoration( kTempSlot.m_item_type, kTempSlot.m_item_code ) )
					{
						pInventory->SetEquipItem( kTempSlot.m_item_type, kTempSlot.m_item_code);

						const CHARACTER &rkCharInfo = rkChar->GetCharInfo();
						CHARACTER rkChangeInfo = rkCharInfo;
						pInventory->GetEquipItemCode( rkChangeInfo );
						rkChar->SetChangeKindred( rkChangeInfo, m_pUser->GetPrivateID().GetHashCode() );
					}
					else
					{
						rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
						return true;
					}
				}

				// ���ο� ġ���� ����
				if( !rkChar->SetCharDecoration( kSlot.m_item_type, kSlot.m_item_code ) )
				{
					rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv SetDecoFail : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
					return true;
				}

				DWORD dwDecoSlotIndex = 0;
				int   iSlotArray  = 0;

				pInventory->AddSlotItem( kSlot, false, 0, LogDBClient::DT_PRESENT, dwDecoSlotIndex, iSlotArray );
				pInventory->SetEquipItem( kSlot.m_item_type, kSlot.m_item_code );

				if( dwDecoSlotIndex != 0 && dwDecoSlotIndex != ioInventory::NEW_INDEX )
				{
					char szItemIndex[MAX_PATH]="";
					StringCbPrintf( szItemIndex, sizeof( szItemIndex ), "%u-%u", dwDecoSlotIndex, iSlotArray+1 ); // db field�� 1���� �̹Ƿ� +1
					g_LogDBClient.OnInsertDeco( m_pUser, kSlot.m_item_type, kSlot.m_item_code, 0, szItemIndex, LogDBClient::DT_PRESENT );
				}

				// ���� ġ���� �����ϸ� ���� ġ�� �������� �����Ѵ�.
				if( kSlot.m_item_type % 1000 == UID_KINDRED )
				{
					CHARACTER kCharInfo;
					kCharInfo.m_class_type = kSlot.m_item_type/100000;
					kCharInfo.m_sex        = 2; // 1����, 2����
					kCharInfo.m_face       = g_DecorationPrice.GetDefaultDecoCode( 1, UID_FACE, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_FACE, kCharInfo.m_class_type ); // 0����, 1����
					kCharInfo.m_hair       = g_DecorationPrice.GetDefaultDecoCode( 1, UID_HAIR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_HAIR, kCharInfo.m_class_type );
					kCharInfo.m_skin_color = g_DecorationPrice.GetDefaultDecoCode( 1, UID_SKIN_COLOR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_SKIN_COLOR, kCharInfo.m_class_type );
					kCharInfo.m_hair_color = g_DecorationPrice.GetDefaultDecoCode( 1, UID_HAIR_COLOR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_HAIR_COLOR, kCharInfo.m_class_type );	
					kCharInfo.m_underwear  = g_DecorationPrice.GetDefaultDecoCode( 1, UID_UNDERWEAR, m_pUser->GetPrivateID().GetHashCode() + kCharInfo.m_class_type + UID_UNDERWEAR, kCharInfo.m_class_type );	
					m_pUser->SetDefaultDecoItem( kCharInfo );
					rkChar->SetCharAllDecoration( *pInventory );
				}
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );

				DWORD dwCharIndex = rkChar->GetCharIndex();
				const CHARACTER &rCharinfo = rkChar->GetCharInfo();

				rkPacket << PRESENT_RECV_OK << dwIndex << dwSlotIndex << kSlot.m_item_type << kSlot.m_item_code << dwCharIndex << (CHARACTER)rCharinfo << bEqual;

				ioUserGrowthLevel *pGrowthLevel = m_pUser->GetUserGrowthLevel();
				if( pGrowthLevel )
					pGrowthLevel->AddCharGrowthPointByDecoWoman( kSlot.m_item_type, kSlot.m_item_code );

				m_pUser->SaveInventory();
				m_pUser->SaveUserData();

				if( rkChar && m_pUser->GetMyRoom() )
				{
					m_pUser->GetMyRoom()->OnModeCharDecoUpdate( m_pUser, rkChar );
				}
			}		
		}
		return true;
	case PRESENT_GRADE_EXP:
		{
			int iPrevGradeLevel = m_pUser->GetGradeLevel();
			int iPrevGradeExp   = m_pUser->GetGradeExpert();

			if( m_pUser->AddGradeExp( rkData.m_iPresentValue1 ) )
				m_pUser->SendGradeSync();

			rkPacket << PRESENT_RECV_OK << dwIndex << dwSlotIndex << m_pUser->GetGradeLevel() << m_pUser->GetGradeExpert();
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );
		}
		return true;
	case PRESENT_MEDALITEM:
		{
			int iResult = m_pUser->SetPresentMedalItem( rkData.m_iPresentValue1,  // �޴�Ÿ��
				rkData.m_iPresentValue2); // �޴޻��Ⱓ
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );		
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;	
		}
		return true;
	case PRESENT_ALCHEMIC_ITEM:
		{
			int iResult = m_pUser->SetPresentAlchemicItem( rkData.m_iPresentValue1,	// code
				rkData.m_iPresentValue2 );	// count
			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );		
			}
			rkPacket << iResult << dwIndex << dwSlotIndex;
		}
		return true;
	case PRESENT_PET_ITEM:
		{
			if( rkData.m_iPresentValue1 == 0 || rkData.m_iPresentValue2 == 0 )
			{
				PACKET_GUARD_bool( rkPacket.Write( PRESENT_RECV_EXCEPTION ) );
				PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
				PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
				return true;;
			}

			int iPetCode = rkData.m_iPresentValue1;;
			int iPetLevel = rkData.m_iPresentValue2 / 10000;	//�� ����
			int iPetRank = rkData.m_iPresentValue2 % 10000;     //�� ��ũ

			int iResult = m_pUser->SetPresentPetItem( iPetCode, iPetLevel, iPetRank, dwIndex, dwSlotIndex );

			if( iResult != PRESENT_RECV_OK )
			{
				PACKET_GUARD_bool( rkPacket.Write( iResult ) );
				PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
				PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
				return true;;
			}
		}
		return false;
	case PRESENT_COSTUME:
		{
			if( rkData.m_iPresentValue1 == 0 )
			{
				PACKET_GUARD_bool( rkPacket.Write( PRESENT_RECV_EXCEPTION ) );
				PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
				PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
				return true;
			}
			int iResult = m_pUser->SetPresentCostume( rkData.m_iPresentValue1,     // ��� �ڵ�
				rkData.m_iPresentValue2	,									   // ��� �Ⱓ
				dwIndex, dwSlotIndex
				);	

			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );	
			}
		
			PACKET_GUARD_bool( rkPacket.Write( iResult ) );
			PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
			return true;
		}

	case PRESENT_BONUS_CASH:
		{
			if( rkData.m_iPresentValue1 == 0 )
			{
				PACKET_GUARD_bool( rkPacket.Write( PRESENT_RECV_EXCEPTION ) );
				PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
				PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
				return true;
			}

			int iResult = m_pUser->SetPresentBonusCash( 
				rkData.m_iPresentValue1,     // �ݾ�
				rkData.m_iPresentValue2	,	 // ��ȿ �Ⱓ
				dwIndex, dwSlotIndex );

			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );	
			}
		
			PACKET_GUARD_bool( rkPacket.Write( iResult ) );
			PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
			return true;
		}

	case PRESENT_ACCESSORY:
		{
			if( rkData.m_iPresentValue1 == 0 )
			{
				PACKET_GUARD_bool( rkPacket.Write( PRESENT_RECV_EXCEPTION ) );
				PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
				PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );
				return true;
			}
			int iResult = m_pUser->SetPresentAccessory( rkData.m_iPresentValue1,
				rkData.m_iPresentValue2	,									   
				dwIndex, dwSlotIndex
				);	

			if( iResult == PRESENT_RECV_OK )
			{
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_RECV );	
			}
		
			PACKET_GUARD_bool( rkPacket.Write( iResult ) );
			PACKET_GUARD_bool( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_bool( rkPacket.Write( dwSlotIndex ) );

			return true;
		}
	
	default:
		{
			rkPacket << PRESENT_RECV_EXCEPTION << dwIndex << dwSlotIndex;
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentRecv None Type : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
		}
		return true;
	}
}

void ioUserPresent::PresentSell( DWORD dwIndex, DWORD dwSlotIndex, SP2Packet &rkPacket )
{
	if( !m_pUser )
	{
		rkPacket << PRESENT_SELL_EXCEPTION << dwIndex << dwSlotIndex;
		return;
	}

	PresentData &rkData = GetPresentData( dwIndex, dwSlotIndex );
	if( rkData.m_iPresentType == 0 || rkData.m_iPresentState == PRESENT_STATE_DELETE )
	{
		rkPacket << PRESENT_SELL_NONE_INDEX << dwIndex << dwSlotIndex;
		return;
	}

	int iMileagePeso = 0;
	bool bMileage    = false;
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal && pLocal->IsMileage() )
	{
		bMileage = true;
	}

	switch( rkData.m_iPresentType )
	{
	case PRESENT_SOLDIER:
		{
			int iResellPeso = 0;

			if( rkData.m_iPresentValue2 == 0 )      // ���� �뺴
			{
				iResellPeso  = g_ItemPriceMgr.GetMortmainCharResellPeso( rkData.m_iPresentValue1 );
			}
			else
			{
				if(ioLocalManager::GetLocalType() == ioLocalManager::LCT_LATIN)
				{
					int nResellPesoSec = rkData.m_iPresentValue2;	// ���� �ʷ� �ٲ۴�.

					if(rkData.m_iPresentValue2 <= 43200 ) // hms 30�� �̻��̸� ���� (30�� �̻�¥�� �������� ����.)
						iResellPeso  = g_ItemPriceMgr.GetTimeCharResellPeso( rkData.m_iPresentValue1, nResellPesoSec  );
				}
				else
				{
					iResellPeso  = g_ItemPriceMgr.GetTimeCharResellPeso( rkData.m_iPresentValue1, rkData.m_iPresentValue2  );
				}
			}

			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_SOLDIER, rkData.m_iPresentValue1, iResellPeso, NULL);
			}			
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;	
	case PRESENT_DECORATION:
		{
			int iResellPeso = max( 0, g_DecorationPrice.GetSellPeso( rkData.m_iPresentValue1, rkData.m_iPresentValue2 ) );
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, (rkData.m_iPresentValue1 % 1000), rkData.m_iPresentValue2, iResellPeso, NULL);
			}			
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );			
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_ETC_ITEM:
		{	
			ioEtcItem* pEtcItem =  g_EtcItemMgr.FindEtcItem( rkData.m_iPresentValue1 );
			if( !pEtcItem )
			{
				rkPacket << PRESENT_SELL_NONE_ITEM << dwIndex << dwSlotIndex;
			}
			else
			{
				if( 1004067 == rkData.m_iPresentValue1 )
				{
					rkPacket << PRESENT_SELL_NOT_SELL << dwIndex << dwSlotIndex;
					return;
				}

				int iResellPeso = pEtcItem->GetSellPeso() * rkData.m_iPresentValue2;
				if( (rkData.m_iPresentValue1 / ioEtcItem::USE_TYPE_CUT_VALUE) == ioEtcItem::UT_DATE &&
					rkData.m_iPresentValue2 == 0 )
				{
					iResellPeso = pEtcItem->GetMortmainSellPeso();
				}

				if( bMileage )
				{
					iMileagePeso = iResellPeso;
					iResellPeso  = 0;
				}
				if( iResellPeso > 0 )
				{
					__int64 iPrevMoney = m_pUser->GetMoney();
					m_pUser->AddMoney( iResellPeso );
					m_pUser->SaveUserData();	
					g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
					g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_ETC_ITEM, rkData.m_iPresentValue1, iResellPeso, NULL);
				}			
				m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
				DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
				rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
			}			
		}
		return;
	case PRESENT_PESO:
		{
			int iResellPeso = rkData.m_iPresentValue1;
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );				
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_PESO, 0, iResellPeso, NULL);
			}
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_EXTRAITEM:
		{
			int iItemCode = rkData.m_iPresentValue1;
			int iItemReinforce = ( rkData.m_iPresentValue2 % PRESENT_EXTRAITEM_DIVISION_1 ) / PRESENT_EXTRAITEM_DIVISION_2;  // ��� ���尪
			int iItemLimitTime = rkData.m_iPresentValue2 % PRESENT_EXTRAITEM_DIVISION_2;   // ��� �Ⱓ

			// Peso ���
			// �Ҽ��� ������ ���� ���߿� �Ҽ��������� ����.
			float fReturnPeso = 0.0f;
			if( iItemLimitTime == 0 )  // ������
			{
				fReturnPeso = g_ExtraItemInfoMgr.GetMortmainItemSellPeso()  * ( 1 + ( (float)iItemReinforce / 25 ) * g_ExtraItemInfoMgr.GetItemSellConst() );
			}
			else if( iItemLimitTime > 0 )
			{				
				DWORD dwTotalMinutes = ( iItemLimitTime * 60 );         // �ð�->�� 
				fReturnPeso = dwTotalMinutes * g_ExtraItemInfoMgr.GetTimeItemSellPeso() * ( 1 + ( (float)iItemReinforce / 25 ) * g_ExtraItemInfoMgr.GetItemSellConst() );;
				fReturnPeso = max( 0, fReturnPeso );
			}

			int iResellPeso = (int)fReturnPeso;
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_EXTRAITEM, iItemCode, iResellPeso, NULL);
			}		
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();			
		}
		return;
	case PRESENT_EXTRAITEM_BOX:
		{
			int iResellPeso = g_ExtraItemInfoMgr.GetSellPeso( rkData.m_iPresentValue1 );
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_EXTRAITEM_BOX, rkData.m_iPresentValue1, iResellPeso, NULL);
			}		
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_RANDOM_DECO:
		{
			int iResellPeso = max( 0, g_DecorationPrice.GetSellPeso( rkData.m_iPresentValue1, rkData.m_iPresentValue2 ) );
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, (rkData.m_iPresentValue1 % 1000), rkData.m_iPresentValue2, iResellPeso, NULL);
			}			
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );			
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_GRADE_EXP:
		{
			int iResellPeso = rkData.m_iPresentValue1;
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}
			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_GRADE_EXP, rkData.m_iPresentValue1, iResellPeso, NULL);
			}			
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );			
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_MEDALITEM:
		{
			int iItemType      = rkData.m_iPresentValue1;
			int iItemLimitTime = rkData.m_iPresentValue2;

			float fReturnPeso = 0.0f;
			if( iItemLimitTime == 0 )
			{
				fReturnPeso = g_MedalItemMgr.GetSellPeso( iItemType );
			}
			else
			{
				fReturnPeso = g_MedalItemMgr.GetSellPeso( iItemType );
				fReturnPeso = fReturnPeso / g_MedalItemMgr.GetSellPesoByMinute();

				CTime kCurTime = CTime::GetCurrentTime();
				CTimeSpan kAddTime( 0, iItemLimitTime, 0, 0 );
				CTime kLimitTime = kCurTime + kAddTime;
				CTimeSpan kRemainTime = kLimitTime - kCurTime;
				DWORD dwTotalTime = 0;
				if( kRemainTime.GetTotalMinutes() > 0 )
					dwTotalTime = kRemainTime.GetTotalMinutes();

				fReturnPeso = dwTotalTime * fReturnPeso;
				fReturnPeso = min( fReturnPeso, g_MedalItemMgr.GetSellPeso( iItemType ) );
			}
			__int64 iPeso = static_cast<__int64>(fReturnPeso);

			if( bMileage )
			{
				iMileagePeso = iPeso;
				iPeso  = 0;
			}
			if( iPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iPeso );
				m_pUser->SaveUserData();	
				g_LogDBClient.OnInsertPeso( m_pUser, static_cast<int>(iPeso), LogDBClient::PT_SELL_PRESENT );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_MEDALITEM, rkData.m_iPresentValue1, iPeso, NULL);
			}			
			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << static_cast<int>(iPeso) << m_pUser->GetMoney();			
		}
		return;
	case PRESENT_ALCHEMIC_ITEM:
		{
			int iCode = rkData.m_iPresentValue1;
			int iItemCnt = rkData.m_iPresentValue2;
			float fSellConst = g_AlchemicMgr.GetSellConst();

			int iResellPeso = iItemCnt * fSellConst;
			if( bMileage )
			{
				iMileagePeso = iResellPeso;
				iResellPeso  = 0;
			}

			if( iResellPeso > 0 )
			{
				__int64 iPrevMoney = m_pUser->GetMoney();
				m_pUser->AddMoney( iResellPeso );
				m_pUser->SaveUserData();
				g_LogDBClient.OnInsertPeso( m_pUser, iResellPeso, LogDBClient::PT_ALCHEMIC_PESO );
				g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_ALCHEMIC_ITEM, rkData.m_iPresentValue1, iResellPeso, NULL);
			}

			m_pUser->SendBillingAddMileage( rkData.m_iPresentType, rkData.m_iPresentValue1, rkData.m_iPresentValue2, iMileagePeso, true );

			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );
			rkPacket << PRESENT_SELL_OK << dwIndex << dwSlotIndex << iResellPeso << m_pUser->GetMoney();
		}
		return;
	case PRESENT_PET_ITEM:
		{
			int iReturnPeso = g_PetInfoMgr.GetSellPeso();
			__int64 iPeso = static_cast<__int64>(iReturnPeso);
			m_pUser->AddMoney( iPeso );
			g_LogDBClient.OnInsertPeso( m_pUser, static_cast<int>(iPeso), LogDBClient::PT_SELL_PRESENT );
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_PET_ITEM, rkData.m_iPresentValue1, iPeso, NULL);
			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );

			PACKET_GUARD_VOID( rkPacket.Write( PRESENT_SELL_OK ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwSlotIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( static_cast<int>(iPeso) ) );
			PACKET_GUARD_VOID( rkPacket.Write(  m_pUser->GetMoney() ) );
			return;
		}
	case PRESENT_COSTUME:
		{
			int iItemLimitTime = rkData.m_iPresentValue2;
			int iReturnPeso = 0;
			if( iItemLimitTime == 0 )
			{
				//����
				iReturnPeso = g_CostumeMgr.GetMortmainItemPrice();
			}
			else if( iItemLimitTime > 0 )
			{
				iReturnPeso = g_CostumeMgr.GetTimeItemPrice();
			}
			__int64 iPeso = static_cast<__int64>(iReturnPeso);
			m_pUser->AddMoney( iPeso );
			g_LogDBClient.OnInsertPeso( m_pUser, static_cast<int>(iPeso), LogDBClient::PT_SELL_PRESENT );
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_COSTUME, rkData.m_iPresentValue1, iPeso, NULL);

			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );

			PACKET_GUARD_VOID( rkPacket.Write( PRESENT_SELL_OK ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwSlotIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( iReturnPeso ) );
			PACKET_GUARD_VOID( rkPacket.Write( m_pUser->GetMoney() ) );
			return;
		}
	case PRESENT_ACCESSORY:
		{
			int iReturnPeso = g_AccessoryMgr.GetAccessorySellPeso(rkData.m_iPresentValue1);
			
			__int64 iPeso = static_cast<__int64>(iReturnPeso);
			m_pUser->AddMoney( iPeso );
			g_LogDBClient.OnInsertPeso( m_pUser, static_cast<int>(iPeso), LogDBClient::PT_SELL_PRESENT );
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, m_pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_SELL_PRESENT, PRESENT_ACCESSORY, rkData.m_iPresentValue1, iPeso, NULL);

			DeletePresentData( rkData.m_dwIndex, rkData.m_dwSlotIndex, DPT_SELL );

			PACKET_GUARD_VOID( rkPacket.Write( PRESENT_SELL_OK ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( dwSlotIndex ) );
			PACKET_GUARD_VOID( rkPacket.Write( iReturnPeso ) );
			PACKET_GUARD_VOID( rkPacket.Write( m_pUser->GetMoney() ) );
			return;
		}

	default:
		{
			rkPacket << PRESENT_SELL_EXCEPTION << dwIndex << dwSlotIndex;
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserPresent::PresentSell None Type : %s - %d", m_pUser->GetPublicID().c_str(), rkData.m_iPresentType );
		}
		return;
	}
}


void ioUserPresent::AddPresentMemory( const ioHashString &szSendName, short iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, 
	int iPresentValue4, short iPresentMent, CTime &rkLimitTime, short iPresentState )
{
	PresentData kPresent;
	kPresent.m_dwIndex = PRESENT_INDEX_MEMORY;
	kPresent.m_dwSlotIndex = GetLastPresentSlotIndex();

	kPresent.m_dwSendUserIndex = 0;				
	kPresent.m_szSendID		   = szSendName;	
	kPresent.m_iPresentType    = iPresentType;	
	kPresent.m_iPresentMent	   = iPresentMent;  
	kPresent.m_iPresentState   = iPresentState; 
	kPresent.m_iPresentValue1  = iPresentValue1;
	kPresent.m_iPresentValue2  = iPresentValue2;
	kPresent.m_iPresentValue3  = iPresentValue3;
	kPresent.m_iPresentValue4  = iPresentValue4;
	kPresent.m_dwLimitDate	   = Help::ConvertCTimeToDate( g_ItemInitControl.CheckPresentFixedLimitDate( iPresentType, iPresentValue1, rkLimitTime ) );

	if( m_vPresentList.size() < ( g_PresentHelper.GetCanPresentCnt() + 100 ) ) 
	{
		m_vPresentList.push_back( kPresent );
		m_vTempMemoryData.push_back( kPresent );
	}

}

void ioUserPresent::SendPresentMemory()
{
	if( m_pUser == NULL ) return;

	int iSendSize = m_vTempMemoryData.size();
	if( iSendSize == 0 ) return;

	SP2Packet kPacket( STPK_PRESENT_DATA );
	kPacket << iSendSize << 0;
	vPresentData::iterator iter = m_vTempMemoryData.begin();
	for(iter = m_vTempMemoryData.begin();iter != m_vTempMemoryData.end();iter++)
	{
		PresentData &kData = *iter;		
		kPacket << kData.m_dwIndex << kData.m_dwSlotIndex << kData.m_szSendID << kData.m_iPresentType << kData.m_iPresentMent
			<< kData.m_iPresentState << kData.m_iPresentValue1 << kData.m_iPresentValue2 << kData.m_iPresentValue3 << kData.m_iPresentValue4 << kData.m_dwLimitDate;
	}
	m_pUser->SendMessage( kPacket );
	m_vTempMemoryData.clear();
}

void ioUserPresent::LogoutMemoryPresentInsert()
{
	if( m_pUser == NULL ) return;

	int iPresentInsert = 0;
	vPresentData::iterator iter = m_vPresentList.begin();
	for(;iter != m_vPresentList.end();iter++)
	{
		PresentData &kData = *iter;		
		if( kData.m_dwIndex != PRESENT_INDEX_MEMORY ) continue;
		if( kData.m_dwSlotIndex == 0 ) continue;
		if( kData.m_iPresentType == 0 ) continue;

		if( kData.m_dwIndex == PRESENT_INDEX_MEMORY )
		{
			iPresentInsert++;

			SHORT iYear   = (SHORT)( 2000 + ( kData.m_dwLimitDate / 100000000 ) );
			SHORT iMonth  = (SHORT)( ( kData.m_dwLimitDate % 100000000 ) / 1000000 );
			SHORT iDay    = (SHORT)( ( kData.m_dwLimitDate % 1000000 ) / 10000 );
			SHORT iHour   = (SHORT)( ( kData.m_dwLimitDate % 10000 ) / 100 );
			SHORT iMinute = (SHORT)( kData.m_dwLimitDate % 100 );
			CTime cLimitDate( Help::GetSafeValueForCTimeConstructor( iYear, iMonth, iDay, iHour, iMinute, 0 ) );

			g_DBClient.OnInsertPresentData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), kData.m_szSendID, m_pUser->GetPublicID(), 
				kData.m_iPresentType, kData.m_iPresentValue1, kData.m_iPresentValue2, kData.m_iPresentValue3, 
				kData.m_iPresentValue4, kData.m_iPresentMent, cLimitDate, 0 );

			// �ι� Insert ��Ű�� �ʱ� ���� �޸𸮿� �ִ� �����͸� �����Ѵ�.
			kData.m_dwIndex = kData.m_dwSlotIndex = 0;
			kData.m_iPresentType = 0;
		}
	}
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LogoutMemoryPresentInsert : %s - %d��", m_pUser->GetPublicID().c_str(), iPresentInsert );
}

void ioUserPresent::FillMoveData( SP2Packet &rkPacket )
{
	rkPacket << m_dwLastDBIndex << m_dwLastSlotIndex;

	rkPacket << (int)m_vPresentList.size();
	vPresentData::iterator iter = m_vPresentList.begin();
	for(;iter != m_vPresentList.end();iter++)
	{
		PresentData &kData = *iter;		
		rkPacket << kData.m_dwIndex << kData.m_dwSlotIndex << kData.m_szSendID << kData.m_iPresentType << kData.m_iPresentMent << kData.m_iPresentState 
			<< kData.m_iPresentValue1 << kData.m_iPresentValue2 << kData.m_iPresentValue3 << kData.m_iPresentValue4 << kData.m_dwLimitDate;
	}
}

void ioUserPresent::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode )
{
	rkPacket >> m_dwLastDBIndex >> m_dwLastSlotIndex;

	int iSize;
	rkPacket >> iSize;
	for(int i = 0;i < iSize;i++)
	{
		PresentData kData;		
		rkPacket >> kData.m_dwIndex >> kData.m_dwSlotIndex >> kData.m_szSendID >> kData.m_iPresentType >> kData.m_iPresentMent >> kData.m_iPresentState 
			>> kData.m_iPresentValue1 >> kData.m_iPresentValue2 >> kData.m_iPresentValue3 >> kData.m_iPresentValue4 >> kData.m_dwLimitDate;
		m_vPresentList.push_back( kData );
	}	
}

BOOL ioUserPresent::HaveAThisItem(DWORD dwType, DWORD dwItemCode)
{
	for( int i = 0; i < (int)m_vPresentList.size(); i++ )
	{
		PresentData& stInfo	= m_vPresentList[i];

		if( stInfo.m_iPresentType == dwType && stInfo.m_iPresentValue1 == dwItemCode )
			return TRUE;
	}

	for( int i = 0; i < (int)m_vTempMemoryData.size(); i++ )
	{
		PresentData& stInfo	= m_vPresentList[i];

		if( stInfo.m_iPresentType == dwType && stInfo.m_iPresentValue1 == dwItemCode )
			return TRUE;
	}

	return FALSE;
}