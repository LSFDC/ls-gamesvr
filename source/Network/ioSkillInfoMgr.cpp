#include "StdAfx.h"
#include "ioSkillInfoMgr.h"

ioSkillInfoMgr::ioSkillInfoMgr(void)
{
#ifdef ANTIHACK
	Init();
#endif
}
ioSkillInfoMgr::~ioSkillInfoMgr(void)
{
}
#ifdef ANTIHACK

void ioSkillInfoMgr::Init()
{
	ioINILoader iLoader("config//ls_skill.ini");
	//디폴트로 바뀔일 없는 애들은 그냥 고정값으로 씀
	iLoader.SetTitle( "default" );
	m_fDefaultRecoveryGauge = 0.1f;
	m_fDefaultRecoveryGaugeTic = 100.f;

	int max_value_info_cnt = iLoader.LoadInt( "max_value_info_cnt", 0 );
	int skillCnt = iLoader.LoadInt( "skill_cnt", 0 );
	m_fErrorRate = iLoader.LoadFloat( "error_rate", 0.2f );

	for( int i = 0; i < max_value_info_cnt; ++i )
	{
		char szKey[MAX_PATH] = {0,};
		wsprintf( szKey, "max_value_info%d", i+1 );
		iLoader.SetTitle( szKey );

		GrowthSkillInfo GSInfo;

		GSInfo.iNumber = iLoader.LoadInt( "maxvalueid", -1 );
		GSInfo.iType = iLoader.LoadInt( "maxvalueType", -1 );
		GSInfo.fMaxValueSection1 = iLoader.LoadFloat( "max_value_section1", 135.f );
		GSInfo.fMaxValueSection2 = iLoader.LoadFloat( "max_value_section2", 110.f );
		GSInfo.fMaxValueSection3 = iLoader.LoadFloat( "max_value_section3", 90.f );
		GSInfo.fMaxValueSectionGap1 = iLoader.LoadFloat( "max_value_section1_gap", 100.f );
		GSInfo.fMaxValueSectionGap2 = iLoader.LoadFloat( "max_value_section2_gap", 200.f );

		m_vecGrowhUPInfo.push_back( GSInfo );
	}
	
	
	for( int i = 0; i < skillCnt; ++i )
	{
		char szKey[MAX_PATH] = {0,};
		char szBuf[MAX_PATH] = "";
		wsprintf( szKey, "skill_%d", i+1 );
		iLoader.SetTitle( szKey );

		SkillInfo SInfo;
		SInfo.iSkillID = iLoader.LoadInt( "skillid", -1 );
		iLoader.LoadString( "skillname", "", szBuf, MAX_PATH );
		SInfo.hsSkillName = szBuf;
		SInfo.iSkillType = iLoader.LoadInt( "skilltype", -1 );
		SInfo.iSkillSubType = iLoader.LoadInt( "sub_type", -1 );
		SInfo.iEquipType = iLoader.LoadInt( "equiptype", -1 );
		SInfo.iBuffType = iLoader.LoadInt( "bufftype", -1 );
		SInfo.iMaxValueType = iLoader.LoadInt( "max_value_type", -1 );
		SInfo.fSkillGaugeRate = iLoader.LoadFloat( "gaugerate", 1.f );

		if( SInfo.iSkillType == 7 )
		{
			SInfo.iNeedCount = iLoader.LoadInt( "needgauge", -1 );
			SInfo.iMaxCount = iLoader.LoadFloat( "maxgauge", -1 );
			SInfo.iTickTime = iLoader.LoadInt( "ticktime", -1 );
		}
		else
		{
			SInfo.fNeedGauge = iLoader.LoadFloat( "needgauge", -1.f );
			SInfo.fMaxGauge = iLoader.LoadFloat( "maxgauge", -1.f );
		}

		/*//buff
		SInfo.fGaugePerTick = iLoader.LoadFloat( "gauge_per_tick", 0.f );
		SInfo.iTickTime = iLoader.LoadInt( "tick_time", 0 );
		SInfo.iFCTickTime = iLoader.LoadInt( "FC_tic_time", 0 );*/

		SInfo.fGaugePerTick = SInfo.iFCTickTime = 0;

		m_mapSkillInfo.insert( SkillInfoMap::value_type(SInfo.hsSkillName,SInfo));
	}
	//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - skillCount1(%d)", skillCnt );

	// 수작업
	ioINILoader iLoader2("config//ls_skill_extra.ini");
	iLoader2.SetTitle( "default" );	
	skillCnt = iLoader2.LoadInt( "skill_cnt", 0 );

	//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - skillCount2(%d)", skillCnt );
	{
		for( int i = 0; i < skillCnt; ++i )
		{
			char szKey[MAX_PATH] = {0,};
			char szBuf[MAX_PATH] = "";
			wsprintf( szKey, "skill_%d", i+1 );
			iLoader2.SetTitle( szKey );
			iLoader2.LoadString( "skillname", "", szBuf, MAX_PATH );
			
			int iExtraType = iLoader2.LoadInt( "extratype", 1 );
			ioHashString hsSkill = szBuf;
			SkillInfo* pSkill = GetSkillInfo( hsSkill );
			if( pSkill )
			{
				if( iExtraType == 1 )
					pSkill->bRecoverGauge = true;
				else if( iExtraType == 2 )
				{
					pSkill->dwExtraValue = iLoader2.LoadInt( "extravalue", 0 );
					pSkill->bReloadBullet = true;
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//총알
	ioINILoader iLoader3("config//ls_bullet_info.ini");
	iLoader3.SetTitle( "default" );	
	skillCnt = iLoader3.LoadInt( "bullet_cnt", 0 );
	for( int i = 0; i < skillCnt; ++i )
	{
		char szKey[MAX_PATH] = {0,};
		char szBuf[MAX_PATH] = "";
		wsprintf( szKey, "bullet_%d", i+1 );
		iLoader3.SetTitle( szKey );

		sWeaponInfo sWeapon;
		DWORD dwItemCode = iLoader3.LoadInt( "itemcode", 0 );
		sWeapon.dwMaxCount = iLoader3.LoadInt( "maxcount", 0 );
		sWeapon.dwIntervalBullet = iLoader3.LoadInt( "intervalbullet", 0 );
		sWeapon.dwIntervalMotion = iLoader3.LoadInt( "intervalmotion", 0 );
		sWeapon.dwReloadTime = iLoader3.LoadInt( "reloadtime", 0 );
		m_mapWeaponInfo.insert( std::map<DWORD,sWeaponInfo>::value_type(dwItemCode,sWeapon) );
	}
}

SkillInfo* ioSkillInfoMgr::GetSkillInfo( ioHashString& hsSkillName )
{
	auto it = m_mapSkillInfo.find(hsSkillName);
	if( it != m_mapSkillInfo.end())
		return &it->second;

	return NULL;
}

float ioSkillInfoMgr::CalGrowthValue( int iLevel, int iNumber )
{
	int iSize = m_vecGrowhUPInfo.size();
	for( int i = 0; i < iSize; ++i )
	{
		if( m_vecGrowhUPInfo[i].iNumber == iNumber )
		{
			//여기서 타입이 7 번인것만 스킬 쿨타임 감소임..
			if( m_vecGrowhUPInfo[i].iType != GT_SKILL_COOL_TIME )
				return 0.f;

			float fCurValue = 0.f;
			float iCurMax = 300.f;	//고정값이라서..

			if( iLevel <= m_vecGrowhUPInfo[i].fMaxValueSectionGap1 )
			{
				fCurValue = m_vecGrowhUPInfo[i].fMaxValueSection1 / iCurMax;
				fCurValue *= iLevel;
			}
			else if( iLevel <= m_vecGrowhUPInfo[i].fMaxValueSectionGap2 )
			{
				fCurValue = m_vecGrowhUPInfo[i].fMaxValueSection1 / iCurMax;
				fCurValue *= m_vecGrowhUPInfo[i].fMaxValueSectionGap1;

				float fCurValue2 = m_vecGrowhUPInfo[i].fMaxValueSection2 / iCurMax;
				fCurValue2 *= (iLevel - m_vecGrowhUPInfo[i].fMaxValueSectionGap1 );

				fCurValue += fCurValue2;
			}
			else if( iLevel > m_vecGrowhUPInfo[i].fMaxValueSectionGap2 )
			{
				fCurValue = m_vecGrowhUPInfo[i].fMaxValueSection1 / iCurMax;
				fCurValue *= m_vecGrowhUPInfo[i].fMaxValueSectionGap1;

				float fCurValue2 = m_vecGrowhUPInfo[i].fMaxValueSection2 / iCurMax;
				fCurValue2 *= (m_vecGrowhUPInfo[i].fMaxValueSectionGap2 - m_vecGrowhUPInfo[i].fMaxValueSectionGap1 );

				float fCurValue3 = m_vecGrowhUPInfo[i].fMaxValueSection3 / iCurMax;
				fCurValue3 *= (iLevel - m_vecGrowhUPInfo[i].fMaxValueSectionGap2 );

				fCurValue = fCurValue + fCurValue2 + fCurValue3;
			}
			else
				return m_vecGrowhUPInfo[i].fMaxValueSection1;

			return fCurValue;
		}
	}

	return 0.f;
}

float ioSkillInfoMgr::CalcSkillMaxValue( int iLevel, int iNumber, float fMaxSkillTime )
{
	float fValue = CalGrowthValue( iLevel, iNumber );
	fValue = 1.f + ( fValue / 100.f );
	float fMaxGauge = fMaxSkillTime / fValue;
	fMaxGauge = max( 0.0f, fMaxGauge );

	return fMaxGauge;
}

float ioSkillInfoMgr::GetMaxSkillCoolTime( ioHashString& hsSkillName, int iLevel )
{
	auto it = m_mapSkillInfo.find(hsSkillName);
	if( it == m_mapSkillInfo.end())
		return 0.f;

	SkillInfo* pSkillInfo = &it->second;
	if( !pSkillInfo )
		return 0.f;

	float fMaxCoolTime = CalcSkillMaxValue( iLevel, pSkillInfo->iMaxValueType, pSkillInfo->fMaxGauge );
	//여기서 추가로 어디까지 검사할지? 일단 기본만.
	return fMaxCoolTime;
}
void ioSkillInfoMgr::GetSkillType( ioHashString& hsSkillName, OUT int& iSkillType, OUT int& iSkillSubType )
{
	auto it = m_mapSkillInfo.find(hsSkillName);
	if( it == m_mapSkillInfo.end())
		return;

	SkillInfo* pSkillInfo = &it->second;
	if( !pSkillInfo )
		return;;

	iSkillType = pSkillInfo->iSkillType;
	iSkillSubType = pSkillInfo->iSkillSubType;
}

sWeaponInfo* ioSkillInfoMgr::GetWeaponInfo( DWORD dwItemCode )
{
	dwItemCode = GetPureItemCode(dwItemCode);

	auto it = m_mapWeaponInfo.find( dwItemCode );
	if( it == m_mapWeaponInfo.end() )
		return NULL;
	return &(it->second);
}

DWORD ioSkillInfoMgr::GetPureItemCode( DWORD itemCode )
{
	DWORD iSlot = itemCode / 100000;
	DWORD iCode = itemCode % 1000;
	DWORD iRare =  ( itemCode % 100000 ) % 10000 / 1000;

	if(iRare == 0)
		return ((iSlot * 100000) + iCode);

	return itemCode;
}

#endif