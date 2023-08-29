#pragma once

#include "../Util/Singleton.h"
#include <boost/unordered_map.hpp>

class ioPowerUpManager : public Singleton< ioPowerUpManager > 
{
protected:
	struct PowerUpCharInfo
	{
		DWORD dwCharCode;
		int	iMaxPowerUpGrade;
		boost::unordered_map<int, std::vector<int>> mPowerUpItemCodeInfo; //<��������, ��ũ������ �ڵ�>
		
		PowerUpCharInfo()
		{
			dwCharCode = 0;
			iMaxPowerUpGrade = 0;
			mPowerUpItemCodeInfo.clear();
		}
	};
	
public:
	ioPowerUpManager();
	virtual ~ioPowerUpManager();
	
	void Init();
	void Destroy();

	static ioPowerUpManager& GetSingleton();

protected:
	typedef std::vector<int> vItemGradeCodeInfo;
	typedef boost::unordered_map<int, vItemGradeCodeInfo> mPowerUpItemCode;
	typedef boost::unordered_map<int, int> mNeedMaterialCnt;
	typedef boost::unordered_map<int, PowerUpCharInfo> mPowerUpCharInfo;
	typedef boost::unordered_map<int, IntVec> mPowerUpRareItemInfo;  // <�⺻code, �ܰ躰 �ڵ�>

public:
	void LoadINI();
	void CheckNeedReload();

public:
	int TargetPowerUp(User *pUser, const int iTargetType, const int iTargetIndex, const int iCurMaterialCount, int &iNeedMaterialCount );
	int HeroPowerUp(User *pUser, const int iTargetIndex, const int iCurMaterialCount, int &iNeedMaterialCount);
	int ItemPowerUp(User *pUser, const int iTargetIndex, const int iCurMaterialCount, int &iNeedMaterialCount);

	BOOL CharGradeUp();
	BOOL ItemGradeUp();

public:
	int GetBasicNeedMaterialCount(const int iTargetType, const int iTargetGrade);
	int GetBasicPowerUpItemCode(const int iItemCode, const int iClassType, const int iGrade);
	int GetPowerUpItemGrade(int iItemCode);
	int GetPowerUpItemType(int iItemCode);

public:
	int ConvertPowerUpTypeCharToItem(int iCharGradeType);
	int ConvertPowerUpTypeItemToChar(int iItemGradeType);
	int ConvertRareItemToRareItemGrade( int iItemCode );

protected:
	mNeedMaterialCnt m_mHeroNeedMaterialCnt;
	mNeedMaterialCnt m_mBasicItemNeedMaterialCnt;
	mPowerUpCharInfo m_mPowerUpCharList;

	IntVec m_vRareItemNeedMaterialCnt;
	mPowerUpRareItemInfo m_mPowerUpRareItemList;

	int m_iMaterialCode;
};

#define g_PowerUpMgr ioPowerUpManager::GetSingleton()