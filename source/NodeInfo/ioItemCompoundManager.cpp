#include "stdafx.h"

//#include "../Window.h"
#include "../MainProcess.h"
#include "../DataBase/LogDBClient.h"

#include "User.h"
#include "ioEtcItem.h"

#include "ioAlchemicMgr.h"

#include "ioItemCompoundManager.h"
#include "MissionManager.h"

#include <strsafe.h>

extern CLog RateCheckLOG;

template<> ioItemCompoundManager* Singleton< ioItemCompoundManager >::ms_Singleton = 0;

ioItemCompoundManager::ioItemCompoundManager()
{
	m_RandomTime.Randomize();

	m_ItemListRandom.SetRandomSeed( timeGetTime()+1 );
	m_PeriodTimeRandom.SetRandomSeed( timeGetTime()+2 );
	m_ReinforceRandom.SetRandomSeed( timeGetTime()+3 );

	m_iMaxReinforceInfo = 0;
	ClearAllInfo();
}

ioItemCompoundManager::~ioItemCompoundManager()
{
	ClearAllInfo();
}

void ioItemCompoundManager::CheckNeedReload()
{
	ioINILoader kLoader;
	kLoader.SetFileName( "config/sp2_item_compound_info.ini" );
	if( kLoader.ReadBool( "common_info", "Change", false ) )
	{
		LoadCompoundInfo();
	}

	if( kLoader.ReadBool( "multiple_compound", "Change", false ) )
	{
		LoadMultipleCompoundInfo();
	}

	if( kLoader.ReadBool( "piece_compound", "Change", false ) )
	{
		LoadMaterialCompoundInfo();
	}
}

void ioItemCompoundManager::LoadCompoundInfo()
{
	ClearCompoundInfoMap();

	ioINILoader kLoader;
	kLoader.ReloadFile( "config/sp2_item_compound_info.ini" );

	kLoader.SetTitle( "common_info" );
	//kLoader.SaveBool( "Change", false );

	// Test
	m_bCheckTestCompound = kLoader.LoadBool( "test_compound", false );

	char szKey[MAX_PATH] = "";

	m_iMaxReinforceInfo = kLoader.LoadInt( "max_info_cnt", 0 );
	if( m_iMaxReinforceInfo > 0 )
	{
		LOOP_GUARD();
		int iTemp = 1;
		while( 1 )
		{
			wsprintf( szKey, "compound_rate%d", iTemp );
			kLoader.SetTitle( szKey );

			DWORD dwCode = (DWORD)kLoader.LoadInt( "compound_item_code", 0 );
			if( dwCode == 0 )
				break;

			iTemp++;

			CompoundInfo kCompoundInfo;
			kCompoundInfo.m_iSmallUpReinforce = kLoader.LoadInt( "small_up_reinforce", 0 );
			kCompoundInfo.m_iBigUpReinforce = kLoader.LoadInt( "big_up_reinforce", 0 );
			kCompoundInfo.m_iDownReinforce = kLoader.LoadInt( "down_reinforce", 0 );

			kCompoundInfo.m_vRateInfoList.reserve( m_iMaxReinforceInfo );
			for( int i=0; i < m_iMaxReinforceInfo; ++i )
			{
				CompoundRateInfo kRateInfo;

				wsprintf( szKey, "compound%d_level", i+1 );
				kRateInfo.m_iLevel = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_same_rate", i+1 );
				kRateInfo.m_iSameItemRate = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_same_special_rate", i+1 );
				kRateInfo.m_iSameItemRateS = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_other_rate", i+1 );
				kRateInfo.m_iOtherItemRate = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_other_special_rate", i+1 );
				kRateInfo.m_iOtherItemRateS = kLoader.LoadInt( szKey, 0 );

				kCompoundInfo.m_vRateInfoList.push_back( kRateInfo );
			}

			m_CompoundInfoMap.insert( CompoundInfoMap::value_type( dwCode, kCompoundInfo ) );
		}
		LOOP_GUARD_CLEAR();
	}

	// Test
	if( m_bCheckTestCompound )
	{
		CheckTestCompound();
	}
}

void ioItemCompoundManager::LoadMultipleCompoundInfo()
{
	ClearRandomInfoMap();
	m_vCurRandomItemList.clear();

	ioINILoader kLoader;
	kLoader.ReloadFile( "config/sp2_item_compound_info.ini" );

	kLoader.SetTitle( "multiple_compound" );
	//kLoader.SaveBool( "Change", false );

	char szTitle[MAX_PATH] = "";
	char szKey1[MAX_PATH] = "";
	char szKey2[MAX_PATH] = "";
	char szKey3[MAX_PATH] = "";

	// Test
	m_bCheckTestMultiCompound = kLoader.LoadBool( "test_multiple_compound", false );

	// Item List
	LOOP_GUARD();
	int iTemp = 1;
	while( 1 )
	{
		wsprintf( szTitle, "multiple_compound_info%d", iTemp );
		kLoader.SetTitle( szTitle );

		DWORD dwCode = (DWORD)kLoader.LoadInt( "multiple_compound_item_code", 0 );
		if( dwCode == 0 )
			break;

		iTemp++;

		m_dwTotalItemRate      = 0;
		m_dwTotalPeriodRate    = 0;
		m_dwTotalReinforceRate = 0;

		RandomInfo kInfo;

		ioRandomMachineGroup::MachineType eType = (ioRandomMachineGroup::MachineType) kLoader.LoadInt( "type", 0 );
		g_ExtraItemInfoMgr.CopyDefaultMachineAll( eType, kInfo, m_dwTotalItemRate, m_dwTotalPeriodRate, m_dwTotalReinforceRate );

		// item
		int iItemCnt = kLoader.LoadInt( "item_cnt", 0 );
		for( int i=0; i < iItemCnt; ++i )
		{
			wsprintf( szKey2, "item%d_code", i+1 );
			int iItemCode = kLoader.LoadInt( szKey2, 0 );

			wsprintf( szKey2, "item%d_rate", i+1 );
			int iItemRate = kLoader.LoadInt( szKey2, 0 );

			RandomItem kItem;
			kItem.m_iItemCode = iItemCode;
			kItem.m_iRandomRate = iItemRate;

			bool bExist = false;
			for(RandomItemList::iterator iter = kInfo.m_vRandomItemList.begin(); iter != kInfo.m_vRandomItemList.end(); ++iter)
			{
				RandomItem &rkItem = (*iter);
				if( rkItem.m_iItemCode != kItem.m_iItemCode )
					continue;
				int iRateAdjust = kItem.m_iRandomRate - rkItem.m_iRandomRate;
				m_dwTotalItemRate += iRateAdjust;
				rkItem.m_iRandomRate = kItem.m_iRandomRate;
				bExist = true;
				break;
			}

			if( !bExist )
			{
				m_dwTotalItemRate += iItemRate;
				kInfo.m_vRandomItemList.push_back( kItem );
			}
		}

		// Period Time
		int iPeriodTimeCnt = kLoader.LoadInt( "period_time_cnt", 0 );
		if( iPeriodTimeCnt > 0 )
		{
			kInfo.m_vPeriodList.clear();
			kInfo.m_vPeriodList.reserve( iPeriodTimeCnt );
			m_dwTotalPeriodRate = 0;
		}

		for( int j=0; j < iPeriodTimeCnt; ++j )
		{
			RandomPeriodTime kPeriodTime;

			wsprintf( szKey1, "period_time%d_rate", j+1 );
			kPeriodTime.m_iRandomRate = kLoader.LoadInt( szKey1, 0 );

			wsprintf( szKey2, "period_time%d_value", j+1 );
			kPeriodTime.m_iPeriodTime = kLoader.LoadInt( szKey2, 0 );

			wsprintf( szKey3, "period_time%d_alarm", j+1 );
			kPeriodTime.m_bAlarm = kLoader.LoadBool( szKey3, false );

			m_dwTotalPeriodRate += kPeriodTime.m_iRandomRate;

			kInfo.m_vPeriodList.push_back( kPeriodTime );
		}

		// Reinforce
		int iReinforceCnt = kLoader.LoadInt( "reinforce_cnt", 0 );
		if( iReinforceCnt > 0 )
		{
			kInfo.m_vReinforceList.clear();
			kInfo.m_vReinforceList.reserve( iReinforceCnt );
			m_dwTotalReinforceRate = 0;
		}

		for( int k=0; k < iReinforceCnt; ++k )
		{
			RandomReinforce kReinforce;

			wsprintf( szKey1, "reinforce%d_rate", k+1 );
			kReinforce.m_iRandomRate = kLoader.LoadInt( szKey1, 0 );

			wsprintf( szKey2, "reinforce%d_value", k+1 );
			kReinforce.m_iReinforce = kLoader.LoadInt( szKey2, 0 );

			m_dwTotalReinforceRate += kReinforce.m_iRandomRate;

			kInfo.m_vReinforceList.push_back( kReinforce );
		}

		m_RandomInfoMap.insert( RandomInfoMap::value_type( dwCode, kInfo ) );

		// Test
		if( m_bCheckTestMultiCompound )
		{
			CheckTestMultiCompound( false, false, dwCode );
			CheckTestMultiCompound( true, false, dwCode );

			CheckTestMultiCompound( false, true, dwCode );
			CheckTestMultiCompound( true, true, dwCode );
		}
	}

	if( m_bCheckTestMultiCompound )
	{
		for(RandomInfoMap::iterator iter = m_RandomInfoMap.begin(); iter != m_RandomInfoMap.end(); ++iter)
		{
			RandomInfo &rkInfo = iter->second;

			int iCnt = 0;
			for(RandomItemList::iterator iter = rkInfo.m_vRandomItemList.begin(); iter != rkInfo.m_vRandomItemList.end(); ++iter)
			{
				iCnt++;
				RandomItem &rkItem = (*iter);
				RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Item%d : %d, %d, %d", iCnt, rkItem.m_iItemCode, rkItem.m_iRandomRate, rkItem.m_iTradeTypeList );
			}
			RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Item TotalRate: %d", m_dwTotalItemRate );

			iCnt = 0;
			for(RandomPeriodTimeList::iterator iter = rkInfo.m_vPeriodList.begin(); iter != rkInfo.m_vPeriodList.end(); ++iter)
			{
				iCnt++;
				RandomPeriodTime &rkPeriodTime = (*iter);
				RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Period%d : %d, %d, %d", iCnt, rkPeriodTime.m_iPeriodTime, rkPeriodTime.m_iRandomRate, (int) rkPeriodTime.m_bAlarm );
			}
			RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Period TotalRate: %d", m_dwTotalPeriodRate );

			iCnt = 0;
			for(RandomReinforceList::iterator iter = rkInfo.m_vReinforceList.begin(); iter != rkInfo.m_vReinforceList.end(); ++iter)
			{
				iCnt++;
				RandomReinforce &rkReinforce = (*iter);
				RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Reinforce%d : %d, %d", iCnt, rkReinforce.m_iReinforce, rkReinforce.m_iRandomRate );
			}
			RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Reinforce TotalRate: %d", m_dwTotalReinforceRate );			
		}
	}

	LOOP_GUARD_CLEAR();
}

void ioItemCompoundManager::LoadMaterialCompoundInfo()
{
	ClearMaterialCompoundInfoMap();

	char szKey[MAX_PATH] = "";
	ioINILoader kLoader;
	kLoader.ReloadFile( "config/sp2_item_material_compound_info.ini" );

	kLoader.SetTitle( "piece_compound" );

	m_iMaterialMaxReinforceInfo = kLoader.LoadInt( "max_reinforce", 0 );
	m_iFixFailReinforce = kLoader.LoadInt( "fixed_fail_reinforce", 0 );

	m_kRightMaterialRate.m_fRightHighMaterialRate = kLoader.LoadFloat("right_high_material", 0.0);
	m_kRightMaterialRate.m_fRightRareMaterialRate = kLoader.LoadFloat("right_rare_material", 0.0);

	m_kWrongMaterialPenalty.m_fDifferPieceRate = kLoader.LoadFloat("penalty_high_diffpiece_rate", 0.0);
	m_kWrongMaterialPenalty.m_fHighItemAdditiveRate = kLoader.LoadFloat("penalty_high_additive_rate", 0.0);
	m_kWrongMaterialPenalty.m_fRareItemDifferRate = kLoader.LoadFloat("penalty_rare_piece_rate", 0.0);

	if( m_iMaterialMaxReinforceInfo > 0 )
	{
		LOOP_GUARD();
		int temp = 1;
		while ( 1 )
		{
			wsprintf( szKey, "piece_compound_rate%d", temp );
			kLoader.SetTitle( szKey );

			DWORD code = (DWORD)kLoader.LoadInt( "compound_item_code", 0 );

			if( code == 0 )
				break;

			temp++;

			MaterialCompoundInfo compoundInfo;
			compoundInfo.m_iRareMaxFailExpRate		= kLoader.LoadFloat( "rare_max_fail_exp_rate", 0.0f );
			compoundInfo.m_fRareNeedMaterialRate	= kLoader.LoadFloat( "rare_need_material_rate", 0.0f );
			compoundInfo.m_iSuccessConstant			= kLoader.LoadInt( "success_constant", 0 );
			compoundInfo.m_fFailExpConstant			= kLoader.LoadFloat( "fail_exp_constant_rate", 0 );
			compoundInfo.m_vMaterialRateInfoList.reserve( m_iMaterialMaxReinforceInfo );


			for( int i=0; i < m_iMaterialMaxReinforceInfo; ++i )
			{
				MaterialCompoundRateInfo rateInfo;

				wsprintf( szKey, "compound%d_level", i+1 );
				rateInfo.m_iLevel = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_need_material", i+1 );
				rateInfo.m_iNeedMaterialCount = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "compound%d_max_fail_exp", i+1 );
				rateInfo.m_iMaxFailExp = kLoader.LoadInt( szKey, 0 );

				wsprintf( szKey, "pc_room_bonus%d", i+1 );
				rateInfo.m_iPcRoomBonus = kLoader.LoadInt( szKey, 0 );

				compoundInfo.m_vMaterialRateInfoList.push_back( rateInfo );
			}
			m_materialCompoundInfoMap.insert( MaterialCompoundInfoMap::value_type( code, compoundInfo ) );
		}
		LOOP_GUARD_CLEAR();
	}
}

void ioItemCompoundManager::ClearAllInfo()
{
	ClearCompoundInfoMap();
	ClearRandomInfoMap();
	ClearMaterialCompoundInfoMap();

	m_vCurRandomItemList.clear();
}

void ioItemCompoundManager::ClearCompoundInfoMap()
{
	CompoundInfoMap::iterator iCreate;
	for(iCreate = m_CompoundInfoMap.begin();iCreate != m_CompoundInfoMap.end();iCreate++)
	{
		CompoundInfo &rkInfo = iCreate->second;
		rkInfo.m_vRateInfoList.clear();
	}
	
	m_CompoundInfoMap.clear();
}

void ioItemCompoundManager::ClearRandomInfoMap()
{
	RandomInfoMap::iterator iter = m_RandomInfoMap.begin();
	for( ; iter != m_RandomInfoMap.end(); ++iter )
	{
		RandomInfo &rkRandomInfo = iter->second;
		rkRandomInfo.Init();
	}

	m_RandomInfoMap.clear();
}

void ioItemCompoundManager::ClearMaterialCompoundInfoMap()
{
	MaterialCompoundInfoMap::iterator iter = m_materialCompoundInfoMap.begin();
	for(	; iter != m_materialCompoundInfoMap.end(); ++iter )
	{
		MaterialCompoundInfo &piceCompound = iter->second;
		piceCompound.m_vMaterialRateInfoList.clear();
	}

	m_materialCompoundInfoMap.clear();
}

ioItemCompoundManager& ioItemCompoundManager::GetSingleton()
{
	return Singleton< ioItemCompoundManager >::GetSingleton();
}

int ioItemCompoundManager::CheckCompoundSuccess( int iTargetSlot, int iVictimSlot, User *pUser, DWORD dwType )
{
	/* return 값은 예외오류 번호. 0번은 오류 없음.*/

	if( !pUser ) return 1;

	ioUserExtraItem::EXTRAITEMSLOT kTargetSlot;
	if( !pUser->GetUserExtraItem()->GetExtraItem( iTargetSlot, kTargetSlot) )
		return 2;

	int iTargetType = kTargetSlot.m_iItemCode / 100000;
	int iTargetReinforce = kTargetSlot.m_iReinforce;

	ioUserExtraItem::EXTRAITEMSLOT kVictimSlot;
	if( !pUser->GetUserExtraItem()->GetExtraItem( iVictimSlot, kVictimSlot) )
		return 3;

	int iVictimType = kVictimSlot.m_iItemCode / 100000;
	if( iTargetType != iVictimType )
		return 4;

	bool bSame = false;
	if( kTargetSlot.m_iItemCode == kVictimSlot.m_iItemCode )
		bSame = true;

	DWORD dwCurRate = 0;
	DWORD dwCurSpecialRate = 0;

	int iSmallUpReinforce = 0;
	int iBigUpReinforce = 0;
	int iDownReinforce = 0;

	CompoundInfoMap::iterator iter = m_CompoundInfoMap.find( dwType );
	if( iter != m_CompoundInfoMap.end() )
	{
		CompoundInfo &rkInfo = iter->second;

		iSmallUpReinforce = rkInfo.m_iSmallUpReinforce;
		iBigUpReinforce = rkInfo.m_iBigUpReinforce;
		iDownReinforce = rkInfo.m_iDownReinforce;

		int iMaxCnt = rkInfo.m_vRateInfoList.size();
		for( int i=0; i < iMaxCnt; ++i )
		{
			if( rkInfo.m_vRateInfoList[i].m_iLevel == iTargetReinforce )
			{
				if( bSame )
				{
					dwCurRate = rkInfo.m_vRateInfoList[i].m_iSameItemRate;
					dwCurSpecialRate = rkInfo.m_vRateInfoList[i].m_iSameItemRateS;
				}
				else
				{
					dwCurRate = rkInfo.m_vRateInfoList[i].m_iOtherItemRate;
					dwCurSpecialRate = rkInfo.m_vRateInfoList[i].m_iOtherItemRateS;
				}
				break;
			}
		}
	}

	if( dwCurRate == 0 && dwCurSpecialRate == 0 )
		return 5;

	if(!pUser->GetUserExtraItem()->DeleteExtraItem( iVictimSlot ))
		return 6;

    bool bSuccess = false;
	int iPreReinforce = kTargetSlot.m_iReinforce;
	DWORD dwRand = m_RandomTime.Random(100);
	kTargetSlot.m_dwFailExp = 0;	// 장비강화도구는 무조건 성공(+1,+2) or 실패(-1)이므로 현재 실패 경험치 초기화.

	if( COMPARE( dwRand, 0, dwCurRate ) )
	{
		kTargetSlot.m_iReinforce += iSmallUpReinforce;
		pUser->GetUserExtraItem()->SetExtraItem( kTargetSlot );	
		bSuccess = true;
	}
	else if( COMPARE( dwRand, dwCurRate, dwCurRate+dwCurSpecialRate ) )
	{
		kTargetSlot.m_iReinforce += iBigUpReinforce;
		pUser->GetUserExtraItem()->SetExtraItem( kTargetSlot );	
		bSuccess = true;
	}
	else
	{
		kTargetSlot.m_iReinforce -= iDownReinforce;
		kTargetSlot.m_iReinforce = max( 0, kTargetSlot.m_iReinforce );

		pUser->GetUserExtraItem()->SetExtraItem( kTargetSlot );
		bSuccess = false;
	}

	RateCheckLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "CheckCompoundSuccess(%s/%d) - ETC: %d, T: %d(%d), V: %d(%d), R: %d(%d), C: %d(%d)", pUser->GetPublicID().c_str(),
																											   pUser->GetUserIndex(),
																											   (int)dwType,
																											   kTargetSlot.m_iItemCode,
																											   iPreReinforce,
																											   kVictimSlot.m_iItemCode,
																											   kVictimSlot.m_iReinforce,
																											   bSuccess,
																											   kTargetSlot.m_iReinforce,
																											   kTargetSlot.m_dwMaleCustom, 
																											   kTargetSlot.m_dwFemaleCustom );

	// Send
	SP2Packet kPacket( STPK_ITEM_COMPOUND );
	PACKET_GUARD_INT( kPacket.Write(ITEM_COMPOUND_OK) );
	PACKET_GUARD_INT( kPacket.Write((int)dwType) );
	PACKET_GUARD_INT( kPacket.Write(bSuccess) );
	PACKET_GUARD_INT( kPacket.Write(iTargetSlot) );
	PACKET_GUARD_INT( kPacket.Write(kTargetSlot.m_iReinforce) );
	PACKET_GUARD_INT( kPacket.Write(iVictimSlot) );
	pUser->SendMessage( kPacket );

	pUser->SaveExtraItem();

	g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_ITEM_REINFORCE, pUser, 0, dwType, bSuccess, kTargetSlot.m_iItemCode, iPreReinforce, kVictimSlot.m_iItemCode, kTargetSlot.m_iReinforce, NULL);

	//미션 체크
	static DWORDVec vValue;
	vValue.clear();
	if( bSuccess )
		vValue.push_back(1);
	else
		vValue.push_back(2);

	g_MissionMgr.DoTrigger(MISSION_CLASS_ITEMREINFORCE, pUser, vValue);
	return 0;
}

bool ioItemCompoundManager::IsRareOrHighItem(const int iItemCode )
{
	int iCode = g_AlchemicMgr.GetDisassembleCode( ADT_EXTRAITEM, iItemCode );

	if( iCode == ADDITIVE_CODE_NUM )
		return true;
	else
		return false;
}

MaterialCompoundInfo* ioItemCompoundManager::GetMaterialCompoundInfo( const DWORD dwEtcItemCode )
{
	MaterialCompoundInfoMap::iterator iter = m_materialCompoundInfoMap.find( dwEtcItemCode );
	if( iter == m_materialCompoundInfoMap.end() )
		return NULL;

	return &(iter->second);
}

float ioItemCompoundManager::GetWrongMaterialRate( const int iMaterialCode, const bool bRare )
{
	if( bRare )
	{
		//레어 아이템에 조각 넣었을 경우
		return m_kWrongMaterialPenalty.m_fRareItemDifferRate;
	}
	
	if( ADDITIVE_CODE_NUM == iMaterialCode )
	{
		//고급장비에 첨가제를 넣었을 경우
		return m_kWrongMaterialPenalty.m_fHighItemAdditiveRate;
	}
	
	//고급장비에 다른 조각을 넣었을 경우
	return m_kWrongMaterialPenalty.m_fDifferPieceRate;
}

float ioItemCompoundManager::GetMaterialConst( const int iItemCode, const int iMaterialCode, const bool bRare )
{
	bool bRightMaterial = IsRightMaterial( iItemCode, iMaterialCode, bRare );

	if( bRightMaterial )
	{
		if( bRare )
			return m_kRightMaterialRate.m_fRightRareMaterialRate;
		else
			return m_kRightMaterialRate.m_fRightHighMaterialRate;
		
	}
	
	return GetWrongMaterialRate( iMaterialCode, bRare );
}

int ioItemCompoundManager::GetSuccessRate( const int iCurExp, const int iCurMaxExp, const int iSuccessConst )
{
	return ceil( ( ( float )iCurExp + iSuccessConst ) / iCurMaxExp * 100);
}

int ioItemCompoundManager::GetGainExp( int iNeedMaterialCount, int fFailExpConst, float fMaterialConst )
{
	return iNeedMaterialCount * fFailExpConst * fMaterialConst;
}

bool ioItemCompoundManager::SetMaterialReinforceConstant( User* pUser, ioUserExtraItem::EXTRAITEMSLOT& extraItemSlot, const int& iMaterialCode, 
															int& iSuccessRate, int& iAddFailExp, int& iMaxFailExp, int& iNeedMaterialCount, bool bRare, const DWORD dwEtcItemCode )
{
	if( !pUser )
		return false;

	MaterialCompoundInfo *compoundInfo = GetMaterialCompoundInfo( dwEtcItemCode );
	if( compoundInfo == NULL )
		return false;
	
	int iPCRoomBonus	= 0;

	for( int i=0; i < m_iMaterialMaxReinforceInfo; i++ )
	{
		if( compoundInfo->m_vMaterialRateInfoList[i].m_iLevel == extraItemSlot.m_iReinforce )
		{
			iNeedMaterialCount = compoundInfo->m_vMaterialRateInfoList[i].m_iNeedMaterialCount;
			iMaxFailExp = compoundInfo->m_vMaterialRateInfoList[i].m_iMaxFailExp;
			if( pUser->IsPCRoomAuthority() )
				iPCRoomBonus	= compoundInfo->m_vMaterialRateInfoList[i].m_iPcRoomBonus;

			break;
		}
	}

	if( bRare )
	{
		iNeedMaterialCount = iNeedMaterialCount * compoundInfo->m_fRareNeedMaterialRate;
		iMaxFailExp = iMaxFailExp * compoundInfo->m_iRareMaxFailExpRate;
	}

	if( iMaxFailExp == 0 ) 
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Material Compound MaxFailExp Zero Error - User ID %s, ItemCode: %d  Reinforce %d", pUser->GetPublicID().c_str(), 
										extraItemSlot.m_iItemCode, extraItemSlot.m_iReinforce );
		return false;
	}

	float fMaterialConst = GetMaterialConst( extraItemSlot.m_iItemCode, iMaterialCode, bRare );
	iSuccessRate = GetSuccessRate( extraItemSlot.m_dwFailExp, iMaxFailExp, compoundInfo->m_iSuccessConstant );
	iAddFailExp = GetGainExp( iNeedMaterialCount, compoundInfo->m_fFailExpConstant, fMaterialConst );

	iSuccessRate = iSuccessRate + iPCRoomBonus;

	iSuccessRate = min( 100, iSuccessRate );

	return true;
}

bool ioItemCompoundManager::CheckMaterialCount( const int iRightCount, const int iUseCount )
{
	if( iRightCount > iUseCount )
		return false;

	return true;
}

bool ioItemCompoundManager::IsRightMaterial( const int iItemCode, const int iMaterialCode, const bool bRare )
{
	if( bRare )
	{
		if( iMaterialCode != ADDITIVE_CODE_NUM )
			return false;
	}
	else
	{
		if( iItemCode % 1000 != iMaterialCode )
			return false;
	}
	return true;
}

bool ioItemCompoundManager::ActReinforce( User *pUser,ioUserExtraItem::EXTRAITEMSLOT& extraItemSlot, const int iReinforceRate, const int iAddFailExp, const int iMaxFailExp )
{ 
	if( !pUser )
		return false;

	DWORD randomValue = m_RandomTime.Random( 100 );
	bool success = false;

	if( COMPARE( (int)randomValue, 0, iReinforceRate ) )
	{
		//최대 강화값 확인.
		if( COMPARE( extraItemSlot.m_iReinforce, 0, m_iMaterialMaxReinforceInfo ) )
		{
			//아이템 실패 경험치 0으로 SET
			extraItemSlot.m_iReinforce++;
			extraItemSlot.m_dwFailExp = 0;
			pUser->GetUserExtraItem()->SetExtraItem( extraItemSlot );
		}
		success = true;
	}
	else
	{
		if( extraItemSlot.m_iReinforce >= ROLLBACK_LEVEL )
		{
			extraItemSlot.m_iReinforce = ROLLBACK_LEVEL;
			extraItemSlot.m_dwFailExp = 0;
			pUser->GetUserExtraItem()->SetExtraItem( extraItemSlot );
		}
		else
		{
			//아이템 실패 경험치 + iAddFailExp
			extraItemSlot.m_dwFailExp += iAddFailExp;

			//최대 경험치 체크.
			extraItemSlot.m_dwFailExp = min( iMaxFailExp, extraItemSlot.m_dwFailExp );

			pUser->GetUserExtraItem()->SetExtraItem( extraItemSlot );
		}
		success = false;
	}

	return success;
}

int ioItemCompoundManager::CheckMaterialCompound( int iTargetSlot, int iMaterialCode, User *pUser, DWORD dwEtcItemCode )
{
	if ( !pUser ) 
		return 1;

	ioUserExtraItem::EXTRAITEMSLOT extraItemSlot;
	ioAlchemicInventory::AlchemicItem materialSlot;
	
	if( !pUser->GetUserExtraItem()->GetExtraItem( iTargetSlot, extraItemSlot ) )
		return 2;

	if( !pUser->GetAlchemicInventory()->FindAlchemicItem( iMaterialCode, materialSlot ) )
		return 3;

	bool bRareItem = IsRareOrHighItem( extraItemSlot.m_iItemCode );
	int iSuccessRate = 0;
	int iAddFailExp = 0; 
	int iMaxFailExp = 0;
	int iNeedMaterialCount = 0;
	int iBeforeReinforce = extraItemSlot.m_iReinforce;

	if( !SetMaterialReinforceConstant( pUser, extraItemSlot, materialSlot.m_iCode, iSuccessRate, iAddFailExp, iMaxFailExp, iNeedMaterialCount, bRareItem, dwEtcItemCode ) )
		return 4;

	if( !CheckMaterialCount( iNeedMaterialCount, materialSlot.m_iCount ) )
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Material Compound Count Error - User ID %s, TargetCode %d Reinforce %d TargetMaterialCode %d Need MaterialCount %d, Curr MaterialCount %d", pUser->GetPublicID().c_str(), 
										extraItemSlot.m_iItemCode, extraItemSlot.m_iReinforce, materialSlot.m_iCode, iNeedMaterialCount, materialSlot.m_iCount );
		return 5;
	}

	//재료 삭제
	if( !pUser->GetAlchemicInventory()->UseAlchemicItem( materialSlot.m_iCode, iNeedMaterialCount ))
		return 6;

	//성공 실패 계산
	bool bSuccess = ActReinforce( pUser, extraItemSlot, iSuccessRate, iAddFailExp, iMaxFailExp );

	//패킷 전송////////////////
	SP2Packet kPacket( STPK_ITEM_MATERIAL_COMPOUND );
	PACKET_GUARD_bool( kPacket.Write( ITEM_COMPOUND_OK ) );
	PACKET_GUARD_bool( kPacket.Write( dwEtcItemCode ) );
	PACKET_GUARD_bool( kPacket.Write( bSuccess ) );
	PACKET_GUARD_bool( kPacket.Write( iTargetSlot ) );
	PACKET_GUARD_bool( kPacket.Write( extraItemSlot.m_iReinforce ) );
	PACKET_GUARD_bool( kPacket.Write( extraItemSlot.m_dwFailExp ) );
	PACKET_GUARD_bool( kPacket.Write( iMaterialCode ) );
	PACKET_GUARD_bool( kPacket.Write( iNeedMaterialCount ) );
	
	pUser->SendMessage( kPacket );
	///////////////////////////////

	pUser->SaveAlchemicInventory();
	pUser->SaveExtraItem();

	if(bSuccess)
	{
		g_LogDBClient.OnInsertMaterialCompound( pUser, extraItemSlot.m_iItemCode, iBeforeReinforce, extraItemSlot.m_iReinforce, materialSlot.m_iCode, LogDBClient::MCR_COMPOUND_SUCCESS );
	}
	else
	{
		g_LogDBClient.OnInsertMaterialCompound( pUser, extraItemSlot.m_iItemCode, iBeforeReinforce, extraItemSlot.m_iReinforce, materialSlot.m_iCode, LogDBClient::MCR_COMPOUND_FAIL );
	}

	g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_ITEM_REINFORCE, pUser, 0, dwEtcItemCode, bSuccess, extraItemSlot.m_iItemCode, iBeforeReinforce, materialSlot.m_iCode, extraItemSlot.m_iReinforce, NULL);

	//미션 체크
	static DWORDVec vValue;
	vValue.clear();
	if( bSuccess )
		vValue.push_back(1);
	else
		vValue.push_back(2);

	g_MissionMgr.DoTrigger(MISSION_CLASS_ITEMREINFORCE, pUser, vValue);

	if( ADDITIVE_CODE_NUM == iMaterialCode)
	{
		pUser->DoAdditiveMission(iNeedMaterialCount, AMT_REINFORCE);
	}


	return 0;
}

bool ioItemCompoundManager::CheckMultipleCompound( int iItem1, int iItem2, int iItem3, User *pUser, DWORD dwType )
{
	bool bEqualItem = false;
	if( dwType == ioEtcItem::EIT_ETC_MULTIPLE_EQUAL_COMPOUND ||
		dwType == ioEtcItem::EIT_ETC_MULTIPLE_EQUAL_COMPOUND2 ||
		dwType == ioEtcItem::EIT_ETC_MULTIPLE_EQUAL_COMPOUND3 )
	{
		bEqualItem = true;
	}

	if( !pUser )
	{
		//SP2Packet kPacket( STPK_ITEM_COMPOUND );
		//kPacket << ITEM_MULTIPLE_COMPOUND_EXCEPTION;
		//kPacket << 1;
		//kPacket << 0;	// item1
		//kPacket << 0;	// item2
		//kPacket << 0;	// item3
		//pUser->SendMessage( kPacket );  pUser가 NULL 인대 sendmessage...
		return false;
	}

	ioUserExtraItem::EXTRAITEMSLOT kItem1;
	if( !pUser->GetUserExtraItem()->GetExtraItem( iItem1, kItem1) )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_ITEM_ERROR;
		kPacket << 1;
		pUser->SendMessage( kPacket );
		return false;
	}

	ioUserExtraItem::EXTRAITEMSLOT kItem2;
	if( !pUser->GetUserExtraItem()->GetExtraItem( iItem2, kItem2) )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_ITEM_ERROR;
		kPacket << 2;
		pUser->SendMessage( kPacket );
		return false;
	}

	ioUserExtraItem::EXTRAITEMSLOT kItem3;
	if( !pUser->GetUserExtraItem()->GetExtraItem( iItem3, kItem3) )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_ITEM_ERROR;
		kPacket << 3;
		pUser->SendMessage( kPacket );
		return false;
	}

	RandomInfoMap::iterator iter = m_RandomInfoMap.find( dwType );
	if( iter == m_RandomInfoMap.end() )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_ITEM_ERROR;
		kPacket << 4;
		pUser->SendMessage( kPacket );
		return false;
	}

	int iItem1Type = kItem1.m_iItemCode / 100000;
	int iItem2Type = kItem2.m_iItemCode / 100000;
	int iItem3Type = kItem3.m_iItemCode / 100000;

	bool bCurEqual = false;
	if( iItem1Type == iItem2Type && iItem2Type == iItem3Type )
		bCurEqual = true;

	if( bEqualItem && !bCurEqual )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_EQUAL_ERROR;
		pUser->SendMessage( kPacket );
		return false;
	}

	// Check CurItemList
	int i = 0;
	m_vCurRandomItemList.clear();
	m_dwTotalItemRate = 0;

	RandomInfo& rkInfo = iter->second;
	RandomItemList vTestList = rkInfo.m_vRandomItemList;

	int iListSize = vTestList.size();
	for( i=0; i < iListSize; ++i )
	{
		RandomItem kItem = vTestList[i];

		int iItemCode = kItem.m_iItemCode;
		int iItemType = kItem.m_iItemCode / 100000;

		if( iItemCode == kItem1.m_iItemCode ) continue;
		if( iItemCode == kItem2.m_iItemCode ) continue;
		if( iItemCode == kItem3.m_iItemCode ) continue;

		// 하나만 체크해도 된다. 모두 같으니까
		if( bEqualItem && iItemType != iItem1Type ) continue;

		m_dwTotalItemRate += kItem.m_iRandomRate;

		m_vCurRandomItemList.push_back( kItem );
	}


	// 아이템 제거를 먼저...
	bool bDeleteFail1 = false;
	bool bDeleteFail2 = false;
	bool bDeleteFail3 = false;

	if(!pUser->GetUserExtraItem()->DeleteExtraItem(iItem1))
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound Del Error - ID: %s(%d), ItemCode: %d ", pUser->GetPublicID().c_str(), pUser->GetUserIndex(), kItem1.m_iItemCode );
		bDeleteFail1 = true;
	}

	if(!pUser->GetUserExtraItem()->DeleteExtraItem(iItem2))
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound Del Error - ID: %s(%d), ItemCode: %d ", pUser->GetPublicID().c_str(), pUser->GetUserIndex(), kItem2.m_iItemCode );
		bDeleteFail2 = true;
	}

	if(!pUser->GetUserExtraItem()->DeleteExtraItem(iItem3))
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound Del Error - ID: %s(%d), ItemCode: %d ", pUser->GetPublicID().c_str(), pUser->GetUserIndex(), kItem3.m_iItemCode );
		bDeleteFail3 = true;
	}

	if( bDeleteFail1 || bDeleteFail2 || bDeleteFail3 )
	{
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_DEL_ERROR;
		kPacket << iItem1;
		kPacket << iItem2;
		kPacket << iItem3;
		kPacket << bDeleteFail1;
		kPacket << bDeleteFail2;
		kPacket << bDeleteFail3;
		pUser->SendMessage( kPacket );
		return false;
	}

	// GetItem, GetPeriod, GetReinforce
	int iItemCode = GetRandomItem( dwType );
	int iPeriodTime = GetRandomPeriod( dwType );
	int iReinforce = GetRandomReinforce( dwType );

	ioUserExtraItem::EXTRAITEMSLOT kExtraItem;
	kExtraItem.m_iItemCode = iItemCode;
	kExtraItem.m_iReinforce = iReinforce;
	kExtraItem.m_PeriodType = ioUserExtraItem::EPT_TIME;

	if( iPeriodTime >= 0 )
	{
		CTime kLimiteTime = CTime::GetCurrentTime();
		CTimeSpan kAddTime( 0, iPeriodTime, 0, 0 );
		kLimiteTime += kAddTime;

		kExtraItem.SetDate( kLimiteTime.GetYear(), kLimiteTime.GetMonth(), kLimiteTime.GetDay(), kLimiteTime.GetHour(), kLimiteTime.GetMinute() );

		if( iPeriodTime == 0 ) // 무제한
			kExtraItem.m_PeriodType = ioUserExtraItem::EPT_MORTMAIN;
	}
	else
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound Create Error - ID: %s(%d), ItemCode: %d ", pUser->GetPublicID().c_str(), pUser->GetUserIndex(), kExtraItem.m_iItemCode );
		
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_EXCEPTION;
		kPacket << 2;
		kPacket << iItem1;
		kPacket << iItem2;
		kPacket << iItem3;
		pUser->SendMessage( kPacket );
		return false;
	}

	DWORD dwIndex = 0;
	int iArrayIndex = 0;
	int iSlotIndex = pUser->GetUserExtraItem()->AddExtraItem( kExtraItem, false, 0, LogDBClient::ERT_COMPOUND, 0, iPeriodTime, dwIndex, iArrayIndex );
	if( iSlotIndex > 0 )
	{
		char szItemIndex[MAX_PATH]="";
		if( dwIndex != 0 )
		{
			StringCbPrintf( szItemIndex, sizeof( szItemIndex ), "%u-%u", dwIndex, iArrayIndex+1 ); // db field는 1부터 이므로 +1
			g_LogDBClient.OnInsertExtraItem( pUser, kExtraItem.m_iItemCode, kExtraItem.m_iReinforce, 0, iPeriodTime, 0, kExtraItem.m_PeriodType, kExtraItem.m_dwMaleCustom, kExtraItem.m_dwFemaleCustom, szItemIndex, LogDBClient::ERT_COMPOUND );
		}
	}
	else
	{
		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound Add Error - ID: %s(%d), ItemCode: %d ", pUser->GetPublicID().c_str(), pUser->GetUserIndex(), kExtraItem.m_iItemCode );
		
		SP2Packet kPacket( STPK_ITEM_COMPOUND );
		kPacket << ITEM_MULTIPLE_COMPOUND_EXCEPTION;
		kPacket << 3;
		kPacket << iItem1;
		kPacket << iItem2;
		kPacket << iItem3;
		pUser->SendMessage( kPacket );
		return false;
	}


	// Send
	SP2Packet kPacket( STPK_ITEM_COMPOUND );
	kPacket << ITEM_MULTIPLE_COMPOUND_OK;
	kPacket << (int)dwType;
	kPacket << iItem1;
	kPacket << iItem2;
	kPacket << iItem3;
	kPacket << kExtraItem.m_iItemCode;
	kPacket << kExtraItem.m_iReinforce;
	kPacket << iSlotIndex;
	kPacket << kExtraItem.m_PeriodType;
	kPacket << kExtraItem.m_iValue1;
	kPacket << kExtraItem.m_iValue2;
	kPacket << iPeriodTime;
	pUser->SendMessage( kPacket );

	pUser->SaveExtraItem();

	RateCheckLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MultipleCompound Success(%d) - ID: %s(%d), Item: %d, %d, %d, NewItem: %d, %d, %d ", bEqualItem, pUser->GetPublicID().c_str(),
																												 pUser->GetUserIndex(),
																												 kItem1.m_iItemCode,
																												 kItem2.m_iItemCode,
																												 kItem3.m_iItemCode,
																												 kExtraItem.m_iItemCode,
																												 kExtraItem.m_iReinforce,
																												 kExtraItem.m_iTradeState );
	
	return true;
}

void ioItemCompoundManager::CheckTestMultiCompound( bool bEqual, bool bRateTest, DWORD dwCode )
{
	RandomInfoMap::iterator iter = m_RandomInfoMap.find( dwCode );
	if( iter == m_RandomInfoMap.end() )
		return;

	RandomInfo& rkInfo = iter->second;
	RandomItemList vTestList = rkInfo.m_vRandomItemList;
	if( !vTestList.empty() )
	{
		std::random_shuffle( vTestList.begin(), vTestList.end() );
	}

	IORandom kTestRandom;
	kTestRandom.Randomize();

	typedef std::map< int, int > InfoMap;
	InfoMap eItemMap;

	int i = 0;
	int iTotalSize = vTestList.size();

	int iLoopCnt = 0;
	if( bRateTest )
	{
		iLoopCnt = 1000000;

		RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MultipleCompound ItemRateTest Start(%d:%d)", dwCode, bEqual );
	}
	else
	{
		iLoopCnt = 100;

		RateCheckLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MultipleCompound ItemCodeTest Start(%d:%d)", dwCode, bEqual );
	}
	
	for( i=0; i < iLoopCnt; ++i )
	{
		// iItem1, 2, 3 선택
		int iItemCode1, iItemCode2, iItemCode3;
		int iItemType1, iItemType2, iItemType3;

		iItemCode1 = iItemCode2 = iItemCode3 = 0;
		iItemType1 = iItemType2 = iItemType3 = -1;

		if( !bEqual )
		{
			int iRand = kTestRandom.Random(iTotalSize);
			iItemCode1 = vTestList[iRand].m_iItemCode;
			iItemType1 = iItemCode1 / 100000;

			iRand = kTestRandom.Random(iTotalSize);
			iItemCode2 = vTestList[iRand].m_iItemCode;
			iItemType2 = iItemCode1 / 100000;

			iRand = kTestRandom.Random(iTotalSize);
			iItemCode3 = vTestList[iRand].m_iItemCode;
			iItemType3 = iItemCode1 / 100000;
		}
		else
		{
			int iRand = kTestRandom.Random(iTotalSize);
			iItemCode1 = vTestList[iRand].m_iItemCode;
			iItemType1 = iItemCode1 / 100000;

			for( int j=0; j < iTotalSize; ++j )
			{
				int iCode = vTestList[j].m_iItemCode;
				int iType = iCode / 100000;
				
				if( iItemCode2 == 0 && iType == iItemType1 )
				{
					iItemCode2 = iCode;
					iItemType2 = iType;
					continue;
				}

				if( iItemCode3 == 0 && iType == iItemType1 )
				{
					iItemCode3 = iCode;
					iItemType3 = iType;
					continue;
				}

				if( iItemCode2 != 0 && iItemCode3 != 0 )
					break;
			}
		}

		// Check CurItemList
		m_vCurRandomItemList.clear();
		m_dwTotalItemRate = 0;

		int iListSize = vTestList.size();
		for( int k=0; k < iListSize; ++k )
		{
			RandomItem kItem = vTestList[k];

			int iItemCode = kItem.m_iItemCode;
			int iItemType = kItem.m_iItemCode / 100000;

			if( iItemCode == iItemCode1 ) continue;
			if( iItemCode == iItemCode2 ) continue;
			if( iItemCode == iItemCode3 ) continue;

			// 하나만 체크해도 된다. 모두 같으니까
			if( bEqual && iItemType != iItemType1 ) continue;

			m_dwTotalItemRate += kItem.m_iRandomRate;

			m_vCurRandomItemList.push_back( kItem );
		}

		// GetItem, GetPeriod, GetReinforce
		int iNewItemCode = GetRandomItem( dwCode );
		int iNewPeriodTime = GetRandomPeriod( dwCode );
		int iNewReinforce = GetRandomReinforce( dwCode );

		if( !bRateTest )
		{
			RateCheckLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MultipleCompound Test(%d:%d) - Item: %d, %d, %d, New: %d, %d, %d", dwCode, bEqual,
																											  iItemCode1, iItemCode2, iItemCode3,
																											  iNewItemCode, iNewPeriodTime, iNewReinforce );
		}
		else
		{
			InfoMap::iterator iter_i = eItemMap.find( iNewItemCode );
			if( iter_i != eItemMap.end() )
				iter_i->second += 1;
			else
				eItemMap.insert( InfoMap::value_type(iNewItemCode, 1) );
		}
	}

	if( bRateTest )
	{
		InfoMap::iterator iter_i = eItemMap.begin();
		for( ; iter_i != eItemMap.end(); ++iter_i )
		{
			int iItemCode = iter_i->first;
			float fRate = (float) iter_i->second / 1000000 * 100.0f;
			RateCheckLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MultipleCompound RateTest(%d:%d) - %d : %f", dwCode, bEqual, iItemCode, fRate );
		}
	}
}

int ioItemCompoundManager::GetRandomItem( DWORD dwCode )
{
	int iRand = m_ItemListRandom.Random( m_dwTotalItemRate );

	int iItemIndex = 0;
	int iCurPosition = 0;

	int iItemCnt = m_vCurRandomItemList.size();
	for( int i=0; i < iItemCnt; ++i )
	{
		int iCurRate = m_vCurRandomItemList[i].m_iRandomRate;
		if( COMPARE( iRand, iCurPosition, iCurPosition+iCurRate ) )
		{
			return m_vCurRandomItemList[i].m_iItemCode;
		}

		iCurPosition += iCurRate;
	}

	return 0;
}

int ioItemCompoundManager::GetRandomPeriod( DWORD dwCode )
{
	RandomInfoMap::iterator iter = m_RandomInfoMap.find( dwCode );
	if( iter == m_RandomInfoMap.end() )
		return -1;

	RandomInfo& rkInfo = iter->second;

	int iRand = m_PeriodTimeRandom.Random( m_dwTotalPeriodRate );

	int iCurPosition = 0;
	int iGroupCnt = rkInfo.m_vPeriodList.size();
	for( int i=0; i < iGroupCnt; ++i )
	{
		int iCurRate = rkInfo.m_vPeriodList[i].m_iRandomRate;
		if( COMPARE( iRand, iCurPosition, iCurPosition+iCurRate ) )
		{
			return rkInfo.m_vPeriodList[i].m_iPeriodTime;
		}

		iCurPosition += iCurRate;
	}

	return -1;
}

int ioItemCompoundManager::GetRandomReinforce( DWORD dwCode )
{
	RandomInfoMap::iterator iter = m_RandomInfoMap.find( dwCode );
	if( iter == m_RandomInfoMap.end() )
		return -1;

	RandomInfo& rkInfo = iter->second;

	m_dwTotalReinforceRate = 0;
	int iGroupCnt = rkInfo.m_vReinforceList.size();
	for( int k=0; k < iGroupCnt; ++k )
	{
		m_dwTotalReinforceRate += rkInfo.m_vReinforceList[k].m_iRandomRate;
	}

	int iRand = m_ReinforceRandom.Random( m_dwTotalReinforceRate );
	int iCurPosition = 0;
	for( int i=0; i < iGroupCnt; ++i )
	{
		int iCurRate = rkInfo.m_vReinforceList[i].m_iRandomRate;
		if( COMPARE( iRand, iCurPosition, iCurPosition+iCurRate ) )
		{
			return rkInfo.m_vReinforceList[i].m_iReinforce;
		}

		iCurPosition += iCurRate;
	}

	return 0;
}

void ioItemCompoundManager::CheckTestCompound()
{
	RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Compound Test Start" );

	typedef std::map< int, int > InfoMap;
	InfoMap eItemMap;

	for( int k=0; k < 25; ++k )
	{
		int iTargetReinforce = k;

		int iLoopCnt = 1000000;
		DWORD dwItemCode = 0;
		
		CompoundInfoMap::iterator iCreate;
		for(iCreate = m_CompoundInfoMap.begin();iCreate != m_CompoundInfoMap.end();iCreate++)
		{
			eItemMap.clear();

			dwItemCode = iCreate->first;
			CompoundInfo &rkInfo = iCreate->second;

			for( int i=0; i < iLoopCnt; ++i )
			{
				DWORD dwCurRate = 0;
				DWORD dwCurSpecialRate = 0;

				int iMaxCnt = rkInfo.m_vRateInfoList.size();
				for( int i=0; i < iMaxCnt; ++i )
				{
					if( rkInfo.m_vRateInfoList[i].m_iLevel == iTargetReinforce )
					{
						dwCurRate = rkInfo.m_vRateInfoList[i].m_iOtherItemRate;
						dwCurSpecialRate = rkInfo.m_vRateInfoList[i].m_iOtherItemRateS;
						break;
					}
				}

				if( dwCurRate == 0 && dwCurSpecialRate == 0 )
					continue;

				DWORD dwRand = m_RandomTime.Random(100);

				int iSuccessType = 1;
				if( COMPARE( dwRand, 0, dwCurRate ) )
					iSuccessType = 2;
				else if( COMPARE( dwRand, dwCurRate, dwCurRate+dwCurSpecialRate ) )
					iSuccessType = 3;


				InfoMap::iterator iter_i = eItemMap.find( iSuccessType );
				if( iter_i != eItemMap.end() )
					iter_i->second += 1;
				else
					eItemMap.insert( InfoMap::value_type(iSuccessType, 1) );
			}

			InfoMap::iterator iter_i = eItemMap.begin();
			for( ; iter_i != eItemMap.end(); ++iter_i )
			{
				int iSuccessType = iter_i->first;
				float fRate = (float) iter_i->second / 1000000 * 100.0f;
				RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Compound RateTest(%d:%d) - %d : %f", dwItemCode,
																					   iTargetReinforce,
																					   iSuccessType,
																					   fRate );
			}
		}
	}

	RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Compound Test End" );
}