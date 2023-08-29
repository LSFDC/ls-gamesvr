#include "stdafx.h"
#include "MainProcess.h"

#include "ioEtcLogManager.h"

#include "NodeInfo/RoomNodeManager.h"
#include "NodeInfo/UserNodeManager.h"
#include "NodeInfo/ServerNodeManager.h"
#include "NodeInfo/ioItemInfoManager.h"
#include "NodeInfo/ioSetItemInfoManager.h"
#include "NodeInfo/ioEtcItemManager.h"
#include "NodeInfo/LevelMatchManager.h"
#include "NodeInfo/ioDecorationPrice.h"
#include "NodeInfo/AnnounceMgr.h"
#include "NodeInfo/ChannelNodeManager.h"
#include "NodeInfo/BattleRoomManager.h"
#include "NodeInfo/LadderTeamManager.h"
#include "NodeInfo/MemoNodeManager.h"
#include "NodeInfo/ioExerciseCharIndexManager.h"
#include "NodeInfo/ioEventManager.h"
#include "NodeInfo/ioPresentHelper.h"
#include "NodeInfo/ioMyLevelMgr.h"
#include "NodeInfo/GrowthManager.h"
#include "NodeInfo/FishingManager.h"
#include "NodeInfo/NewPublicIDRefresher.h"
#include "NodeInfo/ioExtraItemInfoManager.h"
#include "NodeInfo/ioMonsterMapLoadMgr.h"
#include "NodeInfo/ioItemCompoundManager.h"
#include "NodeInfo/ioSaleManager.h"
#include "NodeInfo/ioExcavationManager.h"
#include "NodeInfo/ioQuestManager.h"
#include "NodeInfo/ioMedalItemInfoManager.h"
#include "NodeInfo/HeroRankManager.h"
#include "NodeInfo/licensemanager.h"
#include "NodeInfo/TradeInfoManager.h"
#include "NodeInfo/ioItemInitializeControl.h"
#include "NodeInfo/ioExtraItemGrowthCatalystMgr.h"
#include "NodeInfo/iofirstsoldiermanager.h"
#include "NodeInfo/SchedulerNode.h"
#include "NodeInfo/ioAlchemicMgr.h"
#include "NodeInfo/TournamentManager.h"
#include "NodeInfo/ioBingoManager.h"
#include "NodeInfo/ioPirateRouletteManager.h"
#include "NodeInfo/ioRelativeGradeManager.h"
#include "NodeInfo/ioSuperGashaponMgr.h"
#include "NodeInfo/ioItemRechargeManager.h"
#include "NodeInfo/TestNodeManager.h"
#include "NodeInfo/ShuffleRoomReserveMgr.h"
#include "NodeInfo/ioAttendanceRewardManager.h"
#include "NodeInfo/ioPetInfoManager.h"
#include "NodeInfo/ioPetGashaponManager.h"
#include "NodeInfo/ioCharAwakeManager.h"
#include "NodeInfo/ioPowerUpManager.h"
#include "NodeInfo/CostumeManager.h"
#include "NodeInfo/CostumeShopGoodsManager.h"
#include "NodeInfo/SpecialGoodsManager.h"
#include "NodeInfo/MissionManager.h"
#include "NodeInfo/RollBookManager.h"
#include "NodeInfo/GuildRewardManager.h"
#include "NodeInfo/GuildRoomsBlockManager.h"
#include "NodeInfo/ServerGuildInvenInfo.h"
#include "NodeInfo/ioBlockPropertyManager.h"
#include "NodeInfo/CompensationMgr.h"
#include "NodeInfo/HomeModeBlockManager.h"
#include "NodeInfo/TimeCashManager.h"
#include "NodeInfo/TitleManager.h"
#include "NodeInfo/TradeSyncManger.h"
#include "NodeInfo\AccessoryManager.h"

#include "IPBlocker/IPBlockerManager.h"

#include "MainServerNode/MainServerNode.h"
#include "BillingRelayServer/BillingRelayServer.h"
#include "DataBase/DBClient.h"
#include "DataBase/LogDBClient.h"
 

#include "Network/GameServer.h"
#include "Network/ioPacketQueue.h"
#include "Network/iocpHandler.h"
#include "EtcHelpFunc.h"
#include "Version.h"
#include "Shutdown.h"
#ifdef XTRAP
#include "Xtrap/ioXtrap.h"
#endif
#include "channeling/iochannelingnodemanager.h"
#include "local/iolocalmanager.h"
#include "Local/ioLocalParent.h"
#include <strsafe.h>
#include "MiniDump/CrashHandler.h"

#ifdef NPROTECT
#include "NProtect/ioNProtect.h"
#endif
#include "MonitoringServerNode/monitoringnodemanager.h"
#include "NodeInfo/ioShutDownManager.h"
#include "NodeInfo/ioUserSelectShutDown.h"
#include <iphlpapi.h>
#include <ws2tcpip.h>

extern CLog HackLOG;
extern CLog ProcessLOG;
extern CLog EventLOG;
extern CLog P2PRelayLOG;
extern CLog RateCheckLOG;
extern CLog TradeLOG;
extern CLog CriticalLOG;
extern CLog WemadeLOG;
#ifdef ANTIHACK
extern CLog CheatLOG;
extern CLog CheatUser;
#endif
 
BOOL tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens)
{
	tokens.clear();

	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, lastPos);
	
	std::string token;
	while (string::npos != pos || string::npos != lastPos)
	{
		token = str.substr(lastPos, pos - lastPos);
		tokens.push_back(token);

		lastPos = str.find_first_not_of(delimiters, pos);

		pos = str.find_first_of(delimiters, lastPos);
	}
	return (tokens.size() > 0) ? TRUE : FALSE;
}

ioMainProcess *ioMainProcess::sg_Instance = NULL;
ioMainProcess::ioMainProcess() 
{
	m_pClientBind      = NULL;
	m_pServerBind      = NULL;
	m_pMonitoringBind  = NULL;
	m_pItemInfoMgr     = NULL;
	m_pItemPriceMgr    = NULL;
	m_pSetItemInfoMgr  = NULL;
	m_pEtcItemMgr      = NULL;
	m_pDecorationPrice = NULL;
	m_pExerciseCharIndexMgr = NULL;
	m_pEventMgr  = NULL;
	m_pPresentHelper = NULL;
	m_pLevelMgr  = NULL;
	m_pMonsterMapLoadMgr = NULL;
	m_pGrowthMgr = NULL;
	m_pFishingMgr = NULL;
	m_pNewPublicIDRefresher = NULL;
	m_pChannelingMgr        = NULL;
	m_pExtraItemMgr = NULL;
	m_pLocalMgr     = NULL;
	m_pItemCompoundMgr = NULL;
	m_pSaleMgr      = NULL;
	m_pExcavationMgr = NULL;
	m_pQuestMgr		= NULL;
	m_pMedalItemMgr = NULL;
	m_pLicenseMgr   = NULL;
	m_pTradeInfoMgr = NULL;
	m_pModeINIManager   = NULL;
	m_pAlchemicMgr = NULL;

	m_pItemInitializeControl = NULL;
	m_pExtraItemGrowthCatalystMgr = NULL;
	m_pFirstSoldierManager        = NULL;
	m_pShutDownManager            = NULL;
	m_pGameLogicThread			  = NULL;
	m_pTournamentManager		  = NULL;
	m_pRelativeGradeManager       = NULL;
	m_pSuperGashaponMgr			  = NULL;
	m_pItemRechargeMgr            = NULL;
	m_pTestNodeManager			  = NULL;
	m_pAttendanceRewardManager	  = NULL;
	m_pPetInfoManager			  = NULL;
	m_pPetGashaponManager		  = NULL;
	m_pCharAwakeManager			  = NULL;

	m_pScheduler = NULL;
	m_pRollBookManager	= NULL;

	m_iLogServerPort = 0;
	m_dwMainSleepMilliseconds = 0;

	m_iGameServerID = 0;
	m_bWantExit = false;

	m_dwCurTime = 0;

	m_bReserveLogOut = false;
	m_dwReserveLogOutTime = 0;

	m_iCSPort = 0;
	m_iSSPort = 0;
	m_iMSPort = 0;
	//HRYOON
	m_iServerNo = 0;

	//HRYOON ������ �� �̺�Ʈ 
	m_iEventMainIDX = 0;
	m_iEventSubIDX = 0;
	
	for(int i = 0; i< 50; i++) 
	{
		m_iEventMainQuestIDXArr[i] = 0;
	}

	m_pLastFileName = NULL;
	m_iLastFileLine = 0;
	m_iMainLoopChecker = 0;

	m_pLoopFileName = NULL;
	m_iLoopFileLine = 0;

	m_bTestZone = false;

	m_dwDBQueryTime     = 0;
	m_dwDBQuerySendTime = 0;

	m_bUseMonitoring = false;
	ZeroMemory(m_szLogFolder, sizeof(m_szLogFolder));

	m_dwRoomToRoomCnt = 0;
	m_dwEtcToRoomCnt = 0;
	m_bTestMode = false;

	m_iUseQuestEvent			= 0;
	m_iUseBuyItemEvent		= 0;

	m_iEventMaxMainCount = 0;
	m_iEventMaxSubCount = 0;
	
	m_iEventItemValue1 = 0;
	m_iEventItemValue2 = 0;


}

ioMainProcess::~ioMainProcess()
{
	EndSocket();
}

ioMainProcess &ioMainProcess::GetInstance()
{
	if( sg_Instance == NULL )
		sg_Instance = new ioMainProcess;
	return *sg_Instance;
}

void ioMainProcess::ReleaseInstance()
{
	SAFEDELETE( sg_Instance );
}

bool ioMainProcess::Initialize( SchedulerNode* schedulerPointer )
{	
	if(!CreatePool())		return false;
	if(!LoadINI())			return false;
	if(!ListenNetwork())	return false;
	if(!StartModules())		return false;
	
	m_pScheduler = schedulerPointer;
	
	InitRoomPosCheckCnt();
	//return ReloadINI();

// 	CCrashHandler ch;
// 	ch.SetProcessExceptionHandlers();
// 	ch.SetThreadExceptionHandlers();


	return true;
}

BOOL ioMainProcess::CreatePool()
{
	// ��� ���� ���� �� ��Ʈ�� ���� ����
	g_UserNodeManager.InitMemoryPool();
	g_ServerNodeManager.InitMemoryPool();
	g_MonitoringNodeManager.InitMemoryPool();
	g_LevelMatchMgr.LoadLevelMatchInfo();
	g_ProcessChecker.LoadINI();
	g_PacketChecker.LoadINI();
	g_EtcLogMgr.LoadINI();
	g_CriticalError.Initialize();
	
	Help::InitHelpConstant();

	return TRUE;
}

BOOL ioMainProcess::LoadINI()
{
	//---------------------------------
	//        ini ����
	//---------------------------------

	{
		ioINILoader kLoader( "ls_config_game.ini" );
		kLoader.SetTitle( "NETWORK" );
		m_bUseMonitoring = kLoader.LoadBool( "UseMonitoring", false );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Use monitoring : [%d]", (int) m_bUseMonitoring );

		CheckTestZone( m_szPublicIP.c_str() );
		m_dwIP = StrToDwordIP( m_szPublicIP.c_str() );
	}

	char szTemp[MAX_PATH] = "";

	const char* szINI = g_App.GetINI().c_str();
	GetPrivateProfileString("Default", "NAME", "", szTemp, sizeof(szTemp), szINI);	
	m_szGameServerName = szTemp;
	GetPrivateProfileString("Default", "ClientMoveIP", "", szTemp, sizeof(szTemp), szINI); // NAT ONLY
	if( strcmp( szTemp , "" ) == 0 )
		m_szClientMoveIP = m_szPublicIP;
	else
		m_szClientMoveIP = szTemp;
	
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Server ip info : [%s] [%s] [%s]", m_szPublicIP.c_str(), m_szPrivateIP.c_str(),  m_szClientMoveIP.c_str() );

	m_iCSPort   = GetPrivateProfileInt("Default", "CSPORT",   9000, szINI);	
	m_iSSPort   = GetPrivateProfileInt("Default", "SSPORT",   9001, szINI);	
	m_iMSPort   = GetPrivateProfileInt("Default", "MSPORT",   7138, szINI);	
	m_iServerNo = GetPrivateProfileInt("Default", "SERVERNO", -1,   szINI);	

	bool bUseTest = false;
	{
		ioINILoader kLoader( "ls_config_game.ini" );
		kLoader.SetTitle( "TEST" );
		bUseTest = kLoader.LoadBool( "UseTest", false );
	}

	//HRYOON 20140130 ������ �̺�Ʈ
	ioINILoader kLoaderEvent( "ls_config_game.ini" );
	kLoaderEvent.SetTitle( "Garena_Event" );

	m_iUseQuestEvent   = kLoaderEvent.LoadInt( "UseQuest", 0 );
	m_iUseBuyItemEvent = kLoaderEvent.LoadInt( "UseBuyItem", 0 );
	
	//������ ����Ʈ ���� �� �ֵ��� ������
	char szMainIDX[MAX_PATH]="";
	char szSubIDX[MAX_PATH]="";
	if( m_iUseQuestEvent == 1)
	{
		m_iEventMaxMainCount = kLoaderEvent.LoadInt( "MAXMainCount", 1 );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GARENA QUEST EVENT] %s MaxCount%d",
			__FUNCTION__, m_iEventMaxMainCount );	

		for(int i = 0; i< m_iEventMaxMainCount; i++ )
		{
			StringCbPrintf( szMainIDX, sizeof(szMainIDX), "MainIDX%d", i+1 );
			m_iEventMainQuestIDXArr[i] = kLoaderEvent.LoadInt( szMainIDX, 0 );
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GARENA QUEST EVENT] %s MainIDX%d : %d",
						__FUNCTION__, i+1, m_iEventMainQuestIDXArr[i] );	
		}
		//���� IDX �� ������ ���� �ϳ��� �����ؾ���.
		m_iEventSubIDX = kLoaderEvent.LoadInt( "SubIDX", 0 );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GARENA QUEST EVENT] %s SubIDX %d",
			__FUNCTION__, m_iEventSubIDX );	
		

	}
	if ( m_iUseBuyItemEvent == 1)
	{
		m_iEventItemValue1 = kLoaderEvent.LoadInt( "EventItemValue1", 0 );
		m_iEventItemValue2 = kLoaderEvent.LoadInt( "EventItemValue2", 0 );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GARENA BUY ITEM EVENT]%s ItemValue1:%d, ItemValue2:%d",
			__FUNCTION__, m_iEventItemValue1, m_iEventItemValue2 );
	}
	

	//
	m_pSaleMgr = new ioSaleManager; // �ٸ� �Ŵ��� ���� ���� ���� �Ҵ�

	m_pItemPriceMgr = new ioItemPriceManager;        //DB ���� ���� �Ҵ�.
	if( !m_pItemPriceMgr->LoadPriceInfo( "config/sp2_item_price.ini" ) )
		return false;

	////
	//kLoader.LoadString( "LOGSERVERIP", "", szTemp, MAX_PATH );
	//m_LogServerIP = szTemp;
	//m_iLogServerPort = kLoader.LoadInt( "LOGSERVERPORT", 9999 );	

	ConvertToGameServerID(m_szPublicIP.c_str(), m_iCSPort, m_iGameServerID);
	memset(szTemp, 0, sizeof(szTemp));
	_i64toa_s(m_iGameServerID,szTemp,MAX_PATH,MAX_INT64_TO_STR); 
	m_szGameServerID = szTemp;

	HackCheck::LoadHackCheckValues();

	m_pModeINIManager = new ioModeINIManager;
	m_pModeINIManager->LoadINIData( "config/sp2_mode_ini_load.ini" );

	m_pItemInfoMgr = new ioItemInfoManager;
	m_pItemInfoMgr->LoadItemInfo( "config/sp2_item.ini" );

	m_pSetItemInfoMgr = new ioSetItemInfoManager;
	m_pSetItemInfoMgr->LoadINI();

	m_pEtcItemMgr = new ioEtcItemManager;
	m_pEtcItemMgr->LoadEtcItem( "config/sp2_etcitem_info.ini" );

	m_pExtraItemMgr = new ioExtraItemInfoManager;
	m_pExtraItemMgr->LoadAllExtraItemInfo();

	m_pItemCompoundMgr = new ioItemCompoundManager;
	m_pItemCompoundMgr->LoadCompoundInfo();
	m_pItemCompoundMgr->LoadMultipleCompoundInfo();
	m_pItemCompoundMgr->LoadMaterialCompoundInfo();

	m_pDecorationPrice = new ioDecorationPrice;
	m_pDecorationPrice->LoadPriceInfo();

	m_pExerciseCharIndexMgr = new ioExerciseCharIndexManager;
	m_pAlchemicMgr = new ioAlchemicMgr;
	m_pAlchemicMgr->LoadMgrInfo();

	m_pEventMgr = new ioEventManager;
	m_pEventMgr->LoadINI();
	m_pEventMgr->CheckAlive(false);

	m_pPresentHelper = new ioPresentHelper;
	m_pPresentHelper->LoadINI();

	m_pSuperGashaponMgr	= new ioSuperGashaponMgr;
	m_pSuperGashaponMgr->LoadINI();

	m_pLevelMgr = new ioMyLevelMgr;
	m_pLevelMgr->LoadINIInfo();

	m_pMonsterMapLoadMgr = new ioMonsterMapLoadMgr;
	m_pMonsterMapLoadMgr->LoadMapData();

	m_pItemInitializeControl = new ioItemInitializeControl;
	m_pItemInitializeControl->LoadINIData();

	m_pGrowthMgr = new GrowthManager;
	if( m_pGrowthMgr )
		m_pGrowthMgr->LoadGrowthInfo();

	m_pFishingMgr = new FishingManager;
	if( m_pFishingMgr )
	{
		m_pFishingMgr->LoadFishingInfo();
	}

	m_pQuestMgr = new ioQuestManager;
	m_pQuestMgr->LoadINIData();

	m_pTournamentManager = new TournamentManager;
	m_pTournamentManager->LoadINI();

	m_pNewPublicIDRefresher = new NewPublicIDRefresher;

	m_pChannelingMgr = new ioChannelingNodeManager;
	if( m_pChannelingMgr )
		m_pChannelingMgr->Init();

	m_pLocalMgr = new ioLocalManager;
	if( m_pLocalMgr )
		m_pLocalMgr->Init();

	m_pExcavationMgr = new ioExcavationManager;
	if( m_pExcavationMgr )
		m_pExcavationMgr->LoadExcavation();

	m_pMedalItemMgr = new ioMedalItemInfoManager;
	if( m_pMedalItemMgr )
		m_pMedalItemMgr->LoadINI();

	m_pLicenseMgr = new LicenseManager;

	m_pTradeInfoMgr = new TradeInfoManager;
	m_pTradeInfoMgr->LoadTradeInfo();

	m_pExtraItemGrowthCatalystMgr = new ioExtraItemGrowthCatalystMgr;

	m_pFirstSoldierManager = new ioFirstSoldierManager;
	if( m_pFirstSoldierManager )
		m_pFirstSoldierManager->LoadINI();

	m_pShutDownManager = new ioShutDownManager;
	if( m_pShutDownManager )
		m_pShutDownManager->LoadINI();

	//! ����
	m_pBingoMgr = new ioBingoManager;
	if( m_pBingoMgr->LoadINIData( "config/sp2_bingo.ini" ) == false )
		return false;

	m_pPirateRouletteMgr = new ioPirateRouletteManager;
	if( m_pPirateRouletteMgr->LoadINIData( "config/sp2_oak_of_fear.ini") == false )
	{
		return false;
	}

	m_pItemRechargeMgr = new ioItemRechargeManager;
	if( m_pItemRechargeMgr )
		m_pItemRechargeMgr->LoadInIData();

	m_pRelativeGradeManager = new ioRelativeGradeManager;
	m_pRelativeGradeManager->LoadINI();

	m_pAttendanceRewardManager = new ioAttendanceRewardManager;
	m_pAttendanceRewardManager->LoadINI();

	m_pTestNodeManager = new ioTestNodeManager( bUseTest );
	m_pTestNodeManager->LoadINI();

	m_pPetInfoManager = new ioPetInfoManager;
	m_pPetInfoManager->LoadINI();

	m_pPetGashaponManager = new ioPetGashaponManager;
	m_pPetGashaponManager->LoadINI();

	m_pCharAwakeManager = new ioCharAwakeManager;
	m_pCharAwakeManager->LoadINI();

	m_pPowerUpManager = new ioPowerUpManager;
	m_pPowerUpManager->LoadINI();
	ioUserSelectShutDown::LoadINI();

	m_pCostumeManager = new CostumeManager;
	m_pCostumeManager->LoadINI();

	m_pAccessoryManager = new AccessoryManager;
	m_pAccessoryManager->LoadINI();

	m_pCostumeShopManager	= new CostumeShopGoodsManager;
	if( !m_pCostumeShopManager )
		return false;
	m_pCostumeShopManager->LoadINI();

	m_pSpecialGoodsManager = new SpecailGoodsManager;
	
	m_pMissionManager	= new MissionManager;
	if( !m_pMissionManager->LoadINI() )
	{
		CriticalLOG.PrintTimeAndLog( 0, "[error][missionmgr] value count is error" );
		return false;
	}

	m_pRollBookManager = new RollBookManager;
	if( !m_pRollBookManager )
		return false;

	m_pRollBookManager->LoadInI();

	m_pGuildRewardManager	= new GuildRewardManager;
	if( !m_pGuildRewardManager )
		return false;
	m_pGuildRewardManager->LoadINI();
	m_pBlockPropertyManager		= new ioBlockPropertyManager;

	if( m_pBlockPropertyManager )
		m_pBlockPropertyManager->LoadIni();

	m_pGuildRoomsBlockManager	= new GuildRoomsBlockManager;
	if( m_pGuildRoomsBlockManager )
		m_pGuildRoomsBlockManager->LoadIni();

	m_pHomeModeBlockManager		= new HomeModeBlockManager;
	if( m_pHomeModeBlockManager )
		m_pHomeModeBlockManager->LoadIni();

	m_pTimeCashManager			= new TimeCashManager;
	if( m_pTimeCashManager )
		m_pTimeCashManager->LoadINI();

	m_pTitleManager				= new TitleManager;
	if( m_pTitleManager )
		m_pTitleManager->LoadINI();

	m_pTradeSyncManager			= new TradeSyncManager;
	if( !m_pTradeSyncManager )
		return false;

	m_pServerGuildInvenInfo		= new ServerGuildInvenInfo;
	m_pCompensationManager		= new CompensationMgr;

	g_UserNodeManager.LoadDeveloperID();
	g_UserNodeManager.LoadSpecialIP();
	g_IPBlock.Load();

	ioINILoader kLoader( "ls_config_game.ini" );
	kLoader.SetTitle( "Local" );
	ioLocalManager::SetLocalType( (ioLocalManager::LocalType) kLoader.LoadInt( "Version", ioLocalManager::LCT_KOREA ) );

	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal && !pLocal->IsRightLicense() )
	{
	
		CriticalLOG.PrintTimeAndLog( 0, "This program has expired.");
		return false;
	}

	if( pLocal )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]EXPIRATION DATES : [%d]", pLocal->GetLicenseDate() );
	}

	g_ShuffleRoomReserveMgr.Initialize();

	LoadNotMakeID();


	//
#ifdef XTRAP
	kLoader.SetTitle("Xtrap");
	bool bXtrapUse = kLoader.LoadBool( "use", false );
	if( !g_ioXtrap.LoadDll( bXtrapUse ) )
		return false;

	if( !g_ioXtrap.LoadCS3File() )
		return false;

	if( !g_ioXtrap.Start( m_szPublicIP.c_str() ) )
		return false;
#endif

#ifdef NPROTECT
	kLoader.SetTitle("NProtect");
	bool bNProtectUse = kLoader.LoadBool( "use", false );
	if( !g_ioNProtect.Start( bNProtectUse ) )
		return false;
#endif

#ifdef XIGNCODE
	kLoader.SetTitle("Xigncode");
	bool bXigncodeUse = kLoader.LoadBool( "use", false );
	if( !g_ioXignCode.Start( bXigncodeUse ) )
		return false;
#endif

#ifdef HACKSHIELD
	kLoader.SetTitle("HackShield");
	bool bHackShieldUse = kLoader.LoadBool( "use", false );
	if( !g_ioHackShield.Start( bHackShieldUse ) )
		return false;
#endif

#ifdef SRC_LATIN
	kLoader.SetTitle("Apex");
	bool bApexUse = kLoader.LoadBool( "use", true );
	int nResult = g_ioApex.Start( bApexUse );
	LOG.PrintTimeAndLog(0, "Apex Start %d", nResult);
	if( nResult != 0 )
		return false;
#endif

	// Nagle
	kLoader.SetTitle( "Nagle" );
	m_NagleTime = kLoader.LoadInt( "Nagle_Time", 30 );


	//�׽�Ʈ ����� ��쿣 ���� ����
	kLoader.SetTitle( "TEST" );
	m_bTestMode = (bool)kLoader.LoadInt( "testMode", 0 );

	

	return TRUE;
}

bool ioMainProcess::SetLocalIP( int iPrivateIPFirstByte )
{
	ioHashStringVec vIPList;
	if( !Help::GetLocalIpAddressList( vIPList, false ) ) 
		return false;

	int iSize = vIPList.size();
#ifdef LOCAL_DBG
	if( iSize >= 1 )
	{
		m_szPublicIP  = vIPList[0];
		m_szPrivateIP = vIPList[0];
		return true;
	}
#endif
	// 1, 2 �ƴϸ� ����
	if( !COMPARE( iSize, 1, 3 ) )
	{
		
		CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Size Error %d", __FUNCTION__, iSize );
		return false;
	}

	// 1
	if( iSize == 1 ) 
	{
		m_szPublicIP  = vIPList[0];
		m_szPrivateIP = vIPList[0];

		if( m_szPrivateIP.IsEmpty() || m_szPublicIP.IsEmpty() )
		{
			
			CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Local IP Error %s:%s", __FUNCTION__, m_szPrivateIP.c_str(), m_szPublicIP.c_str() );
			return false;
		}

		return true;
	}

	// 2
	for (int i = 0; i < iSize ; i++)
	{
		if( atoi( vIPList[i].c_str() ) != iPrivateIPFirstByte )
		{
			m_szPublicIP = vIPList[i];
		}
		else
		{
			m_szPrivateIP = vIPList[i];
		}
	}

	if( m_szPrivateIP.IsEmpty() || m_szPublicIP.IsEmpty() )
	{
		
		CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Local IP Empty %s:%s", __FUNCTION__, m_szPrivateIP.c_str(), m_szPublicIP.c_str() );
		return false;
	}

	return true;
}

#pragma comment(lib, "iphlpapi.lib")

bool ioMainProcess::GetLocalIpAddressList2(OUT ioHashStringVec& rvIPList) {


	ULONG bufferSize = 0;
	if (GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &bufferSize) != ERROR_BUFFER_OVERFLOW) {

		Information("GetAdaptersAddresses failed to get buffer size");
		return false;
	}

	IP_ADAPTER_ADDRESSES* pAdapterAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
	if (!pAdapterAddresses) {

		Information("Memory allocation failed");

		return false;
	}

	if (GetAdaptersAddresses(AF_INET, 0, NULL, pAdapterAddresses, &bufferSize) != ERROR_SUCCESS) {

		Information("GetAdaptersAddresses failed");

		free(pAdapterAddresses);
		return false;
	}


	for (IP_ADAPTER_ADDRESSES* pAdapter = pAdapterAddresses; pAdapter; pAdapter = pAdapter->Next) {
		IP_ADAPTER_UNICAST_ADDRESS* pUnicast = pAdapter->FirstUnicastAddress;
		while (pUnicast) {
			SOCKADDR_IN* pSockAddr = (SOCKADDR_IN*)pUnicast->Address.lpSockaddr;
			if (pSockAddr->sin_family == AF_INET) {
				char szTemp[MAX_PATH] = "";
				strcpy_s(szTemp, sizeof(szTemp), inet_ntoa(pSockAddr->sin_addr));
				ioHashString sTemp = szTemp;
				rvIPList.push_back(sTemp);

			}
			pUnicast = pUnicast->Next;
		}
	}

	free(pAdapterAddresses);

	return true;
}

bool ioMainProcess::SetLocalIP2(ioHashString iPrivateIPFirstByte)
{
	ioHashStringVec vIPList;

	if (!GetLocalIpAddressList2(vIPList))
	{
	
		return false;
	}

	int iSize = vIPList.size();

	if (iSize == 0)
	{
	
		return false;
	}

	if (iSize == 1)
	{
		m_szPublicIP = vIPList[0];
		m_szPrivateIP = vIPList[0];

		if (m_szPrivateIP.IsEmpty() || m_szPublicIP.IsEmpty())
		{
			LOG.PrintTimeAndLog(0, "%s Local IP Error %s:%s", __FUNCTION__, m_szPrivateIP.c_str(), m_szPublicIP.c_str());
			return false;
		}

		return true;
	}

	for (int i = 0; i < iSize; i++)
	{
		if (vIPList[i] == iPrivateIPFirstByte.c_str())
		{
			m_szPublicIP = vIPList[i];
			m_szPrivateIP = vIPList[i];
			break;
		}
		else
		{
			printf("Socket IP doesn't match with any IP : %s \n", iPrivateIPFirstByte.c_str());
			return false;
		}
	}

	if (m_szPrivateIP.IsEmpty() || m_szPublicIP.IsEmpty())
	{
	
		return false;
	}

	return true;
}

void ioMainProcess::CheckTestZone( const char *szIP )
{
	if( strcmp( szIP, "211.239.156.226")  == 0 ||
		strcmp( szIP, "211.239.156.227")  == 0 )
	{
		m_bTestZone = true;
	}
	else
		m_bTestZone = false;
}

BOOL ioMainProcess::ListenNetwork()
{
	g_RecvQueue.Initialize();

	if( !g_iocp.Initialize() )  
		return FALSE;

	if( !g_MainServer.ConnectTo() )
	{
		CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Fail connect MainServer.", "IOEnter");
		return FALSE;
	} 
  
 	g_UDPModule; g_UDPNode;

	g_UDPNode.InitMemory(2000,16384*2,2000);
	std::vector<int> m_ports;
	m_ports.push_back(m_iCSPort);
	std::string tmp ="0.0.0.0";
	int workercount = m_ports.size() + 4;
	if( !g_UDPModule.SetUDPModule(m_ports,
								  tmp,
								   10399,
								   &g_UDPNode,
								   workercount)) 
		return FALSE;
	m_ports.clear();
	
	if(!g_Relay.Init(1000,5000,7))
	{ 
		CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Fail BroadCastMgr Init.", "IOEnter");
		return FALSE;
	}
	return TRUE;
}

BOOL ioMainProcess::StartModules()
{
	// logic
	Information( "Starting processor..." );
	m_pGameLogicThread	= new LogicThread;
	m_pGameLogicThread->SetProcessor(&(g_App));

	if( !m_pGameLogicThread->Begin() )
	{
		Information( "failed\n" );
		return FALSE;
	}
	Information( "done\n" );
	
	FrameTimer.Start(30.0f);
	srand(timeGetTime());

	return TRUE;
}

void ioMainProcess::Exit()
{
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- EXIT SERVER" );

	if(m_bWantExit)
	{
		exit(1);
	}
}

void ioMainProcess::Save()
{
	static BOOL saved = FALSE;
	if( !saved )
	{
		saved = TRUE;

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- SAVE DATA" );

		//SendRoomPosCheckCnt();

		CheckLogAllSave();

		g_UserNodeManager.UserNode_AllSave();
		g_ItemPriceMgr.UpdatePriceCollected();
		g_DBClient.OnUpdateAllUserLogout(m_iGameServerID);
		g_DBClient.OnDeleteGameServerInfo(m_iGameServerID);
		
		//MainServer�� ���� ���� ���� ��û.
		g_GuildRoomBlockMgr.DeleteAllRoomInfo();

		Sleep(300);
	}
}

void ioMainProcess::Notice()
{
	SP2Packet kReturn( STPK_RESERVE_LOGOUT );
	kReturn << RESERVE_LOGOUT_SAFETY_EXIT;
	g_UserNodeManager.SendMessageAll(kReturn);
}

void ioMainProcess::Shutdown(const int type)
{

	switch(type)
	{
	case SHUTDOWN_NONE :
		break;

	case SHUTDOWN_TEST :
		//::MessageBox( GetHwnd(), "shutdown", "Lostsaga", MB_OK );
		break;

	case SHUTDOWN_QUICK :
		{
			if( !m_bWantExit && !m_bReserveLogOut )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- GAMESERVER QUICK EXIT" );
				g_EtcLogMgr.WriteLOG();

				m_bWantExit = true;

				Save();
			}
		}
		break;

	case SHUTDOWN_SAFE :
		{
			if( !m_bReserveLogOut )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- GAMESERVER SAFETY EXIT" );
				g_EtcLogMgr.WriteLOG();

				Notice();

				m_bReserveLogOut = true;
				m_dwReserveLogOutTime = TIMEGETTIME();
			}
		}
		break;

	case SHUTDOWN_SERVICE :
	case SHUTDOWN_EMPTYPOOL :
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- SERVICE STOP" );

			m_bWantExit = true;
			Save();
			Exit();
		}
		break;

	case SHUTDOWN_DBAGENT :
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "- GAMESERVER DBAGENT CRASH" );
			g_EtcLogMgr.WriteLOG();

			Notice();

			m_bWantExit = true;
			Save();
			Exit();
		}
		break;

	case SHUTDOWN_CRASH :
		{
			g_EtcLogMgr.WriteLOG();

			if( m_pLastFileName )
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Last File > %s : %dLine", m_pLastFileName, m_iLastFileLine );

			if( m_pLoopFileName )
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Loop File > %s : %dLine", m_pLoopFileName, m_iLoopFileLine );

			m_bWantExit = true;
			Save();
		}
		break;

	default :
		break;
	}
}

void ioMainProcess::SetLastFileLine( LPSTR pFileName, LONG iFileLine )
{
	m_pLastFileName = pFileName;
	m_iLastFileLine = iFileLine;
}

void ioMainProcess::SetLoopFileLine( LPSTR pFileName, LONG iFileLine )
{
	m_pLoopFileName = pFileName;
	m_iLoopFileLine = iFileLine;
}

void ioMainProcess::SetLoopFileLog()
{
	if( m_pLoopFileName )
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Loop File > %s : %dLine", m_pLoopFileName, m_iLoopFileLine );
}

void ioMainProcess::SetNagleTime( uint32 nagleTime )
{
	// assert
	assert( m_pScheduler );

	// Nagle
	BOOL bResult = m_pScheduler->ChangeTickValue( ITPK_SENDBUFFER_FLUSH_PROCESS, nagleTime );

	// log
	if( bResult == TRUE )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Nagle Tick Change Success.. (%d)", nagleTime );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Nagle Tick Change Fail.. (%d)", nagleTime );
	}
}

void ioMainProcess::SetLogServer(const char* szServerIP, const int iServerPort)
{
	m_LogServerIP		= szServerIP;
	m_iLogServerPort	= iServerPort;	
}

void ioMainProcess::SetBeforeLoop()
{
	if( !g_MainServer.IsActive() )
		g_DBClient.OnSelectItemBuyCnt();
	//
	g_HeroRankManager.OnSelectHeroRank();
}

void ioMainProcess::ConvertToGameServerID( IN const char* szIP, IN const int iPort, OUT __int64 &iGameServerID )
{
	char szCopiedIP[MAX_PATH]="";
	strcpy_s(szCopiedIP, szIP);

	char *next_token = NULL;

	const int int64Size = sizeof(__int64);
	const int intSize = sizeof(int);
	unsigned char szByteArray[MAX_PATH];
	memset(szByteArray, 0 , MAX_PATH);

	int iLoopCnt = 0;
	while (iLoopCnt < 100)
	{
		char *pos = NULL;
		if( iLoopCnt == 0)
			pos = strtok_s(szCopiedIP, ".", &next_token);
		else
			pos = strtok_s(NULL, ".", &next_token);

		if(pos == NULL)
			break;
		// IP�� 4���� ������ �ѹ���Ʈ�� �и��ؼ� �ִ´�
		szByteArray[iLoopCnt]=atoi(pos);

		iLoopCnt++;
		if(iLoopCnt >= int64Size)
			break;
	}

	iGameServerID = 0;
	memcpy(&szByteArray[intSize], &iPort, intSize);
	memcpy(&iGameServerID, szByteArray, int64Size);
}

void ioMainProcess::Process(uint32& idleTime)
{
	g_ProcessChecker.MainThreadCheckTimeStart();

	FrameTimer.SetFrame();
	
	g_ProcessChecker.CallMainThread();

	// ��� ��Ŷ�� ó��
	g_PacketChecker.CheckCollectFreezing();
 
 
	int iPacketParsingSize = 0;
	iPacketParsingSize += g_RecvQueue.PacketParsing();
	g_ProcessChecker.MainProcessMaxPacket( iPacketParsingSize );

	g_ProcessChecker.MainThreadCheckTimeEnd();

	ProcessTime();
	
	if(iPacketParsingSize == 0)
	{
		idleTime = 1;
	}
	else
		idleTime = 0;

	
}

void ioMainProcess::ConnectUserCountUpdate()
{
	// 2013-01-02 �ſ���, ������� �����Ƿ� �ּ�ó��
	return;

	//DWORD dwMilSec = 2000;     // ���� �Ѱ� �ѽð� �������� 2�ʿ� �ѹ��� ������Ʈ
	//static DWORD stFirstCheckTime = TIMEGETTIME();
	//if( TIMEGETTIME() - stFirstCheckTime > 3600000 )      // ���� �Ѱ� �ѽð� ���ĺ��ʹ� 10�ʿ� �ѹ��� ������Ʈ
	//	dwMilSec = 10000;

	////dwMilSec ���� ���� ������ �� DB�� ������Ʈ
	//static DWORD stStartTime = TIMEGETTIME();
	//if( TIMEGETTIME() - stStartTime > dwMilSec )
	//{
	//	stStartTime = TIMEGETTIME();
	//	if( g_ServerNodeManager.GetServerIndex() != 0 )
	//	{
	//		g_DBClient.OnUpdateUserCount( g_App.GetGameServerID(), g_UserNodeManager.GetNodeSize() );
	//		m_dwDBQuerySendTime = TIMEGETTIME();
	//	}		
	//}
}

void ioMainProcess::ProcessCreateNewLog()
{
	if( TIMEGETTIME() - m_dwCurTime < 60000 ) return;
	m_dwCurTime = TIMEGETTIME();
	CheckCreateNewLog();
}

void ioMainProcess::LoadNotMakeID()
{
	m_vNotMakeIDVector.clear();

	ioINILoader kLoader( "config/sp2_not_make_id.ini" );
	kLoader.SetTitle("word");

	char szKeyName[MAX_PATH]="";
	char szBuff[MAX_PATH]="";
	int iMax = kLoader.LoadInt("max", 0);
	for (int i = 0; i < iMax ; i++)
	{
		ZeroMemory( szKeyName, sizeof( szKeyName ) );
		ZeroMemory( szBuff, sizeof( szBuff ) );

		StringCbPrintf(szKeyName, sizeof(szKeyName), "%d", i+1);
		kLoader.LoadString(szKeyName, "", szBuff, sizeof(szBuff));
		_strlwr_s( szBuff, sizeof(szBuff) );
		if(!strcmp(szBuff, ""))
			continue;

		m_vNotMakeIDVector.push_back( ioHashString( szBuff ) );
	}

	kLoader.SetTitle( "ascii" );
	iMax = kLoader.LoadInt("max", 0);
	for (int i = 0; i < iMax ; i++)
	{
		ZeroMemory( szKeyName, sizeof( szKeyName ) );
		ZeroMemory( szBuff, sizeof( szBuff ) );

		StringCbPrintf(szKeyName, sizeof(szKeyName), "%d", i+1);
		int iAsciiCode = kLoader.LoadInt(szKeyName, -1);
		if( iAsciiCode == -1)
			break;

		szBuff[0] = iAsciiCode;
		if(!strcmp(szBuff, ""))
			continue;

		m_vNotMakeIDVector.push_back( ioHashString( szBuff ) );
	}
}

bool ioMainProcess::IsNotMakeID( const char *szNewID )
{
	char szLwrID[ID_NUM_PLUS_ONE]="";
	StringCbCopy( szLwrID, sizeof( szLwrID ), szNewID );
	_strlwr_s( szLwrID, sizeof(szLwrID) );
	int iVecSize = m_vNotMakeIDVector.size();
	for (int i = 0; i < iVecSize ; i++)
	{
		if( IsDBCSLeadByte( m_vNotMakeIDVector[i].At( 0 ) ) )
		{
			if( _mbsstr( (const unsigned char*)szLwrID, (const unsigned char*)m_vNotMakeIDVector[i].c_str() ) )
				return true;
		}
		else if( m_vNotMakeIDVector[i].Length() == 1 )
		{
			int iIDSize = strlen( szLwrID );
			for (int i2 = 0; i2 < iIDSize ; i2++)
			{	
				if( IsDBCSLeadByte( szLwrID[i2] ) ) 
				{
					i2++;
				}
				else
				{
					if( szLwrID[i2] == m_vNotMakeIDVector[i].At(0) )
						return true;
				}
			}
		}
		else
		{
			if( strstr( szLwrID, m_vNotMakeIDVector[i].c_str() ) )
				return true;
		}
	}

	return false;
}

void ioMainProcess::DrawModule( GAMESERVERINFO& rInfo )
{
	//GLOBAL TIME
	rInfo.dwGlobalTime = TIMEGETTIME();
	
	//Network Info
	strcpy_s( rInfo.szPublicIP, m_szPublicIP.c_str() );
	rInfo.csPort	= m_iCSPort;
	rInfo.SSPort	= m_iSSPort;
	rInfo.MSPort	= m_iMSPort;
	
	//Thread Info
	rInfo.ThreadCount = ThreadManager::GetInstance()->GetHandleCount();

	//Connect Client Info
	rInfo.NodeSize = g_UserNodeManager.GetNodeSize();
	rInfo.CopyNodeSize = g_UserNodeManager.GetCopyUserNodeSize();

	//Remainder MemPool Info
	rInfo.RemainderNode = g_UserNodeManager.RemainderNode();
	
	//CREATE ROOM 
	rInfo.RoomNodeSize = g_RoomNodeManager.GetRoomNodeSize();
	rInfo.PlazaNodeSize = g_RoomNodeManager.GetPlazaNodeSize();
	rInfo.HeapQuartersNodeSize = g_RoomNodeManager.GetHeadquartersNodeSize();
	rInfo.CopyRoomNodeSize = g_RoomNodeManager.GetCopyRoomNodeSize();
	rInfo.CopyPlazaNodeSize = g_RoomNodeManager.GetCopyPlazaNodeSize();
	rInfo.CopyHeapQuartersNodeSize = g_RoomNodeManager.GetCopyHeadquartersNodeSize();

	//Remainder MemPool Info
	rInfo.RommRemainderNode = g_RoomNodeManager.RemainderNode();

	//CREATE BATTLEROOM
	rInfo.BattleRoomNodeSize = g_BattleRoomManager.GetNodeSize();
	rInfo.BattleRoomCopyNodeSize = g_BattleRoomManager.GetCopyNodeSize();
	rInfo.PartyLevelCheckMinRoom = g_LevelMatchMgr.GetPartyLevelCheckMinRoom();
	
	//Remainder MemPool Info
	rInfo.BattleRoomRemainderNode = g_BattleRoomManager.RemainderNode();

	//CREATE LADDERTEAM
	rInfo.LadderTeamNodeSize = g_LadderTeamManager.GetNodeSize();
	rInfo.LadderTeamCopyNodeSize = g_LadderTeamManager.GetCopyNodeSize();
	rInfo.LadderTeamCampBattlePlay = (int)g_LadderTeamManager.IsCampBattlePlay();

	//Remainder MemPool Info
	rInfo.LadderTeamRemainderNode = g_LadderTeamManager.RemainderNode();
	
	//RECV QUEUE
	int usingCount[4], remainCount[4];
	g_RecvQueue.GetPoolCount( usingCount, remainCount );
	rInfo.RecvQueueNodeSize[ 0 ] = usingCount[0];
	rInfo.RecvQueueNodeSize[ 1 ] = usingCount[1];
	rInfo.RecvQueueNodeSize[ 2 ] = usingCount[2];
	rInfo.RecvQueueNodeSize[ 3 ] = usingCount[3];
 

	rInfo.BroadCastUDPnRelayNodeSize =  g_Relay.GetNodeSize();  

	//Remainder MemPool Info
	rInfo.RecvQueueRemainderNodeSize[ 0 ] = remainCount[0];
	rInfo.RecvQueueRemainderNodeSize[ 1 ] = remainCount[1];
	rInfo.RecvQueueRemainderNodeSize[ 2 ] = remainCount[2];
	rInfo.RecvQueueRemainderNodeSize[ 3 ] = remainCount[3];
 

	//rInfo.BroadCastUDPnRelayRemainderNodeSize = g_Relay.RemainderNode(); //kyg �������Ұ� 

	//DB AGENT SERVER INFO
	rInfo.DBClientNodeSize = g_DBClient.GetNodeSize();
	rInfo.DBClientTotalNodeSize = g_DBClient.GetTotalNodeSize();

	// Game Server Version
	strcpy_s( rInfo.szSTRFILEVER, STRFILEVER );
	strcpy_s( rInfo.szSTRINTERNALNAME, STRINTERNALNAME );
	
	// Game Server ID
	strcpy_s( rInfo.szGameServerID, m_szGameServerID.c_str() );

	// Game Server Name
	strcpy_s( rInfo.szGameServerName, m_szGameServerName.c_str() );

	// LogDB Agent IP / port
	rInfo.IsLogDBAActive = g_LogDBClient.IsActive();
	strcpy_s( rInfo.szLogDBIP, g_LogDBClient.GetDBAgentIP().c_str() );
	rInfo.LogDBPort = g_LogDBClient.GetDBAgentPort();

	// HackCheck
	rInfo.HackCheckMin = HackCheck::SH_LessCheckTime();
	rInfo.HackCheckMax = HackCheck::SH_OverCheckTime();
	rInfo.HackCheckLess = HackCheck::SH_LessCount();
	rInfo.HackCheckOver = HackCheck::SH_OverCount();
	rInfo.HackCheckLessOver = HackCheck::SH_LessOverCount();
	rInfo.HackCheckTotal = HackCheck::SH_TotalCount();

	// Client Version
	rInfo.IsClientVersion = (int)g_MainServer.IsUseClientVersion();
	rInfo.GetClientVersion = g_MainServer.GetClientVersion();
	
	//CREATE Channel
	rInfo.ChannelNodeSize = g_ChannelNodeManager.GetNodeSize();
	rInfo.ChannelRemainderSize = g_ChannelNodeManager.RemainderNode();
	rInfo.ChannelCopyNodeSize = g_ChannelNodeManager.GetCopyChannelNodeSize();

	//MEMO
	rInfo.MemoNodeSize = g_MemoNodeManager.GetNodeSize();

	//GAME SERVER INFO
	rInfo.GetServerIndex = g_ServerNodeManager.GetServerIndex();
	rInfo.ServerNodeSize = g_ServerNodeManager.GetNodeSize();
	rInfo.ServerRemainderNodeSize = g_ServerNodeManager.RemainderNode();

	// Main Server Info
	rInfo.IsMainServerActive = g_MainServer.IsActive();
	strcpy_s( rInfo.MainServerIP, ( g_MainServer.GetMainIP() ).c_str() );
	rInfo.MainServerPort = g_MainServer.GetMainPort();

	//UDP Transfer Count
	rInfo.UDPTransferCount = g_EtcLogMgr.GetUDPTransferCount();
	rInfo.UDPTransferTCPCount = g_EtcLogMgr.GetUDPTransferTCPCount();
	rInfo.UDPTransferTCPSendCount = g_EtcLogMgr.GetUDPTransferTCPSendCount();

	//BILLING RELAY SERVER INFO
	rInfo.IsBillingRelayServerActive = g_BillingRelayServer.IsActive();
	strcpy_s( rInfo.BillingIP, ( g_BillingRelayServer.GetBillingIP() ).c_str() );
	rInfo.BillingPort = g_BillingRelayServer.GetBillingPort();

	// XTRAP
#ifdef XTRAP
	g_ioXtrap.GetTextCS3Version( rInfo.XtrapVersion, sizeof( rInfo.XtrapVersion ) );
#endif

	// Expiration dates
	rInfo.LicenseDate = 0;

	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
	{
		rInfo.LicenseDate = pLocal->GetLicenseDate();
	}
	
	// Exit
	rInfo.m_bReserveLogout = m_bReserveLogOut;
	rInfo.remainSecond = ( WAIT_TIME_FOR_RESERVE_LOGOUT - ( TIMEGETTIME() - m_dwReserveLogOutTime ) ) / 1000;

	// sendBuffer
	rInfo.sendBufferRemainCnt	= g_SendBufferManager->GetRemainCount();
	rInfo.sendBufferUsingCnt	= g_SendBufferManager->GetToalCount() - rInfo.sendBufferRemainCnt;
	rInfo.sendBufferMaxCnt		= g_SendBufferManager->GetMaxUsingCount();

	// log
	LOG.GetMemoryInfo(rInfo.remainLogCount, rInfo.usingLogCount, rInfo.maxUsingLogCount, rInfo.dropLogCount);
}

 
 
bool ioMainProcess::ProcessUDPPacket( sockaddr_in *client_addr, SP2Packet &rkPacket ) //udp ���� ȣ�� �Ǵºκ� 
{
	// ���� �������� ���ư��Ƿ� ũ��Ƽ�� ���� �ʿ����.

	// W - > S
 #if 0
	if(rkPacket.GetPacketID() == 0x4500) //testcode
	{

		char rcv_ip[16] = "";
		sprintf_s( rcv_ip, "%d.%d.%d.%d", client_addr->sin_addr.s_net, client_addr->sin_addr.s_host,
			client_addr->sin_addr.s_lh, client_addr->sin_addr.s_impno );
		int a;
		rkPacket >> a;
		SP2Packet rpk(0x4500);
		rpk << a;
		g_UDPNode.SendMessage(rcv_ip,(short)ntohs( client_addr->sin_port ),rpk);
//		LOG.PrintTimeAndLog(0,"Send!! ProcessUDPPacket");
		return true;
	}
#endif
	if( OnWebUDPParse( client_addr, rkPacket ) )
		return true;	

	// C - > S
	switch( rkPacket.GetPacketID() )
	{
	case CUPK_CONNECT:                   //������ �˾ƾ� �� ����.
	case CUPK_SYNCTIME:
	case CUPK_RESERVE_ROOM_JOIN:
	case CUPK_CHECK_KING_PING:
		{
			char szPublicID[ID_NUM_PLUS_ONE] = "";
			rkPacket >> szPublicID;
			User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
			if( !pUser) return false;	
			if( !pUser->IsConnectState() ) return false;
			
			pUser->SetClientAddr( *client_addr );
			pUser->PacketParsing( rkPacket );
		}
		return true;
	case LUPK_SHUTDOWN:
		{
			g_LicenseMgr.OnLicense( *client_addr, rkPacket );
		}
		return true;
#ifdef PENGUING
	case CUPK_PENGUIN_PING:
		{
			char rcv_ip[16] = "";
			sprintf_s( rcv_ip, "%d.%d.%d.%d", client_addr->sin_addr.s_net, client_addr->sin_addr.s_host,
				client_addr->sin_addr.s_lh, client_addr->sin_addr.s_impno );

			g_UDPNode.SendMessage(rcv_ip,(short)ntohs( client_addr->sin_port ), rkPacket );
	}
		return true;
#endif
	}

	// C - > C
	return OnRelayUDPParse( client_addr, rkPacket );
}

bool ioMainProcess::OnWebUDPParse( sockaddr_in *client_addr, SP2Packet &rkPacket )
{
	if( !COMPARE( rkPacket.GetPacketID(), 0x9501, 0x10000 + 1 ) )
		return false; 

	if( client_addr )
	{
		char szIP[MAX_PATH]="";
		int  iPort = 0;
		StringCbPrintf( szIP, sizeof( szIP ), "%d.%d.%d.%d",client_addr->sin_addr.s_net, client_addr->sin_addr.s_host, client_addr->sin_addr.s_lh, client_addr->sin_addr.s_impno);
		iPort = ntohs(client_addr->sin_port);	

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse Sender %s:%d:0x%x", szIP, iPort, rkPacket.GetPacketID() );
	}
	else if( client_addr == NULL )
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse client_addr == NULL :0x%x", rkPacket.GetPacketID() );
	}

	ioHashString szKey;
	DWORD dwUserIndex = 0;
	ioHashString szPublicID;
	rkPacket >> szKey;
	rkPacket >> dwUserIndex;
	rkPacket >> szPublicID;

	if( szKey != WEB_UDP_KEY_VALUE ) // ������Ŷ���� Ȯ��
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse Wrong Key : %s :0x%x", szKey.c_str(), rkPacket.GetPacketID() );
		return true;
	}

	UserParent *pUserParent = NULL;
	if( dwUserIndex != 0)
		pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	else if( szPublicID != "_" )
		pUserParent = g_UserNodeManager.GetGlobalUserNode( szPublicID );

	if( !pUserParent)
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse Not User:%d:%s:0x%x", dwUserIndex, szPublicID.c_str(), rkPacket.GetPacketID() );
		return true;
	}

	// Success
	if( pUserParent->IsUserOriginal() )
	{
		User *pUser = static_cast<User*> (pUserParent);
		pUser->PacketParsing( rkPacket );
		return true;
	}

	// Move Server
	DWORD dwSSMsgType = 0;
	switch( rkPacket.GetPacketID() )
	{
	case WUPK_EVENT:
		dwSSMsgType = SSTPK_WEB_EVENT;
		break;
	case WUPK_REFRESH_BLOCK:
		dwSSMsgType = SSTPK_WEB_REFRESH_BLOCK;
		break;
	case WUPK_GET_CASH:
		dwSSMsgType = SSTPK_WEB_GET_CASH;
		break;
	case WUPK_REFRESH_USER_ENTRY:
		dwSSMsgType = SSTPK_WEB_REFRESH_USER_ENTRY;
		break;
	default:
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse Not SSMsgType : %s:0x%x", pUserParent->GetPublicID().c_str(),  rkPacket.GetPacketID() );
		return false; 
	}

	int iValue1    = 0;
	int iValue2    = 0;
	rkPacket >> iValue1 >> iValue2;
	UserCopyNode *pUser = static_cast<UserCopyNode*> ( pUserParent );
	SP2Packet kPacket( dwSSMsgType );
	kPacket << pUser->GetUserIndex() << iValue1 << iValue2;
	pUser->SendMessage( kPacket );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CWindow::OnWebUDPParse Send Other Server : %s:0x%x:0x%x", pUser->GetPublicID().c_str(), rkPacket.GetPacketID(), dwSSMsgType );
	return true;
}

bool ioMainProcess::OnRelayUDPParse( sockaddr_in *client_addr,SP2Packet &rkPacket ) //������ ������ �κ� 
{
	SP2Packet kPacket = rkPacket;
	if( !COMPARE( rkPacket.GetPacketID(), CUPK_CONNECT, 0x5000 ) )
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL,"�߰踦 ������� �ʴ� ��Ŷ ����!![ 0x%x ]", rkPacket.GetPacketID() );
		return false;
	}

	DWORD dwIP, dwPort;
	rkPacket >> dwIP >> dwPort;
	if( dwIP == 0 || dwPort == 0 )
	{
		// �������� ������ �볻 �����鿡�� TCP Send
		DWORD dwUserIndex;
		rkPacket >> dwUserIndex;
		User *pUser = g_UserNodeManager.GetUserNode( dwUserIndex );
		if( pUser && pUser->IsConnectState() )
		{
			pUser->PacketParsing( rkPacket );
		}
		g_EtcLogMgr.PlusUDPTransferTCP();
		return true;
	}
	else if( dwIP == g_App.GetDwordIP() && dwPort == (DWORD)g_App.GetCSPort() )
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL,"UDP ��Ŷ �ڱ� �ڽſ��� ��������[ 0x%x ]", rkPacket.GetPacketID() );
		return false;
	}

	char szIP[16] = "";
	sprintf_s( szIP, "%d.%d.%d.%d", (dwIP & 0xff000000)>>24, (dwIP & 0x00ff0000)>>16, (dwIP & 0x0000ff00)>>8,	(dwIP & 0xff) );
	
 
	g_UDPNode.SendMessage( szIP, dwPort, kPacket );
	g_EtcLogMgr.PlusUDPTransfer();
	return true;
}

DWORD ioMainProcess::StrToDwordIP( const char *iip )
{
	int  count       = 0;
	int  cut_ip		 = 0;
	char szCut_ip[4][4];
	memset(szCut_ip,0,sizeof(szCut_ip));
	int  len	     = strlen(iip);
	for(int i = 0;i < len;i++)
	{
		if(iip[i] == '.')
		{
			count = 0;
			cut_ip++;
		}
		else
			szCut_ip[cut_ip][count++] = iip[i];
	}
	return (DWORD)(atoi(szCut_ip[0])<<24) | (DWORD)(atoi(szCut_ip[1])<<16) | (DWORD)(atoi(szCut_ip[2])<<8) | atoi(szCut_ip[3]);	
}

void ioMainProcess::ClientBindStart()
{
	if( m_bWantExit || m_bReserveLogOut ) return;
	if( m_pClientBind ) return;

	m_pClientBind = new ioClientBind;
	if( !m_pClientBind->Start( m_szPublicIP.c_str(), m_iCSPort ) )
		return;

	char strText[MAX_PATH] = "";
	sprintf_s( strText, "%s:%d:%s:%d", m_szPublicIP.c_str(), m_iCSPort, m_szPrivateIP.c_str(), m_iSSPort );
	SetConsoleTitle( strText );
}

void ioMainProcess::ServerBindStart()
{
	if( m_bWantExit || m_bReserveLogOut ) return;
	if( m_pServerBind ) return;

	m_pServerBind = new ioServerBind;
	if( !m_pServerBind->Start( m_szPrivateIP.c_str(), m_iSSPort ) )
		return;
}

void ioMainProcess::MonitoringBindStart()
{
	if( m_bWantExit || m_bReserveLogOut ) return;
	if( m_pMonitoringBind ) return;
	if( !m_bUseMonitoring ) return;

	m_pMonitoringBind = new ioMonitoringBind;
	if( !m_pMonitoringBind->Start( "127.0.0.1", m_iMSPort ) ) // ����͸� Ŭ���̾�Ʈ ���� �������� ����
		return;
}

bool ioMainProcess::IsRightID( const char *iid )
{
	int iIDSize = strlen( iid );
#if !defined (_LSWC)
	if( (iIDSize > ID_NUMBER) || (iIDSize < MIN_ID_NUMBER) )
		return false;
#endif
	for(int i = 0;i < iIDSize;i++)
	{
		if( IsDBCSLeadByte( iid[i] ) ) 
		{
			i++;
			if(iIDSize <= i) // ������ ���ڰ� ���� ���ڴ�.
				return false;
		}
	}
	return true;
}

bool ioMainProcess::IsRightFirstID( const char *szID )
{
	enum { MIN_SIZE = 4, };

	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( !pLocal )
		return false;

	int size = strlen(szID);
	if ((size > pLocal->GetFirstIDMaxSize() )||(size < MIN_SIZE))
		return false;
	for (int i=0; i<size; i++)
	{
		if ((!COMPARE(szID[i], 'A', 'Z'+1)) &&
			(!COMPARE(szID[i], 'a', 'z'+1)) &&
			(!COMPARE(szID[i], '0', '9'+1)) )
		{
			if( pLocal->IsCheckKorean() )
			{
				// �ѱ��� ��� ���� ���
				if (i < size-1)
				{
					// �ѱ� ���� ���
					if ((byte)szID[i]==0xa4&&(byte)szID[i+1] >= 0xa1 && (byte)szID[i+1] <= 0xd3)
					{
						return false;
					}
					if ((byte)szID[i]>=0xb0 && (byte)szID[i]<=0xc8 && (byte)szID[i+1] >= 0xa1 && (byte)szID[i+1] <= 0xfe)
					{
						i++;
						continue;
					}
				}
			}
			else if( pLocal->IsFirstIDCheckPass() )
			{
				i++;
				continue;
			}
			return false;
		}
	}
	return true;
}

void ioMainProcess::ProcessTime()
{
	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����
	if(m_bReserveLogOut)
	{
		if(m_dwReserveLogOutTime != 0)
		{
			if(TIMEGETTIME() - m_dwReserveLogOutTime > WAIT_TIME_FOR_RESERVE_LOGOUT )
			{
				m_bWantExit = true;
				m_dwReserveLogOutTime = 0;

				Save();
			}
		}
	}

	g_TestNodeMgr.OnUpdate();
	ConnectUserCountUpdate();
	ProcessCreateNewLog();

#ifdef SRC_LATIN
	g_ioApex.GetDataFromAS();
#endif
}

void ioMainProcess::CheckLogAllSave()
{
	LOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	LOG.CloseLog();
	HackLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	HackLOG.CloseLog();
	ProcessLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	ProcessLOG.CloseLog();
	EventLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	EventLOG.CloseLog();
	P2PRelayLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	P2PRelayLOG.CloseLog();
	RateCheckLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	RateCheckLOG.CloseLog();
	TradeLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	TradeLOG.CloseLog();
	CriticalLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	CriticalLOG.CloseLog();
	WemadeLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	WemadeLOG.CloseLog();
#ifdef ANTIHACK
 	CheatLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
 	CheatLOG.CloseLog();
	CheatUser.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
	CheatUser.CloseLog();
#endif
}

void ioMainProcess::CheckCreateNewLog( bool bStart )
{
	static char szPrevTime[MAX_PATH] = "";

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szCurTime[MAX_PATH] = "";
	StringCbPrintf(szCurTime, sizeof(szCurTime), "%04d%02d%02d", st.wYear, st.wMonth, st.wDay );

	if( strcmp(szCurTime, szPrevTime) != 0)
	{
		if( !bStart )
		{
			g_EtcLogMgr.WriteLOG();
			g_PacketChecker.WriteLOG();
		}

		// set ip port
		ioHashString szIP;
		int          iCSPort = 0;
		if( bStart )
		{
			szIP    = m_szPublicIP;

			iCSPort = GetPrivateProfileInt("Default", "CSPORT", 9000, g_App.GetINI().c_str());	
		}
		else
		{
			szIP    = m_szPublicIP;
			iCSPort = m_iCSPort;
		}

// �ؿܿ� ���� ���̼��� üũ
#if defined( SRC_OVERSEAS )
		// ������ ���̵𿡸� 30�� ������ ���̼����� ���� ������ ����͸� ���� ��ȭ���ڷ� ���
		if( !bStart )
			CheckLicenseForDev();
#endif

		memset(szPrevTime, 0, sizeof(szPrevTime));
		StringCbCopy(szPrevTime, sizeof(szPrevTime), szCurTime);

		char TimeLogName[MAX_PATH]="";
		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\BUG%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort );
		{			
			LOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			LOG.CloseLog();
			LOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);	
			LOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\HACK%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			HackLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			HackLOG.CloseLog();
			HackLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			HackLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			HackLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\PROCESS%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			ProcessLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			ProcessLOG.CloseLog();
			ProcessLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			ProcessLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			ProcessLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
			ProcessLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[Main][LogDB][UDP][ClientA][ServerA][Work]" );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\EVENT%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			EventLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			EventLOG.CloseLog();
			EventLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			EventLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			EventLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\P2PRELAY%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			P2PRelayLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			P2PRelayLOG.CloseLog();
			P2PRelayLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			P2PRelayLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			P2PRelayLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\RateCheck%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			RateCheckLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			RateCheckLOG.CloseLog();
			RateCheckLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			RateCheckLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			RateCheckLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\Trade%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			TradeLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			TradeLOG.CloseLog();
			TradeLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			TradeLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			TradeLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\Critical%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			CriticalLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			CriticalLOG.CloseLog();
			CriticalLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			CriticalLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			CriticalLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}

		memset(TimeLogName, 0, sizeof(TimeLogName));
		StringCbPrintf(TimeLogName, sizeof(TimeLogName), "%s\\LOG%s-%s-%d.log", GetLogFolder(), szCurTime, szIP.c_str(), iCSPort);
		{
			WemadeLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			WemadeLOG.CloseLog();
			WemadeLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			WemadeLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			WemadeLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Version : [%s] [%s]", STRFILEVER, STRINTERNALNAME );
		}
#ifdef ANTIHACK
		memset(TimeLogName, 0, sizeof(TimeLogName));
		sprintf_s(TimeLogName, sizeof(TimeLogName), "%s\\[%s]AntiCheat.log", GetLogFolder(), szCurTime );
		{
			CheatLOG.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			CheatLOG.CloseLog();
			CheatLOG.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			CheatLOG.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			CheatLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Version : %s | %s", STRFILEVER, STRINTERNALNAME );
		}
#endif
#ifdef ANTIHACK
		memset(TimeLogName, 0, sizeof(TimeLogName));
		sprintf_s(TimeLogName, sizeof(TimeLogName), "%s\\[%s]CheatUser.log", GetLogFolder(), szCurTime );
		{
			CheatUser.PrintTimeAndLog(0, "<<< --------------------  End File -------------------- >>>\r\n");
			CheatUser.CloseLog();
			CheatUser.OpenLog( LOG_RELEASE_LEVEL, TimeLogName, true);
			CheatUser.PrintTimeAndLog(0, "<<< --------------------  Create File -------------------- >>>\r\n");
			CheatUser.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Version : %s | %s", STRFILEVER, STRINTERNALNAME );
		}
#endif
	}
}

bool ioMainProcess::Startup(const char* scriptName)
{
	SetINI(scriptName);

	const char* szINI = GetINI().c_str();
	GetPrivateProfileString("Default", "Log", "MLOG", m_szLogFolder, sizeof(m_szLogFolder), szINI);	

	if(!BeginSocket())
	{
		return false;
	}

	{
		// SET IP
		ioINILoader kLoader( "ls_config_game.ini" );
	

		char szValue[MAX_PATH];
		kLoader.LoadString("NETWORK","SocketIP", "", szValue, MAX_PATH);
		ioHashString iPrivateIPFirstByte(szValue); //converting char to string

		if (strcmp(iPrivateIPFirstByte.c_str(), "") == 0)
		{
			MessageBox(NULL, "Socket IP cannot be empty !", "IOEnter", MB_ICONERROR);
			return FALSE;
		}

		if (!SetLocalIP2(iPrivateIPFirstByte))
		{

			return FALSE;
		}
	}

	CheckCreateNewLog( true );
	return true;
}

void ioMainProcess::PrintTimeAndLog(int debuglv, LPSTR fmt )
{
	LOG.PrintTimeAndLog( debuglv, fmt );
}

void ioMainProcess::DebugLog(int debuglv, LPSTR filename, int linenum, LPSTR fmt )
{
	LOG.DebugLog( debuglv, filename, linenum, fmt );
}

void ioMainProcess::InitRoomPosCheckCnt()
{
	m_dwRoomToRoomCnt = 0;
	m_dwEtcToRoomCnt = 0;
}

void ioMainProcess::IncreaseRoomPosCheckCnt( int iType )
{
	if( iType == 1 )
		m_dwRoomToRoomCnt++;
	else
		m_dwEtcToRoomCnt++;

	if( m_dwRoomToRoomCnt+m_dwEtcToRoomCnt > 1000 )
		SendRoomPosCheckCnt();
}

void ioMainProcess::SendRoomPosCheckCnt()
{
	char szLog[2048] = "";
	sprintf_s( szLog, "������ġ üũ - ��:%d, ��Ÿ:%d", m_dwRoomToRoomCnt, m_dwEtcToRoomCnt );
	SP2Packet kPacket( LUPK_LOG );
	kPacket << "CheckError";
	kPacket << szLog;
	kPacket << 900;  // ������ȣ
	kPacket << false; // write db
	g_UDPNode.SendLog( kPacket );

	InitRoomPosCheckCnt();
}


// �ؿܿ� ���� ���̼��� üũ
#if defined( SRC_OVERSEAS )
// ������ ���̵𿡸� 30�� ������ ���̼����� ���� ������ ����͸� ���� ��ȭ���ڷ� ���
void ioMainProcess::CheckLicenseForDev()
{
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
	{
		int iLicenseDate, iLocalDate, iResultDay = 0;
		SYSTEMTIME sysTimeLicense, sysTimeLocal;

		char chLocalName[256] = {0,};
		sprintf_s( chLocalName, "%s", ioLocalManager::GetLocalName( ioLocalManager::GetLocalType() ) );

		// ���̼��� ��¥
		iLicenseDate			= pLocal->GetLicenseDate();
		sysTimeLicense.wYear	= iLicenseDate / 10000;
		sysTimeLicense.wMonth	= ( iLicenseDate - sysTimeLicense.wYear * 10000 ) / 100;
		sysTimeLicense.wDay		= ( iLicenseDate - sysTimeLicense.wYear * 10000 ) % 100;

		// ���� ���� ��¥
		GetLocalTime( &sysTimeLocal );
		iLocalDate = sysTimeLocal.wYear * 10000 + sysTimeLocal.wMonth * 100 + sysTimeLocal.wDay;

		// ���̼��� ��¥�� ���� ���� ��¥�� ���̸� �޾ƿ´�.
		iResultDay = Help::GetDatePeriod( sysTimeLocal.wYear, sysTimeLocal.wMonth, sysTimeLocal.wDay, 0, 0 
			, sysTimeLicense.wYear, sysTimeLicense.wMonth, sysTimeLicense.wDay, 0, 0, Help::PT_DAY );

		// 30�� ������ ��� �޼���
		if( 30 >= iResultDay )
		{
			SP2Packet alertPacket( MSTPK_LICENSE_ALERT_CHECK );
			alertPacket << iResultDay;		// ��¥ ����		ex) 30
			alertPacket << iLicenseDate;	// ���̼��� ��¥	ex) 20140101
			alertPacket << iLocalDate;		// ������� ��¥	ex) 20140101
			alertPacket << chLocalName;		// ������

			// ���Ӽ��� -> ���μ��� -> ����͸� ��
			g_MainServer.SendMessage( alertPacket );
		}			
	}
}
#endif