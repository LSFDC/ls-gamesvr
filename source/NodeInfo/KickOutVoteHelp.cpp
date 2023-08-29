

#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "../EtcHelpFunc.h"

#include "User.h"
#include "Mode.h"
#include "KickOutVoteHelp.h"

KickOutVoteHelp::KickOutVoteHelp()
{
	InitVote();
}

KickOutVoteHelp::~KickOutVoteHelp()
{
	InitVote();
}

void KickOutVoteHelp::InitVote()
{
	m_pCreator = NULL;
	m_VoteUserList.clear();
	m_VoteProposalUserList.clear();
	m_dwCurVoteTime = 0;	
	m_dwVoteProcessTime = 0;      
	m_iKickVoteUserPool = 0;      
	m_iKickVoteRoundWin = 0;    
	m_dwKickVoteRoundTime = 0;  
	m_fVoteAdoptionPercent = 0.0f;
}

void KickOutVoteHelp::LoadVoteInfo( Mode *pCreator, ioINILoader &rkLoader )
{
	m_pCreator = pCreator;
	m_VoteUserList.clear();
	m_VoteProposalUserList.clear();

	rkLoader.SetTitle( "vote_info" );
	m_iKickVoteUserPool   = rkLoader.LoadInt( "vote_user_pool", 2 );
	m_iKickVoteRoundWin   = rkLoader.LoadInt( "vote_round_win", 2 );
	m_dwKickVoteRoundTime = rkLoader.LoadInt( "vote_round_time", 120000 );
	m_fVoteAdoptionPercent= rkLoader.LoadFloat( "vote_adoption_per", 0.5f );
	m_dwVoteProcessTime   = rkLoader.LoadInt( "vote_process_time", 20000 );
}

void KickOutVoteHelp::ProcessVote()
{
	if( !IsVoting() ) return;

	if( TIMEGETTIME() - m_dwCurVoteTime > m_dwVoteProcessTime + 2000 )
	{
		// ���� �ð� �ʰ�
		CheckKickVoteResult();
	}
}

TeamType KickOutVoteHelp::GetVoteUserTeam( const ioHashString &rkName )
{
	int iSize = m_VoteUserList.size();
	for(int i = 0;i < iSize;i++)
	{
		KickVoteData &rkData = m_VoteUserList[i];
		if( rkData.m_szName == rkName )
			return rkData.m_eTeam;
	}
	return TEAM_NONE;
}

bool  KickOutVoteHelp::IsVoteProposalUser( const ioHashString &rkName )
{
	int iSize = m_VoteProposalUserList.size();
	for(int i = 0;i < iSize;i++)
	{
		if( m_VoteProposalUserList[i] == rkName )
			return true;
	}
	return false;
}

bool  KickOutVoteHelp::IsVoting()
{
	if( m_VoteUserList.empty() )
		return false;
	if( !m_pCreator )
		return false;
	return true;
}

void  KickOutVoteHelp::StartVote( const ioHashString &rkProposalName, const ioHashString &rkTargetName, const ioHashString &rkReason )
{
	m_NowVoteProposalName = rkProposalName;
	m_NowVoteTargetName   = rkTargetName;	
	m_NowVoteReason		  = rkReason;
	m_dwCurVoteTime       = TIMEGETTIME();	

	m_VoteProposalUserList.push_back( m_NowVoteProposalName );
}

void KickOutVoteHelp::EndVote()
{
	m_NowVoteProposalName.Clear();
	m_NowVoteTargetName.Clear();
	m_NowVoteReason.Clear();
	m_VoteUserList.clear();
	m_dwCurVoteTime = 0;	
}

void  KickOutVoteHelp::InsertVoteUserList( const ioHashString &rkName, TeamType eTeam )
{
	int iSize = m_VoteUserList.size();
	for(int i = 0;i < iSize;i++)
	{
		KickVoteData &rkData = m_VoteUserList[i];
		if( rkData.m_szName == rkName )
		{
			rkData.m_iVoteType = 0;
			rkData.m_eTeam     = eTeam;
			return;
		}
	}

	KickVoteData kData;
	kData.m_szName = rkName;
	kData.m_eTeam  = eTeam;
	m_VoteUserList.push_back( kData );
}

void  KickOutVoteHelp::RemoveVoteUserList( const ioHashString &rkName )
{
	if( !IsVoting() ) return;
	if( !m_pCreator )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "KickOutVoteHelp::RemoveVoteUserList : m_pCreator == NULL" );
		return;
	}

	if( m_NowVoteProposalName == rkName )         // ��ǥ �����ڰ� �� ��Ż.
	{
		// �������� ��ǥ ��ȿ �˸�
		SP2Packet kPacket( STPK_USER_KICK_VOTE );
		kPacket << USER_KICK_VOTE_CANCEL_PROPOSAL << rkName << (int)GetVoteUserTeam( rkName );		
		m_pCreator->SendRoomPlayUser( kPacket, m_NowVoteTargetName, false );       // Ÿ�ٰ� ���������� �˸��� ����.
		//
		EndVote();		
	}
	else if( m_NowVoteTargetName == rkName )      // ���� ����ڰ� �� ��Ż
	{
		// �������� ��ǥ ��ȿ �˸�
		SP2Packet kPacket( STPK_USER_KICK_VOTE );
		kPacket << USER_KICK_VOTE_CANCEL_TARGET << rkName << (int)GetVoteUserTeam( rkName );		
		m_pCreator->SendRoomPlayUser( kPacket, m_NowVoteTargetName, false );       // Ÿ�ٰ� ���������� �˸��� ����.
		//
		EndVote();
	}
	else
	{
		int iSize = m_VoteUserList.size();
		for(int i = 0;i < iSize;i++)
		{
			KickVoteData &rkData = m_VoteUserList[i];
			if( rkData.m_szName == rkName )
			{
				// ��ǥ�� �������� ��� ó�� ��
				if( rkData.m_iVoteType == 0 )
					SetKickVote( rkData.m_szName, USER_KICK_VOTE_ABSTENTION );
				break;
			}
		}
	}
}

void  KickOutVoteHelp::SetKickVote( const ioHashString &rkName, int iVoteType )
{
	if( !IsVoting() ) return;

	int i = 0;
	int iSize = m_VoteUserList.size();
	for(i = 0;i < iSize;i++)
	{
		KickVoteData &rkData = m_VoteUserList[i];
		if( rkData.m_szName == rkName )
		{
			rkData.m_iVoteType = iVoteType;
			break;
		}
	}

	CheckKickVoteResult();	
}

void KickOutVoteHelp::CheckKickVoteResult()
{
	if( !IsVoting() ) return;
	if( !m_pCreator )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "KickOutVoteHelp::CheckKickVoteResult : m_pCreator == NULL" );
		return;
	}

	// �̹� ������ �������� ĵ����
	ModeRecord *pKickRecord = m_pCreator->FindModeRecord( m_NowVoteTargetName );
	if( !pKickRecord || !pKickRecord->pUser )
		return;

	// ��ǥ ���� Ȯ��
	int iVoteUserSize = m_VoteUserList.size();
	int i, iApproveCount, iOppositionCount, iAbstention;
	i = iApproveCount = iOppositionCount = iAbstention = 0;
	for(i = 0;i < iVoteUserSize;i++)
	{
		KickVoteData &rkData = m_VoteUserList[i];
		switch( rkData.m_iVoteType )
		{
		case USER_KICK_VOTE_APPROVE:
			iApproveCount++;
			break;
		case USER_KICK_VOTE_OPPOSITION:
			iOppositionCount++;
			break;
		case USER_KICK_VOTE_ABSTENTION:
			iAbstention++;
			break;
		}
	}

	// ��� ������ ��ǥ�Ͽ���.
	if( iApproveCount + iOppositionCount + iAbstention >= iVoteUserSize )
	{
		bool bApprove = false;
		float fVoteAdoptionUser = (float)iVoteUserSize * m_fVoteAdoptionPercent;
		if( (float)iApproveCount > fVoteAdoptionUser )
		{
			bApprove = true;
			// ���������� ������ �ּ� 1���� �����ؾ��Ѵ�.
			if( m_pCreator->GetModeType() != MT_TRAINING && 
				m_pCreator->GetModeType() != MT_HEADQUARTERS &&
				m_pCreator->GetModeType() != MT_SURVIVAL && 
				m_pCreator->GetModeType() != MT_BOSS &&
				m_pCreator->GetModeType() != MT_MONSTER_SURVIVAL &&
				m_pCreator->GetModeType() != MT_GANGSI &&
				m_pCreator->GetModeType() != MT_DUNGEON_A &&
				m_pCreator->GetModeType() != MT_FIGHT_CLUB &&
				m_pCreator->GetModeType() != MT_SHUFFLE_BONUS && 
				m_pCreator->GetModeType() != MT_RAID &&
				!Help::IsMonsterDungeonMode(m_pCreator->GetModeType()) &&
				!pKickRecord->pUser->IsObserver() )
			{
				int iBlueCount, iRedCount;
				iBlueCount = iRedCount = 0;
				for(i = 0;i < iVoteUserSize;i++)
				{
					KickVoteData &rkData = m_VoteUserList[i];
					if( rkData.m_iVoteType == USER_KICK_VOTE_APPROVE )
					{
						if( rkData.m_eTeam == TEAM_BLUE )
							iBlueCount++;
						else if( rkData.m_eTeam == TEAM_RED )
							iRedCount++;
					}
				}

				if( iBlueCount == 0 || iRedCount == 0 )
					bApprove = false;
			}
		}

		if( bApprove )
		{
			// ��ǥ ���� . ���� ����
			SP2Packet kPacket( STPK_USER_KICK_VOTE );
			kPacket << USER_KICK_VOTE_ADOPTION << m_NowVoteTargetName;
			m_pCreator->SendRoomPlayUser( kPacket, m_NowVoteTargetName, true );

			// ���� ����
			pKickRecord->pUser->UserVoteRoomKickOut( m_NowVoteReason );

			// ���� ó���� ����
			EndVote();
		}
		else
		{
			// ��ǥ �ΰ�
			SP2Packet kPacket( STPK_USER_KICK_VOTE );
			kPacket << USER_KICK_VOTE_REJECTION << m_NowVoteTargetName;
			m_pCreator->SendRoomPlayUser( kPacket, m_NowVoteTargetName, true );
			EndVote();
		}
	}
}