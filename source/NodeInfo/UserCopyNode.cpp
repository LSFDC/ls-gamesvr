#include "stdafx.h"

#include "UserNodeManager.h"
#include "UserCopyNode.h"
#include "../Local/ioLocalParent.h"
#include "../MainProcess.h"
#include "../DataBase/DBClient.h"
#include "../EtcHelpFunc.h"

UserCopyNode::UserCopyNode()
{
	InitData();
}

UserCopyNode::~UserCopyNode()
{
}

void UserCopyNode::OnCreate( ServerNode *pCreator )
{
	CopyNodeParent::OnCreate( pCreator );
	m_eCopyType = USER_TYPE;
	InitData();
}

void UserCopyNode::OnDestroy()
{
	CopyNodeParent::OnDestroy();
}

void UserCopyNode::InitData()
{
	// 기본 정보
	m_dwUserIndex	= 0;
	m_dwDBAgentID   = 0;
	m_iCampType     = 0;
	m_szPrivateID.c_str();
	m_szPublicID.c_str();
	m_iGradeLevel	= 0;
	m_iUserPos		= 0;
	m_iKillDeathLevel = 0;
	m_iLadderPoint    = 0;
	m_bSafetyLevel = false;
	m_eModeType  = MT_NONE;
	m_bDeveloper = false;
	m_iUserRank  = 0;
	m_dwPingStep = 0;
	m_dwGuildIndex = 0;
	m_dwGuildMark  = 0;
	m_vBestFriend.clear();

	m_bShuffleGlobalSearch = false;

	m_dwEUContryType = 0;
}

bool UserCopyNode::RelayPacket( SP2Packet &rkPacket )
{
	if( m_dwUserIndex == 0 )
		return false;
	// 날중계를 위해 인덱스를 패킷에 넣어 가공한다.
	SP2Packet kPacket( m_dwUserIndex, rkPacket );
	//
	return CopyNodeParent::SendMessage( kPacket );
}

void UserCopyNode::ApplySyncCreate( SP2Packet &rkPacket )
{
	rkPacket >> m_dwDBAgentID >> m_szPrivateID >> m_szPublicID >> m_iCampType >> m_iUserRank;
	rkPacket >> m_iGradeLevel >> m_iUserPos >> m_iKillDeathLevel >> m_iLadderPoint >> m_bSafetyLevel >> m_dwGuildIndex >> m_dwGuildMark;
	
	if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_LATIN )
	{
		rkPacket >> m_szCountry >> m_szConnTime >> m_iFirstWinCount >> m_iFistLoseCount >> m_iFirstPeso >> m_iGiveUp >> m_iFirstExp >> m_iClientLogoutType >> m_szGender;
		rkPacket >> m_iWinCount >> m_iLoseCount;	
	}
	//추가
	if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_EU )
	{
		rkPacket >> m_dwEUContryType;
	}
	/*
	PACKET_GUARD_VOID( rkPacket.Write(GetUserIndex()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetUserDBAgentID()) );
	PACKET_GUARD_VOID( rkPacket.Write(GetPrivateID()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetPublicID()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetUserCampPos()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetUserRanking()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetGradeLevel()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetUserPos()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetKillDeathLevel()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetLadderPoint()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(IsSafetyLevel()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetGuildIndex()) ); 
	PACKET_GUARD_VOID( rkPacket.Write(GetGuildMark()) );

	PACKET_GUARD_VOID( rkPacket.Write( GetCountry() );
	PACKET_GUARD_VOID( rkPacket.Write( GetLatinConnTime() );
	PACKET_GUARD_VOID( rkPacket.Write( GetFistWinCount() );
	PACKET_GUARD_VOID( rkPacket.Write( GetFirstLoseCount() );
	PACKET_GUARD_VOID( rkPacket.Write( GetFirstMoney() );
	PACKET_GUARD_VOID( rkPacket.Write( GetGiveupCount() );
	PACKET_GUARD_VOID( rkPacket.Write( GetFirstExp() );
	PACKET_GUARD_VOID( rkPacket.Write( GetLogoutType() );
	PACKET_GUARD_VOID( rkPacket.Write( GetGender() );

	m_szCountry.c_str();
	m_szConnTime.c_str();
	m_szGender.c_str();
	m_iFirstWinCount = 0;
	m_iFistLoseCount = 0;	
	m_iFirstPeso = 0;		//라틴 : 로그아웃 시 페소 - 첫 로긴시 페소
	m_iGiveUp = 0;			//게임포기 횟수
	m_iFirstExp = 0;		//로긴시 경험치
	m_iGiveUp = 0;			
	*/
	
	m_vBestFriend.clear();
	int iMaxBestFriend;
	rkPacket >> iMaxBestFriend;
	for(int i = 0;i < iMaxBestFriend;i++)
	{
		DWORD dwBestFriendIndex;
		rkPacket >> dwBestFriendIndex;
		m_vBestFriend.push_back( dwBestFriendIndex );
	}

	m_bDeveloper = g_UserNodeManager.IsDeveloper( m_szPublicID.c_str() );
}

void UserCopyNode::ApplySyncUpdate( SP2Packet &rkPacket )
{
	int iModeType;
	rkPacket >> m_iGradeLevel >> m_iUserPos >> m_dwPingStep >> m_iKillDeathLevel >> m_iLadderPoint >> m_bSafetyLevel >> iModeType;
	m_eModeType = (ModeType)iModeType;
}

void UserCopyNode::ApplySyncPos( SP2Packet &rkPacket )
{
	int iModeType;
	rkPacket >> m_iUserPos >> m_dwPingStep >> iModeType;
	m_eModeType = (ModeType)iModeType;
}

void UserCopyNode::ApplySyncGuild( SP2Packet &rkPacket )
{
	rkPacket >> m_dwGuildIndex >> m_dwGuildMark;
}

void UserCopyNode::ApplySyncCamp( SP2Packet &rkPacket )
{
	rkPacket >> m_iCampType;
}

void UserCopyNode::ApplySyncPublicID( SP2Packet &rkPacket )
{
	rkPacket >> m_szPublicID;
	//LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s (%d:%s)", __FUNCTION__, m_dwUserIndex, m_szPublicID );
}

void UserCopyNode::ApplySyncBestFriend( SP2Packet &rkPacket )
{
	m_vBestFriend.clear();
	int iMaxBestFriend;
	rkPacket >> iMaxBestFriend;
	for(int i = 0;i < iMaxBestFriend;i++)
	{
		DWORD dwBestFriendIndex;
		rkPacket >> dwBestFriendIndex;
		m_vBestFriend.push_back( dwBestFriendIndex );
	}
}

void UserCopyNode::ApplyUserNode( UserParent *pUser )
{
	if( pUser == NULL ) return;

	m_dwDBAgentID = pUser->GetUserDBAgentID();
	m_szPrivateID = pUser->GetPrivateID();
	m_szPublicID  = pUser->GetPublicID();
	m_iCampType   = pUser->GetUserCampPos();
	m_iGradeLevel = pUser->GetGradeLevel();
	m_iUserPos    = pUser->GetUserPos();
	m_iKillDeathLevel  = pUser->GetKillDeathLevel();
	m_iLadderPoint= pUser->GetLadderPoint();
	m_bSafetyLevel= pUser->IsSafetyLevel();
	m_iUserRank   = pUser->GetUserRanking();
	m_dwPingStep  = pUser->GetPingStep();
	m_dwGuildIndex= pUser->GetGuildIndex();
	m_dwGuildMark = pUser->GetGuildMark();
	pUser->GetBestFriend( m_vBestFriend );	
	//hr 라틴 추가
	
	m_szCountry			= pUser->GetCountry();
	m_szConnTime		= pUser->GetLatinConnTime();
	m_iFirstWinCount	= pUser->GetFistWinCount();
	m_iFistLoseCount	= pUser->GetFirstLoseCount();
	m_iFirstPeso		= pUser->GetFirstMoney();
	m_iGiveUp			= pUser->GetGiveupCount();
	m_iFirstExp			= pUser->GetFirstExp();
	m_iClientLogoutType = pUser->GetLogoutType();
	m_szGender			= pUser->GetGender();
	m_iWinCount			= pUser->GetWinCount();
	m_iLoseCount		= pUser->GetLoseCount();
	m_dwEUContryType	= pUser->GetEUCountryType();
}

void UserCopyNode::ApplySyncShuffle( SP2Packet &rkPacket )
{
	rkPacket >> m_bShuffleGlobalSearch;
}

bool  UserCopyNode::IsGuild()
{
	if( m_dwGuildIndex == 0 )
		return false;
	return true;
}

DWORD UserCopyNode::GetGuildIndex()
{
	if( IsGuild() )
		return m_dwGuildIndex;
	return 0;
}

DWORD UserCopyNode::GetGuildMark()
{
	if( IsGuild() )
		return m_dwGuildMark;
	return 0;
}

bool UserCopyNode::IsBestFriend( DWORD dwUserIndex )
{
	for(int i = 0;i < (int)m_vBestFriend.size();i++)
	{
		if( dwUserIndex == m_vBestFriend[i] )
			return true;
	}
	return false;
}

void UserCopyNode::GetBestFriend( DWORDVec &rkUserIndexList )
{
	rkUserIndexList.clear();
	for(int i = 0;i < (int)m_vBestFriend.size();i++)
	{
		rkUserIndexList.push_back( m_vBestFriend[i] );
	}
}

bool UserCopyNode::IsShuffleGlboalSearch()
{
	return m_bShuffleGlobalSearch; 
}

void UserCopyNode::InsertUserLadderList( int competitorIndex, int ladderIndex )
{
	if( competitorIndex != 0 )
	{
		g_DBClient.OnSetLadderUserList( GetUserDBAgentID(), GetAgentThreadID(), GetUserIndex(), competitorIndex, Help::GetLaddeLimitTime(), ladderIndex );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "UserCopyNode::InsertUserLadderList UserID:%s,UserIndex:%d,LadderTeamIndex:%d(competitorIndex:%d)", 
			GetPublicID().c_str(), GetUserIndex(),ladderIndex, competitorIndex );
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "User::InsertUserLadderList %d(competitorIndex Error)", GetUserIndex() );
}

void UserCopyNode::CopyLadderUserList( std::vector<int>& vLadderList )
{
	vLadderList.assign( m_ladderList.begin(), m_ladderList.end() );
}