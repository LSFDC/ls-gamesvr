#pragma once

#include <unordered_map>
#include "../../ioINILoader\ioINILoader.h"

struct RoomSkillInfo
{
	//bool bSafety; 세이프티 모드일 경우 쿨타임조절이 가능한데 1.f로 다 고정임..
	//float fSafety;

	float fRoomOptionType;	//에 따른 보정 비율
	RoomSkillInfo() : fRoomOptionType(1.f) {};
};

struct GrowthSkillInfo
{
	int iNumber;
	int iType;

	float fMaxValueSection1;
	float fMaxValueSection2;
	float fMaxValueSection3;

	float fMaxValueSectionGap1;
	float fMaxValueSectionGap2;
	GrowthSkillInfo() : iNumber(0), fMaxValueSection1(0), fMaxValueSection2(0), fMaxValueSection3(0), fMaxValueSectionGap1(0), fMaxValueSectionGap2(0) {}
};

enum eSkillCheckType
{
	SCT_BASIC,	// 사용시 게이지가 전체 감소
	SCT_NEED,	// 사용시 게이지가 MAX - NEED 만큼 감소	
	SCT_COUNT,	// 99개 채웠다가 33개씩 쓰는 타입
	SCT_AST,	// 특이스킬 처리용..
	SCT_RETURN
};

struct SkillInfo
{
	int iSkillID;
	ioHashString hsSkillName;	
	int iSkillType;
	int iSkillSubType;
	int iEquipType;
	int iBuffType;
	int iMaxValueType;

	float fSkillGaugeRate;
	float fNeedGauge;
	float fMaxGauge;

	int iNeedCount;
	int iMaxCount;

	int iTickTime;	//일단 ST_COUNT 에서만 사용함..

	DWORD dwExtraValue;
	bool bReloadBullet;	//총알수 회복
	bool bRecoverGauge;	//회복스킬일 경우

	//buff -- 일단 뱀
	
	float fGaugePerTick;
	int iFCTickTime;	//파이트클럽에선 이걸로

	float fSkillIncRate;
	GrowthSkillInfo growthInfo;
	SkillInfo() : fSkillIncRate(1.f), fSkillGaugeRate(1.f), bRecoverGauge(false), bReloadBullet(false), dwExtraValue(0) {}

	int CheckSkillType()
	{
		if( iSkillSubType == AST_RANDOM_GENERATE || iSkillSubType == AST_METEOR )
			return SCT_AST;
		if( iSkillType == ST_BUFF )
		{
			if( (fMaxGauge-fNeedGauge) > 1.f )
				return SCT_NEED;
			return SCT_BASIC;
		}
		else if( iSkillType == ST_COUNT )
			return SCT_COUNT;
		
		return SCT_BASIC;
	}
};

struct sWeaponInfo
{
	DWORD dwMaxCount;			//총알 최대 갯수
	DWORD dwIntervalBullet;		//총알과 총알 간격
	DWORD dwIntervalMotion;		//발사 모션과 다음 발사 모션 간격
	DWORD dwReloadTime;		
	sWeaponInfo() : dwMaxCount(0), dwIntervalBullet(100), dwIntervalMotion(150), dwReloadTime(200){}
};

class ioSkillInfoMgr
{
public:
	ioSkillInfoMgr(void);
	~ioSkillInfoMgr(void);

#ifdef ANTIHACK

	void Init();

	//각종 세팅중에서 고정값으로 사용하는 애들은 하드코딩했음
	float m_fDefaultRecoveryGauge;
	float m_fDefaultRecoveryGaugeTic;

	float m_fSkillGaugeRate;
	float m_fExtraRate;
	float m_fSafteyRate;

	float m_fErrorRate;
	float GetErrorRate(){ return m_fErrorRate; }

	typedef boost::unordered_map< ioHashString, SkillInfo, ioHashString::KeyHasher > SkillInfoMap;
	SkillInfoMap m_mapSkillInfo;
	std::vector<GrowthSkillInfo> m_vecGrowhUPInfo;

	SkillInfo* GetSkillInfo( ioHashString& hsSkillName );

	float CalGrowthValue( int iLevel, int iNumber );
	float CalcSkillMaxValue( int iLevel, int iNumber, float fMaxSkillTime );

	float GetMaxSkillCoolTime( ioHashString& hsSkillName, int iLevel );
	void  GetSkillType( ioHashString& hsSkillName, OUT int& iSkillType, OUT int& iSkillSubType );


	//총알 관련 자료형도 여기에 추가
	std::map<DWORD,sWeaponInfo> m_mapWeaponInfo;
	DWORD GetPureItemCode( DWORD itemCode );
	sWeaponInfo* GetWeaponInfo( DWORD dwItemCode );
#endif
};
typedef cSingleton<ioSkillInfoMgr> S_ioSkillInfoMgr;
#define g_SkillInfoMgr S_ioSkillInfoMgr::GetInstance()

//////////////////////////////////////////////////////////////////////////
/*
쿨타임 공식
 1.f * fSkillIncRate * 1.f(비세트/세트아이템) * max(1.f,fSkillGaugeRate) * fExtraRecoveryGaugeRate(버프걸릴경우바뀜)
 * safety(초보방 = 1.f 다 같음 ) * 기여도 * 스코어 * 모드비율 * 인원 비율(모드에따라 복잡하게 계산) * 룸옵션 비율 * 소극적플레이비율( 0.f or 1.f )
 = resultA

 서바 모드 일 경우 스코어에 따라 1.f~1.2f 까지..
 파이트 클럽 연승,체력, 점수에 따라서 1~1.3f * 1~1.3f * 1~1.2f

 이후 스킬 타입에 따라서 
 resultA * default_recovery_gauge( 0.1f or 장비스텟에 따른인데  ) * 기본레이트( 기본 1.f, 죽을경우 0.5f)

 ret 를 tic(100ms)간격으로 + 해줘서 MaxSkillGauge 이상이면 사용가능. ( 여기서 육성,강화에 따라서 max쿨타임 값이 줄어듬 )

 상태가 BT_ESCAPE_DROP_ZONE or CS_DROP_ZONE_DOWN 일 경우 업데이트 안해줌
 */

// 맥스 벨류 계산법
/*
int iCurMax = g_GrowthInfoMgr.GetMaxLevel();
if( iCurMax == 0 )
	return m_fMaxValue_Section1;

float fCurValue = 0.f;
if( m_iCurLevel <= m_nMaxValue_SectionGap1 )
{
	fCurValue = m_fMaxValue_Section1 / iCurMax;
	fCurValue *= m_iCurLevel;
}
else if( m_iCurLevel <= m_nMaxValue_SectionGap2 )
{
	fCurValue = m_fMaxValue_Section1 / iCurMax;
	fCurValue *= m_nMaxValue_SectionGap1;

	float fCurValue2 = m_fMaxValue_Section2 / iCurMax;
	fCurValue2 *= (m_iCurLevel - m_nMaxValue_SectionGap1 );

	fCurValue += fCurValue2;
}
else if( m_iCurLevel > m_nMaxValue_SectionGap2 )
{
	fCurValue = m_fMaxValue_Section1 / iCurMax;
	fCurValue *= m_nMaxValue_SectionGap1;

	float fCurValue2 = m_fMaxValue_Section2 / iCurMax;
	fCurValue2 *= (m_nMaxValue_SectionGap2 - m_nMaxValue_SectionGap1);

	float fCurValue3 = m_fMaxValue_Section3 / iCurMax;
	fCurValue3 *= (m_iCurLevel - m_nMaxValue_SectionGap2 );

	fCurValue = fCurValue + fCurValue2 + fCurValue3;
}
else
	return m_fMaxValue_Section1;*/