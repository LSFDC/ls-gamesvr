

#ifndef _ioAlchemicMgr_h_
#define _ioAlchemicMgr_h_

#include "../Util/Singleton.h"

#include "Item.h"
#include "ioExtraItemInfoManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////
/// ioAlchemicInventory�� �ִ� ��ǰ���� ������ ����Ҽ� �ְ��ϴ� ��ġ��
////////////////////////////////////////////////////////////////////////////////////////////////

#define ALCHEMIC_ADDITIVE_DIV	100000

enum AlchemicType
{
	ALT_NONE,
	ALT_SOLDIER			= 1,	// �뺴
	ALT_ITEM			= 2,	// ���
	ALT_CHANGE			= 3,	// �ٸ��ɷ� ����
	ALT_EXCHANGE		= 4,	// Ư���� ������ ����
	ALT_SELL			= 5,	// peso�� ��ȯ
	ALT_NEW_SOLDIER		= 6,	// ���ο� �뺴 ����
	ALT_NEW_ITEM		= 7,	// ���ο� ��� ����
};

enum AlchemicResultType
{
	ART_SUCCESS,		// ����
	ART_FAIL,			// ����
	ART_NOT_FIND_FUNC,	// ��� code ����
	ART_NOT_MACH_FUNC,	// code�� ������ AlchemicType�� �ٸ�
	ART_NOT_MACH_VALUE,	// ������ ���� ����
	ART_NOT_EMPTY_SLOT,	// ��ĭ����
	ART_OVER_MAX_CNT,	// �ִ밹���ʰ�
	ART_NOT_ENOUGH_CNT,	// ��������
	ART_EXCEPTION,		// ���ܿ���
	ART_NOT_MACH_CODE,	// �ʿ��� ����, ÷���� ������ �ٸ�
	ART_PERIOD_ERROR,	// �Ⱓ�� ����
	ART_TABLE_ERROR,	// alchemic table error
};

enum DisassembleType
{
	ADT_NONE		= 0,
	ADT_SOLDIER		= 1,	// �뺴����
	ADT_EXTRAITEM	= 2,	// ������
};

enum 
{
	SOLDIER_PICE	= 1,
	ADDITION_AGENT	= 2
};

struct RecipeResultInfo
{
	int m_iPieceCode1;
	int m_iPieceCode2;
	int m_iPieceCode3;
	int m_iPieceCode4;
	int m_iAdditiveCode;

	float m_fSuccessRate;

	int m_iResultCode;
	
	RecipeResultInfo()
	{
		m_iPieceCode1 = 0;
		m_iPieceCode2 = 0;
		m_iPieceCode3 = 0;
		m_iPieceCode4 = 0;
		m_iAdditiveCode = 0;

		m_fSuccessRate = 0.0f;

		m_iResultCode = 0;
	}
};
typedef std::vector< RecipeResultInfo > vRecipeResultInfoList;

struct RandomResult
{
	int m_iResultCode;
	int m_iRandomRate;

	RandomResult()
	{
		m_iResultCode = 0;
		m_iRandomRate = 0;
	}
};
typedef std::vector< RandomResult > vRandomResultList;

struct RandomResultInfo
{
	vRandomResultList m_vList;
	DWORD m_dwTotalRate;

	RandomResultInfo()
	{
		m_vList.clear();
		m_dwTotalRate = 0;
	}
};

struct RandomReinforceInfo
{
	RandomReinforceList m_vList;
	DWORD m_dwTotalRate;

	RandomReinforceInfo()
	{
		m_vList.clear();
		m_dwTotalRate = 0;
	}
};

// new alchemic
struct NewAlchemicInfo
{
	int m_iPieceNum;
	int m_iAdditiveNum;
};

struct NewAlchemicPeriodInfo
{
	DWORD m_dwLimitCnt;
	DWORD m_dwTableNum;

	NewAlchemicPeriodInfo()
	{
		m_dwLimitCnt = 0;
		m_dwTableNum = 0;
	}
};
typedef std::vector< NewAlchemicPeriodInfo > vNewAlchemicPeriodInfoList;

struct RandomValue
{
	int m_iRandomRate;
	int m_iValue;

	RandomValue()
	{
		m_iRandomRate = 0;
		m_iValue = 0;
	}
};
typedef std::vector< RandomValue > vRandomValueList;

struct NewAlchemicPeriodTable
{
	vRandomValueList m_vList;
	DWORD m_dwTotalRate;

	NewAlchemicPeriodTable()
	{
		m_vList.clear();
		m_dwTotalRate = 0;
	}
};

struct AlchemicFuncInfo
{
	int m_iCode;
	bool m_bActive;
	AlchemicType m_AlchemicType;

	int m_iMaxCnt1;
	int m_iMaxCnt2;
	int m_iMaxCnt3;
	int m_iMaxCnt4;
	int m_iMaxAdditive;

	float m_fSuccessRate;
	int m_iPeriodValue;

	int m_iRecipeListNum;
	int m_iRandomListNum;
	int m_iReinforceNum;
	int m_iTradeTypeNum;

	int m_iNewAlchemicListTable;

	AlchemicFuncInfo()
	{
		Init();
	}

	void Init()
	{
		m_iCode = 0;
		m_bActive = false;
		m_AlchemicType = ALT_NONE;

		m_iMaxCnt1 = 0;
		m_iMaxCnt2 = 0;
		m_iMaxCnt3 = 0;
		m_iMaxCnt4 = 0;
		m_iMaxAdditive = 0;

		m_fSuccessRate = 0.0f;
		m_iPeriodValue = 0;

		m_iRecipeListNum = 0;
		m_iRandomListNum = 0;
		m_iReinforceNum = 0;
		m_iTradeTypeNum = 0;

		m_iNewAlchemicListTable = 0;
	}
};
typedef std::vector< AlchemicFuncInfo > vAlchemicFuncInfoList;

class PeriodInfoSort : public std::binary_function< const NewAlchemicPeriodInfo&, const NewAlchemicPeriodInfo&, bool >
{
public:
	bool operator()( const NewAlchemicPeriodInfo &lhs , const NewAlchemicPeriodInfo &rhs ) const
	{
		if( lhs.m_dwLimitCnt <= rhs.m_dwLimitCnt )
		{
			return true;
		}

		return false;
	}
};

class ioAlchemicMgr : public Singleton< ioAlchemicMgr >
{
protected:
	typedef std::map< int, vRecipeResultInfoList > RecipeResultInfoMap;
	typedef std::map< int, RandomResultInfo > RandomResultInfoMap;
	typedef std::map< int, RandomReinforceInfo > RandomReinforceInfoMap;
	typedef std::map< int, RandomTradeType > RandomTradeTypeInfoMap;
	typedef std::map< int, int > DisassembleInfoMap;
	typedef std::vector<int> SoulStoneGainRandom;
	typedef std::multimap< int, int > MercenaryRankMap;

	RecipeResultInfoMap m_RecipeMap;
	RandomResultInfoMap m_RandomMap;
	RandomReinforceInfoMap m_ReinforceMap;
	RandomTradeTypeInfoMap m_TradeTypeMap;

	DisassembleInfoMap m_SoldierDisassembleMap;
	DisassembleInfoMap m_ExtraItemDisassembleMap;

	vAlchemicFuncInfoList m_AlchemicFuncInfoList;

	SoulStoneGainRandom m_vSoultoneRandomInfo;
	MercenaryRankMap	m_mMercenaryRank;

	IORandom m_MainFailRandom;
	IORandom m_CountRandom;
	IORandom m_RecipeRandom;
	IORandom m_RandomRandom;
	IORandom m_ReinforceRandom;
	IORandom m_TradeTypeRandom;

	IORandom m_DisassembleRandom;

	IORandom m_NormalRandom;

// new alchemic
protected:
	typedef std::map< int, IntVec > AlcemicListMap;
	typedef std::map< int, NewAlchemicInfo > NewAlchemicInfoMap;
	typedef std::map< int, NewAlchemicPeriodTable > NewAlchemicPeriodTableMap;
	std::vector<int>	m_vecPermanentTable;

	AlcemicListMap m_NewAlchemicListMap;

	NewAlchemicInfoMap m_NewAlchemicSoldierInfoMap;
	NewAlchemicInfoMap m_NewAlchemicItemInfoMap;

	NewAlchemicPeriodTableMap m_SoldierPeriodTabelMap;
	NewAlchemicPeriodTableMap m_ItemPeriodTabelMap;

	vNewAlchemicPeriodInfoList m_SoldierPeriodInfoList;
	vNewAlchemicPeriodInfoList m_ItemPeriodInfoList;

	IORandom m_PeriodRandom;

	int m_iSoldierAdditive;
	int m_iWeaponAdditive;
	int m_iArmorAdditive;
	int m_iHelmetAdditive;
	int m_iCloakAdditive;

	int m_iNewAlchemicMinTotalCnt;

	int m_iPermenentConditional;

protected:
	float m_fMinFailRate;
	float m_fMaxFailRate;

	float m_fAdditiveConstSoldier;
	float m_fAdditiveConstItem;
	float m_fAdditiveConstExchange;

	float m_fMinExchangeRate;
	float m_fMaxExchangeRate;

	int m_iMinTotalCnt;
	float m_fPieceChangeConstMin;
	float m_fPieceChangeConstMax;
	float m_fItemSellConst;

	int m_iTotalSoulStoneRandom;
protected:
	DWORD m_dwSoldierDisassembleConst;
	DWORD m_dwExtraItemDisassembleConst;

	bool m_bPerfectConditions;

public:
	void CheckNeedReload();

	void LoadMgrInfo();

	void LoadRecipeInfo( ioINILoader &rkLoader );
	void LoadRandomInfo( ioINILoader &rkLoader );
	void LoadReinforceInfo( ioINILoader &rkLoader );
	void LoadTradeTypeInfo( ioINILoader &rkLoader );

	void LoadFuncInfo( ioINILoader &rkLoader );
	
	void LoadSoldierDisassembleInfo( ioINILoader &rkLoader );
	void LoadExtraItemDisassembleInfo( ioINILoader &rkLoader );

	void LoadSoulStoneInfo( ioINILoader &rkLoader );
	//
	void LoadNewAlchemicListTable( ioINILoader &rkLoader );
	void LoadNewAlchemicInfo( ioINILoader &rkLoader );
	void LoadNewAlchemicPeriodInfo( ioINILoader &rkLoader );

	void LoadNewAlchemicConditionsListTable( ioINILoader &rkLoader );

protected:
	void ClearAllInfo();

	bool FindAlchemicFunc( int iCode, AlchemicFuncInfo &rkFuncInfo );
	int CheckRecipeFunc( int iRecipeNum,
						 int iPiece1,
						 int iPiece2,
						 int iPiece3,
						 int iPiece4,
						 int iAdditive );

	int CheckRandomFunc( int iRandomNum );
	int CheckReinforceFunc( int iReinforceNum );
	int CheckTradeTypeFunc( int iTradeNum );

	float GetCurFailRandomRate();

public:
	// �뺴 : return ���� ����Ÿ��
	int AlchemicSoldierFunc( User *pUser,
							 int iCode,
							 int iPiece1, int iValue1,
							 int iPiece2, int iValue2,
							 int iPiece3, int iValue3,
							 int iPiece4, int iValue4,
							 int iAdditive, int iAddValue );

	// ��� : return ���� ����Ÿ��
	int AlchemicItemFunc( User *pUser,
						  int iCode,
						  int iPiece1, int iValue1,
						  int iPiece2, int iValue2,
						  int iPiece3, int iValue3,
						  int iPiece4, int iValue4,
						  int iAdditive, int iAddValue );

	// Change : �������� �ٸ��ɷ� : return ���� ����Ÿ��
	int AlchemicChangeFunc( User *pUser,
							int iCode,
							int iPiece1, int iValue1,
							int iPiece2, int iValue2,
							int iPiece3, int iValue3,
							int iPiece4, int iValue4 );

	// Exchange : �ϳ��� ������ �ٸ��ɷ� : return ���� ����Ÿ��
	// piece1�� ���, piece2�� ��ǥ. ��, piece2�� �� �ϳ��� �����ϰ� �־���Ѵ�
	int AlchemicExchangeFunc( User *pUser,
							  int iCode,
							  int iPiece1, int iValue1,
							  int iPiece2, int iValue2,
							  int iAdditive, int iAddValue );

	// Sell : item�� peso�� ��ȯ : return ���� ����Ÿ��
	int AlchemicSellFunc( User *pUser,
						  int iCode,
						  int iPiece1, int iValue1,
						  int iPiece2, int iValue2,
						  int iPiece3, int iValue3,
						  int iPiece4, int iValue4 );

	// new �뺴 : return ���� ����Ÿ��
	int NewAlchemicSoldierFunc( User *pUser,
								int iCode,
								int iClassType,
								int iPiece1, int iValue1,
								int iAdditive, int iAddValue );

	// new ��� : return ���� ����Ÿ��
	int NewAlchemicItemFunc( User *pUser,
							 int iCode,
							 int iItemCode,
							 int iPiece1, int iValue1,
							 int iAdditive, int iAddValue );

	int GetSoldierPeriodTime( int iPieceCnt );
	int GetItemPeriodTime( int iPieceCnt );

	bool CheckAlchemicTable( int iTableNum, int iValue );

	void LoadNewAlchemPermenentTable( ioINILoader &rkLoader );
	bool GetSoldierCheckPermenent( int iValue, int iPieceCnt );

public:
	int GetDisassembleCode( int iType, DWORD dwMagicCode );
	int GetDisassembleCnt( int iType, bool bMortmain, DWORD dwTime, DWORD dwMagicValue );

	int GetSoldierNeedPiece( int iClassType );

	int GetSouleStoneGainCnt( int iClassType = 0 );

public:
	inline float GetSellConst() const { return m_fItemSellConst; }

public:
	static ioAlchemicMgr& GetSingleton();

public:
	ioAlchemicMgr();
	virtual ~ioAlchemicMgr();
};

#define g_AlchemicMgr ioAlchemicMgr::GetSingleton()

#endif
