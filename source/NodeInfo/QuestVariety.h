#ifndef _QuestVariety_h_
#define _QuestVariety_h_

/************************************************************************/
/* ����Ʈ�� ������ ó�� �߰��Ѵ�.                                       */
/************************************************************************/
class User;
class QuestData;
class QuestParent 
{
protected:
	DWORD m_dwMainIndex;                 // ����Ʈ ����
	
protected:
	DWORD m_dwSubIndex;                  // ����Ʈ ������ Ÿ��
	DWORD m_dwPerformType;               // ����Ʈ ���� Ÿ��         : 1 - �Ϲ�( 100������ �ε��� ��� ) , 2 - �̺�Ʈ( 1 ~ 99������ �ε��� ��� )

	bool  m_bOneDayStyle;                // ���� ����Ʈ ��� - �Ϸ� ��Ͽ� ���ԵǸ� ���� �Ϸ� ��Ͽ��� ������.
	int   m_iOneDayCount;				 // ���� ����Ʈ �ݺ����� Ƚ��
	bool  m_bRepeatStyle;                // �ݺ������� ���� ������ ����Ʈ - �Ϸ� ��Ͽ� �������� �ʴ´�.
	bool  m_bGuildStyle;                 // ��� ���� ����Ʈ - ��� �����ؾ� ���� �� �ְ� ��尡 ������ �������� ����Ʈ�� ������
	bool  m_bPCRoomStyle;                // �Ǿ��� ���� ����Ʈ

	ChannelingType m_eChannelingType;    //ä�θ� : �Ϲ�, mgame, ��

	DWORD m_dwOccurValue;                // �߻� ����
	int   m_iOccurModeType;              // �߻��� �ش� ��� üũ
	int   m_iOccurRoomStyle;             // �߻��� �� ��Ÿ��(����or����) üũ

	DWORD m_dwCompleteValue;             // �Ϸ� ����
	int   m_iCompleteModeType;           // �Ϸ�� �ش� ��� üũ
	int   m_iCompleteRoomStyle;          // �Ϸ�� �� ��Ÿ��(����or����) üũ

	DWORD m_dwPeriodHour;                // �ð� ���� ����Ʈ

	DWORDVec m_vRewardPresent;           // �Ϸ� ����
	
	bool  m_bRewardSelectStyle;			 // �Ϸ� ���� ���� ����Ʈ ����
	int	  m_iRewardSelectNum;			 // �Ϸ� ���� ���� ����	

	DWORD m_dwPrevMainIndex;             // �߻��� �Ϸ�Ǿ��־�� �� ����Ʈ MainIndex
	DWORD m_dwPrevSubIndex;              // �߻��� �Ϸ�Ǿ��־�� �� SubIndex

	DWORD m_dwNextMainIndex;             // �Ϸ�� ���� ����Ʈ MainIndex
	DWORD m_dwNextSubIndex;              // �Ϸ�� ���� ����Ʈ SubIndex

	bool  m_bCompleteGameAlarm;          // �Ϸ�(����ȹ��)�� ���ӳ� �����鿡�� �˸�
	bool  m_bCompleteWebAlarm;           // �Ϸ�(����ȹ��)�� ���� �˸�

protected:                               // �̺�Ʈ ����Ʈ : Ư�� �Ⱓ���� �ߵ�.
	WORD  m_wStartYear;
	WORD  m_wStartMonth;
	WORD  m_wStartDate;
	WORD  m_wStartHour;

	WORD  m_wEndYear;
	WORD  m_wEndMonth;
	WORD  m_wEndDate;
	WORD  m_wEndHour;

	bool  m_bAlive;

protected:    // UI��
	ioHashString m_szTitle;

protected:
	enum
	{
		MAX_CUSTOM_VALUE = 50,
	};
	IntVec m_CustomValue;                // ����Ʈ���� ����� ���.

public:
	inline DWORD GetMainIndex(){ return m_dwMainIndex; }
	inline DWORD GetSubIndex(){ return m_dwSubIndex; }
	inline DWORD GetPerformType(){ return m_dwPerformType; }
	inline bool  IsOneDayStyle(){ return m_bOneDayStyle; }
	inline bool  IsRepeatStyle(){ return m_bRepeatStyle; }
	inline bool  IsGuildStyle(){ return m_bGuildStyle; }
	inline bool  IsPCRoomStyle(){ return m_bPCRoomStyle; }
	inline ChannelingType GetChannelingType(){ return m_eChannelingType; }

	inline DWORD GetOccurValue(){ return m_dwOccurValue; }

	inline DWORD GetCompleteValue(){ return m_dwCompleteValue; }

	inline DWORD GetPeriodHour(){ return m_dwPeriodHour; }

	inline int GetMaxRewardPresent(){ return m_vRewardPresent.size(); }
	DWORD GetRewardPresentIndex( int iArray );

	inline bool	 IsRewardSelectStyle() const { return m_bRewardSelectStyle; }
	inline int   GetRewardSelectNum() const { return m_iRewardSelectNum; }

	inline DWORD GetPrevMainIndex(){ return m_dwPrevMainIndex; }
	inline DWORD GetPrevSubIndex(){ return m_dwPrevSubIndex; }
	inline DWORD GetNextMainIndex(){ return m_dwNextMainIndex; }
	inline DWORD GetNextSubIndex(){ return m_dwNextSubIndex; }

	inline const ioHashString &GetTitle(){ return m_szTitle; }
	inline void ChangeSubItemIndex(DWORD dwSubIndex) { m_dwSubIndex = dwSubIndex; }
	inline void ChangeNextIndex( DWORD dwMainIndex, DWORD dwChangeNextSubIndex) { m_dwNextMainIndex = dwMainIndex, m_dwNextSubIndex = dwChangeNextSubIndex; }

public:
	int GetCustomValue( int iIdx );
	
public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsCheckQuestCompeleteUser( User *pUser ){ return false; };

public:
	virtual bool SetQuestData( DWORD dwMainIndex, DWORD dwSubIndex, ioINILoader &rkLoader );

public:
	virtual bool AttainProcess( QuestData *pQuestData, int iCount = 0 );

public:
	virtual void ProcessRewardComplete( User *pUser );          // ���� ���� ���� ó�� ���� ������ ó��

public:
	virtual DWORD GetPresetMagicData(){ return 0; }

public:
	virtual bool IsAbuseCheckQuest(){ return false; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue ){ return false; }	

public:
	virtual bool IsCheckQuestReConnectSeconds(){ return false; }

public:
	void SetAlive( bool bAlive ){ m_bAlive = bAlive; }
	bool  IsAlive(){ return m_bAlive; }
	bool  IsCheckAlive();
	DWORD GetStartDateData();
	DWORD GetEndDateData();

public:
	inline bool IsCompleteGameAlarm(){ return m_bCompleteGameAlarm; } 
	inline bool IsCompleteWebAlarm(){ return m_bCompleteWebAlarm; }

public:   
	QuestParent();
	virtual ~QuestParent();
};
//////////////////////////////////////////////////////////////////////////
class QuestBasic : public QuestParent
{
public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBasic(){}
	virtual ~QuestBasic(){}
};
//////////////////////////////////////////////////////////////////////////
class QuestMonsterModeClear : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CLEAR_ROUND,
		CLEAR_COUNT,
		PLAYER_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMonsterModeClear();
	virtual ~QuestMonsterModeClear();
};
//////////////////////////////////////////////////////////////////////////
class QuestEnterBattlePvPMode : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEnterBattlePvPMode();
	virtual ~QuestEnterBattlePvPMode();
};
//////////////////////////////////////////////////////////////////////////
class QuestEnterBattlePvPModeKO : public QuestParent
{
public:
	enum
	{
		KILL_COUNT = 0,
		BEST_ATTACKER_PASS,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEnterBattlePvPModeKO();
	virtual ~QuestEnterBattlePvPModeKO();
};
//////////////////////////////////////////////////////////////////////////
class QuestEnterBattlePvPModeWin : public QuestParent
{
public:
	enum
	{
		WIN_COUNT = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEnterBattlePvPModeWin();
	virtual ~QuestEnterBattlePvPModeWin();
};
//////////////////////////////////////////////////////////////////////////
class QuestPvEMonsterKill : public QuestParent
{
public:
	enum
	{
		KILL_COUNT = 0,
		LIMIT_MIN_FLOOR,
		LIMIT_MAX_FLOOR,
		KILL_SOLDIER_CODE,
		LIMIT_GRADE,
		MAX_MONSTER_CODE,
		MONSTER_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPvEMonsterKill();
	virtual ~QuestPvEMonsterKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestGradeUP : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		COMPLETE_GRADE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsCheckQuestCompeleteUser( User *pUser );

public:
	QuestGradeUP();
	virtual ~QuestGradeUP();
};
//////////////////////////////////////////////////////////////////////////
class QuestTimeGrowthTry : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestTimeGrowthTry();
	virtual ~QuestTimeGrowthTry();
};
//////////////////////////////////////////////////////////////////////////
class QuestTimeGrowthSuccess : public QuestParent
{
public:
	enum
	{
		GROWTH_UP_LEVEL = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestTimeGrowthSuccess();
	virtual ~QuestTimeGrowthSuccess();
};
//////////////////////////////////////////////////////////////////////////
class QuestPesoGrowthTry : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPesoGrowthTry();
	virtual ~QuestPesoGrowthTry();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingTry : public QuestParent
{
public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingTry();
	virtual ~QuestFishingTry();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingSuccess : public QuestParent
{
public:
	enum
	{
		FISHING_SUCCESS_COUNT = 0,
		LIMIT_FISHING_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingSuccess();
	virtual ~QuestFishingSuccess();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingFailed : public QuestParent
{
public:
	enum
	{
		FISHING_FAIELD_COUNT = 0,
		LIMIT_FISHING_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingFailed();
	virtual ~QuestFishingFailed();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingLevelUP : public QuestParent
{
public:
	enum
	{
		LIMIT_FISHING_LEVEL = 0,
		CHECK_FISHING_LEVEL,
		MINUS_FISHING_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingLevelUP();
	virtual ~QuestFishingLevelUP();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingSellPeso : public QuestParent
{
public:
	enum
	{
		LIMIT_FISHING_LEVEL = 0,
		SELL_FISH_LIMIT_PESO,
		COMPLETE_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingSellPeso();
	virtual ~QuestFishingSellPeso();
};
//////////////////////////////////////////////////////////////////////////
class QuestFishingSuccessItem : public QuestParent
{
public:
	enum
	{
		LIMIT_FISHING_LEVEL = 0,
		FISHING_ITEM_TYPE,
		COMPLETE_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestFishingSuccessItem();
	virtual ~QuestFishingSuccessItem();
};
//////////////////////////////////////////////////////////////////////////
class QuestBattlePvPModeAwardAcquire : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		AWARD_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBattlePvPModeAwardAcquire();
	virtual ~QuestBattlePvPModeAwardAcquire();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoldierPractice : public QuestParent
{
public:
	enum
	{
		SOLDIER_CODE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestSoldierPractice();
	virtual ~QuestSoldierPractice();
};
//////////////////////////////////////////////////////////////////////////
class QuestExtraItemReinforceSuccess : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		EXTRAITEM_REINFORCE_SUCCESS,
		MAX_EXTRA_ITEM_CODE,
		EXTRA_ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExtraItemReinforceSuccess();
	virtual ~QuestExtraItemReinforceSuccess();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoldierLevelUP : public QuestParent
{
public:
	enum
	{
		SOLDIER_CODE = 0,
		SOLDIER_LEVEL,
		SOLDIER_LIMIT_LEVEL,
		SOLDIER_MINUS_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsCheckQuestCompeleteUser( User *pUser );

public:
	QuestSoldierLevelUP();
	virtual ~QuestSoldierLevelUP();
};
//////////////////////////////////////////////////////////////////////////
class QuestOpenManual : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_MANUAL_ID,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestOpenManual();
	virtual ~QuestOpenManual();
};
//////////////////////////////////////////////////////////////////////////
class QuestLoginCount : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LOGIN_COUNT,     // �α��� Ƚ��
		LIMIT_LOW_DAY,   // �α��� ī��Ʈ ���� �ּ� ����
		LIMIT_HIGH_DAY,  // �α��� ī��Ʈ �ʱ�ȭ �ִ밪.
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	virtual DWORD GetPresetMagicData();

public:
	QuestLoginCount();
	virtual ~QuestLoginCount();
};
//////////////////////////////////////////////////////////////////////////
class QuestEnterPlaza : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEnterPlaza();
	virtual ~QuestEnterPlaza();
};
//////////////////////////////////////////////////////////////////////////
class QuestGetPotion : public QuestParent
{
public:
	enum
	{
		CHECK_ROUND = 0,
		CHECK_COUNT,
		LIMIT_MIN_FLOOR,
		LIMIT_MAX_FLOOR,
	};

	enum
	{
		POTION_CODE1 = 401000,        //HP ����
		POTION_CODE2 = 401001,		  //��� HP ����
		POTION_CODE3 = 401002,		  //��ų ����
		POTION_CODE4 = 401003,		  //��� ��ų ����
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGetPotion();
	virtual ~QuestGetPotion();
};
//////////////////////////////////////////////////////////////////////////
class QuestTutorial : public QuestParent
{

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestTutorial();
	virtual ~QuestTutorial();
};
//////////////////////////////////////////////////////////////////////////
class QuestPresentReceive : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		RECV_COUNT,  
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPresentReceive();
	virtual ~QuestPresentReceive();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampJoin : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampJoin();
	virtual ~QuestCampJoin();
};
//////////////////////////////////////////////////////////////////////////
class QuestEnterCampBattle : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEnterCampBattle();
	virtual ~QuestEnterCampBattle();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampBattleKO : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		KILL_COUNT,
		BEST_ATTACKER_PASS,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampBattleKO();
	virtual ~QuestCampBattleKO();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampBattleWin : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		WIN_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampBattleWin();
	virtual ~QuestCampBattleWin();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampSeasonReward : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_WIN_LOSE,
		CHECK_CAMP_POINT,
		CHECK_CAMP_RANK,
		CHECK_CAMP_BONUS,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampSeasonReward();
	virtual ~QuestCampSeasonReward();
};
//////////////////////////////////////////////////////////////////////////
class QuestAwardAcquire : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_AWARD_TYPE,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestAwardAcquire();
	virtual ~QuestAwardAcquire();
};
//////////////////////////////////////////////////////////////////////////
class QuestPrisonerDrop : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPrisonerDrop();
	virtual ~QuestPrisonerDrop();
};
//////////////////////////////////////////////////////////////////////////
class QuestPrisonerSave : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPrisonerSave();
	virtual ~QuestPrisonerSave();
};
//////////////////////////////////////////////////////////////////////////
class QuestBattleLevel : public QuestParent
{
public:
	enum
	{
		LIMIT_BATTLE_LEVEL = 0,
		CHECK_BATTLE_LEVEL,
		MINUS_BATTLE_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBattleLevel();
	virtual ~QuestBattleLevel();
};
//////////////////////////////////////////////////////////////////////////
class QuestAwardLevel : public QuestParent
{
public:
	enum
	{
		LIMIT_AWARD_LEVEL = 0,
		CHECK_AWARD_LEVEL,
		MINUS_AWARD_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestAwardLevel();
	virtual ~QuestAwardLevel();
};
//////////////////////////////////////////////////////////////////////////
class QuestContribute : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_CONTRIBUTE,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestContribute();
	virtual ~QuestContribute();
};
//////////////////////////////////////////////////////////////////////////
class QuestDropKill : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		KILL_COUNT,
		BEST_ATTACKER_PASS,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestDropKill();
	virtual ~QuestDropKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestMultiKill : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		KILL_COUNT,
		BEST_ATTACKER_PASS,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMultiKill();
	virtual ~QuestMultiKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestPvPConsecution : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CONSECUTION_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPvPConsecution();
	virtual ~QuestPvPConsecution();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampConsecution : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CONSECUTION_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampConsecution();
	virtual ~QuestCampConsecution();
};
//////////////////////////////////////////////////////////////////////////
class QuestEtcItemUse : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		USE_COUNT,
		MAX_ETC_ITEM,
		ETC_ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestEtcItemUse();
	virtual ~QuestEtcItemUse();
};
//////////////////////////////////////////////////////////////////////////
class QuestEtcItemCnt : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		ETC_ITEM_CODE,
		BOOLEAN_ETC_ITEM_DELETE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	virtual void ProcessRewardComplete( User *pUser );

public:
	QuestEtcItemCnt();
	virtual ~QuestEtcItemCnt();
};
//////////////////////////////////////////////////////////////////////////
class QuestRequestFriend : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestRequestFriend();
	virtual ~QuestRequestFriend();
};
//////////////////////////////////////////////////////////////////////////
class QuestMakeFriends : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		FRIEND_COUNT,
		LIMIT_FRIEND_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMakeFriends();
	virtual ~QuestMakeFriends();
};
//////////////////////////////////////////////////////////////////////////
class QuestModePlayTime : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_PLAY_MIN,
		CHECK_SOLDIER,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	QuestModePlayTime();
	virtual ~QuestModePlayTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestStoneAttack : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		TEAM_STYLE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestStoneAttack();
	virtual ~QuestStoneAttack();
};
//////////////////////////////////////////////////////////////////////////
class QuestKingAttack : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestKingAttack();
	virtual ~QuestKingAttack();
};
//////////////////////////////////////////////////////////////////////////
class QuestCrownHoldTime : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCrownHoldTime();
	virtual ~QuestCrownHoldTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestBossModeBecomeBoss : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBossModeBecomeBoss();
	virtual ~QuestBossModeBecomeBoss();
};
//////////////////////////////////////////////////////////////////////////
class QuestBossModeBosswithKill : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		DEATH_INIT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBossModeBosswithKill();
	virtual ~QuestBossModeBosswithKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestMortmainSoldierCount : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		LIMIT_MORTMAIN_SOLDIER,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMortmainSoldierCount();
	virtual ~QuestMortmainSoldierCount();
};
////////////////////////////////////////////////////////////////////////// 
class QuestHitCount : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestHitCount();
	virtual ~QuestHitCount();
};
//////////////////////////////////////////////////////////////////////////
class QuestDefenceCount : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestDefenceCount();
	virtual ~QuestDefenceCount();
};
//////////////////////////////////////////////////////////////////////////
class QuestPickItem : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		MAX_ITEM_CODE,
		PICK_ITEM_CODE,		
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPickItem();
	virtual ~QuestPickItem();
};
//////////////////////////////////////////////////////////////////////////
class QuestCampLevel : public QuestParent
{
public:
	enum
	{
		LIMIT_CAMP_LEVEL = 0,
		CHECK_CAMP_LEVEL,
		MINUS_CAMP_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestCampLevel();
	virtual ~QuestCampLevel();
};
//////////////////////////////////////////////////////////////////////////
class QuestBuyItem : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		MAX_ITEM_CODE,
		ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestBuyItem();
	virtual ~QuestBuyItem();
};
//////////////////////////////////////////////////////////////////////////
class QuestMaxFriendSlot : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMaxFriendSlot();
	virtual ~QuestMaxFriendSlot();
};
//////////////////////////////////////////////////////////////////////////
class QuestRealTimeCount : public QuestParent
{
public:
	enum
	{
		LIMIT_MIN_GRADE = 0,
		LIMIT_MAX_GRADE,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );
	virtual bool IsCheckQuestReConnectSeconds(){ return true; }

public:
	QuestRealTimeCount();
	virtual ~QuestRealTimeCount();
};
//////////////////////////////////////////////////////////////////////////
class QuestPlayTimeCount : public QuestParent
{
public:
	enum
	{
		LIMIT_MIN_GRADE = 0,
		LIMIT_MAX_GRADE,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	QuestPlayTimeCount();
	virtual ~QuestPlayTimeCount();
};
//////////////////////////////////////////////////////////////////////////
class QuestFriendRecommendPlayTime : public QuestParent
{
public:
	enum
	{
		LIMIT_MIN_GRADE = 0,
		LIMIT_MAX_GRADE,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	virtual void ProcessRewardComplete( User *pUser );          // ���� ���� ���� ó�� ���� ������ ó��

public:
	QuestFriendRecommendPlayTime();
	virtual ~QuestFriendRecommendPlayTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestQuickStartOption : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_INDEX,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestQuickStartOption();
	virtual ~QuestQuickStartOption();
};
//////////////////////////////////////////////////////////////////////////
class QuestBuySoldier : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		CHECK_TYPE,
		CHECK_STYLE,
		MAX_SOLDIER,
		SOLDIER_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	virtual void ProcessRewardComplete( User *pUser );          // ���� ���� ���� ó�� ���� ������ ó��

public:
	QuestBuySoldier();
	virtual ~QuestBuySoldier();
};
//////////////////////////////////////////////////////////////////////////
class QuestPvETimeAttack : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_ROUND,
		CHECK_SECOND,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestPvETimeAttack();
	virtual ~QuestPvETimeAttack();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoccerBallHit : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestSoccerBallHit();
	virtual ~QuestSoccerBallHit();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoccerGoal : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestSoccerGoal();
	virtual ~QuestSoccerGoal();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoccerAssist : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestSoccerAssist();
	virtual ~QuestSoccerAssist();
};
//////////////////////////////////////////////////////////////////////////
class QuestSoccerSave : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestSoccerSave();
	virtual ~QuestSoccerSave();
};
//////////////////////////////////////////////////////////////////////////
class QuestExcavationTry : public QuestParent
{
public:
	enum
	{
		LIMIT_EXCAVATION_LEVEL = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExcavationTry();
	virtual ~QuestExcavationTry();
};
//////////////////////////////////////////////////////////////////////////
class QuestExcavationLevelUP : public QuestParent
{
public:
	enum
	{
		LIMIT_EXCAVATION_LEVEL = 0,
		CHECK_EXCAVATION_LEVEL,
		MINUS_EXCAVATION_LEVEL,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExcavationLevelUP();
	virtual ~QuestExcavationLevelUP();
};
//////////////////////////////////////////////////////////////////////////
class QuestExcavationSuccess : public QuestParent
{
public:
	enum
	{
		LIMIT_EXCAVATION_LEVEL = 0,
		CHECK_COUNT,
		LIMIT_PESO_GRADE,
		LIMIT_PESO,
		LIMIT_REINFORCE,
		LIMIT_PERIOD_TIME,
		MAX_ARTIFACT,
		ARTIFACT_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExcavationSuccess();
	virtual ~QuestExcavationSuccess();
};
//////////////////////////////////////////////////////////////////////////
class QuestExcavationFail : public QuestParent
{
public:
	enum
	{
		LIMIT_EXCAVATION_LEVEL = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExcavationFail();
	virtual ~QuestExcavationFail();
};
//////////////////////////////////////////////////////////////////////////
class QuestExcavationTime : public QuestParent
{
public:
	enum
	{
		LIMIT_EXCAVATION_LEVEL = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	QuestExcavationTime();
	virtual ~QuestExcavationTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestScreenShot : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestScreenShot();
	virtual ~QuestScreenShot();
};
//////////////////////////////////////////////////////////////////////////
class QuestMovieMode : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMovieMode();
	virtual ~QuestMovieMode();
};
//////////////////////////////////////////////////////////////////////////
class QuestMakeMovie : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestMakeMovie();
	virtual ~QuestMakeMovie();
};
//////////////////////////////////////////////////////////////////////////
class QuestExtraItemAcquire : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_PERIOD_TIME,
		MAX_EXTRA_ITEM,
		EXTRA_ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExtraItemAcquire();
	virtual ~QuestExtraItemAcquire();
};
//////////////////////////////////////////////////////////////////////////
class QuestExtraItemEquip : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_PERIOD_TIME,
		EXTRA_ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExtraItemEquip();
	virtual ~QuestExtraItemEquip();
};
//////////////////////////////////////////////////////////////////////////
class QuestExtraItemEquipKo : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		BEST_ATTACKER_PASS,
		EXTRA_ITEM_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestExtraItemEquipKo();
	virtual ~QuestExtraItemEquipKo();
};
//////////////////////////////////////////////////////////////////////////
class QuestGangsiKill : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGangsiKill();
	virtual ~QuestGangsiKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestGangsiHumanKill : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGangsiHumanKill();
	virtual ~QuestGangsiHumanKill();
};
//////////////////////////////////////////////////////////////////////////
class QuestGangsiAliveTime : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGangsiAliveTime();
	virtual ~QuestGangsiAliveTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestGangsiHumanWin : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGangsiHumanWin();
	virtual ~QuestGangsiHumanWin();
};
//////////////////////////////////////////////////////////////////////////
class QuestHitAttackAttribute : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_COUNT,
		MAX_ATTRIBUTE_CODE,
		ATTRIBUTE_CODE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestHitAttackAttribute();
	virtual ~QuestHitAttackAttribute();
};
//////////////////////////////////////////////////////////////////////////
class QuestGuildLevel : public QuestParent
{
public:
	enum
	{
		CHECK_COUNT = 0,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGuildLevel();
	virtual ~QuestGuildLevel();
};
//////////////////////////////////////////////////////////////////////////
class QuestGuildLevelMaintenance : public QuestParent
{
public:
	enum
	{
		LIMIT_GUILD_LEVEL = 0,
		CHECK_COUNT,
		LIMIT_CAMP_REWARD,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGuildLevelMaintenance();
	virtual ~QuestGuildLevelMaintenance();
};
//////////////////////////////////////////////////////////////////////////
class QuestGuildTeamWin : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		WIN_COUNT,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestGuildTeamWin();
	virtual ~QuestGuildTeamWin();
};
//////////////////////////////////////////////////////////////////////////
class QuestGuildTeamPlayTime : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		CHECK_PLAY_MIN,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	QuestGuildTeamPlayTime();
	virtual ~QuestGuildTeamPlayTime();
};
//////////////////////////////////////////////////////////////////////////
class QuestPlayTimeRepeat : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		REPEAT_TIME,          // �ݺ� �ð�
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );
	virtual bool IsAbuseCheckQuest(){ return true; }
	virtual bool IsAbuseCheck( DWORD dwGapTime, int iGapValue );

public:
	QuestPlayTimeRepeat();
	virtual ~QuestPlayTimeRepeat();
};
//////////////////////////////////////////////////////////////////////////
class QuestDormantUser : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_DAY,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestDormantUser();
	virtual ~QuestDormantUser();
};
//////////////////////////////////////////////////////////////////////////
class QuestDormantUserCustom : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		LIMIT_YEAR,
		LIMIT_MONTH,
		LIMIT_DAY,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestDormantUserCustom();
	virtual ~QuestDormantUserCustom();
};
//////////////////////////////////////////////////////////////////////////
class QuestDevKMove : public QuestParent
{
public:
	enum
	{
		LIMIT_GRADE = 0,
		POSITION_X,
		POSITION_Y,
		POSITION_Z,
		POSITION_RANGE,
	};

public:
	virtual bool IsCheckCompleteTerm( User *pUser, QuestData *pQuestData );

public:
	QuestDevKMove();
	virtual ~QuestDevKMove();
};
#endif