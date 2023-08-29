#ifndef __ioEventManager_h__
#define __ioEventManager_h__

#include "../Util/Singleton.h"
#include "ModeDefine.h"
#include <boost/unordered/unordered_map.hpp>

// server�� ������ �ִ� event
enum EventType
{
	EVT_NONE     = 0,
	EVT_PROPOSAL = 1,
	EVT_COIN     = 2,
	EVT_EXP      = 3, 
	EVT_PESO     = 4,
	EVT_PLAYTIME = 5,
	EVT_CHILDRENDAY = 6,
	EVT_PESOBONUS   = 7,
	EVT_BUY_CHAR_NO_LEVEL_LIMIT = 8,
	EVT_GRADEUP     = 9,
	EVT_PCROOM_BONUS=10,
	EVT_CHANCE_MORTMAIN_CHAR = 11,
	EVT_ONE_DAY_GOLD_ITEM = 12,
	EVT_DORMANCY_USER     = 13,
	EVT_PLAYTIME_PRESENT  = 14,
	EVT_CHRISTMAS         = 15,
	EVT_BUY_ITEM          = 16,
	EVT_FISHING			  = 17,
	EVT_EXERCISESOLDIER   = 18,
	EVT_CONNECTION_TIME   = 19,
	EVT_ONE_DAY_GIFT      = 20,
	EVT_GRADEUP_PRESENT   = 21,
	EVT_CONNECTION_TIME_SELECT_GIFT = 22,
	EVT_ENTRY             = 23,
	EVT_ONE_DAY_GIFT_2    = 24,
	EVT_BUY_ITEM_2        = 25,
	EVT_LADDER_POINT      = 26,
	EVT_BUY_ITEM_3        = 27,
	EVT_ANNOUNCE          = 28,
	EVT_ENTRY_AFTER       = 29,
	EVT_CONNECT_AND_PLAYTIME  = 30,
	EVT_ROULETTE			= 31,

	EVT_PLAZA_MONSTER		= 32,	// ���� ����
	EVT_EXP2				= 33,	// ����ġ2
	EVT_PES02				= 34,	// ���2
	EVT_MODE_BONUS			= 35,	// ��� ���ʽ�
	EVT_MODE_BONUS2			= 36,	// ��� ���ʽ�2

	EVT_MONSTER_DUNGEON		= 37,	// ���� ���� ī�� �߰� ����
	EVT_PCROOM_MONSTER_DUNGEON	= 38,

	EVT_PRESENT_BUY			= 39,	// ���� �� ���� �� �������Ե� �ش� ������ ����.
	EVT_FREEDAY_HERO			= 40,
	EVT_HERO_EXP_BOOST = 41,
};

// event vector array  
enum 
{
	// ALL
	MAX_EVENT_CHECK_MS       = 10000,// millisecond playtime
	MAX_EVENT_TIME_CHECK_SEC = 60,   // seconds connect time

	// �Ʒ� ������ sp2_event.ini �� value ���� ��Ī��.

	// EVT_PROPOSAL 
	EA_MAX_PROPOSAL_CHAR = 0,
	EA_MAX_CHAR_DAY      = 1,
	EA_MAX_CHAR_USE_SEC  = 2,

	// EVT_COIN
	EA_MAX_PLAY_SEC     = 0,
	EA_NORMAL_GIVE_CNT  = 1,
	EA_BURNING_GIVE_CNT = 2,
	EA_GRADEUP_GIVE_CNT = 3,
	EA_BURNING_HOUR_1   = 4,
	EA_BURNING_HOUR_2   = 5,

	// EVT_EXP
	EA_EXP_PER                      = 0,
	EA_EXP_FIRST_EVENT_WEEK_ON_OFF  = 1,
	EA_EXP_FIRST_START_TIME         = 2,
	EA_EXP_FIRST_END_TIME           = 3,
	EA_EXP_SECOND_EVENT_WEEK_ON_OFF = 4,
	EA_EXP_SECOND_START_TIME        = 5,
	EA_EXP_SECOND_END_TIME          = 6,

	// EVT_PESO 
	EA_PESO_PER                 = 0,
	EA_FIRST_EVENT_WEEK_ON_OFF  = 1,
	EA_FIRST_START_TIME         = 2,
	EA_FIRST_END_TIME           = 3,
	EA_SECOND_EVENT_WEEK_ON_OFF = 4,
	EA_SECOND_START_TIME        = 5,
	EA_SECOND_END_TIME          = 6,


	// EVT_PLAYTIME
	EA_PASS_PLAYTIME_1  = 0,
	EA_PASS_PLAYTIME_2  = 1,
	EA_PASS_PLAYTIME_3  = 2,
	EA_PASS_PLAYTIME_4  = 3,
	EA_PASS_PLAYTIME_5  = 4, 
	EA_GIFT_1           = 5,
	EA_GIFT_2           = 6,
	EA_GIFT_3           = 7,
	EA_GIFT_4           = 8,
	EA_GIFT_5           = 9,

	// EVT_PESOBONUS
	EA_BONUS_PESO       = 0,

	// EVT_CHILDRENDAY
	EA_CHILDRENDAY_IS_LIMIT = 0,

	// EVT_BUY_CHAR_NO_LEVEL_LIMIT
	EA_CHAR_LIMIT_SECOND= 0,

	// EVT_GRADEUP
	EA_GRADEUP_1_GRADE   = 0,
	EA_GRADEUP_1_PESO    = 1,
	EA_GRADEUP_2_GRADE   = 2,
	EA_GRADEUP_2_PESO    = 3,
	EA_GRADEUP_3_GRADE   = 4,
	EA_GRADEUP_3_PESO    = 5,
	EA_GRADEUP_4_GRADE   = 6,
	EA_GRADEUP_4_PESO    = 7,

	// EVT_PCROOM_BONUS
	EA_PCROOM_BONUS      = 0,

	EA_PCROOM_PESO_BONUS_PER		   = 0,
	EA_PCROOM_EXP_BONUS_PER			   = 1,
	EA_PCROOM_FIRST_EVENT_WEEK_ON_OFF  = 2,
	EA_PCROOM_FIRST_START_TIME         = 3,
	EA_PCROOM_FIRST_END_TIME           = 4,
	EA_PCROOM_SECOND_EVENT_WEEK_ON_OFF = 5,
	EA_PCROOM_SECOND_START_TIME        = 6,
	EA_PCROOM_SECOND_END_TIME          = 7,

	// EVT_CHANCE_MORTMAIN_CHAR
	EA_CHANCE_MORTMAIN_CHAR_TIME = 0,

	// EVT_ONE_DAY_GOLD_ITEM
	EA_GOLD_ITEM_PRESENT_LIMIT = 0,
	EA_GOLD_ITEM_PRESENT_ALARM = 1,
	EA_GOLD_ITEM_INIT_DATE_HOUR= 2,
	EA_GOLD_ITEM_PRESENT_MENT  = 3,

	// EVT_DORMANCY_USER
	EA_DORMANCY_USER_LIMIT_DATE		= 0,
	EA_DORMANCY_USER_PRESENT_LIMIT	= 1,
	EA_DORMANCY_USER_PRESENT_ALARM	= 2,
	EA_DORMANCY_USER_PRESENT_MENT	= 3,

	// EVT_PLAYTIME_PRESENT
	EA_PLAYTIME_PRESENT_TIME        = 0,

	// EVT_CHRISTMAS
	EA_CHRISTMAS_IS_ONLY_ONE_GIFT   = 0, // 1:�̸� �̺�Ʈ ���� �ѹ��� ����, 0:�̸� �Ϸ翡 1�� ����

	// EVT_BUY_ITEM
	EA_BUY_TYPE                     = 0, // 0:[���]���Žü�������, 1:[ĳ��]���Žü�������, 2:[���+ĳ��]���Žü�������

	// EVT_FISHING
	EA_FS_FIRST_EVENT_WEEK_ON_OFF  = 1,
	EA_FS_FIRST_START_TIME         = 2,
	EA_FS_FIRST_END_TIME           = 3,
	EA_FS_SECOND_EVENT_WEEK_ON_OFF = 4,
	EA_FS_SECOND_START_TIME        = 5,
	EA_FS_SECOND_END_TIME          = 6,

	// EVT_CONNECTION_TIME
	EA_CT_CHANCE_TIME              = 0, // ���� ���� ��� �ð� ( �� )
	EA_CT_PRESENT_LOW_USER_LEVEL   = 1, // ���� ���� ������ ���

	// EVT_ONE_DAY_GIFT , EVT_ONE_DAY_GIFT_2
	EA_OD_IS_EVERYDAY_GIFT         = 0, // 1�̸� ���� ������ ���� , 0�̸� 1���� ������ ����

	// EVT_GRADEUP_PRESENT
	EA_GP_GRADE_OF_PRESENT         = 0, 
	EA_GP_START_ENTRY_DATE         = 1,
	EA_GP_END_ENTRY_DATE           = 2,

	// EVT_CONNECTION_TIME_SELECT_GIFT
	EA_CTSG_CHANCE_TIME            = 0, // ���� ���� ��� �ð� ( �� )

	// EVT_ENTRY
	EA_E_START_ENTRY_DATE          = 0,
	EA_E_END_ENTRY_DATE            = 1,

	// EVT_ENTRY_AFTER
	EA_EA_GIFT_ENTRY_DATE_AFTER    = 0,

	// EVT_CONNECT_AND_PLAYTIME
	EA_CAP_MAX_POINT   = 0,
	EA_CAP_MAX_SEC     = 1,

	// EVT_MODE_BONUS & EVT_MODE_BONUS2
	EA_MB_EXP_PER						= 0,
	EA_MB_EXP_FIRST_EVENT_WEEK_ON_OFF	= 1,
	EA_MB_EXP_FIRST_START_TIME			= 2,
	EA_MB_EXP_FIRST_END_TIME			= 3,
	EA_MB_EXP_SECOND_EVENT_WEEK_ON_OFF	= 4,
	EA_MB_EXP_SECOND_START_TIME			= 5,
	EA_MB_EXP_SECOND_END_TIME			= 6,
	EA_MB_LIST1							= 7,
	EA_MB_LIST2							= 8,

	// EVT_MONSTER_DUNGEON_ADD_CARD, EVT_PCROOM_MONSTER_DUNGEON_ADD_CARD
	EA_MD_ADD_CARD_COUNT				= 0,
	EA_MD_FIRST_EVENT_WEEK_ON_OFF		= 1,
	EA_MD_FIRST_START_TIME				= 2,
	EA_MD_FIRST_END_TIME				= 3,
	EA_MD_SECOND_EVENT_WEEK_ON_OFF		= 4,
	EA_MD_SECOND_START_TIME				= 5,
	EA_MD_SECOND_END_TIME				= 6,

	//EVT_PRESENT_BUY
	EA_PB_FIRST_EVENT_WEEK_ON_OFF		= 0,
	EA_PB_FIRST_START_TIME				= 1,
	EA_PB_FIRST_END_TIME				= 2,
	EA_PB_SECOND_EVENT_WEEK_ON_OFF		= 3,
	EA_PB_SECOND_START_TIME				= 4,
	EA_PB_SECOND_END_TIME				= 5,
};

class EventNode
{
protected:
	WORD m_wStartYear;
	WORD m_wStartMonth;
	WORD m_wStartDate;
	WORD m_wStartHour;

	WORD m_wEndYear;
	WORD m_wEndMonth;
	WORD m_wEndDate;
	WORD m_wEndHour;

	bool      m_bAlive;
	EventType m_eEventType;
	IntVec    m_ValueVec;
	ModeCategory m_eModeCategory;

	typedef std::vector< ChannelingType > vChannelingTypeVec;
	vChannelingTypeVec m_vUseChannelingTypeVec;

	bool IsCheckAlive( SYSTEMTIME &rSt );

	void SetValue( int iArray, int iValue );
	void SetUseChannelingType( int iArray, ChannelingType eUserChannelingType );

public:
	virtual void LoadINI( ioINILoader &a_rkLoader, bool bCreateLoad );
	virtual void CheckAlive( SYSTEMTIME &rSt, bool bSend = true);

	void Update(int iValues[], int iValueCount);

	EventType GetType() const;

	int GetValue( int iArray );
	int GetValueSize() const;
	bool IsAlive()	{ return m_bAlive; }

	int GetUseChannelingTypeSize() const;
	ChannelingType GetUseChannelingType( int iArray );
	bool IsAlive( ChannelingType eUserChannelingType );

	int GetStartDate();
	int GetEndDate();
	virtual	bool IsEventTime(){ return false; }

	inline int GetModeCategory() { return m_eModeCategory; }

public:
	EventNode();
	virtual ~EventNode();
};

class ioPlazaMonsterEventNode : public EventNode
{
public:
	ioPlazaMonsterEventNode();
	virtual ~ioPlazaMonsterEventNode();

	struct GROUP_DAY
	{
		int			iTotal;
		vector<int>	vecDays;
		int			iStartHour;
		int			iEndHour;
	};

private:
	vector< GROUP_DAY >	m_vecGroupDay;

public:
	virtual void LoadINI( ioINILoader &a_rkLoader, bool bCreateLoad );
	virtual	bool IsEventTime();
};

class PresentBuyEventNode : public EventNode
{
public:
	PresentBuyEventNode();
	virtual	~PresentBuyEventNode();

public:
	virtual void LoadINI( ioINILoader &a_rkLoader, bool bCreateLoad );
	virtual void CheckAlive( SYSTEMTIME &rSt, bool bSend = true);

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray );

public:
	bool IsEventTime(SYSTEMTIME& st);
	bool IsEventItem(const int iType, const int iCode);

protected:
	typedef std::vector<int> ALLOWEDITEMTYPE;
	typedef std::vector<DWORD> ITEMCODE;

	//typedef boost::unordered_map<int, ITEMCODE> EXCEPTIONITEM;		// <type, 
	typedef boost::unordered_map<int, ITEMCODE> EVENTITEM;		// <type, 

	EVENTITEM		m_mExceptionItemInfo;		//�ش� �̺�Ʈ ����� ���ܵ� ������ ����.
	EVENTITEM		m_mAllowedItemInfo;
	ALLOWEDITEMTYPE	m_vAllowedItemType;
};
//-----------------------------------------------------------------------------------------------------------------------------------
class ioEventManager : public Singleton< ioEventManager >
{
protected:
	typedef std::vector< EventNode* > EventNodeVec;
	int          m_iMaxDBValue;
	EventNodeVec m_EventNodeVec;
	DWORD        m_dwCurrentTime;
	DWORD        m_dwCurrentDate; // YYYYMMDD
	bool		 m_bMileageShopOpen;
	int			 m_iMileageRatio;
	int			 m_iPcRoomMileageRatio;
	int			 m_iLosaMileageRatio;

	CTime  m_cLosaStartTime ;
	CTime  m_cLosaEndTime ;

	

	bool IsExist( EventType eEventType );
	void Clear();

public: 
	void LoadINI( bool bCreateLoad = true );
	void ProcessTime();
	void CheckAlive( bool bSend = true);
	void Update(int iValues[], int iValueCount);

	int GetSize() const;
	int GetMaxDBValue() const { return m_iMaxDBValue; }

	EventType GetType( int iArray );
	ModeCategory GetModeCategory( int iArray );

	bool IsAlive( EventType eEventType, ChannelingType eChannelingType, ModeCategory eModeCategory = MC_DEFAULT );
	int GetValue( EventType eEventType, int iArray, ModeCategory eModeCategory = MC_DEFAULT );

	int GetValueSize( EventType eEventType );

	ChannelingType GetUseChannelingType( EventType eEventType, int iArray );
	int GetUseChannelingTypeSize( EventType eEventType );

	int GetStartDate( EventType eEventType );
	int GetEndDate( EventType eEventType );
	bool IsEventTime( const EventType eEventType );

	DWORD GetCurrentDate() const { return m_dwCurrentDate; }

	bool isMileageShopOpen() const { return m_bMileageShopOpen; }
	int GetMileageRatio() const { return m_iMileageRatio; }
	int GetPcRoomMileageRatio() const { return m_iPcRoomMileageRatio; }
	int GetLosaMileageRatio() const { return m_iLosaMileageRatio; }

	CTime GetLosaStartDate() const { return m_cLosaStartTime; }
	CTime GetLosaEndDate() const { return m_cLosaEndTime; }

	EventNode *GetNode( EventType eEventType, int iModeCategory = 0 );

	void NotifySpecificEventInfoWhenLogin(User* pUser);

public:
	static ioEventManager& GetSingleton();

public:
	ioEventManager(void);
	virtual ~ioEventManager(void);
};

#define g_EventMgr ioEventManager::GetSingleton()

#endif // __ioEventManager_h__

