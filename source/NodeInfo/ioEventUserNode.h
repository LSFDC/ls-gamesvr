#ifndef __ioEventUserNode_h__
#define __ioEventUserNode_h__

#include "ioEventManager.h"

class User;
class SP2Packet;
class ioPlayStage;

// user�� ������ �ִ� event
class EventUserNode
{
protected:
	DWORD     m_dwIndex;
	IntVec    m_ValueVec;
	IntVec    m_BackupVec;
	bool      m_bMustSave;
	EventType m_eEventType;
	ModeCategory m_eModeCategory;


protected:
	bool IsChange();
	void Clear();

public:
	int  GetSize() const;
	void SetSize(int iSize );
	int  GetValue( int iArray );
	void SetValue( int iArray, int iValue );

	virtual EventType GetType() const;
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void UpdateFirst( User *pUser );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void Process( User *pUser );      // �����÷��� üũ
	virtual void ProcessTime( User *pUser );  // ���ӽð� üũ
	virtual void SendData( User *pUser );

	virtual void FillMoveData( SP2Packet &rkPacket, bool bLog );
	virtual void ApplyMoveData( SP2Packet &rkPacket );
	virtual int GetFillMoveDataSize();
	virtual int GetAddSize();
	virtual bool IsEmptyValue();

	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

	void Backup();
	void SetIndex( DWORD dwIndex );
	DWORD GetIndex() const { return m_dwIndex; }

	void SetType( EventType eEventType );

	inline int GetModeCategory() { return m_eModeCategory; }
	inline void SetModeCategory( ModeCategory eModeCategory ) { m_eModeCategory = eModeCategory; }

public:
	EventUserNode();
	virtual ~EventUserNode();
};

//----------------------------------------------------------------------------------------------------------------------------------
// class �߰��ÿ� CreatEventUserNode() ���� �߰��� ��
enum
{
	// EVT_PROPOSAL 
	  // vec array
	VA_PROPOSAL_CNT       = 0, // from db
	VA_GAVE_CHAR          = 1, // from db
	VA_ADD_PROPOSAL_CNT   = 2, // from memory
	VA_ADD_GAVE_CHAR_CNT  = 3, // from memory
	  // add size
	ADD_PROPOSAL_SIZE     = 2, // from memory ������ 2�� �߰���
	
	// EVT_COIN
	  // vec array
	VA_COIN_CNT           = 0, // from db
	VA_PLAY_SEC           = 1, // from db
	VA_ADD_COIN_CNT       = 2, // from memory
	  // add size
	ADD_COIN_SIZE         = 1, // from memory ������ 1�� �߰���

	// EVT_PLAYTIME
	  // vec array
	VA_PLAYTIME_PLAY_SEC  = 0, // from db
	VA_PLAYTIME_GET_GIFT  = 1, // from db    11111 : 0�̸� ������ 1�̸� ���� 10000�̸� 5��° ������ �����ϰ� �������� ������

	// EVT_CHANCE_MORTMAIN_CHAR
	  // vec array
    VA_CMC_MAGIC_CODE     = 0, // from db   >= 0�̸� �÷����� �ð��̰� -1�� �Ϲ� ���� -2�� �Ǿ��� ���� -3�� �Ϲ�/�Ǿ��� ���� 2�� ����
	VA_CMC_MORTMAIN_CNT   = 1, // from db   ȹ���� �����뺴 ī��Ʈ

	//EVT_ONE_DAY_GOLD_ITEM
	  // vec array
	 VA_GOLD_ITEM_RECV_DATE = 0, // from db   >= ���������� ���� ��:�� ����

    //EVT_PLAYTIME_PRESENT
	  // vec array
    VA_PLAYTIME_PRESENT_TIME_CNT = 0, // from db   >= 0�̸� �÷����� �ð��̰� -1�� ������ ���� �� �ִ� 1���� ��ȸ

	//EVT_CHRISTMAS
	  // vec array
    VA_CHRISTMAS_GET_GIFT_DATE   = 0, // from db   ������ ���� ��¥

	//EVT_CONNECTION_TIME
	  // vec array
    VA_CT_GET_CHANCE_DATE = 0, // from db ������ ���� ��¥
	VA_CT_IS_CHANCE       = 1, // form db 1�̸� ������ �ְ� 0�̸� ������ ����.

	//EVT_ONE_DAY_GIFT
	  // vec array
    VA_OG_GET_GIFT_DATE   = 0, // from db ������ ���� ��¥�� ���� ��) 20100830

	//EVT_GRADEUP_PRESENT
	  // vec array
    VA_GP_CAN_RECEIVE_GIFT = 0, // from memory 1�̸� ������ ���� �� �ְ� 0�̸� ������ �� �޴´�.

	//EVT_CONNECTION_TIME_SELECT_GIFT
	  // vec array
    VA_CTSG_GET_CHANCE_DATE = 0, // from db ������ ���� ��¥
	VA_CTSG_IS_CHANCE       = 1, // form db 1�̸� ������ �ְ� 0�̸� ������ ����.

	//EVT_ENTRY
	 // vec array
	VA_E_GET_GIFT           = 0, // from db 1:�̸� ������ �޾Ҵ�, 0:�̸� ������ ���� �ʾҴ�.
	                             // 1�� array DB�� ���̹Ƿ� �޸𸮰��� 2������ ����
	VA_E_CAN_RECEIVE_GIFT   = 2, // from memory 1:�̸� ������ ���� �����̴�. 0:�̸� ������ ���� ������ �ȵȴ�.
	  // add size
	ADD_ENTRY_SIZE          = 1, // from memory ������ 1�� �߰���

	//EVT_ENTRY_AFTER
	  // vec array
	VA_EA_GET_GIFT          = 0, // from db 1:�̸� ������ �޾Ҵ�, 0:�̸� ������ ���� �ʾҴ�.
	// 1�� array DB�� ���̹Ƿ� �޸𸮰��� 2������ ����
	VA_EA_CAN_RECEIVE_GIFT  = 2, // from memory 1:�̸� ������ ���� �����̴�. 0:�̸� ������ ���� ������ �ȵȴ�.
	  // add size
	ADD_ENTRY_AFTER_SIZE    = 1, // from memory ������ 1�� �߰���

	//EVT_CONNECT_AND_PLAYTIME
		// vec array
	VA_CAP_CONNECT_RESET_DATE = 0, // from db [0000]0000 : ���� ����Ʈ�� ���� ����,    0000[0000] : ����Ʈ�� ���µ� ����.
	VA_CAP_POINT_AND_SEC      = 1, // from db [0000]0000 : �������ӽð����� ��������Ʈ, 0000[0000] : ���� ���� ��
};

class ProposalEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual int GetAddSize();

public:
	bool IsGiveChar( const User *pUser );
	void SetValueGiveChar();

public:
	ProposalEventUserNode();
	virtual ~ProposalEventUserNode();
};

//-------------------------------------------------------------------------------------------------------------------------------
class CoinEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void Process( User *pUser );
	virtual int GetAddSize();

public:
	void SetGradeUpCoin( User *pUser );

public:
	CoinEventUserNode();
	virtual ~CoinEventUserNode();
};

//---------------------------------------------------------------------------------------------------------------------------------
class ExpEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory );

public:
	bool IsEventTime( const User *pUser, ModeCategory eModeCategory );
	float GetEventPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory = MC_DEFAULT );

public:
	ExpEventUserNode();
	virtual ~ExpEventUserNode();
};

//--------------------------------------------------------------------------------------------------------------------------------
class PesoEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory );

public:
	bool IsPesoTime( const User *pUser, ModeCategory eModeCategory );
	float GetPesoPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory = MC_DEFAULT );

public:
	PesoEventUserNode();
	virtual ~PesoEventUserNode();
};

//--------------------------------------------------------------------------------------------------------------------------------
class FishingEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray );

public:
	bool IsEventTime( const User *pUser );

public:
	FishingEventUserNode();
	virtual ~FishingEventUserNode();
};

//-------------------------------------------------------------------------------------------------------------------------------
// VA_PLAYTIME_GET_GIFT : XXXXX �ڸ��� ����ϸ� 1�ڸ� ���� gift 1�� ������(10->2, 100->3, 1000->4, 10000->5) , 11111�� 
// �ȳ� �޼����� ���� ���� ��Ÿ��, 22222�� ��ǰ�� ���� ���� ���� ��Ÿ��, 00112�� gift 1�� ����߰� 2~3���� �ȳ� �޼����� ���´ٴ� ǥ��
class PlayTimeEventUserNode : public EventUserNode
{
public:
	enum GiftType 
	{
		GT_1_PESO    = 10001,
		GT_2_PESO    = 10002,
		GT_3_SOLDIER = 10003,
		GT_4_SOLDIER = 10004,
		GT_5_SOLDIER = 10005, 
	};

	enum GiftState
	{
		GS_NONE   = 0,
		GS_NOTICE = 1,
		GS_USED   = 2,
	};
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void Process( User *pUser );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	void SetGift( GiftType eType, User *pUser, bool bPeso );
	bool IsGift( GiftType eType, User *pUser, bool bPeso );
	int  GetLimitSecond( GiftType eType );
	virtual void SendData( User *pUser );
	
protected:
	bool IsGiftData( GiftType eType, GiftState eGiftState );
	void SetGiftData( GiftType eGiftType , GiftState eGiftState );

public:
	PlayTimeEventUserNode();
	virtual ~PlayTimeEventUserNode();
};

//-------------------------------------------------------------------------------------------------------------------------------
// ��̳� �̺�Ʈ ���
class ChildrenDayEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	ChildrenDayEventUserNode();
	virtual ~ChildrenDayEventUserNode();
};

//-------------------------------------------------------------------------------------------------------------------------------
// ���� �뺴 �̺�Ʈ ���
class PesoBonusEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	void SetPesoBonus( User *pUser );

public:
	PesoBonusEventUserNode();
	virtual ~PesoBonusEventUserNode();
};

//---------------------------------------------------------------------------------------------------------------------------------
// 2�ð� �뺴 �������� ���� �����ϴ� �̺�Ʈ
class BuyCharNoLevelLimitEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	bool IsNoLevelLimit( const User *pUser, bool bCharCreate, int iMsgType, int iCharLimitSecond, int iCharPeriodType );

public:
	BuyCharNoLevelLimitEventUserNode();
	virtual ~BuyCharNoLevelLimitEventUserNode();
};

//----------------------------------------------------------------------------------------------------------------------------------
// �뺴�� �������� ��� �����ϴ� �̺�Ʈ
class GradeUpEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	void SetGift( User *pUser, int iGradeLevel );

public:
	GradeUpEventUserNode();
	virtual ~GradeUpEventUserNode();
};

//-------------------------------------------------------------------------------------------------------------------------------
// �Ǿ��� ���ʽ� �̺�Ʈ ���
class PCRoomEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeType );

public:
	bool IsEventTime( const User *pUser, ModeCategory eModeType );
	void SetPesoAndExpBonus( const User *pUser, float& fPesoBonus, float& fExpBonus, ModeCategory eModeType );

public:
	PCRoomEventUserNode();
	virtual ~PCRoomEventUserNode();
};
//-------------------------------------------------------------------------------------------------------------------------------
class ChanceMortmainCharEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	void UpdatePlayTime( User *pUser, DWORD dwPlayTime );

public:
	ChanceMortmainCharEventUserNode();
	virtual ~ChanceMortmainCharEventUserNode();
};
//-------------------------------------------------------------------------------------------------------------------------------
class OneDayGoldItemEvent : public EventUserNode
{

public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	int GetEventCurrentDate();
	void CheckGoldItemDate( User *pUser );

public:
	OneDayGoldItemEvent();
	virtual ~OneDayGoldItemEvent();
};
//-------------------------------------------------------------------------------------------------------------------------------
class DormancyUserEvent : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	bool CheckDormancyDateToPresent( User *pUser, CTime &rkEntryTime, CTime &rkConnectTime );

public:
	DormancyUserEvent();
	virtual ~DormancyUserEvent();
};
//----------------------------------------------------------------------------------------------------------------------------------
class PlayTimePresentEventUserNode : public EventUserNode
{
public:
	enum 
	{
		CHANCE_GET_PRESENT = -1,
	};
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	void UpdatePlayTime( User *pUser, DWORD dwPlayTime );

public:
	PlayTimePresentEventUserNode();
	virtual ~PlayTimePresentEventUserNode();
};
//-----------------------------------------------------------------------------------------------------------------------------------
class ChristmasEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	ChristmasEventUserNode();
	virtual ~ChristmasEventUserNode();
};
//---------------------------------------------------------------------------------------------------------------------------------
class BuyItemEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	void SendBuyPresent( User *pUser, bool bPeso, short iBuyType, int iBuyValue1, int iBuyValue2, int iBonusMilesage = 0 );
	bool InsertUserPresentByBuyPresent( User *pUser, bool bPeso, DWORD dwRecvUserIndex, short iPresentType, int iBuyValue1, int iBuyValue2, bool bPresentEvent = false );

public:
	BuyItemEventUserNode();
	virtual ~BuyItemEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class ExerciseSoldierEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	ExerciseSoldierEventUserNode();
	virtual ~ExerciseSoldierEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class ConnectionTimeEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void UpdateFirst( User *pUser );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

protected:
	int GetNextCheckTime();
	int GetCheckTime( int iMonth, int iDays, int iHours, int iMins );

public:
	ConnectionTimeEventUserNode();
	virtual ~ConnectionTimeEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class OneDayGiftEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

public:
	OneDayGiftEventUserNode();
	virtual ~OneDayGiftEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class GradePresentEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );

public:
	void SetCanReceiveGift( CTime &rkEntryTime );
	void SetGift( User *pUser, int iGradeLevel );

public:
	GradePresentEventUserNode();
	virtual ~GradePresentEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class ConnectionTimeSelectGiftEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void UpdateFirst( User *pUser );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

protected:
	int GetNextCheckTime();
	int GetCheckTime( int iMonth, int iDays, int iHours, int iMins );

public:
	ConnectionTimeSelectGiftEventUserNode();
	virtual ~ConnectionTimeSelectGiftEventUserNode();
};
//-----------------------------------------------------------------------------------------------------------------------------------
class EntryEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual int GetAddSize();

public:
	void SetCanReceiveGift( CTime &rkEntryTime );
	void SetGift( User *pUser );

public:
	EntryEventUserNode();
	virtual ~EntryEventUserNode();
};
//---------------------------------------------------------------------------------------------------------------------------------------
class LadderPointEventUserNode : public ExpEventUserNode 
{
public:
	LadderPointEventUserNode();
	virtual ~LadderPointEventUserNode();
};
//-----------------------------------------------------------------------------------------------------------------------------------
class EntryAfterEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual int GetAddSize();

public:
	void SetCanReceiveGift( CTime &rkEntryTime, int iUserState );
	void SetGift( User *pUser );

public:
	EntryAfterEventUserNode();
	virtual ~EntryAfterEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class ConnectAndPlayTimeEventUserNode : public EventUserNode
{
public:
	virtual void Init();
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void ProcessTime( User *pUser );
	virtual void OnRecievePacket( User *pUser, SP2Packet &rkPacket );

protected:
	int GetPoint();
	int GetSec();
	int GetPointAndSec( int iPoint, int iSec );

	int GetConnectDate();
	int GetResetDate();
	int GetConnectResetDate( int iConnectDate, int iResetDate );

	int GetCurrentDate();

public:
	ConnectAndPlayTimeEventUserNode();
	virtual ~ConnectAndPlayTimeEventUserNode();
};
//----------------------------------------------------------------------------------------------------------------------------------
class PlazaMonsterEventUserNode : public EventUserNode
{
public:
	PlazaMonsterEventUserNode();
	virtual	~PlazaMonsterEventUserNode();

public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

public:
	bool IsEventTime( User* pUser );
};
//----------------------------------------------------------------------------------------------------------------------------------
class ModeBonusEventUserNode : public EventUserNode
{
public:
	ModeBonusEventUserNode();
	virtual	~ModeBonusEventUserNode();

public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();

protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray, ModeCategory eModeCategory );

public:
	bool IsEventTime( const User *pUser, ModeCategory eModeCategory );
	float GetEventPer( float fPCRoomBonus, const User *pUser, ModeCategory eModeCategory = MC_DEFAULT );
	bool IsEventMode( const int iMode, ModeCategory eModeCategory = MC_DEFAULT );
};

//---------------------------------------------------------------------------------------------------------------------------------
class MonsterDungeonEventUserNode : public EventUserNode
{
protected:
	bool IsEventWeek( WORD wDayOfWeek, int iValueArray );

public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual bool IsEmptyValue();


public:
	bool IsEventTime( const User *pUser );
	int GetAddCount( const User *pUser );

public:
	MonsterDungeonEventUserNode();
	virtual ~MonsterDungeonEventUserNode();
};

//----------------------------------------------------------------------------------------------------------------------------------


class FreeDayEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void ProcessTime( User *pUser );

public:
	FreeDayEventUserNode();
	virtual ~FreeDayEventUserNode();
};

//----------------------------------------------------------------------------------------------------------------------------------


class HeroExpBoostEventUserNode : public EventUserNode
{
public:
	virtual void Save( const DWORD dwAgentID, const DWORD dwThreadID );
	virtual void InsertDB( const DWORD dwAgentID, const DWORD dwThreadID, const char *szUserGUID, DWORD dwUserIndex );
	virtual void ProcessTime( User *pUser );

public:
	HeroExpBoostEventUserNode();
	virtual ~HeroExpBoostEventUserNode();
};


#endif // __ioEventUserNode_h__