
#ifndef _FishingManager_h_
#define _FishingManager_h_

#include "../Util/Singleton.h"
#include "../Util/IORandom.h"

class User;

struct FishingItemInfo
{
	int m_iItemNum;
	__int64 m_iPeso;

	bool m_bRoomAlarm;
	bool m_bAllAlarm;
	bool m_bSellAlarm;

	bool m_bSpecial;

	FishingItemInfo()
	{
		m_iItemNum = 0;
		m_iPeso = 0;

		m_bRoomAlarm = false;
		m_bAllAlarm = true;
		m_bSellAlarm = false;

		m_bSpecial = false;
	}
};
typedef std::vector<FishingItemInfo> FishingItemInfoList;


struct FishingItemRate
{
	int m_iItemNum;
	int m_iEventPresent;

	DWORD m_dwRate;

	bool m_bSellAlarm;

	FishingItemRate()
	{
		m_iItemNum = 0;
		m_iEventPresent = 0;

		m_dwRate = 0;

		m_bSellAlarm = false;
	}
};
typedef std::vector<FishingItemRate> FishingItemRateList;

struct FishingItemGradeInfo
{
	int m_iGradeNum;
	float m_fValue;
	bool m_bAlarm;

	FishingItemGradeInfo()
	{
		m_iGradeNum = 0;
		m_fValue = 1.0f;
		m_bAlarm = false;
	}
};
typedef std::vector<FishingItemGradeInfo> FishingItemGradeInfoList;

struct FishingItemGradeRate
{
	int m_iGradeNum;
	float m_fValue;
	DWORD m_dwRate;

	FishingItemGradeRate()
	{
		m_iGradeNum = 0;
		m_fValue = 1.0f;
		m_dwRate = 0;
	}
};
typedef std::vector<FishingItemGradeRate> FishingItemGradeRateList;


struct FishingTime
{
	DWORD m_dwMinTime;
	DWORD m_dwMaxTime;

	FishingTime()
	{
		m_dwMinTime = 0;
		m_dwMaxTime = 0;
	}
};
typedef std::vector<FishingTime> FishingTimeList;


class FishingManager : public Singleton< FishingManager >
{
protected:
	FishingTimeList m_vFishingTimeList;
	DWORDVec m_vSuccessRateList;

	FishingItemInfoList m_vFishingItemList;
	FishingItemGradeInfoList m_vFishingItemGradeList;

	FishingItemRateList m_vNormalRateList;
	FishingItemRateList m_vEventRateList;
	FishingItemRateList m_vPCRoomRateList;

	FishingItemGradeRateList m_vNormalGradeRateList;
	FishingItemGradeRateList m_vEventGradeRateList;
	FishingItemGradeRateList m_vPCRoomGradeRateList;

	IORandom m_RandomTime;
	IORandom m_RandomSuccess;
	IORandom m_RandomType;
	IORandom m_RandomGrade;

	DWORD m_dwItemListTotal;
	DWORD m_dwEventItemListTotal;
	DWORD m_dwPCRoomItemListTotal;

	DWORD m_dwGradeTotal;
	DWORD m_dwEventGradeTotal;
	DWORD m_dwPCRoomGradeTotal;

	// PC Room
	int   m_iPCRoomSuccesPlusRate;
	int   m_iPCRoomFishTimeMinusMin;
	int   m_iPCRoomFishTimeMinusMax;

	// Test¿ë
	bool m_bTestFishing;

	FishingItemInfoList m_vGuildFisheryItemList;
	FishingItemRateList m_vGuildFisheryRateList;
	FishingItemGradeRateList m_vGuildFisheryGradeRateList;

	DWORD m_dwGuildFisheryItemListTotal;
	DWORD m_dwGuildFisheryGradeTotal;

public:
	void CheckNeedReload();
	void LoadFishingInfo();
 
protected:
	void LoadBaseInfo();
	void LoadRateInfo();

	void ClearAllInfo();
	void ClearBaseInfo();
	void ClearRateInfo();

public:
	bool IsFishingSuccess( int iType, bool bPCRoom );
	bool IsRoomAlarm( int iItemType );
	bool IsAllAlarm( int iItemType );
	bool IsSellAlarm( int iItemType, int iGrade, bool bEvent, bool bPCRoom );
	bool IsSpecial( int iItemType, bool bGuildFishery = false );

	inline bool IsTestFishing() const { return m_bTestFishing; }

	int GetFishingItemNum( bool bEvent, bool bPCRoom, bool bGuildFishery );
	int GetGrade( bool bEvent, bool bPCRoom, bool bGuildFishery );
	__int64 GetFishingItemSellPeso( int iItemType, int iItemGrade, bool bEvent, bool bPCRoom, bool bGuildFishery );
	int GetEventFishingPresentNum( int iItemNum, bool bEvent, bool bPCRoom );

	DWORD GetCurFishingTime( int iType, bool bPCRoom );

	BOOL IsGuildFishery(const DWORD dwCode);

public:
	static FishingManager& GetSingleton();

public:
	FishingManager();
	virtual ~FishingManager();
};

#define g_FishingMgr FishingManager::GetSingleton()

#endif
