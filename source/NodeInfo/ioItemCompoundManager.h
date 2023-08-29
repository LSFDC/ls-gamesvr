
#ifndef _ioItemCompoundManager_h_
#define _ioItemCompoundManager_h_

#include "../Util/Singleton.h"
#include "../Util/IORandom.h"

#include "ioExtraItemInfoManager.h"

class User;

typedef struct tagCompoundRateInfo
{
	int m_iLevel;
	int m_iSameItemRate;
	int m_iSameItemRateS;
	int m_iOtherItemRate;
	int m_iOtherItemRateS;

	tagCompoundRateInfo()
	{
		m_iLevel = 0;
		m_iSameItemRate = 0;
		m_iSameItemRateS = 0;
		m_iOtherItemRate = 0;
		m_iOtherItemRateS = 0;
	}
} CompoundRateInfo;
typedef std::vector<CompoundRateInfo> CompoundRateInfoList;

typedef struct tagCompoundInfo
{
	int m_iSmallUpReinforce;
	int m_iBigUpReinforce;
	int m_iDownReinforce;

	CompoundRateInfoList m_vRateInfoList;

	tagCompoundInfo()
	{
		m_iSmallUpReinforce = 1;
		m_iBigUpReinforce = 2;
		m_iDownReinforce = 1;

		m_vRateInfoList.clear();
	}
} CompoundInfo;

////////////////////용병조각, 첨가제 강화///////////////////////////
typedef struct tagMaterialCompoundRateInfo
{
	int m_iLevel;
	int m_iNeedMaterialCount;
	int m_iMaxFailExp;
	int m_iPcRoomBonus;

	tagMaterialCompoundRateInfo()
	{
		m_iLevel = 0;
		m_iNeedMaterialCount = 0;
		m_iMaxFailExp = 0;
		m_iPcRoomBonus	= 0;
	}
} MaterialCompoundRateInfo;
typedef std::vector< MaterialCompoundRateInfo > MaterialCompoundRateInfoList;

typedef struct tagMaterialCompoundInfo
{
	int m_iSuccessConstant;
	float m_fRareNeedMaterialRate;
	float m_iRareMaxFailExpRate;
	float m_fFailExpConstant;

	MaterialCompoundRateInfoList m_vMaterialRateInfoList;

	tagMaterialCompoundInfo()
	{
		m_iSuccessConstant = 0;
		m_fRareNeedMaterialRate = 0.0f;
		m_iRareMaxFailExpRate = 0.0f;
		m_fFailExpConstant = 0.0f;
	}
} MaterialCompoundInfo;

struct WrongMaterial
{
	float m_fDifferPieceRate;  //고급 -> 다른 조각
	float m_fRareItemDifferRate; //레어 -> 조각
	float m_fHighItemAdditiveRate; //고급 -> 첨가제

	WrongMaterial()
	{
		m_fDifferPieceRate = 0.0f;
		m_fRareItemDifferRate = 0.0f;
		m_fHighItemAdditiveRate = 0.0f;
	}
};

struct RightMaterial
{
	float m_fRightHighMaterialRate;	//고급 -> 올바른 조각
	float m_fRightRareMaterialRate; //레어 -> 첨가제

	RightMaterial()
	{
		m_fRightHighMaterialRate = 0.0f;
		m_fRightRareMaterialRate = 0.0f;
	}
};

class ioItemCompoundManager : public Singleton< ioItemCompoundManager >
{
protected:
	typedef std::map< DWORD, CompoundInfo > CompoundInfoMap;
	CompoundInfoMap m_CompoundInfoMap;
	IORandom m_RandomTime;

//용병 조각, 첨가제 강화	
protected:
	typedef std::map< DWORD, MaterialCompoundInfo  > MaterialCompoundInfoMap;
	MaterialCompoundInfoMap m_materialCompoundInfoMap;

	int m_iMaterialMaxReinforceInfo;
	int m_iFixFailReinforce; //몇강 이상일 시 실패시 이 강화 수로 다운.

	WrongMaterial m_kWrongMaterialPenalty;
	RightMaterial m_kRightMaterialRate;

	enum MaterialCompoundHelp
	{
		ADDITIVE_CODE_NUM = 100001,
		ROLLBACK_LEVEL = 25
	};

// Multiple Compound
protected:
	typedef std::map< DWORD, RandomInfo > RandomInfoMap;
	RandomInfoMap m_RandomInfoMap;

	RandomItemList m_vCurRandomItemList;

	DWORD m_dwTotalItemRate;
	DWORD m_dwTotalPeriodRate;
	DWORD m_dwTotalReinforceRate;

	IORandom m_ItemListRandom;
	IORandom m_PeriodTimeRandom;
	IORandom m_ReinforceRandom;

	bool m_bCheckTestCompound;
	bool m_bCheckTestMultiCompound;
	
	int  m_iMaxReinforceInfo;


protected:
	void ClearAllInfo();
	void ClearCompoundInfoMap();
	void ClearRandomInfoMap();
	void ClearMaterialCompoundInfoMap();

	int GetRandomItem( DWORD dwCode );
	int GetRandomPeriod( DWORD dwCode );
	int GetRandomReinforce( DWORD dwCode );

	int GetPCRoomBonusRate(int iLevel);

public:
	void LoadCompoundInfo();
	void LoadMultipleCompoundInfo();
	void LoadMaterialCompoundInfo();

	int  GetMaxCompoundInfo(){ return m_iMaxReinforceInfo; }

	/* return 값은 예외오류 번호. 0번은 오류 없음.*/
	int CheckCompoundSuccess( int iTargetSlot, int iVictimSlot, User *pUser, DWORD dwType );

	//조각, 첨가제 강화
	int CheckMaterialCompound( int iTargetSlot, int iMaterialCode, User *pUser, DWORD dwEtcItemCode );
	bool CheckMaterialCount( const int iRightCount, const int iUseCount );
	bool IsRareOrHighItem( const int iItemCode );
	bool IsRightMaterial( const int iItemCode, const int iMaterialCode, const bool bRare );
	bool ActReinforce( User *pUser, ioUserExtraItem::EXTRAITEMSLOT& extraItemSlot, const int iReinforceRate, const int iAddFailExp, const int iMaxFailExp );

	MaterialCompoundInfo* GetMaterialCompoundInfo( const DWORD dwEtcItemCode );
	bool SetMaterialReinforceConstant( User* pUser, ioUserExtraItem::EXTRAITEMSLOT& extraItemSlot, const int& iMaterialCode, int& iSuccessRate, 
										int& iAddFailExp, int& iMaxFailExp, int& iNeedMaterialCount, bool bRare, const DWORD dwEtcItemCode );


	float GetWrongMaterialRate( const int iMaterialCode, const bool bRare );
	float GetMaterialConst( const int iItemCode, const int iMaterialCode, const bool bRare );
	int GetSuccessRate( const int iCurExp, const int iCurMaxExp, const int iSuccessConst );
	int GetGainExp( int iNeedMaterialCount, int fFailExpConst, float fMaterialConst );
	////////////////////////////////
	bool CheckMultipleCompound( int iItem1, int iItem2, int iItem3, User *pUser, DWORD dwType );

	void CheckNeedReload();

	void CheckTestCompound();
	void CheckTestMultiCompound( bool bEqual, bool bRateTest, DWORD dwCode );
	
public:
	static ioItemCompoundManager& GetSingleton();

public:
	ioItemCompoundManager();
	virtual ~ioItemCompoundManager();
};

#define g_CompoundMgr ioItemCompoundManager::GetSingleton()

#endif
