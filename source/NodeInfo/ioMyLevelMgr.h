

#ifndef _ioMyLevelMgr_h_
#define _ioMyLevelMgr_h_

#include "../Util/Singleton.h"

class ioMyLevelMgr : public Singleton< ioMyLevelMgr >
{
private:
	struct MortmainCharInfo
	{
		int m_iBelowGradeLevel;  // ��) ���1 ����( �Ʒú� ~ ���1 )
		int m_iHaveMortmainChar;

		MortmainCharInfo()
		{
			m_iBelowGradeLevel  = 0;			
			m_iHaveMortmainChar = 0;
		}
	};

private:
	// Class Level
	DWORDVec m_vSoldierLevelExp;
	int      m_iSoldierMaxLevel;

	// Grade Level
	DWORDVec m_vGradeLevelExp;
	int      m_iGradeMaxUP;	
	int      m_iGradeMaxLevel;
	int      m_iSealLevel;
	int      m_iNextSealLevel;

	// Award Level
	DWORDVec m_vAwardLevelExp;
	int      m_iAwardMaxLevel;

	// Medal Level
	DWORDVec m_vMedalLevelExp;
	int      m_iMedalMaxLevel;

	// Party Level
	float m_fPartyConstantA;
	int   m_iPartyConstantB;
	int   m_iPartyConstantC;
	int   m_iPartyConstantD;

	// Ladder Level
	float m_fLadderConstantA;
	int   m_iLadderConstantB;
	int   m_iLadderConstantC;
	int   m_iLadderConstantD;
	
	// Hero Level 
	float m_fHeroConstantA;
	int   m_iHeroConstantB;
	int   m_iHeroConstantC;
	int   m_iHeroConstantD;

	// Solo Level
	float m_fSoloConstantA;
	int   m_iSoloConstantB;
	int   m_iSoloConstantC;
	int   m_iSoloConstantD;

	// Fishing Level
	DWORDVec m_vFishingLevelExp;
	int		 m_iFishingMaxLevel;
	int		 m_iFishingSuccessExp;
	int		 m_iFishingFailExp;

	// Soldier Exp
	int      m_iFishingSuccessSoldierExp;
	int      m_iFishingFailSoldierExp;

	// Excavation Level
	DWORDVec m_vExcavationLevelExp;
	int		 m_iExcavationMaxLevel;
	int		 m_iExcavationSuccessExp;
	int		 m_iExcavationFailExp;

	// Excavation Soldier Exp
	int      m_iExcavationSuccessSoldierExp;
	int      m_iExcavationFailSoldierExp;
	
	// �ӽð��� ���� ���� �� ��
	int		m_iTemporaryUserLimitDay;

public:
	void LoadINIInfo();

	int GetNextLevelupExp( int iCurLv );
	int GetNextGradeupExp( int iCurLv );
	int GetMaxGradeUp();
	int GetMaxGradeLevel();
	int GetNextAwardupExp( int iCurLv );
	int GetNextMedalupExp( int iCurLv );
	
	int GetNextFishingLevelUpExp( int iCurLv );
	int GetMaxFishingLevel();
	int GetFishingSuccessExp();
	int GetFishingFailExp();
	int GetFishingSuccessSoldierExp();
	int GetFishingFailSoldierExp();

	__int64 GetNextPartyupExp( int iCurLv );
	__int64 GetNextLadderupExp( int iCurLv );
	__int64 GetNextHeroupExp( int iCurLv );
	__int64 GetNextSoloupExp( int iCurLv );

	int GetNextExcavationLevelUpExp( int iCurLv );
	int GetMaxExcavationLevel();
	int GetExcavationSuccessExp();
	int GetExcavationFailExp();
	int GetExcavationSuccessSoldierExp();
	int GetExcavationFailSoldierExp();

	int GetTempUserLimitDay()	{ return m_iTemporaryUserLimitDay; }

public:
	static ioMyLevelMgr& GetSingleton();

public:
	ioMyLevelMgr();
	virtual ~ioMyLevelMgr();
};

#define g_LevelMgr ioMyLevelMgr::GetSingleton()

#endif