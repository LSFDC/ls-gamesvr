#pragma once
 
#define MAX_INT64_TO_STR             10
#define WAIT_TIME_FOR_RESERVE_LOGOUT 180000 // 3�� 
#define DB_QUERY_CHECK_ERROR_MS      60000 // 60��

class ioItemInfoManager;
class ioItemPriceManager;
class ioSetItemInfoManager;
class ioEtcItemManager;
class ioAlchemicMgr;
class ioClientBind;
class ioServerBind;
class ioMonitoringBind;
class ioDecorationPrice;
class ioExerciseCharIndexManager;
class ioEventManager;
class ioPresentHelper;
class ioMyLevelMgr;
class ioMonsterMapLoadMgr;
class GrowthManager;
class FishingManager;
class NewPublicIDRefresher;
class ioChannelingNodeManager;
class ioExtraItemInfoManager;
class ioLocalManager;
class ioItemCompoundManager;
class ioSaleManager;
class ioExcavationManager;
class ioQuestManager;
class ioMedalItemInfoManager;
class LicenseManager;
class TradeInfoManager;
class ioModeINIManager;
class ioItemInitializeControl;
class ioExtraItemGrowthCatalystMgr;
class ioFirstSoldierManager;
class ioShutDownManager;
class LogicThread;
class SchedulerNode;
class CProcessor;
class TournamentManager;
class ioBingoManager;
class ioPirateRouletteManager;
class ioRelativeGradeManager;
class ioSuperGashaponMgr;
class ioItemRechargeManager;
class ioTestNodeManager;
class ioAttendanceRewardManager;
class ioPetInfoManager;
class ioPetGashaponManager;
class ioCharAwakeManager;
class ioPowerUpManager;
class CostumeManager;
class AccessoryManager;
class CostumeShopGoodsManager;
class SpecailGoodsManager;
class MissionManager;
class RollBookManager;
class GuildRewardManager;
class GuildRoomsBlockManager;
class ServerGuildInvenInfo;
class ioBlockPropertyManager;
class CompensationMgr;
class HomeModeBlockManager;
class TimeCashManager;
class TitleManager;
class TradeSyncManager;

class ioMainProcess : public CProcessor
{
	static ioMainProcess *sg_Instance;

public:
	static ioMainProcess &GetInstance();
	static void ReleaseInstance();

protected:
	DWORD m_dwLogCheckTime;

	// �� ���� ��ġ üũ��
	DWORD m_dwRoomToRoomCnt;
	DWORD m_dwEtcToRoomCnt;

private:
	ioHashString m_szINI;
	TCHAR m_szLogFolder[256];

	ioClientBind  *m_pClientBind;
	ioServerBind  *m_pServerBind;
	ioMonitoringBind *m_pMonitoringBind;

	ioHashString m_szPublicIP;
	ioHashString m_szClientMoveIP; // NAT ONLY
	ioHashString m_szPrivateIP;
	DWORD        m_dwIP;
	int			 m_iCSPort;       // C <-> S ��Ʈ
	int          m_iSSPort;       // S <-> S ��Ʈ
	int          m_iMSPort;       // monitoring <-> server ��Ʈ

	//HRYOON -> ��ƾ���� ���
	int			 m_iServerNo;
	
	//HRYOON 20150130 ������ �� �̺�Ʈ
	int			m_iEventMainQuestIDXArr[50];
	
	int			m_iUseQuestEvent;
	int			m_iUseBuyItemEvent;
	int			m_iEventMainIDX;

	int			m_iEventMaxMainCount;
	int			m_iEventMaxSubCount;

	int			m_iEventSubIDX;
	int			m_iEventItemValue1;
	int			m_iEventItemValue2;

	ioHashString m_szGameServerName;
	DWORD        m_dwMainSleepMilliseconds;
	
	ioHashString m_LogServerIP;
	int  m_iLogServerPort;
	
	ioItemInfoManager          *m_pItemInfoMgr;
	ioItemPriceManager         *m_pItemPriceMgr;
	ioSetItemInfoManager       *m_pSetItemInfoMgr;
	ioEtcItemManager           *m_pEtcItemMgr;
	ioDecorationPrice          *m_pDecorationPrice;
	ioExerciseCharIndexManager *m_pExerciseCharIndexMgr;
	ioEventManager             *m_pEventMgr;
	ioPresentHelper            *m_pPresentHelper;
	ioMyLevelMgr               *m_pLevelMgr;
	ioMonsterMapLoadMgr        *m_pMonsterMapLoadMgr;
	ioAlchemicMgr			   *m_pAlchemicMgr;

	GrowthManager			   *m_pGrowthMgr;
	FishingManager			   *m_pFishingMgr;
	NewPublicIDRefresher       *m_pNewPublicIDRefresher;
	ioChannelingNodeManager    *m_pChannelingMgr;
	ioExtraItemInfoManager	   *m_pExtraItemMgr;
	ioLocalManager             *m_pLocalMgr;
	ioItemCompoundManager	   *m_pItemCompoundMgr;
	ioSaleManager              *m_pSaleMgr;
	ioExcavationManager        *m_pExcavationMgr;
	ioQuestManager			   *m_pQuestMgr;
	ioMedalItemInfoManager     *m_pMedalItemMgr;
	LicenseManager             *m_pLicenseMgr;
	TradeInfoManager		   *m_pTradeInfoMgr;
	ioModeINIManager           *m_pModeINIManager;
	ioItemInitializeControl    *m_pItemInitializeControl;
	ioExtraItemGrowthCatalystMgr *m_pExtraItemGrowthCatalystMgr;
	ioFirstSoldierManager        *m_pFirstSoldierManager;
	ioShutDownManager            *m_pShutDownManager;
	LogicThread					 *m_pGameLogicThread;
	TournamentManager			 *m_pTournamentManager;
	ioBingoManager               *m_pBingoMgr;
	ioPirateRouletteManager		 *m_pPirateRouletteMgr;
	ioRelativeGradeManager       *m_pRelativeGradeManager;
	ioSuperGashaponMgr			 *m_pSuperGashaponMgr;
	ioTestNodeManager			 *m_pTestNodeManager;
	ioItemRechargeManager        *m_pItemRechargeMgr;
	ioAttendanceRewardManager    *m_pAttendanceRewardManager;
	ioPetInfoManager			 *m_pPetInfoManager;
	ioPetGashaponManager		 *m_pPetGashaponManager;
	ioCharAwakeManager			 *m_pCharAwakeManager;
	ioPowerUpManager			 *m_pPowerUpManager;
	CostumeManager				 *m_pCostumeManager;
	AccessoryManager			 *m_pAccessoryManager;
	CostumeShopGoodsManager		 *m_pCostumeShopManager;
	SpecailGoodsManager			 *m_pSpecialGoodsManager;
	MissionManager				 *m_pMissionManager;
	RollBookManager				 *m_pRollBookManager;
	GuildRewardManager			 *m_pGuildRewardManager;
	GuildRoomsBlockManager		 *m_pGuildRoomsBlockManager;
	ServerGuildInvenInfo		 *m_pServerGuildInvenInfo;
	ioBlockPropertyManager		 *m_pBlockPropertyManager;
	CompensationMgr				 *m_pCompensationManager;
	HomeModeBlockManager		 *m_pHomeModeBlockManager;
	TimeCashManager				 *m_pTimeCashManager;
	TitleManager				 *m_pTitleManager;
	TradeSyncManager			 *m_pTradeSyncManager;

	SchedulerNode*				m_pScheduler;

private:
	__int64 m_iGameServerID;
	ioHashString m_szGameServerID;

	ioHashString m_szSecondKey;

	bool  m_bWantExit;
	DWORD m_dwCurTime;

	// ũ���� ã�� ���� 
	int   m_iMainLoopChecker;
	LONG  m_iLastFileLine;	
	LPSTR m_pLastFileName;

	// ��ŷ ã�� ����
	LPSTR m_pLoopFileName;
	LONG  m_iLoopFileLine;

	ioHashStringVec m_vNotMakeIDVector;

	// Nagle Time
	uint8 m_NagleTime;

	//testmode
	bool  m_bTestMode;

private:
	bool  m_bReserveLogOut;
	DWORD m_dwReserveLogOutTime;

private:
	bool  m_bTestZone;

	DWORD m_dwDBQueryTime;
	DWORD m_dwDBQuerySendTime;

	bool m_bUseMonitoring;
		
public:	
	void SetINI( const char* szINI )	{ m_szINI = szINI; }

	bool Initialize( SchedulerNode* schedulerPointer );
	BOOL CreatePool();
	BOOL LoadINI();
	void CheckTestZone( const char *szIP );
	BOOL ListenNetwork();
	BOOL StartModules();

private:
	void Process(uint32& idleTime);

public:       // UDP
	bool ProcessUDPPacket( sockaddr_in *client_addr,SP2Packet &rkPacket );

private:
	bool OnWebUDPParse( sockaddr_in *client_addr, SP2Packet &rkPacket );
	bool OnRelayUDPParse( sockaddr_in *client_addr, SP2Packet &rkPacket );

protected:
	DWORD StrToDwordIP( const char *iip );

public:
	void ClientBindStart();
	void ServerBindStart();
	void MonitoringBindStart();

public:	// GET
	const ioHashString& GetINI() const			{ return m_szINI; }
	const ioHashString& GetPublicIP() const		{ return m_szPublicIP;}
	const ioHashString& GetClientMoveIP() const { return m_szClientMoveIP;}
	const ioHashString& GetPrivateIP() const	{ return m_szPrivateIP;}
	const DWORD GetDwordIP() const				{ return m_dwIP; }
	int GetCSPort()								{ return m_iCSPort; }
	int GetSSPort()								{ return m_iSSPort; }
	//HRYOON
	int	GetServerNo()							{ return m_iServerNo; }
	
	//HRYOON 20150130
	int GetUseQuestEvent()								{ return m_iUseQuestEvent; }
	int GetUseBuyItemEvent()							{ return m_iUseBuyItemEvent; }

	int	GetEventMainIDX(int i)							{ return m_iEventMainQuestIDXArr[i]; }
	int	GetEventMaxMainCount()							{ return m_iEventMaxMainCount; }

	int	GetEventSubIDX()								{ return m_iEventSubIDX; }
	int	GetEventMaxSubCount()							{ return m_iEventMaxSubCount; }
	

	int	GetEventItemValue1()							{ return m_iEventItemValue1; }
	int	GetEventItemValue2()							{ return m_iEventItemValue2; }

	const TCHAR* GetLogFolder() const			{ return m_szLogFolder; }
	const ioHashString& GetLogServerIP() const	{ return m_LogServerIP;}
	int   GetLogServerPort() const				{ return m_iLogServerPort;}

	__int64 GetGameServerID() const				{ return m_iGameServerID;}
	const ioHashString& GetszGameServerID() const { return m_szGameServerID; }
	const ioHashString& GetSecondKey() const	{ return m_szSecondKey; }

	const ioHashString GetGameServerName() const { return m_szGameServerName; }

	bool IsReserveLogOut() const				{ return m_bReserveLogOut; }
	bool IsRightID( const char *iid );
	bool IsNotMakeID( const char *szNewID );
	bool IsRightFirstID( const char *szID ); //���ӳ����� public id ������ Ȯ��

	int GetMainLoopChecker() const { return m_iMainLoopChecker; }

	const uint8 GetNagleTime() const { return m_NagleTime; }
	void DrawModule( GAMESERVERINFO& rInfo );

public:
	void SetLogServer(const char* szServerIP, const int iServerPort);
	void SetSecondKey(const ioHashString &szSecondKey) { m_szSecondKey = szSecondKey; }

private:      
	void SetBeforeLoop();
	void ConvertToGameServerID(IN const char* szIP, IN const int iPort, OUT __int64 &iGameServerID);
	void ProcessTime();

	void ConnectUserCountUpdate();
	void ProcessCreateNewLog();
	void ProcessCheckDB();
	void LoadNotMakeID();

public:
	void Exit();
	void Save();
	void Notice();
	void Shutdown(const int type=0);
	bool IsWantExit() { return m_bWantExit; }
	void SetLastFileLine( LPSTR pFileName, LONG iFileLine );
	void SetLoopFileLine( LPSTR pFileName, LONG iFileLine );
	void SetLoopFileLog();
	void SetNagleTime( uint32 nagleTime );

public:
	void CheckLogAllSave();
	void CheckCreateNewLog( bool bStart = false );

public:
	bool IsTestZone() const { return m_bTestZone; }

// �� ���� ��ġ üũ��
public:
	void InitRoomPosCheckCnt();
	void IncreaseRoomPosCheckCnt( int iType );
	void SendRoomPosCheckCnt();

private:
	bool SetLocalIP( int iPrivateIPFirstByte );

	bool GetLocalIpAddressList2(OUT ioHashStringVec& rvIPList);

	bool SetLocalIP2(ioHashString iPrivateIPFirstByte);

public:
	DWORD GetDBQueryTime() const { return m_dwDBQueryTime; }
	DWORD GetDBQuerySendTime() const { return m_dwDBQuerySendTime; }

	void SetDBQueryTime(DWORD dwDBQueryTime) { m_dwDBQueryTime = dwDBQueryTime; }

	bool Startup(const char* scriptName);

	void PrintTimeAndLog(int debuglv, LPSTR fmt );
	void DebugLog(int debuglv, LPSTR filename, int linenum, LPSTR fmt );

// �ؿܿ� ���� ���̼��� üũ
#if defined( SRC_OVERSEAS )
	// ������ ���̵𿡸� 30�� ������ ���̼����� ���� ������ ����͸� ���� ��ȭ���ڷ� ���
	void CheckLicenseForDev();
#endif
	
	bool IsCheckTestMode() const { return m_bTestMode; }

private: /* Singleton */
	ioMainProcess();
	virtual ~ioMainProcess();
};

#define g_App  ioMainProcess::GetInstance()
#define CRASH_GUARD()		g_App.SetLastFileLine( __FILE__, __LINE__ )
#define LOOP_GUARD()		g_App.SetLoopFileLine( __FILE__, __LINE__ )
#define LOOP_GUARD_CLEAR()  g_App.SetLoopFileLine( __FILE__, 0 )
