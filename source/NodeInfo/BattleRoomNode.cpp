#include "stdafx.h"

#include "../EtcHelpFunc.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "../MainServerNode/MainServerNode.h"

#include "BattleRoomManager.h"
#include "UserNodeManager.h"
#include "ServerNodeManager.h"
#include "RoomNodeManager.h"
#include "LevelMatchManager.h"
#include "BattleRoomNode.h"
#include "ioMyLevelMgr.h"
#include "TournamentManager.h"
#include "../Local/ioLocalParent.h"
#include "TeamSurvivalAIMode.h"

extern CLog P2PRelayLOG;

//////////////////////////////////////////////////////////////////////////
BattleRoomSync::BattleRoomSync()
{
	m_pCreator     = NULL;
	m_dwUpdateTime = 0;
	m_dwUpdateType = 0;
	// ������Ʈ �ð�
	m_dwCheckTime[BRS_SELECTMODE]	= 4500;
	m_dwCheckTime[BRS_PLAY]			= 4000;
	m_dwCheckTime[BRS_CHANGEINFO]	= 3000;
	m_dwCheckTime[BRS_CREATE]		= 500;
	m_dwCheckTime[BRS_DESTROY]		= 0;
}

BattleRoomSync::~BattleRoomSync()
{

}

void BattleRoomSync::Init()
{
	m_dwUpdateTime = 0;
	m_dwUpdateType = 0;
}

void BattleRoomSync::SetCreator( BattleRoomNode *pCreator )
{
	Init();
	m_pCreator = pCreator;
}

void BattleRoomSync::Update( DWORD dwUpdateType )
{
	if( !COMPARE( dwUpdateType, BRS_SELECTMODE, MAX_BRS ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomSync::Update �˼� ���� ������Ʈ �� : %d", dwUpdateType );
		return;
	}
	if( !m_pCreator )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomSync::Update m_pCreator == NULL" );
		return;
	}

	if( dwUpdateType < m_dwUpdateType )
		return;

	m_dwUpdateType = dwUpdateType;
	m_dwUpdateTime = TIMEGETTIME();
	Process();
}

void BattleRoomSync::Process()
{
	if( m_dwUpdateTime == 0 ) return;
	if( !COMPARE( m_dwUpdateType, BRS_SELECTMODE, MAX_BRS ) ) return;

	DWORD dwGap = TIMEGETTIME() - m_dwUpdateTime;
	if( dwGap >= m_dwCheckTime[m_dwUpdateType] )
	{
		switch( m_dwUpdateType )
		{
		case BRS_SELECTMODE:
			m_pCreator->SyncSelectMode();
			break;
		case BRS_PLAY:
			m_pCreator->SyncPlay();
			break;
		case BRS_CHANGEINFO:
			m_pCreator->SyncChangeInfo();
			break;
		case BRS_CREATE:
			m_pCreator->SyncCreate();
			break;
		case BRS_DESTROY:
			m_pCreator->SyncDestroy();
			break;
		}
		m_dwUpdateType = m_dwUpdateTime = 0;
	}
}
//////////////////////////////////////////////////////////////////////////
BattleRoomNode::BattleRoomNode( DWORD dwIndex ) : m_dwIndex( dwIndex )
{
	m_bRandomTeamSequence = true;
	InitData();
}

BattleRoomNode::~BattleRoomNode()
{
}

void BattleRoomNode::SyncSelectMode()
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_SELECTMODE) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectModeTerm()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMode()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMap()) );

	g_ServerNodeManager.SendMessageToPartitions( kPacket );
}

void BattleRoomNode::SyncPlay()
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_PLAY) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectModeTerm()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMode()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMap()) );
	PACKET_GUARD_VOID( kPacket.Write(GetPlayModeType()) );
	PACKET_GUARD_VOID( kPacket.Write(IsBattleTimeClose()) );
	PACKET_GUARD_VOID( kPacket.Write(m_bRandomTeamMode) );
	PACKET_GUARD_VOID( kPacket.Write(IsStartRoomEnterX()) );
	PACKET_GUARD_VOID( kPacket.Write(m_bUseExtraOption) );
	PACKET_GUARD_VOID( kPacket.Write(m_bNoChallenger) );
			/*
			<< m_bUseExtraOption << m_iChangeCharType
			<< m_iCoolTimeType << m_iRedHPType << m_iBlueHPType
			<< m_iDropDamageType << m_iGravityType
			<< m_iPreSetModeType << m_iTeamAttackType << m_iGetUpType
			<< m_iKOType << m_iRedBlowType << m_iBlueBlowType
			<< m_iRedMoveSpeedType << m_iBlueMoveSpeedType << m_iKOEffectType
			<< m_iRedEquipType << m_iBlueEquipType;
			*/
	g_ServerNodeManager.SendMessageToPartitions( kPacket );
}

void BattleRoomNode::SyncChangeInfo()
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_CHANGEINFO) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectModeTerm()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMode()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMap()) );
	PACKET_GUARD_VOID( kPacket.Write(GetPlayModeType()) );
	PACKET_GUARD_VOID( kPacket.Write(IsBattleTimeClose()) );
	PACKET_GUARD_VOID( kPacket.Write(m_bRandomTeamMode) );
	PACKET_GUARD_VOID( kPacket.Write(IsStartRoomEnterX()) );
	PACKET_GUARD_VOID( kPacket.Write(m_bUseExtraOption) );
	PACKET_GUARD_VOID( kPacket.Write(m_bNoChallenger) );
			/*
			<< m_bUseExtraOption << m_iChangeCharType
			<< m_iCoolTimeType << m_iRedHPType << m_iBlueHPType
			<< m_iDropDamageType << m_iGravityType
			<< m_iPreSetModeType << m_iTeamAttackType << m_iGetUpType
			<< m_iKOType << m_iRedBlowType << m_iBlueBlowType
			<< m_iRedMoveSpeedType << m_iBlueMoveSpeedType << m_iKOEffectType
			<< m_iRedEquipType << m_iBlueEquipType;
			*/
	PACKET_GUARD_VOID( kPacket.Write(m_szRoomName) );
	PACKET_GUARD_VOID( kPacket.Write(m_szRoomPW) );
	PACKET_GUARD_VOID( kPacket.Write(m_OwnerUserID) );
	PACKET_GUARD_VOID( kPacket.Write((int)m_vUserNode.size()) );
	PACKET_GUARD_VOID( kPacket.Write(GetPlayUserCnt()) );
	PACKET_GUARD_VOID( kPacket.Write(m_iMaxPlayerBlue) );
	PACKET_GUARD_VOID( kPacket.Write(m_iMaxPlayerRed) );
	PACKET_GUARD_VOID( kPacket.Write(m_iMaxObserver) );
	PACKET_GUARD_VOID( kPacket.Write(GetAbilityMatchLevel()) );
	PACKET_GUARD_VOID( kPacket.Write(GetRoomLevel()) );
	PACKET_GUARD_VOID( kPacket.Write(GetBattleEventType()) );

	g_ServerNodeManager.SendMessageToPartitions( kPacket );
}

void BattleRoomNode::SyncCreate()
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_CREATE) );
	FillSyncCreate( kPacket );
	g_ServerNodeManager.SendMessageToPartitions( kPacket );
}

void BattleRoomNode::FillSyncCreate( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID( rkPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetTournamentIndex()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetSelectModeTerm()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetSelectMode()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetSelectMap()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetPlayModeType()) );
	PACKET_GUARD_VOID( rkPacket.Write(IsBattleTimeClose()) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bRandomTeamMode) );
	PACKET_GUARD_VOID( rkPacket.Write(IsStartRoomEnterX()) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bUseExtraOption) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bNoChallenger) );
			 /*
			 << m_bUseExtraOption << m_iChangeCharType
			 << m_iCoolTimeType << m_iRedHPType << m_iBlueHPType
			 << m_iDropDamageType << m_iGravityType
			 << m_iPreSetModeType << m_iTeamAttackType << m_iGetUpType
			 << m_iKOType << m_iRedBlowType << m_iBlueBlowType
			 << m_iRedMoveSpeedType << m_iBlueMoveSpeedType << m_iKOEffectType
			 << m_iRedEquipType << m_iBlueEquipType;
			 */
	PACKET_GUARD_VOID( rkPacket.Write(m_szRoomName) );
	PACKET_GUARD_VOID( rkPacket.Write(m_szRoomPW) );
	PACKET_GUARD_VOID( rkPacket.Write(m_OwnerUserID) );
	PACKET_GUARD_VOID( rkPacket.Write((int)m_vUserNode.size()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetPlayUserCnt()) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxPlayerBlue) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxPlayerRed) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxObserver) );
	PACKET_GUARD_VOID( rkPacket.Write(GetAbilityMatchLevel()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetRoomLevel()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetBattleEventType()) );
}

void BattleRoomNode::SyncDestroy()
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_DESTROY) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	g_ServerNodeManager.SendMessageToPartitions( kPacket );
}

void BattleRoomNode::SyncRealTimeCreate()
{
	SyncCreate();
	m_NodeSync.Init();
}

void BattleRoomNode::InitData()
{
	m_vUserNode.clear();

	m_szRoomName.Clear();
	m_szRoomPW.Clear(); 
	m_OwnerUserID.Clear();
	m_iMaxPlayerBlue = 0; 
	m_iMaxPlayerRed  = 0; 
	m_iMaxObserver = 0;
	m_iSelectMode = -1;
	m_iSelectMap  = -1;
	m_iPreSubType = 0;
	m_iPreMapNum = 0;
	m_PreModeType = MT_NONE;
	m_bSafetyLevelRoom = false;
	m_NodeSync.SetCreator( this );
	m_pBattleRoom = NULL;
	m_bRandomTeamMode  = true;
	m_bStartRoomEnterX = false;
	m_bAutoModeStart   = false;
	m_bBadPingKick     = true;

	if( ioLocalManager::GetSingletonPtr() )
	{
		ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
		if( pLocal && !pLocal->IsBadPingKick( false ) )
			m_bBadPingKick = false;
	}
	m_bUseExtraOption = false;
	m_bNoChallenger = true;

	m_iTeamAttackType = 0;
	m_iChangeCharType = 0;
	m_iCoolTimeType = 0;
	m_iRedHPType = 0;
	m_iBlueHPType = 0;
	m_iDropDamageType = 0;
	m_iGravityType = 0;
	m_iGetUpType = 0;
	m_iRedMoveSpeedType = 0;
	m_iBlueMoveSpeedType = 0;
	m_iKOType = 0;
	m_iKOEffectType = 0;
	m_iRedBlowType = 0;
	m_iBlueBlowType = 0;
	m_iRedEquipType = 0;
	m_iBlueEquipType = 0;

	m_iCatchModeRoundType = -1;
	m_iCatchModeRoundTimeType = -1;

	m_iGrowthUseType = 0;
	m_iExtraItemUseType = 0;

	m_iPreSetModeType = 0;

	m_iBattleEventType = BET_NORMAL;
	m_vInviteUser.clear();
	m_dwReserveTime    = 0;
	m_szBossName.Clear();
	m_szGangsiName.Clear();

	m_TournamentRoundData.Init();

	m_iAILevel = 0;

	InitRecord();
}

void BattleRoomNode::InitRecord()
{
	m_iBlueWin = 0;
	m_iBlueLose = 0;
	m_iBlueVictories = 0;
	m_iRedWin = 0;
	m_iRedLose = 0;
	m_iRedVictories = 0;
}

void BattleRoomNode::OnCreate()
{
	InitData();
	m_NodeSync.Update( BattleRoomSync::BRS_CREATE );
	m_dwReserveTime = TIMEGETTIME();
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::OnCreate (%d)", GetIndex() );
}

void BattleRoomNode::OnDestroy()
{
	BlockNode::Reset();

	InitData();
	m_NodeSync.Update( BattleRoomSync::BRS_DESTROY );
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::OnDestroy (%d)", GetIndex() );
}

void BattleRoomNode::EnterUser( const DWORD dwUserIndex, const ioHashString &szPublicID, const int iGradeLevel, const int iAbilityLevel, const bool bSafetyLevel, const bool bObserver,
							    const ioHashString &szPublicIP, const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iClientPort, const int iTransferPort  )
{
	// ���� ó�� : ������ ������ �ι� ���� ���ɼ��� �����Ƿ�
	// ������ �� �̹� ����Ǿ��ִ� ���̵�� �����Ѵ�.
	if( RemoveUser( dwUserIndex ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ERROR!!! : BattleRoomNode::EnterUser(%d) : %d - %s - %s:%d", 
								GetIndex(), dwUserIndex, szPublicID.c_str(), szTransferIP.c_str(), iTransferPort );
	}
	m_dwReserveTime = 0;

	if( m_OwnerUserID.IsEmpty() )
	{
		m_OwnerUserID = szPublicID;
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::EnterUser(%d) - Owner(%s)", GetIndex(), szPublicID.c_str() );

		if( IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s �����Ͽ� ������ �� : %s(%d)", __FUNCTION__, szPublicID.c_str(), GetIndex() );
		}
	}

	// �����뿡�� ����� ���� ����
	BattleRoomUser kEnterUser;
	kEnterUser.m_dwUserIndex	= dwUserIndex;
	kEnterUser.m_szPublicID		= szPublicID;
	kEnterUser.m_iGradeLevel	= iGradeLevel;
	kEnterUser.m_iAbilityLevel	= iAbilityLevel;	
	kEnterUser.m_bSafetyLevel   = bSafetyLevel;
	kEnterUser.m_bObserver		= bObserver;
	kEnterUser.m_szPublicIP		= szPublicIP;
	kEnterUser.m_szPrivateIP	= szPrivateIP;
	kEnterUser.m_szTransferIP   = szTransferIP;
	kEnterUser.m_iClientPort	= iClientPort;
	kEnterUser.m_iTransferPort  = iTransferPort;
	kEnterUser.m_iServerRelayCount = 0;

	// ���� ���� ����
	if( !bObserver && GetBattleEventType() == BET_TOURNAMENT_BATTLE )
	{
		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
		if( pUser && IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ��ȸ���� : %s(%d)", __FUNCTION__, pUser->GetPublicID().c_str(), GetIndex() );
		}

		if( IsTournamentTeam( dwUserIndex, TEAM_BLUE ) )
			kEnterUser.m_eTeamType = TEAM_BLUE;
		else if( IsTournamentTeam( dwUserIndex, TEAM_RED ) )
			kEnterUser.m_eTeamType = TEAM_RED;
		else
		{
			kEnterUser.m_eTeamType = TEAM_NONE;       // �� ������ ������
			kEnterUser.m_bObserver = true;
		}
	}
	else
	{
		kEnterUser.m_eTeamType      = CreateTeamType();
	}

	if( bObserver )
	{
		kEnterUser.m_eTeamType = TEAM_NONE;
	}

	AddUser( kEnterUser );

	UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUser == NULL ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::EnterUser(%d) ��Ƽ�� ���� ���� ���� : %s(%d)", GetIndex(), szPublicID.c_str(), dwUserIndex );
		return;
	}

	//���� �������� ��Ƽ ���� ����.
	SP2Packet kPacket( STPK_BATTLEROOM_INFO );
	FillBattleRoomInfo( kPacket );
	pUser->RelayPacket( kPacket );
	
	//���� �������� ��� ��Ƽ�� ������(�ڽ� ����) ����.(���� ���� ������ �� ��Ƽ ������ ���� ������.)
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;
		SP2Packet kPacket1( STPK_BATTLEROOM_USER_INFO );
		{
			FillUserInfo( kPacket1, kUser );
		}
		pUser->RelayPacket( kPacket1 );
	}

	//��� ��Ƽ������ ���� ���� ������(�ڽ� ����) ����.
	SP2Packet kPacket2( STPK_BATTLEROOM_USER_INFO );
	FillUserInfo( kPacket2, kEnterUser );
	SendPacketTcp( kPacket2, kEnterUser.m_dwUserIndex );

	// ����� ��� ���� ����
	CRASH_GUARD();
	if( GetOwnerName() == kEnterUser.m_szPublicID )
	{
		SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
		PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_CREATE_OK) );
		CreateBattle( kPacket );
		SendPacketTcp( kPacket );		
	}	
	
	// ���� ���� �¹��� ��� / �ɼ�  ����
	if( IsChangeEnterSyncData() )
	{
		SP2Packet kPacket3( STPK_BATTLEROOM_ENTERUSER_SYNC );
		FillRecord( kPacket3 );
		PACKET_GUARD_VOID( kPacket3.Write(m_bRandomTeamMode) );
		PACKET_GUARD_VOID( kPacket3.Write(m_bStartRoomEnterX) );
		PACKET_GUARD_VOID( kPacket3.Write(m_bAutoModeStart) );
		PACKET_GUARD_VOID( kPacket3.Write(m_bBadPingKick) );
		PACKET_GUARD_VOID( kPacket3.Write(m_bUseExtraOption) );
		PACKET_GUARD_VOID( kPacket3.Write(m_bNoChallenger) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iChangeCharType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iCoolTimeType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iRedHPType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iBlueHPType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iDropDamageType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iGravityType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iPreSetModeType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iTeamAttackType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iGetUpType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iKOType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iRedBlowType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iBlueBlowType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iRedMoveSpeedType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iBlueMoveSpeedType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iKOEffectType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iRedEquipType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iBlueEquipType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iCatchModeRoundType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iCatchModeRoundTimeType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iGrowthUseType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iExtraItemUseType) );
		PACKET_GUARD_VOID( kPacket3.Write(m_iBattleEventType) );

		pUser->RelayPacket( kPacket3 );
	}

	if( GetOwnerName() != kEnterUser.m_szPublicID )
	{
		// �߰� ���� ������ ���� ����.
		CheckBattleJoin( kEnterUser );
	}

	SP2Packet kPacket4( STPK_BATTLEROOM_COMMAND );
	PACKET_GUARD_VOID( kPacket4.Write(BATTLEROOM_MODE_SEL_OK) );
	PACKET_GUARD_VOID( kPacket4.Write(m_iSelectMode) );
	PACKET_GUARD_VOID( kPacket4.Write(m_iSelectMap) );
	PACKET_GUARD_VOID( kPacket4.Write(m_bSafetyLevelRoom) );
	pUser->RelayPacket( kPacket4 );

	// ���� ������ ����
	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE )
	{
		SP2Packet kPacket5( STPK_BATTLEROOM_COMMAND );
		PACKET_GUARD_VOID( kPacket5.Write(BATTLEROOM_TOURNAMENT_INFO) );
		PACKET_GUARD_VOID( kPacket5.Write(m_TournamentRoundData.m_dwTourIndex) );
		PACKET_GUARD_VOID( kPacket5.Write(m_TournamentRoundData.m_dwBlueIndex) );
		PACKET_GUARD_VOID( kPacket5.Write(m_TournamentRoundData.m_dwRedIndex) );

		// ���� ���� �ð� ����
		DWORD dwGapTime = ( TIMEGETTIME() - m_TournamentRoundData.m_dwInviteTimer );
		if( dwGapTime > TournamentRoundData::INVITE_DELAY_TIME )
			dwGapTime = 0;
		else
			dwGapTime = TournamentRoundData::INVITE_DELAY_TIME - dwGapTime;

		PACKET_GUARD_VOID( kPacket5.Write(dwGapTime) );

		if( IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ��ȸ ������ ���� Send : %s(%d) - ��ȸ : %d, ��� : %d, ���� : %d, ���� �ð� : %d", __FUNCTION__, pUser->GetPublicID().c_str(), 
				m_TournamentRoundData.m_dwTourIndex, m_TournamentRoundData.m_dwBlueIndex, m_TournamentRoundData.m_dwRedIndex , dwGapTime );
		}

		pUser->RelayPacket( kPacket5 );
	}

	ModeType eModeType = GetSelectIndexToMode( m_iSelectMode, m_iSelectMap );
	if( eModeType == MT_TEAM_SURVIVAL_AI )
	{
		SP2Packet kPacketAi( STPK_MACRO_COMMAND );
		PACKET_GUARD_VOID( kPacketAi.Write(MACRO_AI_LEVEL_CHANGE) );
		PACKET_GUARD_VOID( kPacketAi.Write(m_iAILevel) );
		pUser->RelayPacket( kPacketAi );
	}

	m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
}

void BattleRoomNode::UserInfoUpdate( const DWORD dwUserIndex, const int iGradeLevel, const int iAbilityLevel,  const bool bSafetyLevel, const int iClientPort, const ioHashString &szTransferIP, const int iTransferPort  )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			kUser.m_iGradeLevel   = iGradeLevel;
			kUser.m_iAbilityLevel = iAbilityLevel;
			kUser.m_bSafetyLevel  = bSafetyLevel;
			kUser.m_szTransferIP  = szTransferIP;
			kUser.m_iClientPort   = iClientPort;
			kUser.m_iTransferPort = iTransferPort;
			return;
		}
	}
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::UserInfoUpdate(%d) %d ���� ����", GetIndex(), dwUserIndex );
}

void BattleRoomNode::UserUDPChange( const DWORD dwUserIndex, const ioHashString &szPublicID, const ioHashString &szPublicIP, const int iClientPort, 
								    const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iTransferPort  )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			kUser.m_szTransferIP  = szTransferIP;
			kUser.m_iTransferPort = iTransferPort;
			kUser.m_iClientPort   = iClientPort;
			
			if( !IsBattleModePlaying() ) // �뿡�� �÷��� ���̶�� �̹� ���´�.
			{
				SP2Packet kPacket( STPK_CHANGE_UDP_INFO );
				PACKET_GUARD_VOID( kPacket.Write(szPublicID) );
				PACKET_GUARD_VOID( kPacket.Write(szPublicIP) );
				PACKET_GUARD_VOID( kPacket.Write(iClientPort) );
				PACKET_GUARD_VOID( kPacket.Write(szPrivateIP) );
				PACKET_GUARD_VOID( kPacket.Write(szTransferIP) );
				PACKET_GUARD_VOID( kPacket.Write(iTransferPort) );

				SendPacketTcp( kPacket, dwUserIndex );
			}			
			return;
		}
	}
}

void BattleRoomNode::UserP2PRelayInfo( const DWORD dwUserIndex, const DWORD dwRelayUserIndex, bool bRelay )
{
	BattleRoomUser &kRelayUser = GetUserNodeByIndex( dwRelayUserIndex );
	if( kRelayUser.m_szPublicID.IsEmpty() ) return;

	BattleRoomUser &kUser = GetUserNodeByIndex( dwUserIndex );
	if( kUser.m_szPublicID.IsEmpty() ) return;

	
	if( bRelay )
	{
		kUser.m_iServerRelayCount++;
	}
	else
	{
		kUser.m_iServerRelayCount--;				
	}
}

void BattleRoomNode::TournamentInfo( const DWORD dwUserIndex, const DWORD dwTeamIndex )
{
	if( GetBattleEventType() != BET_TOURNAMENT_BATTLE )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::TournamentInfo None Type : %d - %d", GetIndex(), GetBattleEventType() );
	}
	else
	{
		if( m_TournamentRoundData.m_dwBlueIndex == dwTeamIndex )
		{
			if( IsTournamentTeam( dwUserIndex, TEAM_BLUE ) == false )
				m_TournamentRoundData.m_BlueUserIndex.push_back( dwUserIndex );
		}
		else if( m_TournamentRoundData.m_dwRedIndex == dwTeamIndex )
		{
			if( IsTournamentTeam( dwUserIndex, TEAM_RED ) == false )
				m_TournamentRoundData.m_RedUserIndex.push_back( dwUserIndex );
		}
	}
}

bool BattleRoomNode::IsTournamentTeam( DWORD dwUserIndex, TeamType eTeam )
{
	if( eTeam == TEAM_BLUE )
	{
		DWORDVec::iterator iter = m_TournamentRoundData.m_BlueUserIndex.begin();
		for(;iter != m_TournamentRoundData.m_BlueUserIndex.end();iter++)
		{
			DWORD &rkUserIndex = *iter;
			if( rkUserIndex == dwUserIndex )
				return true;
		}
	}
	else if( eTeam == TEAM_RED )
	{
		DWORDVec::iterator iter = m_TournamentRoundData.m_RedUserIndex.begin();
		for(;iter != m_TournamentRoundData.m_RedUserIndex.end();iter++)
		{
			DWORD &rkUserIndex = *iter;
			if( rkUserIndex == dwUserIndex )
				return true;
		}
	}
	return false;
}

bool BattleRoomNode::LeaveUser( const DWORD dwUserIndex, const ioHashString &szPublicID  )
{
	if( !RemoveUser( dwUserIndex ) ) return false;

	//������ ������ �����ϸ� ��Ƽ ����.
	if( m_vUserNode.empty() )
	{
		if( GetBattleEventType() != BET_TOURNAMENT_BATTLE )
		{
			g_BattleRoomManager.RemoveBattleRoom( this );
		}
	}
	else
	{
		//�̺�Ʈ�濡�� ������ ������ �Ϲ� ������ ��ȯ
		if( GetBattleEventType() == BET_BROADCAST_AFRICA ||
			GetBattleEventType() == BET_BROADCAST_MBC )      
		{
			if( szPublicID == m_OwnerUserID )
			{
				SetBattleEventType( BET_NORMAL );

				// �̺�Ʈ Ÿ���� ����ǹǷ� ��� �ʱ�ȭ�ϰ� ��ü ����ȭ
				SetDefaultMode( BMT_TEAM_SURVIVAL );

				SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );

				PACKET_GUARD_bool( kPacket.Write(BATTLEROOM_EVENT_TYPE_CHANGE) );
				PACKET_GUARD_bool( kPacket.Write(GetBattleEventType()) ); 
				PACKET_GUARD_bool( kPacket.Write(m_iSelectMode) );
				PACKET_GUARD_bool( kPacket.Write(m_iSelectMap) );
				PACKET_GUARD_bool( kPacket.Write(m_bSafetyLevelRoom) );

				SendPacketTcp( kPacket );
			}
		}

		if( IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ��Ʋ�� ������ : %s(%d)", __FUNCTION__, szPublicID.c_str(), GetIndex() );
		}

		SP2Packet kPacket( STPK_BATTLEROOM_LEAVE );

		PACKET_GUARD_bool( kPacket.Write(szPublicID) );
		PACKET_GUARD_bool( kPacket.Write(GetRoomLevel()) );

		SendPacketTcp( kPacket );

		// ������ ������ ���� ��ü
		CRASH_GUARD();
		if( szPublicID == m_OwnerUserID )
			SelectNewOwner();

		if( m_pBattleRoom )
			m_pBattleRoom->LeaveReserveUser( dwUserIndex );
	}
	m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
	return true;
}

void BattleRoomNode::BattleEnterRandomTeam()
{
	if( !m_bRandomTeamMode ) return;

	// ���� �ٽ� ���´�. ��� ��
	static vBattleRoomUser vUserList;
	vUserList.clear();

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;

		if( kUser.m_bObserver )
		{
			ChangeTeamType( kUser.m_szPublicID, TEAM_NONE );
			continue;
		}

		vUserList.push_back( kUser );
	}

	std::sort( vUserList.begin(), vUserList.end(), BattleRoomUserSort() );
	int iBlueCount = m_iMaxPlayerBlue;
	int iRedCount  = m_iMaxPlayerRed;

	bool bSequenceValue = m_bRandomTeamSequence;
	m_bRandomTeamSequence = !m_bRandomTeamSequence;

	for(int i = 0;i < (int)vUserList.size();i++)
	{
		BattleRoomUser &kUser = vUserList[i];

		if( bSequenceValue && iBlueCount > 0 )
		{
			ChangeTeamType( kUser.m_szPublicID, TEAM_BLUE );
			iBlueCount--;
		}
		else if( iRedCount > 0 )
		{
			ChangeTeamType( kUser.m_szPublicID, TEAM_RED );
			iRedCount--;
		}
		else 
		{
			ChangeTeamType( kUser.m_szPublicID, TEAM_BLUE );
			iBlueCount--;
		}

		if( i % 2 == 0 )
			bSequenceValue = !bSequenceValue;
	}
	vUserList.clear();
}

void BattleRoomNode::BattleEnterRandomTeam( UserRankInfoList &rkUserRankInfoList )
{
	CRASH_GUARD();
	int i = 0;
	// �����濡�� ���������� ���� �뿡�� ���� ���� ������ ���Խ�Ų��.
	// �����̵��߿� �����Ⱑ �Ϸ�Ǹ� �����⿡ �ش� �ȵǴ� ������ ������ ���� �����ϹǷ� �ο� �ұ��� �߻�.
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter = m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		for(i = 0;i < (int)rkUserRankInfoList.size();i++)
		{
			if( rkUserRankInfoList[i].szName == kUser.m_szPublicID )
				break;
		}
		//
		if( i == (int)rkUserRankInfoList.size() )
		{
			UserRankInfo kUserRankInfo;
			kUserRankInfo.szName = kUser.m_szPublicID;
			kUserRankInfo.iRank  = (int)rkUserRankInfoList.size();
			rkUserRankInfoList.push_back( kUserRankInfo );
		}
	}

	int iUserRankCnt     = rkUserRankInfoList.size();
	int iBlueTeamCount   = GetMaxPlayer( TEAM_BLUE );
	int iRedTeamCount    = GetMaxPlayer( TEAM_RED );	

	bool bSequenceValue = m_bRandomTeamSequence;
	m_bRandomTeamSequence = !m_bRandomTeamSequence;

	int iCheckCnt = 0;
	for( i = 0; i < iUserRankCnt; i++ )
	{
		if( IsObserver( rkUserRankInfoList[i].szName ) )
		{

			ChangeTeamType( rkUserRankInfoList[i].szName, TEAM_NONE );

			continue;
		}
		else if( bSequenceValue && iBlueTeamCount > 0 )
		{
			ChangeTeamType( rkUserRankInfoList[i].szName, TEAM_BLUE );
			iBlueTeamCount--;
		}
		else if( iRedTeamCount > 0 )
		{
			ChangeTeamType( rkUserRankInfoList[i].szName, TEAM_RED );
			iRedTeamCount--;
		}
		else
		{
			ChangeTeamType( rkUserRankInfoList[i].szName, TEAM_BLUE );
			iBlueTeamCount--;
		}

		if( iCheckCnt % 2 == 0 )
		{
			bSequenceValue = !bSequenceValue;

		}

		iCheckCnt++;
	}
}

void BattleRoomNode::BattleEnterRoom( Room *pRoom )
{
	m_pBattleRoom = pRoom;

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;
		UserParent *pItem = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pItem == NULL ) continue;

		if( pItem->IsUserOriginal() )
		{
			User *pUser = (User*)pItem;

			// ���� �ε� ���� ����
			SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_READY_GO_OK) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeMapNum()) );
			pUser->SendMessage( kPacket );

			pUser->EnterRoom( m_pBattleRoom );
		}
		else        
		{
			//Ÿ������ �ִ� �������� ���� ���� �̵� ����
			UserCopyNode *pUser = (UserCopyNode*)pItem;
			m_pBattleRoom->EnterReserveUser( pUser->GetUserIndex() );
			SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN) );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN_BATTLE) );
			PACKET_GUARD_VOID( kPacket.Write((int)m_pBattleRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetRoomIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetUserIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeMapNum()) );
			PACKET_GUARD_VOID( kPacket.Write((int)m_pBattleRoom->GetPlazaModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetRoomNumber()) );

			pUser->SendMessage( kPacket );
		}
	}
}

void BattleRoomNode::CreateBattle( SP2Packet &rkPacket )
{	
	//
	PACKET_GUARD_VOID( rkPacket.Write(GetJoinUserCnt()) );

	int iUserCount = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter = m_vUserNode.begin();iter!=iEnd;++iter,++iUserCount)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_eTeamType == TEAM_NONE )
			kUser.m_eTeamType = CreateTeamType();

		PACKET_GUARD_VOID( rkPacket.Write(kUser.m_szPublicID) );
		PACKET_GUARD_VOID( rkPacket.Write((int)kUser.m_eTeamType) );
	}
}

TeamType BattleRoomNode::PlayRoomWantTeamType()
{
	// ��� �÷��� �� ���ϴ� �� Ÿ��
	if( m_pBattleRoom == NULL ) return TEAM_NONE;
	if( m_pBattleRoom->IsRoomEmpty() ) return TEAM_NONE;
	if( !m_pBattleRoom->IsRoomProcess() ) return TEAM_NONE;	

	ModeType eMode = m_pBattleRoom->GetModeType();

	if( eMode == MT_SURVIVAL || eMode == MT_BOSS || eMode == MT_GANGSI || eMode == MT_MONSTER_SURVIVAL ||
		eMode == MT_DUNGEON_A || eMode == MT_FIGHT_CLUB || eMode == MT_RAID ||
		Help::IsMonsterDungeonMode(eMode) )
	{
		return TEAM_NONE;
	}

	return m_pBattleRoom->GetNextTeamType();
}

TeamType BattleRoomNode::CreateTeamType()
{
	TeamType eWantTeam = PlayRoomWantTeamType();

	int iBlueCnt = 0;
	int iRedCnt  = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter = m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;

		if( kUser.m_bObserver ) continue;

		if( kUser.m_eTeamType == TEAM_BLUE )
			iBlueCnt++;
		else if( kUser.m_eTeamType == TEAM_RED )
			iRedCnt++;
	}
	ModeType eModeType = GetSelectIndexToMode( m_iSelectMode, m_iSelectMap );
	if( eModeType == MT_TEAM_SURVIVAL_AI )
		return TEAM_BLUE;
	if( eWantTeam == TEAM_BLUE && iBlueCnt < m_iMaxPlayerBlue )
		return TEAM_BLUE;
	else if( eWantTeam == TEAM_RED && iRedCnt < m_iMaxPlayerRed )
		return TEAM_RED;
	else if( iRedCnt >= iBlueCnt && iBlueCnt < m_iMaxPlayerBlue )
		return TEAM_BLUE;
	else if( iRedCnt < m_iMaxPlayerRed )
		return TEAM_RED;
    return TEAM_BLUE;
}

void BattleRoomNode::CheckBattleJoin( BattleRoomUser &kCheckUser )
{
	// ��Ƽ ���� ���� ����.
//	SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
//	kPacket << BATTLEROOM_UI_SHOW_OK << IsBattleModePlaying() << false;	
	
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kCheckUser.m_dwUserIndex );
	if( pUserParent == NULL ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::CheckBattleJoin ��Ƽ�� ���� ���� ����(%d) : %s(%d)", GetIndex(), kCheckUser.m_szPublicID.c_str(), kCheckUser.m_dwUserIndex );		return;
	}
//	pUserParent->RelayPacket( kPacket );

	if( IsBattleModePlaying() )
	{
		// �÷������̸� ������ �̵���Ų��.
		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = (User*)pUserParent;
			// �� ����.
			SP2Packet kPacket1( STPK_MOVING_ROOM );
			PACKET_GUARD_VOID( kPacket1.Write(m_pBattleRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket1.Write(m_pBattleRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket1.Write(m_pBattleRoom->GetModeMapNum()) );
			PACKET_GUARD_VOID( kPacket1.Write((int)m_pBattleRoom->GetPlazaModeType()) );

			pUser->SendMessage( kPacket1 );

			pUser->EnterRoom( m_pBattleRoom );
		}
		else
		{
			UserCopyNode *pUser = (UserCopyNode*)pUserParent;
			m_pBattleRoom->EnterReserveUser( pUser->GetUserIndex() );
			SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN) );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN_BATTLE) );
			PACKET_GUARD_VOID( kPacket.Write((int)m_pBattleRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetRoomIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetUserIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeMapNum()) );
			PACKET_GUARD_VOID( kPacket.Write((int)m_pBattleRoom->GetPlazaModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetRoomNumber()) );

			pUser->SendMessage( kPacket );
		}
	}
}

bool BattleRoomNode::IsBattleTeamChangeOK( TeamType eChangeTeam )
{
	int iTeamCount = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_eTeamType == eChangeTeam )
			iTeamCount++;
	}

	if( eChangeTeam == TEAM_BLUE )
	{
		if( iTeamCount >= m_iMaxPlayerBlue )
			return false;
	}
	else if( eChangeTeam == TEAM_RED )
	{
		if( iTeamCount >= m_iMaxPlayerRed )
			return false;
	}
	else if( eChangeTeam == TEAM_NONE )
	{
		if( iTeamCount >= m_iMaxObserver )
			return false;
	}

	return true;
}

bool BattleRoomNode::IsBattleModePlaying()
{
	if( m_pBattleRoom && !m_pBattleRoom->IsRoomEmpty() )
		return true;
	return false;
}

bool BattleRoomNode::IsStartRoomEnterX() 
{
	if( m_bStartRoomEnterX == false ) return false;
	if( m_pBattleRoom == NULL ) return false;
	if( !m_pBattleRoom->IsRoomProcess() ) return false;
	if( GetPlayModeType() == MT_NONE ) return false;

	return true;
}

bool BattleRoomNode::IsUseExtraOption()
{
	if( m_bUseExtraOption == false ) return false;

	return true;
}

bool BattleRoomNode::IsNoChallenger()
{
	if( GetPlayModeType() == MT_FIGHT_CLUB )
		return m_bNoChallenger;

	return false;
}

bool BattleRoomNode::IsHidden(int iIndex)
{
	if( GetIndex() == iIndex )							return true;
	if( GetBattleEventType() == BET_BROADCAST_MBC )		return true;
	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE ) return true;
	return false;
}

int BattleRoomNode::GetChangeCharType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iChangeCharType;
}

int BattleRoomNode::GetTeamAttackType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iTeamAttackType;
}

int BattleRoomNode::GetCoolTimeType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iCoolTimeType;
}

int BattleRoomNode::GetRedHPType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iRedHPType;
}

int BattleRoomNode::GetBlueHPType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iBlueHPType;
}

int BattleRoomNode::GetDropDamageType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iDropDamageType;
}

int BattleRoomNode::GetGravityType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iGravityType;
}

int BattleRoomNode::GetGetUpType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iGetUpType;
}

int BattleRoomNode::GetRedMoveSpeedType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iRedMoveSpeedType;
}

int BattleRoomNode::GetBlueMoveSpeedType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iBlueMoveSpeedType;
}

int BattleRoomNode::GetRedEquipType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iRedEquipType;
}

int BattleRoomNode::GetBlueEquipType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iBlueEquipType;
}

int BattleRoomNode::GetKOType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iKOType;
}

int BattleRoomNode::GetKOEffectType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iKOEffectType;
}

int BattleRoomNode::GetRedBlowType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iRedBlowType;
}

int BattleRoomNode::GetBlueBlowType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iBlueBlowType;
}

int BattleRoomNode::GetPreSetModeType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iPreSetModeType;
}

int BattleRoomNode::GetCatchModeRoundType()
{
	if( !m_bUseExtraOption ) return -1;

	return m_iCatchModeRoundType;
}

int BattleRoomNode::GetCatchModeRoundTimeType()
{
	if( !m_bUseExtraOption ) return -1;

	return m_iCatchModeRoundTimeType;
}

int BattleRoomNode::GetGrowthUseType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iGrowthUseType;
}

int BattleRoomNode::GetExtraItemUseType()
{
	if( !m_bUseExtraOption ) return 0;

	return m_iExtraItemUseType;
}

int BattleRoomNode::GetPlayModeType()
{
	if( !m_pBattleRoom || m_pBattleRoom->IsRoomEmpty() ) return MT_NONE;
	if( m_pBattleRoom->GetModeType() == MT_TRAINING ) return MT_NONE;
	if( m_pBattleRoom->GetModeType() == MT_HEADQUARTERS ) return MT_NONE;
	if( m_pBattleRoom->GetModeType() == MT_NONE ) return MT_NONE;

	return (int)m_pBattleRoom->GetModeType();
}

void BattleRoomNode::SetDefaultMode( int iSelectModeTerm )
{
	switch( iSelectModeTerm )
	{
	case BMT_UNDERWEAR:					//����Ż��
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_UNDERWEAR, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_CBT:					//����Ż��
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CBT, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_CATCH:					//����Ż��
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CATCH, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_CATCH_BOSS:            //����Ż�� - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CATCH, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_CATCH_RUNNINGMAN:					//����Ż�� ���׸�
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CATCH_RUNNINGMAN, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_CATCH_RUNNINGMAN_BOSS:            //����Ż�� ���׸� - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CATCH_RUNNINGMAN, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_STONE:                 //��¡��
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_SYMBOL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_STONE_BOSS:            //��¡�� - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_SYMBOL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_KING: 					//���� ũ���
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_KING, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_KING_BOSS: 			//���� ũ��� - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_KING, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_SURVIVAL:              //��ƿ&�����̹�
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_SURVIVAL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_FIGHT_CLUB:            //����ƮŬ��
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_FIGHT_CLUB, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_TEAM_SURVIVAL:              //�� ������ġ
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TEAM_SURVIVAL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_TEAM_SURVIVAL_BOSS:              //�� ������ġ - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TEAM_SURVIVAL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_TEAM_SURVIVAL_FIRST:        //�� ������ġ - �ʺ�
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TEAM_SURVIVAL, true, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		m_bSafetyLevelRoom = true;
		break;
	case BMT_TEAM_SURVIVAL_FIRST_BOSS:        //�� ������ġ - �ʺ� - ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TEAM_SURVIVAL, true, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		m_bSafetyLevelRoom = true;
		break;
	case BMT_BOSS:              //����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_BOSS, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_MONSTER_SURVIVAL_EASY:     //PvE - ���� ���� ����
	case BMT_MONSTER_SURVIVAL_NORMAL:
	case BMT_MONSTER_SURVIVAL_HARD:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_MONSTER_SURVIVAL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_MONSTER_SURVIVAL_EASY ) + 1;       
		break;
	case BMT_USER_CUSTOM:                 // ���� ��� 
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_CATCH, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_FOOTBALL:                 //�౸
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_FOOTBALL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_FOOTBALL_BOSS:            //�౸����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_FOOTBALL, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_GANGSI:				   //����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_GANGSI, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_DUNGEON_A_EASY:     //PvE - ���� ���� ����
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_DUNGEON_A, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_DUNGEON_A_EASY ) + 1;       
		break;
	case BMT_TOWER_DEFENSE_EASY:     //Ÿ�� ���潺
	case BMT_TOWER_DEFENSE_NORMAL:
	case BMT_TOWER_DEFENSE_HARD:
	case BMT_TOWER_DEFENSE_CHALLENGE:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TOWER_DEFENSE, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_TOWER_DEFENSE_EASY ) + 1;       
		break;
	case BMT_DARK_XMAS_EASY:
	case BMT_DARK_XMAS_NORMAL:
	case BMT_DARK_XMAS_HARD:
	case BMT_DARK_XMAS_CHALLENGE:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_DARK_XMAS, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_DARK_XMAS_EASY ) + 1;
		break;
	case BMT_FIRE_TEMPLE_EASY:
	case BMT_FIRE_TEMPLE_NORMAL:
	case BMT_FIRE_TEMPLE_HARD:
	case BMT_FIRE_TEMPLE_CHALLENGE:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_FIRE_TEMPLE, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_FIRE_TEMPLE_EASY ) + 1;       
		break;
	case BMT_DOBULE_CROWN:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_DOBULE_CROWN, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_DOBULE_CROWN_BOSS:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_DOBULE_CROWN, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = -1;
		break;
	case BMT_FACTORY_EASY:    
	case BMT_FACTORY_NORMAL:
	case BMT_FACTORY_HARD:
	case BMT_FACTORY_CHALLENGE:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_FACTORY, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = ( iSelectModeTerm - BMT_FACTORY_EASY ) + 1;       
		break;
	case BMT_TEAM_SURVIVAL_AI_EASY:		
	case BMT_TEAM_SURVIVAL_AI_HARD:
		if( iSelectModeTerm == BMT_TEAM_SURVIVAL_AI_EASY )
			m_iAILevel = 0;
		else
			m_iAILevel = 1;
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_TEAM_SURVIVAL_AI, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = 1;
		break;
	case BMT_RAID:
		m_iSelectMode = g_BattleRoomManager.GetBattleModeToIndex( MT_RAID, false, (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_iSelectMap  = 1;       
		break;

	default:
		m_iSelectMode = -1;
		m_iSelectMap  = -1;
		break;
	}

	if( GetBattleEventType() == BET_BROADCAST_MBC )
		m_iSelectMap = 1;
}

int BattleRoomNode::GetSelectModeTerm()
{
	if( m_iSelectMode == -1 )
	{
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_RANDOM_BOSS;
		else
			return BMT_RANDOM;
	}

	ModeType eSelectMode = GetSelectIndexToMode( GetSelectMode(), GetSelectMap() );
	if( eSelectMode == MT_NONE ) 
	{
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_RANDOM_BOSS;
		else
			return BMT_RANDOM;
	}
	else if( eSelectMode == MT_CATCH )
	{
		if( IsUseExtraOption() )
			return BMT_USER_CUSTOM;
        else if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_CATCH_BOSS;
		else
			return BMT_CATCH;
	}
	else if( eSelectMode == MT_UNDERWEAR )
	{
		return BMT_UNDERWEAR;
	}
	else if( eSelectMode == MT_CBT )
	{
		return BMT_CBT;
	}
	else if( eSelectMode == MT_CATCH_RUNNINGMAN )
	{
		if( IsUseExtraOption() )
			return BMT_USER_CUSTOM;
		else if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_CATCH_RUNNINGMAN_BOSS;
		else
			return BMT_CATCH_RUNNINGMAN;
	}
	else if( eSelectMode == MT_SYMBOL )
	{
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_STONE_BOSS;
		else
			return BMT_STONE;
	}    
	else if( eSelectMode == MT_KING )
	{
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_KING_BOSS;
		else
			return BMT_KING;
	}
	else if( eSelectMode == MT_SURVIVAL )
	{
		return BMT_SURVIVAL;
	}
	else if( eSelectMode == MT_FIGHT_CLUB )
	{
		return BMT_FIGHT_CLUB;
	}
	else if( eSelectMode == MT_TEAM_SURVIVAL )
	{
		if( m_bSafetyLevelRoom )
		{
			if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
				return BMT_TEAM_SURVIVAL_FIRST_BOSS;
			else
				return BMT_TEAM_SURVIVAL_FIRST;
		}
		else
		{
			if( IsUseExtraOption() )
				return BMT_USER_CUSTOM;
			else if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
				return BMT_TEAM_SURVIVAL_BOSS;
			else
				return BMT_TEAM_SURVIVAL;
		}
	}
	else if( eSelectMode == MT_TEAM_SURVIVAL_AI )
	{
		if( m_iAILevel == 0 )
			return BMT_TEAM_SURVIVAL_AI_EASY;
		return BMT_TEAM_SURVIVAL_AI_HARD;
	}
	else if( eSelectMode == MT_BOSS )
	{
		return BMT_BOSS;
	}
	else if( eSelectMode == MT_MONSTER_SURVIVAL )
	{
		int iReturnBMT = BMT_MONSTER_SURVIVAL_EASY + ( m_iSelectMap - 1 );
		if( !COMPARE( iReturnBMT, BMT_MONSTER_SURVIVAL_EASY, BMT_MONSTER_SURVIVAL_HARD + 1 ) )
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::GetSelectModeTerm Error : %d - %d", iReturnBMT, m_iSelectMap );
		else
			return iReturnBMT;
	}
	else if( eSelectMode == MT_FOOTBALL )
	{
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_FOOTBALL_BOSS;
		else
			return BMT_FOOTBALL;
	}   
	else if( eSelectMode == MT_GANGSI )
	{
		return BMT_GANGSI;
	}
	else if( eSelectMode == MT_DUNGEON_A )
	{
		int iReturnBMT = BMT_DUNGEON_A_EASY + ( m_iSelectMap - 1 );
		return iReturnBMT;
	}
	else if( eSelectMode == MT_TOWER_DEFENSE )
	{
		int iReturnBMT = BMT_TOWER_DEFENSE_EASY + ( m_iSelectMap - 1 );
		if( !COMPARE( iReturnBMT, BMT_TOWER_DEFENSE_EASY, BMT_TOWER_DEFENSE_CHALLENGE + 1 ) )
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::GetSelectModeTerm Error : %d - %d", iReturnBMT, m_iSelectMap );
		else
			return iReturnBMT;
	}
	else if( eSelectMode == MT_DARK_XMAS )
	{
		int iReturnBMT = BMT_DARK_XMAS_EASY + ( m_iSelectMap - 1 );
		if( !COMPARE( iReturnBMT, BMT_DARK_XMAS_EASY, BMT_DARK_XMAS_CHALLENGE + 1 ) )
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::GetSelectModeTerm Error : %d - %d", iReturnBMT, m_iSelectMap );
		else
			return iReturnBMT;
	}
	else if( eSelectMode == MT_FIRE_TEMPLE )
	{
		int iReturnBMT = BMT_FIRE_TEMPLE_EASY + ( m_iSelectMap - 1 );
		if( !COMPARE( iReturnBMT, BMT_FIRE_TEMPLE_EASY, BMT_FIRE_TEMPLE_CHALLENGE + 1 ) )
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::GetSelectModeTerm Error : %d - %d", iReturnBMT, m_iSelectMap );
		else
			return iReturnBMT;
	}
	else if( eSelectMode == MT_DOBULE_CROWN )
	{		
		if( GetMaxPlayerBlue() != GetMaxPlayerRed() )
			return BMT_DOBULE_CROWN_BOSS;
		else
			return BMT_DOBULE_CROWN;
	}
	else if( eSelectMode == MT_FACTORY )
	{
		int iReturnBMT = BMT_FACTORY_EASY + ( m_iSelectMap - 1 );
		if( !COMPARE( iReturnBMT, BMT_FACTORY_EASY, BMT_FACTORY_CHALLENGE + 1 ) )
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::GetSelectModeTerm Error : %d - %d", iReturnBMT, m_iSelectMap );
		else
			return iReturnBMT;
	}
	else if( eSelectMode == MT_RAID )
	{
		int iReturnBMT = BMT_RAID;
		return iReturnBMT;
	}

	return BMT_RANDOM;
}

bool BattleRoomNode::IsBattleTimeClose()
{
	if( !m_pBattleRoom || m_pBattleRoom->IsRoomEmpty() ) return false;
	if( m_pBattleRoom->GetModeType() == MT_TRAINING ) return false;
	if( m_pBattleRoom->GetModeType() == MT_HEADQUARTERS ) return false;
	if( m_pBattleRoom->GetModeType() == MT_HOUSE ) return false;
	if( !m_pBattleRoom->IsRoomProcess() ) return false;
	
	return m_pBattleRoom->IsTimeCloseRoom();
}

void BattleRoomNode::AddUser(const BattleRoomUser &kUser )
{
	m_vUserNode.push_back( kUser );
}

bool BattleRoomNode::RemoveUser( const DWORD dwUserIndex )
{
    vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			// P2P Relay Log
			P2PRelayLOG.PrintNoEnterLog( 0, "{[%d]<%s>(%d)+%d-}", GetIndex(), kUser.m_szPublicID.c_str(), kUser.m_iServerRelayCount, (int)kUser.IsNATUser() );
			m_vUserNode.erase( iter );			
			return true;
		}
	}
	return false;
}

void BattleRoomNode::SelectNewOwner()
{
	m_OwnerUserID.Clear();
	if( m_vUserNode.empty() ) return;

	ioHashString szObserver;

    vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( !kUser.m_bObserver )
		{
			m_OwnerUserID = kUser.m_szPublicID;
			break;
		}
		else if( szObserver.IsEmpty() )
		{
			szObserver = kUser.m_szPublicID;
		}
	}

	if( m_OwnerUserID.IsEmpty() && !szObserver.IsEmpty() )
	{
		m_OwnerUserID = szObserver;
		//������ ��ü�Ǹ� ��� ����
		SetPW( "" );
		SP2Packet kPacket( STPK_BATTLEROOM_INFO );
		FillBattleRoomInfo( kPacket );
		SendPacketTcp( kPacket );		
		m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );

		if( IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s �������� ������ ��: %s", __FUNCTION__, m_OwnerUserID.c_str() );
		}
	}
	else if( !m_OwnerUserID.IsEmpty() )
	{
		//������ ��ü�Ǹ� ��� ����
		SetPW( "" );
		SP2Packet kPacket( STPK_BATTLEROOM_INFO );
		FillBattleRoomInfo( kPacket );
		SendPacketTcp( kPacket );		
		m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );

		if( IsRegularTournamentBattle() )
		{
			LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ������ �ٲ�: %s", __FUNCTION__, m_OwnerUserID.c_str() );
		}
	}
}

ModeType BattleRoomNode::GetSelectIndexToMode( int iModeIndex, int iMapIndex )
{
	ModeType eModeType = MT_NONE;
	int      iSubType  = 0;

	if( iModeIndex != -1 )
		g_BattleRoomManager.CheckBattleRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );	
	return eModeType;
}

bool BattleRoomNode::IsFreeForAllMode( int iModeIndex, int iMapIndex )
{
	ModeType eModeType = GetSelectIndexToMode( iModeIndex, iMapIndex );
	if( eModeType == MT_SURVIVAL || eModeType == MT_MONSTER_SURVIVAL || eModeType == MT_DUNGEON_A || 
		eModeType == MT_FIGHT_CLUB || eModeType == MT_RAID ||
		Help::IsMonsterDungeonMode(eModeType) )
		return true;
	return false;
}

bool BattleRoomNode::IsSafetyModeCreate()
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( !kUser.m_bSafetyLevel )
			return false;
	}
	return true;
}

bool BattleRoomNode::KickOutUser( ioHashString &rkName )
{
	CRASH_GUARD();
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == rkName )
		{
			SP2Packet kPacket( STPK_MACRO_COMMAND );
			PACKET_GUARD_bool( kPacket.Write(MACRO_KICK_OUT) );
			PACKET_GUARD_bool( kPacket.Write(kUser.m_szPublicID) );

			SendPacketTcp( kPacket );			
			
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
			if( pUserParent )
			{
				if( pUserParent->IsUserOriginal() )
				{
					User *pUser = (User*)pUserParent;
					pUser->BattleRoomKickOut();
				}            
				else
				{
					UserCopyNode *pUser = (UserCopyNode*)pUserParent;
					SP2Packet kPacket( SSTPK_BATTLEROOM_KICK_OUT );
					PACKET_GUARD_bool( kPacket.Write(kUser.m_dwUserIndex) );
					pUser->SendMessage( kPacket );
				}
			}    
			return true;
		}
	}
	return false;
}

int BattleRoomNode::GetJoinUserCnt() const
{
	return m_vUserNode.size();
}

int BattleRoomNode::GetPlayUserCnt()
{
	int iPlayUserCnt = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter = m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;

		if( kUser.m_bObserver ) continue;

		iPlayUserCnt++;
	}

	return iPlayUserCnt;
}

int BattleRoomNode::GetMaxPlayer() const
{
	return m_iMaxPlayerBlue + m_iMaxPlayerRed;
}

int BattleRoomNode::GetMaxPlayerBlue() const
{
	return m_iMaxPlayerBlue;
}

int BattleRoomNode::GetMaxPlayerRed() const
{
	return m_iMaxPlayerRed;
}

int BattleRoomNode::GetMaxObserver() const
{
	return m_iMaxObserver;
}

int BattleRoomNode::GetSelectMode() const
{
	return m_iSelectMode;
}

int BattleRoomNode::GetSelectMap() const
{
	return m_iSelectMap;
}

void BattleRoomNode::SetName( const ioHashString &rkName )
{
	if( m_szRoomName != rkName )
		m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
	m_szRoomName = rkName;
}

void BattleRoomNode::SetPW( const ioHashString &rkPW )
{
	if( m_szRoomPW != rkPW )
		m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
	m_szRoomPW = rkPW;
}

void BattleRoomNode::SetMaxPlayer( int iBluePlayer, int iRedPlayer, int iObserver )
{
	if( m_iMaxPlayerBlue != iBluePlayer || m_iMaxPlayerRed  != iRedPlayer || m_iMaxObserver != iObserver )
		m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
	
	m_iMaxPlayerBlue = iBluePlayer;
    m_iMaxPlayerRed  = iRedPlayer;
	m_iMaxObserver = iObserver;
}

int BattleRoomNode::GetAbilityMatchLevel()
{
	if( m_bSafetyLevelRoom ) return 0;

	int iSize  = 0;
	int iLevel = 0;
	int iObserverSize = 0;
	int iObserverLevel = 0;

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();

	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		// ������ ���� ����
		if( !g_UserNodeManager.IsDeveloper( kUser.m_szPublicID.c_str() ) && !kUser.m_bObserver )
		{
			iSize++;
			iLevel += kUser.m_iAbilityLevel;
		}
	}

	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		// ������ ����
		if( !g_UserNodeManager.IsDeveloper( kUser.m_szPublicID.c_str() ) && kUser.m_bObserver )
		{
			iObserverSize++;
			iObserverLevel += kUser.m_iAbilityLevel;
		}
	}

	if( iSize <= 0 && iObserverSize > 0 )
	{
		iObserverLevel /= iObserverSize;
		iObserverLevel = min( max( iObserverLevel, 0 ), g_LevelMatchMgr.GetRoomEnterLevelMax() );
		return iObserverLevel;
	}
	else if( iSize <= 0 && iObserverSize <= 0 )
	{
		return 0;
	}

	iLevel /= iSize;
	iLevel = min( max( iLevel, 0 ), g_LevelMatchMgr.GetRoomEnterLevelMax() );
	return iLevel;
}

int BattleRoomNode::GetRoomLevel()
{
	int iMatchLevel = GetAbilityMatchLevel() - g_LevelMatchMgr.GetAddGradeLevel();
	iMatchLevel = min( max( iMatchLevel, 0 ), g_LevelMgr.GetMaxGradeLevel() );

	return iMatchLevel;
}

int BattleRoomNode::GetSortLevelPoint( int iMyLevel )
{
	int iSize = m_vUserNode.size();
	if( iSize == 0 )
		return BATTLE_ROOM_SORT_HALF_POINT;

	int iGapLevel = 5;
	int iLevel    = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		iLevel += kUser.m_iAbilityLevel;
	}
	iLevel /= iSize;
	return ( abs( iLevel - iMyLevel ) / iGapLevel );		
}

int BattleRoomNode::GetBattleEventType()
{
	return m_iBattleEventType;
}

bool BattleRoomNode::IsLevelMatchIgnore()
{
	if( GetBattleEventType() == BET_BROADCAST_AFRICA ||
		GetBattleEventType() == BET_BROADCAST_MBC || 
		GetBattleEventType() == BET_TOURNAMENT_BATTLE )
		return true;
	return false;
}

void BattleRoomNode::SetBattleEventType( int iBattleEventType )
{
	m_iBattleEventType = iBattleEventType;
	if( m_OwnerUserID.IsEmpty() )
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "(%d) BattleRoom EventType - %d", GetIndex(), GetBattleEventType() );
	else
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "(%d) BattleRoom EventType - %d : %s", GetIndex(), GetBattleEventType(), m_OwnerUserID.c_str() );

	if( GetBattleEventType() == BET_BROADCAST_MBC || GetBattleEventType() == BET_TOURNAMENT_BATTLE )
	{
		m_bRandomTeamMode = false;
		m_bAutoModeStart  = false;
	}
}

void BattleRoomNode::SetTournamentBattle( DWORD dwTourIndex, DWORD dwBlueIndex, BYTE BlueTourPos, SHORT BluePosition, DWORD dwRedIndex, BYTE RedTourPos, SHORT RedPosition )
{
	m_TournamentRoundData.Init();

	m_TournamentRoundData.m_dwTourIndex = dwTourIndex;
	m_TournamentRoundData.m_bRegularTour= g_TournamentManager.IsRegularTournament( dwTourIndex );

	m_TournamentRoundData.m_dwBlueIndex = dwBlueIndex;
	m_TournamentRoundData.m_BlueTourPos = BlueTourPos;
	m_TournamentRoundData.m_BluePosition= BluePosition;

	m_TournamentRoundData.m_dwRedIndex  = dwRedIndex;
	m_TournamentRoundData.m_RedTourPos  = RedTourPos;
	m_TournamentRoundData.m_RedPosition = RedPosition;

	m_TournamentRoundData.m_dwInviteTimer = TIMEGETTIME();
}

void BattleRoomNode::SetTournamentTeamChange( DWORD dwNewTeam )
{
	if( m_TournamentRoundData.m_dwBlueIndex == 0 )
	{
		m_TournamentRoundData.m_dwBlueIndex = dwNewTeam;
	}
	else
	{
		m_TournamentRoundData.m_dwRedIndex  = dwNewTeam;
	}

	SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
	PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TOURNAMENT_NEW_TEAM) );
	PACKET_GUARD_VOID( kPacket.Write(dwNewTeam) );
	SendPacketTcp( kPacket );
}

void BattleRoomNode::SyncPlayEnd( bool bAutoStart )
{
	SP2Packet kPacket( SSTPK_BATTLEROOM_SYNC );
	PACKET_GUARD_VOID( kPacket.Write(BattleRoomSync::BRS_PLAY) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectModeTerm()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMap()) );
	PACKET_GUARD_VOID( kPacket.Write(GetSelectMap()) );
	PACKET_GUARD_VOID( kPacket.Write(MT_NONE) );
	PACKET_GUARD_VOID( kPacket.Write(false) );
	PACKET_GUARD_VOID( kPacket.Write(m_bRandomTeamMode) );
	PACKET_GUARD_VOID( kPacket.Write(IsStartRoomEnterX()) );
	PACKET_GUARD_VOID( kPacket.Write(m_bUseExtraOption) );
	PACKET_GUARD_VOID( kPacket.Write(m_bNoChallenger) );
			/*
			<< m_bUseExtraOption << m_iChangeCharType
			<< m_iCoolTimeType << m_iRedHPType << m_iBlueHPType
			<< m_iDropDamageType << m_iGravityType
			<< m_iPreSetModeType << m_iTeamAttackType << m_iGetUpType
			<< m_iKOType << m_iRedBlowType << m_iBlueBlowType
			<< m_iRedMoveSpeedType << m_iBlueMoveSpeedType << m_iKOEffectType
			<< m_iRedEquipType << m_iBlueEquipType;
			*/

	g_ServerNodeManager.SendMessageToPartitions( kPacket );

/*	int iPlayUserCnt = GetPlayUserCnt();
	if( iPlayUserCnt <= 1 )
	{
		// �������� �̵�
		vBattleRoomUser_iter iter, iEnd;
		iEnd = m_vUserNode.end();
		for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
		{
			BattleRoomUser &kUser = *iter;
			User *pUser = g_UserNodeManager.GetUserNode( kUser.m_dwUserIndex );
			if( pUser )
			{
				pUser->PrivatelyLeaveRoomToTraining();
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::SyncPlayEnd(%d)(%s) �������� ���� ������ �̵����� �� �ִ�.", GetIndex(), kUser.m_szPublicID.c_str() );
			}
		}
		m_pBattleRoom = NULL;
	}
	else
*/	{
		// ���� �ϱ� ����
		SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
		PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_UI_SHOW_OK) );
		PACKET_GUARD_VOID( kPacket.Write(false) );
		PACKET_GUARD_VOID( kPacket.Write(bAutoStart) );
		SendPacketTcp( kPacket );
	}
}

void BattleRoomNode::SelectMode( const int iModeIndex )
{
	m_iSelectMode = iModeIndex;
}

void BattleRoomNode::SelectMap( const int iMapIndex )
{
	m_iSelectMap = iMapIndex;
}

TeamType BattleRoomNode::ChangeTeamType( const ioHashString &rkName, TeamType eTeam )
{
	CRASH_GUARD();
	TeamType eCurTeam = TEAM_NONE;

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == rkName )
		{
			if( IsBattleModePlaying() )        // ��� �÷������� ���̸� ������ ���� �ٲ۴�.
			{
				if( m_pBattleRoom->GetRoomStyle() == RSTYLE_BATTLEROOM && 
					m_pBattleRoom->IsPartyProcessEnd() )
				{
					kUser.m_eTeamType = eTeam;

					// ������ ��ȯ
					if( eTeam == TEAM_NONE && !kUser.m_bObserver )
						kUser.m_bObserver = true;
					else if( eTeam != TEAM_NONE && kUser.m_bObserver )
						kUser.m_bObserver = false;

					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
					if( pUserParent && pUserParent->IsUserOriginal() )
					{
						User *pUser = (User*)pUserParent;
						pUser->SetTeam( eTeam );
					}

					eCurTeam = eTeam;
				}
			}
			else
			{
				kUser.m_eTeamType = eTeam;

				// ������ ��ȯ
				if( eTeam == TEAM_NONE && !kUser.m_bObserver )
					kUser.m_bObserver = true;
				else if( eTeam != TEAM_NONE && kUser.m_bObserver )
					kUser.m_bObserver = false;

				eCurTeam = eTeam;
			}

			break;
		}
	}

	return eCurTeam;
}

bool BattleRoomNode::IsObserver( const ioHashString &szPublicID )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == szPublicID )
			return kUser.m_bObserver;
	}

	return false;
}

void BattleRoomNode::SetPreSelectModeInfo( const ModeType eModeType, const int iSubType, const int iMapNum )
{
	m_PreModeType = eModeType;
	m_iPreSubType = iSubType;
	m_iPreMapNum  = iMapNum;
}

void BattleRoomNode::GetPreSelectModeInfo( ModeType &eModeType, int &iSubType, int &iMapNum )
{
	eModeType = m_PreModeType;
	iSubType = m_iPreSubType;
	iMapNum = m_iPreMapNum;
}

bool BattleRoomNode::IsReserveTimeOver()
{
	if( !m_vUserNode.empty() ) return false;

	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE )
	{
		if( TIMEGETTIME() - m_TournamentRoundData.m_dwInviteTimer > TournamentRoundData::TOURNAMENT_ROOM_CLOSE_TIME )
			return true;
	}
	else
	{
		if( m_dwReserveTime == 0 ) return false;
		if( TIMEGETTIME() - m_dwReserveTime > CREATE_RESERVE_DELAY_TIME )
			return true;
	}
	return false;
}

void BattleRoomNode::FillBattleRoomInfo( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID( rkPacket.Write(m_dwIndex) );
	PACKET_GUARD_VOID( rkPacket.Write(m_szRoomName) );
	PACKET_GUARD_VOID( rkPacket.Write(m_OwnerUserID) );
	PACKET_GUARD_VOID( rkPacket.Write(GetJoinUserCnt()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetPlayUserCnt()) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxPlayerBlue) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxPlayerRed) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iMaxObserver) );
	PACKET_GUARD_VOID( rkPacket.Write(m_szRoomPW) );
	PACKET_GUARD_VOID( rkPacket.Write(GetRoomLevel()) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bUseExtraOption) );
	PACKET_GUARD_VOID( rkPacket.Write(m_bNoChallenger) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iChangeCharType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iCoolTimeType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedHPType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueHPType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iDropDamageType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iGravityType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iPreSetModeType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iTeamAttackType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iGetUpType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iKOType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedBlowType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueBlowType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedMoveSpeedType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueMoveSpeedType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iKOEffectType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedEquipType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueEquipType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iCatchModeRoundType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iCatchModeRoundTimeType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iGrowthUseType) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iExtraItemUseType) );

	// �뿡 �ִ� ���� �뿡 ���� ��� Ŭ���̾�Ʈ�� �ٸ� ó���� �Ѵ�. 
	// �뿡 ������ ������ �̵��ϰ� �뿡 ������ �κ��� ���������� �̵��Ѵ�.
	PACKET_GUARD_VOID( rkPacket.Write(IsBattleModePlaying()) );
}

void BattleRoomNode::FillBattleRoomInfoState( SP2Packet &rkPacket, UserParent *pUserParent, DWORD dwPrevBattleIndex )
{
	if( !pUserParent )
	{
		PACKET_GUARD_VOID( rkPacket.Write(0) );
		PACKET_GUARD_VOID( rkPacket.Write(GetBattleEventType()) );
		PACKET_GUARD_VOID( rkPacket.Write(GetTournamentIndex()) );
	}
	else
	{
		PACKET_GUARD_VOID( rkPacket.Write(g_BattleRoomManager.GetSortBattleRoomState( (BattleRoomParent*)this, pUserParent, dwPrevBattleIndex )) );
		PACKET_GUARD_VOID( rkPacket.Write(GetBattleEventType()) );
		PACKET_GUARD_VOID( rkPacket.Write(GetTournamentIndex()) );
	}
}

void BattleRoomNode::FillUserList( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID( rkPacket.Write(GetJoinUserCnt()) );

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		PACKET_GUARD_VOID( rkPacket.Write(kUser.m_iGradeLevel) );
		PACKET_GUARD_VOID( rkPacket.Write(kUser.m_szPublicID) );
		PACKET_GUARD_VOID( rkPacket.Write(kUser.m_bObserver) );
		PACKET_GUARD_VOID( rkPacket.Write((int)kUser.m_eTeamType) );

		//��� ���� & PING
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pUserParent )
		{
			PACKET_GUARD_VOID( rkPacket.Write(pUserParent->GetGuildIndex() ) );
			PACKET_GUARD_VOID( rkPacket.Write(pUserParent->GetGuildMark()) );
			PACKET_GUARD_VOID( rkPacket.Write(pUserParent->GetPingStep()) );
		}
		else
		{
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
		}
	}
}

void BattleRoomNode::FillUserInfo( SP2Packet &rkPacket, const BattleRoomUser &rkUser )
{
	PACKET_GUARD_VOID( rkPacket.Write(GetRoomLevel()) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_szPublicID) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_iGradeLevel) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_iAbilityLevel) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_bSafetyLevel) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_bObserver) );
	PACKET_GUARD_VOID( rkPacket.Write((int)rkUser.m_eTeamType) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_szPublicIP) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_iClientPort) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_szPrivateIP) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_szTransferIP) );
	PACKET_GUARD_VOID( rkPacket.Write(rkUser.m_iTransferPort) );

	//��� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( rkUser.m_dwUserIndex );
	if( pUserParent )
	{
		PACKET_GUARD_VOID( rkPacket.Write(pUserParent->GetGuildIndex()) );
		PACKET_GUARD_VOID( rkPacket.Write(pUserParent->GetGuildMark()) );
	}
	else
	{
		PACKET_GUARD_VOID( rkPacket.Write(0) );
		PACKET_GUARD_VOID( rkPacket.Write(0) );
	}
}

void BattleRoomNode::FillModeInfo( SP2Packet &rkPacket )
{
	// BMT ����
	PACKET_GUARD_VOID( rkPacket.Write(GetSelectModeTerm()) );
	
	// ��� ����
	enum{ BST_START = 0, BST_PLAYING = 1, BST_RESULT = 2, };
	if( m_pBattleRoom && m_pBattleRoom->IsRoomProcess() )
	{
		// ��� �÷��� ���� 
		if( m_pBattleRoom->IsFinalRoundResult() )          // ���� ������
		{
			PACKET_GUARD_VOID( rkPacket.Write((int)BST_RESULT) );
			PACKET_GUARD_VOID( rkPacket.Write((int)m_pBattleRoom->GetModeType()) );
		}
		else
		{
			PACKET_GUARD_VOID( rkPacket.Write((int)BST_PLAYING) );
			PACKET_GUARD_VOID( rkPacket.Write((int)m_pBattleRoom->GetModeType()) );

			if( m_pBattleRoom->GetModeType() != MT_MONSTER_SURVIVAL && m_pBattleRoom->GetModeType() != MT_DUNGEON_A && 
				m_pBattleRoom->GetModeType() != MT_RAID &&
				!Help::IsMonsterDungeonMode(m_pBattleRoom->GetModeType()) )
			{
				if( m_pBattleRoom->GetModeType() == MT_SURVIVAL ||
					m_pBattleRoom->GetModeType() == MT_TEAM_SURVIVAL ||
					m_pBattleRoom->GetModeType() == MT_DOBULE_CROWN ||
					m_pBattleRoom->GetModeType() == MT_BOSS ||
					m_pBattleRoom->GetModeType() == MT_GANGSI ||
					m_pBattleRoom->GetModeType() == MT_TEAM_SURVIVAL_AI ||
					m_pBattleRoom->GetModeType() == MT_FIGHT_CLUB )
				{
					//���� �ð� ����
					PACKET_GUARD_VOID( rkPacket.Write(m_pBattleRoom->GetRemainPlayTime()) );
				}
				else
				{
					//�� ���ھ� ����
					PACKET_GUARD_VOID( rkPacket.Write(m_pBattleRoom->GetBlueWinCnt()) );
					PACKET_GUARD_VOID( rkPacket.Write(m_pBattleRoom->GetRedWinCnt()) );
				}
			}
		}
	}
	else  
	{
		// ������ ��� ����(���� ������)
		PACKET_GUARD_VOID( rkPacket.Write((int)BST_START) );
		PACKET_GUARD_VOID( rkPacket.Write((int)GetSelectIndexToMode( GetSelectMode(), GetSelectMap() )) );
	}
}

void BattleRoomNode::FillRecord( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueWin) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueLose) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iBlueVictories) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedWin) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedLose) );
	PACKET_GUARD_VOID( rkPacket.Write(m_iRedVictories) );
}

bool BattleRoomNode::IsFull()
{
	int iPlayCnt = GetPlayUserCnt();
	if( iPlayCnt >= GetMaxPlayer() )
		return true;

	return false;
}

bool BattleRoomNode::IsMapLimitPlayerFull()
{
	int iPlayCnt = GetPlayUserCnt();
	if( iPlayCnt >= g_BattleRoomManager.GetBattleMapToLimitPlayer( GetSelectMode(), GetSelectMap() ) )
		return true;

	return false;
}

bool BattleRoomNode::IsMapLimitGrade( int iGradeLevel )
{
	if( iGradeLevel < g_BattleRoomManager.GetBattleMapToLimitGrade( GetSelectMode(), GetSelectMap() ) )
		return true;
	return false;
}

bool BattleRoomNode::IsObserverFull()
{
	int iObserverCnt = 0;
	int iJoiner = GetJoinUserCnt();
	int iPlayer = GetPlayUserCnt();

	if( iJoiner > iPlayer )
		iObserverCnt = iJoiner - iPlayer;

	if( iObserverCnt >= m_iMaxObserver )
		return true;

	return false;
}

bool BattleRoomNode::IsEmptyBattleRoom()
{
	return m_vUserNode.empty();
}

void BattleRoomNode::InsertInviteUser( DWORD dwUserIndex )
{
	InviteUser kInviteUser;
	kInviteUser.m_dwInviteTime  = TIMEGETTIME();
	kInviteUser.m_dwUserIndex   = dwUserIndex;
	m_vInviteUser.push_back( kInviteUser );
}

bool BattleRoomNode::IsInviteUser( DWORD dwUserIndex )
{
	int iSize = (int)m_vInviteUser.size();
	for(int i = 0;i < iSize;i++)
	{
		InviteUser &kInviteUser = m_vInviteUser[i];
		if( kInviteUser.m_dwUserIndex == dwUserIndex )
		{
			if( TIMEGETTIME() - kInviteUser.m_dwInviteTime > 20000 )
			{
				m_vInviteUser.erase( m_vInviteUser.begin() + i );
				return false;
			}
			else
			{
				m_vInviteUser.erase( m_vInviteUser.begin() + i );
				return true;
			}
		}
	}
	return false;
}

BattleRoomUser &BattleRoomNode::GetUserNodeByIndex( DWORD dwUserIndex )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
			return kUser;
	}

	static BattleRoomUser kReturn;
	return kReturn;
}

const BattleRoomUser &BattleRoomNode::GetUserNodeByArray( int iArray )
{
	if( COMPARE( iArray, 0, (int) m_vUserNode.size()) )
		return m_vUserNode[iArray];

	static BattleRoomUser kError;
	return kError;
}

UserParent *BattleRoomNode::GetUserNode( const ioHashString &szPublicID )
{
	CRASH_GUARD();
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == szPublicID )
			return g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
	}
	return NULL;
}

TeamType BattleRoomNode::GetUserTeam( const ioHashString &szPublicID )
{
	CRASH_GUARD();
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == szPublicID )
			return kUser.m_eTeamType;
	}
	return TEAM_NONE;
}

int BattleRoomNode::GetUserTeamCount( TeamType eTeam )
{
	int iCount = 0;
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;

		if( eTeam == TEAM_NONE && kUser.m_bObserver )
			iCount++;
		else if( eTeam != TEAM_NONE && kUser.m_eTeamType == eTeam )
			iCount++;
	}
	return iCount;
}

int BattleRoomNode::GetMaxPlayer( TeamType eTeam )
{
	if( eTeam == TEAM_BLUE )
		return m_iMaxPlayerBlue;
	return m_iMaxPlayerRed;
}

void BattleRoomNode::UpdateRecord( TeamType eWinTeam )
{
	if( eWinTeam == TEAM_BLUE )
	{
		m_iBlueWin++;
	    m_iBlueVictories++;

		m_iRedLose++;
		m_iRedVictories = 0;
	}
	else if( eWinTeam == TEAM_RED )
	{
		m_iRedWin++;
		m_iRedVictories++;

		m_iBlueLose++;
		m_iBlueVictories = 0;
	}
	else
	{
		m_iBlueVictories = 0;
		m_iRedVictories = 0;
	}
}

bool BattleRoomNode::IsChangeEnterSyncData()
{
	if( m_iBlueWin > 0 )
		return true;
	else if( m_iBlueLose > 0 )
		return true;
	else if( m_iBlueVictories > 0 )
		return true;
	else if( m_iRedWin > 0 )
		return true;
	else if( m_iRedLose > 0 )
		return true;
	else if( m_iRedVictories > 0 )
		return true;
	else if( !m_bRandomTeamMode )
		return true;
	else if( m_bStartRoomEnterX )
		return true;
	else if( !m_bAutoModeStart )
		return true;
	else if( m_bBadPingKick )
		return true;
	else if( m_bUseExtraOption )
		return true;
	else if( m_bNoChallenger )
		return true;
	else if( m_iChangeCharType > 0 )
		return true;
	else if( m_iTeamAttackType > 0 )
		return true;
	else if( m_iCoolTimeType > 0 )
		return true;
	else if( m_iRedHPType > 0 )
		return true;
	else if( m_iBlueHPType > 0 )
		return true;
	else if( m_iDropDamageType > 0 )
		return true;
	else if( m_iGravityType > 0 )
		return true;
	else if( m_iGetUpType > 0 )
		return true;
	else if( m_iRedMoveSpeedType > 0 )
		return true;
	else if( m_iBlueMoveSpeedType > 0 )
		return true;
	else if( m_iRedEquipType > 0 )
		return true;
	else if( m_iBlueEquipType > 0 )
		return true;
	else if( m_iKOType > 0 )
		return true;
	else if( m_iKOEffectType > 0 )
		return true;
	else if( m_iRedBlowType > 0 )
		return true;
	else if( m_iBlueBlowType > 0 )
		return true;
	else if( m_iCatchModeRoundType > 0 )
		return true;
	else if( m_iCatchModeRoundTimeType > 0 )
		return true;
	else if( m_iPreSetModeType > 0 )
		return true;
	else if( m_iGrowthUseType > 0 )
		return true;
	else if( m_iExtraItemUseType > 0 )
		return true;
	else if( m_iBattleEventType != BET_NORMAL )
		return true;

	return false;
}

void BattleRoomNode::CreateBoss()
{
	ioHashStringVec vOriginalUserName;
	ioHashStringVec vCopyUserName;

	vBattleRoomUser_iter iter;
	for(iter=m_vUserNode.begin();iter!=m_vUserNode.end();++iter)
	{
		BattleRoomUser &kUser = *iter;
		
		// ������ �н�
		if( kUser.m_bObserver ) continue;

		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( !pUser ) continue;
		
		if( pUser->IsUserOriginal() )
		{
			vOriginalUserName.push_back( pUser->GetPublicID() );
		}
		else
		{
			vCopyUserName.push_back( pUser->GetPublicID() );
		}
	}

	if( !vOriginalUserName.empty() )        
	{
		//���� ������ �ִ� ������ ����
		int iSize = vOriginalUserName.size();
		int rSelect = rand()%iSize;
		m_szBossName = vOriginalUserName[rSelect];

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateBoss(%d) ���� ���� - [%s]��÷!", GetIndex(), m_szBossName.c_str() );
	}
	else if( !vCopyUserName.empty() ) 
	{
		//Ÿ������ �ִ� ������ ����
		int iSize = vCopyUserName.size();
		int rSelect = rand()%iSize;
		m_szBossName = vCopyUserName[rSelect];

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateBoss(%d) Ÿ ���� - [%s]��÷!", GetIndex(), m_szBossName.c_str() );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateBoss(%d) : ������ ���õ� ������ ����!!", GetIndex() );
	}
	vOriginalUserName.clear();
	vCopyUserName.clear();
}

const ioHashString &BattleRoomNode::GetBossName()
{
	return m_szBossName;
}

void BattleRoomNode::ClearBossName()
{
	m_szBossName.Clear();
}

void BattleRoomNode::CreateGangsi()
{
	ioHashStringVec vOriginalUserName;
	ioHashStringVec vCopyUserName;

	vBattleRoomUser_iter iter;
	for(iter=m_vUserNode.begin();iter!=m_vUserNode.end();++iter)
	{
		BattleRoomUser &kUser = *iter;

		// ������ �н�
		if( kUser.m_bObserver ) continue;

		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( !pUser ) continue;

		if( pUser->IsUserOriginal() )
		{
			vOriginalUserName.push_back( pUser->GetPublicID() );
		}
		else
		{
			vCopyUserName.push_back( pUser->GetPublicID() );
		}
	}

	if( !vOriginalUserName.empty() )        
	{
		//���� ������ �ִ� ������ ����
		int iSize = vOriginalUserName.size();
		int rSelect = rand()%iSize;
		m_szGangsiName = vOriginalUserName[rSelect];

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateGangsi(%d) ���� ���� - [%s]��÷!", GetIndex(), m_szGangsiName.c_str() );
	}
	else if( !vCopyUserName.empty() ) 
	{
		//Ÿ������ �ִ� ������ ����
		int iSize = vCopyUserName.size();
		int rSelect = rand()%iSize;
		m_szGangsiName = vCopyUserName[rSelect];

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateGangsi(%d) Ÿ ���� - [%s]��÷!", GetIndex(), m_szGangsiName.c_str() );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::CreateGangsi(%d) : ������ ���õ� ������ ����!!", GetIndex() );
	}
	vOriginalUserName.clear();
	vCopyUserName.clear();
}

const ioHashString &BattleRoomNode::GetGangsiName()
{
	return m_szGangsiName;
}

void BattleRoomNode::ClearGangsi()
{
	m_szGangsiName.Clear();
}
	
void BattleRoomNode::SendPacketTcp( SP2Packet &rkPacket, const DWORD dwUserIndex )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex ) continue;

		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pUser )
			pUser->RelayPacket( rkPacket );
	}
}

void BattleRoomNode::SendPacketTcpUser( SP2Packet &rkPacket, const ioHashString &rkSenderName )
{
	CRASH_GUARD();
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_szPublicID == rkSenderName ) 
		{
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( rkSenderName );
			if( pUser )
				pUser->RelayPacket( rkPacket );
			return;
		}		
	}
}

void BattleRoomNode::SendPacketUdp( SP2Packet &rkPacket, const DWORD dwUserIndex /* = 0  */ )
{
	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		BattleRoomUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex ) continue;

		g_UDPNode.SendMessage( (char*)kUser.m_szPublicIP.c_str(), kUser.m_iClientPort, rkPacket );
	}
}

void BattleRoomNode::Process()
{
	m_NodeSync.Process();

	TournamentProcess();
}

void BattleRoomNode::TournamentProcess()
{
	if( GetBattleEventType() != BET_TOURNAMENT_BATTLE ) return;
	
	if( m_TournamentRoundData.m_bRegularTour )
	{
		// ���� ��ȸ ���μ���

	}
	else
	{
		// ���� ��ȸ ���μ���
		CustomTournamentRandomWinProcess();
	}
}

void BattleRoomNode::CustomTournamentRandomWinProcess()
{
	if( m_pBattleRoom ) return;		// ��Ⱑ ���۵Ǿ���
	if( m_TournamentRoundData.m_bSendResult ) return;    // �̹� ����� ���۵Ǿ���.
	if( TIMEGETTIME() - m_TournamentRoundData.m_dwInviteTimer < TournamentRoundData::CUSTOM_TOURNAMENT_RANDOM_WIN_TIME ) return;
	if( GetUserTeamCount( TEAM_BLUE ) != 0 || GetUserTeamCount( TEAM_RED ) != 0 ) return;          // ������ ������ ����.

	if( (BOOL)(rand() % 2) == TRUE )
	{
		SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_RESULT );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwBlueIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwRedIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_BlueTourPos + 1) );

		g_MainServer.SendMessage( kPacket );
	}
	else
	{
		SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_RESULT );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwRedIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwBlueIndex) );
		PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_RedTourPos + 1) );

		g_MainServer.SendMessage( kPacket );
	}
	m_TournamentRoundData.m_bSendResult = true;
}

void BattleRoomNode::OnBattleRoomInfo( UserParent *pUser, int iPrevBattleIndex )
{
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::OnBattleRoomInfo Null User Pointer!!(%d)", GetIndex() );
		return;
	}

	SP2Packet kPacket( STPK_BATTLEROOM_JOIN_INFO );
	FillBattleRoomInfo( kPacket );
	FillBattleRoomInfoState( kPacket, pUser, iPrevBattleIndex );
	FillUserList( kPacket );
	FillModeInfo( kPacket );
	pUser->RelayPacket( kPacket );
}

bool BattleRoomNode::OnProcessPacket( SP2Packet &rkPacket, UserParent *pUser )
{
	switch( rkPacket.GetPacketID() )
	{
	case CTPK_MACRO_COMMAND:
		OnMacroCommand( rkPacket, pUser );
		return true;
	case CTPK_VOICE_INFO:
		OnVoiceInfo( rkPacket, pUser );
		return true;
	case CTPK_BATTLEROOM_INVITE:
		OnBattleRoomInvite( rkPacket, pUser );
		return true;
	case CTPK_BATTLEROOM_COMMAND:
		OnBattleRoomCommand( rkPacket, pUser );
		return true;
	}
	return false;
}

void BattleRoomNode::OnMacroCommand( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;

	int command_type = 0;
	PACKET_GUARD_VOID( rkPacket.Read(command_type) );

	bool bCommandBlock = false;
	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE )
		bCommandBlock = true;

	SP2Packet kPacket( STPK_MACRO_COMMAND );
	switch( command_type )
	{
	case MACRO_KICK_OUT:
		{
			CRASH_GUARD();
			if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
			{
				ioHashString rkName;
				PACKET_GUARD_VOID( rkPacket.Read(rkName) );

				if( !KickOutUser( rkName ) )
				{
					PACKET_GUARD_VOID( kPacket.Write(MACRO_KICK_OUT_FAILED) );
					PACKET_GUARD_VOID( kPacket.Write(rkName) );
					pUser->RelayPacket( kPacket );
				}
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�����ڰ� �ƴѵ� ���� ��ȣ ����(%d) : %s - %d", GetIndex(), pUser->GetPublicID().c_str(), (int)bCommandBlock );
		}
		break;
	case MACRO_ROOM_NAME_PW:
		{
			CRASH_GUARD();
			if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
			{
				ioHashString rkName, rkPW;

				PACKET_GUARD_VOID( rkPacket.Read(rkName) );
				PACKET_GUARD_VOID( rkPacket.Read(rkPW) );

				SetName( rkName );
				SetPW( rkPW );
				PACKET_GUARD_VOID( kPacket.Write(MACRO_ROOM_NAME_PW) );
				PACKET_GUARD_VOID( kPacket.Write(rkName) );
				PACKET_GUARD_VOID( kPacket.Write(rkPW) );

				SendPacketTcp( kPacket, pUser->GetUserIndex() );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�����ڰ� �ƴѵ� ����/��� ���� ��ȣ ����(%d) : %s - %d", GetIndex(), pUser->GetPublicID().c_str(), (int)bCommandBlock );
		}
		break;
	case MACRO_PLAYER_CNT:
		{
			CRASH_GUARD();
			if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
			{
				int iBlueCnt = 0, iRedCnt = 0, iObserver = 0, iWantBlue = 0, iWantRed = 0, iWantObserver = 0;
				
				PACKET_GUARD_VOID( rkPacket.Read(iBlueCnt) );
				PACKET_GUARD_VOID( rkPacket.Read(iRedCnt) );
				PACKET_GUARD_VOID( rkPacket.Read(iObserver) );

				iWantBlue = iBlueCnt;
				iWantRed  = iRedCnt;
				iWantObserver = iObserver;

				//�ƹ��� �ʴ밡 �ǹǷ� �����Ϸ��� �������� ���� �� �ִ�.
				if( iBlueCnt < GetUserTeamCount( TEAM_BLUE ) )         
					iBlueCnt = GetUserTeamCount( TEAM_BLUE );
				if( iRedCnt < GetUserTeamCount( TEAM_RED ) )         
					iRedCnt = GetUserTeamCount( TEAM_RED );
				if( iObserver < GetUserTeamCount( TEAM_NONE ) )
					iObserver = GetUserTeamCount( TEAM_NONE );

				SetMaxPlayer( iBlueCnt, iRedCnt, iObserver );
				PACKET_GUARD_VOID( kPacket.Write(MACRO_PLAYER_CNT) );
				PACKET_GUARD_VOID( kPacket.Write(iBlueCnt) );
				PACKET_GUARD_VOID( kPacket.Write(iRedCnt) );
				PACKET_GUARD_VOID( kPacket.Write(iObserver) );

				SendPacketTcp( kPacket );

				if( iWantBlue != iBlueCnt )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "������ ����� �ο� ���� �ȵ�(%d) : %s - %d - %d", GetIndex(), pUser->GetPublicID().c_str(), iWantBlue, iBlueCnt );
				}

				if( iWantRed != iRedCnt )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "������ ������ �ο� ���� �ȵ�(%d) : %s - %d - %d", GetIndex(), pUser->GetPublicID().c_str(), iWantRed, iRedCnt );
				}

				if( iWantObserver != iObserver )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "������ ������ �ο� ���� �ȵ�(%d) : %s - %d - %d", GetIndex(), pUser->GetPublicID().c_str(), iWantObserver, iObserver );
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�����ڰ� �ƴѵ� �ο� ���� ��ȣ ����(%d) : %s - %d", GetIndex(), pUser->GetPublicID().c_str(), (int)bCommandBlock );
		}
		break;
	case MACRO_OPTION_CHANGE:
		{
			CRASH_GUARD();
			if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
			{
				bool bPrevRoomEnterX = m_bStartRoomEnterX;
				bool bPrevRandomTeamMode = m_bRandomTeamMode;
				bool bPrevuseExtraOption = m_bUseExtraOption;
				bool bPrevNoChallenger = m_bNoChallenger;


				int iPrevTeamAttackType = m_iTeamAttackType;
				int iPrevChangeCharType = m_iChangeCharType;
				int iPrevCoolTimeType = m_iCoolTimeType;
				int iPreRedHPType = m_iRedHPType;
				int iPreBlueHPType = m_iBlueHPType;
				int iPreDropDamageType = m_iDropDamageType;
				int iPreGravityType = m_iGravityType;
				int iPreGetUpType = m_iGetUpType;
				int iPreRedMoveSpeedType = m_iRedMoveSpeedType;
				int iPreBlueMoveSpeedType = m_iBlueMoveSpeedType;
				int iPreKOType = m_iKOType;
				int iPreKOEffectType = m_iKOEffectType;
				int iPreRedBlowType = m_iRedBlowType;
				int iPreBlueBlowType = m_iBlueBlowType;
				int iPreRedEquipType = m_iRedEquipType;
				int iPreBlueEquipType = m_iBlueEquipType;
				int iPreCatchModeRoundType = m_iCatchModeRoundType;
				int iPreCatchModeRoundTimeType = m_iCatchModeRoundTimeType;
				int iPreGrowthUseType = m_iGrowthUseType;
				int iPreExtraItemUseType = m_iExtraItemUseType;


				int iPrePreSetModeType = m_iPreSetModeType;

				PACKET_GUARD_VOID( rkPacket.Read(m_bRandomTeamMode) );
				PACKET_GUARD_VOID( rkPacket.Read(m_bStartRoomEnterX) );
				PACKET_GUARD_VOID( rkPacket.Read(m_bAutoModeStart) );
				PACKET_GUARD_VOID( rkPacket.Read(m_bBadPingKick) );
				PACKET_GUARD_VOID( rkPacket.Read(m_bUseExtraOption) );
				PACKET_GUARD_VOID( rkPacket.Read(m_bNoChallenger) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iChangeCharType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iCoolTimeType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iRedHPType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iBlueHPType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iDropDamageType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iGravityType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iPreSetModeType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iTeamAttackType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iGetUpType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iKOType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iRedBlowType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iBlueBlowType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iRedMoveSpeedType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iBlueMoveSpeedType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iKOEffectType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iRedEquipType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iBlueEquipType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iCatchModeRoundType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iCatchModeRoundTimeType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iGrowthUseType) );
				PACKET_GUARD_VOID( rkPacket.Read(m_iExtraItemUseType) );

				PACKET_GUARD_VOID( kPacket.Write(MACRO_OPTION_CHANGE) );
				PACKET_GUARD_VOID( kPacket.Write(m_bRandomTeamMode) );
				PACKET_GUARD_VOID( kPacket.Write(m_bStartRoomEnterX) );
				PACKET_GUARD_VOID( kPacket.Write(m_bAutoModeStart) );
				PACKET_GUARD_VOID( kPacket.Write(m_bBadPingKick) );
				PACKET_GUARD_VOID( kPacket.Write(m_bUseExtraOption) );
				PACKET_GUARD_VOID( kPacket.Write(m_bNoChallenger) );
				PACKET_GUARD_VOID( kPacket.Write(m_iChangeCharType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iCoolTimeType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iRedHPType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iBlueHPType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iDropDamageType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iGravityType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iPreSetModeType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iTeamAttackType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iGetUpType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iKOType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iRedBlowType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iBlueBlowType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iRedMoveSpeedType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iBlueMoveSpeedType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iKOEffectType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iRedEquipType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iBlueEquipType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iCatchModeRoundType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iCatchModeRoundTimeType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iGrowthUseType) );
				PACKET_GUARD_VOID( kPacket.Write(m_iExtraItemUseType) );

				SendPacketTcp( kPacket, pUser->GetUserIndex() );

				if( bPrevRoomEnterX != m_bStartRoomEnterX ||
					bPrevRandomTeamMode != m_bRandomTeamMode ||
					bPrevuseExtraOption != m_bUseExtraOption ||
					bPrevNoChallenger != m_bNoChallenger )
				{
					m_NodeSync.Update( BattleRoomSync::BRS_PLAY );
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�����ڰ� �ƴѵ� �ɼ� ���� ��ȣ ����(%d) : %s - %d", GetIndex(), pUser->GetPublicID().c_str(), (int)bCommandBlock );
		}
		break;
	case MACRO_AI_LEVEL_CHANGE:
		{
			CRASH_GUARD();
			{
				if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
				{
					PACKET_GUARD_VOID( rkPacket.Read(m_iAILevel) );
					PACKET_GUARD_VOID( kPacket.Write(MACRO_AI_LEVEL_CHANGE) );
					PACKET_GUARD_VOID( kPacket.Write(m_iAILevel) );
					SendPacketTcp( kPacket, pUser->GetUserIndex() );
				}
				else
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�����ڰ� �ƴѵ� AI���� ���� ��ȣ ����(%d) : %s - %d", GetIndex(), pUser->GetPublicID().c_str(), (int)bCommandBlock );
			}
		}
		break;
	}
}

void BattleRoomNode::OnVoiceInfo( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;

	int iType = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iType) );

	if( !COMPARE( iType, ID_VOICE_ON, ID_VOICE_PERMIT + 1) ) return;

	ioHashString szReceiverID;
	PACKET_GUARD_VOID( rkPacket.Read(szReceiverID) );

	SP2Packet kReturnPacket( STPK_VOICE_INFO );
	PACKET_GUARD_VOID( kReturnPacket.Write(iType) );
	PACKET_GUARD_VOID( kReturnPacket.Write(pUser->GetPublicID()) );
	PACKET_GUARD_VOID( kReturnPacket.Write(GetUserTeam( pUser->GetPublicID() )) );

	if( szReceiverID.IsEmpty() ) // ID�� ��� ������ ��Ƽ�� ��ο��� ����
		SendPacketTcp( kReturnPacket, pUser->GetUserIndex() );
	else
	{
		UserParent *pReceiver = GetUserNode( szReceiverID );
		if( pReceiver )
			pReceiver->RelayPacket( kReturnPacket );
	}	

#ifdef _DEBUG
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%d:%s)%d | %s", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iType, szReceiverID.c_str() );
#endif
}

void BattleRoomNode::OnBattleRoomInvite( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;

	int iSize = 0;
	ioHashString szInvitedID;
	PACKET_GUARD_VOID( rkPacket.Read(iSize) );
	PACKET_GUARD_VOID( rkPacket.Read(szInvitedID) );
	MAX_GUARD(iSize, 50);

	if( IsFull() )
	{
		SP2Packet kPacket( STPK_BATTLEROOM_INVITE_RESULT );
		PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_INVITE_USER_FULL) );
		pUser->RelayPacket( kPacket );
		return;
	}
	else
	{
		if( iSize == 1 )
		{
			DWORD dwLevel = 0, dwGuildIndex = 0, dwGuildMark = 0;
			PACKET_GUARD_VOID( rkPacket.Read(dwLevel) );
			PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );
			PACKET_GUARD_VOID( rkPacket.Read(dwGuildMark) );

			SP2Packet kPacket( STPK_BATTLEROOM_INVITE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_INVITE_OK) );
			PACKET_GUARD_VOID( kPacket.Write(iSize) );
			PACKET_GUARD_VOID( kPacket.Write(szInvitedID) );
			PACKET_GUARD_VOID( kPacket.Write(dwLevel) );
			PACKET_GUARD_VOID( kPacket.Write(dwGuildIndex) );
			PACKET_GUARD_VOID( kPacket.Write(dwGuildMark) );
			pUser->RelayPacket( kPacket );		
		}		
		else
		{
			SP2Packet kPacket( STPK_BATTLEROOM_INVITE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_INVITE_OK) );
			PACKET_GUARD_VOID( kPacket.Write(iSize) );
			pUser->RelayPacket( kPacket );		
		}
	}
	//	else if( IsBattleTimeClose() )
	//	{
	//		SP2Packet kPacket( STPK_BATTLEROOM_INVITE_RESULT );
	//		kPacket << BATTLEROOM_INVITE_TIME_CLOSE;
	//		pUser->RelayPacket( kPacket );
	//		return;
	//	}

	SP2Packet kPacket( STPK_BATTLEROOM_INVITE );
	FillBattleRoomInfo( kPacket );
	FillUserList( kPacket );		
	FillModeInfo( kPacket );
	// ù��° ���� ����
	UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( szInvitedID );
	if( pInviteUser && pInviteUser->GetUserPos() != UP_BATTLE_ROOM )
	{
		pInviteUser->RelayPacket( kPacket );
		InsertInviteUser( pInviteUser->GetUserIndex() );
	}
	// ���� ���� ����
	for(int i = 1;i < iSize;i++)
	{		
		PACKET_GUARD_VOID( rkPacket.Read(szInvitedID) );
		UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( szInvitedID );
		if( pInviteUser && pInviteUser->GetUserPos() != UP_BATTLE_ROOM )
		{
			pInviteUser->RelayPacket( kPacket );
			InsertInviteUser( pInviteUser->GetUserIndex() );
		}
	}	
}

void BattleRoomNode::OnBattleRoomCommand( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;

	int iCommand = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iCommand) );

	bool bCommandBlock = false;
	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE )
		bCommandBlock = true;

	switch( iCommand )
	{
	case BATTLEROOM_TEAM_CHANGE:
		{
			int iTeamType = 0;
			PACKET_GUARD_VOID( rkPacket.Read(iTeamType) );

			if( !bCommandBlock && IsBattleTeamChangeOK( (TeamType)iTeamType ) )
			{
				if( IsObserver( pUser->GetPublicID() ) && m_bSafetyLevelRoom && !pUser->IsSafetyLevel() )
				{
					// �������ε� ������ �̵��ҷ��� ������ �¾ƾ��Ѵ�.
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TEAM_CHANGE_NOT_LEVEL) );
					pUser->RelayPacket( kPacket );
				}
				else if( IsObserver( pUser->GetPublicID() ) && !g_LevelMatchMgr.IsPartyLevelJoin( GetAbilityMatchLevel(), pUser->GetKillDeathLevel(), JOIN_CHECK_MIN_LEVEL ) )
				{
					// �������ε� ������ �̵��ҷ��� ������ �¾ƾ��Ѵ�.
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TEAM_CHANGE_NOT_LEVEL) );
					pUser->RelayPacket( kPacket );
				}
				else if( IsObserver( pUser->GetPublicID() ) && IsMapLimitPlayerFull() )
				{
					// �������ε� ������ �� �ִ� �ο����� �¾ƾ��Ѵ�.
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TEAM_CHANGE_MAP_LIMIT) );
					pUser->RelayPacket( kPacket );
				}
				else
				{
					TeamType eCurTeam = ChangeTeamType( pUser->GetPublicID(), (TeamType)iTeamType );

					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TEAM_CHANGE_OK) );
					PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
					PACKET_GUARD_VOID( kPacket.Write((int)eCurTeam) );
					SendPacketTcp( kPacket );

					m_NodeSync.Update( BattleRoomSync::BRS_CHANGEINFO );
				}
			}				
			else
			{
				SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
				PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TEAM_CHANGE_FULL) );
				PACKET_GUARD_VOID( kPacket.Write(iTeamType) );

				pUser->RelayPacket( kPacket );
			}
		}
		break;
	case BATTLEROOM_CANCEL:
		{
			SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_CANCEL_OK) );

			SendPacketTcp( kPacket );				
		}
		break;
	case BATTLEROOM_MODE_SEL:
		{
			CRASH_GUARD();
			if( !bCommandBlock && GetOwnerName() == pUser->GetPublicID() )
			{
				int  iSelectMode = 0, iSelectMap = 0;
				bool bSafetyLevelRoom = false;

				PACKET_GUARD_VOID( rkPacket.Read(iSelectMode) );
				PACKET_GUARD_VOID( rkPacket.Read(iSelectMap) );
				PACKET_GUARD_VOID( rkPacket.Read(bSafetyLevelRoom) );

				if( bSafetyLevelRoom && !IsSafetyModeCreate() )
				{
					// ���� ������ ��� ���� �Ұ���
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_MODE_SEL_FAIL) );
					PACKET_GUARD_VOID( kPacket.Write(m_iSelectMode) );
					PACKET_GUARD_VOID( kPacket.Write(m_iSelectMap) );
					PACKET_GUARD_VOID( kPacket.Write(m_bSafetyLevelRoom) );

					SendPacketTcp( kPacket );
				}
				else
				{
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_MODE_SEL_OK) );
					PACKET_GUARD_VOID( kPacket.Write(iSelectMode) );
					PACKET_GUARD_VOID( kPacket.Write(iSelectMap) );
					PACKET_GUARD_VOID( kPacket.Write(bSafetyLevelRoom) );

					SendPacketTcp( kPacket );
					//AI ��忡�� ������ ������ ���� �ٲ���ߵ�
					ModeType eModeType = GetSelectIndexToMode( iSelectMode, iSelectMap );
					if( eModeType == MT_TEAM_SURVIVAL_AI )
					{						
						int iUserSize = m_vUserNode.size();
						for( int i = 0; i < iUserSize; ++i )
							m_vUserNode[i].m_eTeamType = TEAM_BLUE;
					}
					int  iPrevSelectMode		= m_iSelectMode;
					int  iPrevSelectMap         = m_iSelectMap;
					bool bPrevSafetyLevelRoom	= m_bSafetyLevelRoom;
					m_iSelectMode      = iSelectMode;
					m_iSelectMap       = iSelectMap;
					m_bSafetyLevelRoom = bSafetyLevelRoom;
					if( iPrevSelectMode != m_iSelectMode || bPrevSafetyLevelRoom != m_bSafetyLevelRoom || iPrevSelectMap != m_iSelectMap )
						m_NodeSync.Update( BattleRoomSync::BRS_SELECTMODE );
				}				
			}
		}
		break;
	case BATTLEROOM_START_COUNT:
		{
			SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_START_COUNT_OK) );
			SendPacketTcp( kPacket, pUser->GetUserIndex() );
		}
		break;
	case BATTLEROOM_STOP_COUNT:
		{			
			SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_STOP_COUNT_OK) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
			SendPacketTcp( kPacket, pUser->GetUserIndex() );				
		}
		break;
	case BATTLEROOM_READY_GO:
		{
			OnBattleRoomReadyGO();
		}
		break;
	case BATTLEROOM_TOURNAMENT_START:
		{
			//
			if( GetBattleEventType() == BET_TOURNAMENT_BATTLE )
			{
				// Cheat!!!!
				bool bCheat = false;
				DWORD dwGapTime = TIMEGETTIME() - m_TournamentRoundData.m_dwInviteTimer;
				if( dwGapTime < TournamentRoundData::INVITE_DELAY_TIME - 60000 )
				{
					bCheat = true;
				}

				// ���� ��Ʈ�� �ʿ�? ( ���� ���׸� ���� ��Ʈ�� )
				bool bChangeTeam = false;
				if( m_TournamentRoundData.m_bRegularTour )  
				{
					if( m_TournamentRoundData.m_BlueTourPos == 1 )        // ù �����̸� ���� ��Ʈ�� ��û
					{
						if( !m_TournamentRoundData.m_bChangeEntry )       // �̹� ��û������ ��� ó��
						{
							bChangeTeam = true;
						}
					}
				}

				// ���� Ȯ���Ͽ� �����̳� ��� ����!!
				int iBlueUserCount = GetUserTeamCount( TEAM_BLUE );
				int iRedUserCount  = GetUserTeamCount( TEAM_RED );

				// ������ �ٷ� �����Ѵ�.
				if( iBlueUserCount == iRedUserCount )
				{
					bCheat = false;
				}

				if( bCheat )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - Cheat Tournament Battle Room BATTLEROOM_TOURNAMENT_START", pUser->GetPublicID().c_str() );
					// 
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_TOURNAMENT_START_CHEAT) );
					pUser->RelayPacket( kPacket );
				}
				else if( iBlueUserCount == 0 )
				{
					if( bChangeTeam )
					{
						// ���� ��Ʈ�� ��û
						m_TournamentRoundData.m_bChangeEntry = true;

						SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_TEAM_CHANGE );
						PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
						PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwBlueIndex) );
						PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
						g_MainServer.SendMessage( kPacket );
						
						// ���� ��Ʈ�� �˸�
						SP2Packet kPacket2( STPK_BATTLEROOM_COMMAND );
						PACKET_GUARD_VOID( kPacket2.Write(BATTLEROOM_TOURNAMENT_DELETE_TEAM) );
						PACKET_GUARD_VOID( kPacket2.Write(m_TournamentRoundData.m_dwBlueIndex) );
						PACKET_GUARD_VOID( kPacket2.Write(TournamentRoundData::INVITE_DELAY_TIME) );
						SendPacketTcp( kPacket2 );

						// �� �ʱ�ȭ
						m_TournamentRoundData.m_dwBlueIndex = 0;
						m_TournamentRoundData.m_BlueUserIndex.clear();

						// �ð� �ʱ�ȭ
						m_TournamentRoundData.m_dwInviteTimer = TIMEGETTIME();						

						if( IsRegularTournamentBattle() )
						{							
							LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s - %s - ����� ����Ʈ�� ��û : ��Ʋ�� : %d, �������� ���� �� : %d", __FUNCTION__, 
								pUser->GetPublicID().c_str(), GetIndex(), m_TournamentRoundData.m_dwBlueIndex );
						}
					}
					else if( m_TournamentRoundData.m_bSendResult == false )
					{
						//������ ������
						TournamentUnearnedWinCommand( TEAM_RED, pUser->GetPublicID() );
					}
					else
					{
						LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s - %s - ���� ���� : ��Ʋ�� : %d, �������� ���� ��� �� : %d", __FUNCTION__,
							pUser->GetPublicID().c_str(), GetIndex(), m_TournamentRoundData.m_dwBlueIndex );
					}
				}
				else if( iRedUserCount == 0 )
				{
					if( bChangeTeam )
					{
						// ���� ��Ʈ�� ��û
						m_TournamentRoundData.m_bChangeEntry = true;

						SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_TEAM_CHANGE );
						PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
						PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwRedIndex) );
						PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
						g_MainServer.SendMessage( kPacket );

						// ���� ��Ʈ�� �˸�
						SP2Packet kPacket2( STPK_BATTLEROOM_COMMAND );
						PACKET_GUARD_VOID( kPacket2.Write(BATTLEROOM_TOURNAMENT_DELETE_TEAM) );
						PACKET_GUARD_VOID( kPacket2.Write(m_TournamentRoundData.m_dwRedIndex) );
						PACKET_GUARD_VOID( kPacket2.Write(TournamentRoundData::INVITE_DELAY_TIME) );
						SendPacketTcp( kPacket2 );

						m_TournamentRoundData.m_dwRedIndex = 0;
						m_TournamentRoundData.m_RedUserIndex.clear();

						// �ð� �ʱ�ȭ
						m_TournamentRoundData.m_dwInviteTimer = TIMEGETTIME();

						if( IsRegularTournamentBattle() )
						{
							LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ������ ����Ʈ�� ��û : ��Ʋ�� : %d, �������� ���� �� : %d", __FUNCTION__, GetIndex(), m_TournamentRoundData.m_dwBlueIndex );
						}
					}
					else if( m_TournamentRoundData.m_bSendResult == false )
					{
						//����� ������
						TournamentUnearnedWinCommand( TEAM_BLUE, pUser->GetPublicID() );
					}
					else
					{
						LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s - %s - ���� ���� : ��Ʋ�� : %d, �������� ���� ���� �� : %d", __FUNCTION__, pUser->GetPublicID().c_str(), GetIndex(), m_TournamentRoundData.m_dwRedIndex );
					}
				}
				else 
				{
					// ���� ����
					SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
					PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_START_COUNT_OK) );
					SendPacketTcp( kPacket );

					if( IsRegularTournamentBattle() )
					{
						LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ī��Ʈ ���� : ��Ʋ�� : %d", __FUNCTION__, GetIndex() );
					}
				}				
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - BattleRoom Tournament Start %d", pUser->GetPublicID().c_str(), GetBattleEventType() );
		}
		break;
	}
}

void BattleRoomNode::OnBattleRoomReadyGO()
{
	// ��� ������ ��ȸ��� ������ üũ
	int iBlueUserCount = GetUserTeamCount( TEAM_BLUE );
	int iRedUserCount  = GetUserTeamCount( TEAM_RED );

	if( IsRegularTournamentBattle() )
	{
		if( iBlueUserCount == 0 && 0 < iRedUserCount )
		{
			//������ ������
			TournamentUnearnedWinCommand( TEAM_RED, "����� ���¿����� ������" );
			
			return;
		}
		else if( iRedUserCount == 0 && 0 < iBlueUserCount )
		{
			//����� ������
			TournamentUnearnedWinCommand( TEAM_BLUE, "����� ���¿����� ������" );
			return;
		}
	}

	ioHashString szTitle;
	if( m_pBattleRoom && !m_pBattleRoom->IsRoomEmpty() && m_pBattleRoom->GetRoomStyle() == RSTYLE_BATTLEROOM )
	{
		// �뿡 �ִ� ���
		int iMapNum = -1;
		int iSubType = -1;
		ModeType eModeType = MT_NONE;
		int iModeIndex = GetSelectMode();
		int iMapIndex  = GetSelectMap();

		bool bPrevSafetyRoom    = m_pBattleRoom->IsSafetyLevelRoom();
		bool bPrevBroadcastRoom = m_pBattleRoom->IsBroadcastRoom();
        m_pBattleRoom->SetSafetyRoom( m_bSafetyLevelRoom );
		m_pBattleRoom->SetBroadcastRoom( (GetBattleEventType() == BET_BROADCAST_MBC) );
		m_pBattleRoom->SetTournamentRoom( (GetBattleEventType() == BET_TOURNAMENT_BATTLE), m_TournamentRoundData.m_dwTourIndex );
		m_pBattleRoom->SetRoomStyle( RSTYLE_BATTLEROOM );
		
		if( bPrevSafetyRoom != m_pBattleRoom->IsSafetyLevelRoom() ||
			bPrevBroadcastRoom != m_pBattleRoom->IsBroadcastRoom() )
		{
			m_pBattleRoom->InitModeTypeList();
		}

		if( iModeIndex == -1 )	
		{
			eModeType = m_pBattleRoom->SelectNextMode( MT_NONE );
		}
		else					//��Ÿ
		{
			bool bSuccess = g_BattleRoomManager.CheckBattleRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );
			if( bSuccess )
			{
				eModeType = m_pBattleRoom->SelectNextMode( eModeType, iSubType );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::OnBattleRoomCommand() - SelectIndex is not Exist(%d): %d", GetIndex(), iModeIndex );
				eModeType = m_pBattleRoom->SelectNextMode( MT_NONE );
			}
		}

		// �ɼ� ����
		if( m_bRandomTeamMode )
		{
			m_pBattleRoom->NextShamBattleRandomTeam( eModeType );
		}

		m_pBattleRoom->SetModeType( eModeType, GetCatchModeRoundType(), GetCatchModeRoundTimeType() );
		{
			SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket.Write(BATTLEROOM_READY_GO_OK) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(m_pBattleRoom->GetModeMapNum()) );
			SendPacketTcp( kPacket );

			if( IsRegularTournamentBattle() )
			{
				LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ���� ��� ���� : ��Ʋ�� : %d", __FUNCTION__, GetIndex() );
			}
		}

		if( eModeType == MT_BOSS )
			CreateBoss();
		else if( eModeType == MT_GANGSI )
			CreateGangsi();
		else if ( eModeType == MT_TEAM_SURVIVAL_AI )
			SetAIMode( m_pBattleRoom );

		//m_pBattleRoom->CheckUseFightNPC();
		m_pBattleRoom->CreateNextShamBattle();

		eModeType = m_pBattleRoom->GetModeType();
		iSubType = m_pBattleRoom->GetModeSubNum();
		iMapNum = m_pBattleRoom->GetModeMapNum();

		if( m_pBattleRoom && CheckUseFightNPC() && eModeType == MT_FIGHT_CLUB )
		{
			m_pBattleRoom->CheckUseFightNPC();
		}

		SetPreSelectModeInfo( eModeType, iSubType, iMapNum );

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::OnBattleRoomCommand() Already ReadyGO(%d) : %d", GetIndex(), m_pBattleRoom->GetRoomIndex() );
		m_vInviteUser.clear();
	}
	else
	{
		Room *pRoom = g_RoomNodeManager.CreateNewRoom();
		if( pRoom )
		{	
			int iMapNum = -1;
			int iSubType = -1;
			ModeType eModeType = MT_NONE;
			int iModeIndex = GetSelectMode();
			int iMapIndex  = GetSelectMap();

            pRoom->SetSafetyRoom( m_bSafetyLevelRoom );
			pRoom->SetBroadcastRoom( (GetBattleEventType() == BET_BROADCAST_MBC) );
			pRoom->SetTournamentRoom( (GetBattleEventType() == BET_TOURNAMENT_BATTLE), m_TournamentRoundData.m_dwTourIndex );
			pRoom->SetRoomStyle( RSTYLE_BATTLEROOM );
			pRoom->InitModeTypeList();

			//�ռ� �÷��� ���� �ݿ�
			int iPreMapNum, iPreSubType;
			ModeType ePreModeType;

			GetPreSelectModeInfo( ePreModeType, iPreSubType, iPreMapNum );
			pRoom->SetPreSelectModeInfo( ePreModeType, iPreSubType, iPreMapNum );
			//

			if( iModeIndex == -1 )	//��ŷ
			{
				eModeType = pRoom->SelectNextMode( MT_NONE );
			}
			else					//��Ÿ
			{
				bool bSuccess = g_BattleRoomManager.CheckBattleRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );
				if( bSuccess )
				{
					eModeType = pRoom->SelectNextMode( eModeType, iSubType );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::OnBattleRoomCommand() - SelectIndex is not Exist(%d): %d", GetIndex(), iModeIndex );
					eModeType = pRoom->SelectNextMode( MT_NONE );
				}
			}

			pRoom->SetModeType( eModeType, GetCatchModeRoundType(), GetCatchModeRoundTimeType() );
			if( eModeType != MT_SURVIVAL && eModeType != MT_BOSS && eModeType != MT_GANGSI && eModeType != MT_MONSTER_SURVIVAL && 
				eModeType != MT_DUNGEON_A && eModeType != MT_FIGHT_CLUB && eModeType != MT_RAID && 
				!Help::IsMonsterDungeonMode(eModeType) )
				BattleEnterRandomTeam();
			if( eModeType == MT_BOSS )
				CreateBoss();
			else if( eModeType == MT_GANGSI )
				CreateGangsi();
			else if ( eModeType == MT_TEAM_SURVIVAL_AI )
 				SetAIMode( pRoom );

			BattleEnterRoom( pRoom );

			eModeType = pRoom->GetModeType();
			iSubType = pRoom->GetModeSubNum();
			iMapNum = pRoom->GetModeMapNum();

			if( m_pBattleRoom && CheckUseFightNPC() && eModeType == MT_FIGHT_CLUB )
			{
				m_pBattleRoom->CheckUseFightNPC();
			}

			SetPreSelectModeInfo( eModeType, iSubType, iMapNum );

			if( IsRegularTournamentBattle() )
			{
				LOG.PrintTimeAndLog( 0, "[��ȸ�α�] %s ���� ��� ���� : ��Ʋ�� : %d", __FUNCTION__, GetIndex() );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::OnBattleRoomCommand() New ReadyGO(%d) : %d", GetIndex(), m_pBattleRoom->GetRoomIndex() );
			}
		}
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"BATTLEROOM_READY_GO : �ܿ� ���� ����(%d)", GetIndex() );
	}
	m_NodeSync.Update( BattleRoomSync::BRS_PLAY );                  // �÷��� ����
}

bool BattleRoomNode::CheckUseFightNPC()
{
	int iPlayUserCnt = 0;

	vBattleRoomUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomUser &kUser = *iter;

		if( kUser.m_bObserver )
			continue;

		iPlayUserCnt++;
	}

	if( iPlayUserCnt == 1 )
		return true;

	return false;
}


bool BattleRoomNode::IsRegularTournamentBattle()
{
	if( GetBattleEventType() == BET_TOURNAMENT_BATTLE && m_TournamentRoundData.m_dwTourIndex == TOURNAMENT_REGULAR_INDEX )
		return true;
	
	return false;
}

void BattleRoomNode::TournamentUnearnedWinCommand( TeamType eUnearnedWinTeam,  const ioHashString& szUserName )
{
	switch( eUnearnedWinTeam )
	{
	case TEAM_RED:
		{
			// ������ ������
			SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwRedIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwBlueIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_RedTourPos + 1) );
			g_MainServer.SendMessage( kPacket );

			// ������ �˸�
			SP2Packet kPacket2( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket2.Write(BATTLEROOM_TOURNAMENT_DRAW_BYE) );
			SendPacketTcp( kPacket2 );

			m_TournamentRoundData.m_bSendResult = true;

			if( IsRegularTournamentBattle() )
			{
				LOG.PrintTimeAndLog( 0, "[��ȸ�α�]  %s - %s ������ ������ : ��Ʋ�� : %d", __FUNCTION__, szUserName.c_str(), GetIndex() );
			}
		}
		break;
	case TEAM_BLUE:
		{
			// ����� ������
			SP2Packet kPacket( MSTPK_TOURNAMENT_BATTLE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwTourIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwBlueIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_dwRedIndex) );
			PACKET_GUARD_VOID( kPacket.Write(m_TournamentRoundData.m_BlueTourPos + 1) );
			g_MainServer.SendMessage( kPacket );

			// ������ �˸�
			SP2Packet kPacket2( STPK_BATTLEROOM_COMMAND );
			PACKET_GUARD_VOID( kPacket2.Write(BATTLEROOM_TOURNAMENT_DRAW_BYE) );
			SendPacketTcp( kPacket2 );

			m_TournamentRoundData.m_bSendResult = true;

			if( IsRegularTournamentBattle() )
			{
				LOG.PrintTimeAndLog( 0, "[��ȸ�α�]  %s - %s ����� ������ : ��Ʋ�� : %d", __FUNCTION__, szUserName.c_str(), GetIndex() );
			}
		}
		break;
	default:
		{
			LOG.PrintTimeAndLog( 0, "%s - not unknown team type", __FUNCTION__ );
		}
	}
}

void BattleRoomNode::SetAIMode( Room* pRoom )
{
	if( !pRoom )
		return;

	TeamSurvivalAIMode* pMode = ToTeamSurvivalAIMode( pRoom->GetModeInfo() );
	if( pMode )
		pMode->SetAILevel( m_iAILevel );
}
