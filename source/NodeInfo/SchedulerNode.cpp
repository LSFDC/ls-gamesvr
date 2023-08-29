#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "UserNodeManager.h"
#include "RoomNodeManager.h"
#include "ChannelNodeManager.h"
#include "ServerNodeManager.h"
#include "BattleRoomManager.h"
#include "LadderTeamManager.h"
#include "ShuffleRoomManager.h"
#include "AnnounceMgr.h"
#include "LevelMatchManager.h"
#include "HeroRankManager.h"
#include "LicenseManager.h"
#include "ioShutDownManager.h"
#include "ioSaleManager.h"
#include "ioExcavationManager.h"
#include "ioItemInfoManager.h"
#include "ShuffleRoomReserveMgr.h"
#include "ioItemInitializeControl.h"
#include "../MainServerNode/MainServerNode.h"
#include "../MonitoringServerNode/MonitoringNodeManager.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "../Network/ioPacketQueue.h"
#include "../Network/GameServer.h"
#include "Scheduler.h"
#include "SchedulerNode.h"
#include "MissionManager.h"
#include "../MiniDump/CrashHandler.h"

//extern CLog CheatLOG;

SchedulerNode::SchedulerNode(void)
{
	Init();
}

SchedulerNode::~SchedulerNode(void)
{
	Destroy();
}

void SchedulerNode::Init()
{
}

void SchedulerNode::Destroy()
{
}

void SchedulerNode::ReceivePacket( CPacket &packet )
{
	g_RecvQueue.InsertQueue( (DWORD)this, packet, PK_QUEUE_INTERNAL );
}

void SchedulerNode::PacketParsing( CPacket &packet )
{
	FUNCTION_TIME_CHECKER( 500000.0f, packet.GetPacketID() );          // 0.5 �� �̻� �ɸ���α� ����
	SP2Packet &kPacket = (SP2Packet&)packet;

	switch( packet.GetPacketID() )
	{
	case ITPK_ROOM_PROCESS:
		OnRoomProcess( kPacket );
		break;

	case ITPK_TIMER_PROCESS:
		OnTimerProcess( kPacket );
		break;

	case ITPK_PING_PROCESS:
		OnPingProcess( kPacket );
		break;

	case ITPK_USERGHOST_PROCESS:
		OnUserGhostProcess( kPacket );
		break;

	case ITPK_USERUPDATE_PROCESS:
		OnUserUpdateProcess( kPacket );
		break;

	case ITPK_MONSTERCOIN_PROCESS:
		OnMonsterCoinProcess( kPacket );
		break;

	case ITPK_SENDBUFFER_FLUSH_PROCESS:
		OnSendBufferFlushProcess( kPacket );
		break;

	case ITPK_SHUFFLE_ROOM_PROCESS:
		OnShuffleRoomProcess( kPacket );
		break;
	case ITPK_BILLING_CCU_COUNT:
		OnBillingCCUCountProcess( kPacket );
		break;
#ifdef ANTIHACK
	case ITPK_UDP_ANTIHACK_CHECK:
		OnUDPAntiHackCheck( kPacket );
		break;
	case ITPK_UDP_ANTIHACK_PENALTY:
		OnUDPAntiHackPenalty( kPacket );
		break;
	case ITPK_ANTIHACK_RELOAD:
		OnAntiHackReload( kPacket );
		break;
#endif
	case ITPK_LADDER_TEAM_MATCHING:
		OnLadderteamMatchingProcess( kPacket );
		break;

	default:
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "0x%x Unknown CPacket",  kPacket.GetPacketID() );
		break;
	}
}

void SchedulerNode::Call(const int MSG)
{
	CPacket packet(MSG);
	ReceivePacket(packet);
}

void SchedulerNode::OnRoomProcess( SP2Packet &packet )
{
	//��� ���� �ϰ� ó��.
	g_RoomNodeManager.RoomProcess();
}

void SchedulerNode::OnUserGhostProcess( SP2Packet &packet )
{
	g_UserNodeManager.UserNode_GhostCheck();
}

void SchedulerNode::OnUserUpdateProcess( SP2Packet &packet )
{
	g_UserNodeManager.UserNode_SaveCheck();
}

void SchedulerNode::OnMonsterCoinProcess( SP2Packet &packet )
{
	//���� ���� ���� ����
	g_UserNodeManager.UserNode_RefillMonsterCoin();

	// ���� ���̵� ���� ����
	g_UserNodeManager.UserNode_RefillRaidCoin();

	// ������ Ȯ��
	g_UserNodeManager.UserNode_TimeGashaponCheck();

	// �˴ٿ� �˻�
	g_UserNodeManager.UserNode_SelectShutDownCheck();
	g_UserNodeManager.UserNode_ShutDownCheck();

	g_UserNodeManager.UserNode_EventProcessTime();
}

void SchedulerNode::OnSendBufferFlushProcess( SP2Packet &packet )
{
	g_ServerNodeManager.ProcessFlush();
	g_BillingRelayServer.ProcessFlush();
	g_MonitoringNodeManager.ProcessFlush();
	g_UserNodeManager.ProcessFlush();
	g_LogDBClient.ProcessFlush();
}

void SchedulerNode::OnShuffleRoomProcess( SP2Packet &packet )
{
	//g_ShuffleRoomReserveMgr.Process();
}

void SchedulerNode::OnTimerProcess( SP2Packet &packet )
{
#ifdef XTRAP
	//���� XTRAP Ȯ��
	g_UserNodeManager.UserNode_XtrapCheck();
#endif

#ifdef NPROTECT
	g_UserNodeManager.UserNode_NProtectCheck();
#endif 

#ifdef HACKSHIELD
	g_UserNodeManager.UserNode_HackShieldCheck();
#endif 

	// ���μ���.
	g_UserNodeManager.UserNode_DataSync();
	g_BattleRoomManager.UpdateProcess();
	//g_LadderTeamManager.UpdateProcess();
	g_ShuffleRoomManager.UpdateProcess();
	g_ChannelNodeManager.ChannelNode_AllTimeExit();	

	g_DBClient.ProcessTime();
	g_LogDBClient.ProcessTime();

	g_AnnounceManager.ProcessSendReservedAnnounce();
	g_LevelMatchMgr.ProcessLevelMatch();
	
	g_MainServer.ProcessTime();
	g_BillingRelayServer.ProcessTime();
	g_EventMgr.ProcessTime();

	g_ProcessChecker.Process();
#ifdef XTRAP
	g_ioXtrap.DivideLoadCS3File();
#endif
	g_SaleMgr.ProcessTime();
	g_QuestMgr.ProcessQuest();
	g_HeroRankManager.UpdateProcess();

	g_ExcavationMgr.ProcessExcavation();

	g_ItemPriceMgr.Process();
	g_ItemInitControl.Process();
	g_LicenseMgr.Pocess();
	
	g_UserNodeManager.UserNode_RefillClover();

	g_MissionMgr.ProcessMission();

#ifdef NPROTECT
	g_ioNProtect.Process();
#endif
	g_ShutDownMgr.Process();
}

void SchedulerNode::OnPingProcess( SP2Packet &packet )
{
	g_DBClient.ProcessPing();
	g_LogDBClient.ProcessPing();
	g_ServerNodeManager.ProcessPing();

#ifdef HACKSHIELD
	g_UserNodeManager.UserNode_HackShieldCheck();
#endif 

#ifdef NPROTECT
//	g_UserNodeManager.UserNode_NProtectCheck();
#endif 
}

void SchedulerNode::OnLadderteamMatchingProcess(SP2Packet &packet)
{
	g_LadderTeamManager.UpdateProcess();
}

void SchedulerNode::OnBillingCCUCountProcess( SP2Packet &packet )
{
	/*ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
	if( pLocal )
		pLocal->GetCCUCountSend();*/
	int ccuCount = 0;
	//ccuCount = g_MainServer.GetTotalUserRegCount();
	ccuCount = g_UserNodeManager.GetNodeSize();

	SP2Packet kBillingPacket( BSTPK_CCU_COUNT );
	kBillingPacket << g_App.GetServerNo();
	kBillingPacket << ccuCount;
	
	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %d", "ioLocalLatin::GetCCUCountSend" , ccuCount);
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send : %d", "ioLocalLatin::GetCCUCountSend" , ccuCount);

}
#ifdef ANTIHACK
void SchedulerNode::OnUDPAntiHackCheck( SP2Packet &packet )
{
	g_Relay.InsertTimerOperation();
}

void SchedulerNode::OnUDPAntiHackPenalty( SP2Packet &packet )
{
	packet.SetPosBegin();
	DWORD dwUserIndex = 0;
	PACKET_GUARD_VOID( packet.Read(dwUserIndex) );

	User* pUser = g_UserNodeManager.GetUserNode(dwUserIndex);
	if( pUser )
	{
		if(pUser->GetMyRoom() != NULL)
		{
			pUser->ExitRoomToLobby();
		}
	}
	else
		;//CheatLOG.PrintTimeAndLog( 0, "[ERR] AntiHack Penalty - pUser(%u) is NULL!", dwUserIndex );
}

void SchedulerNode::OnAntiHackReload( SP2Packet &packet )
{
	g_Relay.OnAntihackReload();
}

#endif