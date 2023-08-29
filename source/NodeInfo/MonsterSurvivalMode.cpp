

#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"

#include "MonsterSurvivalMode.h"

#include "Room.h"
#include "RoomNodeManager.h"
#include "ioEventUserNode.h"
#include "ioEtcItemManager.h"
#include "LadderTeamManager.h"
#include "ioPresentHelper.h"
#include "ioMonsterMapLoadMgr.h"
#include "ioItemInfoManager.h"
#include "ioExerciseCharIndexManager.h"
#include "MissionManager.h"

#include "../EtcHelpFunc.h"
#include "../DataBase/LogDBClient.h"
#include "../DataBase/DBClient.h"
#include <strsafe.h>

MonsterSurvivalMode::MonsterSurvivalMode( Room *pCreator ) : Mode( pCreator )
{
	m_dwCurrentHighTurnIdx = 0;
	m_dwAbusePlayTime = 0;
	m_dwPresentPlayRandValue = 0;
	m_dwSpecialAwardMaxPoint = 0;
	m_dwUseStartCoinCnt = 0;
	m_dwCurContinueTime = 0;
	m_dwContinueTime = 0;
	m_fModeTurnPoint = 0.0f;
	m_bStopTime = false;
	m_iUseGoldCoinRevival = 0;
	m_dwDiceRewardTime = 0;
	m_FieldRewardItemUniqueIndex = 1;
	m_dwStartCoinTime = 0;
}

MonsterSurvivalMode::~MonsterSurvivalMode()
{
}

void MonsterSurvivalMode::DestroyMode()
{
	Mode::DestroyMode();

	m_vRecordList.clear();
	m_HighTurnList.clear();
	m_vDeathNPCList.clear();
	m_RandomStartPos.clear();
}

void MonsterSurvivalMode::LoadINIValue()
{
	Mode::LoadINIValue();

	m_dwCurContinueTime = m_dwCurRoundDuration = 0;   // Turn Time Set
	m_vRoundHistory.clear();
}

void MonsterSurvivalMode::LoadRoundCtrlValue( ioINILoader &rkLoader )
{
	Mode::LoadRoundCtrlValue( rkLoader );

	rkLoader.SetTitle( "round" );
	m_dwContinueTime = rkLoader.LoadInt( "continue_time", 10000 );

	// ���α�� �κ� �������� ó��.
	rkLoader.SetTitle( "info" );
	m_dwStartCoinTime = rkLoader.LoadInt( "start_coin_use_time", 30000 );

}

void MonsterSurvivalMode::LoadMonsterRewardRate( ioINILoader &rkLoader )
{
	Mode::LoadMonsterRewardRate( rkLoader );

	m_DamageRankRewardRate.clear();

	rkLoader.SetTitle( "MonsterRewardRate" );
	m_dwDiceRewardTime = rkLoader.LoadInt( "DiceRewardTime", 100 );
	int iMaxDamageRank = rkLoader.LoadInt( "MaxDamageRank", 0 );
	for(int i = 0;i < iMaxDamageRank;i++)
	{
		char szKey[MAX_PATH] = "";
		sprintf_s( szKey, "DamageRank_%d", i + 1 );
		m_DamageRankRewardRate.push_back( rkLoader.LoadFloat( szKey, 0 ) );
	}
}

void MonsterSurvivalMode::InitObjectGroupList()
{
	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );

	int iSubNum = GetModeSubNum();
	int iGroupNum = GetModeMapNum();
	char szBuf[MAX_PATH], szTitle[MAX_PATH];

	m_vPushStructList.clear();
	m_iPushStructIdx = 0;

	wsprintf( szTitle, "monster_survival%d_object_group%d", iSubNum, iGroupNum );
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

void MonsterSurvivalMode::LoadMapINIValue()
{
	char szFileName[MAX_PATH] = "";
	sprintf_s( szFileName, "config/sp2_monstersurvival_mode%d_map.ini", GetModeSubNum() );

	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( szFileName );
	rkLoader.SetTitle( "info" );
	m_dwAbusePlayTime = rkLoader.LoadInt( "abuse_play_time", 0 );
	m_dwPresentPlayRandValue = rkLoader.LoadInt( "present_play_rand_value", 0 );
	m_dwSpecialAwardMaxPoint = rkLoader.LoadInt( "special_award_max_point", 0 );
	// �߰� 16.2.25 kaedoc �������� ��ŷ ������.
	m_dwUseStartCoinCnt = rkLoader.LoadInt( "use_monster_coin_cnt", 0 );

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
		std::random_shuffle( vTurnList.begin(), vTurnList.end() );      //LowTurn���� ���´�.

		iMaxLowTurn = min( (int)vTurnList.size(), rkLoader.LoadInt( "max_play_turn", iMaxLowTurn ) );
		for(iLowTurn = 0;iLowTurn < iMaxLowTurn;iLowTurn++)
		{
			TurnData kTurn;
			kTurn.m_bBossTurn = Help::IsStringCheck( szBossTurn, iLowTurn + 1, '.' );
			if( g_MonsterMapLoadMgr.GetTurnData( vTurnList[iLowTurn], kTurn, iHighTurn, iLowTurn, iNpcNameCount ) )
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
}

void MonsterSurvivalMode::AddNewRecord( User *pUser )
{
	MonsterSurvivalRecord kRecord;
	kRecord.pUser = pUser;
	m_vRecordList.push_back( kRecord );

	UpdateUserRank();

	// �߰� ���� ���� NPC ����ȭ
	PlayMonsterSync( &kRecord );
}

void MonsterSurvivalMode::RemoveRecord( User *pUser, bool bRoomDestroy )
{
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
		RemoveRecordChangeMonsterSync( pUser->GetPublicID() );
	}

	if( GetCurTeamUserCnt( TEAM_BLUE ) > 0 )
	{
		NextTurnCheck();
	}
}

float MonsterSurvivalMode::GetMonsterStartXPos( float fXPos, int &rRandIndex  )
{
	if( fXPos != 0.0f ) return fXPos;
	if( m_RandomStartPos.empty() ) return fXPos;

	rRandIndex = rand()%(int)m_RandomStartPos.size();
	RandomStartPos &rkRandPos = m_RandomStartPos[rRandIndex];
	if( rkRandPos.m_fStartXRange <= 0.0f ) return fXPos;

	return rkRandPos.m_fStartXPos + ( rand() % (int)rkRandPos.m_fStartXRange );
}

float MonsterSurvivalMode::GetMonsterStartZPos( float fZPos, int iRandIndex )
{
	if( fZPos != 0.0f ) return fZPos;
	if( m_RandomStartPos.empty() ) return fZPos;
	if( !COMPARE( iRandIndex, 0, (int)m_RandomStartPos.size() ) ) return fZPos;

	RandomStartPos &rkRandPos = m_RandomStartPos[iRandIndex];
	if( rkRandPos.m_fStartZRange <= 0.0f ) return fZPos;

	return rkRandPos.m_fStartZPos - ( rand() % (int)rkRandPos.m_fStartZRange );
}

const ioHashString &MonsterSurvivalMode::SearchMonsterSyncUser()
{
	static ioHashString szError = "����ȭ��������";
	if( m_vRecordList.empty() ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::SearchMonsterSyncUser(%d) : None User Record", m_pCreator->GetRoomIndex() );
		return szError;
	}

	MonsterSurvivalRecord *pReturnRecord = NULL;
	MonsterSurvivalRecordList::iterator iter = m_vRecordList.begin();
	for(;iter != m_vRecordList.end();iter++)
	{
		MonsterSurvivalRecord &rkRecord = *iter;
		if( rkRecord.pUser == NULL ) continue;
		if( !pReturnRecord )
		{
			pReturnRecord = &rkRecord;
			continue;
		}		
		if( rkRecord.eState != RS_PLAY ) continue;
		if( rkRecord.pUser->GetTeam() != TEAM_BLUE ) continue;        // ���� ������ ����ȭ ��ü���� ���ܽ�Ų��.
		
		int iPrevPoint = pReturnRecord->pUser->GetPingStep() + pReturnRecord->iMonsterSyncCount;
		int iNextPoint = rkRecord.pUser->GetPingStep() + rkRecord.iMonsterSyncCount;
		if( iPrevPoint > iNextPoint )
			pReturnRecord = &rkRecord;
	}

	if( pReturnRecord == NULL || pReturnRecord->pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::SearchMonsterSyncUser(%d) : None Return Record : %d", m_pCreator->GetRoomIndex(), m_vRecordList.size() );
		return szError;
	}
	
	// ���� �Ѹ��� ����ȭ �߰�~
	pReturnRecord->iMonsterSyncCount++;
	return pReturnRecord->pUser->GetPublicID();
}

void MonsterSurvivalMode::RemoveRecordChangeMonsterSync( const ioHashString &rkRemoveName )
{
	if( m_bRoundSetEnd ) return;
	if( m_vRecordList.empty() ) return;
	if( rkRemoveName.IsEmpty() ) return;

	MonsterRecordList vSyncRecord;
	// ���� NPC�� ����ȭ �Ű澲�� �ʰ� ����ִ� NPC�� üũ�Ͽ� �����鿡�� ����
	int i,j,k;
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
				if( rkMonster.szSyncUser != rkRemoveName ) continue;

				rkMonster.szSyncUser = SearchMonsterSyncUser();
				vSyncRecord.push_back( rkMonster );
			}
		}
	}

	// Death NPC Check
	for(i = 0;i < (int)m_vDeathNPCList.size();i++)
	{
		MonsterRecord &rkMonster = m_vDeathNPCList[i];
		if( rkMonster.eState != RS_PLAY ) continue;
		if( rkMonster.szSyncUser != rkRemoveName ) continue;

		rkMonster.szSyncUser = SearchMonsterSyncUser();
		vSyncRecord.push_back( rkMonster );
	}

	if( vSyncRecord.empty() ) return;

	int iSyncSize = vSyncRecord.size();
	SP2Packet kPacket( STPK_MONSTER_SYNC_CHANGE );
	kPacket << iSyncSize;
	for(i = 0;i < iSyncSize;i++)
	{
		MonsterRecord &rkMonster = vSyncRecord[i];
		kPacket << rkMonster.szName << rkMonster.szSyncUser;
	}
	SendRoomAllUser( kPacket );
	vSyncRecord.clear();
}

void MonsterSurvivalMode::PlayMonsterSync( MonsterSurvivalRecord *pSendRecord )
{
	if( m_bRoundSetEnd ) return;
	if( GetState() != MS_PLAY ) return;        // �÷����� ������ �����鿡�Ը� ����ȭ ��Ų��.
	if( pSendRecord == NULL || pSendRecord->pUser == NULL ) return;

	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::PlayMonsterSync(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::PlayMonsterSync(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	SP2Packet kPacket( STPK_MONSTER_INFO_SYNC );

	// �÷��� ���� 
	kPacket << rkTurn.m_bBossTurn << GetCurrentAllLowTurn() << GetMaxAllLowTurn();
	kPacket << rkTurn.m_dwHelpIndex << m_dwCurrentHighTurnIdx << rkHighTurn.m_dwCurrentTurnIdx;
	kPacket << rkTurn.m_dwReduceNpcCreateTime;

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
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.iHighTurnIndex << rkMonster.iLowTurnIndex;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.iHighTurnIndex << rkMonster.iLowTurnIndex;
#endif
		
	}

	// Death NPC Check
	MonsterRecordList vDeathNpcSyncRecord;           // ����ȭ ����
	for(i = 0;i < (int)m_vDeathNPCList.size();i++)
	{
		MonsterRecord &rkMonster = m_vDeathNPCList[i];
		if( rkMonster.eState != RS_PLAY ) continue;

		vDeathNpcSyncRecord.push_back( rkMonster );
	}
	iSyncSize = vDeathNpcSyncRecord.size();	
	kPacket << iSyncSize;
	for(i = 0;i < iSyncSize;i++)
	{
		MonsterRecord &rkMonster = vDeathNpcSyncRecord[i];
		if( rkMonster.dwNPCIndex == 0 )
			rkMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.iHighTurnIndex;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.iHighTurnIndex;
#endif
		
	}
	pSendRecord->pUser->SendMessage( kPacket ); 
	vSyncRecord.clear();
	vDeathNpcSyncRecord.clear();
}

void MonsterSurvivalMode::StartTurn( DWORD dwHighTurnIdx, DWORD dwLowTurnIndex )
{
	m_dwCurrentHighTurnIdx = dwHighTurnIdx;
	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::StartTurn(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	rkHighTurn.m_dwCurrentTurnIdx = dwLowTurnIndex;
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::StartTurn(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
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
	for(i = 0;i < iMaxNPC;i++)
	{
		MonsterRecord &rkMonster = rkTurn.m_vMonsterList[i];
		rkMonster.eState         = RS_PLAY;
		rkMonster.szSyncUser     = SearchMonsterSyncUser();

		int iRandIndex = 0;
		rkMonster.fStartXPos     = GetMonsterStartXPos( rkMonster.fStartXPos, iRandIndex );
		rkMonster.fStartZPos     = GetMonsterStartZPos( rkMonster.fStartZPos, iRandIndex );
		
		if( rkMonster.dwNPCIndex == 0 )
			rkMonster.dwNPCIndex     = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser 
			<< rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser 
			<< rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
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

void MonsterSurvivalMode::StartTurnRevivalChar( SP2Packet &rkPacket )
{
	// ���� �� ���� ���� ��Ȱ
	int i = 0;
	ioHashStringVec vRevivalUser;
	for(i = 0;i < GetRecordCnt();i++)
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord*>(FindModeRecord( i ));
		if( pRecord )
		{
			if( pRecord->pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
			if( pRecord->pUser->GetTeam() != TEAM_BLUE ) continue;

			pRecord->bClientViewState = false;
			if( pRecord->bDieState || pRecord->bPrisoner )
			{
				pRecord->bPrisoner = false;
				pRecord->bDieState = false;
				pRecord->dwCurDieTime = 0;
				pRecord->pUser->ReleaseAllItemSelectChar();
				vRevivalUser.push_back( pRecord->pUser->GetPublicID() );
			}
		}
	}

	int iRevivalUserSize = vRevivalUser.size();
	rkPacket << iRevivalUserSize;
	for(i = 0;i < iRevivalUserSize;i++)
	{
		rkPacket << vRevivalUser[i];
	}	
	vRevivalUser.clear();
}

void MonsterSurvivalMode::CheckTurnTime()
{
	if( m_bRoundSetEnd ) return;

	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::CheckTurnTime(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::CheckTurnTime(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	DWORD dwGapTime = TIMEGETTIME() - m_dwStateChangeTime;
	if( dwGapTime >= m_dwCurRoundDuration )
	{
		if( !rkTurn.m_bTimeLimitEnd && !rkTurn.m_bMonsterAllDie )
		{
			rkTurn.m_bTimeLimitEnd = true;
			EndTurn( rkTurn );
		}
	}	
}

void MonsterSurvivalMode::CheckTurnMonster()
{
	if( m_bRoundSetEnd ) return;

	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::CheckTurnMonster(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::CheckTurnMonster(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	int iLiveMonster = 0;
	int iMaxNPC = rkTurn.m_vMonsterList.size();
	for(int i = 0;i < iMaxNPC;i++)
	{
		MonsterRecord &rkMonster = rkTurn.m_vMonsterList[i];
		if( rkMonster.eState == RS_PLAY )
			iLiveMonster++;
	}

	if( iLiveMonster == 0 )
	{
		rkTurn.m_dwRevivalTime = 0;        // ��Ȱ �ʱ�ȭ.
		if( !rkTurn.m_bMonsterAllDie )
		{
			rkTurn.m_bMonsterAllDie = true;

			// �� ����Ʈ ����
			ClearTurnAddPoint( rkTurn );

			// �� ����
			EndTurn( rkTurn );

			// �� ���� ���� �˸�
			EndTurnToServerAlarm( rkTurn );
		}		
	}
}

void MonsterSurvivalMode::ClearTurnAddPoint( TurnData &rkTurn )
{
	if( rkTurn.m_fTurnPoint <= 0.0f ) return;

	DWORD dwTurnPlayTime = min( TIMEGETTIME() - m_dwStateChangeTime, rkTurn.m_dwTurnTime );
	float fPlayTimeGap = (float)dwTurnPlayTime / 60000.0f;        //60�� ����

	m_fModeTurnPoint += rkTurn.m_fTurnPoint * fPlayTimeGap;
	rkTurn.m_fTurnPoint = 0.0f;
}

void MonsterSurvivalMode::EndTurn( TurnData &rkTurn )
{
	SP2Packet kPacket( STPK_TURN_END );
	kPacket << rkTurn.m_bMonsterAllDie;

	int i = 0;
	// ��� NPC ���
	MonsterRecordList vDeathRecord;		
	for( i = 0;i < (int)m_vDeathNPCList.size();i++)
	{
		MonsterRecord &rkMonster = m_vDeathNPCList[i];
		if( rkMonster.eState == RS_PLAY ) continue;

		rkMonster.eState         = RS_PLAY;
		rkMonster.szSyncUser     = SearchMonsterSyncUser();
		rkMonster.iHighTurnIndex = m_dwCurrentHighTurnIdx;
		vDeathRecord.push_back( rkMonster );
	}

	int iDeathNPCCount = vDeathRecord.size();
	kPacket << iDeathNPCCount;
	for( i = 0;i < iDeathNPCCount;i++)
	{
		MonsterRecord &rkMonster = vDeathRecord[i];

		if( rkMonster.dwNPCIndex == 0 )
			rkMonster.dwNPCIndex = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser 
			<< rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
		kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser 
			<< rkMonster.dwStartTime << rkMonster.fStartXPos << rkMonster.fStartZPos;
#endif
		
	}

	// ��� NPC���� �׾����� ����Ǿ� ���� ���� ��Ȱ
	if( rkTurn.m_bMonsterAllDie )
	{
		ioHashStringVec vRevivalUserID;
		int iRecordCnt = GetRecordCnt();
		for( i=0 ; i<iRecordCnt ; i++ )
		{
			MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord *>( FindModeRecord( i ) );
			if( !pRecord->pUser ) continue;
			if( !pRecord->bDieState ) continue;

			// ���� ���·� ��� ��Ȱ
			pRecord->dwCurDieTime = 0;
			pRecord->bDieState = false;
			pRecord->pUser->ReleaseAllItemSelectChar();
			pRecord->bPrisoner = true;

			vRevivalUserID.push_back( pRecord->pUser->GetPublicID() );
		}
		int iRevivalUserSize = vRevivalUserID.size();
		kPacket << iRevivalUserSize;
		for(i = 0;i < iRevivalUserSize;i++)
			kPacket << vRevivalUserID[i];
		vRevivalUserID.clear();
	}
	SendRoomAllUser( kPacket );
}

void MonsterSurvivalMode::EndTurnToServerAlarm( TurnData &rkTurn )
{
	if( m_szLastKillerName.IsEmpty() ) return;
	if( !rkTurn.m_bClearAlarm || rkTurn.m_iAlarmFloor == 0 ) return;
	if( !rkTurn.m_bMonsterAllDie ) return;

	SP2Packet kPacket( SUPK_SERVER_ALARM_MENT );
	kPacket << UDP_SERVER_ALARM_MONSTER_SURVIVAL_CLEAR << m_szLastKillerName << rkTurn.m_iAlarmFloor;
	g_UserNodeManager.SendAllServerAlarmMent( kPacket );
}

void MonsterSurvivalMode::NextTurnCheck()
{
	if( m_bRoundSetEnd ) return;

	int iRecordCnt = m_vRecordList.size();
	if( iRecordCnt == 0 ) 
		return;

	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord *>( FindModeRecord( i ) );
		if( pRecord )
		{
			if( pRecord->pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
			if( pRecord->pUser->GetTeam() != TEAM_BLUE ) continue;

			if( !pRecord->bClientViewState )
				return;
		}
	}

    // ����LowTurn�� �����ϸ� ������ ����	
	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::NextTurnCheck(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
		return;
	}
	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::NextTurnCheck(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
		return;
	}

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	if( COMPARE( rkHighTurn.m_dwCurrentTurnIdx + 1, 0, (int)rkHighTurn.m_TurnData.size() ) )
	{
		StartTurn( m_dwCurrentHighTurnIdx, rkHighTurn.m_dwCurrentTurnIdx + 1 );
	}
	else if( COMPARE( m_dwCurrentHighTurnIdx + 1, 0, (int)m_HighTurnList.size() ) )        // ���� HighTurn�� �����ϸ� ������ ����
	{
		StartTurn( m_dwCurrentHighTurnIdx + 1, 0 );
	}
}

void MonsterSurvivalMode::CheckStartCoin()
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

void MonsterSurvivalMode::ProcessTime()
{
	Mode::ProcessTime();

	ProcessDiceReward();
	ProcessFieldRewardItem();
}

void MonsterSurvivalMode::ProcessReady()
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

	m_iReadyBlueUserCnt = GetTeamUserCnt( TEAM_BLUE );

	// �� ����
	StartTurn( 0, 0 );
}

void MonsterSurvivalMode::ProcessPlay()
{
	ProcessRevival();

	CheckRoundTimePing();
	CheckFieldItemLiveTime();
	CheckItemSupply( m_dwStateChangeTime );
	CheckBallSupply( m_dwStateChangeTime );
	CheckMachineSupply( m_dwStateChangeTime );
	CheckNeedSendPushStruct();
	CheckRoundEnd( true );
	CheckTurnTime();
	ProcessEvent();
	ProcessBonusAlarm();
	CheckStartCoin();
}

void MonsterSurvivalMode::RestartMode()
{
	// ������� ����.
}

void MonsterSurvivalMode::ProcessRevival()
{
	DWORD dwCurTime = TIMEGETTIME();

	// ���� ��Ȱ
	int iRecordCnt = GetRecordCnt();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord *>( FindModeRecord( i ) );
		if( !pRecord->pUser ) continue;
		if( pRecord->dwCurDieTime == 0 ) continue;

		if( pRecord->dwCurDieTime + pRecord->dwRevivalGap < dwCurTime )
		{
			// ���� ���·� ��� ��Ȱ
			pRecord->dwCurDieTime = 0;
			pRecord->bDieState = false;
			pRecord->pUser->ReleaseAllItemSelectChar();
			pRecord->bPrisoner = true;

			SP2Packet kPacket( STPK_PRISONER_REVIVAL );
			kPacket << pRecord->pUser->GetPublicID();
			SendRoomAllUser( kPacket );
		}
	}

	// ���� ��Ȱ
	ProcessMonsterRevival();
}

void MonsterSurvivalMode::ProcessMonsterRevival()
{
	if( m_bRoundSetEnd ) return;

	if( !COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) ) return;

	HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
	if( !COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) ) return;

	TurnData &rkTurn = rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx];
	if( rkTurn.m_dwRevivalTime == 0 ) return;       //������ ������ ��Ȱ �ð��� 0�̵ȴ�.

	DWORD dwCurTime = TIMEGETTIME();
	int iMonsterCnt = rkTurn.m_vMonsterList.size();
	for(int i = 0;i < iMonsterCnt;i++)
	{
		MonsterRecord &rkMonster = rkTurn.m_vMonsterList[i];
		if( rkMonster.dwCurDieTime == 0 ) continue;
		if( rkMonster.eState != RS_DIE ) continue;

		if( rkMonster.dwCurDieTime + rkTurn.m_dwRevivalTime < dwCurTime )
		{
			rkMonster.dwCurDieTime = 0;
			rkMonster.eState	= RS_PLAY;
			rkMonster.szSyncUser= SearchMonsterSyncUser();

			SP2Packet kPacket( STPK_MONSTER_REVIVAL );
#ifdef ANTIHACK
			kPacket << rkMonster.dwNPCIndex << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.fStartXPos << rkMonster.fStartZPos;
#else
			kPacket << rkMonster.dwCode << rkMonster.szName << rkMonster.szSyncUser << rkMonster.fStartXPos << rkMonster.fStartZPos;
#endif
			
			SendRoomAllUser( kPacket );
		}
	}
}

void MonsterSurvivalMode::ProcessDiceReward()
{
	if( m_DiceRewardList.empty() ) return;

	int i = 0;
	bool bTimeReset = false;
	for(i = 0;i < (int)m_DiceRewardList.size();i++)
	{
		MonsterDieDiceReward &rkDiceReward = m_DiceRewardList[i];
		if( TIMEGETTIME() - rkDiceReward.m_dwRewardTime < m_dwDiceRewardTime ) continue;

		ModeRecord *pPrizeRecord = NULL;        
		DWORD        dwPrizeValue = 0;
		for(int k = 0;k < (int)rkDiceReward.m_vRankUser.size();k++)
		{
			ModeRecord *pRecord = FindModeRecord( rkDiceReward.m_vRankUser[k] );
			if( !pRecord ) continue;
			if( !pRecord->pUser ) continue;
			
			DWORD dwValue = g_MonsterMapLoadMgr.GetMonsterDiceRate( k );
			if( dwValue == 0 ) continue;

			dwValue = rand() % dwValue + 1;
			if( dwValue > dwPrizeValue )
			{
				pPrizeRecord = pRecord;
				dwPrizeValue = dwValue;
			}
		}

		if( pPrizeRecord )
		{
			// ���� Insert
			CTimeSpan cPresentGapTime( rkDiceReward.m_iPresentPeriod, 0, 0, 0 );
			CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
			g_DBClient.OnInsertPresentData( pPrizeRecord->pUser->GetUserDBAgentID(), pPrizeRecord->pUser->GetAgentThreadID(), rkDiceReward.m_szSendID, 
											pPrizeRecord->pUser->GetPublicID(), rkDiceReward.m_iPresentType, rkDiceReward.m_iPresentValue1, rkDiceReward.m_iPresentValue2, 0, 0,
											rkDiceReward.m_iPresentMent, kPresentTime, rkDiceReward.m_iPresentState );
			g_LogDBClient.OnInsertPresent( 0, rkDiceReward.m_szSendID, g_App.GetPublicIP().c_str(), pPrizeRecord->pUser->GetUserIndex(), rkDiceReward.m_iPresentType,
										   rkDiceReward.m_iPresentValue1, rkDiceReward.m_iPresentValue2, 0, 0, LogDBClient::PST_RECIEVE, "DiceReward" );

			pPrizeRecord->pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

			// ��÷�� ����
			SP2Packet kPacket( STPK_MONSTER_DICE_RESULT );
			kPacket << pPrizeRecord->pUser->GetPublicID() << rkDiceReward.m_iPresentType << rkDiceReward.m_iPresentValue1 << rkDiceReward.m_iPresentValue2;
			SendRoomAllUser( kPacket );
		}
		else
		{
			// �� ����
			SP2Packet kPacket( STPK_MONSTER_DICE_RESULT );
			kPacket << "" << rkDiceReward.m_iPresentType << rkDiceReward.m_iPresentValue1 << rkDiceReward.m_iPresentValue2;
			SendRoomAllUser( kPacket );
		}
		
		// ���������� ����Ʈ���� ����
		m_DiceRewardList.erase( m_DiceRewardList.begin() + i );
		bTimeReset = true;
		break;
	}

	if( bTimeReset )
	{
		for(i = 0;i < (int)m_DiceRewardList.size();i++)
		{
			MonsterDieDiceReward &rkDiceReward = m_DiceRewardList[i];
			rkDiceReward.m_dwRewardTime = TIMEGETTIME();
		}
	}
}

void MonsterSurvivalMode::ProcessFieldRewardItem()
{
	if( m_FieldRewardItemList.empty() ) return;

	int i = 0;
	vFieldRewardItem kDeleteItem;
	vFieldRewardItem kOwnerClear;
	for(i = 0;i < (int)m_FieldRewardItemList.size();i++)
	{
		FieldRewardItem &rkRewardItem = m_FieldRewardItemList[i];
		
		DWORD dwCurrentTime = TIMEGETTIME() - rkRewardItem.m_dwFieldDropStartTime;
		if( dwCurrentTime > rkRewardItem.m_ItemData.m_dwReleaseTime )
		{
			// �ʵ� ���� �ð� �ʰ�
			kDeleteItem.push_back( rkRewardItem );
		}
		else if( dwCurrentTime > rkRewardItem.m_ItemData.m_dwAcquireRightTime )
		{
			// ���ʰ� �������� ���������Ƿ� ��� �������� ���� 
			rkRewardItem.m_ItemOwner = "";
			kOwnerClear.push_back( rkRewardItem );
		}
	}

	if( kDeleteItem.empty() && kOwnerClear.empty() )
		return;

	// ������ ����
	for(i = 0;i < (int)kDeleteItem.size();i++)
	{
		ReleaseFieldRewardItem( kDeleteItem[i].m_iUniqueIndex );
	}

	// ��� �������� ����
	SP2Packet kPacket( STPK_FIELD_REWARD_ITEM_CHANGE );

	// ���� ������
	kPacket << (int)kDeleteItem.size();
	for(i = 0;i < (int)kDeleteItem.size();i++)
		kPacket << kDeleteItem[i].m_iUniqueIndex;

	// ���� ����
	kPacket << (int)kOwnerClear.size();
	for(i = 0;i < (int)kOwnerClear.size();i++)
		kPacket << kOwnerClear[i].m_iUniqueIndex;

	SendRoomAllUser( kPacket );
}

void MonsterSurvivalMode::ReleaseFieldRewardItem( int iUniqueIndex )
{
	for(int i = 0;i < (int)m_FieldRewardItemList.size();i++)
	{
		FieldRewardItem &rkRewardItem = m_FieldRewardItemList[i];
		if( rkRewardItem.m_iUniqueIndex == iUniqueIndex )
		{
			m_FieldRewardItemList.erase( m_FieldRewardItemList.begin() + i );
			return;
		}
	}
}

MonsterSurvivalMode::FieldRewardItem *MonsterSurvivalMode::GetFieldRewardItem( int iUniqueIndex )
{
	for(int i = 0;i < (int)m_FieldRewardItemList.size();i++)
	{
		if( m_FieldRewardItemList[i].m_iUniqueIndex == iUniqueIndex )
		{
			return &m_FieldRewardItemList[i];
		}
	}
	return NULL;
}

int MonsterSurvivalMode::GetUserKickVoteLimit( const ioHashString &szKickUserName )
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

ModeRecord* MonsterSurvivalMode::FindModeRecord( const ioHashString &rkName )
{
	if( rkName.IsEmpty() )	return NULL;

	CRASH_GUARD();
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		if( m_vRecordList[i].pUser->GetPublicID() == rkName )
			return &m_vRecordList[i];
	}

	return NULL;
}

ModeRecord* MonsterSurvivalMode::FindModeRecord( User *pUser )
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

ModeRecord* MonsterSurvivalMode::FindModeRecord( int iIdx )
{
	if( COMPARE( iIdx, 0, GetRecordCnt() ) )
		return &m_vRecordList[iIdx];

	return NULL;
}

MonsterSurvivalRecord* MonsterSurvivalMode::FindMonsterSurvivalRecord( const ioHashString &rkName )
{
	return (MonsterSurvivalRecord*)FindModeRecord( rkName );
}

MonsterSurvivalRecord* MonsterSurvivalMode::FindMonsterSurvivalRecord( User *pUser )
{
	return (MonsterSurvivalRecord*)FindModeRecord( pUser );
}

MonsterRecord* MonsterSurvivalMode::FindMonsterInfo( const ioHashString &rkName )
{
	int i,j,k;
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
				MonsterRecord *pMonster = &rkData.m_vMonsterList[k];
				if( pMonster == NULL ) continue;
				if( pMonster->szName == rkName )
					return pMonster;
			}
		}
	}
	return NULL;
}

void MonsterSurvivalMode::UpdateDieState( User *pDier )
{
	MonsterSurvivalRecord *pDieRecord = FindMonsterSurvivalRecord( pDier );
	if( !pDieRecord ) return;
	if( pDieRecord->bDieState ) return;

	pDieRecord->bDieState = true;
	pDieRecord->dwCurDieTime = 0;
	pDieRecord->bExperienceState = false;
}

void MonsterSurvivalMode::UpdateUserDieTime( User *pDier )
{
	MonsterSurvivalRecord *pDieRecord = FindMonsterSurvivalRecord( pDier );
	if( !pDieRecord ) return;
	if( !pDieRecord->bDieState ) return;

	DWORD dwRevivalGap = (DWORD)GetRevivalGapTime( pDieRecord->iRevivalCnt );
	pDieRecord->dwRevivalGap = dwRevivalGap;
	pDieRecord->iRevivalCnt++;
	pDieRecord->dwCurDieTime = TIMEGETTIME();

	// �ش� ���� ��� ���Ͱ� ��� ������ ��Ȱ��Ű�� �ʴ´�..
	if( COMPARE( m_dwCurrentHighTurnIdx, 0, (int)m_HighTurnList.size() ) )
	{
		HighTurnData &rkHighTurn = m_HighTurnList[m_dwCurrentHighTurnIdx];
		if( COMPARE( rkHighTurn.m_dwCurrentTurnIdx, 0, (int)rkHighTurn.m_TurnData.size() ) )
		{
			if( !rkHighTurn.m_TurnData[rkHighTurn.m_dwCurrentTurnIdx].m_bMonsterAllDie )
				pDieRecord->dwCurDieTime = 0;
		}	
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::UpdateUserDieTime(%d) : Index Error 2: %d - %d ", m_pCreator->GetRoomIndex(), rkHighTurn.m_dwCurrentTurnIdx, (int)rkHighTurn.m_TurnData.size() );
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::UpdateUserDieTime(%d) : Index Error 1: %d - %d ", m_pCreator->GetRoomIndex(), m_dwCurrentHighTurnIdx, (int)m_HighTurnList.size() );
}

ModeType MonsterSurvivalMode::GetModeType() const
{
	return MT_MONSTER_SURVIVAL;
}

void MonsterSurvivalMode::GetModeInfo( SP2Packet &rkPacket )
{
	Mode::GetModeInfo( rkPacket );
	GetModeHistory( rkPacket );
}

void MonsterSurvivalMode::GetExtraModeInfo( SP2Packet &rkPacket )
{
}

void MonsterSurvivalMode::GetModeHistory( SP2Packet &rkPacket )
{
	int i = 0;
	int HistorySize = m_vRoundHistory.size();

	if( HistorySize == 0 || m_iCurRound-1 > HistorySize )
	{
		for( i = 0; i < m_iCurRound; i++ )	
		{
			RoundHistory rh;
			rkPacket << rh.iBluePoint << rh.iRedPoint;
		}
	}
	else
	{
		for( i = 0; i < m_iCurRound-1; i++ )	
		{
			RoundHistory rh = m_vRoundHistory[i];
			rkPacket << rh.iBluePoint << rh.iRedPoint;
		}

		if( HistorySize == m_iCurRound )
		{
			RoundHistory rh = m_vRoundHistory[m_iCurRound-1];
			rkPacket << rh.iBluePoint << rh.iRedPoint;
		}
		else
		{
			rkPacket << 0 << 0;
		}
	}
}

void MonsterSurvivalMode::GetCharModeInfo( SP2Packet &rkPacket, const ioHashString &rkName, bool bDieCheck )
{
	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( rkName );
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
		rkPacket << pRecord->bPrisoner;
		rkPacket << pRecord->bCatchState;
	}
	else
	{
		// ���ڵ� ���� ����
		rkPacket << false;
	}
}

int MonsterSurvivalMode::GetRecordCnt() const
{
	return m_vRecordList.size();
}

const char* MonsterSurvivalMode::GetModeINIFileName() const
{
	return "config/monstersurvivalmode.ini";
}

TeamType MonsterSurvivalMode::GetNextTeamType()
{
	return TEAM_BLUE;
}

float MonsterSurvivalMode::GetUserCorrection( TeamType eWinTeam, float fRoundPoint, float fScoreGap )
{
	return 1.0f;
}

void MonsterSurvivalMode::SetRoundContinue()
{
	m_dwCurContinueTime = TIMEGETTIME();
	// �����鿡�� ��Ƽ�� ����
	SP2Packet kPacket( STPK_ROUND_CONTINUE );
	kPacket << m_dwContinueTime;
	SendRoomAllUser( kPacket );
}

bool MonsterSurvivalMode::IsRoundContinue()
{
	if( TIMEGETTIME() - m_dwCurContinueTime > m_dwContinueTime )
	{
		return false;
	}
	return true;
}

void MonsterSurvivalMode::CheckRoundEnd( bool bProcessCall )
{
	int iPrisonerAndDieUser = GetCurPrisonerAndDieUserCnt( TEAM_BLUE );
	WinTeamType eWinTeam = WTT_DRAW;
	if( GetCurTeamUserCnt( TEAM_BLUE ) == iPrisonerAndDieUser )    
	{	
		if( m_dwCurContinueTime == 0 )
			SetRoundContinue();

		// ���� ���� �� ��Ƽ�� �ð��� ��� ������ ��� ����ȴ�.
		if( !IsRoundContinue() )        
		{
			eWinTeam = WTT_RED_TEAM;
		}
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
		eWinTeam = WTT_RED_TEAM;
	else if( GetLiveMonsterCount() == 0 )
		eWinTeam = WTT_BLUE_TEAM;

	SetRoundEndInfo( eWinTeam );
	SendRoundResult( eWinTeam );
}

void MonsterSurvivalMode::SetRoundEndInfo( WinTeamType eWinTeam )
{
	m_CurRoundWinTeam = eWinTeam;

	m_bRoundSetEnd = true;
	m_bCheckContribute = false;
	m_bCheckAwardChoose = false;
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
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::SetRoundEndInfo() m_iCurRound != m_vRoundHistory.size()" );
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

void MonsterSurvivalMode::UpdateRoundRecord()
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
				pRecord->pUser->DeleteExtraItemPassedDate(true);
				pRecord->pUser->DeleteMedalItemPassedDate(true);
				pRecord->pUser->DeleteExMedalSlotPassedDate();
				pRecord->pUser->DeleteCharAwakePassedDate( );
				pRecord->pUser->DeleteCostumePassedDate();
				pRecord->pUser->DeleteAccessoryPassedDate();
				// �ӽ� : �ð�����
				pRecord->pUser->CheckTimeGrowth();
				pRecord->pUser->DeleteExpiredBonusCash();
			}
		}
	}

	UpdateUserRank();

	if( IsRedWin( m_CurRoundWinTeam ) )
		m_iRedTeamWinCnt++;
	else if( IsBlueWin( m_CurRoundWinTeam ) )
		m_iBlueTeamWinCnt++;
}

void MonsterSurvivalMode::OnEventSceneEnd( User *pSend, SP2Packet &rkPacket )
{
	bool bRoundChange;
	int iRoomIndex;
	rkPacket >> bRoundChange >> iRoomIndex;
	if( iRoomIndex != m_pCreator->GetRoomIndex() )
		return;

	if( !bRoundChange )
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
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::OnEventSceneEnd - %s Not Exist Record",
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
	else if( m_bUseViewMode && m_ModeState == MS_PLAY && dwPastTime > m_dwViewCheckTime )
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
		pRecord->pUser->StartCharLimitDate( Mode::GetCharLimitCheckTime(), __FILE__, __LINE__ );
		pRecord->pUser->StartEtcItemTime( __FUNCTION__ );

		SP2Packet kPacket( STPK_ROUND_JOIN );
		kPacket << pRecord->pUser->GetPublicID();
		kPacket << iModeState;
		kPacket << dwPastTime;
		kPacket << GetSelectCharTime();
		kPacket << m_dwCurRoundDuration;
		SendRoomAllUser( kPacket );
	}
}

int MonsterSurvivalMode::GetCurTeamUserCnt( TeamType eTeam )
{
	int iUserCnt = 0;
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( pRecord )
		{
			User *pUser = pRecord->pUser;
			if( pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER )
				continue;	

			if( pUser->GetTeam() == eTeam )
				iUserCnt++;
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::GetCurTeamUserCnt() - User's ModeRecord is Not Exist(%d).", i );
		}
	}

	return iUserCnt;
}

int MonsterSurvivalMode::GetCurPrisonerAndDieUserCnt( TeamType eTeam )
{
	int iUserCnt = 0;
	int iRecordCnt = m_vRecordList.size();
	for( int i=0 ; i<iRecordCnt ; i++ )
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord *>(FindModeRecord( i ));
		if( pRecord )
		{
			User *pUser = pRecord->pUser;
			if( pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER ) continue;	
			if( pUser->GetTeam() != eTeam ) continue;
			
			if( pRecord->bDieState || pRecord->bPrisoner )
				iUserCnt++;
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::GetCurPrisonerAndDieUserCnt() - User's ModeRecord is Not Exist(%d).", i );
		}
	}

	return iUserCnt;
}

float MonsterSurvivalMode::GetDamageRankRewardRate( int iRank )
{
	if( !COMPARE( iRank, 0, (int)m_DamageRankRewardRate.size() ) ) return 0.0f;

	return m_DamageRankRewardRate[iRank];
}

bool MonsterSurvivalMode::CheckRoundJoin( User *pSend )
{
	if( m_ModeState != MS_RESULT && m_ModeState != MS_RESULT_WAIT )
		return false;

	ModeRecord *pRecord = FindModeRecord( pSend );
	if( !pRecord )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::CheckRoundJoin - %s Not Exist Record",
								 pSend->GetPublicID().c_str() );
		return false;
	}

	pRecord->pUser->EquipDBItemToAllChar();
	SetFirstRevivalTime( pRecord );

	int iModeState = MS_RESULT_WAIT;

	DWORD dwPastTime = TIMEGETTIME() - m_dwStateChangeTime;

	pRecord->eState = RS_PLAY;
	pRecord->StartPlaying();        //( ����X, ����Ÿ��X )
	pRecord->pUser->StartCharLimitDate( Mode::GetCharLimitCheckTime(), __FILE__, __LINE__ );
	pRecord->pUser->StartEtcItemTime( __FUNCTION__ );

	SP2Packet kPacket( STPK_ROUND_JOIN );
	kPacket << pRecord->pUser->GetPublicID();
	kPacket << iModeState;
	kPacket << dwPastTime;
	kPacket << GetSelectCharTime();
	kPacket << m_dwCurRoundDuration;
	SendRoomAllUser( kPacket );

	SP2Packet kModeInfoPk( STPK_MODE_INFO );
	GetModeInfo( kModeInfoPk );
	SendRoomPlayUser( kModeInfoPk );
	
	return true;
}

void MonsterSurvivalMode::CheckUserLeaveEnd()
{
	if( m_bRoundSetEnd ) return;

	if( GetTeamUserCnt( TEAM_BLUE ) == 0 )
	{
		CheckRoundEnd( false );
	}
}

void MonsterSurvivalMode::UpdateDropDieRecord( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{
	ModeRecord *pDieRecord = FindModeRecord( pDier );
	if( !pDieRecord )	return;

	// ���������� Ÿ���� ������ NPC�� �� �ִ�.
	ModeRecord *pAttRecord = FindModeRecord( szAttacker );
	if( pAttRecord && pDier->GetPublicID() != szAttacker )        //������ Ÿ���ڰ� ������.
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pAttRecord->pUser->GetTeam() ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != pAttRecord->pUser->GetTeam() )
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
	else    //������ Ÿ���ڰ� NPC�̰ų� �ڻ��̴�.
	{
		pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
	}

	// ���� ���� �������� ���� ����
	ModeRecord *pBastAttRecord = FindModeRecord( szBestAttacker );
	if( pBastAttRecord && pDier->GetPublicID() != szBestAttacker )
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pBastAttRecord->pUser->GetTeam() ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != pBastAttRecord->pUser->GetTeam() )
			pBastAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
	}	
}

void MonsterSurvivalMode::UpdateWeaponDieRecord( User *pDier, const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{
	ModeRecord *pDieRecord = FindModeRecord( pDier );
	if( !pDieRecord )	return;

	// ���������� Ÿ���� ������ NPC�� �� �ִ�.
	ModeRecord *pAttRecord = FindModeRecord( szAttacker );
	if( pAttRecord && pDier->GetPublicID() != szAttacker )        //������ Ÿ���ڰ� ������.
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pAttRecord->pUser->GetTeam() ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != pAttRecord->pUser->GetTeam() )
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
	else    //������ Ÿ���ڰ� NPC�̰ų� �ڻ��̴�.
	{
		pDieRecord->AddDeathCount( m_pCreator->GetRoomStyle(), GetModeType(), GetDeathPoint( pDieRecord->pUser->GetTeam() ) );
	}

	// ���� ���� �������� ���� ����
	ModeRecord *pBastAttRecord = FindModeRecord( szBestAttacker );
	if( pBastAttRecord && pDier->GetPublicID() != szBestAttacker )
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pBastAttRecord->pUser->GetTeam() ) * 0.5f );
		if( pDieRecord->pUser->GetTeam() != pBastAttRecord->pUser->GetTeam() )
			pBastAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
	}
}

void MonsterSurvivalMode::UpdateMonsterDieRecord( const ioHashString &szAttacker, const ioHashString &szBestAttacker )
{
	// ���������� Ÿ���� ���� PS : ������ NPC�� ��µ� ��ų�� ����.
	ModeRecord *pAttRecord = FindModeRecord( szAttacker );
	if( pAttRecord )
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pAttRecord->pUser->GetTeam() ) * 0.5f );
		pAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
	}

	// ���� ���� �������� ���� ����
	ModeRecord *pBastAttRecord = FindModeRecord( szBestAttacker );
	if( pBastAttRecord )
	{
		float fKillPoint = 0.5f + ( (float)GetKillPoint( pBastAttRecord->pUser->GetTeam() ) * 0.5f );
		pBastAttRecord->AddKillCount( m_pCreator->GetRoomStyle(), GetModeType(), fKillPoint );
	}
}

void MonsterSurvivalMode::FinalRoundPoint( ModeRecord *pRecord, bool bAbuseUser, float fTotalVictoriesRate, float fTotalConsecutivelyRate )
{
	fTotalVictoriesRate = 1.0f;
	fTotalConsecutivelyRate = 1.0f;

	User *pUser = pRecord->pUser;
	if( !pUser )
		return;

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

	//�÷��� ���� ���
	ModeCategory ePlayMode = GetPlayModeCategory();

	//�ϸ��� ������ ����Ʈ B
	float fModeTurnPoint = m_fModeTurnPoint;        
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
	if( pRecord->pUser->IsPCRoomAuthority() )
	{		
		pRecord->fBonusArray[BA_FRIEND] = min( GetPcRoomMaxFriendBonus(), GetPcRoomFriendBonus() * (float)GetSameFriendUserCnt( pRecord->pUser ) );
	}
	else
	{
		pRecord->fBonusArray[BA_FRIEND] = min( GetMaxFriendBonus(), GetFriendBonus() * (float)GetSameFriendUserCnt( pRecord->pUser ) );
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

	// ���� ��� ���ʽ� T
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
		fAcquireExp = fModeTurnPoint * fExpTotalMultiply;
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
	
	fAcquirePeso = fModeTurnPoint * fPesoTotalMultiply;
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
		float fAddEventExp = 0;
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

void MonsterSurvivalMode::SendRoundResult( WinTeamType eWinTeam )
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

	// 41�� ~ 60���� ȥ�ڼ� �÷��� ������ DBLog�� �α� ����
	TraceRoundResultAbuse();

#ifdef ANTIHACK
	SendRelayGroupTeamWinCnt();
#endif
}

void MonsterSurvivalMode::TraceRoundResultAbuse()
{
	// Ŭ���� �������� ������ �ʴ´�.
	if( GetLiveMonsterCount() > 0 )
		return;

	// 1 = Easy , 2 = Normal , 3 = Hard
	enum{ EASY = 1, NORMAL = 2, HARD = 3, };
	if( GetModeSubNum() == HARD )
	{
		User *pPlayUser = NULL;
		int iRecordCnt = GetRecordCnt();
		for( int i=0 ; i<iRecordCnt ; i++ )
		{
			ModeRecord *pRecord = FindModeRecord( i );
			if( !pRecord || pRecord->eState != RS_PLAY ) continue;
			if( pPlayUser ) return;

            pPlayUser = pRecord->pUser;
		}

		if( pPlayUser )
		{
			char szLog[2048] = "";
			sprintf_s( szLog, "�ذ񿵿��� ȥ�ڼ� Ŭ���� ����(%d:%d) : %s/%s(%s) : %d", m_pCreator->GetRoomIndex(), GetModeSubNum(),
							pPlayUser->GetPrivateID().c_str(), pPlayUser->GetPublicID().c_str(), pPlayUser->GetPublicIP(), m_iUseGoldCoinRevival );
			SP2Packet kPacket( LUPK_LOG );
			kPacket << "CheckError";
			kPacket << szLog;
			kPacket << 326;  // ������ȣ
			kPacket << true; // write db
			g_UDPNode.SendLog( kPacket );
		}
	}
}

int MonsterSurvivalMode::GetCurrentAllLowTurn()
{
	int iReturnValue = (int)m_dwCurrentHighTurnIdx;
	int iHighTurnCnt = m_HighTurnList.size();
	for(int i = 0;i < iHighTurnCnt;i++)
	{
		HighTurnData &rkHighTurn = m_HighTurnList[i];
		iReturnValue += (int)rkHighTurn.m_dwCurrentTurnIdx;
	}
	return iReturnValue;
}

int MonsterSurvivalMode::GetMaxAllLowTurn()
{
	int iReturnValue = 0;
	int iHighTurnCnt = m_HighTurnList.size();
	for(int i = 0;i < iHighTurnCnt;i++)
	{
		HighTurnData &rkHighTurn = m_HighTurnList[i];
		iReturnValue += (int)rkHighTurn.m_TurnData.size();
	}
	return iReturnValue;
}

int MonsterSurvivalMode::GetLiveMonsterCount()
{
	int iMonsterCount = 0;
	int i,j,k;
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
				if( rkMonster.eState != RS_DIE )
					iMonsterCount++;
			}
		}
	}
	return iMonsterCount;
}

void MonsterSurvivalMode::OnMonsterDieToPresent( DWORD dwPresentCode )
{
	if( dwPresentCode == 0 ) return;

	int iRecordCnt = m_vRecordList.size();
	for(int i = 0;i < iRecordCnt;i++)
	{
		MonsterSurvivalRecord *pRecord = static_cast<MonsterSurvivalRecord *>(FindModeRecord( i ));
		if( pRecord )
		{
			User *pUser = pRecord->pUser;
			if( pUser == NULL ) continue;
			if( pRecord->eState == RS_VIEW || pRecord->eState == RS_OBSERVER || pRecord->eState == RS_LOADING ) continue;	
			if( pUser->GetTeam() != TEAM_BLUE ) continue;

			{   // ���ݱ��� �÷����� �ð��� ����.
				pRecord->AddPlayingTime();
				pRecord->StartPlaying();
			}

			bool bAbuseTime = false;
			if( m_dwPresentPlayRandValue > 0 )
			{
				DWORD dwPlayTime = GetRecordPlayTime( pRecord );
				if( dwPlayTime < m_dwAbusePlayTime || rand()%m_dwPresentPlayRandValue > dwPlayTime )
					bAbuseTime = true;
			}
			g_PresentHelper.SendMonsterPresent( pUser, dwPresentCode, bAbuseTime );
		}
	}
}

void MonsterSurvivalMode::OnMonsterDropItemPos( Vector3Vec &rkPosList, float fRange )
{
	// �������� �������� ����� �� 8�������� ���� �ѷ��ش�.
	rkPosList.push_back( Vector3( fRange, 0.0f, 0.0f ) );
	rkPosList.push_back( Vector3( -fRange, 0.0f, 0.0f ) );
	rkPosList.push_back( Vector3( 0.0f, 0.0f, fRange ) );
	rkPosList.push_back( Vector3( 0.0f, 0.0f, -fRange ) );
	rkPosList.push_back( Vector3( -fRange, 0.0f, fRange ) );
	rkPosList.push_back( Vector3( fRange, 0.0f, fRange ) );
	rkPosList.push_back( Vector3( -fRange, 0.0f, -fRange ) );
	rkPosList.push_back( Vector3( fRange, 0.0f, -fRange ) );
}

void MonsterSurvivalMode::OnMonsterDieToItemDrop( float fDropX, float fDorpZ, MonsterRecord *pDieMonster )
{
	if( pDieMonster == NULL ) return;

	Vector3Vec vRandPos;
	OnMonsterDropItemPos( vRandPos, 150.0f );
	std::random_shuffle( vRandPos.begin(), vRandPos.end() );
	//

	int i = 0;
	ItemVector vItemList;
	int iDropItemSize = pDieMonster->vDropItemList.size();
	for(i = 0;i < iDropItemSize;i++)
	{
		ITEM_DATA kItem;
		kItem.Initialize();
		g_MonsterMapLoadMgr.GetMonsterDropItem( pDieMonster->vDropItemList[i], kItem );
		if( kItem.m_item_code == 0 ) continue;

		ioItem *pItem = m_pCreator->CreateItem( kItem, pDieMonster->szName );
		if( pItem )
		{
			Vector3 vPos( fDropX + vRandPos[0].x, 0.0f, fDorpZ + vRandPos[0].z );
			pItem->SetItemPos( vPos );
			vItemList.push_back( pItem );

			if( vRandPos.size() > 1 )     //�ּ��� 1���� �����.
				vRandPos.erase( vRandPos.begin() );
		}		
	}
	vRandPos.clear();
	if( vItemList.empty() ) return;

	int iDropSize = vItemList.size();
	SP2Packet kPacket( STPK_FIELD_ITEM_SUPPLY );
	kPacket << iDropSize;
	for(i = 0;i < iDropSize;i++)
	{
		ioItem *pItem = vItemList[i];
		if( pItem == NULL ) continue;

		pItem->SetDropTime( TIMEGETTIME() );
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

void MonsterSurvivalMode::OnMonsterDieToRewardItemDrop( float fDropX, float fDropZ, DamageTableList &rkDamageTable, MonsterRecord *pDieMonster )
{
	if( pDieMonster == NULL ) return;
	if( rkDamageTable.empty() ) return;	

	ioHashString kTopDamageName = rkDamageTable[0].szName;
	if( FindMonsterSurvivalRecord( kTopDamageName ) == NULL ) return;

	Vector3Vec vRandPos;
	OnMonsterDropItemPos( vRandPos, 100.0f );
	OnMonsterDropItemPos( vRandPos, 150.0f );
	OnMonsterDropItemPos( vRandPos, 200.0f );
	std::random_shuffle( vRandPos.begin(), vRandPos.end() );
	
	//
	int i = 0;
	vFieldRewardItem kRewardItemList;
	int iDropItemSize = pDieMonster->vDropRewardItemList.size();
	for(i = 0;i < iDropItemSize;i++)
	{
		MonsterDropRewardItem kRewardItem;
		g_MonsterMapLoadMgr.GetMonsterDropRewardItem( pDieMonster->vDropRewardItemList[i], kRewardItem );
		if( kRewardItem.m_dwAcquireRightTime == 0 ) continue;

        if( kRewardItem.m_bAllUserGive )
		{
			// ��� �������� ����
			int iRecordCnt = m_vRecordList.size();
			for(int k = 0;k < iRecordCnt;k++)
			{
				MonsterSurvivalRecord &rkRecord = m_vRecordList[k];
				if( rkRecord.pUser == NULL ) continue;
				if( rkRecord.eState!= RS_PLAY ) continue;

				FieldRewardItem kItem;
				kItem.m_ItemData = kRewardItem;
				kItem.m_ItemOwner= rkRecord.pUser->GetPublicID();
				kItem.m_iUniqueIndex = GetFieldRewardItemUniqueIndex();
				Vector3 vPos( fDropX + vRandPos[0].x, 0.0f, fDropZ + vRandPos[0].z );
				kItem.m_ItemPos = vPos;
				kRewardItemList.push_back( kItem );

				if( vRandPos.size() > 1 )     //�ּ��� 1���� �����.
					vRandPos.erase( vRandPos.begin() );
			}
		}
		else
		{
			// ž �������Ը� ����
			FieldRewardItem kItem;
			kItem.m_ItemData = kRewardItem;
			kItem.m_ItemOwner= kTopDamageName;
			kItem.m_iUniqueIndex = GetFieldRewardItemUniqueIndex();
			Vector3 vPos( fDropX + vRandPos[0].x, 0.0f, fDropZ + vRandPos[0].z );
			kItem.m_ItemPos = vPos;
			kRewardItemList.push_back( kItem );

			if( vRandPos.size() > 1 )     //�ּ��� 1���� �����.
				vRandPos.erase( vRandPos.begin() );
		}
	}

	if( kRewardItemList.empty() ) return;       

	// �������� ���� �� ����Ʈ�� �߰�
	SP2Packet kPacket( STPK_FIELD_REWARD_ITEM_LIST );
	kPacket << (int)kRewardItemList.size();
	for(i = 0;i < (int)kRewardItemList.size();i++)
	{
		FieldRewardItem &rkRewardItem = kRewardItemList[i];
		kPacket << rkRewardItem.m_iUniqueIndex << rkRewardItem.m_ItemData.m_dwResourceValue;
		kPacket << rkRewardItem.m_ItemOwner << rkRewardItem.m_ItemPos;

		rkRewardItem.m_dwFieldDropStartTime = TIMEGETTIME();
		m_FieldRewardItemList.push_back( rkRewardItem );
	}
	SendRoomAllUser( kPacket );
	kRewardItemList.clear();
}

void MonsterSurvivalMode::_OnMonsterDieToExpPeso( DamageTableList &rkDamageTable, vMonsterDieReward &rkDieRewardList, int iExpReward, int iPesoReward )
{
	if( iExpReward == 0 && iPesoReward == 0 )
		return;

	int i = 0;		
	float fTotalConsecutivelyRate = GetTotalModeConsecutivelyRate();
	enum { MAX_GET_POINT_CHAR = 5, };
	for(i = 0;i < (int)rkDamageTable.size();i++)
	{
		DamageTable &rkTable = rkDamageTable[i];

		ModeRecord *pRecord = FindModeRecord( rkTable.szName );
		if( !pRecord )	continue;
		if( !pRecord->pUser ) continue;
		if( rkTable.iDamage == 0 ) continue;

		float fExpPoint = (float)iExpReward * GetDamageRankRewardRate( i );
		float fPesoPoint= (float)iPesoReward * GetDamageRankRewardRate( i );

		MonsterDieReward kRewardUser;
		kRewardUser.pUser = pRecord->pUser;

		//�÷��� ���� ���
		ModeCategory ePlayMode = GetPlayModeCategory();

		//�ο� ���� C
		float fUserCorrection = 1.0f;
		//�÷��� �ð� ������ D
		float fPlayTimeCorrection = 1.0f;
		//��Һ����� E
		float fPesoCorrection = m_fPesoCorrection;
		//����ġ ������ F
		float fExpCorrection  = m_fExpCorrection;
		//���� G
		float fBlockPoint = pRecord->pUser->GetBlockPointPer();
		//�⿩�� H
		float fContributePer = 1.0f;
		//��庸�ʽ� I
		float fGuildBonus = 0.0f;
		//�뺴 ���ʽ� J
		float fSoldierCntBonus = Help::GetSoldierPossessionBonus( pRecord->pUser->GetActiveCharCount() );
		//PC�� ���ʽ� K
		float fPCRoomBonusExp = 0.0f;
		float fPCRoomBonusPeso= 0.0f;
		if( pRecord->pUser->IsPCRoomAuthority() )
		{
			if( g_EventMgr.IsAlive( EVT_PCROOM_BONUS, pRecord->pUser->GetChannelingType() ) )
			{
				fPCRoomBonusExp = ( (float)g_EventMgr.GetValue( EVT_PCROOM_BONUS, EA_PCROOM_BONUS ) / 100.0f );  // 40 - > 0.40f
				fPCRoomBonusPeso= ( (float)g_EventMgr.GetValue( EVT_PCROOM_BONUS, EA_PCROOM_BONUS ) / 100.0f );  // 40 - > 0.40f
			}
			else
			{
				fPCRoomBonusExp  = Help::GetPCRoomBonusExp();
				fPCRoomBonusPeso = Help::GetPCRoomBonusPeso();
			}
		}
		//��� ���ʽ� L
		float tempModeBonus = 0.0f;

		if( pRecord->pUser )
		{
			// EVT_MODE_BONUS (1)
			//���� ��� �ϰ�� ��� üũ �ǳʶٱ� ����. ���� ��尡 �ƴҰ�� ��� üũ 
			ModeCategory eModeBonus = ePlayMode;

			if( eModeBonus != MC_SHUFFLE )
				eModeBonus = MC_DEFAULT;

			EventUserManager &rEventUserManager = pRecord->pUser->GetEventUserMgr();
			ModeBonusEventUserNode* pEvent1 = static_cast< ModeBonusEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_MODE_BONUS, eModeBonus ) );
			if( pEvent1 )
			{
				if( pEvent1->IsEventMode( GetModeType(), eModeBonus ) )
					tempModeBonus = pEvent1->GetEventPer( fPCRoomBonusExp, pRecord->pUser, eModeBonus );
			}

			// EVT_MODE_BONUS2 (2)
			ModeBonusEventUserNode* pEvent2 = static_cast< ModeBonusEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_MODE_BONUS2, eModeBonus ) );
			if( pEvent2 )
			{
				if( pEvent2->IsEventMode( GetModeType(), eModeBonus ) )
					tempModeBonus += pEvent2->GetEventPer( fPCRoomBonusExp, pRecord->pUser, eModeBonus );
			}
		
			if( tempModeBonus == 0.0f )
			{
				tempModeBonus = m_fPlayModeBonus;
			}
		}
		float fModeBonus = tempModeBonus;
		//ģ�� ���ʽ� M		
		float fFriendBonusPer = 0.0f;
		if( pRecord->pUser->IsPCRoomAuthority() )
		{		
			fFriendBonusPer = min( GetPcRoomMaxFriendBonus(), GetPcRoomFriendBonus() * (float)GetSameFriendUserCnt( pRecord->pUser ) );
		}
		else
		{
			fFriendBonusPer = min( GetMaxFriendBonus(), GetFriendBonus() * (float)GetSameFriendUserCnt( pRecord->pUser ) );
		}

		// �̺�Ʈ ����ġ ���ʽ� N
		float fEventBonus = 0.0f;
		EventUserManager &rEventUserManager = pRecord->pUser->GetEventUserMgr();
		ExpEventUserNode *pExpEventNode = static_cast<ExpEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_EXP, ePlayMode ) );
		if( pExpEventNode )
		{
			fEventBonus = pExpEventNode->GetEventPer( fPCRoomBonusExp, pRecord->pUser, ePlayMode );
		}

		// second event : evt_exp2
		ExpEventUserNode* pExp2 = static_cast< ExpEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_EXP2, ePlayMode ) );
		if( pExp2 )
		{
			fEventBonus += pExp2->GetEventPer( fPCRoomBonusExp, pRecord->pUser, ePlayMode );
		}

		// �̺�Ʈ ��� ���ʽ� O
		float fPesoEventBonus = 0.0f;
		PesoEventUserNode *pPesoEventNode = static_cast<PesoEventUserNode*> ( rEventUserManager.GetEventUserNode( EVT_PESO, ePlayMode ) );
		if( pPesoEventNode )
		{
			fPesoEventBonus = pPesoEventNode->GetPesoPer( fPCRoomBonusPeso, pRecord->pUser, ePlayMode );
		}

		// second event : evt_peso2
		PesoEventUserNode* pPeso2 = static_cast< PesoEventUserNode* >( rEventUserManager.GetEventUserNode( EVT_PES02, ePlayMode ) );
		if( pPeso2 )
		{
			fPesoEventBonus += pPeso2->GetPesoPer( fPCRoomBonusPeso, pRecord->pUser, ePlayMode );
		}

		// ���� ������ ���ʽ� P
		float fEtcItemBonus = 0.0f;
		float fEtcItemPesoBonus = 0.0f;
		float fEtcItemExpBonus  = 0.0f;
		ioUserEtcItem *pUserEtcItem = pRecord->pUser->GetUserEtcItem();
		if( pUserEtcItem )
		{
			ioEtcItem *pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_PESO_EXP_BONUS );
			ioUserEtcItem::ETCITEMSLOT kSlot;
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_PESO_EXP_BONUS, kSlot) && pItemItem )
			{
				fEtcItemBonus = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
			}

			pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_PESO_BONUS );
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_PESO_BONUS, kSlot ) && pItemItem )
			{
				fEtcItemPesoBonus = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
			}

			pItemItem = g_EtcItemMgr.FindEtcItem( ioEtcItem::EIT_ETC_EXP_BONUS );
			if( pUserEtcItem->GetEtcItem( ioEtcItem::EIT_ETC_EXP_BONUS, kSlot ) && pItemItem )
			{
				fEtcItemExpBonus = ( (float) pItemItem->GetUseValue() / 100.0f ); // 20 -> 0.20
			}
		}
		// ���� ��� ���ʽ� T
		float fModeConsecutivelyBonus = (1.0f + pRecord->pUser->GetModeConsecutivelyBonus()) * fTotalConsecutivelyRate;

		//ȹ�� ����ġ
		float fExpPlusValue     = max( 0.0f, fContributePer + fGuildBonus + fSoldierCntBonus + fPCRoomBonusExp + fModeBonus + fFriendBonusPer + fEventBonus + fEtcItemBonus + fEtcItemExpBonus );
		float fExpTotalMultiply = fUserCorrection * fPlayTimeCorrection * fBlockPoint * fExpPlusValue;
		fExpPoint = fExpPoint * fExpCorrection * fExpTotalMultiply * fModeConsecutivelyBonus;

		//ȹ�� ���
		float fAcquirePeso       = 0.0f;
		float fPesoPlusValue     = max( 0.0f, fContributePer + fGuildBonus + fPCRoomBonusPeso + fModeBonus + fFriendBonusPer + fPesoEventBonus + fEtcItemBonus + fEtcItemPesoBonus );
		float fPesoTotalMultiply = fUserCorrection * fPlayTimeCorrection * fBlockPoint * fPesoPlusValue;
		fPesoPoint = fPesoPoint * fExpCorrection * fPesoTotalMultiply * fModeConsecutivelyBonus;

		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMonsterDieToExpPeso %s : %d - %d == %.2f - %.2f", pRecord->pUser->GetPublicID().c_str(), iExpReward, iPesoReward, fExpPoint, fPesoPoint );

		// ��� 
		pRecord->pUser->AddMoney( kRewardUser.iRewardPeso );  
		g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_GAIN, pRecord->pUser, 0, 0, LogDBClient::LET_MODE, LogDBClient::PT_BATTLE, GetModeType(), 0, kRewardUser.iRewardPeso, NULL);

		kRewardUser.iRewardPeso = fPesoPoint;

		// ����ġ
		DWORDVec dwPlayTimeList;
		DWORD dwTotalTime = pRecord->GetCurrentHighPlayingTime( pRecord->pUser->GetSelectClassType(), MAX_GET_POINT_CHAR, kRewardUser.vRewardClassType, dwPlayTimeList );
		if( dwTotalTime > 0 ) 
		{
			int iListSize = kRewardUser.vRewardClassType.size();
			for(int k = 0; k < MAX_GET_POINT_CHAR; k++)
			{
				if( !COMPARE( k, 0, iListSize ) ) break;

				if( kRewardUser.vRewardClassType[k] == 0 ) continue;

				float fSoldierPer = (float)dwPlayTimeList[k] / dwTotalTime;
				int iCurPoint = ( fExpPoint * fSoldierPer ) + 0.5f;     //�ݿø�

				kRewardUser.vRewardClassPoint.push_back( iCurPoint );
				kRewardUser.iRewardExp += iCurPoint;
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ClassType[%d] PlayTime[%d - %d] - WinPoint[%d]", kRewardUser.vRewardClassType[k], dwPlayTimeList[k], dwTotalTime,
					kRewardUser.vRewardClassPoint[k] );

				// ����ġ ���� �� ������ Ȯ��
				if( pRecord->pUser->IsClassTypeExerciseStyle( kRewardUser.vRewardClassType[k], EXERCISE_RENTAL ) == false )
					pRecord->pUser->AddClassExp( kRewardUser.vRewardClassType[k], kRewardUser.vRewardClassPoint[k] );
				if( pRecord->pUser->AddGradeExp( kRewardUser.vRewardClassPoint[k] ) )
					kRewardUser.bGradeUP = true;

				if( pRecord->pUser->IsClassTypeExerciseStyle( kRewardUser.vRewardClassType[k], EXERCISE_RENTAL ) )
				{
					// ��� ����ġ�� ȹ���ϰ� �뺴 ����ġ�� ȹ�� �ȵ�
					kRewardUser.vRewardClassPoint[k] = 0;
				}
			}	
			// ������ ����
			kRewardUser.iRewardPeso += pRecord->pUser->GradeNClassUPBonus();
		}
		rkDieRewardList.push_back( kRewardUser );
	}
}

void MonsterSurvivalMode::OnMonsterDieToReward( const ioHashString &rkMonsterName, DamageTableList &rkDamageTable, DWORD dwDiceTable, int iExpReward, int iPesoReward )
{
	if( rkDamageTable.empty() ) return;

	int i = 0;

	// �ֻ��� ����
	MonsterDieDiceReward kDiceReward;
	g_MonsterMapLoadMgr.GetMonsterDicePresent( dwDiceTable, kDiceReward.m_szSendID, kDiceReward.m_iPresentType, kDiceReward.m_iPresentState,
											   kDiceReward.m_iPresentMent, kDiceReward.m_iPresentPeriod, kDiceReward.m_iPresentValue1, kDiceReward.m_iPresentValue2 );
	if( kDiceReward.m_iPresentType != 0 )
	{
		// ���� ȹ�� !!
		for(i = 0;i < (int)rkDamageTable.size();i++)
		{
			DamageTable &rkTable = rkDamageTable[i];
			if( rkTable.iDamage == 0 ) continue;

			kDiceReward.m_vRankUser.push_back( rkTable.szName );
		}
		kDiceReward.m_dwRewardTime = TIMEGETTIME();
		m_DiceRewardList.push_back( kDiceReward );
	}
	
	// ����ġ & ���
	vMonsterDieReward kRewardList;
	_OnMonsterDieToExpPeso( rkDamageTable, kRewardList, iExpReward, iPesoReward );

	if( kRewardList.empty() && kDiceReward.m_iPresentType == 0 )
		return;       // �����Ұ� �ƹ��͵� ����.
	
	int iRecordCnt  = GetRecordCnt();
	for(i = 0;i < iRecordCnt;i++)
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord )	continue;
		if( !pRecord->pUser ) continue;

		SP2Packet kPacket( STPK_MONSTER_DIE_REWARD );

		// �ֻ��� ���� ����
		if( kDiceReward.m_iPresentType == 0 )
			kPacket << false;
		else
		{
			kPacket << true << rkMonsterName;
			kPacket << kDiceReward.m_iPresentType << kDiceReward.m_iPresentValue1 << kDiceReward.m_iPresentValue2;

			int iUserSize = kDiceReward.m_vRankUser.size();
			kPacket << iUserSize;
			for(i = 0;i < iUserSize;i++)
			{
				kPacket << kDiceReward.m_vRankUser[i];
			}
		}

		// ��� & ����ġ ����
		int iRewardSize = kRewardList.size();
		kPacket << iRewardSize;
		for(int k = 0;k < iRewardSize;k++)
		{
			MonsterDieReward &rkRewardUser = kRewardList[k];
			if( rkRewardUser.pUser )
			{
				kPacket << rkRewardUser.pUser->GetPublicID() << rkRewardUser.iRewardExp << rkRewardUser.iRewardPeso;
				// �ڽ��� ������ �������ϰ� ����
				if( rkRewardUser.pUser == pRecord->pUser )
				{
					// 
					kPacket << pRecord->pUser->GetMoney() << pRecord->pUser->GetGradeLevel() << pRecord->pUser->GetGradeExpert();

					// �뺴��
					int iMaxClassExp = min( (int)rkRewardUser.vRewardClassType.size(), (int)rkRewardUser.vRewardClassPoint.size() );
					kPacket << iMaxClassExp;
					for(int c = 0;c < iMaxClassExp;c++)
					{
						kPacket << rkRewardUser.vRewardClassType[c] << rkRewardUser.vRewardClassPoint[c];
					}
				}
			}
			else
			{
				kPacket << "" << 0 << 0;
			}
		}

		pRecord->pUser->SendMessage( kPacket );
	}	
}

void MonsterSurvivalMode::OnMonsterDieToTypeProcess( DWORD dwDieType )
{
	switch( dwDieType )
	{
	case MONSTER_DIE_TYPE_NONE:
		{
			// �Ϲ����� �����̴� ���� ó�� �ʿ����.
		}
		break;
	case MONSTER_DIE_TYPE_ALL_DIE:
		{
			// ����ִ� ���� ���� ���� ó��.
			ioHashStringVec vDieMonsterName;
			int i,j,k;
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
						if( rkMonster.eState == RS_PLAY )
						{
							rkMonster.eState = RS_DIE;
							rkMonster.dwCurDieTime = 0;
							MonsterSurvivalRecord *pSyncUserRecord = FindMonsterSurvivalRecord( rkMonster.szSyncUser );
							if( pSyncUserRecord )
								pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );
							vDieMonsterName.push_back( rkMonster.szName );
						}
					}
				}
			}
			
			if( vDieMonsterName.empty() )
				return;

			// ���� ���� ����
			int iDieSize = vDieMonsterName.size();
			SP2Packet kPacket( STPK_MONSTER_FORCE_DIE );
			kPacket << iDieSize;
			for(i = 0;i < iDieSize;i++)
			{
				kPacket << vDieMonsterName[i];
			}
			SendRoomAllUser( kPacket );
		}
		break;
	}
}

void MonsterSurvivalMode::OnDropDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster ) return;
	if( pDieMonster->eState != RS_PLAY ) return;

	float fDiePosX, fDiePosZ;
	rkPacket >> fDiePosX >> fDiePosZ;

	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	rkPacket >> szLastAttackerName >> szLastAttackerSkillName >> dwLastAttackerWeaponItemCode >> iLastAttackerTeam;
		
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

	// ���� ����
	OnMonsterDieToPresent( pDieMonster->dwPresentCode );

	// ����ġ & ��� & �ֻ��� ���� ����
	OnMonsterDieToReward( pDieMonster->szName, vDamageList, pDieMonster->dwDiceTable, pDieMonster->iExpReward, pDieMonster->iPesoReward );

	// ���� ���� Ÿ�� ó��
	OnMonsterDieToTypeProcess( pDieMonster->dwDieType );

	m_szLastKillerName = szLastAttackerName;
	if( GetState() == Mode::MS_PLAY )
	{
		CheckTurnMonster();
	}
}

void MonsterSurvivalMode::OnWeaponDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	MonsterRecord *pDieMonster = FindMonsterInfo( rkDieName );
	if( !pDieMonster )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::OnWeaponDieNpc(%d) - None Npc : %s", m_pCreator->GetRoomIndex(), rkDieName.c_str() );
		return;
	}

	if( pDieMonster->eState != RS_PLAY )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::OnWeaponDieNpc(%d) - None Live Npc : %s", m_pCreator->GetRoomIndex(), rkDieName.c_str() );
		return;
	}

	// ���Ͱ� ���� ��ġ.
	float fDiePosX, fDiePosZ;
	rkPacket >> fDiePosX >> fDiePosZ;

	// Killer ���� ����.
	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	rkPacket >> szLastAttackerName >> szLastAttackerSkillName >> dwLastAttackerWeaponItemCode >> iLastAttackerTeam;

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
	if( GetState() == Mode::MS_PLAY )
	{
		CheckTurnMonster();
	}
}

bool MonsterSurvivalMode::ProcessTCPPacket( User *pSend, SP2Packet &rkPacket )
{
	if( Mode::ProcessTCPPacket( pSend, rkPacket ) )
		return true;

	switch( rkPacket.GetPacketID() )
	{
	case CTPK_PRISONER_ESCAPE:
		OnPrisonerEscape( pSend, rkPacket );
		return true;
	case CTPK_PRISONER_DROP:
		OnPrisonerDrop( pSend, rkPacket );
		return true;
	case CTPK_PRISONERMODE:
		OnPrisonerMode( pSend, rkPacket );
		return true;
	case CTPK_USE_MONSTER_COIN:
		OnUseMonsterCoin( pSend, rkPacket );
		return true;
	case CTPK_TURN_END_VIEW_STATE:
		OnTurnEndViewState( pSend, rkPacket );
		return true;
	case CTPK_PICK_REWARD_ITEM:
		OnPickRewardItem( pSend, rkPacket );
		return true;
	}

	return false;
}

void MonsterSurvivalMode::OnPrisonerEscape( User *pUser, SP2Packet &rkPacket )
{
	ioHashString szName, szLastAttacker, szLastAttackerSkill;	
	rkPacket >> szName >> szLastAttacker >> szLastAttackerSkill;

	MonsterSurvivalRecord *pEscape = FindMonsterSurvivalRecord( szName );
	if( !pEscape || !pEscape->bPrisoner ) return;

	pEscape->bPrisoner = false;
	pEscape->bDieState = false;
	pEscape->dwCurDieTime = 0;

	SP2Packet kReturn( STPK_PRISONER_ESCAPE );
	kReturn << szName << szLastAttacker << szLastAttackerSkill;
	SendRoomAllUser( kReturn );
}

void MonsterSurvivalMode::OnPrisonerDrop( User *pUser, SP2Packet &rkPacket )
{
	ioHashString szAttacker;
	rkPacket >> szAttacker;

	MonsterSurvivalRecord *pEscape = FindMonsterSurvivalRecord( pUser );
	if( !pEscape || !pEscape->bPrisoner ) return;

	pEscape->bPrisoner = false;
	pEscape->bDieState = true;
	pEscape->dwCurDieTime = 0;

	int iDamageCnt;
	ioHashString szBestAttacker;
	rkPacket >> iDamageCnt;

	int iTotalDamage = 0;
	int iLastDamage = 0;

	if(iDamageCnt > 100)
	{
		LOG.PrintTimeAndLog(0, "%s CTPK_DROP_DIE Error - DamageCnt:%d", __FUNCTION__, iDamageCnt);
		iDamageCnt = 0;
	}

	if( iDamageCnt > 0 )
	{
		DamageTableList vDamageList;
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			rkPacket >> kDamageTable.szName;
			rkPacket >> kDamageTable.iDamage;

			if( kDamageTable.szName == szAttacker )
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

		szBestAttacker = vDamageList[0].szName;
	}

	if( GetState() == MS_PLAY )
	{
		UpdateWeaponDieRecord( pEscape->pUser, szAttacker, szBestAttacker );
	}

	float fLastRate = 0.0f;
	if( iTotalDamage > 0 )
	{
		fLastRate = (float)iLastDamage / iTotalDamage;
	}

	SP2Packet kReturn( STPK_PRISONER_DROP );
	kReturn << pEscape->pUser->GetPublicID();
	kReturn << szAttacker;
	kReturn << fLastRate;
	GetCharModeInfo( kReturn, pEscape->pUser->GetPublicID() );
	GetCharModeInfo( kReturn, szAttacker );
	SendRoomAllUser( kReturn );
#ifdef ANTIHACK
	pEscape->pUser->SetDieState();
#endif
}

void MonsterSurvivalMode::OnPrisonerMode( User *pUser, SP2Packet &rkPacket )
{
	ioHashString szPrisoner, szLastAttacker, szLastAttackerSkill;
	Vector3 vPos;
	rkPacket >> szPrisoner;
	rkPacket >> szLastAttacker >> szLastAttackerSkill;
	rkPacket >> vPos;

	MonsterSurvivalRecord *pPrisoner = FindMonsterSurvivalRecord( szPrisoner );
	if( !pPrisoner ) return;
	if( pPrisoner->bPrisoner ) return;
	if( pPrisoner->pUser->IsEquipedItem() ) return;

	pPrisoner->dwCurDieTime = 0;
	pPrisoner->bPrisoner = true;

	int iDamageCnt;
	ioHashString szBestAttacker;
	rkPacket >> iDamageCnt;

	if(iDamageCnt > 100)
	{
		LOG.PrintTimeAndLog(0, "%s CTPK_DROP_DIE Error - DamageCnt:%d", __FUNCTION__, iDamageCnt);
		iDamageCnt = 0;
	}

	if( iDamageCnt > 0 )
	{
		DamageTableList vDamageList;
		vDamageList.reserve( iDamageCnt );

		for( int i=0; i < iDamageCnt; ++i )
		{
			DamageTable kDamageTable;
			rkPacket >> kDamageTable.szName;
			rkPacket >> kDamageTable.iDamage;

			vDamageList.push_back( kDamageTable );

			if( kDamageTable.iDamage > 0 )
			{
				ModeRecord *pRecord = FindModeRecord( kDamageTable.szName );
				if( pRecord )
				{
					pRecord->iTotalDamage += kDamageTable.iDamage;
				}
			}
		}

		std::sort( vDamageList.begin(), vDamageList.end(), DamageTableSort() );

		szBestAttacker = vDamageList[0].szName;
	}

	if( GetState() == MS_PLAY )
	{
		UpdateWeaponDieRecord( pPrisoner->pUser, szLastAttacker, szBestAttacker );
	}

	SP2Packet kPacket( STPK_PRISONERMODE );
	kPacket << pPrisoner->pUser->GetPublicID();
	kPacket << szLastAttacker;
	kPacket << szLastAttackerSkill;
	kPacket << vPos;
	GetCharModeInfo( kPacket, pPrisoner->pUser->GetPublicID() );
	GetCharModeInfo( kPacket, szLastAttacker );
	SendRoomAllUser( kPacket );	
}

void MonsterSurvivalMode::OnUseMonsterCoin( User *pUser, SP2Packet &rkPacket )
{
	// start������ Ŭ�󿡼� ���޹��� �ʰ� �������� ���� ó��.
	//park
// 	int iUseCommand = 0;
// 	PACKET_GUARD_VOID( rkPacket.Read(iUseCommand) );
// 
// 	UseMonsterCoin(pUser, iUseCommand);
	int iUseCommand;
	rkPacket >> iUseCommand;

	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
	if( !pRecord ) return;
	if( !pRecord->pUser ) return;	

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnUseMonsterCoin Prev Coin Count %d: %s - (%d:%d)", iUseCommand, pRecord->pUser->GetPublicID().c_str(), 
							pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );

	switch( iUseCommand )
	{
	case USE_MONSTER_COIN_START:
		{
			int iUseStartCoinCnt;
			rkPacket >> iUseStartCoinCnt;

			// ���� ���� �켱 ���
			if( !pRecord->pUser->UseModeStartMonsterCoin( iUseStartCoinCnt ) )
			{
				// ���� ���� �˸�
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_FAIL_CNT;
				kPacket << pRecord->pUser->GetEtcMonsterCoin() << pRecord->pUser->GetEtcGoldMonsterCoin();
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnUseMonsterCoin None Coin Count %d: %s - %d(%d:%d)", iUseCommand, 
									pRecord->pUser->GetPublicID().c_str(), iUseStartCoinCnt, pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );
				return;
			}	

			SP2Packet kPacket( STPK_USE_MONSTER_COIN );

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
		break;
	case USE_MONSTER_COIN_REVIVAL:
		{
			// ���� ���µ� �ƴϰ� ���� ���µ� �ƴϴ�.
			if( !pRecord->bPrisoner && !pRecord->bDieState )
			{
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_LIVE_USER;
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnUseMonsterCoin None Prisoner And Die State %d: %s", iUseCommand, pRecord->pUser->GetPublicID().c_str() );
				return;
			}

			if( !pRecord->pUser->UseGoldMonsterCoin() )
			{
				// ���� ���� �˸�
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_FAIL_CNT;
				kPacket << pRecord->pUser->GetEtcMonsterCoin() << pRecord->pUser->GetEtcGoldMonsterCoin();
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnUseMonsterCoin None Coin Count %d: %s - %d", iUseCommand, 
									pRecord->pUser->GetPublicID().c_str(), pRecord->pUser->GetEtcGoldMonsterCoin() );
				return;
			}	

			// ���� Ż�� ��Ű�� ������ Ǯ ����
			m_iUseGoldCoinRevival++;
			pRecord->bPrisoner = false;
			pRecord->bDieState = false;
			pRecord->dwCurDieTime = 0;

			SP2Packet kPacket( STPK_USE_MONSTER_COIN );
			kPacket << USE_MONSTER_COIN_REVIVAL_OK;
			pRecord->pUser->EquipDBItemToAllChar();
			pRecord->pUser->FillEquipItemData( kPacket );
			ioCharacter *rkChar = pRecord->pUser->GetCharacter( pRecord->pUser->GetSelectChar() );   
			if(rkChar)
			{
				rkChar->FillEquipAccessoryInfo( kPacket, pRecord->pUser );
			}
			else
			{
				PACKET_GUARD_VOID( kPacket.Write( "" ) );
				for( int i = 0; i < MAX_CHAR_ACCESSORY_SLOT; i++ )
				{
					PACKET_GUARD_VOID(kPacket.Write(0));
					PACKET_GUARD_VOID(kPacket.Write(0));
					PACKET_GUARD_VOID(kPacket.Write(0));
				}
			}
			pRecord->pUser->FillEquipMedalItem( kPacket );
			SendRoomAllUser( kPacket );

			m_dwCurContinueTime = 0;
		}
		break;
	}	
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnUseMonsterCoin Use Monster Coin %d: %s - %d:%d", iUseCommand, pRecord->pUser->GetPublicID().c_str(), pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );
}

void MonsterSurvivalMode::OnTurnEndViewState( User *pUser, SP2Packet &rkPacket )
{
	if( !IsEnableState( pUser ) )
		return;

	if( GetState() != MS_PLAY ) 
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnTurnEndViewState None Play State : %s", pUser->GetPublicID().c_str() );
		return;
	}

	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
	if( !pRecord ) return;

	// ���� ���·� �ٲٰ� ���� / ���� ����
	pRecord->bClientViewState = true;
	pRecord->bPrisoner = false;
	pRecord->bDieState = false;
	pRecord->dwCurDieTime = 0;

	// ���� ī��Ʈ �ʱ�ȭ
	if( m_dwCurContinueTime != 0 )
	{
		m_dwCurContinueTime = 0;
		SP2Packet kPacket( STPK_ROUND_CONTINUE );
		kPacket << 0;
		SendRoomAllUser( kPacket );
	}
	// Turn Check.
	NextTurnCheck();
}

void MonsterSurvivalMode::OnAwardingResult( User *pUser, SP2Packet &rkPacket )
{
	if( m_bCheckAwardChoose ) return;

	m_bCheckAwardChoose = true;

	int i = 0;
	int iAwardSize = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iAwardSize) );
	MAX_GUARD(iAwardSize, 50);

	if( iAwardSize == 0 || m_dwModePointTime == 0 )
	{
		m_bAwardStage = false;
		return;	
	}

	// ����� ���� ��÷�� ���� �˻� : ����
	int iSendAwardSize = iAwardSize;
	ioHashString kLuckyUser;
	if( CheckSpacialAwardUser( kLuckyUser ) )
	{
		//������ ������ �û���� ������ �ϳ��� ����. ��, �û� ������ 3�� �����̸� �׳� 4��° �û��� ����.
		if( iAwardSize >= m_iSendAwardCount )  
		{
			iAwardSize -= 1;
		}
		else
		{
			iSendAwardSize += 1;       
		}
	}

	SP2Packet kReturnPacket( STPK_AWARDING_RESULT );
	PACKET_GUARD_VOID( kReturnPacket.Write(m_dwAwardingTime) );
	PACKET_GUARD_VOID( kReturnPacket.Write(iSendAwardSize) );

	// �Ϲ� ����
	if( iAwardSize > 10 || iAwardSize < 0 )
	{
		char szUserIp[STR_IP_MAX];
		int iUserPort = 0;

		pUser->GetPeerIP(szUserIp,STR_IP_MAX,iUserPort);

		LOG.PrintTimeAndLog(0, "MonsterSurvivalMode::OnAwardingResult Cheat UserID:%s, Ip:%s, iAwardSize:%d", pUser->GetPublicID().c_str(), szUserIp, iAwardSize);
		iAwardSize = 4;
	}

	LOOP_GUARD();
	for(i = 0;i < iAwardSize;i++)
	{
		ioHashString szName;
		int iType = 0, iValue = 0;
		PACKET_GUARD_VOID( rkPacket.Read(iType) );
		PACKET_GUARD_VOID( rkPacket.Read(szName) );
		PACKET_GUARD_VOID( rkPacket.Read(iValue) );

		ModeRecord *pRecord = FindModeRecord( szName );
		if( !pRecord || !pRecord->pUser ) 
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::OnAwardingResult() - %s is Not Exist.", szName.c_str() );
			continue;
		}

		float fPlayTimePer = (float)GetRecordPlayTime( pRecord ) / m_dwModeRecordPointTime;
		float fAwardBonus  = GetAwardBonus( iType, fPlayTimePer );		
		int iPoint = max( 1, GetAwardPoint( fPlayTimePer ) );	
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnAwardingResult() - RoomIndex[%d] - UserCount:[%d] - RoomStyle:[%d] - %s - Award[%d] - Point[%d] - Bonus[%.2f] - PlayPer[%.2f]", m_pCreator->GetRoomIndex(),
			m_pCreator->GetPlayUserCnt(), m_pCreator->GetRoomStyle(), pRecord->pUser->GetPublicID().c_str(), iType, iPoint, fAwardBonus, fPlayTimePer );
		pRecord->pUser->AddAward( iType, iPoint );
		pRecord->fBonusArray[BA_AWARD_BONUS] += fAwardBonus;

		PACKET_GUARD_VOID( kReturnPacket.Write(iType) );
		PACKET_GUARD_VOID( kReturnPacket.Write(pRecord->pUser->GetGradeLevel()) );
		PACKET_GUARD_VOID( kReturnPacket.Write(pRecord->pUser->GetTeam()) );
		PACKET_GUARD_VOID( kReturnPacket.Write(szName) );
		PACKET_GUARD_VOID( kReturnPacket.Write(iValue) );
		PACKET_GUARD_VOID( kReturnPacket.Write(iPoint) );
		PACKET_GUARD_VOID( kReturnPacket.Write(fAwardBonus) );
		
		{	// �̺�Ʈ�� Ư�� ������ ����
			int iEtcItemCount = ( iPoint + 100 ) / 100;
			if( iEtcItemCount <= Help::GetAwardEtcItemBonusAbusePoint() )
				iEtcItemCount = 0;
			PACKET_GUARD_VOID( kReturnPacket.Write(Help::GetAwardEtcItemBonus()) );
			PACKET_GUARD_VOID( kReturnPacket.Write(iEtcItemCount) );
			// ������ ����
			g_PresentHelper.SendAwardEtcItemBonus( pRecord->pUser, Help::GetAwardEtcItemBonus(), iEtcItemCount );
		}

		// ���� �� �ð� �÷����ߴ� �뺴�� ����(ġ�� ����)
		int iClassType = pRecord->GetHighPlayingClass();
		pRecord->pUser->FillClassData( iClassType, false, kReturnPacket );
		pRecord->pUser->FillEquipMedalItemByClassType( iClassType, kReturnPacket );
	}
	LOOP_GUARD_CLEAR();

	// ����
	if( !kLuckyUser.IsEmpty() )
	{
		ModeRecord *pRecord = FindModeRecord( kLuckyUser );
		if( pRecord && pRecord->pUser ) 
		{
			DWORD dwPlayingTime = GetRecordPlayTime( pRecord );
			float fPlayTimePer = (float)dwPlayingTime / m_dwModeRecordPointTime;
			float fAwardBonus  = GetAwardBonus( m_dwSpacialAwardType, fPlayTimePer );		
			int iPoint = max( 1, GetAwardPoint( fPlayTimePer ) );	
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::OnAwardingResult() - RoomIndex[%d] - UserCount:[%d] - RoomStyle:[%d] - %s - Award[%d] - Point[%d] - Bonus[%.2f] - PlayPer[%.2f]", m_pCreator->GetRoomIndex(),
				m_pCreator->GetPlayUserCnt(), m_pCreator->GetRoomStyle(), pRecord->pUser->GetPublicID().c_str(), m_dwSpacialAwardType, iPoint, fAwardBonus, fPlayTimePer );
			pRecord->pUser->AddAward( m_dwSpacialAwardType, iPoint );
			pRecord->fBonusArray[BA_AWARD_BONUS] += fAwardBonus;

			PACKET_GUARD_VOID( kReturnPacket.Write(m_dwSpacialAwardType) );
			PACKET_GUARD_VOID( kReturnPacket.Write(pRecord->pUser->GetGradeLevel()) );
			PACKET_GUARD_VOID( kReturnPacket.Write(pRecord->pUser->GetTeam()) );
			PACKET_GUARD_VOID( kReturnPacket.Write(kLuckyUser) );
			PACKET_GUARD_VOID( kReturnPacket.Write(0) );
			PACKET_GUARD_VOID( kReturnPacket.Write(iPoint) );
			PACKET_GUARD_VOID( kReturnPacket.Write(fAwardBonus) );

			{	// �̺�Ʈ�� Ư�� ������ ����
				int iEtcItemCount = ( iPoint + 100 ) / 100;
				if( iEtcItemCount <= Help::GetAwardEtcItemBonusAbusePoint() )
					iEtcItemCount = 0;
				kReturnPacket << Help::GetAwardEtcItemBonus() << iEtcItemCount;
				// ������ ����
				g_PresentHelper.SendAwardEtcItemBonus( pRecord->pUser, Help::GetAwardEtcItemBonus(), iEtcItemCount );
			}

			// ���� �� �ð� �÷����ߴ� �뺴�� ����(ġ�� ����)
			int iClassType = pRecord->GetHighPlayingClass();
			pRecord->pUser->FillClassData( iClassType, false, kReturnPacket );	
			pRecord->pUser->FillEquipMedalItemByClassType( iClassType, kReturnPacket );

			// ���� ����
			float fCurTurnRate = 0.0f;
			if( m_dwSpecialAwardMaxPoint > 0 )
				fCurTurnRate = m_fModeTurnPoint / m_dwSpecialAwardMaxPoint;

			g_PresentHelper.SendMonsterSpacialAwardPresent( pRecord->pUser, fCurTurnRate, kReturnPacket );
		}		
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Mode::OnAwardingResult() - %s is Not Exist.", kLuckyUser.c_str() );
	}
	SendRoomAllUser( kReturnPacket );
	m_bAwardStage = true;	
}

void MonsterSurvivalMode::OnPickRewardItem( User *pUser, SP2Packet &rkPacket )
{
	if( pUser == NULL ) return;

	int iFieldRewardUniqueIndex;
	rkPacket >> iFieldRewardUniqueIndex;

	bool bResult = false;
	FieldRewardItem *pRewardItem = GetFieldRewardItem( iFieldRewardUniqueIndex );
	if( pRewardItem && ( pRewardItem->m_ItemOwner.IsEmpty() || pRewardItem->m_ItemOwner == pUser->GetPublicID() ) )
	{	
		CTimeSpan cPresentGapTime( pRewardItem->m_ItemData.m_iPresentPeriod, 0, 0, 0 );
		CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
		pUser->AddPresentMemory( pRewardItem->m_ItemData.m_szSendID, pRewardItem->m_ItemData.m_iPresentType, 
								 pRewardItem->m_ItemData.m_iPresentValue1, pRewardItem->m_ItemData.m_iPresentValue2, 0, 0,
								 pRewardItem->m_ItemData.m_iPresentMent, kPresentTime, pRewardItem->m_ItemData.m_iPresentState );
		pUser->SendPresentMemory();
		ReleaseFieldRewardItem( iFieldRewardUniqueIndex );
		bResult = true;
	}

	SP2Packet kPacket( STPK_PICK_REWARD_ITEM );
	kPacket << bResult;
	kPacket << pUser->GetPublicID();
	kPacket << iFieldRewardUniqueIndex;
	SendRoomAllUser( kPacket );
}

void MonsterSurvivalMode::UseMonsterCoin( User *pUser, int iUseCommand )
{
	if( !IsEnableState( pUser ) )
		return;

	if( GetState() != MS_PLAY ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonsterSurvivalMode::UseMonsterCoin None Play State : %s", pUser->GetPublicID().c_str() );
		return;
	}

	MonsterSurvivalRecord *pRecord = FindMonsterSurvivalRecord( pUser );
	if( !pRecord ) return;
	if( !pRecord->pUser ) return;	

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin Prev Coin Count %d: %s - (%d:%d)", iUseCommand, pRecord->pUser->GetPublicID().c_str(), 
		pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );

	switch( iUseCommand )
	{
	case USE_MONSTER_COIN_START:
		{
			// �������� ���� 16.2.25 kaedoc �������� ��ŷ ������.
			//int iUseStartCoinCnt;
			//rkPacket >> iUseStartCoinCnt;

			bool bUseGoldCoin = true;
			// ���� ���� �켱 ���
			if( !pRecord->pUser->UseModeStartMonsterCoin( m_dwUseStartCoinCnt, bUseGoldCoin ) )
			{
				// ���� ���� �˸�
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_FAIL_CNT;
				kPacket << pRecord->pUser->GetEtcMonsterCoin() << pRecord->pUser->GetEtcGoldMonsterCoin();
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin None Coin Count %d: %s - %d(%d:%d)", iUseCommand, 
					pRecord->pUser->GetPublicID().c_str(), m_dwUseStartCoinCnt, pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );
				return;
			}	


			// ��� Ȯ�νÿ��� ��� �� ���ξ��� ������.
			SP2Packet kPacket( STPK_USE_MONSTER_COIN );
			PACKET_GUARD_VOID( kPacket.Write((int)USE_MONSTER_COIN_START_OK));
			PACKET_GUARD_VOID( kPacket.Write(m_dwUseStartCoinCnt));
			PACKET_GUARD_VOID( kPacket.Write(bUseGoldCoin));
			pRecord->pUser->SendMessage( kPacket );
		}
		break;
	case USE_MONSTER_COIN_REVIVAL:
		{
			// ���� ���µ� �ƴϰ� ���� ���µ� �ƴϴ�.
			if( !pRecord->bPrisoner && !pRecord->bDieState )
			{
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_LIVE_USER;
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin None Prisoner And Die State %d: %s", iUseCommand, pRecord->pUser->GetPublicID().c_str() );
				return;
			}

			if( !pRecord->pUser->UseGoldMonsterCoin() )
			{
				// ���� ���� �˸�
				SP2Packet kPacket( STPK_USE_MONSTER_COIN );
				kPacket << USE_MONSTER_COIN_FAIL_CNT;
				kPacket << pRecord->pUser->GetEtcMonsterCoin() << pRecord->pUser->GetEtcGoldMonsterCoin();
				pRecord->pUser->SendMessage( kPacket );
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin None Coin Count %d: %s - %d", iUseCommand, 
					pRecord->pUser->GetPublicID().c_str(), pRecord->pUser->GetEtcGoldMonsterCoin() );
				return;
			}	

			// ���� Ż�� ��Ű�� ������ Ǯ ����
			m_iUseGoldCoinRevival++;
			pRecord->bPrisoner = false;
			pRecord->bDieState = false;
			pRecord->dwCurDieTime = 0;

			SP2Packet kPacket( STPK_USE_MONSTER_COIN );
			kPacket << USE_MONSTER_COIN_REVIVAL_OK;
			pRecord->pUser->EquipDBItemToAllChar();
			pRecord->pUser->FillEquipItemData( kPacket );
			pRecord->pUser->FillEquipMedalItem( kPacket );
			SendRoomAllUser( kPacket );

			m_dwCurContinueTime = 0;
		}
		break;
	default: 
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin iUseCommand is not Initialize!  %d: %s - %d:%d", iUseCommand, pRecord->pUser->GetPublicID().c_str() );
		}
	}	
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "MonsterSurvivalMode::UseMonsterCoin Use Monster Coin %d: %s - %d:%d", iUseCommand, pRecord->pUser->GetPublicID().c_str(), pRecord->pUser->GetEtcGoldMonsterCoin(), pRecord->pUser->GetEtcMonsterCoin() );

}
