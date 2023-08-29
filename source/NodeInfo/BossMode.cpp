#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"

#include "User.h"
#include "Room.h"
#include "BossMode.h"
#include "ioSetItemInfoManager.h"
#include "ioItemInfoManager.h"
#include "ioSetItemInfo.h"
#include "../EtcHelpFunc.h"
#include "../DataBase/LogDBClient.h"

BossMode::BossMode( Room *pCreator ) : Mode( pCreator )
{	
	m_iBossMaxLevel = 0;
	m_SingleTeamPosArray.clear();
}

BossMode::~BossMode()
{
}

void BossMode::LoadINIValue()
{
	Mode::LoadINIValue();

	m_dwCurRoundDuration = m_dwRoundDuration;
}

void BossMode::LoadRoundCtrlValue( ioINILoader &rkLoader )
{
	rkLoader.SetTitle( "info" );
	m_iMaxPlayer = rkLoader.LoadInt( "max_player", MAX_PLAYER );
	if( m_iMaxPlayer > MAX_PLAYER )
		m_iMaxPlayer = MAX_PLAYER;

	rkLoader.SetTitle( "round" );
	m_iMaxRound = 1;

	m_dwRoundDuration   = rkLoader.LoadInt( "round_time", 300000 );

	m_dwReadyStateTime  = rkLoader.LoadInt( "ready_state_time", 4000 );
	m_dwSuddenDeathTime = rkLoader.LoadInt( "sudden_death_time", 60000 );
	m_dwResultStateTime = rkLoader.LoadInt( "result_state_time", 7000 );
	m_dwFinalResultWaitStateTime = rkLoader.LoadInt( "final_result_wait_state_time", 10000 );
	m_dwFinalResultStateTime = rkLoader.LoadInt( "final_result_state_time", 13000 );
	m_dwTournamentRoomFinalResultAddTime = rkLoader.LoadInt( "tournament_room_final_result_time", 10000 );
	m_dwTimeClose       = rkLoader.LoadInt("room_close_time", 10000 );

	m_bUseViewMode = rkLoader.LoadBool( "use_view_mode", false );
	m_dwViewCheckTime = rkLoader.LoadInt( "view_check_time", 30000 );

	m_iAbuseMinDamage = rkLoader.LoadInt( "abuse_min_damage", 1 );

	m_iScoreRate = rkLoader.LoadInt( "win_team_score_rate", 1 );

	m_dwRoomExitTime = rkLoader.LoadInt( "exit_room_time", 5000 );

	m_dwCharLimitCheckTime = rkLoader.LoadInt( "char_limit_check_min", 60000 );

	rkLoader.SetTitle( "bossinfo" );
	m_iBossMaxLevel = rkLoader.LoadInt( "boss_max_level", 10 );

	// ���常ŭ ���� ����.
	m_vRoundHistory.clear();
	for(int i = 0; i < m_iMaxRound;i++)
	{
		RoundHistory rh;
		m_vRoundHistory.push_back( rh );
	}

	SetStartPosArray();
}

void BossMode::SetStartPosArray()
{
	m_SingleTeamPosArray.reserve( MAX_PLAYER );
	for( int i=0; i<MAX_PLAYER; i++ )
	{
		m_SingleTeamPosArray.push_back(i);
	}

	std::random_shuffle( m_SingleTeamPosArray.begin(), m_SingleTeamPosArray.end() );

	m_iBluePosArray = m_SingleTeamPosArray[0];
	m_iRedPosArray  = m_SingleTeamPosArray[1];
}

void BossMode::DestroyMode()
{
	Mode::DestroyMode();
	m_vRecordList.clear();
}

void BossMode::InitObjectGroupList()
{
	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );

	int iSubNum = GetModeSubNum();
	int iGroupNum = GetModeMapNum();
	char szBuf[MAX_PATH], szTitle[MAX_PATH];

	m_vPushStructList.clear();
	m_iPushStructIdx = 0;

	wsprintf( szTitle, "boss%d_object_group%d", iSubNum, iGroupNum );
	rkLoader.SetTitle( szTitle );

	int iPushStructCnt = rkLoader.LoadInt( "push_struct_cnt", 0 );
	m_vPushStructList.reserve( iPushStructCnt );

	for( int i=0; i<iPushStructCnt; i++ )
	{
		PushStruct kPush;
		kPush.m_iIndex = i + 1;

		wsprintf( szTitle, "push_struct%d_num", i+1 );
		kPush.m_iNum = rkLoader.LoadInt( szTitle, 0 );
		
		wsprintf( szTitle, "push_struct%d_pos_x", i+1 );
		kPush.m_CreatePos.x = rkLoader.LoadFloat( szTitle, 0.0f );

		wsprintf( szTitle, "push_struct%d_pos_y", i+1 );
		kPush.m_CreatePos.y = rkLoader.LoadFloat( szTitle, 0.0f );
		
		wsprintf( szTitle, "push_struct%d_pos_z", i+1 );
		kPush.m_CreatePos.z = rkLoader.LoadFloat( szTitle, 0.0f );

		m_iPushStructIdx = kPush.m_iIndex;
		m_vPushStructList.push_back( kPush );
	}

	int iObjectItemCnt = rkLoader.LoadInt( "object_item_cnt", 0 );
	ObjectItemList vObjectItemList;
	vObjectItemList.clear();
	vObjectItemList.reserve( iObjectItemCnt );

	for(int i=0; i<iObjectItemCnt; i++ )
	{
		ObjectItem kObjectItem;
		wsprintf( szTitle, "object_item%d_name", i+1 );
		rkLoader.LoadString( szTitle, "", szBuf, MAX_PATH );
		kObjectItem.m_ObjectItemName = szBuf;
		wsprintf( szTitle, "object_item%d_pos_x", i+1 );
		kObjectItem.m_fPosX = rkLoader.LoadFloat( szTitle, 0.0f );
		wsprintf( szTitle, "object_item%d_pos_z", i+1 );
		kObjectItem.m_fPosZ = rkLoader.LoadFloat( szTitle, 0.0f );

		vObjectItemList.push_back( kObjectItem );
	}

	//Push Struct
	SP2Packet kPushPacket( STPK_PUSHSTRUCT_INFO );
	if( GetPushStructInfo( kPushPacket ) )
	{
		SendRoomAllUser( kPushPacket );
	}

	//Object Item
	ItemVector vItemList;
	int iObjectCnt = vObjectItemList.size();

	for(int i=0; i<iObjectCnt; i++ )
	{
		const ObjectItem &rkObjItem = vObjectItemList[i];

		ioItem *pItem = m_pCreator->CreateItemByName( rkObjItem.m_ObjectItemName );
		if( pItem )
		{
			Vector3 vPos( rkObjItem.m_fPosX, 0.0f, rkObjItem.m_fPosZ );
			pItem->SetItemPos( vPos );
			vItemList.push_back( pItem );
		}
	}

	if( vItemList.empty() )
		return;

	SP2Packet kPacket( STPK_FIELD_ITEM_SUPPLY );
	int iNewItemCnt = vItemList.size();
	kPacket << iNewItemCnt;
	for(int i=0; i<iNewItemCnt; i++ )
	{
		ioItem *pItem = vItemList[i];
		m_pCreator->AddFieldItem( pItem );
		kPacket << pItem->GetItemCode();
		kPacket << pItem->GetItemReinforce();
		kPacket << pItem->GetItemMaleCustom();
		kPacket << pItem->GetItemFemaleCustom();
		kPacket << pItem->GetGameIndex();
		kPacket << pItem->GetItemPos();
		kPacket << pItem->GetOwnerName();
		kPacket << "";
	}

	SendRoomAllUser( kPacket );
}



void BossMode::AddNewRecord( User *pUser )
{
	BossRecord kRecord;
	kRecord.pUser = pUser;
	m_vRecordList.push_back( kRecord );

	UpdateUserRank();
}

void BossMode::RemoveRecord( User *pUser, bool bRoomDestroy )
{
	if(!pUser) return;
	
	int iCharCnt = m_vRecordList.size();
	for( int i=0 ; i<iCharCnt ; i++ )
	{
		if( m_vRecordList[i].pUser == pUser )
		{
			SetModeEndDBLog( &m_vRecordList[i], iCharCnt, LogDBClient::PRT_EXIT_ROOM );
			m_vRecordList.erase( m_vRecordList.begin() + i );
			break;
		}
	}

	UpdateUserRank();

	if( pUser )
	{
		m_KickOutVote.RemoveVoteUserList( pUser->GetPublicID() );
	}
}

void BossMode::ChangeBossUser( User *pDier, const ioHashString &szNextBossUser )
{
	if( !pDier ) return;

	if( pDier->GetTeam() == TEAM_BLUE )
	{		
		// ���� ���� ������ �������� �ʾҴ�.
		if( szNextBossUser.IsEmpty() ) 
		{
			ioHashStringVec vTeamList;
			int iRecordCnt = m_vRecordList.size();
			for( int i=0 ; i<iRecordCnt ; i++ )
			{
				if( m_vRecordList[i].pUser->GetTeam() == TEAM_RED )
				{
					vTeamList.push_back( m_vRecordList[i].pUser->GetPublicID() );
				}
			}

			int iSize = vTeamList.size();
			if( iSize != 0 )
			{
				int r = rand()%iSize;

				BossRecord *pBossRecord = FindBossRecord( vTeamList[r] );
				if( pBossRecord && pBossRecord->pUser )
				{
					// ���� ���� ��Ŷ ����
					pBossRecord->pUser->SetTeam( TEAM_BLUE );

					SP2Packet kPacket( STPK_CHANGE_BOSS );
					kPacket << pBossRecord->pUser->GetPublicID() << pBossRecord->iBossLevel;
					m_pCreator->RoomSendPacketTcp( kPacket );
				}
			}			
		}
		else 
		{
			BossRecord *pBossRecord = FindBossRecord( szNextBossUser );
			if( pBossRecord && pBossRecord->pUser )
			{
				// ���� ���� ��Ŷ ����
				pBossRecord->pUser->SetTeam( TEAM_BLUE );

				SP2Packet kPacket( STPK_CHANGE_BOSS );
				kPacket << pBossRecord->pUser->GetPublicID() << pBossRecord->iBossLevel;
				m_pCreator->RoomSendPacketTcp( kPacket );
			}
		}
		// ���� ������ ������
		if( pDier )
			pDier->SetTeam( TEAM_RED );
		CheckBossToRandomBoss();
	}
}

void BossMode::CheckBossToRandomBoss()
{
	int i = 0;
	int iRecordCnt = m_vRecordList.size();
	for(int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser->GetTeam() == TEAM_BLUE )
		{
			return; // ���� ����
		}
	}

	// ���� ����
	ioHashStringVec vTeamList;
	for(int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser->GetTeam() == TEAM_RED )
		{
			vTeamList.push_back( m_vRecordList[i].pUser->GetPublicID() );
		}
	}

	int iSize = vTeamList.size();
	if( iSize != 0 )
	{
		int r = rand()%iSize;

		BossRecord *pBossRecord = FindBossRecord( vTeamList[r] );
		if( pBossRecord && pBossRecord->pUser )
		{
			// ���� ���� ��Ŷ ����
			pBossRecord->pUser->SetTeam( TEAM_BLUE );

			SP2Packet kPacket( STPK_CHANGE_BOSS );
			kPacket << pBossRecord->pUser->GetPublicID() << pBossRecord->iBossLevel;
			m_pCreator->RoomSendPacketTcp( kPacket );
		}
	}			
}

void BossMode::ProcessReady()
{
	DWORD dwCurTime = TIMEGETTIME();
	if( m_dwStateChangeTime + m_dwReadyStateTime >= dwCurTime )
		return;
	// ���� ���� ������̸� �÷��� ���·� ��ȯ���� �ʴ´�.
	if( m_pCreator->IsRoomEnterUserDelay() )
		return;

	SP2Packet kPacket( STPK_ROUND_START );
	kPacket << m_iCurRound;
	SendRoomPlayUser( kPacket );
	SetModeState( MS_PLAY );

	// ���尡 ���۵� �� ���� ���� Ȯ��
	m_iReadyBlueUserCnt = max( m_iReadyBlueUserCnt, GetTeamUserCnt( TEAM_BLUE ) );
	m_iReadyRedUserCnt  = max( m_iReadyRedUserCnt, GetTeamUserCnt( TEAM_RED ) );

	// ����� ���� ���۵� �� ���� Ȯ��
	if( m_pCreator->GetRoomStyle() == RSTYLE_LADDERBATTLE )
	{
		m_iReadyBlueGuildLevel = max( m_iReadyBlueGuildLevel, m_pCreator->GetLadderTeamLevel( TEAM_BLUE ) );
		m_iReadyRedGuildLevel  = max( m_iReadyRedGuildLevel, m_pCreator->GetLadderTeamLevel( TEAM_RED ) );
	}

	// ������ ���� ���ϸ� ������ �ٽ� �����Ѵ�.
	CheckBossToRandomBoss();

	// ���� ���¿��� ��Ż�� ������ ���� üũ
	CheckUserLeaveEnd();
}

void BossMode::ProcessPlay()
{
	ProcessRevival();
	CheckFieldItemLiveTime();
	CheckItemSupply( m_dwStateChangeTime );
	CheckBallSupply( m_dwStateChangeTime );
	CheckMachineSupply( m_dwStateChangeTime );
	CheckNeedSendPushStruct();
	CheckRoundEnd( true );
	ProcessEvent();
	ProcessBonusAlarm();
}

void BossMode::CheckRoundEnd( bool bProcessCall )
{
	if( !bProcessCall && m_pCreator->GetPlayUserCnt() < 2 )
	{
		m_dwCurRoundDuration = 0;

		WinTeamType eWinTeam = WTT_DRAW;
		SetRoundEndInfo( eWinTeam );
		SendRoundResult( eWinTeam );
		return;
	}

	DWORD dwGapTime = TIMEGETTIME() - m_dwStateChangeTime;

	if( m_dwCurRoundDuration < dwGapTime+1000 )
	{
		if( !m_bZeroHP )
		{
			SP2Packet kPacket( STPK_ZERO_HP );
			kPacket << m_dwSuddenDeathTime;
			SendRoomAllUser( kPacket );
			m_bZeroHP = true;

			m_dwCurRoundDuration = 0;

			m_dwCurSuddenDeathDuration = TIMEGETTIME();			
			m_fSuddenDeathBlueCont	   = 0.0f;
			m_fSuddenDeathRedCont	   = 0.0f;

			// 0�ʰ��Ǹ� �ð��� �����.
			int iRecordCnt = GetRecordCnt();
			for( int i=0 ; i<iRecordCnt ; i++ )
			{
				ModeRecord *pRecord = FindModeRecord( i );
				if( !pRecord ) continue;

				// ��Ȱ ����
				pRecord->dwCurDieTime = 0;
				if( pRecord->pUser )
				{
					if( pRecord->eState != RS_VIEW && pRecord->eState != RS_OBSERVER )
						pRecord->pUser->SetStartTimeLog( TIMEGETTIME() );
				}
			}
		}
	}

	if( m_bZeroHP )
	{
		CheckSuddenDeathEnd();
		
		if( m_bTournamentRoom )
		{
			if( m_fSuddenDeathBlueCont > 0.0f && m_fSuddenDeathRedCont > 0.0f )
			{
				WinTeamType eWinTeam = WTT_DRAW;
				if( m_fSuddenDeathBlueCont > m_fSuddenDeathRedCont )
					eWinTeam = WTT_BLUE_TEAM;
				else
					eWinTeam = WTT_RED_TEAM;
				SetRoundEndInfo( eWinTeam );
				SendRoundResult( eWinTeam );
				return;
			}
		}

		int iBlueUser = 0;
		int iRedUser  = 0;
		int iRecordCnt = m_vRecordList.size();
		for( int i=0 ; i<iRecordCnt ; i++ )
		{
			ModeRecord *pRecord = FindModeRecord( i );
			if( pRecord )
			{
				if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER )
					continue;
				if( pRecord->bDieState ) continue;

				User *pUser = pRecord->pUser;
				if( pUser && pUser->GetTeam() == TEAM_BLUE )
					iBlueUser++;
				else
					iRedUser++;
			}
		}

		if( iBlueUser == 0 || iRedUser == 0 )
		{
			WinTeamType eWinTeam = WTT_DRAW;
			SetRoundEndInfo( eWinTeam );
			SendRoundResult( eWinTeam );
		}
	}
}

void BossMode::SetRoundEndInfo( WinTeamType eWinTeam )
{
	m_CurRoundWinTeam = eWinTeam;
	m_bRoundSetEnd = true;
	m_bCheckContribute = false;
	m_bCheckAwardChoose = false;
	m_bCheckSuddenDeathContribute = false;
	SetModeState( MS_RESULT_WAIT );

	UpdateRoundRecord();

	m_vPushStructList.clear();
	m_vBallStructList.clear();
	m_vMachineStructList.clear();
	m_pCreator->DestroyAllFieldItems();

	// PlayingTime Update
	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( pRecord )
		{
			pRecord->AddPlayingTime();
			pRecord->AddClassPlayingTime();
		}
	}

	ClearObjectItem();
}

void BossMode::UpdateRoundRecord()
{
	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( pRecord )
		{
			if( pRecord->pUser )
			{
				pRecord->pUser->UpdateCharLimitDate();
				pRecord->pUser->UpdateEtcItemTime( __FUNCTION__ );
				pRecord->pUser->DeleteEtcItemPassedDate();
				pRecord->pUser->DeleteExtraItemPassedDate( true );
				pRecord->pUser->DeleteMedalItemPassedDate( true );
				pRecord->pUser->DeleteExMedalSlotPassedDate();
				pRecord->pUser->DeleteCharAwakePassedDate( );
				pRecord->pUser->DeleteCostumePassedDate();
				// �ӽ� : �ð�����
				pRecord->pUser->CheckTimeGrowth();
				pRecord->pUser->DeleteExpiredBonusCash();
			}
		}
	}

	UpdateUserRank();	
}

ModeType BossMode::GetModeType() const
{
	return MT_BOSS;
}

void BossMode::GetCharModeInfo( SP2Packet &rkPacket, const ioHashString &szName, bool bDieCheck )
{
	BossRecord *pRecord = FindBossRecord( szName );
	if( pRecord )
	{
		// ���ڵ� ���� ����
		rkPacket << true;

		int iKillSize = pRecord->iKillInfoMap.size();
		rkPacket << iKillSize;

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_k = pRecord->iKillInfoMap.begin();
		while( iter_k != pRecord->iKillInfoMap.end() )
		{
			rkPacket << iter_k->first;
			rkPacket << iter_k->second;

			++iter_k;
		}
		LOOP_GUARD_CLEAR();

		int iDeathSize = pRecord->iDeathInfoMap.size();
		rkPacket << iDeathSize;

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_d = pRecord->iDeathInfoMap.begin();
		while( iter_d != pRecord->iDeathInfoMap.end() )
		{
			rkPacket << iter_d->first;
			rkPacket << iter_d->second;

			++iter_d;
		}
		LOOP_GUARD_CLEAR();

		if( bDieCheck )
		{
			rkPacket << pRecord->bDieState;
		}
		rkPacket << pRecord->bCatchState;
	}
	else
	{
		// ���ڵ� ���� ����
		rkPacket << false;
	}
}

int BossMode::GetRecordCnt() const
{
	return m_vRecordList.size();
}

const char* BossMode::GetModeINIFileName() const
{
	return "config/bossmode.ini";
}

void BossMode::GetModeInfo( SP2Packet &rkPacket )
{
	rkPacket << GetModeType();
	rkPacket << m_iCurRound;
	rkPacket << m_iMaxRound;

	rkPacket << m_dwRoundDuration;

	int iPosCnt = m_SingleTeamPosArray.size();
	rkPacket << iPosCnt;

	for( int i=0; i < iPosCnt; ++i )
		rkPacket << m_SingleTeamPosArray[i];
}

TeamType BossMode::GetNextTeamType()
{
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "BossMode::GetNextTeamType Call Error!!" );
	return TEAM_NONE;
}



int BossMode::GetUserKickVoteLimit( const ioHashString &szKickUserName )
{
	ModeRecord *pKickRecord = FindModeRecord( szKickUserName );
	if( !pKickRecord || !pKickRecord->pUser )
		return USER_KICK_VOTE_PROPOSAL_ERROR_7;

	// �ο� üũ 
	if( !pKickRecord->pUser->IsObserver() )
	{
		int iPlayUserCount = 0;
		int iRecordCnt = GetRecordCnt();
		for(int i = 0;i < iRecordCnt;i++)
		{
			ModeRecord *pRecord = FindModeRecord( i );
			if( !pRecord || !pRecord->pUser ) continue;
			if( pRecord->pUser->IsObserver() ) continue;
			if( pRecord->pUser->IsStealth() ) continue;

			iPlayUserCount++;
		}
		if( iPlayUserCount <= m_KickOutVote.GetKickVoteUserPool() )
			return USER_KICK_VOTE_PROPOSAL_ERROR_11;
	}

	// ���� or �ð� üũ
	if( IsRoundSetEnd() )
		return USER_KICK_VOTE_PROPOSAL_ERROR_9;

	// �ð� üũ
	DWORD dwGapTime = TIMEGETTIME() - m_dwModeStartTime;
	if( dwGapTime > m_KickOutVote.GetKickVoteRoundTime() )
		return USER_KICK_VOTE_PROPOSAL_ERROR_9;

	return 0;
}

ModeRecord* BossMode::FindModeRecord( const ioHashString &rkName )
{
	if( rkName.IsEmpty() )	return NULL;

	CRASH_GUARD();
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser->GetPublicID() == rkName )
		{
			return &m_vRecordList[i];
		}
	}

	return NULL;
}

ModeRecord* BossMode::FindModeRecord( User *pUser )
{
	if( !pUser )	return NULL;

	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser == pUser )
			return &m_vRecordList[i];
	}

	return NULL;
}

ModeRecord* BossMode::FindModeRecord( int iIdx )
{
	if( COMPARE( iIdx, 0, GetRecordCnt() ) )
		return &m_vRecordList[iIdx];

	return NULL;
}


BossRecord* BossMode::FindBossRecord( const ioHashString &rkName )
{
	return (BossRecord*)FindModeRecord( rkName );
}

BossRecord* BossMode::FindBossRecord( User *pUser )
{
	return (BossRecord*)FindModeRecord( pUser );
}

BossRecord* BossMode::FindBossRecordByUserID( const ioHashString &rkUserID )
{
	if( rkUserID.IsEmpty() )	return NULL;

	CRASH_GUARD();
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser->GetPublicID() == rkUserID )
		{
			return &m_vRecordList[i];
		}
	}

	return NULL;
}

int BossMode::GetCurTeamUserCnt( TeamType eTeam )
{
	int iUserCnt = 0;
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( pRecord )
		{
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER )
				continue;

			User *pUser = pRecord->pUser;

			if( pUser && pUser->GetTeam() == eTeam )
				iUserCnt++;
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BossMode::GetCurTeamUserCnt() - User's ModeRecord is Not Exist(%d).", i );
		}
	}

	return iUserCnt;
}

void BossMode::UpdateUserDieTime( User *pDier )
{
	ModeRecord *pDieRecord = FindModeRecord( pDier );
	if( pDieRecord && pDieRecord->bDieState )
	{
		DWORD dwRevivalGap = (DWORD)GetRevivalGapTime( pDieRecord->iRevivalCnt );
		pDieRecord->dwCurDieTime = TIMEGETTIME();
		pDieRecord->dwRevivalGap = dwRevivalGap;
		pDieRecord->iRevivalCnt++;

		//����Ÿ�Ӷ��� ��Ȱ ����
		if( m_bZeroHP )
			pDieRecord->dwCurDieTime = 0;
	}
}

void BossMode::UpdateDropDieRecord( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{
	ModeRecord *pDieRecord = FindModeRecord( pDier );
	if( !pDieRecord )	return;

	// ���������� Ÿ���� ������ ���� ����
	if( !szAttacker.IsEmpty() && pDier->GetPublicID() != szAttacker )
	{
		TeamType eAttackerTeam = pDieRecord->eLastAttackerTeam;
		ModeRecord *pAttRecord = FindModeRecord( szAttacker );
		if( !pAttRecord )	return;

		float fKillPoint = 0.5f + ( (float)GetKillPoint( eAttackerTeam ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != eAttackerTeam )
		{
			pAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
			pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
		}
		else
		{
			if( pAttRecord->pUser != pDieRecord->pUser )	// team kill
			{
				pAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), -fKillPoint );
				pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
			}
			else
			{
				pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
			}
		}
	}
	else
	{
		pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
	}

	// ���� ���� �������� ���� ����
	if( !szBestAttacker.IsEmpty() && pDier->GetPublicID() != szBestAttacker )
	{
		ModeRecord *pAttRecord = FindModeRecord( szBestAttacker );
		if( !pAttRecord )	return;

		float fKillPoint = 0.5f + ( (float)GetKillPoint( pAttRecord->pUser->GetTeam() ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != pAttRecord->pUser->GetTeam() )
			pAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
	}
}

void BossMode::UpdateUserDieNextProcess( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{
	if( GetState() != Mode::MS_PLAY ) return;

	CheckRoundEnd( true );

	if( !pDier ) return;
	if( m_bZeroHP ) return;

	// ���� ������ ������ �ƴϸ� ���� ������
	if( pDier->GetTeam() == TEAM_RED )
	{
		// ���� ���� ������
		int iRecordCnt = m_vRecordList.size();
		for( int i=0 ; i<iRecordCnt ; i++ )
		{
			BossRecord *pRecord = &m_vRecordList[i];
			if( pRecord && pRecord->pUser )
			{
				if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER )
					continue;

				if( pRecord->pUser->GetTeam() == TEAM_BLUE )
					pRecord->iBossLevel = min( pRecord->iBossLevel + 1, m_iBossMaxLevel );
			}
		}
	}

	ChangeBossUser( pDier, szBestAttacker );
}

void BossMode::CheckUserLeaveEnd()
{
	if( m_bRoundSetEnd )
		return;

	if( m_pCreator->GetPlayUserCnt() < 2 )
		CheckRoundEnd( false );
	else 
	{
		CheckBossToRandomBoss();
	}
}

void BossMode::OnEventSceneEnd( User *pSend, SP2Packet &rkPacket )
{
	bool bRoundChange;
	int iRoomIndex;
	rkPacket >> bRoundChange >> iRoomIndex;
	if( iRoomIndex != m_pCreator->GetRoomIndex() )
		return;

	if( !bRoundChange && !IsZeroHP() )
	{
		if( !pSend->IsObserver() && !pSend->IsStealth() )
		{
			SP2Packet kPacket( STPK_START_SELECT_CHAR );
			kPacket << GetSelectCharTime();
			pSend->SendMessage( kPacket );
		}

		return;
	}

	ModeRecord *pRecord = FindModeRecord( pSend );
	if( !pRecord )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BossMode::OnEventSceneEnd - %s Not Exist Record",
			pSend->GetPublicID().c_str() );
		return;
	}

	//�ε� �ð��� �α׷� ����
	pRecord->CheckLoadingTime();
	pRecord->pUser->EquipDBItemToAllChar();
	SetFirstRevivalTime( pRecord );

	int iModeState;
	switch( m_ModeState )
	{
	case MS_READY:
	case MS_PLAY:
		iModeState = m_ModeState;
		break;
	case MS_RESULT_WAIT:
	case MS_RESULT:
		iModeState = MS_RESULT_WAIT;
		break;
	}

	DWORD dwPastTime = TIMEGETTIME() - m_dwStateChangeTime;
	if( pRecord->pUser->IsObserver() || pRecord->pUser->IsStealth() )
	{
		pRecord->eState = RS_OBSERVER;
		pRecord->pUser->SetStartTimeLog( TIMEGETTIME() );
		SP2Packet kPacket( STPK_ROUND_JOIN_OBSERVER );
		kPacket << pRecord->pUser->GetPublicID();
		kPacket << iModeState;
		kPacket << dwPastTime;
		kPacket << m_dwCurRoundDuration;
		SendRoomAllUser( kPacket );
	}
	else if( m_ModeState == MS_PLAY && IsZeroHP() )        //����Ÿ�� �� ����
	{
		pRecord->eState = RS_VIEW;
		pRecord->pUser->SetStartTimeLog( TIMEGETTIME() );
		SP2Packet kPacket( STPK_ROUND_JOIN_VIEW );
		kPacket << pRecord->pUser->GetPublicID();
		kPacket << iModeState;
		kPacket << dwPastTime;
		kPacket << m_dwCurRoundDuration;
		SendRoomAllUser( kPacket );
	}
	else
	{
		pRecord->eState = RS_PLAY;
		pRecord->StartPlaying();        //( ����X, ����Ÿ��X )
		pRecord->pUser->StartCharLimitDate( GetCharLimitCheckTime(), __FILE__, __LINE__ );

		SP2Packet kPacket( STPK_ROUND_JOIN );
		kPacket << pRecord->pUser->GetPublicID();
		kPacket << iModeState;
		kPacket << dwPastTime;
		kPacket << GetSelectCharTime();
		kPacket << m_dwCurRoundDuration;
		SendRoomAllUser( kPacket );
	}
}