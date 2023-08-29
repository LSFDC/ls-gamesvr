#include "stdafx.h"

#include "../EtcHelpFunc.h"
//#include "../Window.h"
#include "../MainProcess.h"

#include "LadderTeamManager.h"
#include "UserNodeManager.h"
#include "ServerNodeManager.h"
#include "RoomNodeManager.h"
#include "LevelMatchManager.h"
#include "LadderTeamNode.h"
#include "ioMyLevelMgr.h"
#include "../Local/ioLocalParent.h"

//////////////////////////////////////////////////////////////////////////
LadderTeamSync::LadderTeamSync()
{
	m_pCreator     = NULL;
	m_dwUpdateTime = 0;
	m_dwUpdateType = 0;
	// ������Ʈ �ð�
	m_dwCheckTime[LTS_CHANGEINFO]	= 4000;
	m_dwCheckTime[LTS_CHANGERECORD] = 3000;
	m_dwCheckTime[LTS_CREATE]		= 500;
	m_dwCheckTime[LTS_DESTROY]		= 0;
}

LadderTeamSync::~LadderTeamSync()
{

}

void LadderTeamSync::Init()
{
	m_dwUpdateTime = 0;
	m_dwUpdateType = 0;
}

void LadderTeamSync::SetCreator( LadderTeamNode *pCreator )
{
	Init();
	m_pCreator = pCreator;
}

void LadderTeamSync::Update( DWORD dwUpdateType )
{
	if( !COMPARE( dwUpdateType, LTS_CHANGEINFO, MAX_LTS ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamSync::Update �˼� ���� ������Ʈ �� : %d", dwUpdateType );
		return;
	}
	if( !m_pCreator )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamSync::Update m_pCreator == NULL" );
		return;
	}

	if( dwUpdateType < m_dwUpdateType )
		return;

	m_dwUpdateType = dwUpdateType;
	m_dwUpdateTime = TIMEGETTIME();
	Process();
}

void LadderTeamSync::Process()
{
	if( m_dwUpdateTime == 0 ) return;
	if( !COMPARE( m_dwUpdateType, LTS_CHANGEINFO, MAX_LTS ) ) return;

	DWORD dwGap = TIMEGETTIME() - m_dwUpdateTime;
	if( dwGap >= m_dwCheckTime[m_dwUpdateType] )
	{
		switch( m_dwUpdateType )
		{
		case LTS_CHANGEINFO:
			m_pCreator->SyncChangeInfo();
			break;
		case LTS_CHANGERECORD:
			m_pCreator->SyncChangeRecord();
			break;
		case LTS_CREATE:
			m_pCreator->SyncCreate();
			break;
		case LTS_DESTROY:
			m_pCreator->SyncDestroy();
			break;
		}
		m_dwUpdateType = m_dwUpdateTime = 0;
	}
}
//////////////////////////////////////////////////////////////////////////
LadderTeamNode::LadderTeamNode( DWORD dwIndex ) : m_dwIndex( dwIndex )
{
	InitData();
}

LadderTeamNode::~LadderTeamNode()
{
}

void LadderTeamNode::SyncChangeInfo()
{
	SP2Packet kPacket( SSTPK_LADDERTEAM_SYNC );
	kPacket << LadderTeamSync::LTS_CHANGEINFO << GetIndex() << GetTeamName() << GetPW() << GetJoinUserCnt() << GetMaxPlayer() << GetAbilityMatchLevel() << GetHeroMatchPoint();
	kPacket << GetJoinGuildIndex() << IsSearchLevelMatch() << IsSearchSameUser() << IsBadPingKick() << GetTeamState() << GetGuildIndex() << GetGuildMark() << GetSelectMode() << GetSelectMap();
	g_ServerNodeManager.SendMessageAllNode( kPacket );
}

void LadderTeamNode::SyncChangeRecord()
{
	SP2Packet kPacket( SSTPK_LADDERTEAM_SYNC );
	kPacket << LadderTeamSync::LTS_CHANGERECORD << GetIndex() << GetTeamName() << GetPW() << GetJoinUserCnt() << GetMaxPlayer() << GetAbilityMatchLevel() << GetHeroMatchPoint();
	kPacket << GetJoinGuildIndex() << IsSearchLevelMatch() << IsSearchSameUser() << IsBadPingKick() << GetTeamState() << GetGuildIndex() << GetGuildMark() << GetSelectMode() << GetSelectMap();
	kPacket << GetWinRecord() << GetLoseRecord() << GetVictoriesRecord();
	g_ServerNodeManager.SendMessageAllNode( kPacket );
}

void LadderTeamNode::SyncCreate()
{
	SP2Packet kPacket( SSTPK_LADDERTEAM_SYNC );
	kPacket << LadderTeamSync::LTS_CREATE;
	FillSyncCreate( kPacket );
	g_ServerNodeManager.SendMessageAllNode( kPacket );
}

void LadderTeamNode::SyncDestroy()
{
	SP2Packet kPacket( SSTPK_LADDERTEAM_SYNC );
	kPacket << LadderTeamSync::LTS_DESTROY << GetIndex();
	g_ServerNodeManager.SendMessageAllNode( kPacket );
}

void LadderTeamNode::SyncRealTimeCreate()
{
	SyncCreate();
	m_NodeSync.Init();
}

void LadderTeamNode::InitData()
{
	m_pMatchRoom = NULL;
	m_iCampType  = 0;
	m_vUserNode.clear();
	m_szTeamName.Clear();
	m_szTeamPW.Clear();
	m_OwnerUserID.Clear();
	m_dwGuildIndex= 0;
	m_dwGuildMark = 0;
	m_iMaxPlayer     = MAX_LADDERTEAM_PLAYER; 
	m_iSaveMaxPlayer = MAX_LADDERTEAM_PLAYER;
	m_iSelectMode = -1;
	m_iSelectMap  = -1;
	m_iPreSubType = 0;
	m_iPreMapNum  = 0;
	m_PreModeType = MT_NONE;
	m_NodeSync.SetCreator( this );
	m_dwReserveTime    = 0;
	m_dwLastMatchTeam  = 0;
	m_dwCurLastBattleTime = 0;
	m_dwJoinGuildIndex = 0;
	m_bSearchLevelMatch = false;
	m_bSearchSameUser  = false;
	m_bHeroMatchMode   = false;
	m_bBadPingKick	   = true;

	m_dwUserIndex = 0;
	m_dwCompetiorIndex = 0;

	if( ioLocalManager::GetSingletonPtr() )
	{
		ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
		if( pLocal && !pLocal->IsBadPingKick( true ) )
			m_bBadPingKick = false;

		// �±��϶��� ������ �������϶� �⺻���� BadPingKick�ɼ� ����
		if(ioLocalManager::GetLocalType() == ioLocalManager::LCT_THAILAND)
		{
			m_bBadPingKick = true;
		}

	}
	m_iTeamRanking     = -1;
	m_dwTeamState      = TMS_READY;
	m_dwSearchMatchCurTime  = 0;
	m_iSearchMatchCurSec    = 0;
	m_dwMatchReserveIndex   = 0;
	m_iSearchCount			= 0;
	m_MatchReserve.clear();
	InitRecord();
}

void LadderTeamNode::InitRecord()
{
	m_iWinRecord  = 0;
	m_iLoseRecord = 0;
	m_iVictoriesRecord = 0;
}

void LadderTeamNode::OnCreate()
{
	InitData();
	m_NodeSync.Update( LadderTeamSync::LTS_CREATE );
	m_dwReserveTime = TIMEGETTIME();
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::OnCreate (%d)", GetIndex() );
}

void LadderTeamNode::OnDestroy()
{
	InitData();
	m_NodeSync.Update( LadderTeamSync::LTS_DESTROY );
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::OnDestroy (%d)", GetIndex() );
}

void LadderTeamNode::EnterUser( const DWORD dwUserIndex, const ioHashString &szPublicID, const int iGradeLevel, const int iAbilityLevel, const int iHeroMatchPoint, const int iLadderPoint, const DWORD dwGuildIndex, const DWORD dwGuildMark,
				  		        const ioHashString &szPublicIP, const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iClientPort, const int iTransferPort  )
{
	// ���� ó�� : ������ ������ �ι� ���� ���ɼ��� �����Ƿ�
	// ������ �� �̹� ����Ǿ��ִ� ���̵�� �����Ѵ�.
	if( RemoveUser( dwUserIndex ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ERROR!!! : LadderTeamNode::EnterUser(%d) : %d - %s - %s:%d", 
								GetIndex(), dwUserIndex, szPublicID.c_str(), szTransferIP.c_str(), iTransferPort );
	}

	m_dwReserveTime = 0;

	if( m_OwnerUserID.IsEmpty() )
	{
		m_OwnerUserID = szPublicID;
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::EnterUser(%d) : Owner : %s", GetIndex(), szPublicID.c_str() );
	}

	LadderTeamUser kEnterUser;
	kEnterUser.m_dwUserIndex	= dwUserIndex;
	kEnterUser.m_szPublicID		= szPublicID;
	kEnterUser.m_iGradeLevel	= iGradeLevel;
	kEnterUser.m_iAbilityLevel	= iAbilityLevel;	
	kEnterUser.m_iHeroMatchPoint= iHeroMatchPoint;
	kEnterUser.m_iLadderPoint   = iLadderPoint;
	kEnterUser.m_dwGuildIndex   = dwGuildIndex;
	kEnterUser.m_dwGuildMark    = dwGuildMark;
	kEnterUser.m_szPublicIP		= szPublicIP;
	kEnterUser.m_szPrivateIP	= szPrivateIP;
	kEnterUser.m_szTransferIP   = szTransferIP;
	kEnterUser.m_iClientPort	= iClientPort;
	kEnterUser.m_iTransferPort  = iTransferPort;
    
	// ������ �ΰ�� 
	if( IsHeroMatchMode() )
	{
		SetCompetitorIndex( 0 );	//������ �Բ� ������ �ߴ� ������ �ε����� �ʱ�ȭ�Ѵ�.
	}

	AddUser( kEnterUser );

	UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUser == NULL ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::EnterUser ���� ���� ���� ����(%d) : %s(%d)", GetIndex(), szPublicID.c_str(), dwUserIndex );
		return;
	}

	//���� �������� �� ���� ����.
	SP2Packet kPacket( STPK_LADDERTEAM_INFO );
	FillLadderTeamInfo( kPacket );
	pUser->RelayPacket( kPacket );
	
	//���� �������� ��� ��Ƽ�� ������(�ڽ� ����) ����.(���� ���� ������ �� ��Ƽ ������ ���� ������.)
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		LadderTeamUser &kUser = *iter;
		SP2Packet kPacket1( STPK_LADDERTEAM_USER_INFO );
		{
			FillUserInfo( kPacket1, kUser );
		}
		pUser->RelayPacket( kPacket1 );
	}

	//��� ��Ƽ������ ���� ���� ������(�ڽ� ����) ����.
	SP2Packet kPacket2( STPK_LADDERTEAM_USER_INFO );
	FillUserInfo( kPacket2, kEnterUser );
	SendPacketTcp( kPacket2, kEnterUser.m_dwUserIndex );

	// ����� ������ �����Ѵ�
	{		
		SP2Packet kBattlePacket( STPK_LADDERTEAM_MACRO );
		kBattlePacket << LADDERTEAM_MACRO_BATTLE_INFO;
		kBattlePacket << GetSelectMode() << GetSelectMap() << GetJoinGuildIndex() << IsSearchLevelMatch() << IsSearchSameUser() << IsBadPingKick();
		pUser->RelayPacket( kBattlePacket );
	}
	
	CheckGuildTeam();
	m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );

	if( IsHeroMatchMode() )
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "HeroMode Enter : %s - %d", kEnterUser.m_szPublicID.c_str(), GetAbilityMatchLevel() );
}

void LadderTeamNode::UserInfoUpdate( const DWORD dwUserIndex, const int iGradeLevel, const int iAbilityLevel, const int iHeroMatchPoint, const int iLadderPoint, const DWORD dwGuildIndex, const DWORD dwGuildMark, const int iClientPort, const ioHashString &szTransferIP, const int iTransferPort  )
{
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			if( kUser.m_iGradeLevel != iGradeLevel ||
				kUser.m_iAbilityLevel != iAbilityLevel ||
				kUser.m_iHeroMatchPoint != iHeroMatchPoint )
			{
				m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
			}
			kUser.m_iGradeLevel   = iGradeLevel;
			kUser.m_iAbilityLevel = iAbilityLevel;
			kUser.m_iHeroMatchPoint = iHeroMatchPoint;
			kUser.m_iLadderPoint  = iLadderPoint;
			kUser.m_szTransferIP  = szTransferIP;
			kUser.m_iClientPort   = iClientPort;
			kUser.m_iTransferPort = iTransferPort;
			kUser.m_dwGuildIndex  = dwGuildIndex;
			kUser.m_dwGuildMark   = dwGuildMark;		
			CheckGuildTeam();

			if( IsHeroMatchMode() )
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "HeroMode InfoUpdate : %s - %d", kUser.m_szPublicID.c_str(), GetAbilityMatchLevel() );			
			return;
		}
	}
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::UserInfoUpdate(%d) %d ���� ����", GetIndex(), dwUserIndex );
}

void LadderTeamNode::UserUDPChange( const DWORD dwUserIndex, const ioHashString &szPublicID, const ioHashString &szPublicIP, const int iClientPort, 
								   const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iTransferPort  )
{
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			kUser.m_szTransferIP  = szTransferIP;
			kUser.m_iTransferPort = iTransferPort;
			kUser.m_iClientPort   = iClientPort;
			
			SP2Packet kPacket( STPK_CHANGE_UDP_INFO );
			kPacket << szPublicID << szPublicIP << iClientPort << szPrivateIP << szTransferIP << iTransferPort;
			SendPacketTcp( kPacket, dwUserIndex );
			return;
		}
	}
}

bool LadderTeamNode::LeaveUser( const DWORD dwUserIndex, const ioHashString &szPublicID  )
{
	if( !RemoveUser( dwUserIndex ) ) return false;

	//������ ������ �����ϸ� ��Ƽ ����.
	if( m_vUserNode.empty() )
	{
		g_LadderTeamManager.RemoveLadderTeam( this );			
	}
	else
	{
		// ������ ������ ���� ��ü
		CRASH_GUARD();
		if( szPublicID == m_OwnerUserID  )
			SelectNewOwner();

		// ������ ������ ���� ���̵� ����
		SP2Packet kPacket( STPK_LADDERTEAM_LEAVE );

		PACKET_GUARD_bool( kPacket.Write(szPublicID) );
		PACKET_GUARD_bool( kPacket.Write(m_OwnerUserID) );

		SendPacketTcp( kPacket );

		// �˻� ���
		SetTeamState( TMS_READY );
	}

	CheckGuildTeam();
	m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
	return true;
}

void LadderTeamNode::AddUser(const LadderTeamUser &kUser )
{
	m_vUserNode.push_back( kUser );
}

bool LadderTeamNode::RemoveUser( const DWORD dwUserIndex )
{
    vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex )
		{
			m_vUserNode.erase( iter );			
			return true;
		}
	}
	return false;
}

void LadderTeamNode::SelectNewOwner()
{
	m_OwnerUserID.Clear();
	if( m_vUserNode.empty() ) return;

	LadderTeamUser &kNewOwner = m_vUserNode[0];
	m_OwnerUserID = kNewOwner.m_szPublicID;	
}

ModeType LadderTeamNode::GetSelectIndexToMode( int iModeIndex, int iMapIndex )
{
	ModeType eModeType = MT_NONE;
	int      iSubType  = 0;

	if( iModeIndex != -1 )
		g_LadderTeamManager.CheckLadderTeamRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );	
	return eModeType;
}

int LadderTeamNode::GetJoinUserCnt() const
{
	return m_vUserNode.size();
}

int LadderTeamNode::GetMaxPlayer() const
{
	return m_iMaxPlayer;
}

int LadderTeamNode::GetSelectMode() const
{
	return m_iSelectMode;
}

int LadderTeamNode::GetSelectMap() const
{
	return m_iSelectMap;
}

int LadderTeamNode::GetRankingPoint()
{
	int iRankingPoint = 0;

	int iWinRecord = GetWinRecord()  + 1;
	int iLoseRecord= GetLoseRecord() + 1;
	int iWinPoint = ( iWinRecord * g_LadderTeamManager.GetLadderBattleWinPoint() ) * 1000;
	int iLosePoint= ( iLoseRecord* g_LadderTeamManager.GetLadderBattleLosePoint() ) * 1000;
	iRankingPoint = iRankingPoint - (float)( ( iWinPoint + iLosePoint ) * iWinRecord ) / ( iWinRecord + iLoseRecord );

	return iRankingPoint;
}

void LadderTeamNode::SetCampType( int iCampType )
{
	m_iCampType = iCampType;
}

void LadderTeamNode::SetTeamName( const ioHashString &rkName )
{
	m_szTeamName = rkName;
}

void LadderTeamNode::SetTeamPW( const ioHashString &rkPW )
{
	m_szTeamPW = rkPW; 
}

void LadderTeamNode::SetMaxPlayer( int iMaxPlayer )
{
	if( m_iMaxPlayer != iMaxPlayer )
		m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
	
	m_iMaxPlayer = max( GetJoinUserCnt(), iMaxPlayer );
}

void LadderTeamNode::SetSaveMaxPlayer( int iMaxPlayer )
{
	m_iSaveMaxPlayer = max( GetJoinUserCnt(), iMaxPlayer );
}

void LadderTeamNode::SetJoinGuildIndex( DWORD dwJoinGuildIndex )
{
	m_dwJoinGuildIndex = dwJoinGuildIndex;
	m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
}

void LadderTeamNode::SelectMode( const int iModeIndex )
{
	m_iSelectMode = iModeIndex;
}

void LadderTeamNode::SelectMap( const int iMapIndex )
{
	m_iSelectMap = iMapIndex;
}

void LadderTeamNode::SetHeroMatchMode( bool bHeroMatchMode )
{
	m_bHeroMatchMode = bHeroMatchMode;
	m_bSearchLevelMatch = true;
}

void LadderTeamNode::SetTeamState( DWORD dwState )
{
	DWORD dwPrevState = m_dwTeamState;
	switch( dwState )
	{
	case TMS_READY:
		{
			if( dwPrevState == dwState ) return;
			if( dwPrevState == TMS_MATCH_RESERVE || dwPrevState == TMS_MATCH_PLAY )
				return;      

			DWORD dwGapTime = TIMEGETTIME() - m_dwSearchMatchCurTime;
			if( dwGapTime <= g_LadderTeamManager.GetSearchMatchFullTime() )
			{
				// �˻� ���
				SetMaxPlayer( m_iSaveMaxPlayer );
				SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
				kPacket << LADDERTEAM_MACRO_TEAM_STATE << dwState << GetMaxPlayer();
				SendPacketTcp( kPacket );

				g_LadderTeamManager.RemoveSearchingList(GetIndex(), IsHeroMatchMode());
			}
			else
			{
				// �˻� ����
				SetMaxPlayer( m_iSaveMaxPlayer );
				SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
				kPacket << LADDERTEAM_MACRO_SEARCH_FAILED << GetMaxPlayer();
				SendPacketTcp( kPacket );

				g_LadderTeamManager.RemoveSearchingList(GetIndex(), IsHeroMatchMode());
			}

			//Search Init
			m_dwSearchMatchCurTime  = 0;
			m_iSearchMatchCurSec    = 0;
			m_dwMatchReserveIndex   = 0;
		}
		break;
	case TMS_SEARCH_RESERVE:
		{
			if( dwPrevState == dwState ) return;
			if( dwPrevState == TMS_MATCH_RESERVE || dwPrevState == TMS_MATCH_PLAY )
				return;
			
			// �ִ� �ο��� �����ο��� �°� �ٲ۴�.
			SetSaveMaxPlayer( GetMaxPlayer() );
			SetMaxPlayer( GetJoinUserCnt() ); 
			
			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			kPacket << LADDERTEAM_MACRO_TEAM_STATE << dwState << GetMaxPlayer() << g_LadderTeamManager.GetSearchMatchFullTime();
			SendPacketTcp( kPacket );

			//Search Start
			m_dwSearchMatchCurTime  = TIMEGETTIME();
			m_iSearchMatchCurSec    = 0;
			m_dwMatchReserveIndex   = 0;

			g_LadderTeamManager.RemoveSearchingList(GetIndex(), IsHeroMatchMode());
		}
		break;
	case TMS_SEARCH_PROCEED:
		{
			if( dwPrevState == TMS_MATCH_RESERVE || dwPrevState == TMS_MATCH_PLAY )
				return;      

			if( dwPrevState != TMS_SEARCH_PROCEED )
			{
				g_LadderTeamManager.AddSearchingList(GetIndex(), IsHeroMatchMode());
			}

			if( g_LadderTeamManager.SearchBattleMatch( this ) ) // ��Ī�� �Ǹ� ���ο��� SetTeamState�� ȣ���Ѵ�.
			{
				return;
			}
		}		
		break;
	case TMS_MATCH_RESERVE:
		break;
	case TMS_MATCH_PLAY:
		{
			if( dwPrevState == dwState ) return;

			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			kPacket << LADDERTEAM_MACRO_TEAM_STATE << dwState << GetMaxPlayer();
			SendPacketTcp( kPacket );

			//Search Init
			m_dwSearchMatchCurTime  = 0;
			m_iSearchMatchCurSec    = 0;
			m_dwMatchReserveIndex   = 0;
		}
		break;
	}

	m_dwTeamState = dwState;
	if( dwPrevState != m_dwTeamState )
	{
		m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
	}
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "(%d) Team State Change : %d < - > %d", GetIndex(), dwPrevState, dwState );
}

void LadderTeamNode::MatchReserveCancel()
{
	if( GetTeamState() == TMS_MATCH_RESERVE )
	{
		// ��... ������Ʈ�� �ٲ�����ϴµ�;;;
		if( m_dwSearchMatchCurTime != 0 )
		{
			// �˻��� �����Ѵ�.
			m_dwTeamState = TMS_SEARCH_PROCEED;
		}
		else
		{
			// �˻� ����
			m_dwTeamState = TMS_READY;
			SetMaxPlayer( m_iSaveMaxPlayer );
			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			kPacket << LADDERTEAM_MACRO_SEARCH_FAILED << GetMaxPlayer();
			SendPacketTcp( kPacket );

			//Search Init
			m_dwSearchMatchCurTime  = 0;
			m_iSearchMatchCurSec    = 0;
			m_dwMatchReserveIndex   = 0;
		}

		m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );	
		m_pMatchRoom = NULL;
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "[%s] LadderTeamNode::MatchReserveCancel(%d)", GetTeamName().c_str(), GetIndex() );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "[%s] LadderTeamNode::MatchReserveCancel(%d) - ERROR[%d]", GetTeamName().c_str(), GetIndex(), GetTeamState() );
	}
}

void LadderTeamNode::MatchPlayEndSync()
{
	m_dwTeamState = TMS_READY;
	SetMaxPlayer( m_iSaveMaxPlayer );
	SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
	kPacket << LADDERTEAM_MACRO_TEAM_STATE << GetTeamState() << GetMaxPlayer();
	SendPacketTcp( kPacket );

	CheckGuildTeam( true );
	m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );	
	m_pMatchRoom = NULL;
	m_dwCurLastBattleTime = TIMEGETTIME();
	//LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "[%s] LadderTeamNode::MatchPlayEndSync(%d)", GetTeamName().c_str(), GetIndex() );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::MatchPlayEndSync(Name:%s,Index:%d)", GetTeamName().c_str(), GetIndex() );
	SetCompetitorIndex(0);		//������ �������� ���� �ε��� �ʱ�ȭ

}

/************************************************************************/
/* # ���������� ������ �������� ���� �����Ѵ�						    */
/************************************************************************/
void LadderTeamNode::MatchRoomRequest( DWORD dwRequestIndex )
{
	InitSearchCount();
	if( GetTeamState() != TMS_SEARCH_PROCEED )
	{
		bool bCancelMatch = true;
		if( GetTeamState() == TMS_MATCH_RESERVE ) //�̹� ��ġ ���� �����̴�
		{
			if( dwRequestIndex == m_dwMatchReserveIndex )       // ���ÿ� ���� ������ �� ����
			{
				if( GetIndex() < dwRequestIndex )    // ���ÿ� ���� ������ �Ǿ����Ƿ� �������� �޴� ���� �ȴ�.
					return;
				bCancelMatch = false;
			}	
		}

		if( bCancelMatch )
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::MatchRoomRequest(%d) - (%d) : �ð����� ���� ��Ī ����", GetIndex(), dwRequestIndex );
			LadderTeamParent *pRequestTeam = g_LadderTeamManager.GetGlobalLadderTeamNode( dwRequestIndex );
			if( pRequestTeam )
			{
				// ������� ��ġ ���� ���¸� ����
				pRequestTeam->MatchReserveCancel();
			}
			return;
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::MatchRoomRequest(%d) - (%d) : ������ ���� ���� ��ġ �����Ͽ� �� ����", GetIndex(), dwRequestIndex );
		}
	}

	LadderTeamParent *pRequestTeam = g_LadderTeamManager.GetGlobalLadderTeamNode( dwRequestIndex );
	if( !pRequestTeam )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::MatchRoomRequest(%d) - (%d) : ��û �� ����", GetIndex(), dwRequestIndex );
		return;
	}

	// �� ���� �� ��������� ����.
	Room *pRoom = g_RoomNodeManager.CreateNewRoom();
	if( pRoom )
	{	
		int iMapNum = -1;
		int iSubType = -1;
		ModeType eModeType = MT_NONE;
		int iModeIndex = GetSelectMode();
		int iMapIndex  = GetSelectMap();
		
		// ������� ���� ���� �����ߴٸ� ������� ������.
		if( pRequestTeam->GetSelectMode() != -1 )
			iModeIndex = pRequestTeam->GetSelectMode();
		if( pRequestTeam->GetSelectMap() != -1 )
			iMapIndex  = pRequestTeam->GetSelectMap();

		pRoom->SetRoomStyle( RSTYLE_LADDERBATTLE );
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
			bool bSuccess = g_LadderTeamManager.CheckLadderTeamRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );
			if( bSuccess )
			{
				eModeType = pRoom->SelectNextMode( eModeType, iSubType );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::MatchRoomRequest() - SelectIndex is not Exist(%d): %d", GetIndex(), iModeIndex );
				eModeType = pRoom->SelectNextMode( MT_NONE );
			}
		}

		pRoom->SetModeType( eModeType, -1, -1 );
		// ������ �ε��� �뿡 ����( ����� & ������ )
		pRoom->SetLadderTeam( GetIndex(), pRequestTeam->GetIndex() );
		
		int iCampType = 0;
		int iTeamType = 0;

		iCampType = pRoom->GetLadderTeamCampTypeByTeamIndex(dwRequestIndex);
		iTeamType = pRoom->GetLadderTeamTeamTypeByTeamIndex(dwRequestIndex);

		// ���� ����
        MatchEnterRoom( pRoom, dwRequestIndex, iCampType, iTeamType );

		SetPreSelectModeInfo( pRoom->GetModeType(), pRoom->GetModeSubNum(), pRoom->GetModeMapNum() );


		// ��û�� ���� ���������� ���� ���� ��Ų��.
		iCampType = pRoom->GetLadderTeamCampTypeByTeamIndex(GetIndex());
		iTeamType = pRoom->GetLadderTeamTeamTypeByTeamIndex(GetIndex());

		if( pRequestTeam->IsOriginal() )
		{
			LadderTeamNode *pOtherTeam = (LadderTeamNode*)pRequestTeam;
			pOtherTeam->MatchEnterRoom( pRoom, GetIndex(), iCampType, iTeamType );
		}
		else
		{
			pRequestTeam->MatchRoomRequestJoin( GetIndex(), pRoom->GetModeType(), pRoom->GetModeSubNum(), pRoom->GetModeMapNum(), iCampType, iTeamType );
		}
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BattleRoomNode::MatchRoomRequest() New ReadyGO (%d) : %d", GetIndex(), pRoom->GetRoomIndex() );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomNode::MatchRoomRequest() ���� ������ ���� ����(%d)", GetIndex() );
		return;
	}
}

/************************************************************************/
/* ������ ��� ������ �������ִ� ���� �� ����                           */
/************************************************************************/
void LadderTeamNode::MatchEnterRoom( Room *pRoom, DWORD dwMatchTeamIndex, int iCampType, int iTeamType )
{
	if( !pRoom ) return;

	SetTeamState( TMS_MATCH_PLAY );
	m_pMatchRoom = pRoom;
	m_dwLastMatchTeam = dwMatchTeamIndex;

	if( IsHeroMatchMode() )
	{
		// ��Ī �Ǵ� ������ �����Ѵ�.- ������ �� ���� ���̵��̹Ƿ� �ش� ���̵�� ������ ������ ������ ��û
		LadderTeamParent *pMatchNode = g_LadderTeamManager.GetGlobalLadderTeamNode( m_dwLastMatchTeam );
		if( pMatchNode )
		{
			SP2Packet kPacket( STPK_HERO_MATCH_OTHER_NAME );
			kPacket << pMatchNode->GetTeamName();
			SendPacketTcp( kPacket );
		}
	}

	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for( iter=m_vUserNode.begin() ; iter!=iEnd ; ++iter )
	{
		LadderTeamUser &kUser = *iter;
		UserParent *pUserParent= g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pUserParent == NULL ) continue;

		if( pUserParent->IsUserOriginal() )
		{
			User *pUser = (User*)pUserParent;

			// ���� �ε� ���� ����
			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			PACKET_GUARD_VOID( kPacket.Write(LADDERTEAM_MACRO_MODE_READY_GO) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetModeMapNum()) );
			PACKET_GUARD_VOID( kPacket.Write(iCampType) );
			PACKET_GUARD_VOID( kPacket.Write(iTeamType) );
			pUser->SendMessage( kPacket );
			pUser->EnterRoom( pRoom );
		}
		else        
		{
			//Ÿ������ �ִ� �������� ���� ���� �̵� ����
			UserCopyNode *pUser = (UserCopyNode*)pUserParent;
			pRoom->EnterReserveUser( pUser->GetUserIndex() );
			SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN) );
			PACKET_GUARD_VOID( kPacket.Write(SS_MOVING_ROOM_JOIN_LADDER) );
			PACKET_GUARD_VOID( kPacket.Write((int)pRoom->GetModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetRoomIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetUserIndex()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetModeSubNum()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetModeMapNum()) );
			PACKET_GUARD_VOID( kPacket.Write((int)pRoom->GetPlazaModeType()) );
			PACKET_GUARD_VOID( kPacket.Write(pRoom->GetRoomNumber()) );
			PACKET_GUARD_VOID( kPacket.Write(true) );
			PACKET_GUARD_VOID( kPacket.Write(iCampType) );
			PACKET_GUARD_VOID( kPacket.Write(iTeamType) );
			
			pUser->SendMessage( kPacket );
		}
	}
}

/************************************************************************/
/* ������ ��� �ٸ� ������ �ִ� ���� �������� ������ ���� �ִ� ������   */
/* �������� �����Ѵ�. �����ϱ����� �������� �� ���� �ε����·� �����.  */
/* �ε� ���¾��� ��� �����ϸ� Ŭ���̾�Ʈ���� ���������� ��带 ��������*/
/* ���ϹǷ� ũ������ �߻��Ѵ�                                           */
/************************************************************************/
void LadderTeamNode::MatchRoomRequestJoin( DWORD dwOtherTeamIndex, ModeType eModeType, int iModeSubNum, int iModeMapNum, int iCampType, int iTeamType )
{
	LadderTeamParent *pRequestTeam = g_LadderTeamManager.GetGlobalLadderTeamNode( dwOtherTeamIndex );
	if( !pRequestTeam )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::MatchRoomRequestJoin(%d) - [%d] : ��û �� ����", GetIndex(), dwOtherTeamIndex );
		return;
	}

	if( !pRequestTeam->IsOriginal() )
	{
		// ��������� �������� ����
		SetTeamState( TMS_MATCH_PLAY );
		m_dwLastMatchTeam = dwOtherTeamIndex;

		if( IsHeroMatchMode() )
		{
			// ��Ī �Ǵ� ������ �����Ѵ�.- ������ �� ���� ���̵��̹Ƿ� �ش� ���̵�� ������ ������ ������ ��û
			SP2Packet kPacket( STPK_HERO_MATCH_OTHER_NAME );
			PACKET_GUARD_VOID( kPacket.Write(pRequestTeam->GetTeamName()) );
			SendPacketTcp( kPacket );
		}

		LadderTeamCopyNode *pOtherTeam = (LadderTeamCopyNode*)pRequestTeam;
		// �������� �ٸ��� ���ϰ� �ε�ȭ������ ��ȯ
		SP2Packet kTempPacket( STPK_LADDERTEAM_MACRO );
		PACKET_GUARD_VOID( kTempPacket.Write(LADDERTEAM_MACRO_MODE_READY_GO) );
		PACKET_GUARD_VOID( kTempPacket.Write((int)eModeType) );
		PACKET_GUARD_VOID( kTempPacket.Write(iModeSubNum) );
		PACKET_GUARD_VOID( kTempPacket.Write(iModeMapNum) );
		PACKET_GUARD_VOID( kTempPacket.Write(iCampType) );
		PACKET_GUARD_VOID( kTempPacket.Write(iTeamType) );
		SendPacketTcp( kTempPacket );

		// ���� ��û�ϱ����� �������� ������ ����
		SP2Packet kPacket( SSTPK_LADDERTEAM_ENTER_ROOM_USER );
		PACKET_GUARD_VOID( kPacket.Write(pOtherTeam->GetIndex()) );
		PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
		FillUserIndex( kPacket );
		pOtherTeam->SendMessage( kPacket );		
	}
}

/************************************************************************/
/* Ÿ������ ������ ���������� ������ ������ �뿡 �����Ų��.            */
/* ���� ������ ���������� �ε����·� ��������Ƿ� �ε� ���� �н�.       */
/************************************************************************/
bool LadderTeamNode::MatchRequstTeamEnterRoom( SP2Packet &rkPacket )
{
	if( m_pMatchRoom == NULL ) return false;

	DWORD dwMatchTeamIndex = 0;
	int i = 0, iSize = 0;
	PACKET_GUARD_bool( rkPacket.Read(dwMatchTeamIndex) );
	PACKET_GUARD_bool( rkPacket.Read(iSize) );
	MAX_GUARD(iSize, 50);

	for(i = 0;i < iSize;i++)
	{
		DWORD dwUserIndex = 0;
		PACKET_GUARD_bool( rkPacket.Read(dwUserIndex) );
		
		UserParent *pUserParent= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
		if( pUserParent == NULL ) continue;

		if( pUserParent->IsUserOriginal() )
		{
			/************************************************************************************************/
			/* Ÿ������ ������ �������� �������� �� �������� ���� ��Ż�� ������� �� ������ ���Ѽ��� �ȵȴ�.*/
			/************************************************************************************************/
			User *pUser = (User*)pUserParent;						
			LadderTeamParent *pLadderTeam = pUser->GetMyLadderTeam();
			if( pLadderTeam && pLadderTeam->GetIndex() == dwMatchTeamIndex )
			{
				pUser->EnterRoom( m_pMatchRoom );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::MatchRequstTeamEnterRoom(%d) - [%s] : �� ���� �� ����", GetIndex(), pUser->GetPublicID().c_str() );
			}
		}
		else        
		{
			//Ÿ������ �ִ� �������� ���� ���� �̵� ����
			UserCopyNode *pUser = (UserCopyNode*)pUserParent;
			m_pMatchRoom->EnterReserveUser( pUser->GetUserIndex() );
			SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
			PACKET_GUARD_bool( kPacket.Write(SS_MOVING_ROOM_JOIN) );
			PACKET_GUARD_bool( kPacket.Write(SS_MOVING_ROOM_JOIN_LADDER) );
			PACKET_GUARD_bool( kPacket.Write((int)m_pMatchRoom->GetModeType()) );
			PACKET_GUARD_bool( kPacket.Write(m_pMatchRoom->GetRoomIndex()) );
			PACKET_GUARD_bool( kPacket.Write(pUser->GetUserIndex()) );
			PACKET_GUARD_bool( kPacket.Write(m_pMatchRoom->GetModeSubNum()) );
			PACKET_GUARD_bool( kPacket.Write(m_pMatchRoom->GetModeMapNum()) );
			PACKET_GUARD_bool( kPacket.Write((int)m_pMatchRoom->GetPlazaModeType()) );
			PACKET_GUARD_bool( kPacket.Write(m_pMatchRoom->GetRoomNumber()) );
			PACKET_GUARD_bool( kPacket.Write(false) );
			pUser->SendMessage( kPacket );
		}
	}

	return true;
}

void LadderTeamNode::MatchReStartRoom( Room *pRoom, DWORD dwOtherTeamIndex, int iOtherSelectMode, int iOtherSelectMap )
{
	if( !pRoom ) return;

	m_pMatchRoom = pRoom;

	// �� �����
	int iMapNum = -1;
	int iSubType = -1;
	ModeType eModeType = MT_NONE;
	int iModeIndex = GetSelectMode();
	int iMapIndex  = GetSelectMap();

	// ������� ���� ���� �����ߴٸ� ������� ������.
	if( iOtherSelectMode != -1 )
		iModeIndex = iOtherSelectMode;
	if( iOtherSelectMap != -1 )
		iMapIndex  = iOtherSelectMap;

	m_pMatchRoom->SetRoomStyle( RSTYLE_LADDERBATTLE );

	if( iModeIndex == -1 )	
	{
		eModeType = m_pMatchRoom->SelectNextMode( MT_NONE );
	}
	else					//��Ÿ
	{
		bool bSuccess = g_LadderTeamManager.CheckLadderTeamRandomMode( iModeIndex, iMapIndex, eModeType, iSubType );
		if( bSuccess )
		{
			eModeType = m_pMatchRoom->SelectNextMode( eModeType, iSubType );
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::OnLadderBattleRestart() - SelectIndex is not Exist (%d) : %d", GetIndex(), iModeIndex );
			eModeType = m_pMatchRoom->SelectNextMode( MT_NONE );
		}
	}

	m_pMatchRoom->SetLadderTeam( GetIndex(), dwOtherTeamIndex );
	m_pMatchRoom->SetModeType( eModeType, -1, -1 );
	m_pMatchRoom->CreateNextLadderBattle();
	SetPreSelectModeInfo( m_pMatchRoom->GetModeType(), m_pMatchRoom->GetModeSubNum(), m_pMatchRoom->GetModeMapNum() );
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::MatchReStartRoom - Already ReadyGO(%d) : (%d) vs (%d)", m_pMatchRoom->GetRoomIndex(), 
							GetIndex(), dwOtherTeamIndex );
}

void LadderTeamNode::SetPreSelectModeInfo( const ModeType eModeType, const int iSubType, const int iMapNum )
{
	m_PreModeType = eModeType;
	m_iPreSubType = iSubType;
	m_iPreMapNum  = iMapNum;
}

void LadderTeamNode::GetPreSelectModeInfo( ModeType &eModeType, int &iSubType, int &iMapNum )
{
	eModeType = m_PreModeType;
	iSubType = m_iPreSubType;
	iMapNum = m_iPreMapNum;
}

bool LadderTeamNode::IsReserveTimeOver()
{
	if( m_dwReserveTime == 0 ) return false;
	if( !m_vUserNode.empty() ) return false;
	if( TIMEGETTIME() - m_dwReserveTime > CREATE_RESERVE_DELAY_TIME )
		return true;
	return false;
}

void LadderTeamNode::SetMatchReserve( DWORD dwTeamIndex )
{
	m_dwMatchReserveIndex = dwTeamIndex;

	int iSize = m_MatchReserve.size();
	for(int i = 0;i < iSize;i++)
	{
		if( m_MatchReserve[i].m_dwTeamIndex == dwTeamIndex )
		{
			m_MatchReserve[i].m_dwReserveTime = TIMEGETTIME();
			return;
		}
	}

	MatchReserve kMatchReserve;
	kMatchReserve.m_dwTeamIndex = dwTeamIndex;
	kMatchReserve.m_dwReserveTime = TIMEGETTIME();
	m_MatchReserve.push_back( kMatchReserve );
}

bool LadderTeamNode::IsMatchReserve( DWORD dwTeamIndex )
{
	int iSize = m_MatchReserve.size();
	for(int i = 0;i < iSize;i++)
	{
		if( m_MatchReserve[i].m_dwTeamIndex == dwTeamIndex )
		{
			DWORD dwGapTime = TIMEGETTIME() - m_MatchReserve[i].m_dwReserveTime;
			if( dwGapTime < 5000 )
				return true;
			return false;
		}
	}
	return false;
}

bool LadderTeamNode::IsReMatchLimit( DWORD dwCheckTeam, DWORD dwLimitTime )
{
	if( m_dwLastMatchTeam == 0 ) return false;
	if( m_dwCurLastBattleTime == 0 ) return false;
	if( m_dwLastMatchTeam != dwCheckTeam ) return false;
	if( TIMEGETTIME() - m_dwCurLastBattleTime > dwLimitTime ) return false;

    return true;
}

DWORD LadderTeamNode::GetGuildIndex() const
{
	return m_dwGuildIndex;
}
DWORD LadderTeamNode::GetGuildMark() const
{
	return m_dwGuildMark;
}

bool LadderTeamNode::IsGuildTeam() const
{
	if( GetGuildIndex() == 0 ) return false;

	return true;
}

bool LadderTeamNode::IsFull() const
{
	if( GetJoinUserCnt() >= GetMaxPlayer() ) return true;

	return false;
}

bool LadderTeamNode::IsEmptyUser()
{
	return m_vUserNode.empty();
}

bool LadderTeamNode::IsSearchLevelMatch()
{
	return m_bSearchLevelMatch;
}

bool LadderTeamNode::IsSearchSameUser()
{
	return m_bSearchSameUser;
}

bool LadderTeamNode::IsBadPingKick()
{
	return m_bBadPingKick;
}

bool LadderTeamNode::IsSearching()
{
	if( GetTeamState() == TMS_SEARCH_PROCEED )
		return true;
	return false;
}

bool LadderTeamNode::IsMatchPlay()
{
	if( GetTeamState() == TMS_MATCH_PLAY )
		return true;
	return false;
}

bool LadderTeamNode::IsHeroMatchMode()
{
	return m_bHeroMatchMode;
}

int LadderTeamNode::GetAbilityMatchLevel()
{
	int iLevel = 0;	
	if( IsHeroMatchMode() )
	{
		const int iDivide = (float)g_LevelMatchMgr.GetRoomEnterLevelMax() * 0.5f;
		if( iDivide == 0 )
			return 0;

		int iMatchPoint = GetHeroMatchPoint();
		int iAvgStep    = ( (float)g_LadderTeamManager.GetHeroMatchAveragePoint() / iDivide );
		if( iAvgStep == 0 )
			return 0;
		iLevel = (float)iMatchPoint / iAvgStep;
	}
	else
	{
		int iSize  = 0;
		vLadderTeamUser_iter iter, iEnd;
		iEnd = m_vUserNode.end();
		for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
		{
			LadderTeamUser &kUser = *iter;
			if( IsHeroMatchMode() )
			{
				iSize++;
				iLevel += kUser.m_iHeroMatchPoint;
			}
			else if( !g_UserNodeManager.IsDeveloper( kUser.m_szPublicID.c_str() ) )
			{
				iSize++;
				iLevel += kUser.m_iAbilityLevel;
			}
		}

		if( iSize <= 0 )
		{
			return 0;
		}

		iLevel /= iSize;

	}
	return min( g_LevelMatchMgr.GetRoomEnterLevelMax() - 1, iLevel );
}

//HRYOON LADDER
//������ ��� ���� ��Ī
//-00�� ���� �����ߴ� �����ʹ� �������� ����
bool LadderTeamNode::FindMatchingUser( int userIndex, int competitorIndex )
{
	int iSize = (int)m_ladderList.size();

	if( iSize == 0 )
		return true;

	for(int i = 0;i < iSize;i++)
	{
		int dwCompetitorIndex = m_ladderList[i];
		if( dwCompetitorIndex == competitorIndex )
		{
			return false;
		}
	}
	return true;

}

void LadderTeamNode::ClearLadderList()
{
	m_ladderList.clear();
}

int LadderTeamNode::GetHeroMatchPoint()
{
	if( !IsHeroMatchMode() ) return 0;

	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		
		return kUser.m_iHeroMatchPoint;
	}
	return 0;
}

int LadderTeamNode::GetTeamLevel()
{
	int iTeamLevel = GetAbilityMatchLevel() - g_LevelMatchMgr.GetAddGradeLevel();
	iTeamLevel = min( max( iTeamLevel, 0 ), g_LevelMgr.GetMaxGradeLevel() );

	return iTeamLevel;
}

const LadderTeamUser &LadderTeamNode::GetUserNodeByArray( int iArray )
{
	if( COMPARE( iArray, 0, (int) m_vUserNode.size()) )
		return m_vUserNode[iArray];

	static LadderTeamUser kError;
	return kError;
}

UserParent *LadderTeamNode::GetUserNode( const ioHashString &szPublicID )
{
	CRASH_GUARD();
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_szPublicID == szPublicID )
			return g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
	}
	return NULL;
}

void LadderTeamNode::UpdateRecord( TeamType ePlayTeam, TeamType eWinTeam )
{
	if( eWinTeam == TEAM_NONE )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamNode::UpdateRecord(%d) : %s Draw", GetIndex(), GetTeamName().c_str() );
		return;
	}
	else if( eWinTeam == ePlayTeam )
	{
		m_iWinRecord++;
	    m_iVictoriesRecord++;		
	}
	else
	{
		m_iLoseRecord++;
		m_iVictoriesRecord = 0;		
	}
	m_NodeSync.Update( LadderTeamSync::LTS_CHANGERECORD );

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeamNode::UpdateRecord(%d) : %s (%d - %d) %d,%d,%d", GetIndex(), GetTeamName().c_str(), (int)ePlayTeam, (int)eWinTeam,
							 m_iWinRecord, m_iLoseRecord, m_iVictoriesRecord );
}

void LadderTeamNode::UpdateRanking( int iTeamRanking )
{
	if( m_iTeamRanking != iTeamRanking )
	{
		m_iTeamRanking = iTeamRanking;
	}
}

int  LadderTeamNode::GetWinRecord()
{
	return m_iWinRecord;
}

int  LadderTeamNode::GetLoseRecord()
{
	return m_iLoseRecord;
}

int  LadderTeamNode::GetVictoriesRecord()
{
	return m_iVictoriesRecord;
}

void LadderTeamNode::SendPacketTcp( SP2Packet &rkPacket, const DWORD dwUserIndex )
{
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex ) continue;

		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pUser )
			pUser->RelayPacket( rkPacket );
	}
}

void LadderTeamNode::SendPacketTcpUser( SP2Packet &rkPacket, const ioHashString &rkSenderName )
{
	CRASH_GUARD();
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_szPublicID == rkSenderName ) 
		{
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
			if( pUser )
				pUser->RelayPacket( rkPacket );
			return;
		}		
	}
}

void LadderTeamNode::SendPacketUdp( SP2Packet &rkPacket, const DWORD dwUserIndex )
{
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_dwUserIndex == dwUserIndex ) continue;

		g_UDPNode.SendMessage( (char*)kUser.m_szPublicIP.c_str(), kUser.m_iClientPort, rkPacket );
	}
}

void LadderTeamNode::CheckGuildTeam( bool bPlayEnd /* = false  */ )
{
	DWORD dwPrevIndex = m_dwGuildIndex;
	m_dwGuildIndex = 0;
	m_dwGuildMark  = 0;

	// �������� ������� ���� ����.
	if( !IsHeroMatchMode() )
	{
		vLadderTeamUser_iter iter, iEnd;
		iEnd = m_vUserNode.end();
		for(iter = m_vUserNode.begin();iter != iEnd;++iter)
		{
			LadderTeamUser &kUser = *iter;
			// ������ ���� ���Ѿ���

			if( kUser.m_dwGuildIndex == 0 )
			{
				m_dwGuildIndex = 0;
				break;
			}
			else if( m_dwGuildIndex == 0 )
			{
				m_dwGuildIndex = kUser.m_dwGuildIndex;
				m_dwGuildMark  = kUser.m_dwGuildMark;
			}
			else if( m_dwGuildIndex != kUser.m_dwGuildIndex )
			{
				m_dwGuildIndex = 0;
				break;
			}
		}
	}

	if( dwPrevIndex != m_dwGuildIndex || bPlayEnd )
	{
		m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );

		// �÷����߿��� ���� ������ �ݿ����� �ʴ´�.
		if( GetTeamState() != TMS_MATCH_PLAY )
		{
			// �����鿡�� ����
			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			kPacket << LADDERTEAM_MACRO_CHANGE_GUILD << m_dwGuildIndex;
			SendPacketTcp( kPacket );
		}
	}
}

bool LadderTeamNode::LadderTeamKickOutUser( ioHashString &rkName )
{
	CRASH_GUARD();
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter = m_vUserNode.begin();iter != iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		if( kUser.m_szPublicID == rkName )
		{
			SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
			kPacket << LADDERTEAM_MACRO_KICK_OUT << kUser.m_szPublicID;
			SendPacketTcp( kPacket );			

			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
			if( pUserParent )
			{
				if( pUserParent->IsUserOriginal() )
				{
					User *pUser = (User*)pUserParent;
					pUser->LadderTeamKickOut();
				}            
				else
				{
					UserCopyNode *pUser = (UserCopyNode*)pUserParent;
					SP2Packet kPacket( SSTPK_LADDERTEAM_KICK_OUT );
					kPacket << kUser.m_dwUserIndex;
					pUser->SendMessage( kPacket );
				}
			}    
			return true;
		}
	}
	return false;
}

void LadderTeamNode::FillSyncCreate( SP2Packet &rkPacket )
{
	//LTS_CHANGEINFO
	rkPacket << GetIndex() << GetTeamName() << GetPW()  << GetJoinUserCnt() << GetMaxPlayer() << GetAbilityMatchLevel() << GetHeroMatchPoint();	
	rkPacket << GetJoinGuildIndex() << IsSearchLevelMatch() << IsSearchSameUser() << IsBadPingKick() << GetTeamState() << GetGuildIndex() << GetGuildMark() << GetSelectMode() << GetSelectMap();
	//LTS_CHANGERECORD
	rkPacket << GetWinRecord() << GetLoseRecord() << GetVictoriesRecord();								
	//LTS_CREATE
	rkPacket << GetCampType() << IsHeroMatchMode();                          
}

void LadderTeamNode::FillLadderTeamInfo( SP2Packet &rkPacket )
{
	rkPacket << GetIndex() << GetCampType() << GetTeamName() << GetPW() << GetOwnerName() << GetMaxPlayer();
	rkPacket << GetWinRecord() << GetLoseRecord() << GetVictoriesRecord();
}

void LadderTeamNode::FillUserInfo( SP2Packet &rkPacket, const LadderTeamUser &kUser )
{
	rkPacket << kUser.m_szPublicID << kUser.m_iGradeLevel << kUser.m_iAbilityLevel << kUser.m_iLadderPoint;
	rkPacket << kUser.m_dwUserIndex << kUser.m_szPublicIP << kUser.m_iClientPort << kUser.m_szPrivateIP << kUser.m_szTransferIP << kUser.m_iTransferPort;

	//��� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
	if( pUserParent )
		rkPacket << pUserParent->GetGuildIndex() << pUserParent->GetGuildMark();
	else
		rkPacket << 0 << 0;
}

void LadderTeamNode::FillUserList( SP2Packet &rkPacket )
{
	rkPacket << (int)m_vUserNode.size();
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		rkPacket << kUser.m_iGradeLevel << kUser.m_szPublicID << kUser.m_iLadderPoint;

		//��� ���� & �� ����
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( kUser.m_dwUserIndex );
		if( pUserParent )
			rkPacket << pUserParent->GetGuildIndex() << pUserParent->GetGuildMark() << pUserParent->GetPingStep();
		else
			rkPacket << 0 << 0 << 0;
	}
}

void LadderTeamNode::FillUserIndex( SP2Packet &rkPacket )
{
	rkPacket << (int)m_vUserNode.size();
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		rkPacket << kUser.m_dwUserIndex;
	}
}

void LadderTeamNode::FillUserIndex( DWORDVec &rkUserIndex )
{
	vLadderTeamUser_iter iter, iEnd;
	iEnd = m_vUserNode.end();
	for(iter=m_vUserNode.begin();iter!=iEnd;++iter)
	{
		LadderTeamUser &kUser = *iter;
		rkUserIndex.push_back( kUser.m_dwUserIndex );
	}
}

void LadderTeamNode::Process()
{
	m_NodeSync.Process();

	// 3�ʸ��� �˻�
	if( GetTeamState() == TMS_SEARCH_RESERVE || GetTeamState() == TMS_SEARCH_PROCEED )
	{
		//Search Start
		DWORD dwGapTime = TIMEGETTIME() - m_dwSearchMatchCurTime;
		if( dwGapTime <= g_LadderTeamManager.GetSearchMatchFullTime() )
		{
			int iNextSec = dwGapTime / 1000;
			if( iNextSec != m_iSearchMatchCurSec )
			{
				m_iSearchMatchCurSec = iNextSec;
				if( m_iSearchMatchCurSec % g_LadderTeamManager.GetSearchMatchSendSec() == 0 )
				{
					SetTeamState( TMS_SEARCH_PROCEED );
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "LadderTeam Search State: (%d) - %d", GetIndex(), GetTeamState() );
				}
			}
		}
		else
		{
			SetTeamState( TMS_READY );
		}
	}	
}

bool LadderTeamNode::OnProcessPacket( SP2Packet &rkPacket, UserParent *pUser )
{
	switch( rkPacket.GetPacketID() )
	{
	case CTPK_LADDERTEAM_MACRO:
		OnMacroCommand( rkPacket, pUser );
		return true;
	case CTPK_LADDERTEAM_INVITE:
		OnLadderTeamInvite( rkPacket, pUser );
		return true;
	case CTPK_VOICE_INFO:
		OnVoiceInfo( rkPacket, pUser );
		return true;
	}
	return false;
}

void LadderTeamNode::OnLadderTeamInfo( UserParent *pUser )
{
	if( !pUser ) return;

	SP2Packet kPacket( STPK_LADDERTEAM_JOIN_INFO );
	kPacket << GetIndex() << g_LadderTeamManager.GetSortLadderTeamState( (LadderTeamParent*)this, pUser );
	kPacket << GetCampType() << GetTeamName() << GetPW() << GetTeamLevel() << GetWinRecord() << GetLoseRecord() << GetVictoriesRecord() << GetMaxPlayer();
	FillUserList( kPacket );
	pUser->RelayPacket( kPacket );
}	

void LadderTeamNode::OnVoiceInfo( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;

	int iType;
	rkPacket >> iType;

	if( !COMPARE( iType, ID_VOICE_ON, ID_VOICE_PERMIT + 1) ) return;

	ioHashString szReceiverID;
	rkPacket >> szReceiverID;

	SP2Packet kReturnPacket( STPK_VOICE_INFO );
	kReturnPacket << iType;
	kReturnPacket << pUser->GetPublicID();
	kReturnPacket << (int)TEAM_BLUE;     /*���� ����*/

	if( szReceiverID.IsEmpty() ) // ID�� ��� ������ ��Ƽ�� ��忡�� ����
	{
		SendPacketTcp( kReturnPacket, pUser->GetUserIndex() );
	}
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

void LadderTeamNode::OnLadderTeamInvite( SP2Packet &rkPacket, UserParent *pUser )
{
	if( pUser == NULL ) return;
	if( IsFull() ) return;

	SP2Packet kPacket( STPK_LADDERTEAM_INVITED );
	PACKET_GUARD_VOID( kPacket.Write(GetCampType()) );
	PACKET_GUARD_VOID( kPacket.Write(GetIndex()) );
	PACKET_GUARD_VOID( kPacket.Write(GetTeamName()) );
	PACKET_GUARD_VOID( kPacket.Write(GetPW()) );
	PACKET_GUARD_VOID( kPacket.Write(GetOwnerName()) );
	PACKET_GUARD_VOID( kPacket.Write(GetMaxPlayer()) );
	PACKET_GUARD_VOID( kPacket.Write(GetTeamLevel()) );
	PACKET_GUARD_VOID( kPacket.Write(GetWinRecord()) );
	PACKET_GUARD_VOID( kPacket.Write(GetLoseRecord()) );
	PACKET_GUARD_VOID( kPacket.Write(GetVictoriesRecord()) );
	FillUserList( kPacket );
	// ����.....
	int iSize = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iSize) );
	MAX_GUARD(iSize, 50);

	for(int i = 0;i < iSize;i++)
	{		
		ioHashString szInvitedID;
		PACKET_GUARD_VOID( rkPacket.Read(szInvitedID) );
		UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( szInvitedID );
		if( pInviteUser && pInviteUser->GetUserPos() != UP_BATTLE_ROOM && pInviteUser->GetUserPos() != UP_LADDER_TEAM && GetCampType() == pInviteUser->GetUserCampPos() )
		{
			pInviteUser->RelayPacket( kPacket );
		}
	}	
}

void LadderTeamNode::OnMacroCommand( SP2Packet &rkPacket, UserParent *pUser )
{
	int iMacroCmd;
	rkPacket >> iMacroCmd;
	switch( iMacroCmd )
	{
	case LADDERTEAM_MACRO_CHANGE_INFO:
		{
			CRASH_GUARD();
			if( GetOwnerName() == pUser->GetPublicID() )
			{
				rkPacket >> m_szTeamName >> m_szTeamPW >> m_dwJoinGuildIndex;

				SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
				kPacket << LADDERTEAM_MACRO_CHANGE_INFO << m_szTeamName << m_szTeamPW << m_dwJoinGuildIndex;
				SendPacketTcp( kPacket, pUser->GetUserIndex() );

				m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "����� �����ڰ� �ƴѵ� ����/��� ���� ��ȣ ����(%d) : %s", GetIndex(), pUser->GetPublicID().c_str() );
		}
		break;
	case LADDERTEAM_MACRO_MAX_PLAYER:
		{
			CRASH_GUARD();
			if( GetOwnerName() == pUser->GetPublicID() )
			{				
				// ��Ī ���� ���¶�� ���¸� ���� �� ������.
				if( GetTeamState() == TMS_MATCH_RESERVE )     
				{
					// �������� ������
					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_MAX_PLAYER << GetMaxPlayer();
					pUser->RelayPacket( kPacket );
				}
				else
				{
					int iWantMaxPlayer;
					rkPacket >> iWantMaxPlayer;
					if( iWantMaxPlayer < GetJoinUserCnt() )         
						iWantMaxPlayer = GetJoinUserCnt();
					else if( iWantMaxPlayer > MAX_LADDERTEAM_PLAYER )         
						iWantMaxPlayer = MAX_LADDERTEAM_PLAYER;
					SetSaveMaxPlayer( GetMaxPlayer() );
					SetMaxPlayer( iWantMaxPlayer );

					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_MAX_PLAYER << GetMaxPlayer();
					SendPacketTcp( kPacket );

					//�˻� �������
					SetTeamState( TMS_READY );
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "����� �����ڰ� �ƴѵ� �ο� ���� ��ȣ ���� (%d) : %s", GetIndex(), pUser->GetPublicID().c_str() );
		}
		break;
	case LADDERTEAM_MACRO_KICK_OUT:
		{
			CRASH_GUARD();
			if( GetOwnerName() == pUser->GetPublicID() )
			{
				ioHashString rkName;
				rkPacket >> rkName;
				if( !LadderTeamKickOutUser( rkName ) )
				{
					// ���� ����
					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_KICK_OUT << "";
					pUser->RelayPacket( kPacket );
				}
				//�˻� �������
				SetTeamState( TMS_READY );
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "����� �����ڰ� �ƴѵ� ���� ��ȣ ���� (%d) : %s", GetIndex(), pUser->GetPublicID().c_str() );
		}
		break;
	case LADDERTEAM_MACRO_MODE_SEL:
		{
			CRASH_GUARD();
			
			if( GetOwnerName() == pUser->GetPublicID() )
			{
				// ��Ī ���� ���¶�� ���¸� ���� �� ������.
				if( GetTeamState() == TMS_MATCH_RESERVE )     
				{
					// �������� ������
					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_MODE_SEL << m_iSelectMode << m_iSelectMap;
					pUser->RelayPacket( kPacket );
				}
				else
				{
					rkPacket >> m_iSelectMode >> m_iSelectMap;
					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_MODE_SEL << m_iSelectMode << m_iSelectMap;
					SendPacketTcp( kPacket );
					//�˻� �������
					SetTeamState( TMS_READY );
				}
			}
		}
		break;
	case LADDERTEAM_MACRO_CHANGE_OPTION:
		{
			CRASH_GUARD();
			if( GetOwnerName() == pUser->GetPublicID() )
			{
				// ��Ī ���� ���¶�� ���¸� ���� �� ������.
				if( GetTeamState() == TMS_MATCH_RESERVE )     
				{
					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_CHANGE_OPTION << m_bSearchLevelMatch << m_bSearchSameUser << m_bBadPingKick;
					pUser->RelayPacket( kPacket );
				}
				else
				{
					rkPacket >> m_bSearchLevelMatch >> m_bSearchSameUser >> m_bBadPingKick;

					SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
					kPacket << LADDERTEAM_MACRO_CHANGE_OPTION << m_bSearchLevelMatch << m_bSearchSameUser << m_bBadPingKick;
					SendPacketTcp( kPacket );

					m_NodeSync.Update( LadderTeamSync::LTS_CHANGEINFO );

					//�˻� �������
					SetTeamState( TMS_READY );
				}
			}
		}
		break;
	case LADDERTEAM_MACRO_TEAM_STATE:
		{
			DWORD dwState;
			rkPacket >> dwState;
			SetTeamState( dwState );
			if( dwState != GetTeamState() )
			{
				// ������Ʈ ���� �Ұ�. ��κ� �˻� ��Ұ� ���� �ʴ� ��Ȳ( ��Ī ���� ���� )
				SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
				kPacket << LADDERTEAM_MACRO_SEARCH_CANCEL_FAILED;
				pUser->RelayPacket( kPacket );
			}
		}
		break;
	case LADDERTEAM_MACRO_MODE_READY_GO:
		{
		}
		break;
	}
}


void LadderTeamNode::ladderUserCopy( std::vector<int>& vLadderList )
{
	m_ladderList.assign( vLadderList.begin(), vLadderList.end() ); 
}