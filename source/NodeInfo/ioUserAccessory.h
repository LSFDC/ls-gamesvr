#pragma once

#include "Accessory.h"

class User;
class ioCharacter;

class ioUserAccessory
{
public:
	ioUserAccessory();
	~ioUserAccessory();

	void Init(User* pUser);
	void Destroy();

public:
	void DBtoData( CQueryResultData *query_data, int& iLastIndex );	
	void FillMoveData( SP2Packet &rkPacket );
	void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	void SetEquipInfo(int& iClassType, int iAccessoryCode, int iAccessoryIndex, int iAccessoryValue);

	void AddAccessoryItem(Accessory& AccessoryInfo);
	bool DeleteAccessoryItem(DWORD& dwIndex);

	void EquipAccessory(const int iClassType, Accessory* pAccessory);

	void ReleaseAccessoryWithAccessoryIndex(DWORD dwIndex);
	void ReleaseAccessoryWithClassType(int iClassType);
	void ReleaseAccessory(Accessory* pAccessory);

	void DeleteAccessoryPassedDate(IntVec &vDeleteIndex);

	int ChangeEquipInfo(int iClassArray, DWORD& dwTargetIndex, int iEquipPos, bool bEquip, Accessory* pAccessory);

	bool IsEnableAdd();
	bool IsEmpty();

	Accessory* GetAccessory(DWORD dwIndex);
	int GetAccessoryEquipPos(int iCode);

	void SendAllAccessoryInfo();

protected:
	typedef boost::unordered_map<DWORD, Accessory> UserAccessoryItem;	//<인덱스, 해당아이템>

protected:
	User* m_pUser;
	UserAccessoryItem m_mUserAccessoryMap;
};