

#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"

#include "CatchRunningManMode.h"

#include "Room.h"
#include "ModeHelp.h"
#include "RoomNodeManager.h"
#include "../DataBase/LogDBClient.h"

CatchRunningManMode::CatchRunningManMode( Room *pCreator ) : CatchMode( pCreator )
{
}

CatchRunningManMode::~CatchRunningManMode()
{
}

void CatchRunningManMode::LoadINIValue()
{
	CatchMode::LoadINIValue();

	m_RunningManDecoList.clear();
	for(int i = 0;i < MAX_RUNNINGMAN_DECO;i++)
		m_RunningManDecoList.push_back( i + 1 );
	std::random_shuffle( m_RunningManDecoList.begin(), m_RunningManDecoList.end() );
}

void CatchRunningManMode::DestroyMode()
{
	CatchMode::DestroyMode();
	m_RunningManDecoList.clear();
}

void CatchRunningManMode::AddNewRecord( User *pUser )
{
	CatchMode::AddNewRecord( pUser );
}

void CatchRunningManMode::RemoveRecord( User *pUser, bool bRoomDestroy )
{
	int iCharCnt = m_vRecordList.size();
	for( int i=0 ; i<iCharCnt ; i++ )
	{
		if( m_vRecordList[i].pUser == pUser )
		{
			SetModeEndDBLog( &m_vRecordList[i], iCharCnt, LogDBClient::PRT_EXIT_ROOM );

			if( m_vRecordList[i].bPrisoner || m_vRecordList[i].bDieState )
			{
				if( m_vRecordList[i].pUser->GetTeam() == TEAM_RED )
				{
					m_iBlueCatchRedPlayer--;
					m_iBlueCatchRedPlayer = max( 0, m_iBlueCatchRedPlayer );
				}
				else if( m_vRecordList[i].pUser->GetTeam() == TEAM_BLUE )
				{
					m_iRedCatchBluePlayer--;
					m_iRedCatchBluePlayer = max( 0, m_iRedCatchBluePlayer );
				}
			}
			SetRunningManDecoIndex( m_vRecordList[i].dwRunningManDeco );
			m_vRecordList.erase( m_vRecordList.begin() + i );			
			break;
		}
	}

	UpdateUserRank();

	if( pUser )
	{
		m_KickOutVote.RemoveVoteUserList( pUser->GetPublicID() );
	}

	if( !m_bRoundSetEnd && !bRoomDestroy && GetState() == MS_PLAY )
	{
		// ������ ���� ������ 1�� !!
		WinTeamType eWinTeam = WTT_DRAW;	
		int iBlueUser = GetCurTeamUserCnt( TEAM_BLUE );
		int iRedUser  = GetCurTeamUserCnt( TEAM_RED );
		if( iBlueUser == 0 )
			eWinTeam = WTT_RED_TEAM;
		else if( iRedUser == 0 )
			eWinTeam = WTT_BLUE_TEAM;
		else 
			return;

		SetRoundEndInfo( eWinTeam );
		if( m_bRoundSetEnd )       //��Ʈ�� ����Ǹ� ��Ʈ�� ����� ����
		{
			if( m_iRedTeamWinCnt > m_iBlueTeamWinCnt )
				eWinTeam = WTT_RED_TEAM;
			else if( m_iRedTeamWinCnt < m_iBlueTeamWinCnt )
				eWinTeam = WTT_BLUE_TEAM;
			else
				eWinTeam = WTT_DRAW;

			if( m_bTournamentRoom )
			{
				// ���� ���� ��忡���� �������� ����.
				if( iBlueUser == 0 )
					eWinTeam = WTT_RED_TEAM;
				else
					eWinTeam = WTT_BLUE_TEAM;
			}
		}
		SendRoundResult( eWinTeam );
	}
}

ModeType CatchRunningManMode::GetModeType() const
{
	return MT_CATCH_RUNNINGMAN;
}

const char* CatchRunningManMode::GetModeINIFileName() const
{
	return "config/catch_runningman_mode.ini";
}

DWORD CatchRunningManMode::GetRunningManDecoIndex()
{
	if( m_RunningManDecoList.empty() ) 
	{
		return 1;         // ������ ó����
	}

	DWORD dwIndex = *m_RunningManDecoList.begin();
	m_RunningManDecoList.erase( m_RunningManDecoList.begin() );
	
	return dwIndex;
}

void CatchRunningManMode::SetRunningManDecoIndex( DWORD dwIndex )
{
	if( dwIndex == 0 )
		return;

	m_RunningManDecoList.push_back( dwIndex );
}

void CatchRunningManMode::GetCharModeInfo( SP2Packet &rkPacket, const ioHashString &rkName, bool bDieCheck )
{
	CatchRecord *pRecord = FindCatchRecord( rkName );
	if( pRecord )
	{
		// ���ڵ� ���� ����
		rkPacket << true;

		// ���׸�
		rkPacket << pRecord->dwRunningManDeco << pRecord->szRunningManName;

		int iKillSize = pRecord->iKillInfoMap.size();
		rkPacket << iKillSize;

		KillDeathInfoMap::iterator iter_k = pRecord->iKillInfoMap.begin();
		while( iter_k != pRecord->iKillInfoMap.end() )
		{
			rkPacket << iter_k->first;
			rkPacket << iter_k->second;

			++iter_k;
		}

		int iDeathSize = pRecord->iDeathInfoMap.size();
		rkPacket << iDeathSize;

		KillDeathInfoMap::iterator iter_d = pRecord->iDeathInfoMap.begin();
		while( iter_d != pRecord->iDeathInfoMap.end() )
		{
			rkPacket << iter_d->first;
			rkPacket << iter_d->second;

			++iter_d;
		}

		if( bDieCheck )
		{
			rkPacket << pRecord->bDieState;
		}
		rkPacket << pRecord->bPrisoner;
		rkPacket << pRecord->bCatchState;
	}
	else
	{
		// ���ڵ� ���� ����
		rkPacket << false;
	}
}

bool CatchRunningManMode::OnModeChangeChar( User *pSend, int iCharArray, bool bWait, int iSelectCharArray, DWORD dwCharChangeIndex )
{
	CatchRecord *pRecord = FindCatchRecord( pSend );
	if( pRecord == NULL )
		return false;

	// ���׸� ġ�� ����
	if( pRecord->dwRunningManDeco == 0 )
	{
		pRecord->dwRunningManDeco = GetRunningManDecoIndex();
		SP2Packet kPacket( STPK_RUNNINGMAN_DECO_SYNC );
		kPacket << pSend->GetPublicID() << pRecord->dwRunningManDeco;
		SendRoomAllUser( kPacket );
	}
	
	return false;            // �뺴 ��ü�� pSend�� ó���ϰ� ���⼭�� ���׸� ġ�� �ε����� ����
}

bool CatchRunningManMode::ProcessTCPPacket( User *pSend, SP2Packet &rkPacket )
{
	if( CatchMode::ProcessTCPPacket( pSend, rkPacket ) )
		return true;

	switch( rkPacket.GetPacketID() )
	{
	case CTPK_RUNNINGMAN_NAME_SYNC:
		OnRunningManNameSync( pSend, rkPacket );
		return true;
	}

	return false;
}

void CatchRunningManMode::OnRunningManNameSync( User *pUser, SP2Packet &rkPacket )
{
	CatchRecord *pRecord = FindCatchRecord( pUser );
	if( pRecord == NULL )
		return;
	
	rkPacket >> pRecord->szRunningManName;

	SP2Packet kPacket( STPK_RUNNINGMAN_NAME_SYNC );
	kPacket << pUser->GetPublicID() << pRecord->szRunningManName;
	SendRoomAllUser( kPacket );
}