#pragma once

#include "../Util/Singleton.h"
#include "../Util/IORandom.h"

class Accessory;

class AccessoryManager : public Singleton< AccessoryManager >
{
public:

	typedef std::map< int, int > HeroLevelInfoMap;
	
	struct AccessoryInfo
	{
		int iGradeMinLevel;
		int iUserPeriod;
		HeroLevelInfoMap m_HeroLevelInfoMap;
		int iMinAbility;
		int iMaxAbility;
		int iGapAbility;
		int iSellPeso;

		AccessoryInfo()
		{
			iGradeMinLevel		= 0;
			iUserPeriod			= 0;
			m_HeroLevelInfoMap.clear();
			iMinAbility			= 0;
			iMaxAbility			= 0;
			iGapAbility			= 0;
			iSellPeso			= 0;
		}
	};

public:
	AccessoryManager();
	virtual ~AccessoryManager();

	void Init();
	void Destroy();

	static AccessoryManager& GetSingleton();

public:
	void LoadINI();

protected:
	void LoadAllAccessory( ioINILoader &rkLoader );

public:
	bool IsExistAccessoryItem(DWORD dwItemCode);
//	__int64 SellAccessory(Accessory* pAccessoryInfo);
//	bool DisassembleAccessory(Accessory* pAccessoryInfo, AccessoryDisassemble& stItemInfo);
//
	bool IsEquipHero( DWORD dwItemCode, int iHeroClassType );
	void CalcLimitDateValue( SYSTEMTIME &sysTime, int& iYMD, int& iHM);
	void ConvertCTimeToSystemTime( SYSTEMTIME &sysTime, CTime& cTime);
	
	int	GetAccessoryValue(DWORD dwItemCode);
	int GetAccessoryPeriod(DWORD dwItemCode);
	int GetAccessorySellPeso(DWORD dwItemCode);
protected:

	typedef std::map< DWORD, AccessoryInfo > AccessoryInfoMap;
	AccessoryInfoMap m_AccessoryInfoMap;

protected:

	bool m_bINILoading;
	IORandom m_AccessoryRandom;
};

#define g_AccessoryMgr AccessoryManager::GetSingleton()