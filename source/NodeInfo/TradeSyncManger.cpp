#include "stdafx.h"
#include "TradeSyncManger.h"

template<> TradeSyncManager* Singleton< TradeSyncManager >::ms_Singleton = 0;

TradeSyncManager::TradeSyncManager()
{
	Init();
}


TradeSyncManager::~TradeSyncManager()
{
	Destroy();
}


TradeSyncManager& TradeSyncManager::GetSingleton()
{
	return Singleton< TradeSyncManager >::GetSingleton();
}


void TradeSyncManager::Init()
{
	m_mapTradeSyncInfo.clear();
}


void TradeSyncManager::Destroy()
{
	m_mapTradeSyncInfo.clear();
}

// ���� ������ ���� ��� �ŷ��� ������ ����Ʈ�� �޴´�.
void TradeSyncManager::RecvAllTradeItem( SP2Packet& rkPacket )
{
	TradeSyncInfo	stTradeSyncInfo;
	int				iSize = 0;

	PACKET_GUARD_VOID( rkPacket.Read( iSize ) );

	for( int i = 0; i < iSize; ++i )
	{		
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwUserIndex ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwTradeIndex ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemType ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemMagicCode ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemValue ) );

		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemMaleCustom ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemFemaleCustom ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_iItemPrice ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterDate1 ) );
		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterDate2 ) );

		PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterPeriod ) );

		mapTradeSyncInfo::iterator iter = m_mapTradeSyncInfo.find( stTradeSyncInfo.m_dwTradeIndex );
		if( iter != m_mapTradeSyncInfo.end() )
			return;
		else
			m_mapTradeSyncInfo.insert( make_pair( stTradeSyncInfo.m_dwTradeIndex, stTradeSyncInfo ) );
	}
}


// ���� ������ Ŭ���̾�Ʈ�� ������ ����Ʈ�� �����ش�.
void TradeSyncManager::SendTradeItemList( User *pUser )
{
	int iMaxSize		= m_mapTradeSyncInfo.size();
	int	iDivideValue	= 0;						// 200 ���� ���� �Ǵ� ��
	BYTE byListType		= TRADE_ITEM_LIST_FIRST;	// ���۵Ǵ� ������ ����Ʈ Ÿ��.
													// 0 : ó�� ���۵Ǵ� ������ ����Ʈ
													// 1 : ���� ���� ������ ����Ʈ
													// 2 : ������ ������ ����Ʈ
	int iListValue		= 0;

	static vTradeSyncInfo vSendTradeItem;
	vSendTradeItem.clear();

	mapTradeSyncInfo::iterator iter = m_mapTradeSyncInfo.begin();
	for( ; iter != m_mapTradeSyncInfo.end(); ++iter )
	{
		TradeSyncInfo stTradeItem;
		stTradeItem.m_dwUserIndex			= iter->second.m_dwUserIndex;
		stTradeItem.m_dwTradeIndex			= iter->second.m_dwTradeIndex;
		stTradeItem.m_dwItemType			= iter->second.m_dwItemType;
		stTradeItem.m_dwItemMagicCode		= iter->second.m_dwItemMagicCode;
		stTradeItem.m_dwItemValue			= iter->second.m_dwItemValue;

		stTradeItem.m_dwItemMaleCustom		= iter->second.m_dwItemMaleCustom;
		stTradeItem.m_dwItemFemaleCustom	= iter->second.m_dwItemFemaleCustom;
		stTradeItem.m_iItemPrice			= iter->second.m_iItemPrice;
		stTradeItem.m_dwRegisterDate1		= iter->second.m_dwRegisterDate1;
		stTradeItem.m_dwRegisterDate2		= iter->second.m_dwRegisterDate2;

		stTradeItem.m_dwRegisterPeriod		= iter->second.m_dwRegisterPeriod;

		vSendTradeItem.push_back( stTradeItem );

		++iListValue;

		// �ѹ� ���� �� �ִ� ������ �Ǿ��� �� ( 200�� )
		if( iDivideValue == MAX_SEND_ITEM )
		{
			SP2Packet kPacket( STPK_TRADE_LIST_REQ );

			PACKET_GUARD_VOID( kPacket.Write( byListType ) );
			PACKET_GUARD_VOID( kPacket.Write( iDivideValue ) );

			iDivideValue	= 0;

			for( int j = 0; j < MAX_SEND_ITEM; ++j )
			{
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwUserIndex ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwTradeIndex ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemType ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemMagicCode ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemValue ) );

				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemMaleCustom ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemFemaleCustom ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_iItemPrice ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterDate1 ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterDate2 ) );

				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterPeriod ) );
			}

			pUser->SendMessage( kPacket );

			// ó�� ���� �� Ÿ���� TRADE_ITEM_LIST_SENDING �� �ٲ��ش�.
			byListType = TRADE_ITEM_LIST_SENDING;

			vSendTradeItem.clear();
		}
		else
		{
			++iDivideValue;
		}

		// ���� �� ������ ����Ʈ ������ ����Ʈ�� �� ���
		if( iListValue == iMaxSize )
		{
			// ������ ���� Ÿ���� TRADE_ITEM_LIST_SENDING �� ��츸 TRADE_ITEM_LIST_LAST �� �ٲ��ش�.
			// �ִ� ���� (200��) �� �ѱ��� ���� ���� TRADE_ITEM_LIST_FIRST �� �����ش�.
			if( byListType ==  TRADE_ITEM_LIST_SENDING)
				byListType = TRADE_ITEM_LIST_LAST;

			SP2Packet kPacket( STPK_TRADE_LIST_REQ );

			PACKET_GUARD_VOID( kPacket.Write( byListType ) );
			PACKET_GUARD_VOID( kPacket.Write( iDivideValue ) );

			for( int j = 0; j < iDivideValue; ++j )
			{
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwUserIndex ) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwTradeIndex) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemType) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemMagicCode) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemValue) );

				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemMaleCustom) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwItemFemaleCustom) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_iItemPrice) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterDate1) );
				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterDate2) );

				PACKET_GUARD_VOID( kPacket.Write( vSendTradeItem[j].m_dwRegisterPeriod) );
			}

			pUser->SendMessage( kPacket );

			vSendTradeItem.clear();
		}
	}
}



// ���� ������ ���� ���� �ŷ��� ������ �߰� ����
void TradeSyncManager::RecvAddTradeItem( SP2Packet& rkPacket )
{
	TradeSyncInfo stTradeSyncInfo;
	
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwTradeIndex ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwUserIndex ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemType ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemMagicCode ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemValue ) );

	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemMaleCustom ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_iItemPrice ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterDate1 ) );
	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterDate2 ) );

	PACKET_GUARD_VOID( rkPacket.Read( stTradeSyncInfo.m_dwRegisterPeriod ) );

	mapTradeSyncInfo::iterator iter = m_mapTradeSyncInfo.find( stTradeSyncInfo.m_dwTradeIndex );
	if( iter != m_mapTradeSyncInfo.end() )
		return;
	else
		m_mapTradeSyncInfo.insert( make_pair( stTradeSyncInfo.m_dwTradeIndex, stTradeSyncInfo ) );	
}


// ���� ������ ���� ���� �ŷ��� ������ ���� ����
void TradeSyncManager::RecvDelTradeItem( SP2Packet& rkPacket )
{
	DWORD dwTradeIndex = 0;
	PACKET_GUARD_VOID( rkPacket.Read( dwTradeIndex ) );

	mapTradeSyncInfo::iterator iter = m_mapTradeSyncInfo.find( dwTradeIndex );
	if( iter != m_mapTradeSyncInfo.end() )
		m_mapTradeSyncInfo.erase( dwTradeIndex );
}