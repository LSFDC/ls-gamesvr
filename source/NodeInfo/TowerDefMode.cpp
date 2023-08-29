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

#include "../EtcHelpFunc.h"
#include "../DataBase/LogDBClient.h"

#include "TowerDefMode.h"

#define TIME_SPAWN		2000
#define TCARD_MAX 10

#ifdef ANTIHACK
extern CLog CheatUser;
#endif

CTowerDefMode::CTowerDefMode( Room *pCreator, ModeType eModeType ) : MonsterSurvivalMode( pCreator )
{
	m_eModeType = eModeType;
	m_dwTreasureCardIndex = 0;
	m_dwExpUpTime = 0;
	m_dwNextExpUpTime = 0;
	m_dwExpUpAmount = 0;
	m_dwCheckSpawn = 0;
	m_fModeTurnPoint = 0.0f;

	for(int i = 0;i < MAX_TREASURE_CARD;i++)
		m_vTreasureCard.push_back( i + 1 );

	m_bTreasureStop = false;
	m_iClearMVPCardCount = 0;
}

CTowerDefMode::~CTowerDefMode()
{
}

void CTowerDefMode::LoadINIValue()
{
	Mode::LoadINIValue();

	m_UserResultPieceInfoMap.clear();
}

ModeType CTowerDefMode::GetModeType() const
{
	return m_eModeType;
}

const char* CTowerDefMode::GetModeINIFileName() const
{
	switch(m_eModeType)
	{
	case MT_TOWER_DEFENSE:
		return "config/towerdefmode.ini";
		break;

	case MT_DARK_XMAS:
		return "config/darkxmasmode.ini";
		break;

	case MT_FIRE_TEMPLE:
		return "config/firetemplemode.ini";
		break;

	case MT_FACTORY:
		return "config/factorymode.ini";
		break;

	default:
		return "config/towerdefmode.ini";
		break;
	}
	
}

void CTowerDefMode::InitObjectGroupList()
{
	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );

	int iSubNum = GetModeSubNum();
	int iGroupNum = GetModeMapNum();
	char szBuf[MAX_PATH], szTitle[MAX_PATH];

	m_vPushStructList.clear();
	m_iPushStructIdx = 0;

	switch(m_eModeType)
	{
	case MT_TOWER_DEFENSE:
		wsprintf( szTitle, "towerdef%d_object_group%d", iSubNum, iGroupNum );
		break;

	case MT_DARK_XMAS:
		wsprintf( szTitle, "darkxmas%d_object_group%d", iSubNum, iGroupNum );
		break;

	case MT_FIRE_TEMPLE:
		wsprintf( szTitle, "firetemple%d_object_group%d", iSubNum, iGroupNum );
		break;

	case MT_FACTORY:
		wsprintf( szTitle, "factory%d_object_group%d", iSubNum, iGroupNum );
		break;

	default:
		wsprintf( szTitle, "towerdef%d_object_group%d", iSubNum, iGroupNum );
		break;
	}


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

void CTowerDefMode::ProcessPlay()
{
	Mode::ProcessRevival();
//	SendLevelUpEvent();
	CheckTurnMonster();
	CheckRoundTimePing();
	CheckFieldItemLiveTime();
	CheckItemSupply( m_dwStateChangeTime );
	CheckBallSupply( m_dwStateChangeTime );
	CheckMachineSupply( m_dwStateChangeTime );
//	CheckNeedSendPushStruct();
//	CheckRoundEnd( true );
//	CheckTurnTime();
	ProcessEvent();
	ProcessBonusAlarm();
	CheckStartCoin();
}

void CTowerDefMode::UpdateUserDieTime( User *pDier )
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


void CTowerDefMode::CheckRoundEnd( bool bProcessCall )
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

void CTowerDefMode::SetRoundEndInfo( WinTeamType eWinTeam )
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
				{
					int iUserMsCoin = pRecord->pUser->GetEtcMonsterCoin();
					pRecord->pUser->SetBeforeMonsterCoin(iUserMsCoin);

					pRecord->AddDeathTime( TIMEGETTIME() - pRecord->pUser->GetStartTimeLog() );
				}
				else
					g_LogDBClient.OnInsertTime( pRecord->pUser, LogDBClient::TT_VIEW );


				pRecord->pUser->SetStartTimeLog(0);
			}
		}
	}

	int HistorySize = m_vRoundHistory.size();
	if( m_iCurRound-1 > HistorySize )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "TowerDef::SetRoundEndInfo() m_iCurRound != m_vRoundHistory.size()" );
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


void CTowerDefMode::LoadMapINIValue()
{
	char szFileName[MAX_PATH] = "";

	switch(m_eModeType)
	{
	case MT_TOWER_DEFENSE:
		sprintf_s( szFileName, "config/sp2_towerdef_mode%d_map.ini", GetModeSubNum() );
		break;

	case MT_DARK_XMAS:
		sprintf_s( szFileName, "config/sp2_darkxmas_mode%d_map.ini", GetModeSubNum() );
		break;

	case MT_FIRE_TEMPLE:
		sprintf_s( szFileName, "config/sp2_firetemple_mode%d_map.ini", GetModeSubNum() );
		break;

	case MT_FACTORY:
		sprintf_s( szFileName, "config/sp2_factory_mode%d_map.ini", GetModeSubNum() );
		break;

	default:
		sprintf_s( szFileName, "config/sp2_towerdef_mode%d_map.ini", GetModeSubNum() );
		break;
	}


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
	// �߰� 16.2.25 kaedoc �������� ��ŷ ������.
	m_dwUseStartCoinCnt = rkLoader.LoadInt( "use_monster_coin_cnt", 0 );


	m_dwExpUpTime		= rkLoader.LoadInt( "Exp_Up_Duration", 10000 );  
	m_dwExpUpAmount		= rkLoader.LoadInt( "Exp_Up_Amount", 1000 );

	m_fDefeatRatio = rkLoader.LoadFloat("Defeat_Handicap", 0.25f);

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
	m_dwCurrentHighTurnIdx = 0;

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

	// Death NPC
	m_vDeathNPCList.clear();	
	rkLoader.SetTitle( "death_npc" );
	int iMaxDeathNPC = rkLoader.LoadInt( "max_death_npc", 0 );
	for(i = 0;i < iMaxDeathNPC;i++)
	{
		char szKey[MAX_PATH] = "";
		MonsterRecord kDeathNpc;
		// NPC �ڵ�
		sprintf_s( szKey, "monster%d_code", i + 1 );
		kDeathNpc.dwCode = rkLoader.LoadInt( szKey, 0 );

		// NPC ��� �ð�
		sprintf_s( szKey, "monster%d_start_time", i + 1 );
		kDeathNpc.dwStartTime = rkLoader.LoadInt( szKey, 0 );

		// NPC ���� ��ġ
		int iRandIndex = 0;
		kDeathNpc.fStartXPos = GetMonsterStartXPos( kDeathNpc.fStartXPos, iRandIndex );
		kDeathNpc.fStartZPos = GetMonsterStartZPos( kDeathNpc.fStartZPos, iRandIndex );

		// NPC �̸��� ������ �ڵ����� �����.
		char szNpcName[MAX_PATH] = "";
		sprintf_s( szNpcName, " -N%d- ", ++iNpcNameCount );        // �¿쿡 ������ �־ ���� �г��Ӱ� �ߺ����� �ʵ��� �Ѵ�.
		kDeathNpc.szName = szNpcName;

		m_vDeathNPCList.push_back( kDeathNpc );
	}

	//////////////////////////////////////////////////////////////////////////
	// Spawn Npc From Lair
	//////////////////////////////////////////////////////////////////////////
	m_vecLairAttr.clear();
	rkLoader.SetTitle("Spawn_Npc");
	int nMax = rkLoader.LoadInt( "max_spawn_set", 0 );

	for(i = 0;i < nMax;i++)
	{
		char szKey[MAX_PATH] = "";
		stLairAttribute stLairAttr;
		ZeroMemory(&stLairAttr, sizeof(stLairAttribute));

		sprintf_s( szKey, "spawn%d_rand_table", i + 1 );
		stLairAttr.nRandTable = rkLoader.LoadInt( szKey, 1 );

		sprintf_s( szKey, "spawn%d_rand_table_spare", i + 1 );
		stLairAttr.nRandTableSpare = rkLoader.LoadInt( szKey, 1 );

		sprintf_s( szKey, "spawn%d_group", i + 1 );
		stLairAttr.nGroupIdx = rkLoader.LoadInt( szKey, 0 );

		sprintf_s( szKey, "spawn%d_num_npc", i + 1 );
		stLairAttr.nNumNpc = rkLoader.LoadInt( szKey, 1 );

		sprintf_s( szKey, "spawn%d_max_npc", i + 1 );
		stLairAttr.nMaxNpc = rkLoader.LoadInt( szKey, 1 );

		sprintf_s( szKey, "spawn%d_team", i + 1 );
		stLairAttr.nTeam = rkLoader.LoadInt( szKey, 1 );

		sprintf_s( szKey, "spawn%d_start", i + 1 );
		stLairAttr.dwStartTime = rkLoader.LoadInt( szKey, 0 );

		sprintf_s( szKey, "spawn%d_duration", i + 1 );
		stLairAttr.dwDurationTime = rkLoader.LoadInt( szKey, 0xffff );
		stLairAttr.dwDefDurationTime = stLairAttr.dwDurationTime;

		m_vecLairAttr.push_back(stLairAttr);
	}

	m_nBlueLair = 0;
}

void CTowerDefMode::CheckTurnMonster()
{
	if( m_bRoundSetEnd ) return;

	DWORD dwCurTime = TIMEGETTIME();
	if( dwCurTime < m_dwCheckSpawn)
		return;

	m_dwCheckSpawn = dwCurTime + TIME_SPAWN;

	MonsterRecordList	vecMonster;

	for( int i = 0; i < (int)m_vecLairAttr.size(); i++)
	{
		if( dwCurTime > m_vecLairAttr[i].dwStartTime )
		{
			m_vecLairAttr[i].dwStartTime = dwCurTime +  m_vecLairAttr[i].dwDurationTime;

			if( m_vecLairAttr[i].nMaxNpc > m_vecLairAttr[i].nAliveNpc )
			{
				bool bSpawn = false;

				for(int j = 0; j <  m_vecLairAttr[i].nNumNpc; j++)
				{
					DWORD dwTable = 1;

					if((TeamType)m_vecLairAttr[i].nTeam == TEAM_RED )
						dwTable = m_vecLairAttr[i].nRandTable;
					else
						dwTable = m_vecLairAttr[i].nRandTableSpare;

					if(g_MonsterMapLoadMgr.GetRandMonster(dwTable, vecMonster, m_nNpcCount, 1, false, false))
					{
						m_vecLairAttr[i].nAliveNpc++;
						bSpawn = true;
						vecMonster[vecMonster.size()-1].eTeam = (TeamType)m_vecLairAttr[i].nTeam;
						vecMonster[vecMonster.size()-1].nGrowthLvl = m_vecLairAttr[i].nTurn;
					}
				}

				if( bSpawn )
					m_vecLairAttr[i].nTurn++;
			}
		}
	}

	if( !vecMonster.empty())
	{
		SP2Packet kPacket( STPK_SPAWN_MONSTER );
		kPacket << (int)vecMonster.size();

		for(int i = 0; i < (int)vecMonster.size(); i++)
		{
			MonsterRecord &rkMonster = vecMonster[i];

			if(rkMonster.nNpcType != 0)
				continue;

			rkMonster.eState         = RS_PLAY;
			rkMonster.szSyncUser     = SearchMonsterSyncUser();

			int iRandIndex = 0;
			rkMonster.fStartXPos     = GetMonsterStartXPos( rkMonster.fStartXPos, iRandIndex );
			rkMonster.fStartZPos     = GetMonsterStartZPos( rkMonster.fStartZPos, iRandIndex );

		if( rkMonster.dwNPCIndex == 0 ) 
			rkMonster.dwNPCIndex	 = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam << rkMonster.nGrowthLvl 
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam << rkMonster.nGrowthLvl 
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#endif
			


			HighTurnData &rkHighTurn = m_HighTurnList[0];
			MonsterRecordList &vecRecord = rkHighTurn.m_TurnData[0].m_vMonsterList;

			vecRecord.push_back(rkMonster);
		}

		SendRoomAllUser(kPacket);
	}

	vecMonster.clear();

}


void CTowerDefMode::ProcessResultWait()
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

void CTowerDefMode::ProcessResult()
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

int CTowerDefMode::AddPickCardCount( User *pUser )
{
	int iAddCount = 0;

	EventUserManager &rEventUserManager = pUser->GetEventUserMgr();
	if( pUser->IsPCRoomAuthority() )
	{
		MonsterDungeonEventUserNode *pEventNode = static_cast<MonsterDungeonEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PCROOM_MONSTER_DUNGEON ) );
		if( pEventNode )
		{
			iAddCount = pEventNode->GetAddCount( pUser );
		}
	}
	else
	{
		MonsterDungeonEventUserNode *pEventNode = static_cast<MonsterDungeonEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_MONSTER_DUNGEON ) );
		if( pEventNode )
		{
			iAddCount = pEventNode->GetAddCount( pUser );
		}
	}

	return iAddCount;
}

void CTowerDefMode::OnLastPlayRecordInfo( User *pUser, SP2Packet &rkPacket )
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
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TowerDefense::OnLastContributeInfo Recv: %s - %d - %d - %d - %d", szName.c_str(), iContribute, iUniqueTotalKill, iUniqueTotalDeath, iVictories );

			// error check
			if( iContribute == 0 )
			{
				if( iUniqueTotalKill != 0 || iUniqueTotalDeath != 0 )
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TowerDefense::OnLastContributeInfo Error: %s - %d - %d - %d - %d", szName.c_str(), iContribute, iUniqueTotalKill, iUniqueTotalDeath, iVictories );
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

			pRecord->fContributePer = (float)( iRecordCnt - iOb ) * ((float)pRecord->iContribute / iMaxContribute);
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TowerDefense::OnLastContributeInfo Per: %s - %.2f", pRecord->pUser->GetPublicID().c_str(), pRecord->fContributePer );
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

			if( COMPARE( pRecord->iCurRank - 1, 0, (int)m_vClearCardCount.size() ) )
			{
				int nIdx = pRecord->iCurRank - 1 + (GetMaxPlayer() - nBlueTeam);

				nIdx = min((int)m_vClearCardCount.size() - 1, nIdx );

				int iAddCardCount = AddPickCardCount( pRecord->pUser );
				pRecord->iTreasureCardCount = m_vClearCardCount[nIdx]+iAddCardCount;

				if(pRecord->iTreasureCardCount >= TCARD_MAX)
					pRecord->iTreasureCardCount = TCARD_MAX;

				LOG.PrintTimeAndLog(LOG_TEST_LEVEL, "%s TreasureCard %d ", pRecord->pUser->GetPublicID().c_str(), pRecord->iTreasureCardCount);
			}

			if( pRecord->iCurRank == 1 )
			{
				// ���� ����!!
				if( nBlueTeam > 2)
				{
					nMVPCard = m_iClearMVPCardCount;
					pRecord->iTreasureCardCount += m_iClearMVPCardCount;

					if(pRecord->iTreasureCardCount >= TCARD_MAX)
						pRecord->iTreasureCardCount = TCARD_MAX;
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


		// ���� ���� Ŭ�� 
		PACKET_GUARD_VOID( kPacket.Write(pRecord->iTreasureCardCount) );

		g_UserNodeManager.UpdateUserSync( pRecord->pUser );      //���� ���� ����
	}
	m_pCreator->RoomSendPacketTcp( kPacket );
}


void CTowerDefMode::SendLevelUpEvent()  // �Ⱦ�
{
	DWORD dwCurTime = TIMEGETTIME();
	if( dwCurTime > m_dwNextExpUpTime)
	{
		SP2Packet kPacket( STPK_EXP_UP_BY_TIME );
		kPacket << m_dwExpUpAmount;
		SendRoomPlayUser( kPacket );
		m_dwNextExpUpTime = dwCurTime + m_dwExpUpTime;
	}
}

void CTowerDefMode::ProcessReady()
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

	// ���� ���¿��� ��Ż�� ������ ���� üũ
	CheckUserLeaveEnd();
	
	for( int i = 0; i < (int)m_vecLairAttr.size(); i++)
		m_vecLairAttr[i].dwStartTime += dwCurTime;

	StartTurn( 0, 0 );
}


void CTowerDefMode::StartTurn( DWORD dwHighTurnIdx, DWORD dwLowTurnIndex )
{
	m_dwCurrentHighTurnIdx = dwHighTurnIdx;
	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "TowerDefense::StartTurn(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	rkHighTurn.m_dwCurrentTurnIdx = dwLowTurnIndex;
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "TowerDefense::StartTurn(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];

	
	SP2Packet kPacket( STPK_TURN_START );
	kPacket << rkTurn.m_bBossTurn << GetCurrentAllLowTurn() << GetMaxAllLowTurn();
	kPacket << rkTurn.m_dwHelpIndex << rkTurn.m_dwTurnTime;
	kPacket << m_dwCurrentHighTurnIdx << rkHighTurn.m_dwCurrentTurnIdx;
	kPacket << rkTurn.m_dwReduceNpcCreateTime;

	int i = 0;
	// ��� NPC ����
	MonsterRecordList vDeathRecord;	
	for(i = 0;i < (int)m_vDeathNPCList.size();i++)
	{
		MonsterRecord &rkMonster = m_vDeathNPCList[i];
		if( rkMonster.eState != RS_PLAY ) continue;
		
		rkMonster.eState = RS_LOADING;         //�ε� ���·� ��ȯ.
		MonsterSurvivalRecord *pSyncUserRecord = FindMonsterSurvivalRecord( rkMonster.szSyncUser );
		if( pSyncUserRecord )
			pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

		vDeathRecord.push_back( rkMonster );

// 		LOG.PrintTimeAndLog( 0, "TowerDefense::StartTurn(%d) : turn(%d), vDeathRecord(%d)", 
// 				m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, vDeathRecord.size());
	}

	int iDeathNPCCount = vDeathRecord.size();
	kPacket << iDeathNPCCount;
	for(i = 0;i < iDeathNPCCount;i++)
	{
		kPacket << vDeathRecord[i].szName;
	}

	// ���� ���� ���� ����	
	int iMaxNPC = rkTurn.m_vMonsterList.size();
	kPacket << iMaxNPC;

	m_nNpcCount = iMaxNPC;
	for(i = 0;i < iMaxNPC;i++)
	{
		MonsterRecord &rkMonster = rkTurn.m_vMonsterList[i];

		if(rkMonster.nNpcType == 0)
			continue;

		if( (rkMonster.nGroupIdx != 1) && (rkMonster.eTeam == TEAM_BLUE) )
			continue;

		rkMonster.eState         = RS_PLAY;
		rkMonster.szSyncUser     = SearchMonsterSyncUser();

		int iRandIndex = 0;
		rkMonster.fStartXPos     = GetMonsterStartXPos( rkMonster.fStartXPos, iRandIndex );
		rkMonster.fStartZPos     = GetMonsterStartZPos( rkMonster.fStartZPos, iRandIndex );

		if( rkMonster.dwNPCIndex == 0 ) 
			rkMonster.dwNPCIndex     = GetUniqueMonsterIDGenerate();
#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#endif
		
	}		
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

			pRecord->pUser->m_bUsedCoin = false;

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



void CTowerDefMode::OnDropDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster ) return;
	if( pDieMonster->eState != RS_PLAY ) return;

	// ���Ͱ� ���� ��ġ.
	float fDiePosX, fDiePosZ;
	rkPacket >> fDiePosX >> fDiePosZ;

	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	rkPacket >> szLastAttackerName >> szLastAttackerSkillName >> dwLastAttackerWeaponItemCode >> iLastAttackerTeam;

	if( pDieMonster->nNpcType == 0)
	{
		for(int i = 0; i < (int)m_vecLairAttr.size(); i++)
		{
			if( m_vecLairAttr[i].nGroupIdx == pDieMonster->nGroupIdx )
			{
				if( (m_vecLairAttr[i].nTeam == pDieMonster->eTeam) && (m_vecLairAttr[i].nAliveNpc > 0) )
				{
					m_vecLairAttr[i].nAliveNpc--;
					m_vecLairAttr[i].dwStartTime = TIMEGETTIME() +  m_vecLairAttr[i].dwDurationTime;
				}
			}
		}
	}

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
	rkPacket >> iDamageCnt;

	int iTotalDamage = 0;
	int iLastDamage = 0;
	int iBestDamage = 0;

	DamageTableList vDamageList;

	if(iDamageCnt > 100)
	{
		LOG.PrintTimeAndLog(0, "%s CTPK_DROP_DIE Error - DamageCnt:%d", __FUNCTION__, iDamageCnt);
		iDamageCnt = 0;
	}

	if( iDamageCnt > 0 )
	{
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			rkPacket >> kDamageTable.szName;
			rkPacket >> kDamageTable.iDamage;

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
	kReturn << pDieMonster->szName;
	kReturn << szLastAttackerName;
	kReturn << szLastAttackerSkillName;
	kReturn << dwLastAttackerWeaponItemCode;
	kReturn << iLastAttackerTeam;
	kReturn << szBestAttackerName;
	kReturn << fLastRate;
	kReturn << fBestRate;
	GetCharModeInfo( kReturn, pDieMonster->szName );
	GetCharModeInfo( kReturn, szLastAttackerName );
	SendRoomAllUser( kReturn );

	// ������ ���
	OnMonsterDieToItemDrop( fDiePosX, fDiePosZ, pDieMonster );	

	// ���� ������ ���
	OnMonsterDieToRewardItemDrop( fDiePosX, fDiePosZ, vDamageList, pDieMonster );

	// ���� ����
	OnMonsterDieToPresent( pDieMonster->dwPresentCode );

	// ����ġ & ��� & �ֻ��� ���� ����
	OnMonsterDieToReward( pDieMonster->szName, vDamageList, pDieMonster->dwDiceTable, pDieMonster->iExpReward, pDieMonster->iPesoReward );

	// ���� ���� Ÿ�� ó��
	OnMonsterDieToTypeProcess( pDieMonster->dwDieType );

	m_szLastKillerName = szLastAttackerName;
}


void CTowerDefMode::OnWeaponDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster )
		return;

	if( pDieMonster->eState != RS_PLAY )
		return;

	// ���Ͱ� ���� ��ġ.
	float fDiePosX, fDiePosZ;
	rkPacket >> fDiePosX >> fDiePosZ;

	// Killer ���� ����.
	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	rkPacket >> szLastAttackerName >> szLastAttackerSkillName >> dwLastAttackerWeaponItemCode >> iLastAttackerTeam;

	if( pDieMonster->nNpcType == 0)
	{
		for(int i = 0; i < (int)m_vecLairAttr.size(); i++)
		{
			if( m_vecLairAttr[i].nGroupIdx == pDieMonster->nGroupIdx )
			{
				if( (m_vecLairAttr[i].nTeam == pDieMonster->eTeam) && (m_vecLairAttr[i].nAliveNpc > 0) )
				{
					m_vecLairAttr[i].nAliveNpc--;
					m_vecLairAttr[i].dwStartTime = TIMEGETTIME() +  m_vecLairAttr[i].dwDurationTime;
				}
			}
		}
	}

	// ���� ���� ó��
	{
		pDieMonster->eState = RS_DIE;
		pDieMonster->dwCurDieTime = TIMEGETTIME();
		MonsterSurvivalRecord *pSyncUserRecord = FindMonsterSurvivalRecord( pDieMonster->szSyncUser );
		if( pSyncUserRecord )
			pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );
	}

	// ������ ����Ʈ ó��
	int iDamageCnt = 0;
	ioHashString szBestAttackerName;
	rkPacket >> iDamageCnt;

	int iTotalDamage = 0;
	int iLastDamage = 0;
	int iBestDamage = 0;

	DamageTableList vDamageList;

	if(iDamageCnt > 100)
	{
		LOG.PrintTimeAndLog(0, "%s CTPK_DROP_DIE Error - DamageCnt:%d", __FUNCTION__, iDamageCnt);
		iDamageCnt = 0;
	}

	if( iDamageCnt > 0 )
	{
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			rkPacket >> kDamageTable.szName;
			rkPacket >> kDamageTable.iDamage;

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
	kReturn << pDieMonster->szName;
	kReturn << szLastAttackerName;
	kReturn << szLastAttackerSkillName;
	kReturn << dwLastAttackerWeaponItemCode;
	kReturn << iLastAttackerTeam;
	kReturn << szBestAttackerName;
	kReturn << fLastRate;
	kReturn << fBestRate;
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

	// ������ ���
	OnMonsterDieToItemDrop( fDiePosX, fDiePosZ, pDieMonster );	

	// ���� ������ ���
	OnMonsterDieToRewardItemDrop( fDiePosX, fDiePosZ, vDamageList, pDieMonster );

	// ���� ����
	OnMonsterDieToPresent( pDieMonster->dwPresentCode );

	// ����ġ & ��� & �ֻ��� ���� ����
	OnMonsterDieToReward( pDieMonster->szName, vDamageList, pDieMonster->dwDiceTable, pDieMonster->iExpReward, pDieMonster->iPesoReward );

	// ���� ���� Ÿ�� ó��
	OnMonsterDieToTypeProcess( pDieMonster->dwDieType );

	m_szLastKillerName = szLastAttackerName;

	if(pDieMonster->bGroupBoss)
	{
		SendReviveMonstersEvent(pDieMonster, iLastAttackerTeam);
	}

	if(pDieMonster->bEndBoss)
	{
		OnforciblyUseMonsterCoin();

		WinTeamType eWinTeam = WTT_RED_TEAM;
		if( pDieMonster->eTeam == TEAM_RED)
			eWinTeam = WTT_BLUE_TEAM;

		SetRoundEndInfo( eWinTeam );
		SendRoundResult( eWinTeam );
		return;
	}
}

void CTowerDefMode::SendRoundResult( WinTeamType eWinTeam )
{
	SP2Packet kPacket( STPK_ROUND_END );
	kPacket << eWinTeam;
	kPacket << m_iRedTeamWinCnt;
	kPacket << m_iBlueTeamWinCnt;

	kPacket << GetPlayingUserCnt();

	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord || pRecord->eState == RS_LOADING )
			continue;

		kPacket << pRecord->pUser->GetPublicID();

		//
		int iMyVictories = 0;
		if( pRecord->pUser )
		{
			if( m_bRoundSetEnd && eWinTeam != WTT_DRAW && eWinTeam != WTT_NONE )
				pRecord->pUser->IncreaseMyVictories( IsWinTeam(eWinTeam, pRecord->pUser->GetTeam()) );

			iMyVictories = pRecord->pUser->GetMyVictories();
		}

		kPacket << iMyVictories;
		//

		//
		int iKillSize = pRecord->iKillInfoMap.size();
		kPacket << iKillSize;

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_k = pRecord->iKillInfoMap.begin();
		while( iter_k != pRecord->iKillInfoMap.end() )
		{
			kPacket << iter_k->first;
			kPacket << iter_k->second;

			++iter_k;
		}
		LOOP_GUARD_CLEAR();

		int iDeathSize = pRecord->iDeathInfoMap.size();
		kPacket << iDeathSize;

		LOOP_GUARD();
		KillDeathInfoMap::iterator iter_d = pRecord->iDeathInfoMap.begin();
		while( iter_d != pRecord->iDeathInfoMap.end() )
		{
			kPacket << iter_d->first;
			kPacket << iter_d->second;

			++iter_d;
		}
		LOOP_GUARD_CLEAR();
		//

		kPacket << pRecord->iCurRank;
		kPacket << pRecord->iPreRank;
	}

	kPacket << m_bRoundSetEnd;

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

#ifdef ANTIHACK
	SendRelayGroupTeamWinCnt();
#endif
}

void CTowerDefMode::UpdateUserDieNextProcess( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{

}

void CTowerDefMode::SetBalanceNpcSpawnDuration(int nTeam)
{
	if(nTeam == TEAM_BLUE)
		m_nBlueLair++;
	else
		m_nBlueLair--;

	for( int i = 0; i < (int)m_vecLairAttr.size(); i++)
	{
		m_vecLairAttr[i].dwDurationTime = m_vecLairAttr[i].dwDefDurationTime;

		if( m_vecLairAttr[i].nTeam == TEAM_RED) 
		{
			m_vecLairAttr[i].dwDurationTime = m_vecLairAttr[i].dwDurationTime - (m_nBlueLair * 1000);
			m_vecLairAttr[i].dwDurationTime = max(5000, m_vecLairAttr[i].dwDurationTime);
		}
	}
}

void CTowerDefMode::AddNewRecord( User *pUser )
{
	MonsterSurvivalRecord kRecord;
	kRecord.pUser = pUser;
	m_vRecordList.push_back( kRecord );

	kRecord.StartPlaying();
	UpdateUserRank();

	// �߰� ���� ���� NPC ����ȭ
	PlayMonsterSync( &kRecord );
}

void CTowerDefMode::PlayMonsterSync( MonsterSurvivalRecord *pSendRecord )
{
	if( m_bRoundSetEnd ) return;
	if( GetState() != MS_PLAY ) return;        // �÷����� ������ �����鿡�Ը� ����ȭ ��Ų��.
	if( pSendRecord == NULL || pSendRecord->pUser == NULL ) return;

	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
		return;

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
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
	kPacket << iSyncSize;
	for(i = 0;i < iSyncSize;i++)
	{
		MonsterRecord &rkMonster = vSyncRecord[i];
		if( rkMonster.dwNPCIndex == 0 )
			rkMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.eTeam
			<< rkMonster.bGroupBoss << rkMonster.nGroupIdx << rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#endif
		
	}

	pSendRecord->pUser->SendMessage( kPacket ); 
	vSyncRecord.clear();
}


bool CTowerDefMode::SendReviveMonstersEvent(const MonsterRecord *pDieMonster, int nTeam)
{
	HighTurnData &rkHighTurn = m_HighTurnList[0];
	MonsterRecordList &vecRecord = rkHighTurn.m_TurnData[0].m_vMonsterList;

	int nGroup = pDieMonster->nGroupIdx;

	for( int i = 0; i < (int)m_vecLairAttr.size(); i++)
	{
		if(m_vecLairAttr[i].nGroupIdx == nGroup)
		{
			m_vecLairAttr[i].nTeam = nTeam;
			m_vecLairAttr[i].nTurn = 0;
			m_vecLairAttr[i].nAliveNpc = 0;
			m_vecLairAttr[i].dwStartTime = TIMEGETTIME() +  m_vecLairAttr[i].dwDurationTime;
		}
	}

//	SetBalanceNpcSpawnDuration(nTeam);

	
	SP2Packet kPacket( STPK_RESET_NPCTOWER );

	int nDeadNpc = 0;

	for(int i = 0; i < (int)vecRecord.size(); i++)
	{
		MonsterRecord &stMonster = vecRecord[i];

		if( (stMonster.nNpcType != 0) && (nGroup == stMonster.nGroupIdx) && (!stMonster.bGroupBoss) ) 
		{
			if( stMonster.eTeam != nTeam)
			{
				stMonster.eState = RS_DIE;
				kPacket << stMonster.szName;
				nDeadNpc++;
			}
		}
	}


	int nNpc = 0;

	for(int i = 0; i < (int)vecRecord.size(); i++)
	{
		MonsterRecord &stMonster = vecRecord[i];

		if( (stMonster.nNpcType != 0) && (nGroup == stMonster.nGroupIdx)) 
		{
			if( nTeam == TEAM_BLUE)
			{
				if( stMonster.eTeam == TEAM_RED)
					continue;

				stMonster.dwCurDieTime = 0;
				stMonster.eState	= RS_PLAY;
				stMonster.szSyncUser= SearchMonsterSyncUser();

				if( stMonster.dwNPCIndex == 0 )
					stMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();
#ifdef ANTIHACK
				kPacket << stMonster.dwNPCIndex << stMonster.dwCode << stMonster.szName << nTeam << stMonster.szSyncUser << stMonster.fStartXPos << stMonster.fStartZPos;
#else
				kPacket << stMonster.dwCode << stMonster.szName << nTeam << stMonster.szSyncUser << stMonster.fStartXPos << stMonster.fStartZPos;
#endif
				nNpc++;
			}
			else if(nTeam == TEAM_RED )
			{
				if( stMonster.eTeam == TEAM_BLUE)
					continue;

				stMonster.dwCurDieTime = 0;
				stMonster.eState	= RS_PLAY;
				stMonster.szSyncUser= SearchMonsterSyncUser();
				
				if( stMonster.dwNPCIndex == 0 )
					stMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();
#ifdef ANTIHACK
				kPacket << stMonster.dwNPCIndex << stMonster.dwCode << stMonster.szName << nTeam << stMonster.szSyncUser << stMonster.fStartXPos << stMonster.fStartZPos;
#else
				kPacket << stMonster.dwCode << stMonster.szName << nTeam << stMonster.szSyncUser << stMonster.fStartXPos << stMonster.fStartZPos;
#endif
				nNpc++;
			}
			
		}
	}

	if( nNpc || nDeadNpc)
	{
		SP2Packet cPacket( STPK_RESET_NPCTOWER );
		cPacket << nDeadNpc << nNpc;
		cPacket.SetDataAdd( (char*)kPacket.GetData(), kPacket.GetDataSize() );
		SendRoomAllUser( cPacket );
	}

	return ( ((nNpc>0) || (nDeadNpc>0)) ? true : false);
}




bool CTowerDefMode::ProcessTCPPacket( User *pSend, SP2Packet &rkPacket )
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
	}

	return false;
}


void CTowerDefMode::OnforciblyUseMonsterCoin()
{
	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pMapRecord = FindModeRecord( i );
		if( pMapRecord )
		{
			MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pMapRecord->pUser );

			if( !pRecord ) continue;
			if( !pRecord->pUser ) continue;

			if(pRecord->pUser->m_bUsedCoin == true)
			{
				pRecord->pUser->m_bUsedCoin = false;
			}
			else
			{
				if( pRecord->eState != RS_VIEW && pRecord->eState != RS_OBSERVER )
				{
					/*
#ifdef ANTIHACK
					CheatUser.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterCoin Cheating User :%d:%s:%s = MonsterCoin before: %d, After: %d", 
						pRecord->pUser->GetUserIndex(), pRecord->pUser->GetPublicID().c_str(), pRecord->pUser->GetPrivateID().c_str(), pRecord->pUser->GetBeforeMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );
#endif
						*/
				}	
			}
		}
	}
}

void CTowerDefMode::ExitRoom( User *pSend, bool bExitLobby, int iExitType, int iPenaltyPeso )
{
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
			//hr ��ƾ���� �����
			pSend->IncreaseGiveupCount();

			iDownPeso = pSend->DecreasePenaltyPeso( iPenaltyPeso );
			int iMinusLadderPoint = DecreasePenaltyLadderPoint( pSend );

			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetMoney()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetLadderPoint()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(iPenaltyPeso) );

			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(true) ); 
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)GetModeType()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write(m_iLosePoint) );      // �й� ����Ʈ ����
			}
			else
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(false) );
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
			PACKET_GUARD_VOID( kPacket.Write(EXIT_ROOM_SPEEDHACK) );
			PACKET_GUARD_VOID( kPacket.Write(-1) );
			PACKET_GUARD_VOID( kPacket.Write(!bNoBattle) );
			PACKET_GUARD_VOID( kPacket.Write(-1) );

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
		iExitType == EXIT_ROOM_BAD_NETWORK )
	{
		if( iExitType == EXIT_ROOM_BAD_NETWORK )
		{
			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetMoney()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetLadderPoint()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(0) );
			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(true) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)GetModeType()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write(m_iLosePoint) );      // �й� ����Ʈ ����
			}
			else
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(false) );
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
			PACKET_GUARD_VOID( kPacket.Write(iResult) );
			PACKET_GUARD_VOID( kPacket.Write(-1) );
			PACKET_GUARD_VOID( kPacket.Write(false) );
			PACKET_GUARD_VOID( kPacket.Write(pSend->GetPublicID()) );
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
			//hr ��ƾ���� �����
			pSend->IncreaseGiveupCount();

			int iDownPeso = pSend->DecreasePenaltyPeso( iPenaltyPeso );
			int iMinusLadderPoint = DecreasePenaltyLadderPoint( pSend );

			SP2Packet kPenaltyPacket( STPK_EXIT_ROOM_PENALTY );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetMoney()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(pSend->GetLadderPoint()) );
			PACKET_GUARD_VOID( kPenaltyPacket.Write(iPenaltyPeso) );

			// ���������� ��Ż�ÿ� �з� ���
			if( SetLeaveUserPenalty( pSend ) )
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(true) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)m_pCreator->GetRoomStyle()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write((int)GetModeType()) );
				PACKET_GUARD_VOID( kPenaltyPacket.Write(m_iLosePoint) );      // �й� ����Ʈ ����
			}
			else
			{
				PACKET_GUARD_VOID( kPenaltyPacket.Write(false) );
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

void CTowerDefMode::CheckStartCoin()
{
	if(m_bRoundSetEnd)
		return;
	if(m_dwUseStartCoinCnt == 0)
		return;
	if(m_dwStartCoinTime == 0)
		return;

	DWORD curTime = TIMEGETTIME();

	DWORD recordSize = m_vRecordList.size();
	for(int i = 0; i < recordSize; ++i)
	{
		MonsterSurvivalRecord & rkRecord = m_vRecordList[i];
		if(rkRecord.pUser == NULL)
			continue;
		if(rkRecord.pUser->IsObserver() || rkRecord.pUser->IsStealth())
			continue;
		if(rkRecord.bStartCoinDec)
			continue;
		if(rkRecord.dwPlayingStartTime == 0)
			continue;

		if(m_dwStartCoinTime < (curTime - rkRecord.dwPlayingStartTime) )
		{
			// ���� ���ڶ�� �ٷ� ƨ��..
			int curMonsterCoinCnt = rkRecord.pUser->GetEtcMonsterCoin() + rkRecord.pUser->GetEtcGoldMonsterCoin();
			if(curMonsterCoinCnt < m_dwUseStartCoinCnt)
			{
				rkRecord.bStartCoinDec = true;
				ExitRoom(rkRecord.pUser, true, EXIT_ROOM_MONSTER_COIN_LACK, 0);
			}
			else
			{
				rkRecord.bStartCoinDec = true;
				UseMonsterCoin(rkRecord.pUser, USE_MONSTER_COIN_START);
			}
		}
	}
}

void CTowerDefMode::UseMonsterCoin( User *pUser, int iUseCommand )
{
	if( !IsEnableState( pUser ) )
		return;

	if( GetState() != MS_PLAY ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CTowerDefMode::OnUseMonsterCoin None Play State : %s", pUser->GetPublicID().c_str() );
		return;
	}

	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
	if( !pRecord ) return;
	if( !pRecord->pUser ) return;	

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "CTowerDefMode::OnUseMonsterCoin Prev Coin Count %d: %s - (%d:%d)", iUseCommand, pRecord->pUser->GetPublicID().c_str(), 
		pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );

	if( iUseCommand == USE_MONSTER_COIN_START)
	{
		// �������� ���� 16.2.25 kaedoc �������� ��ŷ ������.
		//int iUseStartCoinCnt;
		//rkPacket >> iUseStartCoinCnt;

		bool bUseGoldCoin = false;
		// ���� ���� �켱 ���
		if( !pRecord->pUser->UseModeStartMonsterCoin( m_dwUseStartCoinCnt, bUseGoldCoin ) )
		{
			// ���� ���� �˸�
			SP2Packet kPacket( STPK_USE_MONSTER_COIN );
			kPacket << USE_MONSTER_COIN_FAIL_CNT;
			kPacket << pRecord->pUser->GetEtcMonsterCoin() << pRecord->pUser->GetEtcGoldMonsterCoin();
			pRecord->pUser->SendMessage( kPacket );
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "CTowerDefMode::OnUseMonsterCoin None Coin Count %d: %s - %d(%d:%d)", iUseCommand, 
				pRecord->pUser->GetPublicID().c_str(), m_dwUseStartCoinCnt, pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );
			return;
		}	

		pRecord->pUser->m_bUsedCoin = true;

		SP2Packet kPacket( STPK_USE_MONSTER_COIN );
		PACKET_GUARD_VOID( kPacket.Write((int)USE_MONSTER_COIN_START_OK));
		PACKET_GUARD_VOID( kPacket.Write(m_dwUseStartCoinCnt));
		PACKET_GUARD_VOID( kPacket.Write(bUseGoldCoin));

		if( pRecord->pUser->IsPCRoomAuthority() )
		{
			//PACKET_GUARD_VOID( kPacket.Write(USE_MONSTER_COIN_AT_PCROOM) );
			PACKET_GUARD_VOID( kPacket.Write(USE_MONSTER_COIN_START_OK) );
		}
		else
		{
			PACKET_GUARD_VOID( kPacket.Write(USE_MONSTER_COIN_START_OK) );
		}

		pRecord->pUser->SendMessage( kPacket );
	}
}


void CTowerDefMode::OnUseMonsterCoin( User *pUser, SP2Packet &rkPacket )
{
	// start������ Ŭ�󿡼� ���޹��� �ʰ� �������� ���� ó��.
	// �׷��� �̺κ��� ������� �ʴ´�.

	int iUseCommand = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iUseCommand) );

	UseMonsterCoin(pUser, iUseCommand);
}



void CTowerDefMode::OnTreasureCardCommand( User *pUser, SP2Packet &rkPacket )
{
	if( pUser == NULL ) return;

	int iCommand;
	rkPacket >> iCommand;

	switch( iCommand )
	{
	case TREASURE_CARD_CMD_TIME_OUT:
		{
			TreasureCardStop();
			CheckResultPieceInfo();
		}
		break;
	case TREASURE_CARD_CMD_CLICK:
		{
			DWORD dwID;
			rkPacket >> dwID;

			MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
			if( pRecord == NULL )
				return;

			if( pRecord->iTreasureCardCount <= 0 || !COMPARE( dwID, 1, MAX_TREASURE_CARD + 1 ) || IsTreasureAlreadyID( dwID ) )
			{
				// �̹� Ŭ���Ǿ��ų� ��ġ�� �ƴ� ���� Ƚ���� ����
				SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
				kPacket << TREASURE_CARD_CMD_CLICK << 0;
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


DWORD CTowerDefMode::CreateTreasureCardIndex( DWORD dwPlayTime )
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

bool CTowerDefMode::IsTreasureAlreadyID( DWORD dwID )
{
	for(int i = 0;i < (int)m_vTreasureCard.size();i++)
	{
		if( m_vTreasureCard[i] == dwID )
			return false;
	}
	return true;
}

void CTowerDefMode::EraseTreasureCardID( DWORD dwID )
{
	for(int i = 0;i < (int)m_vTreasureCard.size();i++)
	{
		if( m_vTreasureCard[i] == dwID )
		{
			m_vTreasureCard.erase( m_vTreasureCard.begin() + i );
			break;
		}
	}
}

void CTowerDefMode::OpenTreasureCard( MonsterSurvivalRecord *pRecord, DWORD dwID )
{
	if( pRecord == NULL || pRecord->pUser == NULL ) return;

	CreateTreasureCardIndex(pRecord->GetAllPlayingTime());

	for(int i = 0;i < (int)m_vTreasureCard.size();i++)
	{
		if( m_vTreasureCard[i] == dwID )
		{
			m_vTreasureCard.erase( m_vTreasureCard.begin() + i );

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

				//
				SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
				kPacket << TREASURE_CARD_CMD_CLICK << dwID << pRecord->pUser->GetPublicID() << pRecord->iTreasureCardCount << iPresentType << iPresentValue1 << iPresentValue2;
				SendRoomAllUser( kPacket );

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
				kPacket << TREASURE_CARD_CMD_CLICK << dwID << pRecord->pUser->GetPublicID() << pRecord->iTreasureCardCount << iPresentType << iPresentValue1 << iPresentValue2;
				SendRoomAllUser( kPacket );
			}
			break;
		}
	}
}

int CTowerDefMode::GetTotalRemainTreasureCard()
{
	int iReturn = 0;

	int iRecordCnt = m_vRecordList.size();
	for(int i = 0;i < iRecordCnt;i++)
	{
		iReturn += m_vRecordList[i].iTreasureCardCount;
	}

	if( iReturn > (TCARD_MAX * 4) )
		iReturn = (TCARD_MAX * 4); 

	return iReturn;
}

void CTowerDefMode::TreasureCardStop()
{
	if( m_bTreasureStop )
		return;

	m_bTreasureStop = true;

	SP2Packet kPacket( STPK_TREASURE_CARD_COMMAND );
	kPacket << TREASURE_CARD_CMD_TIME_OUT;
	kPacket << TIMEGETTIME();      // ���� �õ尪

	int iRemainCount = GetTotalRemainTreasureCard();
	kPacket << iRemainCount;

	if( !m_vTreasureCard.empty() )
		std::random_shuffle( m_vTreasureCard.begin(), m_vTreasureCard.end() );

	int iRecordCnt = m_vRecordList.size();
	for(int i = 0;i < iRecordCnt;i++)
	{
		if( m_vRecordList[i].iTreasureCardCount <= 0 ) continue;
		if( m_vRecordList[i].pUser == NULL ) continue;

		CreateTreasureCardIndex(m_vRecordList[i].GetAllPlayingTime());

		for(int k = 0;k < m_vRecordList[i].iTreasureCardCount;k++)
		{
			ioHashString szSendID; 
			short iPresentType, iPresentState, iPresentMent;
			int iPresentPeriod, iPresentValue1, iPresentValue2;
			g_MonsterMapLoadMgr.GetTreasureCardPresent( m_dwTreasureCardIndex, szSendID, iPresentType, iPresentState,
				iPresentMent, iPresentPeriod, iPresentValue1, iPresentValue2 );
			if( iPresentType == 0 )
			{
				// ��ǰ ����. â���� ��!!!!
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "TowerDef::TreasureCardStop None Card Present : %d", m_dwTreasureCardIndex );
			}
			else if( iPresentType == PRESENT_ALCHEMIC_ITEM )
			{
				int iResult = m_vRecordList[i].pUser->SetPresentAlchemicItem( iPresentValue1, iPresentValue2 );
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
					m_vRecordList[i].pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
						0, 0, iPresentMent, kPresentTime, iPresentState );
					m_vRecordList[i].pUser->SendPresentMemory();
				}

				//
				DWORD dwID = *m_vTreasureCard.begin();
				kPacket << dwID << m_vRecordList[i].pUser->GetPublicID() << iPresentType << iPresentValue1 << iPresentValue2;

				int iUserIndex = m_vRecordList[i].pUser->GetUserIndex();
				ResultPieceInfoMap::iterator iter = m_UserResultPieceInfoMap.find( iUserIndex );
				if( iter != m_UserResultPieceInfoMap.end() )
				{
					iter->second.m_iPieceCnt += iPresentValue2;
				}
				else
				{
					ResultPieceInfo kPieceInfo;
					kPieceInfo.m_ID = m_vRecordList[i].pUser->GetPublicID();
					kPieceInfo.m_iLevel = m_vRecordList[i].pUser->GetGradeLevel();
					kPieceInfo.m_iPieceCnt = iPresentValue2;
					kPieceInfo.m_iPlayTime = GetRecordPlayTime( &m_vRecordList[i] );

					m_UserResultPieceInfoMap.insert( ResultPieceInfoMap::value_type(iUserIndex, kPieceInfo) );
				}

				if( m_vTreasureCard.size() > 1 )
					m_vTreasureCard.erase( m_vTreasureCard.begin() );
			}
			else
			{
				DWORD dwID = *m_vTreasureCard.begin();
				CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
				CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
				m_vRecordList[i].pUser->AddPresentMemory( szSendID, iPresentType, iPresentValue1, iPresentValue2,
					0, 0, iPresentMent, kPresentTime, iPresentState );

				kPacket << dwID << m_vRecordList[i].pUser->GetPublicID() << iPresentType << iPresentValue1 << iPresentValue2;

				if( m_vTreasureCard.size() > 1 )
					m_vTreasureCard.erase( m_vTreasureCard.begin() );
			}
		}
		m_vRecordList[i].iTreasureCardCount = 0;
		m_vRecordList[i].pUser->SendPresentMemory();
	}
	SendRoomAllUser( kPacket );
}


void CTowerDefMode::UpdateUserRank()
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


void CTowerDefMode::FinalRoundPoint( ModeRecord *pRecord, bool bAbuseUser, float fTotalVictoriesRate, float fTotalConsecutivelyRate )
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
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "[%d] %s [%d]: %s EXP: ( %.2f ) x %.2f x %.4f x %.2f x %.2f x ( %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f ) x %.2f = %.2f",
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

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "[%d] %s [%d]: %s PESO : (( %.2f ) x %.2f x %.4f x %.2f x %.2f x ( %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f + %.2f ) x %.2f x %.2f ) = %.2f",
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
		float fAddEventExp = pUser->GetExpBonusEvent();
		float fSoldierExpBonus = ( fClassPoint * ( fClassBonus + fAddEventExp ) );		
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


void CTowerDefMode::OnNpcSpawn( SP2Packet &rkPacket )
{
	int nNpc = 0;
	static int nNpcName = 0;

	rkPacket >> nNpc;

	if( nNpc > 0)
	{
		SP2Packet kPacket( STPK_SPAWN_MONSTER );
		kPacket << nNpc;

		for(int i = 0; i < nNpc; i++)
		{
			int nNpcCode;
			rkPacket >> nNpcCode;

			char szNpcName[MAX_PATH] = "";
			sprintf_s( szNpcName, " -N%d- ", ++m_nNpcCount );  // Spawn Npc Name.

			ioHashString szSyncUser = SearchMonsterSyncUser();
			int eTeam = (int)TEAM_RED;
			int dwStartTime;
			float fStartXPos, fStartZPos;

			rkPacket >> eTeam >> dwStartTime >> fStartXPos >> fStartZPos;


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

			if( rkMonster.dwNPCIndex == 0 ) 
				rkMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();

			HighTurnData &rkHighTurn = m_HighTurnList[0];
			MonsterRecordList &vecRecord = rkHighTurn.m_TurnData[0].m_vMonsterList;
			
			vecRecord.push_back(rkMonster);

#ifdef ANTIHACK
			kPacket << rkMonster.dwNPCIndex << nNpcCode << szNpcName << szSyncUser << eTeam << 5 << false << 100 << dwStartTime << fStartXPos << fStartZPos;
#else
			kPacket << nNpcCode << szNpcName << szSyncUser << eTeam << 5 << false << 100 << dwStartTime << fStartXPos << fStartZPos;
#endif
		}

		SendRoomAllUser(kPacket);
	}
}


void CTowerDefMode::OnExitRoom( User *pSend, SP2Packet &rkPacket )
{

	bool bExitLobby = false;
	int iExitType = 0;
	int iPenaltyPeso = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iExitType) );
	PACKET_GUARD_VOID( rkPacket.Read(iPenaltyPeso) );
	PACKET_GUARD_VOID( rkPacket.Read(bExitLobby) );

	ExitRoom(pSend, bExitLobby, iExitType, iPenaltyPeso);
}

void CTowerDefMode::CheckResultPieceInfo()
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

