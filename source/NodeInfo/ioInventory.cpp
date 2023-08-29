
#include "stdafx.h"

//#include "../Window.h"
#include "../MainProcess.h"
#include "../EtcHelpFunc.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "User.h"
#include "ioDecorationPrice.h"

#include "ioInventory.h"
#include <strsafe.h>


extern CLog CriticalLOG;

ioInventory::ioInventory()
{
	Initialize( NULL );
}

ioInventory::~ioInventory()
{
	m_vItemList.clear();
}

void ioInventory::Initialize( User *pUser )
{
	m_pUser = pUser;
	m_vItemList.clear();
}

void ioInventory::InsertDBInventory( ITEMDB &kItemDB, bool bBuyCash, int iBuyPrice, int iLogType )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::InsertDBInventory() User NULL!!"); 
		return;
	}

	std::vector<int> contents;
	contents.reserve(MAX_SLOT * 2);
	for(int i = 0;i < MAX_SLOT;i++)
	{
		contents.push_back( kItemDB.m_Slot[i].m_item_type );
		contents.push_back( kItemDB.m_Slot[i].m_item_code );
	}

	// ���� �߰��� �κ��丮 DB Insert
	g_DBClient.OnInsertInvenData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetGUID(), m_pUser->GetPublicID(), m_pUser->GetUserIndex(), contents, bBuyCash, iBuyPrice, iLogType );
}

bool ioInventory::DBtoNewIndex( DWORD dwIndex )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::DBtoNewIndex() User NULL!!"); 
		return false;
	}

	{	// �� �ε��� ������ �н�
		bool bEmptyIndex = false;
		vITEMDB::iterator iter, iEnd;
		iEnd = m_vItemList.end();
		for(iter = m_vItemList.begin();iter != iEnd;iter++)
		{
			ITEMDB &kItemDB = *iter;
			if( kItemDB.m_dwIndex == NEW_INDEX )
			{
				bEmptyIndex = true;
				break;
			}
		}

		if( !bEmptyIndex )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::DBtoNewIndex() None Empty Index : %s - %d", m_pUser->GetPublicID().c_str(), dwIndex  ); 
			return false;
		}
	}

	{	// �̹� �����ϰ� �ִ� �ε������ �ٽ� �����´�.
		vITEMDB::iterator iter, iEnd;
		iEnd = m_vItemList.end();
		for(iter = m_vItemList.begin();iter != iEnd;iter++)
		{
			ITEMDB &kItemDB = *iter;
			if( kItemDB.m_dwIndex == dwIndex )
			{
				g_DBClient.OnSelectInvenIndex( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetGUID(), m_pUser->GetPublicID(), m_pUser->GetUserIndex(), false, 0, 0 );
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::DBtoNewIndex() Already Index : %s - %d", m_pUser->GetPublicID().c_str(), dwIndex  );
				return false;
			}
		}
	}

	{   // �� �ε����� ���� �ε��� ����
		vITEMDB::iterator iter, iEnd;
		iEnd = m_vItemList.end();
		for(iter = m_vItemList.begin();iter != iEnd;iter++)
		{
			ITEMDB &kItemDB = *iter;
			if( kItemDB.m_dwIndex == NEW_INDEX )
			{
				kItemDB.m_dwIndex = dwIndex;
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ioInventory::DBtoNewIndex() Add : %s - %d", m_pUser->GetPublicID().c_str(), dwIndex  );
				return true;
			}
		}
	}
	return false;
}

int ioInventory::SendInventory( int iStartArray, int iSendSize )
{
	if( m_pUser == NULL )
		return (int)m_vItemList.size();

	int iMaxList = (int)m_vItemList.size();
	if( iStartArray >= iMaxList )
		return iMaxList;

	iSendSize = min( iMaxList - iStartArray, iSendSize );

	SP2Packet kPacket( STPK_SLOT_ITEM );

	PACKET_GUARD_INT( kPacket.Write(iSendSize) );

	int iLoop = iStartArray;
	for(;iLoop < iStartArray + iSendSize;iLoop++)
	{
		ITEMDB &kItemDB = m_vItemList[iLoop];
		for(int i = 0;i < MAX_SLOT;i++)
		{
			PACKET_GUARD_INT( kPacket.Write(kItemDB.m_Slot[i].m_item_type) );
			PACKET_GUARD_INT( kPacket.Write(kItemDB.m_Slot[i].m_item_code) ); 
			PACKET_GUARD_INT( kPacket.Write(kItemDB.m_Slot[i].m_item_equip) );
		}
	}		
	m_pUser->SendMessage( kPacket );
	return iLoop;
}

void ioInventory::DBtoData( CQueryResultData *query_data )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::DBtoData() User NULL!!"); 
		return;
	}

	Initialize( m_pUser );

	LOOP_GUARD();
	while( query_data->IsExist() )
	{		
		ITEMDB kItemDB;
		PACKET_GUARD_BREAK( query_data->GetValue( kItemDB.m_dwIndex, sizeof(int) ) );
		for(int i = 0;i < MAX_SLOT;i++)
		{
			PACKET_GUARD_BREAK( query_data->GetValue( kItemDB.m_Slot[i].m_item_type, sizeof(int) ) );
			int iDBItemCode = 0;
			PACKET_GUARD_BREAK(query_data->GetValue( iDBItemCode, sizeof(int) ) );
			kItemDB.m_Slot[i].m_item_code  = iDBItemCode % 10000;
			kItemDB.m_Slot[i].m_item_equip = iDBItemCode / 10000;
		} 
		m_vItemList.push_back( kItemDB );
	}
	LOOP_GUARD_CLEAR();
	g_CriticalError.CheckDecoTableCount( m_pUser->GetPublicID(), m_vItemList.size() );

	if( !m_vItemList.empty() )
	{
		// ���� ���� - ��Ŷ ����� 1024byte�� �ǹǷ� ���� ������. Slot * 4 = 960byte
		const int iSendListSize = 4;
		for(int iStartArray = 0;iStartArray < (int)m_vItemList.size();)
		{
			iStartArray = SendInventory( iStartArray, iSendListSize ); //���н� -1�ε� ��� ó���Ұ��ΰ�? 
		}

/*		SP2Packet kPacket( STPK_SLOT_ITEM );
		kPacket << (int)m_vItemList.size();      
		{
			vITEMDB::iterator iter, iEnd;
			iEnd = m_vItemList.end();
			for(iter = m_vItemList.begin();iter != iEnd;iter++)
			{
				ITEMDB &kItemDB = *iter;
				for(int i = 0;i < MAX_SLOT;i++)
				{
					kPacket << kItemDB.m_Slot[i].m_item_type << kItemDB.m_Slot[i].m_item_code << kItemDB.m_Slot[i].m_item_equip;
				}
			}
		}		
		m_pUser->SendMessage( kPacket );
*/
		//m_pUser->SetAllCharAllDecoration();
	}
}

void ioInventory::DBtoData( CQueryResultData *query_data, int& iLastIndex )
{
	if( !m_pUser )
		return;
	
	LOOP_GUARD();
	while( query_data->IsExist() )
	{		
		ITEMDB kItemDB;

		PACKET_GUARD_BREAK( query_data->GetValue( kItemDB.m_dwIndex, sizeof(int) ) );

		for(int i = 0;i < MAX_SLOT;i++)
		{
			PACKET_GUARD_BREAK( query_data->GetValue( kItemDB.m_Slot[i].m_item_type, sizeof(int) ) );
			int iDBItemCode = 0;
			PACKET_GUARD_BREAK(query_data->GetValue( iDBItemCode, sizeof(int) ) );
			kItemDB.m_Slot[i].m_item_code  = iDBItemCode % 10000;
			kItemDB.m_Slot[i].m_item_equip = iDBItemCode / 10000;
		} 

		m_vItemList.push_back( kItemDB );
		iLastIndex	= kItemDB.m_dwIndex;
	}

	LOOP_GUARD_CLEAR();
}

void ioInventory::SaveData()
{
	if( m_vItemList.empty() ) return;
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::SaveData() User NULL!!"); 
		return;
	}

	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;
		if( kItemDB.m_bChange )
		{
			std::vector<int> contents;
			contents.reserve(MAX_SLOT * 2);
			for(int i = 0;i < MAX_SLOT;i++)
			{
				int iDBItemCode = ( (kItemDB.m_Slot[i].m_item_equip * 10000) + kItemDB.m_Slot[i].m_item_code );
				contents.push_back( kItemDB.m_Slot[i].m_item_type );
				contents.push_back( iDBItemCode );
			}

			if( kItemDB.m_dwIndex == NEW_INDEX )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "SaveInventory(%s:%d) : None Index", m_pUser->GetPublicID().c_str(), kItemDB.m_dwIndex );
				//LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s", szContent );
			}
			else
			{
				g_DBClient.OnUpdateInvenData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetPublicID(), kItemDB.m_dwIndex, contents );
				kItemDB.m_bChange = false;
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "SaveInventory(%s:%d)", m_pUser->GetPublicID().c_str(), kItemDB.m_dwIndex );
			}
		}		
	}
}
//////////////////////////////////////////////////////////////////////////

bool ioInventory::IsSlotItem( ITEMSLOT &kSlot )
{
	if( kSlot.m_item_type%1000 == UID_KINDRED )
	{
		// ���� ġ��
		// ������ �������� �ʴ´�.
		if( kSlot.m_item_code == RDT_HUMAN_MAN ) return true;        //�ΰ� ���ڴ� �⺻

		vITEMDB::iterator iter;
		for(iter = m_vItemList.begin();iter != m_vItemList.end();iter++)
		{
			ITEMDB &kItemDB = *iter;
			for(int i = 0;i < MAX_SLOT;i++)
			{
				if( ( kItemDB.m_Slot[i].m_item_type / 100000 ) != ( kSlot.m_item_type / 100000 ) ) continue;
				if( kItemDB.m_Slot[i].m_item_type % 1000 != UID_KINDRED ) continue;

				if( kItemDB.m_Slot[i].m_item_code == kSlot.m_item_code )
					return true;
			}		
		}
	}
	else
	{
		vITEMDB::iterator iter, iEnd;
		iEnd = m_vItemList.end();
		for(iter = m_vItemList.begin();iter != iEnd;iter++)
		{
			ITEMDB &kItemDB = *iter;
			for(int i = 0;i < MAX_SLOT;i++)
			{
				if( kItemDB.m_Slot[i].m_item_type == kSlot.m_item_type &&
					kItemDB.m_Slot[i].m_item_code == kSlot.m_item_code )
					return true;
			}		
		}
	}
	return false;
}

bool ioInventory::IsFull()
{
	return (GetCount() >= MAX_ROW) ? true : false;
}

bool ioInventory::FindOtherSlotItem( IN ITEMSLOT &kSlot, OUT ITEMSLOT &rkSlot )
{
	rkSlot.Initialize();
	IntVec vItemCodeList;

	if( kSlot.m_item_type%1000 == UID_KINDRED )
	{
		// ���� ġ��
		// ������ �������� �ʴ´�.
		vITEMDB::iterator iter;
		for(iter = m_vItemList.begin();iter != m_vItemList.end();iter++)
		{
			ITEMDB &kItemDB = *iter;
			for(int i = 0;i < MAX_SLOT;i++)
			{
				if( kItemDB.m_Slot[i].m_item_type % 1000 != UID_KINDRED ) continue;
				if( ( kItemDB.m_Slot[i].m_item_type / 100000 ) != ( kSlot.m_item_type / 100000 ) ) continue;

				if( kSlot.m_item_code == RDT_HUMAN_WOMAN )
				{
					vItemCodeList.push_back( RDT_HUMAN_MAN );
				}
				else
				{
					if( kItemDB.m_Slot[i].m_item_code != kSlot.m_item_code )
						vItemCodeList.push_back( kItemDB.m_Slot[i].m_item_code );
				}
			}		
		}
	}
	else
	{
		vITEMDB::iterator iter, iEnd;
		iEnd = m_vItemList.end();
		for(iter = m_vItemList.begin();iter != iEnd;iter++)
		{
			ITEMDB &kItemDB = *iter;
			for(int i = 0;i < MAX_SLOT;i++)
			{
				// ���� Ÿ�Կ� �ٸ� �ڵ尪
				if( kItemDB.m_Slot[i].m_item_type == kSlot.m_item_type &&
					kItemDB.m_Slot[i].m_item_code != kSlot.m_item_code )
				{
					vItemCodeList.push_back( kItemDB.m_Slot[i].m_item_code );
				}
			}
		}
	}

	if( vItemCodeList.empty() )
		return false;

	std::random_shuffle( vItemCodeList.begin(), vItemCodeList.end() );
	rkSlot.m_item_type = kSlot.m_item_type;
	rkSlot.m_item_code = vItemCodeList.front();
	return true;
}

bool ioInventory::AddSlotItem( IN ITEMSLOT &kSlot, IN bool bBuyCash, IN int iBuyPrice, IN int iLogType, OUT DWORD &rdwIndex, OUT int &riArray )
{
	//ġ�� ���Խ� ġ�� �� �ʰ��� ��� return ���� ���� ��Ҵ� ���ҵǰ� ġ�� �������� ����ȵǴ� ������ ��ȯ���� bool�� ����.

	if( kSlot.m_item_type == 0 && kSlot.m_item_code == 0 )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioInventory::AddSlotItem Type and code are Zero." );
		return false;
	}

	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;
		for(int i = 0;i < MAX_SLOT;i++)
		{
			if( kItemDB.m_Slot[i].m_item_type == 0 &&
				kItemDB.m_Slot[i].m_item_code == 0 )
			{
				kItemDB.m_Slot[i].m_item_type = kSlot.m_item_type;
				kItemDB.m_Slot[i].m_item_code = kSlot.m_item_code;
				kItemDB.m_bChange = true;

				rdwIndex = kItemDB.m_dwIndex;
				riArray  = i;
				return true;
			}
		}		
	}

	// 20131210 youngdie, ġ�� ���� �ִ븦 �Ѿ�� �� ó��
	/*if( IsFull() && !bFullSkip )
	{
		CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Decoration is full %s-%d(%d, %d)", m_pUser->GetPublicID().c_str(), GetCount(), kSlot.m_item_type, kSlot.m_item_code );
		return false;
	}*/ //20140321 ������, ġ�� ���� �ִ븦 �Ѿ�� ��� ���Խ� ��Ҵ� �����ǰ� �������� ���� ���ϴ� ����. ġ�� ������ ���� �� ��ܿ��� �ִ� ���� ���� �߰�. 

	// New
	ITEMDB kItemDB;
	kItemDB.m_dwIndex = NEW_INDEX;
	kItemDB.m_Slot[0].m_item_type = kSlot.m_item_type;
	kItemDB.m_Slot[0].m_item_code = kSlot.m_item_code;
	m_vItemList.push_back( kItemDB );
	InsertDBInventory( kItemDB , bBuyCash, iBuyPrice, iLogType );

	if( m_pUser )
	{
		g_CriticalError.CheckDecoTableCount( m_pUser->GetPublicID(), m_vItemList.size() );
	}
	return true;
}

RaceDetailType ioInventory::GetEquipRaceType( const int iClassType )
{
	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;
		for(int i = 0;i < MAX_SLOT;i++)
		{
			if( kItemDB.m_Slot[i].m_item_equip == 0 ) continue;
			if( kItemDB.m_Slot[i].m_item_type / 100000 != iClassType ) continue;
			if( kItemDB.m_Slot[i].m_item_type % 1000 != UID_KINDRED ) continue;

			return (RaceDetailType)kItemDB.m_Slot[i].m_item_code;
		}		
	}
	return RDT_HUMAN_MAN;        // �ΰ� ���ڴ� �κ��丮�� �����Ƿ� �⺻���̴�.
}

void ioInventory::GetEquipItemCode( CHARACTER &charInfo )
{
	RaceDetailType eRaceType = GetEquipRaceType( charInfo.m_class_type );
	switch( eRaceType )
	{
	case RDT_HUMAN_MAN:
		charInfo.m_kindred = 1;
		charInfo.m_sex = 1;
		break;
	case RDT_HUMAN_WOMAN:
		charInfo.m_kindred = 1;
		charInfo.m_sex = 2;
		break;
	case RDT_ELF_MAN:
		charInfo.m_kindred = 2;
		charInfo.m_sex = 1;
		break;
	case RDT_ELF_WOMAN:
		charInfo.m_kindred = 2;
		charInfo.m_sex = 2;
		break;
	case RDT_DWARF_MAN:
		charInfo.m_kindred = 3;
		charInfo.m_sex = 1;
		break;
	case RDT_DWARF_WOMAN:
		charInfo.m_kindred = 3;
		charInfo.m_sex = 2;
		break;
	}

	int iItemType = ( charInfo.m_class_type * 100000 ) + ( ( charInfo.m_sex - 1) * 1000 );
	// ��       UID_FACE
	charInfo.m_face = GetEquipItemCode( iItemType + UID_FACE );
	// �Ӹ�       UID_HAIR  
	charInfo.m_hair = GetEquipItemCode( iItemType + UID_HAIR );
	// �Ǻλ�     UID_SKIN_COLOR
	charInfo.m_skin_color = GetEquipItemCode( iItemType + UID_SKIN_COLOR );
	// �Ӹ���     UID_HAIR_COLOR
	charInfo.m_hair_color = GetEquipItemCode( iItemType + UID_HAIR_COLOR );
	// �ӿ�       UID_UNDERWEAR
	charInfo.m_underwear = GetEquipItemCode( iItemType + UID_UNDERWEAR );
}

int ioInventory::GetEquipItemCode( const int iItemType )
{
	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;
		for(int i = 0;i < MAX_SLOT;i++)
		{
			if( kItemDB.m_Slot[i].m_item_type  == iItemType &&
				kItemDB.m_Slot[i].m_item_equip ==  1 ) // ���� 
			{
				return ( kItemDB.m_Slot[i].m_item_code );				
			}
		}		
	}

	return -1;
}

void ioInventory::SetEquipItem( const int iItemType, const int iItemCode )
{
	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;
		for(int i = 0;i < MAX_SLOT;i++)
		{
			// ���� ġ�� 
			if( iItemType % 1000 == UID_KINDRED )
			{
				if( iItemType / 100000 != kItemDB.m_Slot[i].m_item_type / 100000 ) continue;
				if( UID_KINDRED != kItemDB.m_Slot[i].m_item_type % 1000 ) continue;

				if( kItemDB.m_Slot[i].m_item_code == iItemCode)
				{
					kItemDB.m_Slot[i].m_item_equip = 1;
					kItemDB.m_bChange = true;
				}
				else
				{
					kItemDB.m_Slot[i].m_item_equip = 0;
					kItemDB.m_bChange = true;
				}
			}
			else      //�Ϲ� ġ��
			{
				if( kItemDB.m_Slot[i].m_item_type == iItemType && kItemDB.m_Slot[i].m_item_code == iItemCode)
				{
					kItemDB.m_Slot[i].m_item_equip = 1;
					kItemDB.m_bChange = true;
				} 
				else if( kItemDB.m_Slot[i].m_item_type == iItemType )
				{
					kItemDB.m_Slot[i].m_item_equip = 0;
					kItemDB.m_bChange = true;
				}
			}
		}		
	}
}

void ioInventory::DecoFillMoveData( SP2Packet &rkPacket, int iStartIndex )
{
	//�ѹ��� ġ�� �� 100���� ���� , �ϳ��� �࿡ 20��������. �� 2000�� ġ�� ���� ����
	int iEndIndex = iStartIndex + DB_DECO_SELECT_COUNT;

	if( iEndIndex > GetCount() )
		iEndIndex = GetCount();

	PACKET_GUARD_VOID( rkPacket.Write( iStartIndex ) );
	PACKET_GUARD_VOID( rkPacket.Write( iEndIndex ) );

	for( int i =  iStartIndex; i < iEndIndex; i++ )
	{
		PACKET_GUARD_VOID( rkPacket.Write( m_vItemList[i].m_dwIndex )  );
		PACKET_GUARD_VOID( rkPacket.Write( m_vItemList[i].m_bChange ) );

		for( int j = 0; j < MAX_SLOT; j++ )
		{
			PACKET_GUARD_VOID( rkPacket.Write( m_vItemList[i].m_Slot[j].m_item_type ) );
			PACKET_GUARD_VOID( rkPacket.Write( m_vItemList[i].m_Slot[j].m_item_code ) );
			PACKET_GUARD_VOID( rkPacket.Write( m_vItemList[i].m_Slot[j].m_item_equip ) );
		}
	}

}

void ioInventory::DecoApplyMoveData( SP2Packet &rkPacket, bool bDummyNode  )
{
	int iStartIndex = 0;
	int iEndIndex = 0;

	PACKET_GUARD_VOID( rkPacket.Read( iStartIndex ) );
	PACKET_GUARD_VOID( rkPacket.Read( iEndIndex ) );

	for( int i = iStartIndex; i < iEndIndex; i++ )
	{
		ITEMDB kItemDB;
		PACKET_GUARD_VOID( rkPacket.Read( kItemDB.m_dwIndex ) );
		PACKET_GUARD_VOID( rkPacket.Read( kItemDB.m_bChange ) );

		for( int j = 0; j < MAX_SLOT; j++ ) 
		{
			PACKET_GUARD_VOID( rkPacket.Read( kItemDB.m_Slot[j].m_item_type ) );
			PACKET_GUARD_VOID( rkPacket.Read( kItemDB.m_Slot[j].m_item_code ) );
			PACKET_GUARD_VOID( rkPacket.Read( kItemDB.m_Slot[j].m_item_equip ) );
		}
		if( kItemDB.m_dwIndex == NEW_INDEX && !bDummyNode )
			g_DBClient.OnSelectInvenIndex( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetGUID(), m_pUser->GetPublicID(), m_pUser->GetUserIndex(), false, 0, 0 );

		m_vItemList.push_back( kItemDB );
	}

	if( m_pUser )
	{
		g_CriticalError.CheckDecoTableCount( m_pUser->GetPublicID(), m_vItemList.size() );
	}
}

void ioInventory::FillMoveData( SP2Packet &rkPacket )
{
	rkPacket << (int)m_vItemList.size();

	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();

	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{
		ITEMDB &kItemDB = *iter;

		PACKET_GUARD_VOID(rkPacket.Write(kItemDB.m_dwIndex));
		PACKET_GUARD_VOID(rkPacket.Write(kItemDB.m_bChange));
		for(int i = 0;i < MAX_SLOT;i++)
		{
			PACKET_GUARD_VOID(rkPacket.Write(kItemDB.m_Slot[i].m_item_type) );
			PACKET_GUARD_VOID(rkPacket.Write(kItemDB.m_Slot[i].m_item_code) );
			PACKET_GUARD_VOID(rkPacket.Write(kItemDB.m_Slot[i].m_item_equip) );
		}
	}
}

void ioInventory::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode )
{
	int iSize;
	rkPacket >> iSize;

	for(int i = 0;i < iSize;i++)
	{
		ITEMDB kItemDB;
		PACKET_GUARD_VOID(rkPacket.Read(kItemDB.m_dwIndex));
		PACKET_GUARD_VOID(rkPacket.Read(kItemDB.m_bChange));
		for(int j = 0;j < MAX_SLOT;j++)
		{
			PACKET_GUARD_VOID(rkPacket.Read(kItemDB.m_Slot[j].m_item_type) );
			PACKET_GUARD_VOID(rkPacket.Read(kItemDB.m_Slot[j].m_item_code) );
			PACKET_GUARD_VOID(rkPacket.Read(kItemDB.m_Slot[j].m_item_equip) );
		}
		if( kItemDB.m_dwIndex == NEW_INDEX && !bDummyNode )
			g_DBClient.OnSelectInvenIndex( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetGUID(), m_pUser->GetPublicID(), m_pUser->GetUserIndex(), false, 0, 0 );
		m_vItemList.push_back( kItemDB );
	}	
	
	if( m_pUser )
	{
		g_CriticalError.CheckDecoTableCount( m_pUser->GetPublicID(), m_vItemList.size() );
	}
}

bool ioInventory::GetItemInfo( IN DWORD dwIndex, OUT ITEMSLOT kItemSlot[MAX_SLOT] )
{
	bool bReturn = false;
	vITEMDB::iterator iter, iEnd;
	iEnd = m_vItemList.end();
	for(iter = m_vItemList.begin();iter != iEnd;iter++)
	{	
		ITEMDB &kItemDB = *iter;
		if( kItemDB.m_dwIndex != dwIndex )
			continue;

		for(int i = 0;i < MAX_SLOT;i++)
		{
			kItemSlot[i] = kItemDB.m_Slot[i];
			bReturn = true;
		}		

		if( bReturn )
			break;
	}

	return bReturn;
}

