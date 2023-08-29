#include "stdafx.h"
#include "AccessoryManager.h"
#include "Accessory.h"
#include "../EtcHelpFunc.h"

template<> AccessoryManager *Singleton< AccessoryManager >::ms_Singleton = 0;

AccessoryManager::AccessoryManager()
{
	m_bINILoading = false;
	Init();
}

AccessoryManager::~AccessoryManager()
{
	Destroy();
}

void AccessoryManager::Init()
{
	m_AccessoryInfoMap.clear();
}

void AccessoryManager::Destroy()
{
	m_AccessoryInfoMap.clear();
}

AccessoryManager& AccessoryManager::GetSingleton()
{
	return Singleton< AccessoryManager >::GetSingleton();
}

void AccessoryManager::LoadINI()
{
	m_bINILoading = true;
	Init();
	char szKey[MAX_PATH]="";

	ioINILoader kLoader;
	kLoader.ReloadFile( "config/sp2_accessory.ini" );
	LoadAllAccessory(kLoader);
	m_bINILoading = false;
}

void AccessoryManager::LoadAllAccessory( ioINILoader &rkLoader )
{
	Init();
	char szKey[MAX_PATH]="";

	rkLoader.SetTitle( "common" );
	int iAccessoryCount				= rkLoader.LoadInt("accessory_cnt", 0);

	for( int i = 0; i < iAccessoryCount; i++ )
	{
		AccessoryInfo stAccessoryInfo;
		StringCbPrintf( szKey, sizeof( szKey ), "accessory%d", i+1 );
		rkLoader.SetTitle(szKey);

		DWORD dwItemCode = rkLoader.LoadInt("item_code", 0);
		stAccessoryInfo.iGradeMinLevel = rkLoader.LoadInt("grade_min_level", 0);
		stAccessoryInfo.iUserPeriod = rkLoader.LoadInt("use_period", 0);
		int iHeroCount = rkLoader.LoadInt("hero_count", 0);

		for( int i = 0; i < iHeroCount; i++ )
		{
			wsprintf( szKey, "hero_class_type%d", i+1 );
			int iClassType = rkLoader.LoadInt( szKey, 0 );

			wsprintf( szKey, "hero_min_level%d", i+1 );
			int iHeroMinLevel = (__int64)rkLoader.LoadInt( szKey, 0 );

			stAccessoryInfo.m_HeroLevelInfoMap.insert( HeroLevelInfoMap::value_type( iClassType, iHeroMinLevel ) );
		}

		stAccessoryInfo.iMinAbility = rkLoader.LoadInt("ability_min", 0);
		stAccessoryInfo.iMaxAbility = rkLoader.LoadInt("ability_max", 0);
		stAccessoryInfo.iGapAbility = rkLoader.LoadInt("ability_gap", 0);

		stAccessoryInfo.iSellPeso = rkLoader.LoadInt("sell_peso", 0);

		m_AccessoryInfoMap.insert(AccessoryInfoMap::value_type(dwItemCode, stAccessoryInfo));
	}

	m_AccessoryRandom.Randomize();
}

bool AccessoryManager::IsExistAccessoryItem(DWORD dwItemCode)
{
	if ( m_bINILoading == true )
		return false;

	AccessoryInfoMap::iterator iter = m_AccessoryInfoMap.find(dwItemCode);
	if( iter == m_AccessoryInfoMap.end() )
		return false;

	return true;
}

//__int64 AccessoryManager::SellCostume(Costume* pCostumeInfo)
//{
//	if ( m_bINILoading == true )
//		return -1;
//
//	if( !pCostumeInfo )
//		return -1;
//	
//	//float fReturnPeso = 0.0f;
//	int iPeriodType = pCostumeInfo->GetPeriodType();
//	/*
//	if( PCPT_TIME == iPeriodType )
//	{
//		CTime kCurTime = CTime::GetCurrentTime();
//		CTime kLimitTime( Help::GetSafeValueForCTimeConstructor(pCostumeInfo->GetYear(),
//																pCostumeInfo->GetMonth(),
//																pCostumeInfo->GetDay(),
//																pCostumeInfo->GetHour(),
//																pCostumeInfo->GetMinute(),
//																0) );
//		CTimeSpan kRemainTime = kLimitTime - kCurTime;
//		DWORD dwTotalTime = 0;
//		if( kRemainTime.GetTotalMinutes() > 0 )
//			dwTotalTime = kRemainTime.GetTotalMinutes();
//
//		fReturnPeso = dwTotalTime * m_stTimeSellInfo.fPrice * ( 1 + m_stTimeSellInfo.fRate );
//		fReturnPeso = max(0, fReturnPeso);
//		fReturnPeso = min(fReturnPeso, m_stTimeSellInfo.iMaxPrice);
//	}
//	else if( PCPT_MORTMAIN == iPeriodType )
//	{
//		fReturnPeso = m_stMortmainSellInfo.fPrice * ( 1 + m_stMortmainSellInfo.fRate );
//		fReturnPeso = max(0, fReturnPeso);
//		fReturnPeso = min(fReturnPeso, m_stMortmainSellInfo.iMaxPrice);
//	}
//	else
//		return -1;
//	*/
//
//	if( PCPT_TIME == iPeriodType )
//	{
//		return (__int64)m_iTimeItemPrice;
//	}
//	else if( PCPT_MORTMAIN == iPeriodType )
//	{
//		return (__int64)m_iMortmainItemPrice;
//	}
//
//	return 0;
//}
//
//bool AccessoryManager::DisassembleCostume(Costume* pCostumeInfo, CostumeDisassemble& stItemInfo )
//{
//	if ( m_bINILoading == true )
//		return false;
//
//	if( !pCostumeInfo )
//		return false;
//
//	GetDisassembleItemInfo(stItemInfo);
//	if( stItemInfo.iGainCode == 0 )
//		return false;
//
//	return true;
//}
//
//void AccessoryManager::GetDisassembleItemInfo( CostumeDisassemble& stItemInfo )
//{
//	int iRandom = m_DisassembleRandom.Random(m_iTotalDisassembleRandomValue);
//	int iCurPosition = 0;
//
//	for( int i = 0; i < (int)m_vDisassembleInfo.size(); i++ )
//	{
//		int iCurRate = m_vDisassembleInfo[i].iRandom;
//		if( COMPARE(iRandom, iCurPosition, iCurPosition+iCurRate) )
//		{
//			stItemInfo = m_vDisassembleInfo[i];
//			break;
//		}
//
//		iCurPosition += iCurRate;
//	}
//}
//
void AccessoryManager::CalcLimitDateValue( SYSTEMTIME &sysTime, int& iYMD, int& iHM)
{
	iYMD = ( sysTime.wYear * 10000 ) + ( sysTime.wMonth * 100 ) + sysTime.wDay;
	iHM = ( sysTime.wHour * 100 ) + sysTime.wMinute;
}

void AccessoryManager::ConvertCTimeToSystemTime( SYSTEMTIME &sysTime, CTime& cTime)
{
	sysTime.wYear = cTime.GetYear();
	sysTime.wMonth = cTime.GetMonth();
	sysTime.wDay = cTime.GetDay();
	sysTime.wHour = cTime.GetHour();
	sysTime.wMinute = cTime.GetMinute();
}

int	AccessoryManager::GetAccessoryValue(DWORD dwItemCode)
{
	AccessoryInfoMap::iterator iter = m_AccessoryInfoMap.find(dwItemCode);
	if( iter == m_AccessoryInfoMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] accessory null : [code:%d]", dwItemCode );
		return 0;
	}

	AccessoryInfo info = iter->second;
	if(info.iGapAbility == 0)
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] accessory value gap = 0: [code:%d]", dwItemCode );
		return 0;
	}

	int iRand = m_AccessoryRandom.Random(info.iMinAbility, info.iMaxAbility);
	int var = iRand % info.iGapAbility;
	iRand = iRand - var;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] accessory value : [code:%d value:%d]", dwItemCode, iRand );

	return iRand;
}

int AccessoryManager::GetAccessoryPeriod(DWORD dwItemCode)
{
	AccessoryInfoMap::iterator iter = m_AccessoryInfoMap.find(dwItemCode);
	if( iter == m_AccessoryInfoMap.end() )
		return 0;

	AccessoryInfo info = iter->second;
	return info.iUserPeriod;
}

bool AccessoryManager::IsEquipHero( DWORD dwItemCode, int iHeroClassType )
{
	AccessoryInfoMap::iterator iter = m_AccessoryInfoMap.find(dwItemCode);
	if( iter == m_AccessoryInfoMap.end() )
		return false;

	AccessoryInfo info = iter->second;
	HeroLevelInfoMap::iterator iterhero = info.m_HeroLevelInfoMap.find(iHeroClassType);
	if( iterhero == info.m_HeroLevelInfoMap.end() )
		return false;

	return true;
}

int AccessoryManager::GetAccessorySellPeso(DWORD dwItemCode)
{
	AccessoryInfoMap::iterator iter = m_AccessoryInfoMap.find(dwItemCode);
	if( iter == m_AccessoryInfoMap.end() )
		return 0;

	AccessoryInfo info = iter->second;
	return info.iSellPeso;
}