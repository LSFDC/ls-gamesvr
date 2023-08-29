#pragma once

#include <unordered_map>
#include "../../ioINILoader\ioINILoader.h"

struct RoomSkillInfo
{
	//bool bSafety; ������Ƽ ����� ��� ��Ÿ�������� �����ѵ� 1.f�� �� ������..
	//float fSafety;

	float fRoomOptionType;	//�� ���� ���� ����
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
	SCT_BASIC,	// ���� �������� ��ü ����
	SCT_NEED,	// ���� �������� MAX - NEED ��ŭ ����	
	SCT_COUNT,	// 99�� ä���ٰ� 33���� ���� Ÿ��
	SCT_AST,	// Ư�̽�ų ó����..
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

	int iTickTime;	//�ϴ� ST_COUNT ������ �����..

	DWORD dwExtraValue;
	bool bReloadBullet;	//�Ѿ˼� ȸ��
	bool bRecoverGauge;	//ȸ����ų�� ���

	//buff -- �ϴ� ��
	
	float fGaugePerTick;
	int iFCTickTime;	//����ƮŬ������ �̰ɷ�

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
	DWORD dwMaxCount;			//�Ѿ� �ִ� ����
	DWORD dwIntervalBullet;		//�Ѿ˰� �Ѿ� ����
	DWORD dwIntervalMotion;		//�߻� ��ǰ� ���� �߻� ��� ����
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

	//���� �����߿��� ���������� ����ϴ� �ֵ��� �ϵ��ڵ�����
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


	//�Ѿ� ���� �ڷ����� ���⿡ �߰�
	std::map<DWORD,sWeaponInfo> m_mapWeaponInfo;
	DWORD GetPureItemCode( DWORD itemCode );
	sWeaponInfo* GetWeaponInfo( DWORD dwItemCode );
#endif
};
typedef cSingleton<ioSkillInfoMgr> S_ioSkillInfoMgr;
#define g_SkillInfoMgr S_ioSkillInfoMgr::GetInstance()

//////////////////////////////////////////////////////////////////////////
/*
��Ÿ�� ����
 1.f * fSkillIncRate * 1.f(��Ʈ/��Ʈ������) * max(1.f,fSkillGaugeRate) * fExtraRecoveryGaugeRate(�����ɸ����ٲ�)
 * safety(�ʺ��� = 1.f �� ���� ) * �⿩�� * ���ھ� * ������ * �ο� ����(��忡���� �����ϰ� ���) * ��ɼ� ���� * �ұ����÷��̺���( 0.f or 1.f )
 = resultA

 ���� ��� �� ��� ���ھ ���� 1.f~1.2f ����..
 ����Ʈ Ŭ�� ����,ü��, ������ ���� 1~1.3f * 1~1.3f * 1~1.2f

 ���� ��ų Ÿ�Կ� ���� 
 resultA * default_recovery_gauge( 0.1f or ����ݿ� �����ε�  ) * �⺻����Ʈ( �⺻ 1.f, ������� 0.5f)

 ret �� tic(100ms)�������� + ���༭ MaxSkillGauge �̻��̸� ��밡��. ( ���⼭ ����,��ȭ�� ���� max��Ÿ�� ���� �پ�� )

 ���°� BT_ESCAPE_DROP_ZONE or CS_DROP_ZONE_DOWN �� ��� ������Ʈ ������
 */

// �ƽ� ���� ����
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