#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "../Shutdown.h"
#include "../EtcHelpFunc.h"
#include "../Network/GameServer.h"
#include "../Network/iocpHandler.h"
#include "../Network/ioPacketQueue.h"
#include "../QueryData/QueryResultData.h"
#include "../NodeInfo/UserNodeManager.h"
#include "../NodeInfo/RoomNodeManager.h"
#include "../NodeInfo/BattleRoomManager.h"
#include "../NodeInfo/ShuffleRoomManager.h"
#include "../NodeInfo/ServerNodeManager.h"
#include "../NodeInfo/AnnounceMgr.h"
#include "../NodeInfo/LadderTeamManager.h"
#include "../NodeInfo/ioPresentHelper.h"
#include "../NodeInfo/TradeInfoManager.h"
#include "../NodeInfo/ioExtraItemGrowthCatalystMgr.h"
#include "../NodeInfo/ioItemInfoManager.h"
#include "../NodeInfo/ioDecorationPrice.h"
#include "../NodeInfo/ioEtcItemManager.h"
#include "../NodeInfo/ioExcavationManager.h"
#include "../NodeInfo/FishingManager.h"
#include "../NodeInfo/ioItemCompoundManager.h"
#include "../NodeInfo/LevelMatchManager.h"
#include "../NodeInfo/ioItemInitializeControl.h"
#include "../NodeInfo/ChannelNodeManager.h"
#include "../NodeInfo/ioExerciseCharIndexManager.h"
#include "../NodeInfo/ioAlchemicMgr.h"
#include "../NodeInfo/TournamentManager.h"
#include "../NodeInfo/ioSuperGashaponMgr.h"
#include "../NodeInfo/TestNodeManager.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../WemadeLog/ioWemadeLogger.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "../NodeInfo/ioPetInfoManager.h"
#include "../NodeInfo/ioPetGashaponManager.h"
#include "../NodeInfo/SpecialGoodsManager.h"
#include "../NodeInfo/MissionManager.h"
#include "../NodeInfo/GuildRewardManager.h"
#include "../NodeInfo/CostumeShopGoodsManager.h"
#include "../NodeInfo/CostumeManager.h"
#include "../NodeInfo/ioMedalItemInfoManager.h"
#include "../NodeInfo/ioSetItemInfoManager.h"
#include "../NodeInfo/GuildRoomsBlockManager.h"
#include "../NodeInfo/CompensationMgr.h"
#include "../Local/ioLocalParent.h"
#include "../NodeInfo/CompensationMgr.h"
#include "../NodeInfo/TradeSyncManger.h"
#include "../NodeInfo/AccessoryManager.h"
#include "MainServerNode.h"

#include <strsafe.h>
#include <algorithm>

#ifdef XTRAP
#include "../Xtrap/ioXtrap.h"
#endif

extern CLog TradeLOG;
extern ioWemadeLogger g_WemadeLogger;
extern BOOL tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens);


MainServerNode *MainServerNode::sg_Instance = NULL;
MainServerNode::MainServerNode( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize ) : CConnectNode( s, dwSendBufSize, dwRecvBufSize )
{
	m_iMainPort = 0;
	InitData();
}

MainServerNode::~MainServerNode()
{	
}

MainServerNode &MainServerNode::GetInstance()
{
	if(sg_Instance == NULL)
	{
		ioINILoader kLoader( "ls_config_game.ini" );
		kLoader.SetTitle( "MainServer Session" );
		int iSendBufferSize = kLoader.LoadInt( "SendBufferSize", MAX_BUFFER );
		sg_Instance = new MainServerNode( INVALID_SOCKET, iSendBufferSize, MAX_BUFFER * 2 );

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][memorypool]Main server session send buffer : [%d]", iSendBufferSize );
	}
	return *sg_Instance;
}

void MainServerNode::ReleaseInstance()
{		
	SAFEDELETE( sg_Instance );
}

bool MainServerNode::ConnectTo()
{
	Information( "Mainserver connect .. " );

	ioINILoader kLoader( "../global_define.ini" );
	kLoader.SetTitle( "main" );

	char szValue[MAX_PATH];
	kLoader.LoadString( "1", "", szValue, MAX_PATH );

	std::string values = szValue;
	std::string delimiter = ":";
	std::vector<std::string> tokens;
	tokenize(values, delimiter, tokens);
	if(tokens.size() != 2) return false;

	m_szMainIP = tokens[0].c_str();
	m_iMainPort= atoi(tokens[1].c_str());

	SOCKET socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( socket == INVALID_SOCKET )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MainServerNode::ConnectTo socket %d[%s:%d]", GetLastError(), m_szMainIP.c_str(), m_iMainPort );
		Information( "failed\n" );
		return false;
	}
	sockaddr_in serv_addr;
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr( m_szMainIP.c_str() );
	serv_addr.sin_port			= htons( m_iMainPort );

	if( ::connect( socket, (sockaddr*)&serv_addr, sizeof(serv_addr) ) != 0 )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MainServerNode::ConnectTo connect %d[%s:%d]", GetLastError(), m_szMainIP.c_str(), m_iMainPort );
		Information( "failed\n" );
		return false;
	}

	g_iocp.AddHandleToIOCP( (HANDLE)socket, (DWORD)this );
	CConnectNode::SetSocket( socket );
	CConnectNode::SetNagleAlgorithm( false );
	
	OnCreate();	
	AfterCreate();
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][mainserver]OnConnect : [%s] [%d] [%d]", m_szMainIP.c_str(), m_iMainPort, 0 );

	Information( "done\n" );
	
	SendNodeInfoRequest();
	return true;
}

void MainServerNode::InitData()
{
	m_dwCurrentTime = 0;
	m_dwLastSendServerUpdateTime = 0;

	m_iHeadquartersUserCount = 0;
	m_iSafetySurvivalRoomUserCount =0;
	m_iPlazaUserCount		 = 0;
	m_iBattleRoomUserCount	 = 0;
	m_iLadderBattleUserCount  = 0;

	m_iTotalUserRegCount     = 0;
}

void MainServerNode::OnCreate()
{
	CConnectNode::OnCreate();
	InitData();

	m_dwCurrentTime = TIMEGETTIME();
}

void MainServerNode::OnDestroy()
{
	CConnectNode::OnDestroy();

	g_CriticalError.CheckMainServerDisconnect();
	g_UserNodeManager.AllUserUpdateLadderPointNExpert();

	Information( "MainServer Connection Close..\n" );
}

void MainServerNode::SessionClose(BOOL safely)
{
	if(!safely)
	{
		g_CriticalError.CheckMainServerExceptionDisconnect( GetLastError() );
	}
	OnDestroy();
}

bool MainServerNode::SendMessage( CPacket &rkPacket )
{
	g_ProcessChecker.MainServerSendMessage( rkPacket.GetPacketID(), rkPacket.GetBufferSize() );
	return CConnectNode::SendMessage( rkPacket, TRUE );
}

bool MainServerNode::CheckNS( CPacket &rkPacket )
{
	return true;             //��Ʈ�� ���� �ʿ����.
}

int MainServerNode::GetConnectType()
{
	return CONNECT_TYPE_MAIN_SERVER;
}

void MainServerNode::ProcessTime()
{	
	if( g_App.IsWantExit() ) return;

	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	if( TIMEGETTIME() - m_dwCurrentTime >= DEFAULT_SERVER_UPDATE_TIME )       
	{
		if( !IsActive() )
		{
			ConnectTo();
		}
		else
		{
			//hr ���μ����� ����
			// DEFAULT_SERVER_UPDATE_TIME �� ���� ���� ���� ����
			
			SP2Packet kPacket( MSTPK_SERVER_UPDATE );
			
			DWORD dwPlazaNHQSize		= g_RoomNodeManager.GetPlazaNodeSize() + g_RoomNodeManager.GetHeadquartersNodeSize();
			DWORD dwBattleNLadderSize   = g_BattleRoomManager.GetNodeSize() + g_LadderTeamManager.GetNodeSize();

			//��� ����
			kPacket << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_WEMADEBUY) << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_MGAME ) 
				    << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_DAUM )  << 0 // ���� �����Ǿ� 0���� ǥ��
					<< g_UserNodeManager.GetNodeSizeByChannelingType( CNT_NAVER ) << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_TOONILAND ) 
					<< g_UserNodeManager.GetNodeSizeByChannelingType( CNT_NEXON ) << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_HANGAME );
			if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_US ) //�Ϲ��� ��� ���� ccu ����
			{
				kPacket << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_STEAM );
			}
				kPacket << g_RoomNodeManager.GetRoomNodeSize() << dwPlazaNHQSize << dwBattleNLadderSize;			
			if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_US ) //�Ϲ��� ��� ���� ccu ����
			{
				kPacket << g_UserNodeManager.GetNodeSizeByChannelingType( CNT_STEAM );
			}
				    kPacket << g_RoomNodeManager.GetRoomNodeSize() << dwPlazaNHQSize << dwBattleNLadderSize;	

			//��ġ ����
			kPacket << g_RoomNodeManager.GetSafetySurvivalRoomUserCount() 
				    << ( g_RoomNodeManager.GetPlazaUserCount() + g_RoomNodeManager.GetHeadquartersUserCount() )
				    << g_BattleRoomManager.GetBattleRoomUserCount() << g_BattleRoomManager.GetBattleRoomPlayUserCount()
					<< g_LadderTeamManager.GetLadderTeamUserCount();

			// ��Ÿ
			kPacket << g_App.GetDBQueryTime();
			
			//�����̼��� ���� �߰� 
			kPacket << g_Relay.IsUsingRelayServer();


			//����� ���� �ε�����.
			g_ServerNodeManager.FillAllServerIndex( kPacket );
			SendMessage( kPacket );

			 //������ ccu �� ���� ����
			if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_EU )
				SendCountryCCU();

			//������ �ð�
			m_dwLastSendServerUpdateTime = TIMEGETTIME();
		}		
		m_dwCurrentTime = TIMEGETTIME();
	}
}

void MainServerNode::SendCountryCCU() 
{
	//HRYOON �߰� �����ڵ�
	// ���� - ���� - ������ - ��Ż���� -������ - ��Ű
	//
	SP2Packet kPacket( MSTPK_SERVER_COUNTRY_UPDATE );
	/*
	kPacket << g_UserNodeManager.GetNodeSizeByCountryType( EUCT_ENGLISH ) << g_UserNodeManager.GetNodeSizeByCountryType( EUCT_DEUTSCH )
			<< g_UserNodeManager.GetNodeSizeByCountryType( EUCT_FRANCH )  << g_UserNodeManager.GetNodeSizeByCountryType( EUCT_ITALIANO )
			<< g_UserNodeManager.GetNodeSizeByCountryType( EUCT_POLISKY ) << g_UserNodeManager.GetNodeSizeByCountryType( EUCT_TURKEY )
			<< g_UserNodeManager.GetNodeSizeByCountryType( EUCT_NONE );
	*/
	//hr ������ ��� �����ڵ� ���� ccu ��Ŷ ���� ���μ����� ����
	ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
	{
		pLocal->FillCountryCCU( kPacket );
	}


	g_ServerNodeManager.FillAllServerIndex( kPacket );
	SendMessage( kPacket );
}

void MainServerNode::SendReserveCloseAnn( CConnectNode *pConnectUser )
{
	if( !pConnectUser || m_szReserveCloseAnn.IsEmpty() ) return;

	SP2Packet kPacket( STPK_ANNOUNCE );
	kPacket << m_szReserveCloseAnn;
	pConnectUser->SendMessage( kPacket );
}

void MainServerNode::ReceivePacket( CPacket &packet )
{
	g_RecvQueue.InsertQueue( (DWORD)this, packet, PK_QUEUE_SESSION );
}

void MainServerNode::PacketParsing( CPacket &packet )
{
	SP2Packet &kPacket = (SP2Packet&)packet;

	FUNCTION_TIME_CHECKER( 500000.0f, kPacket.GetPacketID() );          // 0.5 �� �̻� �ɸ���α� ����

	switch( kPacket.GetPacketID() )
	{
	case MSTPK_SERVER_CONNECT:
		OnMainServerConnect( kPacket );
		break;
	case MSTPK_SERVER_INDEX:
		OnServerIndex( kPacket );
		break;
	case MSTPK_ALL_SERVER_LIST:
		OnAllServerList( kPacket );
		break;
	case MSTPK_CLASS_PRICE_INFO:
		OnClassPrice( kPacket );
		break;
	case MSTPK_SERVER_UPDATE:
		OnServerUpdate( kPacket );
		break;
	case MSTPK_TOTAL_REG_USER_CNT:
		OnTotalUserReg( kPacket );
		break;
	case MSTPK_GUILD_RANK_LIST:
		OnGuildRankList( kPacket );
		break;
	case MSTPK_GUILD_INFO:
		OnGuildInfo( kPacket );
		break;
	case MSTPK_GUILD_JOINER_CHANGE:
		OnGuildChangeJoiner( kPacket );
		break;
	case MSTPK_GUILD_SIMPLE_INFO:
		OnGuildSimpleInfo( kPacket );
		break;
	case MSTPK_GUILD_EXIST:
		OnGuildExist( kPacket ); 
		break;
	case MSTPK_GUILD_MARK_BLOCK_INFO:
		OnGuildMarkBlockInfo( kPacket );
		break;
	case MSTPK_GUILD_TITLE_SYNC:
		OnGuildTitleSync( kPacket );
		break;
	case MSTPK_GUILD_ENTRY_AGREE:
		OnGuildEntryAgreeResult(kPacket);
		break;
	case MSTPK_CAMP_ROOM_BATTLE_INFO:
		OnCampRoomBattleInfo( kPacket );
		break;
	case MSTPK_CAMP_DATA_SYNC:
		OnCampDataSync( kPacket );
		break;
	case MSTPK_CAMP_BATTLE_INFO:
		OnCampBattleInfo( kPacket );
		break;
	case MSTPK_USER_CAMP_POINT_RECORD_INIT:
		OnUserCampPointNRecordInit( kPacket );
		break;
	case MSTPK_CAMP_INFLUENCE_ALARM:
		OnCampInfluenceAlarm( kPacket );
		break;
	case MSTPK_LOW_CONNECT_SERVER_EXIT:
		OnLowConnectExit( kPacket );
		break;

	case MSTPK_TRADE_LIST:
		OnTradeList( kPacket );
		break;
	case MSTPK_TRADE_ITEM_TRADE:
		OnTradeItemComplete( kPacket );
		break;
	case MSTPK_TRADE_ITEM_CANCEL:
		OnTradeItemCancel( kPacket );
		break;
	case MSTPK_TRADE_TIME_OUT:
		OnTradeItemTimeOut( kPacket );
		break;
	
	case MSTPK_EVENT_SHOP_GOODS_LIST:
		OnEventShopGoodsList( kPacket );
		break;
	case MSTPK_EVENT_SHOP_GOODS_BUY:
		OnEventShopGoodsBuy( kPacket );
		break;

	case MSTPK_DBAGENT_EXTEND:
		OnDBAgentExtend( kPacket );
		break;
	case MSTPK_GAME_SERVER_RELOAD_INI:
		OnGameServerReloadINI( kPacket );
		break;
	case MSTPK_GAME_SERVER_OPTION:
		OnGameServerOption( kPacket );
		break;
	case MSTPK_EXTRAITEM_GROWTH_CATALYST_DATA:
		OnExtraItemGrowthCatalyst( kPacket );
		break;
	case MSTPK_EXTRAITEM_GROWTH_MORTMAIN_CHECK:
		OnExtraItemGrowthMortmainCheck( kPacket );
		break;
	case MSTPK_EXTRAITEM_GROWTH_CATALYST_INFO:
		OnExtraItemGrowthMortmainInfo( kPacket );
		break;

		// Tool
	case MSTPK_SERVER_INFO_REQ:
		{
			OnServerInfoRequest( kPacket );
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_HACK_CONSTANT:
		{
			HackCheck::LoadHackCheckValues();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_USER_DISPERSION:
		{
			g_ServerNodeManager.LoadINI();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_PROCESS_INI:
		{
			g_ProcessChecker.LoadINI();
			g_PacketChecker.LoadINI();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_INI_CLASS:
		{
			// INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_ItemPriceMgr.LoadPriceInfo( "config/sp2_item_price.ini" , false );
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_INI_DECO:
		{
			// INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_DecorationPrice.LoadPriceInfo( false );
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_INI_ETC:
		{
			// INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_EtcItemMgr.LoadEtcItem( "config/sp2_etcitem_info.ini", false );
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_INI_EVENT:
		{
			// INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_EventMgr.LoadINI( false );
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_QUEST_INI:
		{
			g_QuestMgr.LoadINIData();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_MODE_INI:
		{
			g_ModeINIMgr.ReloadINIData();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_PRESENT_INI:
		{
			g_PresentHelper.CheckNeedReload();
			g_SuperGashaponMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_FISHING_INI:
		{
			g_FishingMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_EXCA:
		{
			g_ExcavationMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_ITEM_COMPOUND_INI:
		{
			g_CompoundMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_EXTRAITEM_INI:
		{
			g_ExtraItemInfoMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_TRADE_INI:
		{
			g_TradeInfoMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_LEVELMATCH_INI:
		{
			g_LevelMatchMgr.CheckNeedReload();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_ITEM_INIT_CONTROL:
		{
			g_ItemInitControl.LoadINIData();
		}
		break;

	case MSTPK_GAMESERVER_RELOAD_CONFIG_INI:
		{
			Help::InitHelpConstant();
		}
		break;

	case MSTPK_GAMESERVER_QUICK_EXIT:
		{
			g_App.Shutdown(SHUTDOWN_QUICK);

			//ioINILoader kLoader( "config/sp2_auto_run.ini" );
			//kLoader.SaveBool( "config", "AutoRun", false );
		}
		break;

	case MSTPK_GAMESERVER_SAFETY_EXIT:
		{
			g_App.Shutdown(SHUTDOWN_SAFE);

			//ioINILoader kLoader( "config/sp2_auto_run.ini" );
			//kLoader.SaveBool( "config", "AutoRun", false );
		}
		break;

	case MSTPK_GAMESERVER_SETNAGLE_REFCOUNT:
		{
			int refCount = 0;
			kPacket >> refCount;
			User::LoadNagleReferenceValue( refCount );
		}
		break;

	case MSTPK_GAMESERVER_SETNAGLE_TIME:
		{
			uint32 nagleTime = 0;
			kPacket >> nagleTime;
			g_App.SetNagleTime( nagleTime );
		}
		break;

	case MSTPK_TOURNAMENT_REGULAR_INFO:
		OnTournamentRegularInfo( kPacket );
		break;
	case MSTPK_TOURNAMENT_MAIN_INFO:
		OnTournamentMainInfo( kPacket );
		break;
	case MSTPK_TOURNAMENT_LIST_REQUEST:
		OnTournamentListRequest( kPacket );
		break;

	// �Ŵ��� �� - > ���� ���� - > ���� ���� 
	case MSTPK_ADMIN:
		//OnAnnounce( kPacket );
		OnAdminCommand( kPacket );
		break;
	// �Ŵ��� �� - > ���� ���� - > ���� ���� OR ���� ���� - > ���� ����
	case MSTPK_UPDATE_CLIENT_VERSION:
		OnUpdateClientVersion( kPacket );
		break;
	// �Ŵ��� �� - > ���� ���� - > ���� ����
	case MSTPK_LOAD_CS3_FILE:
		OnLoadCS3File( kPacket );
		break;
	case MSTPK_CS3_FILE_VERSION:
		OnCS3FileVersion( kPacket );
		break;
	case MSTPK_AUTO_CLOSE_ANNOUNCE:
		OnAutoCloseAnnounce( kPacket );
		break;
	case MSTPK_SERVER_PING_CHECK:
		OnServerPingCheck( kPacket );
		break;
	case MSTPK_TOURNAMENT_TEAM_CREATE:
		OnTournamentCreateTeam( kPacket );
		break;
	case MSTPK_TOURNAMENT_TEAM_INFO:
		OnTournamentTeamInfo( kPacket );
		break;
	case MSTPK_TOURNAMENT_SERVER_SYNC:
		OnTournamentServerSync( kPacket );
		break;
	case MSTPK_TOURNAMENT_END_PROCESS:
		OnTournamentEndProcess( kPacket );
		break;
	case MSTPK_TOURNAMENT_SCHEDULE_INFO:
		OnTournamentScheduleInfo( kPacket );
		break;
	case MSTPK_TOURNAMENT_TEAM_POSITION_SYNC:
		OnTournamentTeamPositionSync( kPacket );
		break;
	case MSTPK_TOURNAMENT_ROUND_TEAM_DATA:
		OnTournamentRoundTeamData( kPacket );
		break;
	case MSTPK_TOURNAMENT_ROUND_START:
		OnTournamentRoundStart( kPacket );
		break;
	case MSTPK_TOURNAMENT_BATTLEROOM_INVITE:
		OnTournamentBattleRoomInvite( kPacket );
		break;
	case MSTPK_TOURNAMENT_BATTLE_TEAM_CHANGE:
		OnTournamentBattleTeamChange( kPacket );
		break;
	case MSTPK_TOURNAMENT_CUSTOM_CREATE:
		OnTournamentCustomCreate( kPacket );
		break;
	case MSTPK_TOURNAMENT_TEAM_ALLOCATE_LIST:
		OnTournamentTeamAllocateList( kPacket );
		break;
	case MSTPK_TOURNAMENT_TEAM_ALLOCATE_DATA:
		OnTournamentTeamAllocateData( kPacket );
		break;
	case MSTPK_TOURNAMENT_JOIN_CONFIRM_CHECK:
		OnTournamentJoinConfirmCheck( kPacket );
		break;
	case MSTPK_TOURNAMENT_ANNOUNCE_CHANGE:
		OnTournamentAnnounceChange( kPacket );
		break;
	case MSTPK_TOURNAMENT_TOTAL_TEAM_LIST:
		OnTournamentTotalTeamList( kPacket );
		break;
	case MSTPK_TOURNAMENT_CUSTOM_STATE_START:
		OnTournamentCustomStateStart( kPacket );
		break;
	case MSTPK_TOURNAMENT_CUSTOM_REWARD_LIST:
		OnTournamentCustomRewardList( kPacket );
		break;
	case MSTPK_TOURNAMENT_CUSTOM_REWARD_REG_CHECK:
		OnTournamentCustomRewardRegCheck( kPacket );
		break;
	case MSTPK_NODEINFO_REQUEST:
		OnNodeInfoResponse(kPacket);
		break;
	case MSTPK_EVENT_NPC_CLOSE_REQUEST:
		OnNpcEventRequest(kPacket);
		break;
	case MSTPK_GAMESERVER_CHANGE_RELAYSERVER_STATE:
		{
			OnRelayServerChangeState(kPacket);
		}
		break;
	case MSTPK_UPDATE_RELATIVE_GRADE:
		OnUpdateRelativeGrade( kPacket );
		break;
	case MSTPK_SUPER_GASHPON_LIMIT_CHECK:
		OnSuperGashponLimitCheckResult( kPacket );
		break;
	case MSTPK_SUPER_GASHPON_LIMIT_INFO:
		OnSuperGashponLimitInfo( kPacket );
		break;
	case MSTPK_TOURNAMENT_CHEER_DECISION:
		OnTournamentCheerDecision( kPacket );
		break;
	case MSTPK_TOURNAMENT_PREV_CHAMP_SYNC:
		OnTournamentPrevChampSync( kPacket );
		break;
	case MSTPK_WHITELIST_REQUEST:
		OnWhiteListRequest( kPacket );
		break;
	case MSTPK_SPECIAL_SHOP_GOODS_INFO:
		OnSpecialShopGoodsInfo( kPacket );
		break;
	case MSTPK_SPECIAL_SHOP_GOODS_BUY:
		OnSpecialShopGoodsBuy( kPacket );
		break;
	case MSTPK_SPECIAL_SHOP_STATE_CHANGE:
		OnChangeSpecialShopState( kPacket );
		break;
	case MSTPK_SPECIAL_SHOP_GOODS_BUY_RESULT:
		OnSpecialShopGoodsBuyResult( kPacket );
		break;
	case MSTPK_REQ_GUILD_ROOM_INDEX:
		OnResultGuildRoomReq( kPacket);
		break;
	case MSTPK_REQ_ALL_GUILD_ROOM_INFO:
		OnSendAllGuildRoomInfo(kPacket);
		break;
	case MSTPK_REQ_REGIST_COMPENSATION:
		OnRegistCompensation(kPacket);
		break;
	case MSTPK_TRADE_ITEM_GAMESVR_SYNC:
		OnTradeGameSvrSync( kPacket );
		break;
// �ؿܿ� ���� ���̼��� üũ
#if defined( SRC_OVERSEAS )
	case MSTPK_LICENSE_REQUEST:
		OnRequestLicense( kPacket );
		break;
#endif

	default:
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[warning][mainserver]Invalid packet : [0x%x]", kPacket.GetPacketID() );
		break;
	}
}

void MainServerNode::OnMainServerConnect( SP2Packet &rkPacket )
{
	// ���� ���� ����
	SP2Packet kPacket( MSTPK_SERVER_INFO );
	kPacket << g_ServerNodeManager.GetServerIndex() << g_App.GetGameServerID() << g_App.GetGameServerName() 
		    << g_App.GetPublicIP() << g_App.GetPrivateIP() << g_App.GetSSPort() << g_App.GetCSPort();
	SendMessage( kPacket );

	LOG.PrintTimeAndLog( 0, "[%s] index = %d, ID : %I64i, %s, %s, %s, %d, %d", __FUNCTION__, g_ServerNodeManager.GetServerIndex(), g_App.GetGameServerID(), 
		g_App.GetGameServerName().c_str(), g_App.GetPublicIP().c_str(), g_App.GetPrivateIP().c_str(), g_App.GetSSPort(), g_App.GetCSPort());

}

void MainServerNode::OnServerIndex( SP2Packet &rkPacket )
{
	DWORD dwServerIndex;
	rkPacket >> dwServerIndex;
	Information("@Server Index : %d\r\n", dwServerIndex);
	LOG.PrintTimeAndLog( 0, "@Server Index : %d", dwServerIndex);

	if(0 == dwServerIndex)
	{
		LOG.PrintTimeAndLog( 0, "Fail Get Server-Index");
		g_App.Shutdown(SHUTDOWN_QUICK);	
		return;
	}

	g_ServerNodeManager.SetServerIndex( dwServerIndex );

	g_DBClient.OnUpdateAllUserLogout( g_App.GetGameServerID() );
	g_DBClient.OnSelectSecondEncryptKey();	

	// �� / ������ / ������ / ä�� / ü�� �뺴 �ε��� ���� �� �޸� �Ҵ�
	g_RoomNodeManager.InitMemoryPool( dwServerIndex );
	g_BattleRoomManager.InitMemoryPool( dwServerIndex );
	g_LadderTeamManager.InitMemoryPool( dwServerIndex );
	g_ShuffleRoomManager.InitMemoryPool( dwServerIndex );
	g_ChannelNodeManager.InitMemoryPool( dwServerIndex );
	g_ExerciseCharIndexMgr.Init( dwServerIndex );

	if( !g_BillingRelayServer.ConnectTo() )
	{
		LOG.PrintTimeAndLog( 0, "Fail connect BillingRelayServer");
		g_App.Shutdown(SHUTDOWN_QUICK);	
		return;
	}

	if( !g_LogDBClient.ConnectTo() )
	{
		LOG.PrintTimeAndLog( 0, "Fail connect LogDBAgentServer");
		g_App.Shutdown(SHUTDOWN_QUICK);	
		return;
	}
}

void MainServerNode::OnAllServerList( SP2Packet &rkPacket )
{
	int iServerListSize;
	rkPacket >> iServerListSize;
	for(int i = 0;i < iServerListSize;i++)
	{
		int   iSSPort;
		DWORD dwServerIndex;
		ioHashString szServerIP;
		rkPacket >> dwServerIndex >> szServerIP >> iSSPort;
		if( dwServerIndex == 0 ) continue;
		if( szServerIP.IsEmpty() ) continue;

		if( g_ServerNodeManager.GetServerIndex() == dwServerIndex ) 
		{
			if( g_App.GetPrivateIP() == szServerIP && g_App.GetSSPort() == iSSPort )
				continue;
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�̹� ������ �ε����� ������ �ִ�!!! ( %d - %s:%d )", dwServerIndex, szServerIP.c_str(), iSSPort );
				g_App.Shutdown(SHUTDOWN_QUICK);	
				return;
			}
		}

		//����Ʈ�� �߰� �� ���� 
		if( !g_ServerNodeManager.ConnectTo(  dwServerIndex, szServerIP.c_str(), iSSPort ) )
		{
			// ������ ���� �� ������.
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MainServerNode::OnAllServerList ServerConnect Failed!!! : %d [%s:%d]", dwServerIndex, szServerIP.c_str(), iSSPort );
			g_App.Shutdown(SHUTDOWN_QUICK);	
			return;
		}
	}

	// All List Send
	bool bEndPacket;
	rkPacket >> bEndPacket;
	if( bEndPacket )
	{
		//���� ���� ����
		g_App.ServerBindStart();
		//���� ���� ����.
		g_App.ClientBindStart();
		//����͸� �� ���� ����.
		g_App.MonitoringBindStart();

		// ���� ����ȭ �۾��� �Ϸ�Ǹ� ���� ���� ���
		g_ServerNodeManager.IsConnectWorkComplete();

		//ioINILoader kLoader( "config/sp2_auto_run.ini" );
		//kLoader.SaveBool( "config", "AutoRun", true );
	}
}

//void MainServerNode::OnAnnounce( SP2Packet &rkPacket )
//{
//	AnnounceInfo kAInfo;
//	rkPacket >> kAInfo.szAnnounce;
//	rkPacket >> kAInfo.iMsgType;
//
//	if(kAInfo.iMsgType != ANNOUNCE_TYPE_ALL)
//		rkPacket >> kAInfo.szUserID;
//
//	rkPacket >> kAInfo.wYear;
//	rkPacket >> kAInfo.wMonth;
//	rkPacket >> kAInfo.wDay;
//	rkPacket >> kAInfo.wHour;
//	rkPacket >> kAInfo.wMinute;
//	rkPacket >> kAInfo.dwEndTime;
//
//	g_AnnounceManager.AddAnnouce( kAInfo );
//}

void MainServerNode::OnAdminKickUser( SP2Packet &rkPacket )
{
	// ���� ����
	ioHashString szUserID;
	rkPacket >> szUserID;

	g_UserNodeManager.DisconnectNode(szUserID);
}

void MainServerNode::OnAdminAnnounce( SP2Packet &rkPacket )
{
	// ��������
	AnnounceInfo kAInfo;
	rkPacket >> kAInfo.szAnnounce >> kAInfo.iMsgType;
	if(kAInfo.iMsgType != ANNOUNCE_TYPE_ALL)
	{
		rkPacket >> kAInfo.szUserID;
	}

	g_AnnounceManager.AddAnnouce( kAInfo );
}

void MainServerNode::OnAdminItemInsert( SP2Packet &rkPacket )
{
	int iPresentType = 0, iPresentValue1 = 0, iPresentValue2 = 0;
	int iPresentMent = 1;
	ioHashString szUserID;
	int iPublcIDState = 0;

	PACKET_GUARD_VOID( rkPacket.Read(szUserID) );
	PACKET_GUARD_VOID( rkPacket.Read(iPresentType) );
	PACKET_GUARD_VOID( rkPacket.Read(iPresentValue1) );
	PACKET_GUARD_VOID( rkPacket.Read(iPresentValue2) );
	PACKET_GUARD_VOID( rkPacket.Read(iPresentMent) );
	PACKET_GUARD_VOID( rkPacket.Read(iPublcIDState) );
	
	DWORD dwAgentID = 0, dwThreadID = 0; 
	DWORD dwUserIndex = 0;
	User* pUser = g_UserNodeManager.GetUserNodeByPublicID( szUserID );
	if( pUser )
	{
		dwAgentID	= pUser->GetUserDBAgentID();
		dwThreadID	= pUser->GetAgentThreadID();
		dwUserIndex	= pUser->GetUserIndex();
	}

	CTimeSpan cPresentGapTime( 30, 0, 0, 0 );
	CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;

	switch(iPublcIDState)
	{
	case MONITER_INSERT_ITEM_PRIVATE_ID:
		{
			if(pUser)
			{
				g_DBClient.OnPresentInsertByPrivateID( 
					dwAgentID, 
					dwThreadID, 
					"DeveloperK", 
					szUserID, 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 0, 
					iPresentMent, 
					kPresentTime, 
					1,
					iPublcIDState);

				g_LogDBClient.OnInsertPresent( 
					0, 
					"DeveloperK", 
					g_App.GetPublicIP().c_str(), 
					pUser->GetUserIndex(), 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 
					0, 
					LogDBClient::PST_RECIEVE, 
					"Monitor" );	

				pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û
			}
			else
			{
				g_DBClient.OnPresentInsertByPrivateID( 
					dwAgentID, 
					dwThreadID, 
					"DeveloperK", 
					szUserID, 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 0, 
					iPresentMent, 
					kPresentTime, 
					1,
					iPublcIDState);
			}
		}
		break;
	default:
		{
			if(pUser)
			{
				g_DBClient.OnInsertPresentData( 
					dwAgentID, 
					dwThreadID, 
					"DeveloperK", 
					szUserID, 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 0, 
					iPresentMent, 
					kPresentTime, 
					1 );

				g_LogDBClient.OnInsertPresent( 
					0, 
					"DeveloperK", 
					g_App.GetPublicIP().c_str(), 
					pUser->GetUserIndex(), 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 
					0, 
					LogDBClient::PST_RECIEVE, 
					"Monitor" );	

				pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û
			}
			else
			{
				g_DBClient.OnInsertPresentDataLog( 
					dwAgentID, 
					dwThreadID, 
					"DeveloperK", 
					szUserID, 
					(short)iPresentType, 
					iPresentValue1, 
					iPresentValue2, 
					0, 0, 
					iPresentMent, 
					kPresentTime, 
					1 );	
			}
		}
		break;
	}
}

void MainServerNode::OnAdminEventInsert( SP2Packet &rkPacket )
{
	int iValueCount = 0;
	int iValues[64] = {};

	rkPacket >> iValueCount;
	if(iValueCount < 2) return;

	for(int i = 0 ; i < iValueCount ; i++)
	{
		rkPacket >> iValues[i];
	}

	g_EventMgr.Update(iValues, iValueCount);
}

void MainServerNode::OnAdminUserBlock( SP2Packet &rkPacket )
{
	ioHashString szUserID;
	int	nBlockLevel;

	PACKET_GUARD_VOID( rkPacket.Read( szUserID ) );
	PACKET_GUARD_VOID( rkPacket.Read( nBlockLevel ) );

	UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( szUserID );

	if ( pUser )
	{
		SP2Packet kPacket( STPK_BLOCK_TYPE );
		PACKET_GUARD_VOID( kPacket.Write( nBlockLevel ) );

		pUser->RelayPacket( kPacket );
	}
}

void MainServerNode::OnAdminCommand( SP2Packet &rkPacket )
{
	int iType = 0;
	rkPacket >> iType;

	switch( iType )
	{
	case ADMINCOMMAND_KICK :
		OnAdminKickUser( rkPacket );
		break;

	case ADMINCOMMAND_ANNOUNCE :
		OnAdminAnnounce( rkPacket );
		break;

	case ADMINCOMMAND_ITEMINSERT :
		OnAdminItemInsert( rkPacket );
		break;

	case ADMINCOMMAND_EVENTINSERT :
		OnAdminEventInsert( rkPacket );
		break;
		
	case ADMINCOMMAND_USERBLOCK:
		OnAdminUserBlock( rkPacket );
		break;
	}
}

void MainServerNode::OnClassPrice( SP2Packet &rkPacket )
{
	CQueryResultData query_data;
	rkPacket >> query_data;
	g_UserNodeManager.OnResultSelectItemBuyCnt( &query_data );
}

extern CLog ProcessLOG;

void MainServerNode::OnServerUpdate( SP2Packet &rkPacket )
{
	rkPacket >> m_iHeadquartersUserCount >> m_iSafetySurvivalRoomUserCount >> m_iPlazaUserCount >> m_iBattleRoomUserCount >> m_iLadderBattleUserCount;

	if( m_dwLastSendServerUpdateTime != 0 )
	{
		DWORD dwGapTime = TIMEGETTIME() - m_dwLastSendServerUpdateTime;
		if( dwGapTime >= 100 )   // 100ms �̻� �ɸ��� �α�
			ProcessLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MSTPK_SERVER_UPDATE Packet Delay : %dms", dwGapTime );
	}
}

void MainServerNode::OnTotalUserReg( SP2Packet &rkPacket )
{
	rkPacket >> m_iTotalUserRegCount;

	// ���� �� �����ڼ� ����
	SP2Packet kPacket( STPK_TOTAL_REG_USER_CNT );
	kPacket << GetTotalUserRegCount();
	g_UserNodeManager.SendMessageAll( kPacket );
}

void MainServerNode::OnGuildRankList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_GUILD_RANK_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnGuildInfo( SP2Packet &rkPacket )
{
	bool bLoginInfo		= false;
	DWORD dwUserIndex	= 0, dwGuildIndex = 0, dwGuildMark = 0, dwGuildRank = 0, dwGuildLevel = 0;

	PACKET_GUARD_VOID( rkPacket.Read(bLoginInfo) );
	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildMark) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildRank) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildLevel) );
	
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_GUILD_INFO );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + ( sizeof( DWORD ) + sizeof( bool ) ), rkPacket.GetDataSize() - ( sizeof( DWORD ) + sizeof( bool ) ) );
		pUserParent->RelayPacket( kPacket );

		if( bLoginInfo )
		{
			//��� ���� ����
			if( pUserParent->IsUserOriginal() )
			{
				User* pUser = (User*)pUserParent;
				if( pUser )
				{
					ioUserGuild* pUserGuild = pUser->GetUserGuild();
					if( pUserGuild )
					{
						pUserGuild->SetGuildLevel(dwGuildLevel);
						pUserGuild->DoRecvGuildRankReward();
					}
				}
			}
		}
	}
}

void MainServerNode::OnGuildChangeJoiner( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwGuildIndex, dwGuildMaxUser;
	rkPacket >> dwUserIndex >> dwGuildIndex >> dwGuildMaxUser;
	
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MainServerNode::OnGuildChangeJoiner USER FIND NOT! :%d", dwUserIndex );	
		return;
	}

	//�������� �˸�
	SP2Packet kPacket( STPK_GUILD_JOINER_CHANGE );
	kPacket << dwGuildIndex << dwGuildMaxUser;
	pUserParent->RelayPacket( kPacket );
}

void MainServerNode::OnGuildSimpleInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_GUILD_SIMPLE_DATA );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnUpdateClientVersion( SP2Packet &rkPacket )
{
	bool bUseClientVersion = false;
	int  iClientVersion    = 0;

	rkPacket >> bUseClientVersion;
	if( bUseClientVersion )
	{
		rkPacket >> iClientVersion;

		m_vVersions.push_back(iClientVersion);
	}
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][main]Client version : [%d] [%d]", (int)bUseClientVersion, iClientVersion );
}

bool MainServerNode::IsRightClientVersion( int iUserClientVersion )
{
	if( !IsUseClientVersion() )
		return true;

	VERSIONS::iterator it = std::find(m_vVersions.begin(), m_vVersions.end(), iUserClientVersion);
	if( it != m_vVersions.end())
	{
		return true;
	}
	return false;
}

void MainServerNode::OnGuildExist( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_GUILD_EXIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnGuildMarkBlockInfo( SP2Packet &rkPacket )
{
	ioHashString szDeveloperID;
	DWORD dwGuildIndex, dwGuildMark;
	rkPacket >> szDeveloperID >> dwGuildIndex >> dwGuildMark;

	g_DBClient.OnSelectGuildMarkBlockInfo( Help::GetUserDBAgentID( szDeveloperID ), szDeveloperID.GetHashCode(), szDeveloperID, dwGuildIndex, dwGuildMark );
}

void MainServerNode::OnGuildTitleSync( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_GUILD_TITLE_SYNC );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnGuildEntryAgreeResult( SP2Packet &rkPacket )
{
	int iResult = 0;
	DWORD dwGuildIndex = 0, dwMasterIndex = 0, dwEntryUserIndex = 0;

	PACKET_GUARD_VOID(rkPacket.Read(iResult));
	PACKET_GUARD_VOID(rkPacket.Read(dwGuildIndex));
	PACKET_GUARD_VOID(rkPacket.Read(dwMasterIndex));
	PACKET_GUARD_VOID(rkPacket.Read(dwEntryUserIndex));

	UserParent* pMasterUser = g_UserNodeManager.GetGlobalUserNode(dwMasterIndex);
	
	if( !pMasterUser )
	{
		if( ENTRY_GUILD_AGREE_OK == iResult )
		{
			SP2Packet kPacket(MSTPK_GUILD_ENTRY_FAIL);
			PACKET_GUARD_VOID(kPacket.Write(dwGuildIndex));
			SendMessage(kPacket);
		}
		return;
	}

	if( !pMasterUser->IsUserOriginal() )
	{
		if( ENTRY_GUILD_AGREE_OK == iResult )
		{
			SP2Packet kPacket(MSTPK_GUILD_ENTRY_FAIL);
			PACKET_GUARD_VOID(kPacket.Write(dwGuildIndex));
			SendMessage(kPacket);
		}
		return;
	}

	if( ENTRY_GUILD_AGREE_OK == iResult )
	{
		//DB Update
		User* pUser = (User*)pMasterUser;
		if( pUser )
			g_DBClient.OnSelectGuildEntryAgree( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), dwEntryUserIndex, dwGuildIndex );
	}
}

void MainServerNode::OnCampRoomBattleInfo( SP2Packet &rkPacket )
{
	// ������� ���º�, �������� ���º�, ����� ��� �ε���, ����� ��� ���ʽ�, ������ ��� �ε���, ������ ��� ���ʽ�
	DWORD dwRoomIndex, dwBlueCampPoint, dwRedCampPoint, dwGuildIndex1, dwGuildIndex2;
	float fGuildBonus1, fGuildBonus2;
	rkPacket >> dwRoomIndex >> dwBlueCampPoint >> dwRedCampPoint >> dwGuildIndex1 >> fGuildBonus1 >> dwGuildIndex2 >> fGuildBonus2;

	Room *pRoom = g_RoomNodeManager.GetRoomNode( (int)dwRoomIndex );
	if( pRoom )
	{
		pRoom->SetCampRoomInfo( dwBlueCampPoint, dwRedCampPoint, dwGuildIndex1, fGuildBonus1, dwGuildIndex2, fGuildBonus2 );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MainServerNode::OnRoomGuildBonus Room None :%d", dwRoomIndex );
	}
}

void MainServerNode::OnCampDataSync( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_CAMP_DATA_SYNC );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnCampBattleInfo( SP2Packet &rkPacket )
{
	bool bBattlePaly		= false;
	DWORD dwActiveCampDate	= 0;

	PACKET_GUARD_VOID(rkPacket.Read(bBattlePaly));
	PACKET_GUARD_VOID(rkPacket.Read(dwActiveCampDate));

	g_LadderTeamManager.SetCampBattlePlay( bBattlePaly );
	g_GuildRewardMgr.SetActiveCampDate( dwActiveCampDate );

	if( !bBattlePaly ) //������ ����
	{
		g_UserNodeManager.AllUserUpdateLadderPointNExpert();        //�������� ��������Ʈ�� ������Ʈ�Ѵ�.
	}
	else               //������ ����
	{
		// ������ ���ԵǾ��ִ� �������� ���� ������ �����Ѵ�.
		g_UserNodeManager.AllCampUserCampDataSync();
	}
}

void MainServerNode::OnUserCampPointNRecordInit( SP2Packet &rkPacket )
{
	g_UserNodeManager.InitUserLadderPointNRecord();       //������ ��������Ʈ & ������ ���� ���� �ʱ�ȭ
}

void MainServerNode::OnCampInfluenceAlarm( SP2Packet &rkPacket )
{
	SP2Packet kPacket( SUPK_CAMP_INFLUENCE_ALARM );
	kPacket.SetDataAdd( (char*)rkPacket.GetData(), rkPacket.GetDataSize() );
	g_UserNodeManager.SendUDPMessageCampUserAll( kPacket );
}

void MainServerNode::OnLowConnectExit( SP2Packet &rkPacket )
{
	if( TIMEGETTIME() < 300000 )       // ���� ���� �� 5�� �̳����� �ش� ��Ŷ���� ���� ���� �ٿ��� �߻���Ű�� �ʴ´�.
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OnLowConnectExit Failed : %d", TIMEGETTIME() );
	}
	else
	{
		// iServerConnect���� �ڽŵ� ���ԵǾ��ִ�.
		int iServerConnect;
		rkPacket >> iServerConnect;
		g_CriticalError.CheckServerDownMsg( iServerConnect - 1, g_ServerNodeManager.GetNodeSize() );
		g_App.Shutdown(SHUTDOWN_SAFE);
	}
}

void MainServerNode::OnLoadCS3File( SP2Packet &rkPacket )
{
	ioHashString szGUID;
	int iVersion = 0;
	int iChange  = 0;
	rkPacket >> szGUID;
	rkPacket >> iVersion;
	rkPacket >> iChange;
#ifdef XTRAP
	g_ioXtrap.OpenCS3File( szGUID, iVersion, iChange );
#endif
}

void MainServerNode::OnCS3FileVersion( SP2Packet &rkPacket )
{
	ioHashString szGUID;
	rkPacket >> szGUID;
#ifdef XTRAP
	g_ioXtrap.SendCS3Version( szGUID );
#endif
}

void MainServerNode::OnAutoCloseAnnounce( SP2Packet &rkPacket )
{
	rkPacket >> m_szReserveCloseAnn;

	// ��ü ���� �߼�
	if( !m_szReserveCloseAnn.IsEmpty() )
	{
		SP2Packet kPacket( STPK_ANNOUNCE );
		kPacket << m_szReserveCloseAnn;
		g_UserNodeManager.SendMessageAll( kPacket );
	}
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MainServerNode::OnAutoCloseAnnounce : %s", m_szReserveCloseAnn.c_str() );
}

void MainServerNode::OnTradeList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TRADE_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTradeItemComplete( SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;

	if( iResult == TRADE_ITEM_TRADE_OK )
	{
		DWORD dwBuyUserIndex, dwRegisterUserIndex, dwTradeIndex;
		DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
		__int64 iItemPrice;
		ioHashString szRegisterUserName;

		rkPacket >> dwBuyUserIndex >> dwTradeIndex;
		rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
		rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue;
		rkPacket >> dwItemMaleCustom >> dwItemFemaleCustom >> iItemPrice;

		UserParent *pBuyUserParent = g_UserNodeManager.GetGlobalUserNode( dwBuyUserIndex );
		if( pBuyUserParent )
		{
			if( pBuyUserParent->IsUserOriginal() )
			{
				User *pBuyUser = (User*)pBuyUserParent;

				__int64 iCurPeso = pBuyUser->GetMoney();
				__int64 iResultPeso	= 0;
				if( pBuyUser->IsPCRoomAuthority() )
					iResultPeso = iItemPrice + (iItemPrice * g_TradeInfoMgr.GetPCRoomBuyTexRate());
				else
					iResultPeso = iItemPrice + (iItemPrice * g_TradeInfoMgr.GetBuyTexRate());		// ������ ���

				if( iCurPeso < iResultPeso )
				{
					// ���μ����� ��������
					SP2Packet kPacket( MSTPK_TRADE_ITEM_TRADE );

					PACKET_GUARD_VOID( kPacket.Write( TRADE_ITEM_TRADE_FAIL ) );
					PACKET_GUARD_VOID( kPacket.Write( dwTradeIndex ) );
					
					SendMessage( kPacket );

					// Ŭ���̾�Ʈ�� ��������
					SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );
					kSuccess << TRADE_BUY_PESO;
					pBuyUser->SendMessage( kSuccess );
					return;
				}
				else
				{
					// ��� ����
					pBuyUser->RemoveMoney( iResultPeso );
					g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_CONSUME, pBuyUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_TRADE, PRESENT_EXTRAITEM, dwItemMagicCode, iResultPeso, NULL);

					TradeLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TradeItem - [Peso:%I64d] [Index:%d] [RegUser:%d] [BuyUser:%d]",
												 iResultPeso, dwTradeIndex, dwRegisterUserIndex, dwBuyUserIndex );
				}

				// DB�� ������û
				g_DBClient.OnTradeItemComplete( pBuyUser->GetUserDBAgentID(),
												pBuyUser->GetAgentThreadID(),
												dwBuyUserIndex,
												dwRegisterUserIndex,
												szRegisterUserName,
												dwTradeIndex,
												dwItemType,
												dwItemMagicCode,
												dwItemValue,
												dwItemMaleCustom,
												dwItemFemaleCustom,
												iItemPrice );

				char szNote[MAX_PATH]="";
				StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

				g_LogDBClient.OnInsertTrade( dwBuyUserIndex, pBuyUser->GetPublicID(),
											 dwTradeIndex,
											 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_BUY,
											 pBuyUser->GetPublicIP(), szNote );
			}
			else
			{
				UserCopyNode *pUser = (UserCopyNode*)pBuyUserParent;
				SP2Packet kPacket( SSTPK_TRADE_ITEM_COMPLETE );
				kPacket << pUser->GetUserIndex();
				kPacket << TRADE_S_GET_INFO_OK;
				kPacket << dwBuyUserIndex << dwTradeIndex;
				kPacket << dwRegisterUserIndex << szRegisterUserName;
				kPacket << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
				kPacket << iItemPrice;
				pUser->SendMessage( kPacket );
			}
		}
		else     // ������ ���� ����ó��
		{
			// ���μ����� ��������
			SP2Packet kPacket( MSTPK_TRADE_ITEM_TRADE );

			PACKET_GUARD_VOID( kPacket.Write( TRADE_ITEM_TRADE_FAIL ) );
			PACKET_GUARD_VOID( kPacket.Write( dwTradeIndex ) );
			
			SendMessage( kPacket );
			return;
		}
	}
	else
	{
		DWORD dwBuyUserIndex;
		rkPacket >> dwBuyUserIndex;


		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwBuyUserIndex );
		if( !pUserParent )
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OnTradeItemComplete USER FIND NOT! :%d", dwBuyUserIndex );
			return;
		}

		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = (User*)pUserParent;

			int iFailType = TRADE_BUY_ERROR;
			switch( iResult )
			{
			case TRADE_ITEM_NO_ITEM:
				iFailType = TRADE_BUY_NO_ITEM;
				break;
			case TRADE_ITEM_PESO:
				iFailType = TRADE_BUY_PESO;
				break;
			case TRADE_ITEM_OWNER:
				iFailType = TRADE_BUY_OWNER;
				break;
			}

			SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );
			kSuccess << iFailType;
			pUser->SendMessage( kSuccess );
		}	
		else
		{
			UserCopyNode *pUser = (UserCopyNode*)pUserParent;
			SP2Packet kPacket( SSTPK_TRADE_ITEM_COMPLETE );
			kPacket << pUser->GetUserIndex();
			kPacket << TRADE_S_GET_INFO_FAIL;
			kPacket << iResult;
			pUser->SendMessage( kPacket );
		}
	}
}

void MainServerNode::OnTradeItemCancel( SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;

	if( iResult == TRADE_ITEM_CANCEL_OK )
	{
		DWORD dwUserIndex, dwRegisterUserIndex, dwTradeIndex;
		DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
		__int64 iItemPrice;
		ioHashString szRegisterUserName;

		rkPacket >> dwUserIndex >> dwTradeIndex;
		rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
		rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
		rkPacket >> iItemPrice;

		UserParent *pRegUserParent = g_UserNodeManager.GetGlobalUserNode( dwRegisterUserIndex );
		if( pRegUserParent )
		{
			if( pRegUserParent->IsUserOriginal() )
			{
				User *pRegUser = (User*)pRegUserParent;

				TradeLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TradeCancel - [TradeDelete] [RequestUser:%d] [RegisterUser:%d] [TradeIndex:%d] [Type:%d] [Code:%d] [Value:%d] [Custom:%d,%d]",
											 dwUserIndex, dwRegisterUserIndex, dwTradeIndex, dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom );

				// DB�� ������û
				g_DBClient.OnTradeItemCancel( pRegUser->GetUserDBAgentID(),
											  pRegUser->GetAgentThreadID(),
											  dwRegisterUserIndex,
											  szRegisterUserName,
											  dwTradeIndex,
											  dwItemType,
											  dwItemMagicCode,
											  dwItemValue,
											  dwItemMaleCustom,
											  dwItemFemaleCustom,
											  iItemPrice );

				char szNote[MAX_PATH]="";
				StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

				g_LogDBClient.OnInsertTrade( dwRegisterUserIndex, pRegUser->GetPublicID(),
											 dwTradeIndex,
											 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_CANCEL,
											 pRegUser->GetPublicIP(), szNote );
			}
			else
			{
				UserCopyNode *pUser = (UserCopyNode*)pRegUserParent;
				SP2Packet kPacket( SSTPK_TRADE_CANCEL );
				kPacket << pUser->GetUserIndex();
				kPacket << TRADE_CANCEL_S_GET_INFO_OK;
				kPacket << dwTradeIndex;
				kPacket << dwRegisterUserIndex << szRegisterUserName;
				kPacket << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
				kPacket << iItemPrice;
				pUser->SendMessage( kPacket );
			}
		}
		else
		{
			// ���μ����� ��������
			SP2Packet kPacket( MSTPK_TRADE_ITEM_CANCEL );
			kPacket << TRADE_ITEM_CANCEL_FAIL;
			kPacket << dwTradeIndex;
			SendMessage( kPacket );
			return;
		}
	}
	else
	{
		DWORD dwUserIndex;
		rkPacket >> dwUserIndex;


		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
		if( !pUserParent )
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "DB OnResultSelectCreateGuild USER FIND NOT! :%d", dwUserIndex );
			return;
		}

		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = (User*)pUserParent;

			int iFailType = TRADE_CANCEL_ERROR;
			switch( iResult )
			{
			case TRADE_ITEM_CANCEL_NO_ITEM:
				iFailType = TRADE_CANCEL_NO_ITEM;
				break;
			case TRADE_ITEM_CANCEL_NOT_OWNER:
				iFailType = TRADE_CANCEL_NOT_OWNER;
				break;
			}

			SP2Packet kSuccess( STPK_TRADE_CANCEL );
			kSuccess << iFailType;
			pUser->SendMessage( kSuccess );
		}	
		else
		{
			UserCopyNode *pUser = (UserCopyNode*)pUserParent;
			SP2Packet kPacket( SSTPK_TRADE_CANCEL );
			kPacket << pUser->GetUserIndex();
			kPacket << TRADE_CANCEL_S_GET_INFO_FAIL;
			kPacket << iResult;
			pUser->SendMessage( kPacket );
		}
	}
}

void MainServerNode::OnTradeItemTimeOut( SP2Packet &rkPacket )
{
	DWORD dwRegisterUserIndex, dwTradeIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
	rkPacket >> iItemPrice;

	UserParent *pRegUserParent = g_UserNodeManager.GetGlobalUserNode( dwRegisterUserIndex );
	if( pRegUserParent )
	{
		if( pRegUserParent->IsUserOriginal() )
		{
			User *pRegUser = (User*)pRegUserParent;

			g_PresentHelper.SendPresentByTradeTimeOut( pRegUser->GetUserDBAgentID(),
													   pRegUser->GetAgentThreadID(),
													   dwRegisterUserIndex,
													   dwRegisterUserIndex,
													   dwItemType,
													   dwItemMagicCode,
													   dwItemValue,
													   dwItemMaleCustom,
													   dwItemFemaleCustom,
													   szRegisterUserName );

			char szNote[MAX_PATH]="";
			StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

			g_LogDBClient.OnInsertTrade( dwRegisterUserIndex, pRegUser->GetPublicID(),
										 dwTradeIndex,
										 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_TIMEOUT,
										 pRegUser->GetPublicIP(), szNote );

			// DB���� ���� ���� ���������ϰ�, Ŭ���̾�Ʈ�� ���� ����.
			pRegUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

			SP2Packet kSuccess( STPK_TRADE_TIME_OUT );


// 			kSuccess << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
// 			kSuccess << iItemPrice;

			PACKET_GUARD_VOID( kSuccess.Write( dwTradeIndex ) );
			PACKET_GUARD_VOID( kSuccess.Write( dwItemType ) );
			PACKET_GUARD_VOID( kSuccess.Write( dwItemMagicCode ) );
			PACKET_GUARD_VOID( kSuccess.Write( dwItemValue ) );
			PACKET_GUARD_VOID( kSuccess.Write( dwItemMaleCustom ) );
			PACKET_GUARD_VOID( kSuccess.Write( dwItemFemaleCustom ) );
			PACKET_GUARD_VOID( kSuccess.Write( iItemPrice ) );

			pRegUser->SendMessage( kSuccess );
		}
		else
		{
			UserCopyNode *pUser = (UserCopyNode*)pRegUserParent;
			SP2Packet kPacket( SSTPK_TRADE_TIME_OUT );
			kPacket << pUser->GetUserIndex();
			kPacket << dwTradeIndex;
			kPacket << dwRegisterUserIndex << szRegisterUserName;
			kPacket << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
			kPacket << iItemPrice;
			pUser->SendMessage( kPacket );
		}
	}
	else
	{
		g_PresentHelper.SendPresentByTradeTimeOut( 0, 0,
												   dwRegisterUserIndex,
												   dwRegisterUserIndex,
												   dwItemType,
												   dwItemMagicCode,
												   dwItemValue,
												   dwItemMaleCustom,
												   dwItemFemaleCustom,
												   szRegisterUserName );

		char szNote[MAX_PATH]="";
		StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

		g_LogDBClient.OnInsertTrade( dwRegisterUserIndex, szRegisterUserName,
									 dwTradeIndex,
									 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_TIMEOUT,
									 g_App.GetPublicIP().c_str(), szNote );

		return;
	}
}

void MainServerNode::OnEventShopGoodsList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_EVENT_SHOP_GOODS_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnEventShopGoodsBuy( SP2Packet &rkPacket )
{
	int iBuyState;
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex >> iBuyState;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = static_cast< User * >( pUserParent );
			pUser->_OnEventShopGoodsBuy( iBuyState, rkPacket );
		}
		else if( iBuyState == EVENT_SHOP_GOODS_BUY_RESERVE )
		{
			DWORD dwGoodsIndex;
			rkPacket >> dwGoodsIndex;

			// �������� ���� ���� ���� 
			SP2Packet kPacket( STPK_EVENT_SHOP_GOODS_BUY );
			kPacket << EVENT_SHOP_GOODS_BUY_FAILED;
			pUserParent->RelayPacket( kPacket );

			// �����̵��� ������ ������ ��ǰ ���� ���
			SP2Packet kMainPacket( MSTPK_EVENT_SHOP_GOODS_BUY_RESULT );
			kMainPacket << EVENT_SHOP_GOODS_BUY_RESULT_CANCEL << dwGoodsIndex;
			SendMessage( kMainPacket );
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MainServerNode::OnEventShopGoodsBuy ������ Ÿ������ �̵��Ͽ� ���� ��� : %d - %d", dwUserIndex, dwGoodsIndex );
		}
		else
		{
			// �������� ���� ���� ����
			SP2Packet kPacket( STPK_EVENT_SHOP_GOODS_BUY );
			kPacket << iBuyState;
			pUserParent->RelayPacket( kPacket );
		}
	}
	else if( iBuyState == EVENT_SHOP_GOODS_BUY_RESERVE )
	{
		DWORD dwGoodsIndex;
		rkPacket >> dwGoodsIndex;

		// ��Ż�� ������ ������ ��ǰ ���� ���
		SP2Packet kPacket( MSTPK_EVENT_SHOP_GOODS_BUY_RESULT );
		kPacket << EVENT_SHOP_GOODS_BUY_RESULT_CANCEL << dwGoodsIndex;
		SendMessage( kPacket );

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MainServerNode::OnEventShopGoodsBuy ������ ��Ż�Ͽ� ���� ��� : %d - %d", dwUserIndex, dwGoodsIndex );
	}
}

void MainServerNode::OnDBAgentExtend( SP2Packet &rkPacket )
{
	// g_DBClient.ConnectExtend(); 2012-12-03 �ſ���, �̻�� 
}

void MainServerNode::OnGameServerReloadINI( SP2Packet &rkPacket )
{
	int iMaxINI;
	rkPacket >> iMaxINI;

	for(int i = 0;i < iMaxINI;i++)
	{
		int iType;
		rkPacket >> iType;
		
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][mainserver]Reload INI Type : [%d]", iType );
		switch( iType )
		{
		case RELOAD_HACK:
			HackCheck::LoadHackCheckValues();
			break;
		case RELOAD_USERDISPERSION:
			g_ServerNodeManager.LoadINI();
			break;
		case RELOAD_PROCESS:
			g_ProcessChecker.LoadINI();
			g_PacketChecker.LoadINI();
			break;
		case RELOAD_CLASS: // INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_ItemPriceMgr.LoadPriceInfo( "config/sp2_item_price.ini" , false );
			break;
		case RELOAD_DECO: // INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_DecorationPrice.LoadPriceInfo( false );
			break;
		case RELOAD_ETC: // INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_EtcItemMgr.LoadEtcItem( "config/sp2_etcitem_info.ini", false );
			break;
		case RELOAD_EVENT: // INI �̹� �Ҵ�� �������� �����Ҷ� ����Ұ�
			g_EventMgr.LoadINI( false );
			break;
		case RELOAD_QUEST:
			g_QuestMgr.LoadINIData();
			break;
		case RELOAD_MODE:
			g_ModeINIMgr.ReloadINIData();
			break;
		case RELOAD_PRESENT:
			g_PresentHelper.CheckNeedReload();
			break;
		case RELOAD_EXCAVATION:
			g_ExcavationMgr.CheckNeedReload();
			break;
		case RELOAD_FISHING:
			g_FishingMgr.CheckNeedReload();
			break;
		case RELOAD_ITEMCOMPOUND:
			g_CompoundMgr.CheckNeedReload();
			break;
		case RELOAD_EXTRAITEM:
			g_ExtraItemInfoMgr.CheckNeedReload();
			break;
		case RELOAD_TRADE:
			g_TradeInfoMgr.CheckNeedReload();
			break;
		case RELOAD_LEVELMATCH:
			g_LevelMatchMgr.CheckNeedReload();
			break;
		case RELOAD_ITEMINIT_CONTROL:
			g_ItemInitControl.LoadINIData();
			break;
		case RELOAD_CONFIG:
			Help::InitHelpConstant();
			break;
		case RELOAD_ALCHEMIC_MGR:
			g_AlchemicMgr.CheckNeedReload();
			break;
		case RELOAD_PET:
			g_PetInfoMgr.CheckNeedReload();
			g_PetGashaponMgr.CheckNeedReload();
			break;
		case RELOAD_AWAKE:

			break;

		case RELOAD_MISSION:
			g_MissionMgr.LoadINI(TRUE);
			break;
		case RELOAD_ROLLBOOK:
			g_RollBookMgr.LoadInI(TRUE);
			break;
		case RELOAD_COSTUMESHOP:
			g_CostumeShopGoodsMgr.LoadINI();
			break;
		case RELOAD_COSTUME:
			g_CostumeMgr.LoadINI();
			break;
		case RELOAD_MEDAL:
			g_MedalItemMgr.LoadINI();
			break;
		case RELOAD_SETITEM:
			g_SetItemInfoMgr.LoadINI();
			break;
		case RELOAD_SUPERGASHAPON:
			g_SuperGashaponMgr.LoadINI();
			break;
		case RELOAD_ACCESSORY:
			g_AccessoryMgr.LoadINI();
			break;
		}
	}
}

void MainServerNode::OnGameServerOption( SP2Packet &rkPacket )
{
	bool bOnlyServerRelay, bNagleAlgorithm, bPlazaNagleAlgorithm, bCharChangeToUDP, bWholeChatOn;
	rkPacket >> bOnlyServerRelay >> bNagleAlgorithm >> bPlazaNagleAlgorithm >> bCharChangeToUDP >> bWholeChatOn;

	g_RoomNodeManager.NagleOptionChange( bNagleAlgorithm, bPlazaNagleAlgorithm );

	Help::SetOnlyServerRelay( bOnlyServerRelay );
	Help::SetNagleAlgorithm( bNagleAlgorithm );
	Help::SetPlazaNagleAlgorithm( bPlazaNagleAlgorithm );
	Help::SetCharChangeToUDP( bCharChangeToUDP );
	Help::SetWholeChatOn( bWholeChatOn );

	//kyg ���μ����� �������ؼ� �ɼ��� �ٲ�������� ��� 
	if(g_Relay.IsRelayServerOn())
	{
		SP2Packet kPacket(RSTPK_ON_CONTROL);
		kPacket << bWholeChatOn;

		g_ServerNodeManager.SendMessageRelay( kPacket );
	}
}

void MainServerNode::OnExtraItemGrowthCatalyst( SP2Packet &rkPacket )
{
	g_ExtraItemGrowthCatalystMgr.ApplyLoadData( rkPacket );
}

void MainServerNode::OnExtraItemGrowthMortmainCheck( SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;

	DWORD dwUserIndex;
	int iEtcItemType, iTargetSlot, iItemCode, iReinforce;
	rkPacket >> dwUserIndex >> iEtcItemType >> iTargetSlot >> iItemCode >> iReinforce;
	switch( iResult )
	{
	case EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_OK:
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUserParent )
			{
				bool bCheckOK = false;
				if( pUserParent->IsUserOriginal() )
				{
					User *pUser = static_cast< User * > (pUserParent);
					if( pUser )
					{
						ioUserExtraItem *pUserExtraItem = pUser->GetUserExtraItem();
						if( pUserExtraItem )
						{
							ioUserExtraItem::EXTRAITEMSLOT kSlot;
							if( pUserExtraItem->GetExtraItem( iTargetSlot, kSlot ) )
							{
								kSlot.m_PeriodType = ioUserExtraItem::EPT_MORTMAIN;     // ���� ���� ����
								kSlot.m_iReinforce = g_ExtraItemGrowthCatalystMgr.GetGrowthCatalystReinforce( iReinforce + 1 );
								pUserExtraItem->SetExtraItem( kSlot );
								pUser->SaveExtraItem();

								SP2Packet kPacket( STPK_EXTRAITEM_GROWTH_CATALYST );
								kPacket << EXTRAITEM_GROWTH_CATALYST_OK;
								kPacket << iEtcItemType;
								kPacket << true;
								kPacket << iTargetSlot;
								kPacket << kSlot.m_iReinforce;
								kPacket << (int)kSlot.m_PeriodType;
								pUser->SendMessage( kPacket );
								bCheckOK = true;
								LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_OK : %s - %d - %d - %d - %d", pUser->GetPublicID().c_str(), iTargetSlot, iItemCode, iReinforce, (int)kSlot.m_PeriodType );
							}
						}
					}					
				}

				if( bCheckOK == false )
				{
					// ���� �̵������Ƿ� ���� ó��
					SP2Packet kPacket( STPK_EXTRAITEM_GROWTH_CATALYST );
					kPacket << EXTRAITEM_GROWTH_CATALYST_OK;
					kPacket << iEtcItemType;
					kPacket << false;
					kPacket << iTargetSlot;
					kPacket << iReinforce;
					kPacket << (int)ioUserExtraItem::EPT_GROW_TIME;
					pUserParent->RelayPacket( kPacket );

					SP2Packet kPacket2( MSTPK_EXTRAITEM_GROWTH_MORTMAIN_CHECK );
					kPacket2 << EXTRAITEM_GROWTH_MORTMAIN_COUNT_MINUS << dwUserIndex << iItemCode;
					SendMessage( kPacket2 );
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_FAIL 2: %s - %d - %d - %d", pUserParent->GetPublicID().c_str(), iTargetSlot, iItemCode, iReinforce );
				}
			}
			else
			{
				// ���� ������ ���� ó��
				SP2Packet kPacket( MSTPK_EXTRAITEM_GROWTH_MORTMAIN_CHECK );
				kPacket << EXTRAITEM_GROWTH_MORTMAIN_COUNT_MINUS << dwUserIndex << iItemCode;
				SendMessage( kPacket );

				if ( pUserParent != NULL )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_FAIL 3: %d - %d - %d", iTargetSlot, iItemCode, iReinforce );
				}
			}
		}
		break;
	case EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_FAIL:
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUserParent )
			{
				SP2Packet kPacket( STPK_EXTRAITEM_GROWTH_CATALYST );
				kPacket << EXTRAITEM_GROWTH_CATALYST_OK;
				kPacket << iEtcItemType;
				kPacket << false;
				kPacket << iTargetSlot;
				kPacket << iReinforce;
				kPacket << (int)ioUserExtraItem::EPT_GROW_TIME;
				pUserParent->RelayPacket( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "EXTRAITEM_GROWTH_MORTMAIN_CHECK_RESULT_FAIL 1: %s - %d - %d - %d", pUserParent->GetPublicID().c_str(), iTargetSlot, iItemCode, iReinforce );
			}
		}
		break;
	}
}

void MainServerNode::OnExtraItemGrowthMortmainInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwItemCode, dwItemCount, dwItemDate;
	rkPacket >> dwUserIndex >> dwItemCode >> dwItemCount >> dwItemDate;

	SP2Packet kPacket( STPK_DEVELOPER_MACRO );
	kPacket << DEVELOPER_EXTRAITEM_GROWTH_CATALYST_INFO;
	kPacket << dwItemCode << dwItemCount << dwItemDate;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnServerInfoRequest( SP2Packet& rkPacket )
{
	ioHashString szGUID;
	rkPacket >> szGUID;

	// ��Ŷ����
	SP2Packet kPacket( MSTPK_SERVER_INFO_ACK );

	// ���Ӽ��� ���� ����
	GAMESERVERINFO ServerInfo;
	g_App.DrawModule( ServerInfo );

	kPacket << szGUID;
	kPacket << ServerInfo;

	// �����̼��� ���� ����
 	g_Relay.MakeInfoPacket( kPacket );

	// ����
	SendMessage( kPacket );
}

void MainServerNode::OnServerPingCheck( SP2Packet &rkPacket )
{
	SP2Packet kPacket( MSTPK_SERVER_PING_CHECK );
	SendMessage( kPacket );
}

void MainServerNode::OnTournamentRegularInfo( SP2Packet &rkPacket )
{
	BYTE  State;
	int iReguralResource;
	DWORD dwUserIndex, dwIndex;
	bool bDisable;
	rkPacket >> dwUserIndex >> dwIndex >> State >> iReguralResource >> bDisable;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_REGULAR_INFO );
		kPacket << dwIndex << State << iReguralResource << bDisable;
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentMainInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_MAIN_INFO );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentListRequest( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_LIST_REQUEST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentCreateTeam( SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;	

	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );

	if( pUserParent )
	{
		if( iResult == MS_TOURNAMENT_TEAM_CREATE_OK )
		{
			SHORT Position;
			ioHashString kTeamName;
			int iCheerPoint, iLadderPoint;
			BYTE MaxPlayer, CampPos, TourPos;
			DWORD dwTourIndex, dwTeamIndex, dwOwnerIndex;
			rkPacket >> dwTourIndex >> dwTeamIndex >> kTeamName >> dwOwnerIndex;
			rkPacket >> Position >> MaxPlayer >> iCheerPoint >> TourPos >> iLadderPoint >> CampPos;

			if( pUserParent->IsUserOriginal() )
			{
				User *pUser = static_cast< User * >( pUserParent );
				ioUserTournament *pTournament = pUser->GetUserTournament();
				if( pTournament )
				{
					pTournament->CreateTeamData( dwTourIndex, dwTeamIndex, dwOwnerIndex, kTeamName, Position, TourPos );

					//
					SP2Packet kPacket( STPK_TOURNAMENT_TEAM_CREATE );
					kPacket << TOURNAMENT_TEAM_CREATE_OK << dwTourIndex << dwTeamIndex << kTeamName << dwOwnerIndex 
						    << Position << MaxPlayer << iCheerPoint << TourPos << iLadderPoint << CampPos;
					pUser->SendMessage( kPacket );
				}
			}
			else
			{
				//�ش� ���� ������ ����
				UserCopyNode *pUser = (UserCopyNode*)pUserParent;
				SP2Packet kPacket( SSTPK_TOURNAMENT_TEAM_CREATE );
				kPacket << dwUserIndex  << dwTourIndex << dwTeamIndex << kTeamName << dwOwnerIndex 
						<< Position << MaxPlayer << iCheerPoint << TourPos << iLadderPoint << CampPos;
				pUser->SendMessage( kPacket );
			}
		}
		else if( iResult == MS_TOURNAMENT_TEAM_CREATE_ALREADY_TEAM )
		{
			SP2Packet kPacket( STPK_TOURNAMENT_TEAM_CREATE );
			kPacket << TOURNAMENT_TEAM_CREATE_ALREADY_TEAM1;
			pUserParent->RelayPacket( kPacket );
		}
		else
		{
			SP2Packet kPacket( STPK_TOURNAMENT_TEAM_CREATE );
			kPacket << TOURNAMENT_TEAM_CREATE_END_LEAGUE;
			pUserParent->RelayPacket( kPacket );
		}
	}
	else
	{
		TournamentBot* pBot = TournamentBot::GetTournamentBot( dwUserIndex );

		if( g_TestNodeMgr.IsTestServer() )
		{
			if( iResult == MS_TOURNAMENT_TEAM_CREATE_OK )
			{
				SHORT Position;
				ioHashString kTeamName;
				int iCheerPoint, iLadderPoint;
				BYTE MaxPlayer, CampPos, TourPos;
				DWORD dwTourIndex, dwTeamIndex, dwOwnerIndex;
				rkPacket >> dwTourIndex >> dwTeamIndex >> kTeamName >> dwOwnerIndex;
				rkPacket >> Position >> MaxPlayer >> iCheerPoint >> TourPos >> iLadderPoint >> CampPos;
								
				if( pBot )				
					pBot->ApplyTeamCreateOk( dwTeamIndex );
				else
					LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s not find bot(1)" );
			}
			else
			{				
				if( pBot )				
					pBot->ApplyTeamCreateFail();
				else
					LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s not find bot(2)" );
			}
		}
	}
}

void MainServerNode::OnTournamentTeamInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_TEAM_INFO );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentServerSync( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournament( rkPacket );
}

void MainServerNode::OnTournamentEndProcess( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournamentEnd( rkPacket );
}

void MainServerNode::OnTournamentScheduleInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_SCHEDULE_INFO );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentTeamPositionSync( SP2Packet &rkPacket )
{
	int iMaxTeam;
	rkPacket >> iMaxTeam;

	// ������ �� �������� �����Ѵ�.
	for(int i = 0;i < iMaxTeam;i++)
	{
		bool bSync;
		BYTE TourPos;
		SHORT Position;
		DWORD dwTeamIndex;
		rkPacket >> dwTeamIndex >> Position >> TourPos >> bSync;

		// ���� ����Ʈ - ��ʸ�Ʈ - 
		g_UserNodeManager.AllUserTournamentTeamPosSync( dwTeamIndex, Position, TourPos, bSync );

	}
}

void MainServerNode::OnTournamentRoundTeamData( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_ROUND_TEAM_DATA );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentRoundStart( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournamentRoundStart( rkPacket );
}

void MainServerNode::OnTournamentBattleRoomInvite( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournamentBattleRoomInvite( rkPacket );
}

void MainServerNode::OnTournamentBattleTeamChange( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournamentBattleTeamChange( rkPacket );
}

void MainServerNode::OnTournamentCustomCreate( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwTourIndex;
	rkPacket >> dwUserIndex >> dwTourIndex;

	// �������� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		// �α��� ����
		SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_CREATE );
		kPacket << TOURNAMENT_CUSTOM_CREATE_OK << dwTourIndex;
		pUserParent->RelayPacket( kPacket );
	}
	else 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MainServerNode::OnTournamentCustomCreate - Leave User %d : %d", dwUserIndex, dwTourIndex );			
	}
}

void MainServerNode::OnTournamentTeamAllocateList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_TEAM_ALLOCATE_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentTeamAllocateData( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_TEAM_ALLOCATE_DATA );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentJoinConfirmCheck( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_JOIN_CONFIRM_CHECK );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentAnnounceChange( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_ANNOUNCE_CHANGE );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentTotalTeamList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_TOTAL_TEAM_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentCustomStateStart( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_STATE_START );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentCustomRewardList( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_LIST );
		kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
		pUserParent->RelayPacket( kPacket );
	}
}

void MainServerNode::OnTournamentCustomRewardRegCheck( SP2Packet &rkPacket )
{
	int iResult;
	DWORD dwUserIndex, dwTourIndex;
	rkPacket >> dwUserIndex >> iResult >> dwTourIndex;

	User *pUser = g_UserNodeManager.GetUserNode( dwUserIndex );
	if( pUser == NULL ) return;

	switch( iResult )
	{
	case TOURNAMENT_CUSTOM_REWARD_REG_CHECK_OK:
		{
			// Ư�� ������ ����
			BYTE TourPos;
			int iRewardTotalCount, iRewardSize;
			rkPacket >> TourPos >> iRewardTotalCount >> iRewardSize;

			int iTotalPrice = 0;
			DWORDVec vRewardEtcItem;
			for(int i = 0;i < iRewardSize;i++)
			{
				DWORD dwEtcItem;
				rkPacket >> dwEtcItem;
				vRewardEtcItem.push_back( dwEtcItem );
				int iCustomRewardEtcItemPrice = g_TournamentManager.GetCustomRewardEtcItemPrice( dwEtcItem );
				if( iCustomRewardEtcItemPrice == -1 )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "OnTournamentCustomRewardRegCheck : %s - Price Error : %d", pUser->GetPublicID().c_str(), dwEtcItem );
				}
				else
				{
					iTotalPrice += iCustomRewardEtcItemPrice * iRewardTotalCount;
				}
			}

			ioUserEtcItem::ETCITEMSLOT kSlot;
			ioUserEtcItem *pUserEtcItem = pUser->GetUserEtcItem();
			if( pUserEtcItem && pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_TOURNAMENT_COIN, kSlot ) )
			{
				if( kSlot.GetUse() < iTotalPrice )
				{
					SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
					kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_COIN_LACK << dwTourIndex << iTotalPrice;
					pUser->SendMessage( kPacket );
				}
				else
				{
					// ���� ����
					kSlot.AddUse( -iTotalPrice );
					if( kSlot.GetUse() <= 0 )
					{
						pUserEtcItem->DeleteEtcItem( ioEtcItem::EIT_ETC_TOURNAMENT_COIN, LogDBClient::ET_DEL );
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Use Tournament Etc Item Delete (%s/%d/%d) : %d", pUser->GetPublicID().c_str(), kSlot.m_iType, kSlot.m_iValue1, iTotalPrice );
					}
					else
					{
						pUserEtcItem->SetEtcItem( kSlot );
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Use Tournament Etc Item (%s/%d/%d) : %d", pUser->GetPublicID().c_str(), kSlot.m_iType, kSlot.m_iValue1, iTotalPrice );
					}

					SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
					kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_OK << dwTourIndex << iTotalPrice;
					pUser->SendMessage( kPacket );

					//���μ����� ���
					iRewardSize = (int)vRewardEtcItem.size();
					SP2Packet kMainPacket( MSTPK_TOURNAMENT_CUSTOM_REWARD_REG_UPDATE );
					kMainPacket << dwTourIndex << pUser->GetPublicID() << TourPos << iRewardSize;
					for(int i = 0;i < iRewardSize;i++)
						kMainPacket << vRewardEtcItem[i];
					SendMessage( kMainPacket );
					vRewardEtcItem.clear();
				}
			}
			else
			{
				SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
				kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_COIN_LACK << dwTourIndex << iTotalPrice;
				pUser->SendMessage( kPacket );
			}
		}
		break;
	case TOURNAMENT_CUSTOM_REWARD_REG_CLOSE:
		{
			SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
			kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_CLOSE << dwTourIndex;
			pUser->SendMessage( kPacket );
		}
		break;
	case TOURNAMENT_CUSTOM_REWARD_REG_EMPTY_SLOT:
		{
			SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
			kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_EMPTY_SLOT << dwTourIndex;
			pUser->SendMessage( kPacket );
		}
		break;
	case TOURNAMENT_CUSTOM_REWARD_REG_NOT_OWNER:
		{
			SP2Packet kPacket( STPK_TOURNAMENT_CUSTOM_REWARD_BUY );
			kPacket << TOURNAMENT_CUSTOM_REWARD_BUY_NOT_OWNER << dwTourIndex;
			pUser->SendMessage( kPacket );
		}
		break;
	}
}

void MainServerNode::OnTournamentCheerDecision( SP2Packet &rkPacket )
{
	DWORD dwUserIdx;
	int iResult;

	rkPacket >> iResult;
	rkPacket >> dwUserIdx;

	UserParent* pParent = g_UserNodeManager.GetUserNode( dwUserIdx );
	if( !pParent )
		return;	

	if( !pParent->IsUserOriginal() )
		return;

	User* pUser = static_cast<User*>( pParent );
	switch( iResult )
	{
	case TOURNAMENT_CHEER_DECISION_OK:
		{			
			pUser->OnTournamentCheerDecisionOK( rkPacket );
		}
		break;
	default:
		{
			SP2Packet kPacket( STPK_TOURNAMENT_CHEER_DECISION );
			kPacket << iResult;
			pUser->SendMessage( kPacket );
		}
		break;
	}
}

void MainServerNode::OnTournamentPrevChampSync( SP2Packet &rkPacket )
{
	g_TournamentManager.ApplyTournamentPrevChampSync( rkPacket );
}

void MainServerNode::OnWhiteListRequest( SP2Packet &kPacket )
{
	BOOL bWhiteList = FALSE;
	kPacket >> bWhiteList;

	g_IPBlock.SetWhiteList( bWhiteList );
}

void MainServerNode::OnNpcEventRequest( SP2Packet & kPacket )
{
	int nNpc = 0;
	kPacket >> nNpc;

	bool bNpc = false;

	if( nNpc > 0 )
		bNpc = true;

	LOG.PrintTimeAndLog( 0, "Monster mode got trouble %d(0:false 1:true)", nNpc );

	Help::SetSpawnNpc( bNpc );
}

void MainServerNode::OnUpdateRelativeGrade( SP2Packet &rkPacket )
{
	DWORD dwUniqueCode;
	rkPacket >> dwUniqueCode;

	g_UserNodeManager.UpdateRelativeGradeAllUser( dwUniqueCode );
}

void MainServerNode::SendNodeInfoRequest()
{
	SP2Packet packet(MSTPK_NODEINFO_REQUEST);
	SendMessage(packet);
}

void MainServerNode::OnNodeInfoResponse( SP2Packet & kPacket )
{
	if(g_DBClient.GetTotalNodeSize() > 0)
	{
		PrintTimeAndLog(0,"MSTPK_NODEINFO_REQUEST::Already Have data");
		return;
	}

	std::vector<std::string> vServerIP;
	std::vector<int> vServerPort;

	int iMaxCount;
	int iServerPort;
	TCHAR szServerIP[64];

	//////////////////////////////////////////////////////
	// ��������
	//////////////////////////////////////////////////////
	vServerIP.clear();
	vServerPort.clear();

	kPacket >> iMaxCount;
	for(int i = 0 ; i < iMaxCount ; i++)
	{
		kPacket >> szServerIP;
		kPacket >> iServerPort;

		vServerIP.push_back(szServerIP);
		vServerPort.push_back(iServerPort);
	}

	if(0 == iMaxCount)
	{
		PrintTimeAndLog(0,"MSTPK_NODEINFO_REQUEST::Error Billing");
		g_App.Shutdown(SHUTDOWN_QUICK);
		return;
	}

	g_BillingRelayServer.SetBillingServerInfo(vServerIP, vServerPort);

	//////////////////////////////////////////////////////
	// �α� DBAgent
	//////////////////////////////////////////////////////
	vServerIP.clear();
	vServerPort.clear();

	kPacket >> iMaxCount;
	for(int i = 0 ; i < iMaxCount ; i++)
	{
		kPacket >> szServerIP;
		kPacket >> iServerPort;

		vServerIP.push_back(szServerIP);
		vServerPort.push_back(iServerPort);
	}

	if(0 == iMaxCount)
	{
		PrintTimeAndLog(0,"MSTPK_NODEINFO_REQUEST::Error LogDBAgent");
		g_App.Shutdown(SHUTDOWN_QUICK);
		return;
	}

	g_LogDBClient.SetLOGDBAgentInfo(vServerIP, vServerPort);

	//////////////////////////////////////////////////////
	// ���� DBAgent
	//////////////////////////////////////////////////////
	vServerIP.clear();
	vServerPort.clear();

	kPacket >> iMaxCount;
	for(int i=0; i<iMaxCount; ++i)
	{
		kPacket >> szServerIP;
		kPacket >> iServerPort;

		g_DBClient.AddDBAgentInfo(szServerIP, iServerPort);
	}

	if(0 == iMaxCount)
	{
		PrintTimeAndLog(0,"MSTPK_NODEINFO_REQUEST::Error DBAgent");
		g_App.Shutdown(SHUTDOWN_QUICK);
		return;
	}

	//////////////////////////////////////////////////////
	// �α� ����
	//////////////////////////////////////////////////////
	kPacket >> iMaxCount;
	for(int i=0; i<iMaxCount; ++i)
	{
		kPacket >> szServerIP;
		kPacket >> iServerPort;
	}

	if(0 == iMaxCount)
	{
		PrintTimeAndLog(0,"MSTPK_NODEINFO_REQUEST::Error LogServer");
		g_App.Shutdown(SHUTDOWN_QUICK);
		return;
	}

	g_App.SetLogServer(szServerIP, iServerPort);

	//////////////////////////////////////////////////////
	// Wemade �α׼���
	//////////////////////////////////////////////////////
	kPacket >> iMaxCount;
	for(int i=0; i<iMaxCount; ++i)
	{
		kPacket >> szServerIP;
		g_WemadeLogger.Register( szServerIP );
	}

	g_WemadeLogger.Create();

	//////////////////////////////////////////////////////
	// ���� ����
	//////////////////////////////////////////////////////
	// �������Ʈ ����
	if( g_DBClient.ConnectTo() == false)
	{
		PrintTimeAndLog(0,"Fail connect DBAgentServer ");
		g_App.Shutdown(SHUTDOWN_QUICK);
	}
}

void MainServerNode::OnRelayServerChangeState( SP2Packet & kPacket )
{
	ioHashString szGUID;
	BOOL bRelayServerState;
	kPacket >> szGUID;
	kPacket >> bRelayServerState;
	g_Relay.SetUseRelayServer(bRelayServerState);
	Information("MSTPK_GAMESERVER_CHANGE_RELAYSERVER_STATE :: State[%d]\n",bRelayServerState);
	LOG.PrintTimeAndLog(0,"MSTPK_GAMESERVER_CHANGE_RELAYSERVER_STATE :: State[%d]\n",bRelayServerState);
}


void MainServerNode::OnSuperGashponLimitCheckResult( SP2Packet & rkPacket )
{
	int iResult;

	DWORD dwUserIndex;
	DWORD dwEtcItemType;

	rkPacket >> iResult;
	rkPacket >> dwUserIndex;
	rkPacket >> dwEtcItemType;

	User *pUser = NULL;
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );	
	if( pUserParent && pUserParent->IsUserOriginal() )	
		pUser = static_cast<User*>( pUserParent );
	
	switch( iResult )
	{
	case SUPER_GASHPON_LIMIT_REMAIN:
		{
			int iUseType;
			DWORD dwPackageIndex;				
			rkPacket >> dwPackageIndex;
			rkPacket >> iUseType;

			if( pUser )
			{
				pUser->OnSendSuperGashponPackage( dwEtcItemType, dwPackageIndex, iUseType );
			}
			else
			{
				//���μ����� Limit ī��Ʈ ���� ��Ŷ �߻�!
				SP2Packet kPacket( MSTPK_SUPER_GASHPON_LIMIT_DECREASE );
				kPacket << dwEtcItemType;
				SendMessage( kPacket );

				LOG.PrintTimeAndLog( 0, " %s %d disconnect, supergashpon limit decrase", __FUNCTION__, dwUserIndex );
				return;				
			}
		}
		break;
	case SUPER_GASHPON_LIMIT_FULL:
		{
			int iUseType;
			rkPacket >> iUseType;
			if( pUser )
			{
				pUser->OnSendSuperGashponSubPackage( dwEtcItemType, iUseType );				
			}
			else
			{
				LOG.PrintTimeAndLog( 0, " %s %d disconnect, supergashpon limit full", __FUNCTION__, dwUserIndex );
			}
		}
		break;
	}
	
}

void MainServerNode::OnSuperGashponLimitInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwEtcItemType, dwLimit;
	rkPacket >> dwUserIndex >> dwEtcItemType >> dwLimit;

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );	
	if( pUserParent && pUserParent->IsUserOriginal() )	
	{
		User *pUser = static_cast<User*>( pUserParent );
		if( pUser )
		{
			pUser->OnSendSuperGashponLimitInfo( dwEtcItemType, dwLimit );
			return;
		}
	}

	LOG.PrintTimeAndLog( 0, " %s %d disconnect, supergashpon info send fail", __FUNCTION__, dwUserIndex );
}

void MainServerNode::OnSpecialShopGoodsInfo( SP2Packet &rkPacket )
{
	g_SpecialGoodsMgr.SetSpecialGoodsList(rkPacket);
}

void MainServerNode::OnSpecialShopGoodsBuy( SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0;
	int iBuyState		= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iBuyState) );

	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent )
	{
		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = static_cast< User * >( pUserParent );
			pUser->_OnSpecialGoodsBuy( iBuyState, rkPacket );		//��� ó�� ����.
		}
		else if( SPECIAL_SHOP_GOODS_BUY_RESERVE == iBuyState )
		{
			DWORD dwGoodsType = 0;
			PACKET_GUARD_VOID( rkPacket.Read(dwGoodsType) );

			// �������� ���� ���� ���� 
			SP2Packet kPacket( STPK_ETCITEM_BUY );
			PACKET_GUARD_VOID( kPacket.Write(ETCITEM_BUY_EXCEPTION) );
			pUserParent->RelayPacket( kPacket );
		}
		else
		{
			//�׿� ���� ���� ����.
			SP2Packet kPacket( STPK_ETCITEM_BUY );
			PACKET_GUARD_VOID( kPacket.Write(iBuyState) );
			pUserParent->RelayPacket( kPacket );
		}
	}
}

void MainServerNode::OnChangeSpecialShopState( SP2Packet &rkPacket )
{
	bool bOpen = false;
	PACKET_GUARD_VOID( rkPacket.Read(bOpen) );
	g_SpecialGoodsMgr.ChangeState(bOpen);
}

void MainServerNode::OnSpecialShopGoodsBuyResult( SP2Packet &rkPacket )
{
	DWORD dwGoodsCode		= 0;
	int iCurCount			= 0;
	DWORD dwBuyUserIndex	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwGoodsCode) );
	PACKET_GUARD_VOID( rkPacket.Read(iCurCount) );
	PACKET_GUARD_VOID( rkPacket.Read(dwBuyUserIndex) );

	g_SpecialGoodsMgr.ChangeCurGoodsCount(dwGoodsCode, iCurCount);

	if( dwBuyUserIndex != 0 )
	{
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwBuyUserIndex );
		if( pUserParent )
		{
			if( pUserParent->IsUserOriginal() )
			{
				User *pUser = static_cast< User * >( pUserParent );
				if( pUser )
				{
					SP2Packet kPacket( STPK_SPECIAL_SHOP_BUY_GOODS_COUNT );
					PACKET_GUARD_VOID( kPacket.Write(dwGoodsCode) );
					PACKET_GUARD_VOID( kPacket.Write(iCurCount) );
					pUser->SendMessage(kPacket);
				}
			}
		}
	}
}

void MainServerNode::OnResultGuildRoomReq( SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0;
	
	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );

	if( pUserParent )
	{
		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = static_cast< User * >( pUserParent );
				
			pUser->ResultGuildRoomReq(rkPacket);
		}
	}
}

void MainServerNode::OnSendAllGuildRoomInfo(SP2Packet &rkPacket)
{
	SP2Packet kPacket(MSTPK_REQ_ALL_GUILD_ROOM_INFO);
	g_GuildRoomBlockMgr.FillAllGuildRoomInfo(kPacket);
	SendMessage(kPacket);
}

void MainServerNode::OnRegistCompensation(SP2Packet &rkPacket)
{
	int iItemType		= 0;
	DWORD dwItemCode	= 0;
	int iValue			= 0;
	__int64 iEndDate	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(iItemType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwItemCode) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue) );
	PACKET_GUARD_VOID( rkPacket.Read(iEndDate) );
	
	if( iItemType != PRESENT_ETC_ITEM && iItemType != PRESENT_EXTRAITEM_BOX ) // ����� ����ϴ� ������ Ÿ��
	{
		LOG.PrintTimeAndLog( 0, "[warning][compensation]Regist fail : [%d][%d][%d]", iItemType, dwItemCode, iValue);
		return;
	}

	ioEtcItem* pItem = g_EtcItemMgr.FindEtcItem(dwItemCode);
	if( !pItem )
	{
		LOG.PrintTimeAndLog( 0, "[warning][compensation]Regist fail - invalid item: [%d][%d][%d]", iItemType, dwItemCode, iValue);
		return;
	}

	CTime cCurTime = CTime::GetCurrentTime();
	CTime cEndTime(iEndDate);

	if( cCurTime > cEndTime )
	{
		LOG.PrintTimeAndLog( 0, "[warning][compensation]Regist fail - invalid endDate");
		return;
	}

	g_CompensationMgr.RegistCompensation(CT_MAINTENANCE, iItemType, dwItemCode, iValue, iEndDate);
}


// ���� ������ ���� �ŷ��� ������ ����Ʈ�� �޴´�.
void MainServerNode::OnTradeGameSvrSync(SP2Packet &rkPacket)
{
	int iTradeType = 0;

	PACKET_GUARD_VOID( rkPacket.Read( iTradeType ) );

	switch( iTradeType )
	{
	case TRADE_ALL:
		{
			// ���μ����� ���� �޴� �ŷ��� ������ ��ü ����Ʈ
			g_TradeSyncMgr.RecvAllTradeItem( rkPacket );
		}
		break;

	case TRADE_ADD:
		{
			// ���� ������ ���� ���� �ŷ��� ������ �߰� ����
			g_TradeSyncMgr.RecvAddTradeItem( rkPacket );
		}
		break;

	case TRADE_DEL:
		{
			// ���� ������ ���� ���� �ŷ��� ������ ���� ����
			g_TradeSyncMgr.RecvDelTradeItem( rkPacket );
		}
		break;

	default:
		break;
	}
	
}


// �ؿܿ� ���� ���̼��� üũ
#if defined( SRC_OVERSEAS )
//����͸���-> ���μ���-> ���̼��� ��û ����
void MainServerNode::OnRequestLicense( SP2Packet &rkPacket )
{
	g_App.CheckLicenseForDev();
}
#endif