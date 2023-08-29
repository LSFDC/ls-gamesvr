
#include "stdafx.h"

#include "ioCharRentalData.h"


ioCharRentalData::ioCharRentalData()
{
}

ioCharRentalData::~ioCharRentalData()
{
}

void ioCharRentalData::Initialize()
{
	m_RentalDataList.clear();
}

int ioCharRentalData::GetClassLevel( const DWORD dwCharIndex )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			return rkRentalData.m_iClassLevel;
		}
	}
	return 0;
}

void ioCharRentalData::GetEquipItem( const DWORD dwCharIndex, ITEM_DATA &rkEquipItem, int iSlot )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			if( COMPARE( iSlot, 0, MAX_CHAR_DBITEM_SLOT ) )
				rkEquipItem = rkRentalData.m_EquipItem[iSlot];
			break;
		}
	}
}

void ioCharRentalData::GetEquipMedal( const DWORD dwCharIndex, IntVec &rkEquipMedal )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			for(int i = 0;i < (int)rkRentalData.m_EquipMedal.size();i++)
			{
				rkEquipMedal.push_back( rkRentalData.m_EquipMedal[i] );
			}
			break;
		}
	}
}

void ioCharRentalData::GetCharGrowth( const DWORD dwCharIndex, BYTE &rkCharGrowth, int iSlot )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			if( COMPARE( iSlot, 0, MAX_CHAR_GROWTH ) )
				rkCharGrowth = rkRentalData.m_CharGrowth[iSlot];
			break;
		}
	}
}

void ioCharRentalData::GetItemGrowth( const DWORD dwCharIndex, BYTE &rkItemGrowth, int iSlot )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			if( COMPARE( iSlot, 0, MAX_ITEM_GROWTH ) )
				rkItemGrowth = rkRentalData.m_ItemGrowth[iSlot];
			break;
		}
	}
}

RentalData &ioCharRentalData::GetRentalData( const DWORD dwCharIndex )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			return rkRentalData;
		}
	}

	static RentalData stNoneData;
	return stNoneData;
}

void ioCharRentalData::InsertRentalData( const ioHashString &rkOwnerName, RentalData &rkRentalData )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkData = *iter;
		if( rkData.m_dwCharIndex == rkRentalData.m_dwCharIndex )
		{
			m_RentalDataList.erase( iter );
			break;
		}
	}
	rkRentalData.m_szOwnerName = rkOwnerName;
	m_RentalDataList.push_back( rkRentalData );
}

void ioCharRentalData::DeleteRentalData( const DWORD dwCharIndex )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkData = *iter;
		if( rkData.m_dwCharIndex == dwCharIndex )
		{
			m_RentalDataList.erase( iter );
			break;
		}
	}
}

void ioCharRentalData::FillMoveData( SP2Packet &rkPacket )
{
	rkPacket << (int)m_RentalDataList.size();
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		rkPacket << rkRentalData.m_szOwnerName;
		rkRentalData.FillData( rkPacket );
	}
}

void ioCharRentalData::ApplyMoveData( SP2Packet &rkPacket )
{
	int iSize;
	rkPacket >> iSize;
	for(int i = 0;i < iSize;i++)
	{
		RentalData kRentalData;
		rkPacket >> kRentalData.m_szOwnerName;
		kRentalData.ApplyData( rkPacket );
		m_RentalDataList.push_back( kRentalData );
	}	
}

bool ioCharRentalData::FillGrowthData( const DWORD dwCharIndex, SP2Packet &rkPacket )
{
	RentalDataList::iterator iter = m_RentalDataList.begin();
	for(;iter != m_RentalDataList.end();++iter)
	{
		RentalData &rkRentalData = *iter;
		if( rkRentalData.m_dwCharIndex == dwCharIndex )
		{
			//Char
			for( int j=0; j < MAX_CHAR_GROWTH; ++j )
			{
				rkPacket << rkRentalData.m_CharGrowth[j];
			}

			//Item
			for( int k=0; k < MAX_ITEM_GROWTH; ++k )
			{
				rkPacket << rkRentalData.m_ItemGrowth[k];
			}			
			return true;
		}
	}
	return false;
}