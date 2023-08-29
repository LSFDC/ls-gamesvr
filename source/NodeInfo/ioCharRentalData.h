
#ifndef _ioCharRentalData_h_
#define _ioCharRentalData_h_

#include "NodeHelpStructDefine.h"
#include "ioUserGrowthLevel.h"

struct RentalData
{
	ioHashString m_szOwnerName;

	int       m_dwCharIndex;
	int       m_iClassLevel;
	ITEM_DATA m_EquipItem[MAX_CHAR_DBITEM_SLOT];
	DWORDVec  m_EquipMedal;
	BYTE      m_CharGrowth[MAX_CHAR_GROWTH];
	BYTE      m_ItemGrowth[MAX_ITEM_GROWTH];

	RentalData()
	{
		Initialize();
	}

	void Initialize()
	{
		m_dwCharIndex = 0;
		m_iClassLevel = 0;

		int i = 0;
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
			m_EquipItem[i].Initialize();
		for(i = 0;i < MAX_CHAR_GROWTH;i++)
			m_CharGrowth[i] = 0;
		for(i = 0;i < MAX_ITEM_GROWTH;i++)
			m_ItemGrowth[i] = 0;

		m_EquipMedal.clear();
	}

	void FillData( SP2Packet &rkPacket )
	{
		rkPacket << m_dwCharIndex;
		rkPacket << m_iClassLevel;

		int i = 0;
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
		{
			rkPacket << m_EquipItem[i].m_item_code << m_EquipItem[i].m_item_reinforce;
			rkPacket << m_EquipItem[i].m_item_male_custom << m_EquipItem[i].m_item_female_custom;
		}
   
		rkPacket << (int)m_EquipMedal.size();
		for(i = 0;i < (int)m_EquipMedal.size();i++)
			rkPacket << m_EquipMedal[i];

		for(i = 0;i < MAX_CHAR_GROWTH;i++)
			rkPacket << m_CharGrowth[i];
		
		for(i = 0;i < MAX_ITEM_GROWTH;i++)
			rkPacket << m_ItemGrowth[i];
	}

	void ApplyData( SP2Packet &rkPacket )
	{
		rkPacket >> m_dwCharIndex;
		rkPacket >> m_iClassLevel;

		int i = 0;
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
		{
			rkPacket >> m_EquipItem[i].m_item_code >> m_EquipItem[i].m_item_reinforce;
			rkPacket >> m_EquipItem[i].m_item_male_custom >> m_EquipItem[i].m_item_female_custom;
		}

		int iMedalSize;
		rkPacket >> iMedalSize;
		for(i = 0;i < iMedalSize;i++)
		{
			int iMedalCode;
			rkPacket >> iMedalCode;
			m_EquipMedal.push_back( iMedalCode );
		}

		for(i = 0;i < MAX_CHAR_GROWTH;i++)
			rkPacket >> m_CharGrowth[i];

		for(i = 0;i < MAX_ITEM_GROWTH;i++)
			rkPacket >> m_ItemGrowth[i];
	}
};
typedef std::vector< RentalData > RentalDataList;

class ioCharRentalData
{
protected:	
	RentalDataList m_RentalDataList;
    
public:
	void Initialize();

public:
	int GetClassLevel( const DWORD dwCharIndex );
	void GetEquipItem( const DWORD dwCharIndex, ITEM_DATA &rkEquipItem, int iSlot );
	void GetEquipMedal( const DWORD dwCharIndex, IntVec &rkEquipMedal );
	void GetCharGrowth( const DWORD dwCharIndex, BYTE &rkCharGrowth, int iSlot );
	void GetItemGrowth( const DWORD dwCharIndex, BYTE &rkItemGrowth, int iSlot );
	RentalData &GetRentalData( const DWORD dwCharIndex );

public:
	void InsertRentalData( const ioHashString &rkOwnerName, RentalData &rkRentalData );
	void DeleteRentalData( const DWORD dwCharIndex );

public:
	void FillMoveData( SP2Packet &rkPacket );
	void ApplyMoveData( SP2Packet &rkPacket );

public:
	bool FillGrowthData( const DWORD dwCharIndex, SP2Packet &rkPacket );

public:
	ioCharRentalData();
	virtual ~ioCharRentalData();
};

#endif