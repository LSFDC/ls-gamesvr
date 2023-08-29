// DBClient.h: interface for the DBClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGDBCLIENT_H__8BBCE6AF_B8A6_4D7C_A9FA_B2B4B32491CA__INCLUDED_)
#define AFX_LOGDBCLIENT_H__8BBCE6AF_B8A6_4D7C_A9FA_B2B4B32491CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../QueryData/QueryData.h"
#include "../Nodeinfo/NodeHelpStructDefine.h"

//DB AGENT MSG TYPE
// GET : Select , SET : Insert , DEL : Delete , UPD : Update

#define LOGDBAGENT_SET          	0x0001 // return ���� ������ �Ѱ��� ������
#define DBAGENT_LOG_PINGPONG		0x1000

//�۾� ���
#define _INSERTDB       0
#define _DELETEDB       1
#define _SELECTDB       2
#define _UPDATEDB       3   
#define _SELECTEX1DB    4 

//��� �ൿ
#define _RESULT_CHECK   0
#define _RESULT_NAUGHT  1
#define _RESULT_DESTROY 2

struct ModeRecord;
class User;
class Room;
class CConnectNode;

class LogDBClient : public CConnectNode 
{
private:
	static LogDBClient *sg_Instance;
	DWORD m_dwCurrentTime;
	int   m_iClassPriceTime;
	static int m_iUserCntSendDelay;

protected:
	std::vector<std::string> m_vServerIP;
	std::vector<int> m_vServerPort;

	ioHashString m_DBAgentIP;
	int          m_iDBAgentPort;

public:
	static LogDBClient &GetInstance();
	static void ReleaseInstance();

private:
	ValueType GetValueType(VariableType nType,int len);

public:
	inline ioHashString &GetDBAgentIP(){ return m_DBAgentIP; }
	inline int GetDBAgentPort(){ return m_iDBAgentPort; }

	void SetUserCntSendDelay(int iDelay) { m_iUserCntSendDelay = iDelay; }
	int GetUserCntSendDelay() { return m_iUserCntSendDelay; }

public:
	virtual void SessionClose( BOOL safely=TRUE );
	virtual bool SendMessage( CPacket &rkPacket );
	virtual void ReceivePacket( CPacket &packet );
	virtual void PacketParsing( CPacket &packet );

public:
	virtual void OnCreate();       //�ʱ�ȭ
	virtual void OnDestroy();
	virtual bool CheckNS( CPacket &rkPacket );	
	virtual int  GetConnectType();

public:
	bool ConnectTo();

	void GenerateLOGDBAgentInfo();
	void SetLOGDBAgentInfo(std::vector<std::string>& vServerIP, std::vector<int>& vServerPort );

public:
	void ProcessTime();
	void ProcessPing();
	void ProcessFlush();

	void OnPing();

// insert
public:
	// OnInsert �Լ����� *���ڷ� ���� ���� ������������. �˼����� ���� ������ �� �� ����.
	enum PlayResultType 
	{
		PRT_END_SET   = 1,
		PRT_EXIT_ROOM = 2,
	};
	enum GameType
	{
		GT_NONE              = 0, // ������ ����ߴ� ���� �־� ���������� �������� ����.
		GT_SQUARE            = 3, 
		GT_BATTLE            = 4, 
		GT_LADDER            = 5,
		GT_SHUFFLE           = 6,
	};
	void OnInsertPlayResult( ModeRecord *pRecord, PlayResultType ePlayResultType );
	void OnInsertCharacterResult( ModeRecord *pRecord, int iArray );
	
	enum PesoType 
	{
		PT_TUTORIAL			= 1,
		PT_LEVELUP			= 2,
		PT_BANKRUPT			= 3,
		PT_EXIT_ROOM		= 4,
		PT_BUY_CASH			= 5,
		PT_DEL_CHAR			= 6,
		PT_GROWTH			= 7,
		PT_MGAME_OPEN		= 8,
		PT_LADDER_BONUS		= 9,	// ��������� DB���� �־��ִ� �� ( ���Ӽ��������� ������� ���� DB���� �־��� )
		PT_EVENT_PESO		= 10,	// �̺�Ʈ�� ��� ȹ��
		PT_FISH_ITEM		= 11,
		PT_SELL_PRESENT		= 12,
		PT_RECV_PRESENT		= 13,
		PT_SELL_EXTRAITEM	= 14,
		PT_PACKAGE_DECO		= 15,	// ��Ű�� ���������� ���Ž� �������� ġ�� ����
		PT_SELL_ETCITEM		= 16,	// ���� ������ �Ǹ�
		PT_GROWTH_DOWN		= 17,	// ���� ������ ȸ�� ���
		PT_EXCAVATION_ITEM	= 18,  
		PT_ALCHEMIC_PESO	= 19,	// ���տ��� ��ҷ� ��ȯ
		PT_GROWTH_ALL_DOWN  = 20,   // ��� ���� ������ ȸ�� ���
		PT_SELL_MEDALITEM   = 21,	// �޴� ���ȱ�
		PT_SELL_PET			= 22,	// �� �ȱ�
		PT_SELL_COSTUME		= 23,	// �ڽ�Ƭ �ȱ�
		PT_BATTLE			= 24,	// ���� ����( ��Ż, ����� )
		PT_TRADE			= 25,	// �ŷ��� ����
		PT_TRADE_TEX		= 26,	// �ŷ��� ������
		PT_GUILD_LEAVE		= 27,	// ��� Ż��
		PT_GUILD_KICKOUT	= 28,	// ��� �߹�
		PT_BUY_CHAR			= 29,	
		PT_BUY_DECO			= 30,	
		PT_BUY_ETCITEM		= 31,	
		PT_BUY_EXTRAITEM	= 32,
		PT_BUY_COSTUME		= 33,	
		PT_DEL_CHEAT_PESO	= 34,	// ġƮ�� ȹ�� ��� ����
		PT_SELL_ACCESSORY	= 34,	// �Ǽ��縮 �ȱ�
	};
	void OnInsertPeso( User *pUser, int iPeso, PesoType ePesoType );

	enum TimeType
	{
		TT_ENTER_ROOM   = 1, // ���ο��� ������ �̵��� ���νð�
		TT_EXIT_PROGRAM = 2, // ���ο��� ���� ����� ���νð�
		TT_MOVE_SERVER  = 3, // ���ο��� ���� �̵��� ���νð� �� �뿡�� ������ �̵��� ���� �ð�
		// 4~10�������� ���� ��ȣ��
		TT_VIEW         = 11,
		TT_WAIT_NEXT    = 12,
		TT_WAIT_LADDER_NEXT = 13,
		TT_VIEW_LADDER      = 14,
		TT_FISHING		= 15,
		TT_EXCAVATING   = 16,
		TT_FIGHTMODE_VIEW = 17,
	};
	void OnInsertTime( User *pUser, TimeType eTimeType );

	enum CharType
	{
		CT_BUY     = 1,
		CT_LEVELUP = 2,
		CT_PCROOM  = 3,
		CT_DEL     = 4,
		CT_TUTORIAL= 5,
		CT_BANKRUPT= 6,
		CT_EVENT_PROPOSAL = 7,
		CT_EVENT_PLAYTIME = 8,
		CT_EVENT_CHAR = 9,      // �̺�Ʈ�� �뺴 ȹ��
		CT_PRESENT = 10,
		CT_PACKAGE = 11,
		CT_RENTAL  = 12,
		CT_DISASSEMBLE	= 13,
		CT_ALCHEMIC_SOLDIER	= 14,

		CT_CASH    = 1000000, // DB���� ������� �ʴ� Ÿ������ ���п����� ���
	};
	// �����뺴�� iLimitDate�� 0
	void OnInsertChar( User *pUser, int iClassType, int iLimitDate, int iPesoPrice, const char *szItemIndex, CharType eCharType );

	enum DecoType
	{
		DT_BUY     = 1,
		DT_DEL     = 2,
		DT_DEFAULT = 3,
		DT_PRESENT = 4,
		DT_PACKAGE = 5,
	};
	void OnInsertDeco( User *pUser, int iItemType, int iItemCode, int iPesoPrice, const char *szItemIndex, DecoType eDecoType );

	enum EtcType
	{
		ET_BUY = 1,
		ET_DEL = 2,
		ET_PRESENT = 3,
		ET_SELL = 4,
		ET_REFILL = 5,
		ET_DATE_DEL = 6,
		ET_APPLY    = 7,
		ET_USE		= 8,	// ��ø ������ ����
	};
	void OnInsertEtc( User *pUser, int iItemType, int iItemValue, int iPesoPrice, const char *szItemIndex, EtcType eEtcType );

	enum ExtraType
	{
		ERT_BUY        = 1,
		ERT_DEL        = 2,
		ERT_PRESENT    = 3,
		ERT_COMPOUND   = 4,
		ERT_EXCAVATION = 5,
		ERT_CUSTOM_ADD = 6,
		ERT_CUSTOM_DEL = 7,
		ERT_ALCHEMIC_ITEM	= 8,
		ERT_RECHARGE_ITEM   = 9,
	};
	void OnInsertExtraItem( User *pUser, int iItemCode, int iReinforce, int iMachineCode, int iLimitDays, int iPesoPrice, int iPeriodType, DWORD dwMaleCustom, DWORD dwFemaleCustom, const char *szItemIndex, ExtraType eExtraType );

	void OnInsertTutorialTime( User *pUser );

	enum CashItemType
	{
		CIT_CHAR    = 1,
		CIT_DECO    = 2,
		CIT_ETC     = 3,
		CIT_PESO    = 4,
		CIT_EXTRA   = 5,
		CIT_PRESENT = 6,
		CIT_POPUP	= 7,
	};
	// itemtype -> classType, decoType, etcType / itemvlaue -> limitDate, itemcode, item Ƚ���� ��
	void OnInsertCashItem( User *pUser, int iItemType, int iItemValue, int iCashPrice, const char *szItemIndex, CashItemType eCashItemType, ioHashString szSubscriptionID = "" );

	enum PresentType
	{
		PST_RECIEVE = 1,
		PST_DEL     = 2,
		PST_SELL    = 3,
		PST_TRADE   = 4,
		PST_EVENTSHOP = 5,
		PST_WEB_INSERT = 6,			//������ ���� ����
		PST_MASTER_INSERT = 7,		//�������������� ��������
		PST_ALCHEMIC	= 8,		//���տ����� ���� ����
		PST_CLOVERSHOP	= 9,		// Ŭ�ι���
		PST_MILEAGESHOP = 10,		// ���ϸ�����
	};
	void OnInsertPresent( DWORD dwSendUserIndex, const ioHashString &rszSendPublicID, const char *szSendPublicIP, DWORD dwRecievUserIndex, short iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, int iPresentValue4, PresentType ePresentType, const char *szNote );

	// Pet
	enum PetDataType
	{
		PDT_CREATE			= 1,
		PDT_DELETE			= 2,
		PDT_EQUIP			= 3,
		PDT_RANKUP			= 4,
		PDT_LEVELUP			= 5,
		PDT_NURTURE			= 6,
		PDT_COMPOUND		= 7,
	};
	void OnInsertPetLog( User *pUser, const int& iPetIndex, const int& iPetCode, const int& iPetRank, const int& iPetLevel, const int& iPetExp, const int& iCode, PetDataType ePetDataType, int iSubInfo = 0 );

	// subscription
	enum SubscriptionState
	{
		SST_RECIVE	= 1,
		SST_DELETE	= 2,
	};

	void OnInsertSubscription( User *pUser, int iIndex, short iPresentType, int iValue1, int iValue2, int iSubscriptionGold, ioHashString szSubscriptionID, DWORD dwLimitDate, SubscriptionState eStateType );

	enum MedalType
	{
		MT_PRESENT      = 1,
		MT_DEL          = 2,
	};
	void OnInsertMedalItem( User *pUser, int iItemType, int iPeriodType, const char *szItemIndex, MedalType eMedalType );

	enum ExMedalType
	{
		EMT_USE             = 1,
		EMT_DELETE_DATE     = 2,
		EMT_DELETE_GRADE_UP = 3,
	};
	void OnInsertExMedalSlot( User *pUser, int iClassType, BYTE iSlotNumber, int iLimitTime, ExMedalType eMedalType );

	enum QuestType
	{
		QT_PROGRESS = 1,		// ������ - ���� ����
		QT_ATTAIN   = 2,		// �޼� - ����Ʈ �޼������� ������ ���� �ʾ���
		QT_COMPLETE = 3,		// �Ϸ� - ���� �޾���.
		QT_REPEAT_REWARD = 4,   // ������ - ���� ���� - �������� �� �ݺ��ؼ� ���� ������.
	};
	void OnInsertQuest( User *pUser, DWORD dwMainIndex, DWORD dwSubIndex, QuestType eQuestType );

	enum TradeSysType
	{
		TST_REG		= 1,
		TST_BUY		= 2,
		TST_CANCEL	= 3,
		TST_TIMEOUT	= 4,
	};
	void OnInsertTrade( DWORD dwUserIndex,
						const ioHashString &rszPublicID,
						DWORD dwTradeIndex,
						DWORD dwItemType,
						DWORD dwMagicCode,
						DWORD dwValue,
						__int64 iItemPrice,
						TradeSysType eType,
						const char *szPublicIP,
						const char *szNote );

	enum RecordTypes
	{
		RCT_LOGIN = 0,
		RCT_PCROOM  = 1,
		RCT_LOGOUT = 2,
	};
	enum PCRoomSubType
	{
		PCST_FREE_PCROOM   = 0,
		PCST_CHARGE_PCROOM = 1,
	};
	//void OnInsertPCRoom( User *pUser, int iPlayTime,  PCRoomType ePCRoomType , PCRoomSubType ePCRoomSubType );
	void OnInsertRecordInfo( User *pUser, int iPlayTime,  RecordTypes eRecordType );


	void OnInsertPCInfo( User *pUser, const ioHashString &rsOS, const ioHashString &rsWebBrowser, const ioHashString &rsDXVersion, const ioHashString &rsCPU, const ioHashString &rsGPU
		                , int iMemory, const ioHashString &rsDesktopWH, const ioHashString &rsGameWH, bool bFullScreen );

	// alchemic
	// ��������
	enum AlchemicFuncType
	{
		AFT_SOLDIER		= 1,	// �뺴����
		AFT_EXTRAITEM	= 2,	// �������
		AFT_CHANGE		= 3,	// ������ȯ
		AFT_EXCHANGE	= 4,	// ���ú�ȯ
		AFT_PESO		= 5,	// ��Һ�ȯ
	};

	// ���Ÿ��
	enum AlchemicFuncResultType
	{
		AFRT_RECIPE_SUCC	= 1,
		AFRT_RANDOM_SUCC	= 2,
		AFRT_CHANGE_SUCC	= 3,
		AFRT_FAIL			= 4,
	};

	// ����Ÿ��
	enum DisassembleType
	{
		DST_SOLDIER		= 1,
		DST_EXTRAITEM	= 2,
	};

	// �ڽ�Ƭ Ÿ��
	enum CostumeType
	{
		COT_PRESENT = 1,
		COT_DEL     = 2,
	};

	// �α� ���� Ÿ��
	enum GameLogType
	{
		//�α���, �α� �ƿ�
		GLT_LOGIN					= 10003,
		GLT_LOGOUT					= 10004,
		GLT_PCROOM					= 10005,

		//�̼�
		GLT_UPDATE_MISSION			= 20001,
		GLT_COMPLETE_MISSION		= 20002,
		GLT_COMPLETE_COMPENSATION	= 20003,
		GLT_INIT					= 20004,

		//�⼮��
		GLT_ATTENDANCE				= 20005,

		//��庸��
		GLT_GUILD_ATTENDANCE		= 20006,
		GLT_GUILD_ATTENDANCE_REWARD	= 20007,
		GLT_GUILD_RANK_REWARD		= 20008,

		// Ȯ�� ��°�í
		GLT_RISING_GASHA			= 20009,

		GLT_ITEM_BUY				= 30001,
		GLT_POPUPSTORE				= 30002,

		//��� �帧
		GLT_PESO_GAIN				= 30003,
		GLT_PESO_CONSUME			= 30004,

		//��� �Һ�
		GLT_SPEND_CASH				= 30006,

		GLT_ITEM_REINFORCE			= 30007,
		//������ ��ȭ
	};

	enum LogEventType
	{
		LET_MODE	= 1,
		LET_ITEM	= 2,
		LET_ETC		= 3,
	};

	enum SpendCashType
	{
		SCT_REAL_CASH	= 1,
		SCT_BONUS_CASH	= 2,
	};


	// Ȯ�� ��� ��í Ÿ��
	enum RisingGashaType
	{
		RGT_BUY_PESO = 0,
		RGT_BUY_CASH,
	};
	// ����
	void OnInsertAlchemicFunc( User *pUser, short iFuncType, int iFuncCode, BYTE iResultType,
							   int iUseCnt1, int iUseCnt2, int iResult1, int iResult2 );
	// ����
	void OnInsertDisassemble( User *pUser, int iType, int iCode, int iResultCode, int iResultCnt );
	// ����ȹ��
	void OnInsertAddAlchemicPiece( int iUserIndex, const ioHashString &rszPublicID, int iLevel, int iPlayTime, BYTE iDifficulty, int iCnt );

	// Ŭ�ι�
	void OnInsertCloverInfo( const int iUserIndex, const int iFriendIndex, const BYTE byCloverType, const int iCount );

	// ����
	void OnInsertBingoChoiceNumber( const int iUserIndex, const ioHashString& szPublicID, const BYTE byChoiceType, const int iSelectNumber, const BYTE byState );

	enum EventCashType
	{
		ECT_USE = 1, 
	};

	void OnInsertEventCash( User *pUser, DWORD dwEtcItemType, int iAddCash, EventCashType eEventCashType );

	// ��ȭ
	enum MaterialCompoundResult
	{
		MCR_COMPOUND_SUCCESS	= 1,
		MCR_COMPOUND_FAIL	    = 2,
	};

	void OnInsertMaterialCompound( User *pUser, const int& iExtraItemCode, const int& iBeforeReinforce, const int& iNowReinforce, const int& iMaterialCode, MaterialCompoundResult eCompoundResult );

	// ����
	void OnInsertCharAwakeInfo( User *pUser, const int& iSoldierCode, const int& iMaterialCode, const short& iUseMaterialCount, BYTE chAwakeType );

	// �ڽ�Ƭ
	void OnInsertCostumeInfo( User *pUser, const int& iCostumeCode, int iCount, int iEventType );

	// ���� �α� ����
	enum OverseasLogType //�ؿ����� �α� Ÿ��
	{
		//�뺴 ���� �α�
		OVS_CAHNGE_CHAR = 1,
		OVS_PRESENT_CHAR = 2
	};
	void OnInsertGameLogInfo( int iLogType, User *pUser, int iTableIndex, int iCode, BYTE byEventType, int iParam1, int iParam2, int iParam3, int iParam4, char* szParam5 );

	// ���ʽ� ĳ��
	void OnInsertUsageOfBonusCash( User *pUser, int iBonusCashIndex, int iAmount, int iType, int iCode, int iValue, const ioHashString &szBillingGUID ,int iStatus);

private:
	void OnConConnect( const uint64 gameServerId, const ioHashString &szIP, const int iPort , const ioHashString &szServerName , const int iConConnect, ChannelingType eChannelingType );
	
// ���
private: 
	const ioHashString GetClassName(const int iClassType);
public:
	GameType GetGameType( ModeType eModeType, Room *pRoom );


private:			/* Singleton Class */
	LogDBClient( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize );
	virtual ~LogDBClient();
};
#define g_LogDBClient LogDBClient::GetInstance()
#endif // !defined(AFX_LOGDBCLIENT_H__8BBCE6AF_B8A6_4D7C_A9FA_B2B4B32491CA__INCLUDED_)
