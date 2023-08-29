#ifndef __ioPetInfoManager_h__
#define __ioPetInfoManager_h__

#include "../Util/Singleton.h"
#include "ioUserPet.h"

class User;
class ioPetInfoManager : public Singleton< ioPetInfoManager >
{
public:
	struct PetInfo
	{
		int iPetCode;
		//int iMaxRank;
		int iRecommedMaterialCode;

		PetInfo()
		{
			iPetCode = 0;
			//iMaxRank = 0;
			iRecommedMaterialCode = 0;
		}
	};

	struct PetRankInfo
	{
		int iRank;
		float iRankConst;
		int iRankMaxLevel;

		PetRankInfo()
		{
			iRank = 0;
			iRankConst = 0.0f;
			iRankMaxLevel = 0;
		}
	};

	struct AddExpMaterialConst
	{
		float fRightMaterial;
		float fDiffMaterial;
		float fAdditiveMaterial;

		AddExpMaterialConst()
		{
			fRightMaterial = 0.0f;
			fDiffMaterial = 0.0f;
			fAdditiveMaterial = 0.0f;
		}
	};

	//랭크, 레벨별 필요 재료 갯수
	struct PetNeedMaterialCount
	{
		int iRank;
		std::vector< int > m_vLevelNeedMaterialCount;

		PetNeedMaterialCount()
		{
			iRank = 0;
			m_vLevelNeedMaterialCount.clear();
		}
	};

protected:
enum MaterialCodeHelp
{
	ADDITIVE_CODE_NUM = 100001,
};

protected:
	void Clear();

protected:
	typedef std::vector< PetInfo > vPetInfoVec;
	typedef std::vector< PetRankInfo > vPetRankInfoVec;
	typedef std::vector< PetNeedMaterialCount > vPetNeedMaterialCountVec;

protected:
	vPetInfoVec m_vPetInfoVec;
	vPetRankInfoVec m_vPetRankInfoVec;
	//vPetNeedMaterialCountVec m_vNeedMaterialCountVec;
	vPetNeedMaterialCountVec m_vNeedMaterialCountVec;

protected:
	float m_fMaxExpConst; //최대 경험치 계산 상수
	AddExpMaterialConst m_rkAddExpMaterialConst; //경험치 상승 재료에 따른 계산 상수
	int m_iPetSellPeso;	//펫 판메시 획득 페소
	int m_iMaxPetCount;	//최대 소지 가능한 펫 숫자
	int m_iMaxRank;

public:
	static ioPetInfoManager& GetSingleton();

public:
	void LoadINI();
	void CheckNeedReload();

	int GetSellPeso( );
	bool SetMaxExp( ioUserPet::PETSLOT& rkPetSlot );

	int GetMaxExp( int iCurLevel, int iPetRank );
	float GetRankConst( const int& iPetRank );

	int GetAddExp( const ioUserPet::PETSLOT& rkPetSlot, const int& iMaterialCode );
	int GetPetRightMaterialCode( const int& iPetCode );
	int GetPetNeedMaterialCount( const int& iPetRank, const int& iPetLevel );
	int GetPetMaxLevel( const int& iRank );

	inline int GetPetMaxRank() { return m_iMaxRank; }
	PetRankInfo* GetPetRankInfo( const int& iPetRank );
	PetInfo*	GetPetInfo( const int& iPetCode );

	inline int GetMaxPetCount() { return m_iMaxPetCount; }

public:
	bool RightMaterialCheck( const int iPetCode , const int& iMaterialCode );
	bool AddExp( User *pUser, ioUserPet::PETSLOT& rkPetSlot, const int& iMaterialCode );

	void SetPetLevelUpInfo( ioUserPet::PETSLOT& rkPetSlot, int iAddExp, int iRankMaxLevel );
	int LevelUpCheck( ioUserPet::PETSLOT& rkPetSlot, int& iAddExp, const int& iRankMaxLevel );

	bool CheckRightPetCode( const int& iPetCode );
public:
	ioPetInfoManager();
	virtual ~ioPetInfoManager();
};

#define g_PetInfoMgr ioPetInfoManager::GetSingleton()

#endif