#include "stdafx.h"

#include "../MainProcess.h"


#include "Room.h"
#include "RoomNodeManager.h"
#include "ioEventUserNode.h"
#include "ioEtcItemManager.h"
#include "ioItemInfoManager.h"
#include "LadderTeamManager.h"
#include "ioPresentHelper.h"
#include "ioExerciseCharIndexManager.h"
#include "ioMonsterMapLoadMgr.h"
#include "MonsterSurvivalMode.h"
#include "MonsterSurvivalModeHelp.h"
#include "MissionManager.h"
#include "ModeItemCrator.h"

#include "../EtcHelpFunc.h"
#include "../DataBase/LogDBClient.h"

#include "TowerDefMode.h"
#include "RaidMode.h"

#define RAID_TREASURECARD_MAX 4

const int C_MILISEC_TO_SECOND = 1000;
const int C_PCROOM_ADD_TREASURE_CARD_CNT = 1;


RaidMode::RaidMode( Room *pCreator, ModeType eModeType ) : CTowerDefMode( pCreator, eModeType )
{
	m_eRaidPhaseType = RAID_PHASE_TYPE_NONE;

	m_dwCurHighTurn = 0;
	m_dwCurLowTurn = 0;
	m_dwDecreaseRaidCoin = 0;
	m_dwBossCntTime = 0;
	m_dwBossCntOffset = 0;
	m_dwHunterCoinWaitTime = 0;
	m_dwPhaseStartTime = 0;
	m_iDropZoneUserHunterCoin = 0;

}

RaidMode::~RaidMode()
{
}

void RaidMode::LoadINIValue()
{
	// ���ʱ�ȭ
	m_iCurHunterCoinRegenPos = 0;
	m_iHunterCoinCount = 0;	
	m_dwPhaseStartTime = 0;

	Mode::LoadINIValue();


	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );
	LoadINIHunterCoinInfo(rkLoader);

	rkLoader.SetTitle( "info" );

	m_dwBossCntTime = (DWORD)rkLoader.LoadInt( "boss_cnt_sec", 0 );
	m_dwBossCntOffset = (DWORD)rkLoader.LoadInt( "boss_cnt_offset_sec", 1 );
	m_dwMaxContributeAmp = (DWORD)rkLoader.LoadInt( "max_contribute_amplify", 0 );

	m_UserResultPieceInfoMap.clear();
}

ModeType RaidMode::GetModeType() const
{
	return MT_RAID;
}

const char* RaidMode::GetModeINIFileName() const
{
	return "config/raidmode.ini";
}

void RaidMode::InitObjectGroupList()
{
	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );

	int iSubNum = GetModeSubNum();
	int iGroupNum = GetModeMapNum();
	char szBuf[MAX_PATH], szTitle[MAX_PATH];

	m_vPushStructList.clear();
	m_iPushStructIdx = 0;

	wsprintf( szTitle, "raid%d_object_group%d", iSubNum, iGroupNum );


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
	PACKET_GUARD_VOID(kPacket.Write(iNewItemCnt));

	for(int i=0; i<iNewItemCnt; i++ )
	{
		ioItem *pItem = vItemList[i];
		m_pCreator->AddFieldItem( pItem );
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetItemCode()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetItemReinforce()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetItemMaleCustom()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetItemFemaleCustom()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetGameIndex()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetItemPos()));
		PACKET_GUARD_VOID(kPacket.Write(pItem->GetOwnerName()));
		PACKET_GUARD_VOID(kPacket.Write(""));
	}

	SendRoomAllUser( kPacket );
}

void RaidMode::ProcessPlay()
{
	Mode::ProcessRevival();
	// ���̵�� ���μ���
	ProcessPlayRaid();
	CheckRoundTimePing();
	CheckFieldItemLiveTime();
	CheckItemSupply( m_dwStateChangeTime );
	CheckBallSupply( m_dwStateChangeTime );
	CheckMachineSupply( m_dwStateChangeTime );
	ProcessEvent();
	ProcessBonusAlarm();
}

void RaidMode::UpdateUserDieTime( User *pDier )
{
	ModeRecord *pDieRecord = FindModeRecord( pDier );
	if( pDieRecord && pDieRecord->bDieState )
	{
		DWORD dwRevivalGap = (DWORD)GetRevivalGapTime( pDieRecord->iRevivalCnt );
		pDieRecord->dwCurDieTime = TIMEGETTIME();
		pDieRecord->dwRevivalGap = dwRevivalGap;
		pDieRecord->iRevivalCnt++;
	}
}


void RaidMode::CheckRoundEnd( bool bProcessCall )
{
	int iPrisonerAndDieUser = GetCurPrisonerAndDieUserCnt( TEAM_BLUE );
	WinTeamType eWinTeam = WTT_BLUE_TEAM;
	if( GetCurTeamUserCnt( TEAM_BLUE ) == iPrisonerAndDieUser )    
	{	
		eWinTeam = WTT_BLUE_TEAM;
	}
	else if( GetLiveMonsterCount() == 0 )
	{
		eWinTeam = WTT_BLUE_TEAM;
	}

	if( bProcessCall && eWinTeam == WTT_DRAW )
		return;

	if( GetState() != MS_PLAY && GetState() != MS_READY )
	{
		eWinTeam = WTT_DRAW;
		return;
	}
	else if( GetCurTeamUserCnt( TEAM_BLUE ) == iPrisonerAndDieUser )    
		eWinTeam = WTT_BLUE_TEAM;
	else if( GetLiveMonsterCount() == 0 )
		eWinTeam = WTT_BLUE_TEAM;

	SetRoundEndInfo( eWinTeam );
	SendRoundResult( eWinTeam );
}

void RaidMode::SetRoundEndInfo( WinTeamType eWinTeam )
{
	m_CurRoundWinTeam = eWinTeam;

	m_bRoundSetEnd = true;
	m_bCheckContribute = false;
	m_bCheckAwardChoose = true;		// Ư����, ���� ���� ������ ������ ���� �ʴ´�.
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

			if( pRecord->pUser && pRecord->pUser->GetStartTimeLog() > 0 )
			{
				if( pRecord->eState != RS_VIEW && pRecord->eState != RS_OBSERVER )
					pRecord->AddDeathTime( TIMEGETTIME() - pRecord->pUser->GetStartTimeLog() );
				else
					g_LogDBClient.OnInsertTime( pRecord->pUser, LogDBClient::TT_VIEW );
				pRecord->pUser->SetStartTimeLog(0);
			}
		}
	}

	int HistorySize = m_vRoundHistory.size();
	if( m_iCurRound-1 > HistorySize )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::SetRoundEndInfo() m_iCurRound != m_vRoundHistory.size()" );
	}
	else
	{
		RoundHistory rh;
		if( eWinTeam == TEAM_RED )
		{
			rh.iBluePoint = 0;
			rh.iRedPoint = 1;
		}
		else if( eWinTeam == TEAM_BLUE )
		{
			rh.iBluePoint = 1;
			rh.iRedPoint = 0;
		}
		m_vRoundHistory.push_back( rh );
	}
}


void RaidMode::LoadMapINIValue()
{
	char szFileName[MAX_PATH] = "";

	sprintf_s( szFileName, "config/sp2_raid_mode%d_map.ini", GetModeSubNum() );


	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( szFileName );

	int j = 0;
	char szKey[MAX_PATH] = "";

	rkLoader.SetTitle( "treasure_card" );
	m_iClearMVPCardCount = rkLoader.LoadInt( "treasure_mvp_card", 0 );		
	int iMaxClearCard = rkLoader.LoadInt( "max_clear_card", 0 );
	for(j = 0;j < iMaxClearCard;j++)
	{
		sprintf_s( szKey, "clear_card_%d", j + 1 );			
		m_vClearCardCount.push_back( rkLoader.LoadInt( szKey, 0 ) );
	}

	int iMaxTreasureCardIndex= rkLoader.LoadInt( "max_treasure_card_index", 0 );
	for(j = 0;j < iMaxTreasureCardIndex;j++)
	{
		TreasureCardIndex kCardIndex;
		sprintf_s( szKey, "treasure_card_second_%d", j + 1 );
		kCardIndex.m_dwPlaySecond = rkLoader.LoadInt( szKey, 0 );

		sprintf_s( szKey, "treasure_card_index_%d", j + 1 );
		kCardIndex.m_dwCardIndex = rkLoader.LoadInt( szKey, 0 );

		m_TreasureCardIndexList.push_back( kCardIndex );
	}

	rkLoader.SetTitle( "info" );
	m_dwAbusePlayTime = rkLoader.LoadInt( "abuse_play_time", 0 );
	m_dwPresentPlayRandValue = rkLoader.LoadInt( "present_play_rand_value", 0 );
	m_dwSpecialAwardMaxPoint = rkLoader.LoadInt( "special_award_max_point", 0 );
	m_dwTreasureCardIndex    = rkLoader.LoadInt( "treasure_card_index", 0 );

	m_dwExpUpTime		= rkLoader.LoadInt( "Exp_Up_Duration", 10000 );  
	m_dwExpUpAmount		= rkLoader.LoadInt( "Exp_Up_Amount", 1000 );

	m_fDefeatRatio = rkLoader.LoadFloat("Defeat_Handicap", 0.25f);


	// ���̵����� ����.
	m_dwDecreaseRaidCoin = rkLoader.LoadInt( "decrease_raid_coin_cnt", 0 );

	// �������� �״�� ������ ���� �����ų� ���� ����.
	int iNpcNameCount = 0;
	int iHighTurn, iLowTurn;
	int iMaxHighTurn = rkLoader.LoadInt( "max_high_turn", 0 );
	for(iHighTurn = 0;iHighTurn < iMaxHighTurn;iHighTurn++)
	{
		char szKey[MAX_PATH];
		sprintf_s( szKey, "high_turn%d", iHighTurn + 1 );
		rkLoader.SetTitle( szKey );
		HighTurnData kHighTurn;
		int iMaxLowTurn = rkLoader.LoadInt( "max_low_turn", 0 );

		// �� �ð�
		DWORD dwPlusTurnTime   = rkLoader.LoadInt( "plus_turn_time", 0 );
		DWORD dwNormalTurnTime = rkLoader.LoadInt( "normal_turn_time", 0 );
		DWORD dwBossTurnTime   = rkLoader.LoadInt( "boss_turn_time", 0 );

		// ��Ȱ �ð�
		DWORD dwTurnRevivalTime = rkLoader.LoadInt( "boss_turn_revival_time", 0 );
		DWORD dwTurnRevivalDownTime = rkLoader.LoadInt( "boss_turn_revival_down_time", 0 );

		// ������ ����
		char szBossTurn[MAX_PATH] = "";
		rkLoader.LoadString( "boss_turn_list", "", szBossTurn, MAX_PATH );		
		IntVec vTurnList;
		for(iLowTurn = 0;iLowTurn < iMaxLowTurn;iLowTurn++)
		{
			sprintf_s( szKey, "turn%d_idx", iLowTurn + 1 );
			vTurnList.push_back( rkLoader.LoadInt( szKey, 0 ) );
		}

		//std::random_shuffle( vTurnList.begin(), vTurnList.end() );      //LowTurn���� ���´�. = PvE������ ���� �ʴ´�.

		iMaxLowTurn = min( (int)vTurnList.size(), rkLoader.LoadInt( "max_play_turn", iMaxLowTurn ) );
		for(iLowTurn = 0;iLowTurn < iMaxLowTurn;iLowTurn++)
		{
			TurnData kTurn;
			kTurn.m_bBossTurn = Help::IsStringCheck( szBossTurn, iLowTurn + 1, '.' );
			if( g_MonsterMapLoadMgr.GetTurnData( vTurnList[iLowTurn], kTurn, iHighTurn, iLowTurn, iNpcNameCount, false ) )
			{
				if( kTurn.m_bBossTurn )
				{
					kTurn.m_dwTurnTime = dwBossTurnTime + ( dwPlusTurnTime * iLowTurn );   // ���� �� �ð�
					kTurn.m_dwRevivalTime = dwTurnRevivalTime;          // ��Ȱ �ð� 
					dwTurnRevivalTime -= dwTurnRevivalDownTime;
				}
				else
				{
					kTurn.m_dwTurnTime = dwNormalTurnTime + ( dwPlusTurnTime * iLowTurn );   // �Ϲ� �� �ð�
				}
				kHighTurn.m_TurnData.push_back( kTurn );
			}						
		}
		kHighTurn.m_dwCurrentTurnIdx = 0;
		m_HighTurnList.push_back( kHighTurn );
	}

	// ���� ���� ��ġ
	int i = 0;
	rkLoader.SetTitle( "start_pos" );
	m_RandomStartPos.clear();
	int iMaxRandomStartPos = rkLoader.LoadInt( "max_rand_pos", 0 );
	for(i = 0;i < iMaxRandomStartPos;i++)
	{
		RandomStartPos kRandomPos;
		char szKey[MAX_PATH];

		sprintf_s( szKey, "start%d_pos_x", i + 1 );		
		kRandomPos.m_fStartXPos = rkLoader.LoadFloat( szKey, 0.0f );
		sprintf_s( szKey, "start%d_pos_z", i + 1 );		
		kRandomPos.m_fStartZPos = rkLoader.LoadFloat( szKey, 0.0f );

		sprintf_s( szKey, "start%d_range_x", i + 1 );		
		kRandomPos.m_fStartXRange = rkLoader.LoadFloat( szKey, 0.0f );
		sprintf_s( szKey, "start%d_range_z", i + 1 );		
		kRandomPos.m_fStartZRange = rkLoader.LoadFloat( szKey, 0.0f );

		m_RandomStartPos.push_back( kRandomPos );
	}


	// �������� ���� ��ġ
	LoadMapINIHunterCoinPosInfo(rkLoader);

	m_nBlueLair = 0;

	// ������ �ʱ�ȭ
	m_dwCurHighTurn = 0;
	m_dwCurLowTurn = 0;

}

bool RaidMode::CheckAllDieMonster()
{
	if( m_bRoundSetEnd ) 
		return true;

	// ���Ͱ� ��� �׾���?

	TurnData * pTurn = GetTurnData(m_dwCurHighTurn, m_dwCurLowTurn);

	if(pTurn)
	{
		DWORD monSize = pTurn->m_vMonsterList.size();

		// �ѳ��̶� ���׾�����..
		// �׷� ������ üũ onspawn���� �����ϸ� �׷캸������ ������ 0��
		for(DWORD i = 0; i < monSize; ++i)
		{
			MonsterRecord & rkMonster = pTurn->m_vMonsterList[i];

			if( rkMonster.eTeam == TEAM_RED && rkMonster.bGroupBoss &&
				rkMonster.eState != RS_DIE)
				return false;
		}

		return true;

	}
	else
	{
		// �ش����� �̻��ϴϱ� �� �� �����ɷ� ó��.
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::CheckAllDieMonster(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurHighTurn, m_dwCurLowTurn );
		return true;
	}
}


void RaidMode::ProcessResultWait()
{
	// �û�� ����
	if( m_bRoundSetEnd )
	{
		if( m_bCheckContribute && m_bCheckAllRecvDamageList )
		{
			if( m_dwStateChangeTime + m_dwFinalResultWaitStateTime < TIMEGETTIME() )
			{
				FinalRoundProcess();
				SetModeState( MS_RESULT );
			}
		}
		else
		{
			if( m_dwStateChangeTime + 2000 < TIMEGETTIME() )
			{
				FinalRoundProcess();
				SetModeState( MS_RESULT );
			}
		}
	}
	else
	{
		SetModeState( MS_RESULT );
	}
}

void RaidMode::ProcessResult()
{
	// �û�� ����
	DWORD dwResultDuration = m_dwResultStateTime;
	if( m_bRoundSetEnd )
	{
		dwResultDuration = m_dwFinalResultStateTime;
	}

	if( m_dwStateChangeTime + dwResultDuration > TIMEGETTIME() )
		return;

	if( m_bRoundSetEnd )
	{
		if( ExecuteReservedExit() )
			return;	// true�� ���ϵ� ���� ��� ������ �������� ����
	}

	if( !m_bRoundSetEnd )
	{
		m_iCurRound++;                 // ��Ʈ�� ������� ���� ���¿����� ���带 ������Ų��.
		RestartMode();
	}
	else
	{
		m_bRequestDestroy = true;		

		// log
		int iRecordCnt = GetRecordCnt();
		for(int i = 0;i < iRecordCnt;i++ )       
		{
			ModeRecord *pRecord = FindModeRecord( i );
			SetModeEndDBLog( pRecord, iRecordCnt, LogDBClient::PRT_END_SET );
		}
		//
	}
}

int RaidMode::AddPickCardCount( User *pUser )
{
	int iAddCount = 0;
	// �̺�Ʈ ���� ó�� ���� �������� ���� �ϴ� �Ǿ����̸� 1�� �߰�
	if( pUser->IsPCRoomAuthority() )
	{
		iAddCount = C_PCROOM_ADD_TREASURE_CARD_CNT;
	}


	//EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
	//if( pUser->IsPCRoomAuthority() )
	//{
	//	MonsterDungeonEventUserNode *pEventNode = static_cast<MonsterDungeonEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PCROOM_MONSTER_DUNGEON ) );
	//	if( pEventNode )
	//	{
	//		iAddCount = pEventNode->GetAddCount( pUser );
	//	}
	//}
	//else
	//{
	//	MonsterDungeonEventUserNode *pEventNode = static_cast<MonsterDungeonEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_MONSTER_DUNGEON ) );
	//	if( pEventNode )
	//	{
	//		iAddCount = pEventNode->GetAddCount( pUser );
	//	}
	//}

	return iAddCount;
}

void RaidMode::OnLastPlayRecordInfo( User *pUser, SP2Packet &rkPacket )
{
	if( m_bCheckContribute ) return;
	if( !pUser )	return;

	m_bCheckContribute = true;

	int i = 0;
	int iCharCnt = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iCharCnt) );
	MAX_GUARD(iCharCnt, 50);

	for(i = 0;i < iCharCnt;i++)
	{
		ioHashString szName;
		int iContribute = 0, iUniqueTotalKill = 0, iUniqueTotalDeath = 0, iVictories = 0;
		PACKET_GUARD_VOID( rkPacket.Read(szName) );
		PACKET_GUARD_VOID( rkPacket.Read(iContribute) );
		PACKET_GUARD_VOID( rkPacket.Read(iUniqueTotalKill) );
		PACKET_GUARD_VOID( rkPacket.Read(iUniqueTotalDeath) );
		PACKET_GUARD_VOID( rkPacket.Read(iVictories) );

		ModeRecord *pRecord = FindModeRecord( szName );
		if( pRecord )
		{
			pRecord->iContribute	   = iContribute;
			pRecord->iUniqueTotalKill  = iUniqueTotalKill;
			pRecord->iUniqueTotalDeath = iUniqueTotalDeath;
			pRecord->iVictories		   = iVictories;
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::OnLastContributeInfo Recv: %s - %d - %d - %d - %d", szName.c_str(), iContribute, iUniqueTotalKill, iUniqueTotalDeath, iVictories );

			// error check
			if( iContribute == 0 )
			{
				if( iUniqueTotalKill != 0 || iUniqueTotalDeath != 0 )
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "RaidMode::OnLastContributeInfo Error: %s - %d - %d - %d - %d", szName.c_str(), iContribute, iUniqueTotalKill, iUniqueTotalDeath, iVictories );
			}
		}
	}

	int iRecordCnt		= GetRecordCnt();
	int iMaxContribute	= 0;
	for(i = 0;i < iRecordCnt;i++)
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord ) continue;
		if( !pRecord->pUser ) continue;
		iMaxContribute += pRecord->iContribute;
	}

	// ������ ���� ����
	int iOb = 0;
	for(i = 0;i < iRecordCnt;i++)
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord ) continue;
		if( !pRecord->pUser ) continue;
		if( pRecord->pUser->IsObserver() || pRecord->pUser->IsStealth() )
			iOb++;
	}

	if( iMaxContribute > 0 )
	{
		for(i = 0;i < iRecordCnt;i++)
		{
			ModeRecord *pRecord = FindModeRecord( i );
			if( !pRecord ) continue;
			if( !pRecord->pUser ) continue;

			// �ִ� �⿩�� ��� �߰� ��.
			DWORD contributeAmp = iRecordCnt - iOb;
			if(m_dwMaxContributeAmp > 0)
			{
				contributeAmp = min(contributeAmp, m_dwMaxContributeAmp);
			}

			pRecord->fContributePer = (float)( contributeAmp ) * ((float)pRecord->iContribute / iMaxContribute);
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::OnLastContributeInfo Per: %s - %.2f", pRecord->pUser->GetPublicID().c_str(), pRecord->fContributePer );
		}	
	}	

	//	CreateTreasureCardIndex( m_dwAllTurnPlayTime );

	UpdateUserRank();

	int nMVPCard = 0;
	// MVP
	if( IsBlueWin( m_CurRoundWinTeam ) )
	{
		int nBlueTeam = GetTeamUserCnt(TEAM_BLUE);

		for(i = 0;i < iRecordCnt;i++)
		{
			MonsterSurvivalRecord *pRecord = static_cast< MonsterSurvivalRecord * >( FindModeRecord( i ) );
			if( !pRecord ) continue;
			if( !pRecord->pUser ) continue;
			if( pRecord->pUser->IsObserver() || pRecord->pUser->IsStealth() ) continue;

			if( COMPARE( pRecord->iCurRank - 1, 0, (int)m_vClearCardCount.size() ) )
			{
				int nIdx = pRecord->iCurRank - 1 + (GetMaxPlayer() - nBlueTeam);

				nIdx = min((int)m_vClearCardCount.size() - 1, nIdx );

				int iAddCardCount = AddPickCardCount( pRecord->pUser );
				pRecord->iTreasureCardCount = m_vClearCardCount[nIdx]+iAddCardCount;

				if(pRecord->iTreasureCardCount >= RAID_TREASURECARD_MAX)
					pRecord->iTreasureCardCount = RAID_TREASURECARD_MAX;

				LOG.PrintTimeAndLog(LOG_TEST_LEVEL, "%s TreasureCard %d ", pRecord->pUser->GetPublicID().c_str(), pRecord->iTreasureCardCount);
			}

			if( pRecord->iCurRank == 1 )
			{
				// ���� ����!!
				if( nBlueTeam > 2)
				{
					nMVPCard = m_iClearMVPCardCount;
					pRecord->iTreasureCardCount += m_iClearMVPCardCount;

					if(pRecord->iTreasureCardCount >= RAID_TREASURECARD_MAX)
						pRecord->iTreasureCardCount = RAID_TREASURECARD_MAX;
				}
			}
		}	
	}

	// ��ü ������ �⿩���� ų���������� ��ü �������� �����Ѵ�.
	SP2Packet kPacket( STPK_FINAL_RESULT_USER_INFO );
	PACKET_GUARD_VOID( kPacket.Write( nMVPCard ) );
	PACKET_GUARD_VOID( kPacket.Write( iRecordCnt ) );

	for(i = 0;i < iRecordCnt;i++)
	{
		MonsterSurvivalRecord *pRecord = static_cast< MonsterSurvivalRecord * >( FindModeRecord( i ) );
		if( !pRecord ) continue;
		if( !pRecord->pUser ) continue;

		int iVictories = pRecord->iVictories;
		if( pRecord->pUser->IsLadderTeam() )
			pRecord->pUser->GetMyVictories();

		bool bAbuseUser = IsAbuseUser( i );
		if( bAbuseUser )
		{
			pRecord->iTreasureCardCount = 0;
		}

		PACKET_GUARD_VOID( kPacket.Write(pRecord->pUser->GetPublicID()) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iCurRank) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iContribute) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iUniqueTotalKill) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iUniqueTotalDeath) );
		PACKET_GUARD_VOID( kPacket.Write(iVictories) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->pUser->GetKillDeathLevel()) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->pUser->IsSafetyLevel()) );

		// ���� �� �ð� �÷����ߴ� �뺴�� ����(ġ�� ����)
		int iClassType = pRecord->GetHighPlayingClass();
		PACKET_GUARD_VOID( kPacket.Write(iClassType) );
		PACKET_GUARD_VOID( kPacket.Write(pRecord->pUser->GetCharIndex(pRecord->pUser->GetCharArrayByClass(iClassType))) );


		// ������
		// ���߿� ������.
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s user = %d, cardCnt = %d, abuse = %d", __FUNCTION__, pRecord->pUser->GetUserIndex(), pRecord->iTreasureCardCount, bAbuseUser);
		// ���� ���� Ŭ�� 
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iTreasureCardCount) );

		g_UserNodeManager.UpdateUserSync( pRecord->pUser );      //���� ���� ����
	}
	m_pCreator->RoomSendPacketTcp( kPacket );
}

void RaidMode::ProcessReady()
{
	DWORD dwCurTime = TIMEGETTIME();
	if( m_dwStateChangeTime + m_dwReadyStateTime >= dwCurTime )
		return;
	// ���� ���� ������̸� �÷��� ���·� ��ȯ���� �ʴ´�.
	if( m_pCreator->IsRoomEnterUserDelay() )
		return;

	SP2Packet kPacket( STPK_ROUND_START );
	PACKET_GUARD_VOID(kPacket.Write(m_iCurRound));
	SendRoomPlayUser( kPacket );
	SetModeState( MS_PLAY );

	// ���尡 ���۵� �� ���� ���� Ȯ��
	m_iReadyBlueUserCnt = max( m_iReadyBlueUserCnt, GetTeamUserCnt( TEAM_BLUE ) );
	m_iReadyRedUserCnt  = max( m_iReadyRedUserCnt, GetTeamUserCnt( TEAM_RED ) );

	// ���� ���¿��� ��Ż�� ������ ���� üũ
	CheckUserLeaveEnd();
	 
	// ������ �ʱ�ȭ
	m_dwCurHighTurn = 0;
	m_dwCurLowTurn = 0;

	// �Ͻ��� ��Ŷ����
	FirstTurnStart();

	// ���̵� ���� ����.
	DecreaseRaidCoin();

	// ���� ����
	SetRaidPhaseType(RAID_PHASE_TYPE_START_SPAWN);

	//StartTurn( 0, 0 );
}

void RaidMode::FirstTurnStart()
{
	// ���������� �ʿ� ���� �������� �ϴ� ��Ŷ�� ������.
	DWORD dwHighTurnIdx = 0;
	if( !COMPARE( dwHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::StartTurn(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), dwHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	DWORD dwLowTurnIndex = 0;
	HighTurnData &rkHighTurn = m_HighTurnList[dwHighTurnIdx];
	if( !COMPARE( dwLowTurnIndex, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::StartTurn(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), dwLowTurnIndex, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[dwLowTurnIndex];

	SP2Packet kPacket( STPK_TURN_START );
	PACKET_GUARD_VOID(kPacket.Write(rkTurn.m_bBossTurn));
	PACKET_GUARD_VOID(kPacket.Write(GetCurrentAllLowTurn()));
	PACKET_GUARD_VOID(kPacket.Write(GetMaxAllLowTurn()));
	PACKET_GUARD_VOID(kPacket.Write(rkTurn.m_dwHelpIndex));
	PACKET_GUARD_VOID(kPacket.Write(rkTurn.m_dwTurnTime));
	PACKET_GUARD_VOID(kPacket.Write(m_dwCurrentHighTurnIdx));
	PACKET_GUARD_VOID(kPacket.Write(rkHighTurn.m_dwCurrentTurnIdx));
	PACKET_GUARD_VOID(kPacket.Write(rkTurn.m_dwReduceNpcCreateTime));


	int i = 0;
	// ��� NPC ����
	MonsterRecordList vDeathRecord;	
	int iDeathNPCCount = 0;
	PACKET_GUARD_VOID(kPacket.Write(iDeathNPCCount));

	// ���� ���� ���� ����	
	int iMaxNPC = 0;
	PACKET_GUARD_VOID(kPacket.Write(iMaxNPC));
	SendRoomAllUser( kPacket );

	// ������ ���� ���� ����
	for(i = 0;i < GetRecordCnt();i++)
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord*>(FindModeRecord( i ));
		if( pRecord )
		{
			if( pRecord->pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
			if( pRecord->pUser->GetTeam() != TEAM_BLUE ) continue;

			pRecord->bClientViewState = false;
		}
	}

	rkTurn.m_bFirstTurn    = false;
	rkTurn.m_bTimeLimitEnd = rkTurn.m_bMonsterAllDie = false;
	m_dwRoundDuration = m_dwCurRoundDuration = rkTurn.m_dwTurnTime;
	m_dwStateChangeTime    = TIMEGETTIME();

	// �� ���۽� ��ȯ�� / �ʵ� ������ ����
	m_vPushStructList.clear();
	m_vBallStructList.clear();
	m_vMachineStructList.clear();
	m_pCreator->DestroyAllFieldItems();
}

void RaidMode::DecreaseRaidCoin()
{
	for(int i = 0;i < GetRecordCnt();i++)
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord*>(FindModeRecord( i ));
		if( pRecord )
		{
			if( pRecord->pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
			if( pRecord->pUser->GetTeam() != TEAM_BLUE ) continue;

			// TODO!!!!!!!!!
			// ���̵����� ���̱� + ��Ŷ ����
			// ���� Ȯ���� 
			// ���� ���̸� ¥����.
			int iRaidCoinCnt = pRecord->pUser->GetRaidTicket();
			if(iRaidCoinCnt < (int)m_dwDecreaseRaidCoin)
			{
				// ���� Ȯ���� 
				// ���� ���̸� ¥����.
				// ���̵����� Ÿ�� �߰�����.
				ExitRoom(pRecord->pUser, true, EXIT_ROOM_RAID_COIN_LACK, 0);
				LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] : %s RoomExit By RaidCoin", __FUNCTION__);
				
			}
			else
			{
				int iDecreaseRaidCoin = -(int)m_dwDecreaseRaidCoin;
				pRecord->pUser->AddEtcRaidTicket(iDecreaseRaidCoin);

				// ���� ���μ� ��ũ
				SP2Packet kPacket( STPK_RAID_COIN_INFO );
				byte byType = RAID_COIN_INFO_USE;
				int iTicketCount = pRecord->pUser->GetRaidTicket();
				PACKET_GUARD_VOID(kPacket.Write(byType));
				PACKET_GUARD_VOID(kPacket.Write(iTicketCount));
				pRecord->pUser->SendMessage(kPacket);
			}

		}
	}

}

bool RaidMode::WaitBossCnt()
{
	DWORD dwCurTime = TIMEGETTIME();
	DWORD dwWaitTime = m_dwPhaseStartTime + ((m_dwBossCntTime + m_dwBossCntOffset) * C_MILISEC_TO_SECOND);
	if(dwCurTime > dwWaitTime)
		return true;
	else
		return false;
}

bool RaidMode::WaitCoinTime()
{
	DWORD dwCurTime = TIMEGETTIME();
	DWORD dwWaitTime = m_dwHunterCoinWaitTime + m_dwPhaseStartTime;
	if(dwCurTime > dwWaitTime)
		return true;
	else
		return false;

}

void RaidMode::SetRaidPhaseType( RAID_PHASE_TYPE val )
{
	m_eRaidPhaseType = val;
	m_dwPhaseStartTime = TIMEGETTIME();
}

void RaidMode::StartMonsterSpawn( DWORD dwHighTurnIdx, DWORD dwLowTurnIndex )
{
	// �������� ���� ������ �� ��� �Ѵ�.

	// ������ ��ȿ���˻��ϱ�.
	if( !COMPARE( dwHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::StartTurn(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), dwHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}
	m_dwCurHighTurn = dwHighTurnIdx;
	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurHighTurn];


	if( !COMPARE( dwLowTurnIndex, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::StartTurn(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), dwLowTurnIndex, (int)rkHighTurn.m_TurnData.size() );
		return;
	}
	// ������ ����.
	m_dwCurLowTurn = dwLowTurnIndex;
	rkHighTurn.m_dwCurrentTurnIdx = m_dwCurLowTurn;
	TurnData &rkTurn = rkHighTurn.m_TurnData[m_dwCurLowTurn];





	// �ϼ������� ���� ���� �ٷ� ����.
	int monsterSize = rkTurn.m_vMonsterList.size();

	if(monsterSize > 0)
	{
		SP2Packet kPacket( STPK_SPAWN_MONSTER );
		PACKET_GUARD_VOID(kPacket.Write(monsterSize));
		for(int i = 0; i < monsterSize; i++)
		{
			MonsterRecord & rkMonster = rkTurn.m_vMonsterList[i];
			rkMonster.szSyncUser = SearchMonsterSyncUser();
			/// �÷��������� ǥ�ô� ������. �ƴϸ� ���� ��ũ ���� ���ҵ�.
			rkMonster.eState         = RS_PLAY;

			PACKET_GUARD_VOID(kPacket.Write(rkMonster.dwCode));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.szName));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.szSyncUser));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.eTeam));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.nGrowthLvl));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.bGroupBoss));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.nGroupIdx));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.dwStartTime));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.fStartXPos));
			PACKET_GUARD_VOID(kPacket.Write(rkMonster.fStartZPos));

			LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s  %d, %s, %s, %d, %d, %d, %d, %d, %f, %f", __FUNCTION__, 
				rkMonster.dwCode, rkMonster.szName.c_str(), rkMonster.szSyncUser.c_str(), (int)rkMonster.eTeam, rkMonster.nGrowthLvl, rkMonster.bGroupBoss, rkMonster.nGroupIdx,
				rkMonster.dwStartTime, rkMonster.fStartXPos, rkMonster.fStartZPos);

		}

		SendRoomAllUser(kPacket);

	}

	///// ù�ϸ�ó��
	//if( m_dwCurHighTurn == 0 &&
	//	m_dwCurLowTurn == 0)
	//{
	//	// ������ ���� ���� ����
	//	for(int i = 0;i < GetRecordCnt();i++)
	//	{
	//		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord*>(FindModeRecord( i ));
	//		if( pRecord )
	//		{
	//			if( pRecord->pUser == NULL ) continue;
	//			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
	//			if( pRecord->pUser->GetTeam() != TEAM_BLUE ) continue;

	//			pRecord->bClientViewState = false;
	//		}
	//	}

	//	rkTurn.m_bFirstTurn    = false;
	//	rkTurn.m_bTimeLimitEnd = rkTurn.m_bMonsterAllDie = false;
	//	m_dwRoundDuration = m_dwCurRoundDuration = rkTurn.m_dwTurnTime;
	//	m_dwStateChangeTime    = TIMEGETTIME();

	//	// �� ���۽� ��ȯ�� / �ʵ� ������ ����
	//	m_vPushStructList.clear();
	//	m_vBallStructList.clear();
	//	m_vMachineStructList.clear();
	//	m_pCreator->DestroyAllFieldItems();

	//}

}



void RaidMode::OnDropDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster ) return;

	LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s start  %d, %s, %s, %d, %d, %d, %d, %d, %f, %f", __FUNCTION__, 
		pDieMonster->nNpcType, pDieMonster->szName.c_str(), pDieMonster->szSyncUser.c_str(), (int)pDieMonster->eTeam, pDieMonster->nGrowthLvl, pDieMonster->bGroupBoss, pDieMonster->nGroupIdx,
		pDieMonster->dwStartTime, pDieMonster->fStartXPos, pDieMonster->fStartZPos);

	if( pDieMonster->eState != RS_PLAY ) return;

	// ���Ͱ� ���� ��ġ.
	float fDiePosX, fDiePosZ;
	PACKET_GUARD_VOID(rkPacket.Read(fDiePosX));
	PACKET_GUARD_VOID(rkPacket.Read(fDiePosZ));

	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	PACKET_GUARD_VOID(rkPacket.Read(szLastAttackerName));
	PACKET_GUARD_VOID(rkPacket.Read(szLastAttackerSkillName));
	PACKET_GUARD_VOID(rkPacket.Read(dwLastAttackerWeaponItemCode));
	PACKET_GUARD_VOID(rkPacket.Read(iLastAttackerTeam));


	// ���� ���� ó��
	{
		pDieMonster->eState = RS_DIE;
		pDieMonster->dwCurDieTime = TIMEGETTIME();
		MonsterSurvivalRecord *pSyncUserRecord = FindMonsterSurvivalRecord( pDieMonster->szSyncUser );
		if( pSyncUserRecord )
			pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );
	}

	// ������ ����Ʈ ó��
	int iDamageCnt;
	ioHashString szBestAttackerName;
	PACKET_GUARD_VOID(rkPacket.Read(iDamageCnt));

	int iTotalDamage = 0;
	int iLastDamage = 0;
	int iBestDamage = 0;

	DamageTableList vDamageList;

	if(iDamageCnt > 100)
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "CTPK_DROP_DIE(Champs Mode)Error - DamageCnt:%d  LastAttacker:%s", iDamageCnt, szLastAttackerName.c_str());
		iDamageCnt = 0;
	}

	if( iDamageCnt > 0 )
	{
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			PACKET_GUARD_VOID(rkPacket.Read(kDamageTable.szName));
			PACKET_GUARD_VOID(rkPacket.Read(kDamageTable.iDamage));

			if( kDamageTable.szName == szLastAttackerName )
				iLastDamage = kDamageTable.iDamage;

			vDamageList.push_back( kDamageTable );

			if( kDamageTable.iDamage > 0 )
			{
				iTotalDamage += kDamageTable.iDamage;

				ModeRecord *pRecord = FindModeRecord( kDamageTable.szName );
				if( pRecord )
				{
					pRecord->iTotalDamage += kDamageTable.iDamage;
				}
			}
		}

		std::sort( vDamageList.begin(), vDamageList.end(), DamageTableSort() );

		szBestAttackerName = vDamageList[0].szName;
		iBestDamage = vDamageList[0].iDamage;
	}

	if( GetState() == Mode::MS_PLAY )
	{
		UpdateMonsterDieRecord( szLastAttackerName, szBestAttackerName );
	}	

	float fLastRate = 0.0f;
	float fBestRate = 0.0f;

	if( iTotalDamage > 0 )
	{
		fLastRate = (float)iLastDamage / iTotalDamage;
		fBestRate = (float)iBestDamage / iTotalDamage;
	}

	SP2Packet kReturn( STPK_DROP_DIE );
	PACKET_GUARD_VOID(kReturn.Write(pDieMonster->szName));
	PACKET_GUARD_VOID(kReturn.Write(szLastAttackerName));
	PACKET_GUARD_VOID(kReturn.Write(szLastAttackerSkillName));
	PACKET_GUARD_VOID(kReturn.Write(dwLastAttackerWeaponItemCode));
	PACKET_GUARD_VOID(kReturn.Write(iLastAttackerTeam));
	PACKET_GUARD_VOID(kReturn.Write(szBestAttackerName));
	PACKET_GUARD_VOID(kReturn.Write(fLastRate));
	PACKET_GUARD_VOID(kReturn.Write(fBestRate));
	GetCharModeInfo( kReturn, pDieMonster->szName );
	GetCharModeInfo( kReturn, szLastAttackerName );
	SendRoomAllUser( kReturn );

	//// ������ ���
	//OnMonsterDieToItemDrop( fDiePosX, fDiePosZ, pDieMonster );	

	// ���� ������ ���
	OnMonsterDieToRewardItemDrop( fDiePosX, fDiePosZ, vDamageList, pDieMonster );

	// ���� ����
	OnMonsterDieToPresent( pDieMonster->dwPresentCode );

	// ����ġ & ��� & �ֻ��� ���� ����
	OnMonsterDieToReward( pDieMonster->szName, vDamageList, pDieMonster->dwDiceTable, pDieMonster->iExpReward, pDieMonster->iPesoReward );

	// ���� ���� Ÿ�� ó��
	OnMonsterDieToTypeProcess( pDieMonster->dwDieType );

	m_szLastKillerName = szLastAttackerName;

	//// ��� �׾����� ��������.
	//if(CheckAllDieMonster())
	//{
	//	SetRaidPhaseType(RAID_PHASE_TYPE_DIE_BOSS);
	//}

	LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s  %d, %s, %s, %d, %d, %d, %d, %d, %f, %f", __FUNCTION__, 
		pDieMonster->nNpcType, pDieMonster->szName.c_str(), pDieMonster->szSyncUser.c_str(), (int)pDieMonster->eTeam, pDieMonster->nGrowthLvl, pDieMonster->bGroupBoss, pDieMonster->nGroupIdx,
		pDieMonster->dwStartTime, pDieMonster->fStartXPos, pDieMonster->fStartZPos);


	if(pDieMonster->bEndBoss)
	{
		WinTeamType eWinTeam = WTT_RED_TEAM;
		if( pDieMonster->eTeam == TEAM_RED)
			eWinTeam = WTT_BLUE_TEAM;

		SetRoundEndInfo( eWinTeam );
		SendRoundResult( eWinTeam );
		return;
	}

}


void RaidMode::OnWeaponDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster )
		return;

	LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s start  %d, %s, %s, %d, %d, %d, %d, %d, %f, %f", __FUNCTION__, 
		pDieMonster->dwCode, pDieMonster->szName.c_str(), pDieMonster->szSyncUser.c_str(), (int)pDieMonster->eTeam, pDieMonster->nGrowthLvl, pDieMonster->bGroupBoss, pDieMonster->nGroupIdx,
		pDieMonster->dwStartTime, pDieMonster->fStartXPos, pDieMonster->fStartZPos);

	if( pDieMonster->eState != RS_PLAY )
		return;

	// ���Ͱ� ���� ��ġ.
	float fDiePosX, fDiePosZ;
	PACKET_GUARD_VOID(rkPacket.Read(fDiePosX));
	PACKET_GUARD_VOID(rkPacket.Read(fDiePosZ));

	// Killer ���� ����.
	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	PACKET_GUARD_VOID(rkPacket.Read(szLastAttackerName));
	PACKET_GUARD_VOID(rkPacket.Read(szLastAttackerSkillName));
	PACKET_GUARD_VOID(rkPacket.Read(dwLastAttackerWeaponItemCode));
	PACKET_GUARD_VOID(rkPacket.Read(iLastAttackerTeam));


	// ���� ���� ó��
	{
		pDieMonster->eState = RS_DIE;
		pDieMonster->dwCurDieTime = TIMEGETTIME();
		MonsterSurvivalRecord *pSyncUserRecord = FindMonsterSurvivalRecord( pDieMonster->szSyncUser );
		if( pSyncUserRecord )
			pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );
	}

	// ������ ����Ʈ ó��
	int iDamageCnt;
	ioHashString szBestAttackerName;
	PACKET_GUARD_VOID(rkPacket.Read(iDamageCnt));

	int iTotalDamage = 0;
	int iLastDamage = 0;
	int iBestDamage = 0;

	DamageTableList vDamageList;
	if( iDamageCnt > 0 )
	{
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			PACKET_GUARD_VOID(rkPacket.Read(kDamageTable.szName));
			PACKET_GUARD_VOID(rkPacket.Read(kDamageTable.iDamage));

			if( kDamageTable.szName == szLastAttackerName )
				iLastDamage = kDamageTable.iDamage;

			vDamageList.push_back( kDamageTable );

			if( kDamageTable.iDamage > 0 )
			{
				iTotalDamage += kDamageTable.iDamage;

				ModeRecord *pRecord = FindModeRecord( kDamageTable.szName );
				if( pRecord )
				{
					pRecord->iTotalDamage += kDamageTable.iDamage;
				}
			}
		}

		std::sort( vDamageList.begin(), vDamageList.end(), DamageTableSort() );

		szBestAttackerName = vDamageList[0].szName;
		iBestDamage = vDamageList[0].iDamage;
	}

	if( GetState() == Mode::MS_PLAY )
	{
		UpdateMonsterDieRecord( szLastAttackerName, szBestAttackerName );
	} 

	float fLastRate = 0.0f;
	float fBestRate = 0.0f;

	if( iTotalDamage > 0 )
	{
		fLastRate = (float)iLastDamage / iTotalDamage;
		fBestRate = (float)iBestDamage / iTotalDamage;
	}


	SP2Packet kReturn( STPK_WEAPON_DIE );
	PACKET_GUARD_VOID(kReturn.Write(pDieMonster->szName));
	PACKET_GUARD_VOID(kReturn.Write(szLastAttackerName));
	PACKET_GUARD_VOID(kReturn.Write(szLastAttackerSkillName));
	PACKET_GUARD_VOID(kReturn.Write(dwLastAttackerWeaponItemCode));
	PACKET_GUARD_VOID(kReturn.Write(iLastAttackerTeam));
	PACKET_GUARD_VOID(kReturn.Write(szBestAttackerName));
	PACKET_GUARD_VOID(kReturn.Write(fLastRate));
	PACKET_GUARD_VOID(kReturn.Write(fBestRate));
	GetCharModeInfo( kReturn, pDieMonster->szName );
	GetCharModeInfo( kReturn, szLastAttackerName );
	SendRoomAllUser( kReturn );

	//�̼� üũ
	ModeRecord *pRecord = FindModeRecord( szLastAttackerName );
	static DWORDVec vValues;
	vValues.clear();
	if( pRecord )
	{
		DWORD dwMonsterCode = pDieMonster->dwCode;
		vValues.push_back(dwMonsterCode);
		if( pRecord->pUser )
			g_MissionMgr.DoTrigger(MISSION_CLASS_MONSTER_KILL , pRecord->pUser, vValues);
	}

	//// ������ ���
	//OnMonsterDieToItemDrop( fDiePosX, fDiePosZ, pDieMonster );	

	// ���� ������ ���
	OnMonsterDieToRewardItemDrop( fDiePosX, fDiePosZ, vDamageList, pDieMonster );

	// ���� ����
	OnMonsterDieToPresent( pDieMonster->dwPresentCode );

	// ����ġ & ��� & �ֻ��� ���� ����
	OnMonsterDieToReward( pDieMonster->szName, vDamageList, pDieMonster->dwDiceTable, pDieMonster->iExpReward, pDieMonster->iPesoReward );

	// ���� ���� Ÿ�� ó��
	OnMonsterDieToTypeProcess( pDieMonster->dwDieType );

	m_szLastKillerName = szLastAttackerName;

	//if(pDieMonster->bGroupBoss)
	//{
	//	//// ������ ��������.
	//	//SendReviveMonstersEvent(pDieMonster, iLastAttackerTeam);
	//}

	//// ��� �׾����� ��������.
	//if(CheckAllDieMonster())
	//{
	//	SetRaidPhaseType(RAID_PHASE_TYPE_DIE_BOSS);
	//}

	LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] %s end  %d, %s, %s, %d, %d, %d, %d, %d, %f, %f", __FUNCTION__, 
		pDieMonster->dwCode, pDieMonster->szName.c_str(), pDieMonster->szSyncUser.c_str(), (int)pDieMonster->eTeam, pDieMonster->nGrowthLvl, pDieMonster->bGroupBoss, pDieMonster->nGroupIdx,
		pDieMonster->dwStartTime, pDieMonster->fStartXPos, pDieMonster->fStartZPos);

	if(pDieMonster->bEndBoss)
	{
		WinTeamType eWinTeam = WTT_RED_TEAM;
		if( pDieMonster->eTeam == TEAM_RED)
			eWinTeam = WTT_BLUE_TEAM;

		SetRoundEndInfo( eWinTeam );
		SendRoundResult( eWinTeam );
		return;
	}
}

void RaidMode::SendRoundResult( WinTeamType eWinTeam )
{
	SP2Packet kPacket( STPK_ROUND_END );
	PACKET_GUARD_VOID(kPacket.Write(eWinTeam));
	PACKET_GUARD_VOID(kPacket.Write(m_iRedTeamWinCnt));
	PACKET_GUARD_VOID(kPacket.Write(m_iBlueTeamWinCnt));
	PACKET_GUARD_VOID(kPacket.Write(GetPlayingUserCnt()));

	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord || pRecord->eState == RS_LOADING )
			continue;

		PACKET_GUARD_VOID(kPacket.Write(pRecord->pUser->GetPublicID()));

		//
		int iMyVictories = 0;
		if( pRecord->pUser )
		{
			if( m_bRoundSetEnd && eWinTeam != WTT_DRAW && eWinTeam != WTT_NONE )
				pRecord->pUser->IncreaseMyVictories( IsWinTeam(eWinTeam, pRecord->pUser->GetTeam()) );

			iMyVictories = pRecord->pUser->GetMyVictories();
		}

		PACKET_GUARD_VOID(kPacket.Write(iMyVictories));
		//

		//
		int iKillSize = pRecord->iKillInfoMap.size();
		PACKET_GUARD_VOID(kPacket.Write(iKillSize));

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_k = pRecord->iKillInfoMap.begin();
		while( iter_k != pRecord->iKillInfoMap.end() )
		{
			PACKET_GUARD_VOID(kPacket.Write(iter_k->first));
			PACKET_GUARD_VOID(kPacket.Write(iter_k->second));

			++iter_k;
		}
		LOOP_GUARD_CLEAR();

		int iDeathSize = pRecord->iDeathInfoMap.size();
		PACKET_GUARD_VOID(kPacket.Write(iDeathSize));

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_d = pRecord->iDeathInfoMap.begin();
		while( iter_d != pRecord->iDeathInfoMap.end() )
		{
			PACKET_GUARD_VOID(kPacket.Write(iter_d->first));
			PACKET_GUARD_VOID(kPacket.Write(iter_d->second));

			++iter_d;
		}
		LOOP_GUARD_CLEAR();
		//

		PACKET_GUARD_VOID(kPacket.Write(pRecord->iCurRank));
		PACKET_GUARD_VOID(kPacket.Write(pRecord->iPreRank));
	}

	PACKET_GUARD_VOID(kPacket.Write(m_bRoundSetEnd));

	FillResultSyncUser( kPacket );

	SendRoomAllUser( kPacket );

	// Ŭ���̾�Ʈ�� ���� ��Ŷ�� ������ ������ ĳ���� �츮�� ��Ŷ�� �����µ� ���ذ� ���� �ʴ´�. 
	// �׳� �Ʒ�ó���ϸ� ��Ŷ ���� �ʿ� ���� ������?  LJH..... 20081002
	for(int i = 0;i < iRecordCnt;i++)
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord || !pRecord->pUser )
			continue;
		pRecord->pUser->SetCharDie( false );
	}
}

void RaidMode::UpdateUserDieNextProcess( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{

}

void RaidMode::AddNewRecord( User *pUser )
{
	MonsterSurvivalRecord kRecord;
	kRecord.pUser = pUser;

	// ī������ �ʱ�ȭ
	kRecord.m_vTreasureCard.clear();
	for(int i = 0;i < RAID_TREASURECARD_MAX;i++)
		kRecord.m_vTreasureCard.push_back( i + 1 );


	m_vRecordList.push_back( kRecord );

	kRecord.StartPlaying();
	UpdateUserRank();

	// �߰� ���� ���� NPC ����ȭ
	PlayMonsterSync( &kRecord );
}

void RaidMode::PlayMonsterSync( MonsterSurvivalRecord *pSendRecord )
{
	if( m_bRoundSetEnd ) return;
	if( GetState() != MS_PLAY ) return;        // �÷����� ������ �����鿡�Ը� ����ȭ ��Ų��.
	if( pSendRecord == NULL || pSendRecord->pUser == NULL ) return;

	if( !COMPARE( m_dwCurHighTurn, 0, (int)m_HighTurnList.size() ) )
		return;

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurHighTurn];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
		return;

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	SP2Packet kPacket( STPK_MONSTER_INFO_SYNC );

	// �÷������� NPC�� ����ȭ ���� ����.
	int i,j,k;
	MonsterRecordList vSyncRecord;           // ����ȭ ����
	int iHighTurnCnt = m_HighTurnList.size();
	for(i = 0;i < iHighTurnCnt;i++)
	{
		HighTurnData &rkHighTurn = m_HighTurnList[i];
		int iLowTurnCnt = rkHighTurn.m_TurnData.size();
		for(j = 0;j < iLowTurnCnt;j++)
		{
			TurnData &rkData = rkHighTurn.m_TurnData[j];
			int iMonsterCnt = rkData.m_vMonsterList.size();
			for(k = 0;k < iMonsterCnt;k++)
			{
				MonsterRecord &rkMonster = rkData.m_vMonsterList[k];
				if( rkMonster.eState != RS_PLAY ) continue;

				vSyncRecord.push_back( rkMonster );
			}
		}
	}
	int iSyncSize = vSyncRecord.size();	
	PACKET_GUARD_VOID(kPacket.Write(iSyncSize));
	for(i = 0;i < iSyncSize;i++)
	{
		MonsterRecord &rkMonster = vSyncRecord[i];

		PACKET_GUARD_VOID(kPacket.Write(rkMonster.dwCode));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.szName));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.szSyncUser));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.eTeam));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.bGroupBoss));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.nGroupIdx));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.dwStartTime));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.fStartXPos));
		PACKET_GUARD_VOID(kPacket.Write(rkMonster.fStartZPos));

	}

	pSendRecord->pUser->SendMessage( kPacket ); 
	vSyncRecord.clear();
}



bool RaidMode::ProcessTCPPacket( User *pSend, SP2Packet &rkPacket )
{
	if( Mode::ProcessTCPPacket( pSend, rkPacket ) )
		return true;

	switch( rkPacket.GetPacketID() )
	{
	case CTPK_PRISONER_ESCAPE:
		//		OnPrisonerEscape( pSend, rkPacket );
		return true;
	case CTPK_PRISONER_DROP:
		//		OnPrisonerDrop( pSend, rkPacket );
		return true;
	case CTPK_PRISONERMODE:
		//		OnPrisonerMode( pSend, rkPacket );
		return true;
	case CTPK_USE_MONSTER_COIN:
		OnUseMonsterCoin( pSend, rkPacket );
		return true;
	case CTPK_TURN_END_VIEW_STATE:
		//		OnTurnEndViewState( pSend, rkPacket );
		return true;
	case CTPK_PICK_REWARD_ITEM:
		OnPickRewardItem( pSend, rkPacket );
		return true;
	case CTPK_TREASURE_CARD_COMMAND:
		OnTreasureCardCommand( pSend, rkPacket );
		return true;
	case CTPK_SHUFFLEROOM_DROPZONE:
		OnDropZoneDrop( rkPacket );
		return true;

	}

	return false;
}



void RaidMode::OnTreasureCardCommand( User *pUser, SP2Packet &rkPacket )
{
	if( pUser == NULL ) return;

	int iCommand;

	PACKET_GUARD_VOID(rkPacket.Read(iCommand));

	switch( iCommand )
	{
	case TREASURE_CARD_CMD_TIME_OUT:
		{
			TreasureCardStop(pUser);
			//TreasureCardStop();
			CheckResultPieceInfo();
		}
		break;
	case TREASURE_CARD_CMD_CLICK:
		{
			DWORD dwID;
			PACKET_GUARD_VOID(rkPacket.Read(dwID));

			MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
			if( pRecord == NULL )
				return;

			if( pRecord->iTreasureCardCount <= 0 || !COMPARE( dwID, 1, RAID_TREASURECARD_MAX + 1 ) || IsTreasureAlreadyID( pRecord, dwID ) )
			{
				// �̹� Ŭ���Ǿ��ų� ��ġ�� �ƴ� ���� Ƚ���� ����
				SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
				PACKET_GUARD_VOID(rkPacket.Write(TREASURE_CARD_CMD_CLICK));
				PACKET_GUARD_VOID(rkPacket.Write(0));
				pUser->SendMessage( kPacket );
			}
			else
			{
				// ���� ����
				OpenTreasureCard( pRecord, dwID );
			}
		}
		break;
	}
}


DWORD RaidMode::CreateTreasureCardIndex( DWORD dwPlayTime )
{
	if( m_TreasureCardIndexList.empty() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CreateTreasureCardIndex None List!!" );
		return 0;
	}

	int iSize = m_TreasureCardIndexList.size();
	for(int i = 0;i < iSize;i++)
	{
		TreasureCardIndex &rkIndexData = m_TreasureCardIndexList[i];
		if( i == 0 )    
		{
			// ����ó�� : Ȥ�� �ð��� ���ٸ� �⺻ ����
			m_dwTreasureCardIndex = rkIndexData.m_dwCardIndex;
		}

		if( dwPlayTime <= rkIndexData.m_dwPlaySecond )
		{
			m_dwTreasureCardIndex = rkIndexData.m_dwCardIndex;
			break;
		}
	}

	return m_dwTreasureCardIndex;
}

bool RaidMode::IsTreasureAlreadyID( MonsterSurvivalRecord *pRecord, DWORD dwID )
{
	if(pRecord == NULL)
		return false;
	
	for(int i = 0;i < (int)pRecord->m_vTreasureCard.size();i++)
	{
		if( pRecord->m_vTreasureCard[i] == dwID )
			return false;
	}
	return true;

}

void RaidMode::EraseTreasureCardID( MonsterSurvivalRecord *pRecord, DWORD dwID )
{
	if(pRecord == NULL)
		return;

	for(int i = 0;i < (int)pRecord->m_vTreasureCard.size();i++)
	{
		if( pRecord->m_vTreasureCard[i] == dwID )
		{
			pRecord->m_vTreasureCard.erase( pRecord->m_vTreasureCard.begin() + i );
			break;
		}
	}

}

void RaidMode::OpenTreasureCard( MonsterSurvivalRecord *pRecord, DWORD dwID )
{
	if( pRecord == NULL || pRecord->pUser == NULL ) return;

	CreateTreasureCardIndex(pRecord->GetAllPlayingTime());

	for(int i = 0;i < (int)pRecord->m_vTreasureCard.size();i++)
	{
		if( pRecord->m_vTreasureCard[i] == dwID )
		{
			pRecord->m_vTreasureCard.erase( pRecord->m_vTreasureCard.begin() + i );

			// ���� ����

			if(pRecord->iTreasureCardCount > 0)
			{
				pRecord->iTreasureCardCount--;
			}
			else
			{
				if(pRecord->pUser)
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::iTeasureCardCount IS NULL (%s)", pRecord->pUser->GetPrivateID());
				break;
			}



			ioHashString szSendID; 
			short iPresentType = 0; short iPresentState = 0; short iPresentMent = 0;
			int iPresentPeriod = 0; int iPresentValue1 = 0; int iPresentValue2 = 0;


			g_MonsterMapLoadMgr.GetTreasureCardPresent( m_dwTreasureCardIndex, szSendID, iPresentType, iPresentState,
				iPresentMent, iPresentPeriod, iPresentValue1, iPresentValue2 );
			if( iPresentType == 0 )
			{
				// ��ǰ ����. â���� ��!!!!
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::OpenTreasureCard None Card Present : %d", m_dwTreasureCardIndex );
			}
			else if( iPresentType == PRESENT_ALCHEMIC_ITEM )
			{
				int iResult = pRecord->pUser->SetPresentAlchemicItem( iPresentValue1, iPresentValue2 );
				bool bSendPresent = false;
				switch( iResult )
				{
				case PRESENT_RECV_MAX_COUNT:
				case PRESENT_RECV_MAX_SLOT:
					bSendPresent = true;
					break;
				}

				if( bSendPresent )
				{
					CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
					CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
					pRecord->pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
						0, 0, iPresentMent, kPresentTime, iPresentState );
					pRecord->pUser->SendPresentMemory();
				}

				// �Ѹ� ������.
				SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
				PACKET_GUARD_VOID(kPacket.Write(TREASURE_CARD_CMD_CLICK));
				PACKET_GUARD_VOID(kPacket.Write(dwID));
				PACKET_GUARD_VOID(kPacket.Write(pRecord->pUser->GetPublicID()));
				PACKET_GUARD_VOID(kPacket.Write(pRecord->iTreasureCardCount));
				PACKET_GUARD_VOID(kPacket.Write(iPresentType));
				PACKET_GUARD_VOID(kPacket.Write(iPresentValue1));
				PACKET_GUARD_VOID(kPacket.Write(iPresentValue2));
				//SendRoomAllUser( kPacket );
				pRecord->pUser->SendMessage(kPacket);

				//
				int iUserIndex = pRecord->pUser->GetUserIndex();
				ResultPieceInfoMap::iterator iter = m_UserResultPieceInfoMap.find( iUserIndex );
				if( iter != m_UserResultPieceInfoMap.end() )
				{
					iter->second.m_iPieceCnt += iPresentValue2;
				}
				else
				{
					ResultPieceInfo kPieceInfo;
					kPieceInfo.m_ID = pRecord->pUser->GetPublicID();
					kPieceInfo.m_iLevel = pRecord->pUser->GetGradeLevel();
					kPieceInfo.m_iPieceCnt = iPresentValue2;
					kPieceInfo.m_iPlayTime = pRecord->GetAllPlayingTime()/1000;

					m_UserResultPieceInfoMap.insert( ResultPieceInfoMap::value_type(iUserIndex, kPieceInfo) );
				}
			}
			else
			{
				CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
				CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
				pRecord->pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
					0, 0, iPresentMent, kPresentTime, iPresentState );
				pRecord->pUser->SendPresentMemory();

				//
				SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
				PACKET_GUARD_VOID(kPacket.Write(TREASURE_CARD_CMD_CLICK));
				PACKET_GUARD_VOID(kPacket.Write(dwID));
				PACKET_GUARD_VOID(kPacket.Write(pRecord->pUser->GetPublicID()));
				PACKET_GUARD_VOID(kPacket.Write(pRecord->iTreasureCardCount));
				PACKET_GUARD_VOID(kPacket.Write(iPresentType));
				PACKET_GUARD_VOID(kPacket.Write(iPresentValue1));
				PACKET_GUARD_VOID(kPacket.Write(iPresentValue2));
				//SendRoomAllUser( kPacket );
				pRecord->pUser->SendMessage(kPacket);

			}
			break;
		}
	}
}


void RaidMode::TreasureCardStop( User *pUser )
{

	MonsterSurvivalRecord * pRecord = FindMonsterSurvivalRecord(pUser);
	if(pRecord == NULL)
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] : %s Can't find User Record %d", __FUNCTION__, pUser->GetUserIndex());
	}

	int iRemainCount = pRecord->iTreasureCardCount;
	if(iRemainCount <= 0)
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] : %s UserCardCount is not valid!! %d, %d", __FUNCTION__, pUser->GetUserIndex(), iRemainCount);
	}

	SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
	PACKET_GUARD_VOID(kPacket.Write(TREASURE_CARD_CMD_TIME_OUT));
	PACKET_GUARD_VOID(kPacket.Write(TIMEGETTIME()));
	PACKET_GUARD_VOID(kPacket.Write(iRemainCount));

	if( !pRecord->m_vTreasureCard.empty() )
		std::random_shuffle( pRecord->m_vTreasureCard.begin(), pRecord->m_vTreasureCard.end() );


	CreateTreasureCardIndex(pRecord->GetAllPlayingTime());

	for(int k = 0; k < pRecord->iTreasureCardCount; k++)
	{
		ioHashString szSendID; 
		short iPresentType, iPresentState, iPresentMent;
		int iPresentPeriod, iPresentValue1, iPresentValue2;
		g_MonsterMapLoadMgr.GetTreasureCardPresent( m_dwTreasureCardIndex, szSendID, iPresentType, iPresentState,
			iPresentMent, iPresentPeriod, iPresentValue1, iPresentValue2 );
		if( iPresentType == 0 )
		{
			// ��ǰ ����. â���� ��!!!!
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::TreasureCardStop None Card Present : %d", m_dwTreasureCardIndex );
		}
		else if( iPresentType == PRESENT_ALCHEMIC_ITEM )
		{
			int iResult = pUser->SetPresentAlchemicItem( iPresentValue1, iPresentValue2 );
			bool bSendPresent = false;
			switch( iResult )
			{
			case PRESENT_RECV_MAX_COUNT:
			case PRESENT_RECV_MAX_SLOT:
				bSendPresent = true;
				break;
			}

			if( bSendPresent )
			{
				CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
				CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
				pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
					0, 0, iPresentMent, kPresentTime, iPresentState );
				pUser->SendPresentMemory();
			}

			//
			DWORD dwID = *(pRecord->m_vTreasureCard.begin());
			PACKET_GUARD_VOID(kPacket.Write(dwID));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetPublicID()));
			PACKET_GUARD_VOID(kPacket.Write(iPresentType));
			PACKET_GUARD_VOID(kPacket.Write(iPresentValue1));
			PACKET_GUARD_VOID(kPacket.Write(iPresentValue2));

			int iUserIndex = pUser->GetUserIndex();
			ResultPieceInfoMap::iterator iter = m_UserResultPieceInfoMap.find( iUserIndex );
			if( iter != m_UserResultPieceInfoMap.end() )
			{
				iter->second.m_iPieceCnt += iPresentValue2;
			}
			else
			{
				ResultPieceInfo kPieceInfo;
				kPieceInfo.m_ID = pUser->GetPublicID();
				kPieceInfo.m_iLevel = pUser->GetGradeLevel();
				kPieceInfo.m_iPieceCnt = iPresentValue2;
				kPieceInfo.m_iPlayTime = GetRecordPlayTime( pRecord );

				m_UserResultPieceInfoMap.insert( ResultPieceInfoMap::value_type(iUserIndex, kPieceInfo) );
			}

			if( pRecord->m_vTreasureCard.size() > 1 )
				pRecord->m_vTreasureCard.erase( pRecord->m_vTreasureCard.begin() );
		}
		else
		{
			DWORD dwID = *(pRecord->m_vTreasureCard.begin());
			CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
			CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
			pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
				0, 0, iPresentMent, kPresentTime, iPresentState );

			PACKET_GUARD_VOID(kPacket.Write(dwID));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetPublicID()));
			PACKET_GUARD_VOID(kPacket.Write(iPresentType));
			PACKET_GUARD_VOID(kPacket.Write(iPresentValue1));
			PACKET_GUARD_VOID(kPacket.Write(iPresentValue2));

			if( pRecord->m_vTreasureCard.size() > 1 )
				pRecord->m_vTreasureCard.erase( pRecord->m_vTreasureCard.begin() );
		}
	}
	pRecord->iTreasureCardCount = 0;
	pUser->SendPresentMemory();
	pUser->SendMessage(kPacket);
}

void RaidMode::UpdateUserRank()
{
	int iRecordCnt = GetRecordCnt();

	RankList vRankList;
	vRankList.reserve( iRecordCnt );

	for( int i=0 ;i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord || pRecord->eState == RS_LOADING )
			continue;

		RankInfo kInfo;
		kInfo.szName	  = pRecord->pUser->GetPublicID(); 
		kInfo.iTotalKill  = pRecord->iUniqueTotalKill;
		kInfo.iTotalDeath = pRecord->iUniqueTotalDeath;

		if( m_bRoundSetEnd )	// Final Round
		{
			kInfo.bAbuse = IsAbuseUser( i );
		}

		vRankList.push_back( kInfo );
	}

	if( !m_bRoundSetEnd )
	{
		std::sort( vRankList.begin(), vRankList.end(), RankInfoSort() );
	}
	else
	{
		std::sort( vRankList.begin(), vRankList.end(), FinalRankInfoSort() );
	}

	int iRank = 1;
	RankList::iterator iter, iEnd;
	iEnd = vRankList.end();
	for( iter=vRankList.begin() ; iter!=iEnd ; ++iter, ++iRank )
	{
		ModeRecord *pRecord = FindModeRecord( iter->szName );
		if( pRecord )
		{
			if( pRecord->iPreRank == 0 )
			{
				pRecord->iPreRank = iRank;
			}
			else
			{
				pRecord->iPreRank = pRecord->iCurRank;
			}

			pRecord->iCurRank = iRank;

			if( iRecordCnt < 4)
				pRecord->iCurRank++;
		}
	}
}


void RaidMode::FinalRoundPoint( ModeRecord *pRecord, bool bAbuseUser, float fTotalVictoriesRate, float fTotalConsecutivelyRate )
{
	fTotalVictoriesRate = 1.0f;
	fTotalConsecutivelyRate = 1.0f;

	if( !pRecord )
		return;

	User *pUser = pRecord->pUser;
	if( !pUser ) return;

	// �������� �� �Լ��� ������ �ʿ䰡 ����.
	if( pUser->IsObserver() || pUser->IsStealth() ) 
	{
		pUser->SetModeConsecutively( MT_NONE );       // �������� ���� ���� �ʱ�ȭ
		return;
	}

	int iCurMaxSlot = pUser->GetCurMaxCharSlot();

	if( m_dwModePointTime == 0 || m_dwModeRecordPointTime == 0 )
		return;

	TeamType eWinTeam  = GetWinTeam();

	if( IsBlueWin(m_CurRoundWinTeam))
		eWinTeam = TEAM_BLUE;
	else
		eWinTeam = TEAM_RED;

	//�÷��� ���� ���
	ModeCategory ePlayMode = GetPlayModeCategory();

	//�ϸ��� ������ ����Ʈ B
	HighTurnData &rkHighTurn = m_HighTurnList[0];
	TurnData &rkTurn = rkHighTurn.m_TurnData[0];

	DWORD dwTurnPlayTime = min( pRecord->GetAllPlayingTime(), rkTurn.m_dwTurnTime );
	float fPlayTimeGap = (float)dwTurnPlayTime / 60000.0f;        //60�� ����

	//	float fModeTurnPoint = m_fModeTurnPoint;        
	float fModeTurnPoint = rkTurn.m_fTurnPoint * fPlayTimeGap;
	//�ο� ���� C
	float fUserCorrection = GetUserCorrection( eWinTeam, 0.0f, 0.0f );
	//�÷��� �ð� ������ D : �÷��� ���� �ð��� �������� �ʴ´�.
	float fPlayTimeCorrection = 1.0f; //(float)GetRecordPlayTime( pRecord ) / m_dwModePointTime;
	//��Һ����� E
	float fPesoCorrection = m_fPesoCorrection;
	//����ġ ������ F
	float fExpCorrection  = m_fExpCorrection;
	//���� G
	float fBlockPoint = pUser->GetBlockPointPer();
	//�⿩�� H
	float fContributePer = pRecord->fContributePer;
	//��庸�ʽ� I
	pRecord->fBonusArray[BA_GUILD] = m_pCreator->GetGuildBonus( pRecord->pUser->GetTeam() );
	float fGuildBonus = pRecord->fBonusArray[BA_GUILD];
	//�뺴 ���ʽ� J
	pRecord->fBonusArray[BA_SOLDIER_CNT] = Help::GetSoldierPossessionBonus( pUser->GetActiveCharCount() );
	float fSoldierCntBonus = pRecord->fBonusArray[BA_SOLDIER_CNT];
	//PC�� ���ʽ� K
	if( pUser->IsPCRoomAuthority() )
	{
		pRecord->fBonusArray[BA_PCROOM_EXP] = Help::GetPCRoomBonusExp();
		pRecord->fBonusArray[BA_PCROOM_PESO]= Help::GetPCRoomBonusPeso();

		if( g_EventMgr.IsAlive( EVT_PCROOM_BONUS, pUser->GetChannelingType(), ePlayMode ) )
		{
			EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
			PCRoomEventUserNode* pPcroomEventNode = static_cast<PCRoomEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PCROOM_BONUS, ePlayMode ) );

			if( pPcroomEventNode )
				pPcroomEventNode->SetPesoAndExpBonus( pUser ,pRecord->fBonusArray[BA_PCROOM_PESO], pRecord->fBonusArray[BA_PCROOM_EXP], ePlayMode ); 
		}
	}
	float fPCRoomBonusExp  = pRecord->fBonusArray[BA_PCROOM_EXP];
	float fPCRoomBonusPeso = pRecord->fBonusArray[BA_PCROOM_PESO];

	//��� ���ʽ� L
	if( pUser )
	{
		// EVT_MODE_BONUS (1)
		//���� ��� �ϰ�� ��� üũ �ǳʶٱ� ����. ���� ��尡 �ƴҰ�� ��� üũ 
		ModeCategory eModeBonus = ePlayMode;

		if( eModeBonus != MC_SHUFFLE )
			eModeBonus = MC_DEFAULT;

		EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
		ModeBonusEventUserNode* pEvent1 = static_cast< ModeBonusEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_MODE_BONUS, eModeBonus ) );
		if( pEvent1 )
		{
			if( pEvent1->IsEventMode( GetModeType(), eModeBonus ) )
				pRecord->fBonusArray[ BA_PLAYMODE ] = pEvent1->GetEventPer( fPCRoomBonusExp, pUser, eModeBonus );
		}

		// EVT_MODE_BONUS2 (2)
		ModeBonusEventUserNode* pEvent2 = static_cast< ModeBonusEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_MODE_BONUS2, eModeBonus ) );
		if( pEvent2 )
		{
			if( pEvent2->IsEventMode( GetModeType(), eModeBonus ) )
				pRecord->fBonusArray[ BA_PLAYMODE ] += pEvent2->GetEventPer( fPCRoomBonusExp, pUser, eModeBonus );
		}

		if( pRecord->fBonusArray[BA_PLAYMODE] == 0.0f )
		{
			pRecord->fBonusArray[BA_PLAYMODE] = m_fPlayModeBonus;
		}
	}
	float fModeBonus = pRecord->fBonusArray[BA_PLAYMODE];

	//ģ�� ���ʽ� M	
	if( pUser->IsPCRoomAuthority() )
	{		
		pRecord->fBonusArray[BA_FRIEND] = min( GetPcRoomMaxFriendBonus(), GetPcRoomFriendBonus() * (float)GetSameFriendUserCnt( pUser ) );
	}
	else
	{
		pRecord->fBonusArray[BA_FRIEND] = min( GetMaxFriendBonus(), GetFriendBonus() * (float)GetSameFriendUserCnt( pUser ) );
	}

	float fFriendBonusPer = pRecord->fBonusArray[BA_FRIEND];

	// �̺�Ʈ ����ġ ���ʽ� N
	float fEventBonus = 0.0f;
	if( pUser )
	{
		EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
		ExpEventUserNode *pEventNode = static_cast<ExpEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_EXP, ePlayMode ) );
		if( pEventNode )
		{
			pRecord->fBonusArray[BA_EVENT] = pEventNode->GetEventPer( fPCRoomBonusExp, pUser, ePlayMode );
			fEventBonus = pRecord->fBonusArray[BA_EVENT];
		}

		// second event : evt_exp2
		ExpEventUserNode* pExp2 = static_cast< ExpEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_EXP2, ePlayMode ) );
		if( pExp2 )
		{
			pRecord->fBonusArray[ BA_EVENT ] += pExp2->GetEventPer( fPCRoomBonusExp, pUser, ePlayMode );
			fEventBonus = pRecord->fBonusArray[ BA_EVENT ];
		}
	}
	// �̺�Ʈ ��� ���ʽ� O
	float fPesoEventBonus = 0.0f;
	if( pUser )
	{
		EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
		PesoEventUserNode *pEventNode = static_cast<PesoEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PESO, ePlayMode ) );
		if( pEventNode )
		{
			pRecord->fBonusArray[BA_EVENT_PESO] = pEventNode->GetPesoPer( fPCRoomBonusPeso, pUser, ePlayMode );
			fPesoEventBonus = pRecord->fBonusArray[BA_EVENT_PESO];
		}

		// second event : evt_peso2
		PesoEventUserNode* pPeso2 = static_cast< PesoEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_PES02, ePlayMode ) );
		if( pPeso2 )
		{
			pRecord->fBonusArray[ BA_EVENT_PESO ] += pPeso2->GetPesoPer( fPCRoomBonusPeso, pUser, ePlayMode );
			fPesoEventBonus = pRecord->fBonusArray[ BA_EVENT_PESO ];
		}
	}
	// ���� ������ ���ʽ� P
	float fEtcItemBonus = 0.0f;
	float fEtcItemPesoBonus = 0.0f;
	float fEtcItemExpBonus  = 0.0f;
	if( pUser )
	{
		ioUserEtcItem *pUserEtcItem = pUser->GetUserEtcItem();
		if( pUserEtcItem )
		{
			ioEtcItem *pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_PESO_EXP_BONUS );
			ioUserEtcItem::ETCITEMSLOT kSlot;
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_PESO_EXP_BONUS, kSlot) && pItemItem )
			{
				pRecord->fBonusArray[BA_ETC_ITEM] = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
				fEtcItemBonus = pRecord->fBonusArray[BA_ETC_ITEM];
			}

			pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_PESO_BONUS );
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_PESO_BONUS, kSlot ) && pItemItem )
			{
				pRecord->fBonusArray[BA_ETC_ITEM_PESO] = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
				fEtcItemPesoBonus = pRecord->fBonusArray[BA_ETC_ITEM_PESO];
			}

			pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_EXP_BONUS );
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_EXP_BONUS, kSlot ) && pItemItem )
			{
				pRecord->fBonusArray[BA_ETC_ITEM_EXP] = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
				fEtcItemExpBonus = pRecord->fBonusArray[BA_ETC_ITEM_EXP];
			}
		}
	}
	// ������ ���ʽ� Q
	if( m_pCreator->GetRoomStyle() == RSTYLE_LADDERBATTLE )
		pRecord->fBonusArray[BA_CAMP_BONUS] = Help::GetLadderBonus();
	float fCampBattleBonus = pRecord->fBonusArray[BA_CAMP_BONUS];

	// �û�� ���ʽ� R
	float fAwardBonus = pRecord->fBonusArray[BA_AWARD_BONUS];

	// ������ Ÿ��Ʋ ���ʽ� S
	pRecord->fBonusArray[BA_HERO_TITLE_PESO] = GetHeroTitleBonus( pUser );
	float fHeroTitlePesoBonus = pRecord->fBonusArray[BA_HERO_TITLE_PESO];

	// ���� ��� ���ʽ�
	//pRecord->fBonusArray[BA_MODE_CONSECUTIVELY] = pUser->GetModeConsecutivelyBonus();
	//float fModeConsecutivelyBonus = (1.0f + pRecord->fBonusArray[BA_MODE_CONSECUTIVELY]) * fTotalConsecutivelyRate;
	float fModeConsecutivelyBonus = 1.0f;

	//ȹ�� ����ġ
	float fAcquireExp       = 0.0f;
	float fExpPlusValue     = max( 0.0f, fContributePer + fGuildBonus + fSoldierCntBonus + fPCRoomBonusExp + fModeBonus + fFriendBonusPer + fEventBonus + fEtcItemBonus + fCampBattleBonus + fEtcItemExpBonus );
	float fExpTotalMultiply = fUserCorrection * fPlayTimeCorrection * fExpCorrection * fBlockPoint * fExpPlusValue;
	char szLogArguWinLose[MAX_PATH]="";
	if( pUser->GetTeam() == eWinTeam )
	{
		fAcquireExp = fModeTurnPoint * fExpTotalMultiply;
		StringCbCopy( szLogArguWinLose, sizeof(szLogArguWinLose), "WIN" );
	}
	else
	{
		fAcquireExp = fModeTurnPoint * fExpTotalMultiply * m_fDefeatRatio;
		StringCbCopy( szLogArguWinLose, sizeof(szLogArguWinLose), "LOSE" );
	}
	fAcquireExp = fAcquireExp * fModeConsecutivelyBonus;
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[%d] %s [%d]: %s EXP: ( %.2f ) x %.2f x %.4f x %.2f x %.2f x ( %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f ) x %.2f = %.2f",
		m_pCreator->GetRoomIndex(), pUser->GetPublicID().c_str(), (int)bAbuseUser, szLogArguWinLose, 
		fModeTurnPoint, fUserCorrection, fPlayTimeCorrection, fExpCorrection, fBlockPoint, fContributePer, fGuildBonus, fSoldierCntBonus, fPCRoomBonusExp, fModeBonus, fFriendBonusPer, fEventBonus, fEtcItemBonus, fCampBattleBonus, fEtcItemExpBonus, fModeConsecutivelyBonus, fAcquireExp );

	//ȹ�� ���
	float fAcquirePeso       = 0.0f;
	float fPesoPlusValue     = max( 0.0f, fContributePer + fGuildBonus + fPCRoomBonusPeso + fModeBonus + fFriendBonusPer + fPesoEventBonus + fEtcItemBonus + fCampBattleBonus + fAwardBonus + fEtcItemPesoBonus + fHeroTitlePesoBonus );
	float fPesoTotalMultiply = fUserCorrection * fPlayTimeCorrection * fPesoCorrection * fBlockPoint * fPesoPlusValue;

	if( pUser->GetTeam() == eWinTeam )
		fAcquirePeso = fModeTurnPoint * fPesoTotalMultiply;
	else
		fAcquirePeso = fModeTurnPoint * fPesoTotalMultiply * m_fDefeatRatio;

	fAcquirePeso = fAcquirePeso * fModeConsecutivelyBonus;

	//���� ���ʽ� ���� ����
	pRecord->fBonusArray[BA_VICTORIES_PESO] = 0.0f;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[%d] %s [%d]: %s PESO : (( %.2f ) x %.2f x %.4f x %.2f x %.2f x ( %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f ) x %.2f x %.2f ) = %.2f",
		m_pCreator->GetRoomIndex(), pUser->GetPublicID().c_str(), (int)bAbuseUser, szLogArguWinLose, 
		fModeTurnPoint, fUserCorrection, fPlayTimeCorrection, fPesoCorrection, fBlockPoint, fContributePer, fGuildBonus, fPCRoomBonusPeso, fModeBonus, fFriendBonusPer, fPesoEventBonus, fEtcItemBonus, fCampBattleBonus, fAwardBonus, fEtcItemPesoBonus, fHeroTitlePesoBonus, fModeConsecutivelyBonus, 0.0f, fAcquirePeso );

	fAcquirePeso += 0.5f;     //�ݿø�

	//����� ����
	if( bAbuseUser )
	{
		fAcquireExp = 0.0f;
		fAcquirePeso= 0.0f;
	}
	else
	{
		// �÷��� �ð� �̺�Ʈ
		EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
		ChanceMortmainCharEventUserNode *pEventNode = static_cast<ChanceMortmainCharEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_CHANCE_MORTMAIN_CHAR ) );
		if( pEventNode )
		{
			pEventNode->UpdatePlayTime( pUser, GetRecordPlayTime( pRecord ) );
		}
		PlayTimePresentEventUserNode *pPlayTimePresentEventNode = static_cast<PlayTimePresentEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PLAYTIME_PRESENT ) );
		if( pPlayTimePresentEventNode )
			pPlayTimePresentEventNode->UpdatePlayTime( pUser, GetRecordPlayTime( pRecord ) );
	}

	//�������� ����Ǹ� ����Ʈ�� ��Ҹ� �������� �ʴ´�.
	if( m_pCreator->GetRoomStyle() == RSTYLE_LADDERBATTLE )
	{
		if( !g_LadderTeamManager.IsCampBattlePlay() )
		{
			fAcquireExp = 0.0f;
			fAcquirePeso= 0.0f;
		}
	}

	pRecord->iTotalExp  = 0;
	pRecord->iTotalPeso = 0;
	pRecord->iTotalAddPeso = 0;

	// ��� ����.
	pUser->AddMoney( (int)fAcquirePeso );
	g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, pUser, 0, 0, LogDBClient::LET_MODE, LogDBClient::PT_BATTLE, GetModeType(), 0, (int)fAcquirePeso, NULL);

	pRecord->iTotalPeso = (int)fAcquirePeso;
	pRecord->iTotalAddPeso = (int)fAcquirePeso;

	int i = 0;
	enum { MAX_GET_POINT_CHAR = 5, };

	DWORDVec dwPlayTimeList;
	dwPlayTimeList.clear();

	pRecord->iResultClassTypeList.clear();
	pRecord->iResultClassPointList.clear();
	pRecord->bResultLevelUP = false;		

	if( !bAbuseUser )
	{
		pUser->SetModeConsecutively( GetModeType() );
	}

	if( !bAbuseUser )
	{
		// ������ �뺴 ������ ���� �÷��� �ð��� ���� �����Ѵ�.
		g_ItemPriceMgr.SetGradePlayTimeCollected( pRecord->pUser->GetGradeLevel(), GetRecordPlayTime( pRecord ) / 1000 );
	}

	// ���� �ð��� ���� ���� Ŭ������ ���Ѵ�.
	DWORD dwTotalTime = pRecord->GetHighPlayingTime( MAX_GET_POINT_CHAR, pRecord->iResultClassTypeList, dwPlayTimeList );
	if( dwTotalTime == 0 ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Return Playing Time Zeor!!!" );
		return;
	}

	int iListSize = pRecord->iResultClassTypeList.size();
	for(i = 0; i < MAX_GET_POINT_CHAR; i++)
	{
		if( i >= iCurMaxSlot ) break;
		if( !COMPARE( i, 0, iListSize ) ) break;

		if( pRecord->iResultClassTypeList[i] == 0 ) continue;

		float fSoldierPer = (float)dwPlayTimeList[i] / dwTotalTime;
		int iCurPoint = ( fAcquireExp * fSoldierPer ) + 0.5f;     //�ݿø�
		pRecord->iResultClassPointList.push_back( iCurPoint );
		pRecord->iTotalExp += pRecord->iResultClassPointList[i];

		// �뺴 ����ġ Ư�� ������ - ���ʽ�
		float fClassPoint = (float)pRecord->iResultClassPointList[i];
		float fClassBonus = pUser->GetSoldierExpBonus( pRecord->iResultClassTypeList[i] );
		float fSoldierExpBonus = ( fClassPoint * fClassBonus );		
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ClassType[%d] PlayTime[%d - %d - %d] - WinPoint[%d] - Bonus[%.2f]", pRecord->iResultClassTypeList[i], 
			dwPlayTimeList[i],
			dwTotalTime,
			GetRecordPlayTime( pRecord ),
			pRecord->iResultClassPointList[i], fSoldierExpBonus );

		// ����ġ ���� �� ������ Ȯ��
		if( pUser->IsClassTypeExerciseStyle( pRecord->iResultClassTypeList[i], EXERCISE_RENTAL ) == false )
			pUser->AddClassExp( pRecord->iResultClassTypeList[i], pRecord->iResultClassPointList[i] + fSoldierExpBonus );
		if( pUser->AddGradeExp( pRecord->iResultClassPointList[i] ) )
			pRecord->bResultLevelUP = true;

		// 
		pRecord->iResultClassPointList[i] += fSoldierExpBonus;

		if( pUser->IsClassTypeExerciseStyle( pRecord->iResultClassTypeList[i], EXERCISE_RENTAL ) )
		{
			// ��� ����ġ�� ȹ���ϰ� �뺴 ����ġ�� ȹ�� �ȵ�
			pRecord->iResultClassPointList[i] = 0;
		}
	}
	// �뺴�ܰ� �뺴���� ������ ������ �����Ѵ�.
	pRecord->iTotalPeso += pUser->GradeNClassUPBonus();	

	//�̼�
	static DWORDVec vValues;
	vValues.clear();
	if( pUser )
	{
		if( pRecord && m_pCreator )
		{
			vValues.push_back(pRecord->GetAllPlayingTime());
			vValues.push_back(m_pCreator->GetRoomStyle());
			vValues.push_back(GetModeType());

			g_MissionMgr.DoTrigger(MISSION_CLASS_MODEPLAY, pUser, vValues);
		}
	}
}


void RaidMode::OnNpcSpawn( SP2Packet &rkPacket )
{
	int nNpc = 0;
	static int nNpcName = 0;

	PACKET_GUARD_VOID(rkPacket.Read(nNpc));

	if( nNpc > 0)
	{
		SP2Packet kPacket( STPK_SPAWN_MONSTER );
		PACKET_GUARD_VOID(kPacket.Write(nNpc));

		for(int i = 0; i < nNpc; i++)
		{
			int nNpcCode;
			PACKET_GUARD_VOID(rkPacket.Read(nNpcCode));

			char szNpcName[MAX_PATH] = "";
			sprintf_s( szNpcName, " -N%d- ", ++m_nNpcCount );  // Spawn Npc Name.

			ioHashString szSyncUser = SearchMonsterSyncUser();
			int eTeam = (int)TEAM_RED;
			int dwStartTime;
			float fStartXPos, fStartZPos;

			PACKET_GUARD_VOID(rkPacket.Read(eTeam));
			PACKET_GUARD_VOID(rkPacket.Read(dwStartTime));
			PACKET_GUARD_VOID(rkPacket.Read(fStartXPos));
			PACKET_GUARD_VOID(rkPacket.Read(fStartZPos));


			MonsterRecord rkMonster;
			ZeroMemory(&rkMonster, sizeof(MonsterRecord));

			rkMonster.szName = szNpcName;
			rkMonster.eTeam = (TeamType)eTeam;
			rkMonster.dwStartTime = dwStartTime;
			rkMonster.fStartXPos = fStartXPos;
			rkMonster.fStartZPos = fStartZPos;
			rkMonster.dwCode = nNpcCode;
			rkMonster.szSyncUser = szSyncUser;
			rkMonster.nGroupIdx = 100;
			rkMonster.nNpcType = 0;
			rkMonster.eState = RS_PLAY;
			rkMonster.bEndBoss = false;

			HighTurnData &rkHighTurn = m_HighTurnList[0];
			MonsterRecordList &vecRecord = rkHighTurn.m_TurnData[0].m_vMonsterList;

			vecRecord.push_back(rkMonster);

			PACKET_GUARD_VOID(kPacket.Write(nNpcCode));
			PACKET_GUARD_VOID(kPacket.Write(szNpcName));
			PACKET_GUARD_VOID(kPacket.Write(szSyncUser));
			PACKET_GUARD_VOID(kPacket.Write(eTeam));
			PACKET_GUARD_VOID(kPacket.Write(5));
			PACKET_GUARD_VOID(kPacket.Write(false));
			PACKET_GUARD_VOID(kPacket.Write(100));
			PACKET_GUARD_VOID(kPacket.Write(dwStartTime));
			PACKET_GUARD_VOID(kPacket.Write(fStartXPos));
			PACKET_GUARD_VOID(kPacket.Write(fStartZPos));

		}

		SendRoomAllUser(kPacket);
	}
}


void RaidMode::OnExitRoom( User *pSend, SP2Packet &rkPacket )
{
	bool bExitLobby;
	int iExitType, iPenaltyPeso;
	PACKET_GUARD_VOID(rkPacket.Read(iExitType));
	PACKET_GUARD_VOID(rkPacket.Read(iPenaltyPeso));
	PACKET_GUARD_VOID(rkPacket.Read(bExitLobby));

	pSend->SetExitPosition( bExitLobby );

	if( !IsEnableState( pSend ) && iExitType == EXIT_ROOM_LOBBY )
		return;	

	if( iExitType == EXIT_ROOM_KEY_ABUSE ||
		iExitType == EXIT_ROOM_DAMAGE_ABUSE ||
		iExitType == EXIT_ROOM_SPEEDHACK ||
		iExitType == EXIT_ROOM_MACRO_OUT )
	{
		bool bNoBattle = false;
		if( GetModeType() == MT_TRAINING || GetModeType() == MT_HEADQUARTERS || GetModeType() == MT_HOUSE )
		{
			bNoBattle = true;
		}

		int iDownPeso = 0;
		if( !bNoBattle )
		{
			ModeRecord *pRecord = FindModeRecord( pSend );
			if( pRecord )
			{
				pRecord->AddPlayingTime();
			}

			if( iPenaltyPeso < 0 )
			{
				//				HackLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s PenaltyPeso Hack :%d:%s:%s:%d->10000", __FUNCTION__, pSend->GetUserIndex(), pSend->GetPublicID().c_str(), pSend->GetPrivateID().c_str(), iPenaltyPeso );
				iPenaltyPeso = 10000; // ������ ū ��
			}
			iDownPeso = pSend->DecreasePenaltyPeso( iPenaltyPeso );
			int iMinusLadderPoint = DecreasePenaltyLadderPoint( pSend );

			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetMoney()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetLadderPoint()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(iPenaltyPeso));
			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(true));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)GetModeType()));
				PACKET_GUARD_VOID(kPenaltyPacket.Write(m_iLosePoint));
			}
			else
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(false));
			}

			pSend->SendMessage( kPenaltyPacket );

			// log			
			g_LogDBClient.OnInsertPeso( pSend, -iDownPeso, LogDBClient::PT_EXIT_ROOM );
		}

		pSend->LeaveRoom( true );
		if( pSend->IsBattleRoom() )      
			pSend->LeaveBattleRoom();
		else if( pSend->IsLadderTeam() )
			pSend->LeaveLadderTeam();

		if( iExitType == EXIT_ROOM_SPEEDHACK )
		{
			SP2Packet kPacket( STPK_EXIT_ROOM );
			PACKET_GUARD_VOID(kPacket.Write(EXIT_ROOM_SPEEDHACK));
			PACKET_GUARD_VOID(kPacket.Write(-1));
			PACKET_GUARD_VOID(kPacket.Write(bNoBattle));
			PACKET_GUARD_VOID(kPacket.Write(-1));

			pSend->SendMessage( kPacket );
		}
		else
		{
			pSend->ExitRoomToTraining( iExitType, !bNoBattle );
		}
	}
	else if( iExitType == EXIT_ROOM_QUICK_OUT || 
		iExitType == EXIT_ROOM_CHAR_LIMIT ||
		iExitType == EXIT_ROOM_MONSTER_COIN_LACK ||
		iExitType == EXIT_ROOM_RAID_COIN_LACK ||
		iExitType == EXIT_ROOM_BAD_NETWORK )
	{
		if( iExitType == EXIT_ROOM_BAD_NETWORK )
		{
			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetMoney()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetLadderPoint()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(0));
			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(true));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)GetModeType()));
				// �й� ����Ʈ ����
				PACKET_GUARD_VOID(kPenaltyPacket.Write(m_iLosePoint));
			}
			else
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(false));
			}
			pSend->SendMessage( kPenaltyPacket );

			pSend->LeaveRoom( RoomParent::RLT_BADNETWORK );
		}
		else
			pSend->LeaveRoom();

		//��Ƽ���� ��� ��Ƽ Ż�� ����.
		LeavePartyByStyle( pSend, m_pCreator->GetRoomStyle() );

		pSend->ExitRoomToTraining( iExitType, false );
	}
	else if( iExitType == EXIT_ROOM_RESERVED || iExitType == EXIT_ROOM_CANCELED ) //���� ó��
	{
		if( GetState() != MS_PLAY )
			return;

		int iResult;

		if( pSend->ToggleExitRoomReserve() )
			iResult = EXIT_SURRENDER;
		else
			iResult = EXIT_NEVER_SURRENDER;

		bool bCheckEnd = true;
		int iRecordCnt = GetRecordCnt();
		for(int i = 0;i < iRecordCnt;i++ )       
		{
			ModeRecord *pRecord = FindModeRecord( i );

			if(!pRecord) continue;
			if(!pRecord->pUser) continue;
			if(pRecord->pUser->IsObserver()) continue;

			if(!pRecord->pUser->IsExitRoomReserved())
			{
				bCheckEnd = false;
				break;
			}
		}

		if( bCheckEnd)
		{
			for(int i = 0;i < iRecordCnt;i++ )       
			{
				ModeRecord *pRecord = FindModeRecord( i );

				if(!pRecord) continue;
				if(!pRecord->pUser) continue;
				if(pRecord->pUser->IsObserver()) continue;

				pRecord->pUser->ToggleExitRoomReserve();
			}

			WinTeamType eWinTeam = WTT_RED_TEAM;

			SetRoundEndInfo( eWinTeam );
			SendRoundResult( eWinTeam );
			return;
		}
		else
		{
			SP2Packet kPacket( STPK_EXIT_ROOM );
			PACKET_GUARD_VOID(kPacket.Write(iResult));
			PACKET_GUARD_VOID(kPacket.Write(-1));
			PACKET_GUARD_VOID(kPacket.Write(false));
			PACKET_GUARD_VOID(kPacket.Write(pSend->GetPublicID()));
			SendRoomAllUser(kPacket);		
		}
	}
	else
	{
		bool bPenalty = false;
		if( !pSend->IsExitRoomDelay() && iExitType == EXIT_ROOM_PENALTY )
		{
			ModeRecord *pRecord = FindModeRecord(pSend);
			if( pRecord )
			{
				pRecord->AddPlayingTime();
			}

			if( iPenaltyPeso < 0 )
			{
				//				HackLOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s PenaltyPeso Hack :%d:%s:%s:%d->10000", __FUNCTION__, pSend->GetUserIndex(), pSend->GetPublicID().c_str(), pSend->GetPrivateID().c_str(), iPenaltyPeso );
				iPenaltyPeso = 10000; // ������ ū ��
			}
			int iDownPeso = pSend->DecreasePenaltyPeso( iPenaltyPeso );
			int iMinusLadderPoint = DecreasePenaltyLadderPoint( pSend );

			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetMoney()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(pSend->GetLadderPoint()));
			PACKET_GUARD_VOID(kPenaltyPacket.Write(iPenaltyPeso));
			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(true));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()));
				PACKET_GUARD_VOID(kPenaltyPacket.Write((int)GetModeType()));
				// �й� ����Ʈ ����
				PACKET_GUARD_VOID(kPenaltyPacket.Write(m_iLosePoint));
			}
			else
			{
				PACKET_GUARD_VOID(kPenaltyPacket.Write(false));
			}
			pSend->SendMessage( kPenaltyPacket );

			bPenalty = true;
			g_LogDBClient.OnInsertPeso( pSend, -iDownPeso, LogDBClient::PT_EXIT_ROOM );
		}

		pSend->ClearPreRoomNum();
		pSend->LeaveRoom();

		//��Ƽ���� ��� ��Ƽ Ż�� ����.
		LeavePartyByStyle( pSend, m_pCreator->GetRoomStyle() );

		pSend->ExitRoomToTraining( iExitType, bPenalty );
	}
}

void RaidMode::CheckResultPieceInfo()
{
	if( m_UserResultPieceInfoMap.empty() )
		return;

	ResultPieceInfoMap::iterator iter = m_UserResultPieceInfoMap.begin();

	int iDifficulty = m_iModeSubNum;

	while( iter != m_UserResultPieceInfoMap.end() )
	{
		g_LogDBClient.OnInsertAddAlchemicPiece( iter->first, iter->second.m_ID, iter->second.m_iLevel,
			iter->second.m_iPlayTime, iDifficulty, iter->second.m_iPieceCnt );

		++iter;
	}

	m_UserResultPieceInfoMap.clear();
}

TurnData * RaidMode::GetTurnData( DWORD dwHighTurnIdx, DWORD dwLowTurnIndex )
{
	if( !COMPARE( dwHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::GetTurnData(%d) : HighTurn Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), dwHighTurnIdx, (int)m_HighTurnList.size() );
		return NULL;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[dwHighTurnIdx];
	if( !COMPARE( dwLowTurnIndex, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::GetTurnData(%d) :LowTurn Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), dwLowTurnIndex, (int)rkHighTurn.m_TurnData.size() );
		return NULL;
	}

	return &rkHighTurn.m_TurnData[dwLowTurnIndex];

}

void RaidMode::SetNextTurn()
{

	if( !COMPARE( m_dwCurHighTurn, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::SetNextTurn(%d) : HighTurn Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurHighTurn, (int)m_HighTurnList.size() );
		m_dwCurHighTurn = 0;
	}

	// �ο��ϸ� �÷��ش�.
	DWORD dwLowTurnIndex = m_dwCurLowTurn + 1;

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurHighTurn];
	if( !COMPARE( dwLowTurnIndex, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RaidMode::SetNextTurn(%d) : LowTurn Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), dwLowTurnIndex, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	m_dwCurLowTurn = dwLowTurnIndex;

	// ���������� ī���� ���� ������.
	SP2Packet kPacket( STPK_RAID_PHASE_INFO );
	byte byType = RAID_PHASE_INFO_COUNT_START;
	PACKET_GUARD_VOID( kPacket.Write( byType ) );
	SendRoomAllUser( kPacket );

}

void RaidMode::ProcessPlayRaid()
{
	switch(m_eRaidPhaseType)
	{
	case RAID_PHASE_TYPE_START:
		{
			SetRaidPhaseType(RAID_PHASE_TYPE_START_SPAWN);
		}
		break;
	case RAID_PHASE_TYPE_START_SPAWN:
		{
			// ���� ����
			// ���� ������ �ο��ϸ� ����.
			StartMonsterSpawn(m_dwCurHighTurn, m_dwCurLowTurn);
			SetRaidPhaseType(RAID_PHASE_TYPE_CHECK_DIE);
		}
		break;
	case RAID_PHASE_TYPE_CHECK_DIE:
		{
			/// �������� ���� ���Ѵ��
			// ��� �׾����� ��������.
			if(CheckAllDieMonster())
			{
				SetRaidPhaseType(RAID_PHASE_TYPE_DIE_BOSS);
			}
		}
		break;
	case RAID_PHASE_TYPE_DIE_BOSS:
		{
			SetRaidPhaseType(RAID_PHASE_TYPE_CREATE_COIN);
		}
		break;
	case RAID_PHASE_TYPE_CREATE_COIN:
		{
			// �ٷ� ���λ���
			// ������ �ο��� * ������.
			int nBlueTeam = GetTeamUserCnt(TEAM_BLUE);
			int coinCnt = m_iHunterCoinCount * nBlueTeam;
			m_dwHunterCoinWaitTime = m_dwDefHunterCoinWaitTime * 2;
			GenerateHunterCoin(coinCnt);
			SetRaidPhaseType(RAID_PHASE_TYPE_COIN_WAIT);

		}
		break;
	case RAID_PHASE_TYPE_COIN_WAIT:
		{
			//// ������ ������ �ٽ� �����ؼ� ������.
			//GenerateDropHunterCoinGenerate();
			// ���λ������ �ð� ��ٸ�
			if(WaitCoinTime())
			{
				SetRaidPhaseType(RAID_PHASE_TYPE_COUNT_SPAWN);
			}
		}
		break;
	case RAID_PHASE_TYPE_COUNT_SPAWN:
		{
			// ���λ������ �ð� ��ٸ�
			SetNextTurn();
			SetRaidPhaseType(RAID_PHASE_TYPE_WAIT_SPAWN);
			break;
		}
	case RAID_PHASE_TYPE_WAIT_SPAWN:
		{
			// ī��Ʈ �� ��ٸ�
			if(WaitBossCnt())
				SetRaidPhaseType(RAID_PHASE_TYPE_START_SPAWN);
			break;
		}
	case RAID_PHASE_TYPE_END:
		{
		}
		break;

	default:
		break;
	}

}

void RaidMode::DestroyMode()
{
	CTowerDefMode::DestroyMode();
	DestoryModeItemByHunterCoin();
}

void RaidMode::DestoryModeItemByHunterCoin()
{
	for( ModeItemVec::iterator iter = m_vDropZoneHunterCoin.begin(); iter != m_vDropZoneHunterCoin.end(); ++iter )
	{
		ModeItem* pItem = *iter;
		if( pItem )
			SAFEDELETE( pItem );
	}

	m_vDropZoneHunterCoin.clear();
	m_iDropZoneUserHunterCoin = 0;
}


void RaidMode::OnGetModeItem( SP2Packet &rkPacket )
{
	if( m_ModeState != MS_PLAY )
		return;

	DWORD dwUserIndex     = 0;
	DWORD dwModeItemIndex = 0;
	PACKET_GUARD_VOID( rkPacket.Read( dwUserIndex ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwModeItemIndex ) );

	User *pUser = g_UserNodeManager.GetUserNode( dwUserIndex );
	if( !pUser )
		return;

	const ModeItem* pItem = FindModeItem( dwModeItemIndex );
	if( pItem )
	{
		SP2Packet kPacket( STPK_GET_MODE_ITEM );
		PACKET_GUARD_VOID( kPacket.Write( dwUserIndex ) );
		PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
		PACKET_GUARD_VOID( kPacket.Write( pItem->GetType() ) );
		PACKET_GUARD_VOID( kPacket.Write( dwModeItemIndex ) );

		switch( pItem->GetType() )
		{
		case MIT_HUNTER_COIN:
			{			
				MonsterSurvivalRecord * pRecord = FindMonsterSurvivalRecord( pUser );
				if(pRecord)
				{
					pRecord->m_iHunterCoinCount++;
					PACKET_GUARD_VOID( kPacket.Write( pRecord->m_iHunterCoinCount ) );

					LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL, "[Raid] : %s, HunterCoin = %d", __FUNCTION__, pRecord->m_iHunterCoinCount);
				}
			}
			break;
		}

		SendRoomAllUser( kPacket );
		DeleteModeItem( dwModeItemIndex );
	}

}

void RaidMode::FinalRoundProcessByShuffle()
{
	int iRecordCnt = GetRecordCnt();
	for( int i = 0; i < iRecordCnt; ++i )
	{
		MonsterSurvivalRecord *pRecord = dynamic_cast<MonsterSurvivalRecord *>(FindModeRecord( i ));
		if( !pRecord )
			continue;

		if( pRecord->m_iHunterCoinCount <= 0 )
			continue;

		User *pUser = pRecord->pUser;
		if( !pUser )
			continue;

		SendHunterCoin( pUser, pRecord->m_iHunterCoinCount );
	}

}

void RaidMode::LoadINIHunterCoinInfo( ioINILoader & rkLoader )
{
	// ��� ini���� ����

	rkLoader.SetTitle( "star_item" );

	m_dwHunterCoinActivateTime = (DWORD)rkLoader.LoadInt( "star_activate_time", 0 );
	m_dwDefHunterCoinWaitTime	 = (DWORD)rkLoader.LoadInt( "star_wait_time", 0 );
	m_iHunterCoinCount	 = rkLoader.LoadInt( "star_cnt_by_player", 0 );

	m_fFloatPowerMin = rkLoader.LoadFloat( "star_float_power_min", 500.0f );
	m_fFloatPowerMax = rkLoader.LoadFloat( "star_float_power_max", 1000.0f );
	m_fFloatSpeedMin = rkLoader.LoadFloat( "star_float_speed_min", 300.0f );
	m_fFloatSpeedMax = rkLoader.LoadFloat( "star_float_speed_max", 500.0f );

	m_fDropHunterCoinDecRate    = rkLoader.LoadFloat( "drop_decrease_rate", 0.0f );

	rkLoader.SetTitle( "raward_item" );
	m_iRewardType = rkLoader.LoadInt( "reward_item_type", 0 );
	m_iRewardPeriod = rkLoader.LoadInt( "reward_item_period", 0 );
	m_iRewardIndex = rkLoader.LoadInt( "reward_item_index", 0 );
	m_iRewardMent = rkLoader.LoadInt( "reward_item_ment", 0 );

}

void RaidMode::LoadMapINIHunterCoinPosInfo( ioINILoader & rkLoader )
{
	// ��ini���� �о��

	char szKey[MAX_PATH] = "";
	rkLoader.SetTitle( "shuffle_bonus_star" );

	m_iMaxHunterCoinRegenPos = rkLoader.LoadInt( "max_regen_pos", 0 );

	m_vHunterCoinRegenPos.reserve( m_iMaxHunterCoinRegenPos );

	for( int i=0; i< m_iMaxHunterCoinRegenPos; ++i )
	{		
		sprintf_s( szKey, "regen%d_x", i+1 );
		float fXPos = rkLoader.LoadFloat( szKey, 0.0f );

		sprintf_s( szKey, "regen%d_z", i+1 );
		float fZPos = rkLoader.LoadFloat( szKey, 0.0f );

		HunterCoinRegenPos kStarInfo;
		kStarInfo.m_fXPos = fXPos;
		kStarInfo.m_fZPos = fZPos;
		m_vHunterCoinRegenPos.push_back( kStarInfo );

	}
	ShuffleHunterCoin();
}

void RaidMode::GenerateHunterCoin(int coinCnt)
{

	if( m_vHunterCoinRegenPos.empty() )
		return;

	if( m_ModeState != MS_PLAY )
		return;

	IORandom eRandom;
	eRandom.Randomize();


	DWORD dwActivateTime = 0;

	SP2Packet kPacket( STPK_CREATE_MODE_ITEM );

	PACKET_GUARD_VOID( kPacket.Write( static_cast<int>( MIT_HUNTER_COIN ) ) );
	PACKET_GUARD_VOID( kPacket.Write( coinCnt ) );
	PACKET_GUARD_VOID( kPacket.Write( m_dwDefHunterCoinWaitTime ) );

	LOOP_GUARD();
	for( int i = 0; i < coinCnt; ++i )
	{		
		ModeItem* pItem = CreateModeItem( MIT_HUNTER_COIN );
		if( !pItem )		
			continue;		

		PACKET_GUARD_VOID( kPacket.Write( pItem->m_dwModeItemIdx ) );		

		PACKET_GUARD_VOID( kPacket.Write( m_vHunterCoinRegenPos[m_iCurHunterCoinRegenPos].m_fXPos ) );
		PACKET_GUARD_VOID( kPacket.Write( m_vHunterCoinRegenPos[m_iCurHunterCoinRegenPos].m_fZPos ) );

		int iAngle = eRandom.Random( 360 );
		PACKET_GUARD_VOID( kPacket.Write( iAngle ) );

		float fSpeed = eRandom.Random( m_fFloatSpeedMax - m_fFloatSpeedMin ) + m_fFloatSpeedMin;
		PACKET_GUARD_VOID( kPacket.Write( fSpeed ) );

		float fPower = eRandom.Random( m_fFloatPowerMax - m_fFloatPowerMin) + m_fFloatPowerMin;
		PACKET_GUARD_VOID( kPacket.Write( fPower ) );

		PACKET_GUARD_VOID( kPacket.Write( dwActivateTime ) );
		dwActivateTime += m_dwHunterCoinActivateTime;

		m_iCurHunterCoinRegenPos++;
		if( m_iCurHunterCoinRegenPos >= m_iMaxHunterCoinRegenPos )
		{
			m_iCurHunterCoinRegenPos = 0;
		}
	}
	m_pCreator->RoomSendPacketTcp( kPacket );
	LOOP_GUARD_CLEAR();

	m_dwHunterCoinWaitTime += dwActivateTime + m_dwHunterCoinActivateTime;
}

void RaidMode::ShuffleHunterCoin()
{
	std::random_shuffle( m_vHunterCoinRegenPos.begin(), m_vHunterCoinRegenPos.end() );
}


void RaidMode::SendHunterCoin( User * pUser, int coinCnt )
{
	int iType   = GetRewardType();
	int iPeriod = GetRewardPeriod();
	int iIndex  = GetRewardIndex();
	int iMent = GetRewardMent();

	CTimeSpan cPresentGapTime( iPeriod, 0, 0, 0 );
	CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;

	pUser->AddPresentMemory( "������K", iType, iIndex, coinCnt, 0, 0, iMent, kPresentTime, 0 );
	pUser->SendPresentMemory();

	g_LogDBClient.OnInsertPresent( 0, "������K", g_App.GetPublicIP().c_str(), pUser->GetUserIndex(), iType, iIndex, coinCnt, 0, 0, LogDBClient::PST_RECIEVE, "RaidReward" );
}

ModeItem* RaidMode::FindModeItemByHunterCoinDrop( DWORD dwModeItemIndex )
{	
	if( m_vDropZoneHunterCoin.empty() )
		return NULL;

	for( ModeItemVec::iterator iter = m_vDropZoneHunterCoin.begin(); iter != m_vDropZoneHunterCoin.end(); ++iter )
	{
		ModeItem* pItem = *iter;
		if( pItem && pItem->m_dwModeItemIdx == dwModeItemIndex )
			return pItem;
	}

	return NULL;
}

void RaidMode::OnDropZoneDropByUser( SP2Packet &rkPacket )
{
	ioHashString szName;
	PACKET_GUARD_VOID( rkPacket.Read( szName ) );

	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( szName );
	if( pRecord && pRecord->pUser )
	{		
		int iDecreaseHunterCoin = (int)max( 0.0f, (float)pRecord->m_iHunterCoinCount * m_fDropHunterCoinDecRate );		
		if( iDecreaseHunterCoin > pRecord->m_iHunterCoinCount )
		{		
			iDecreaseHunterCoin = pRecord->m_iHunterCoinCount;
		}

		pRecord->m_iHunterCoinCount = pRecord->m_iHunterCoinCount - iDecreaseHunterCoin;
		m_iDropZoneUserHunterCoin += iDecreaseHunterCoin;

		SP2Packet kPacket( STPK_SHUFFLEROOM_DROPZONE );
		PACKET_GUARD_VOID( kPacket.Write( SHUFFLEROOM_DROP_USER ) );
		PACKET_GUARD_VOID( kPacket.Write( pRecord->pUser->GetPublicID() ) );
		PACKET_GUARD_VOID( kPacket.Write( pRecord->m_iHunterCoinCount ) );
		SendRoomAllUser( kPacket );
	}
}

void RaidMode::OnDropZoneDropByHunterCoin( SP2Packet &rkPacket )
{
	int iCount = 0;
	PACKET_GUARD_VOID( rkPacket.Read( iCount ) );

	DWORDVec dwDropVec;
	for( int i = 0; i < iCount; ++i )
	{
		DWORD dwDropItemIdx = 0;
		PACKET_GUARD_VOID( rkPacket.Read( dwDropItemIdx ) );

		if( dwDropItemIdx == 0 )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - item index == 0", __FUNCTION__ );
			continue;
		}

		ModeItem* pModeItem = FindModeItem( dwDropItemIdx );
		if( !pModeItem )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - pModeItem == NULL : %d", __FUNCTION__, dwDropItemIdx );
			continue;
		}

		const ModeItem* pDropItem = FindModeItemByHunterCoinDrop( dwDropItemIdx );
		if( pDropItem )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - exist mode item : %d", __FUNCTION__, dwDropItemIdx );
			continue;
		}

		m_vDropZoneHunterCoin.push_back( pModeItem );
		DeleteModeItem( dwDropItemIdx );

		dwDropVec.push_back( dwDropItemIdx );
	}

	if( dwDropVec.empty() )
		return;

	SP2Packet kPacket( STPK_SHUFFLEROOM_DROPZONE );
	PACKET_GUARD_VOID( kPacket.Write( SHUFFLEROOM_DROP_STAR ) );
	PACKET_GUARD_VOID( kPacket.Write( (int)dwDropVec.size() ) );
	for( int i = 0; i <(int)dwDropVec.size(); ++i )
	{
		PACKET_GUARD_VOID( kPacket.Write( dwDropVec[i] ) );
	}
	m_pCreator->RoomSendPacketTcp( kPacket );
}

void RaidMode::OnDropZoneDrop( SP2Packet &rkPacket )
{
	int iDropType = 0;
	PACKET_GUARD_VOID( rkPacket.Read( iDropType ) );

	switch( iDropType )
	{
	case SHUFFLEROOM_DROP_USER:
		{
			// ��������� ������.
			//OnDropZoneDropByUser( rkPacket );
		}
		break;
	case SHUFFLEROOM_DROP_STAR:
		{
			OnDropZoneDropByHunterCoin( rkPacket );
		}
		break;
	default:
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - invaild drop type : %d", __FUNCTION__, iDropType );
		}
		break;
	}	
}

void RaidMode::GenerateDropHunterCoinGenerate()
{
	// ������ ���������� �ִ�.
	DWORD dropStar = m_vDropZoneHunterCoin.size() + m_iDropZoneUserHunterCoin;
	if(dropStar > 0)
	{
		GenerateHunterCoin(dropStar);
		DestoryModeItemByHunterCoin();
	}
}

