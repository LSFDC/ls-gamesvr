#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"

#include "User.h"
#include "Room.h"
#include "RoomNodeManager.h"
#include "ModeHelp.h"
#include "HeadquartersMode.h"
#include "ioMonsterMapLoadMgr.h"
#include "ioSetItemInfoManager.h"
#include "ioItemInfoManager.h"
#include "ioSetItemInfo.h"
#include "../EtcHelpFunc.h"
#include ".\ioexercisecharindexmanager.h"
#include "../DataBase/LogDBClient.h"
#include "../NodeInfo/ioPowerUpManager.h"

HeadquartersMode::HeadquartersMode( Room *pCreator ) : Mode( pCreator )
{
	m_bCharacterCreate     = false;
	m_iCharacterCreateNameCount = 0;

	m_bJoinLock = false;

	m_TeamList.clear();
	m_TeamPosArray.clear();

	m_dwCharState = STATE_OPEN_CHAR;

	m_iCurRound = 2;		
}

HeadquartersMode::~HeadquartersMode()
{
	CharacterEquipSlotMap::iterator iCreate = m_CharacterEquipSlotMap.begin();
	for(;iCreate != m_CharacterEquipSlotMap.end();++iCreate)
	{
		SAFEDELETE( iCreate->second );
	}
	m_CharacterEquipSlotMap.clear();
	m_MonsterTable.m_MonsterCodeList.clear();
	m_MonsterTable.m_StartXPosList.clear();
	m_MonsterTable.m_StartZPosList.clear();
}

void HeadquartersMode::LoadRoundCtrlValue( ioINILoader &rkLoader )
{
	rkLoader.SetTitle( "info" );
	m_iMaxPlayer = min( MAX_PLAZA_PLAYER, rkLoader.LoadInt( "max_player", MAX_PLAZA_PLAYER ) );
	m_iConstMaxPlayer = min( MAX_PLAZA_PLAYER, rkLoader.LoadInt( "const_max_player", MAX_PLAZA_PLAYER ) );

	rkLoader.SetTitle( "round" );

	m_iMaxRound  = rkLoader.LoadInt( "max_round", 4 );

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

	int i = 0;
	// ���常ŭ ���� ����.
	m_vRoundHistory.clear();
	for(i = 0; i < m_iMaxRound;i++)
	{
		RoundHistory rh;
		m_vRoundHistory.push_back( rh );
	}

	rkLoader.SetTitle( "record" );
	m_iKillDeathPoint = rkLoader.LoadInt( "kill_death_point", 10 );
	m_iWinPoint		  = rkLoader.LoadInt( "win_point", 30 );
	m_iDrawPoint	  = rkLoader.LoadInt( "draw_point", 20 );
	m_iLosePoint	  = rkLoader.LoadInt( "lose_point", 10 );

	m_TeamList.clear();
	for(i=TEAM_PRIVATE_1;i < TEAM_PRIVATE_190+1;i++)
	{
		m_TeamList.push_back( i );
	}

	// ���� ���� ( ������ K )
	char szKey[MAX_PATH] = "";
	rkLoader.SetTitle( "character" );
	int iMaxCharacter = rkLoader.LoadInt( "max_character", 0 );
	for(i = 0;i < iMaxCharacter;i++)
	{
		HeadquartersCharRecord kCharacter;
		
		// NPC �ڵ�
		sprintf_s( szKey, "character%d_code", i + 1 );
		kCharacter.dwCode = rkLoader.LoadInt( szKey, 0 );

		// NPC ��� �ð�
		sprintf_s( szKey, "character%d_start_time", i + 1 );
		kCharacter.dwStartTime = rkLoader.LoadInt( szKey, 0 );

		// NPC ���� ��ġ
		sprintf_s( szKey, "character%d_start_x", i + 1 );
		kCharacter.fStartXPos = rkLoader.LoadFloat( szKey, 0.0f );
		sprintf_s( szKey, "character%d_start_z", i + 1 );
		kCharacter.fStartZPos = rkLoader.LoadFloat( szKey, 0.0f );

		// NPC ��
		kCharacter.eTeam = GetNextTeamType();
		RemoveTeamType( kCharacter.eTeam );

		// NPC �̸��� ������ �ڵ����� �����.
		char szNpcName[MAX_PATH] = "";
		sprintf_s( szNpcName, " -N%d- ", ++m_iCharacterCreateNameCount );        // �¿쿡 ������ �־ ���� �г��Ӱ� �ߺ����� �ʵ��� �Ѵ�.
		kCharacter.szName = szNpcName;

		m_vCharacterList.push_back( kCharacter );
	}

	// ���� ���� ( ���� ������ �� ���� )
	rkLoader.SetTitle( "monster_info" );

	IntVec kMonster;
	char szBuf[MAX_PATH] = "";
	rkLoader.LoadString( "monster_code", "", szBuf, MAX_PATH );
	Help::SplitString( szBuf, kMonster, '.' );
	for(i = 0;i < (int)kMonster.size();i++)
	{
		m_MonsterTable.m_MonsterCodeList.push_back( (DWORD)kMonster[i] );
	}

	m_MonsterTable.m_iMonsterCreateCount = rkLoader.LoadInt( "monster_limit_count", 0 );
	m_MonsterTable.m_dwDropItemIndex     = rkLoader.LoadInt( "monster_drop_item", 0 );
	for(i = 0;i < m_MonsterTable.m_iMonsterCreateCount;i++)
	{
		// ���� ��ǥ
		sprintf_s( szKey, "monster_startx_%d", i + 1 );
		m_MonsterTable.m_StartXPosList.push_back( rkLoader.LoadFloat( szKey, 0.0f ) );
		sprintf_s( szKey, "monster_startz_%d", i + 1 );
		m_MonsterTable.m_StartZPosList.push_back( rkLoader.LoadFloat( szKey, 0.0f ) );
	}
	m_bCharacterCreate = false;	
	SetStartPosArray();
}

void HeadquartersMode::SetStartPosArray()
{
	int iTeamPosCnt = m_TeamList.size();
	if(iTeamPosCnt <= 1) // �ּ��� 2�� �ʿ�
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "Error 2 - HeadquartersMode::GetStartPosArray");
		return;
	}

	m_TeamPosArray.reserve(iTeamPosCnt);
	for( int i=0; i<iTeamPosCnt; i++ )
	{
		m_TeamPosArray.push_back(i);
	}

	std::random_shuffle( m_TeamPosArray.begin(), m_TeamPosArray.end() );

	m_iBluePosArray = m_TeamPosArray[0];
	m_iRedPosArray  = m_TeamPosArray[1];
}

void HeadquartersMode::DestroyMode()
{
	Mode::DestroyMode();

	m_vRecordList.clear();
}

void HeadquartersMode::SetCharState( DWORD dwState )
{
	if( m_dwCharState == dwState )
		return;

	m_dwCharState = dwState;
	
	if( GetRecordCnt() == 0 ) 
		return;

	CheckCharacterCreate( true );
}

void HeadquartersMode::InsertEquipSlotMap( DWORD dwCharIndex )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter != m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::InsertEquipSlotMap Already Insert : %d", dwCharIndex );
		return;
	}
	
	m_CharacterEquipSlotMap.insert( CharacterEquipSlotMap::value_type( dwCharIndex, new ioEquipSlot ) );
}

void HeadquartersMode::DeleteEquipSlotMap( DWORD dwCharIndex )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::DeleteEquipSlotMap None Data : %d", dwCharIndex );
		return;
	}

	SAFEDELETE( iter->second );
	m_CharacterEquipSlotMap.erase( iter );
}

ioItem* HeadquartersMode::EquipItem( DWORD dwCharIndex, ioItem *pItem )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::EquipItem None Data : %d", dwCharIndex );
		return NULL;
	}
	
	ioEquipSlot *pEquipSlot = iter->second;
	if( pEquipSlot )
		return pEquipSlot->EquipItem( pItem );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::EquipItem Null Data : %d", dwCharIndex );
	return NULL;
}

ioItem* HeadquartersMode::EquipItem( DWORD dwCharIndex, int iSlot, ioItem *pItem )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::EquipItem None Data : %d - %d", dwCharIndex, iSlot );
		return NULL;
	}

	ioEquipSlot *pEquipSlot = iter->second;
	if( pEquipSlot )
		return pEquipSlot->EquipItem( iSlot, pItem );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::EquipItem Null Data : %d - %d", dwCharIndex, iSlot );
	return NULL;
}

ioItem* HeadquartersMode::ReleaseItem( DWORD dwCharIndex, int iSlot )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::ReleaseItem None Data : %d - %d", dwCharIndex, iSlot );
		return NULL;
	}

	ioEquipSlot *pEquipSlot = iter->second;
	if( pEquipSlot )
		return pEquipSlot->ReleaseItem( iSlot );
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::ReleaseItem Null Data : %d - %d", dwCharIndex, iSlot );
	return NULL;
}

ioItem* HeadquartersMode::ReleaseItem( DWORD dwCharIndex, int iGameIndex, int iItemCode )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::ReleaseItem None Data : %d - %d - %d", dwCharIndex, iGameIndex, iItemCode );
		return NULL;
	}

	ioEquipSlot *pEquipSlot = iter->second;
	if( pEquipSlot )
		return pEquipSlot->ReleaseItem( iGameIndex, iItemCode );
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::ReleaseItem Null Data : %d - %d - %d", dwCharIndex, iGameIndex, iItemCode );
	return NULL;
}

void HeadquartersMode::FillCharEquipItem( DWORD dwCharIndex, SP2Packet &rkPacket )
{
	CharacterEquipSlotMap::iterator iter = m_CharacterEquipSlotMap.find( dwCharIndex );
	if( iter == m_CharacterEquipSlotMap.end() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::FillEquipItem None Data : %d ", dwCharIndex );
		for(int i = 0;i < MAX_EQUIP_SLOT;i++)
		{
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write("") );
		}
		return;
	}

	ioEquipSlot *pEquipSlot = iter->second;
	if( pEquipSlot )
		pEquipSlot->FillEquipItemInfo( rkPacket );
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::FillEquipItem Null Data : %d ", dwCharIndex );
		for(int i = 0;i < MAX_EQUIP_SLOT;i++)
		{
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write(0) );
			PACKET_GUARD_VOID( rkPacket.Write("") );
		}
	}
}

void HeadquartersMode::AddNewRecord( User *pUser )
{
	HeadquartersRecord kRecord;
	kRecord.pUser = pUser;
	m_vRecordList.push_back( kRecord );

	RemoveTeamType( pUser->GetTeam() );
	UpdateUserRank();

	if( pUser && pUser->GetPublicID() == m_szMasterName )
	{
		// �ٸ� �������� ������� �� �����̿��� ����
		if( GetRecordCnt() > 1 )
		{
			ReSetCreateCharacter( pUser );
			CheckCharacterCreate( true );
		}
	}
	else
	{
		if( m_dwCharState != STATE_DISPLAY_CHAR)
			SetCharState( STATE_DISPLAY_CHAR );
		else
		{
			// �߰� ���� ���� NPC ����ȭ
			PlayCharacterSync( &kRecord );
		}
	}
}

void HeadquartersMode::RemoveRecord( User *pUser, bool bRoomDestroy )
{
	if(!pUser) return;
	
	int iCharCnt = m_vRecordList.size();
	for( int i=0 ; i<iCharCnt ; i++ )
	{
		if( m_vRecordList[i].pUser == pUser )
		{
			SetModeEndDBLog( &m_vRecordList[i], iCharCnt, LogDBClient::PRT_EXIT_ROOM );
			m_vRecordList.erase( m_vRecordList.begin() + i );
			AddTeamType( pUser->GetTeam() );			
			break;
		}
	}

	UpdateUserRank();
	
	if( pUser )
	{
		m_KickOutVote.RemoveVoteUserList( pUser->GetPublicID() );
		RemoveRecordChangeCharacterSync( pUser->GetPublicID() );
	}
}

void HeadquartersMode::AddTeamType( TeamType eTeam )
{
	if( COMPARE( eTeam, TEAM_PRIVATE_1, TEAM_PRIVATE_120+1 ) )
		m_TeamList.push_back( eTeam ); 
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::AddTeamType %d", eTeam );
}

void HeadquartersMode::RemoveTeamType( TeamType eTeam )
{
	int iTeamSize = m_TeamList.size();
	for(int i = 0;i < iTeamSize;i++)
	{
		if( m_TeamList[i] == eTeam )
		{
			m_TeamList.erase( m_TeamList.begin() + i );
			break;
		}
	}
}

void HeadquartersMode::ReSetCreateCharacter( User *pUser )
{
	if( pUser == NULL ) return;

	int i = 0;

	// �뺴 NPC ���� ����
	for(i = 0;i < (int)m_vCharacterList.size();)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCode == 0 )
		{
			// ����
			HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
			if( pSyncUserRecord )
				pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

			AddTeamType( rkCharacter.eTeam );

			for(int k = 0;k < MAX_CHAR_DBITEM_SLOT;k++)
			{
				// �������� ������ ����
				ioItem *pPreItem = ReleaseItem( rkCharacter.dwCharIndex, k );
				SAFEDELETE( pPreItem );
			}		
			DeleteEquipSlotMap( rkCharacter.dwCharIndex );
			m_vCharacterList.erase( m_vCharacterList.begin() + i );
		}
		else
		{
			i++;
		}
	}

	// �뺴 NPC �߰�
	for(i = 0;i < pUser->GetCharCount();i++)
	{
		ioCharacter *pCharacter = pUser->GetCharacter( i );
		if( pCharacter == NULL ) continue;

		InsertCharacter( pUser, pCharacter );
	}
}

void HeadquartersMode::CreateCharacter( User *pUser )
{
	if( pUser == NULL ) return;

	m_bJoinLock = pUser->IsHeadquartersLock();

	int i = 0;
	for(i = 0;i < pUser->GetCharCount();i++)
	{
		ioCharacter *pCharacter = pUser->GetCharacter( i );
		if( pCharacter == NULL ) continue;

		InsertCharacter( pUser, pCharacter );
	}
}

void HeadquartersMode::InsertCharacter( User *pUser, ioCharacter *pCharacter )
{
	if( pUser == NULL ) return;
	if( pCharacter == NULL ) return;

	HeadquartersCharRecord kCharacter;

	// ���� ��ġ�� ������ Ŭ���̾�Ʈ �������� ����
	kCharacter.fStartXPos = 0.0f; 
	kCharacter.fStartZPos = 0.0f;

	// ��
	kCharacter.eTeam = GetNextTeamType();
	RemoveTeamType( kCharacter.eTeam );

	// �̸��� ������ �ڵ����� �����.
	char szNpcName[MAX_PATH] = "";
	sprintf_s( szNpcName, " -N%d- ", ++m_iCharacterCreateNameCount );        // �¿쿡 ������ �־ ���� �г��Ӱ� �ߺ����� �ʵ��� �Ѵ�.
	kCharacter.szName = szNpcName;

	//
	kCharacter.bDisplayCharacter = pUser->GetDisplayCharacter( pCharacter->GetCharIndex(), kCharacter.fStartXPos, kCharacter.fStartZPos );  // ���� On / Off
	kCharacter.dwDisplayEtcMotion= pUser->GetDisplayCharMotion( pCharacter->GetCharIndex() );
	kCharacter.dwCharIndex = pCharacter->GetCharIndex();
	kCharacter.iClassLevel = pUser->GetClassLevelByType( pCharacter->GetCharInfo().m_class_type, false );
	kCharacter.kCharInfo   = (CHARACTER)pCharacter->GetCharInfo();

	int k = 0;
	// ���
	ioUserExtraItem *pExtraItem = pUser->GetUserExtraItem();
	if( pExtraItem )
	{
		for(k = 0;k < MAX_CHAR_DBITEM_SLOT;k++)
		{
			ioUserExtraItem::EXTRAITEMSLOT kSlot;
			if( pExtraItem->GetExtraItem( kCharacter.kCharInfo.m_extra_item[k], kSlot) )
			{
				kCharacter.kEquipItem[k].m_item_code = kSlot.m_iItemCode;
				kCharacter.kEquipItem[k].m_item_reinforce = kSlot.m_iReinforce;
				kCharacter.kEquipItem[k].m_item_male_custom = kSlot.m_dwMaleCustom;
				kCharacter.kEquipItem[k].m_item_female_custom = kSlot.m_dwFemaleCustom;
			}
		}
	}

	// �޴�
	pUser->GetEquipMedalClassTypeArray( kCharacter.kCharInfo.m_class_type, kCharacter.vEquipMedal );

	// ����
	ioUserGrowthLevel *pGrowthLevel = pUser->GetUserGrowthLevel();
	if( pGrowthLevel )
	{
		for(k = 0;k < MAX_CHAR_GROWTH;k++)
			kCharacter.kCharGrowth[k] = pGrowthLevel->GetCharGrowthLevel( kCharacter.kCharInfo.m_class_type, k, false );
		for(k = 0;k < MAX_ITEM_GROWTH;k++)
			kCharacter.kItemGrowth[k] = pGrowthLevel->GetItemGrowthLevel( kCharacter.kCharInfo.m_class_type, k, false );
	}

	InsertEquipSlotMap( kCharacter.dwCharIndex );
	m_vCharacterList.push_back( kCharacter );
}

void HeadquartersMode::FillCharacterInfo( HeadquartersCharRecord &rkCharacter, bool bCreate, SP2Packet &rkPacket )
{
	if( m_pCreator == NULL ) return;

	// �ε��� / ����
	rkPacket << rkCharacter.dwCharIndex << rkCharacter.iClassLevel;

	// ĳ���� ����
	rkPacket << rkCharacter.kCharInfo;

	// ��� ����
	rkPacket << rkCharacter.dwDisplayEtcMotion;

	int i = 0;
	// ������ ���� 
	if( bCreate )
	{
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
		{
			ITEM_DATA kItemData = rkCharacter.kEquipItem[i];
			if( kItemData.m_item_code == 0 )
			{
				if( rkCharacter.kCharInfo.m_byReinforceGrade != 0 )
				{
					int iCode = (100000 * i) + g_PowerUpMgr.ConvertPowerUpTypeCharToItem(rkCharacter.kCharInfo.m_byReinforceGrade) + rkCharacter.kCharInfo.m_class_type;
					const ItemInfo *pItemInfo = g_ItemInfoMgr.GetItemInfo( iCode );
					if( pItemInfo )
					{
						kItemData.m_item_code = iCode;
					}
					else
						kItemData.m_item_code = ( 100000 * i ) + rkCharacter.kCharInfo.m_class_type;
				}
				else
				{
					kItemData.m_item_code = ( 100000 * i ) + rkCharacter.kCharInfo.m_class_type;
				}
			}

			// ������ ����
			ioItem *pNewItem = m_pCreator->CreateItem( kItemData, rkCharacter.szName );
			if( pNewItem )
			{
				ioItem *pPreItem = EquipItem( rkCharacter.dwCharIndex, i, pNewItem );
				SAFEDELETE( pPreItem );			
			}
		}
	}
	FillCharEquipItem( rkCharacter.dwCharIndex, rkPacket );

	// �޴�
	rkPacket << (int)rkCharacter.vEquipMedal.size();
	for(i = 0;i < (int)rkCharacter.vEquipMedal.size();i++)
	{
		PACKET_GUARD_VOID( rkPacket.Write(rkCharacter.vEquipMedal[i]) );
	}    

	// ����
	for(i = 0;i < MAX_CHAR_GROWTH;i++)
	{
		PACKET_GUARD_VOID( rkPacket.Write(rkCharacter.kCharGrowth[i]) );
	}

	for(i = 0;i < MAX_ITEM_GROWTH;i++)
	{
		PACKET_GUARD_VOID( rkPacket.Write(rkCharacter.kItemGrowth[i]) );
	}
}

void HeadquartersMode::FillCharacterSimpleInfo( HeadquartersCharRecord &rkCharacter, bool bCreate, SP2Packet &rkPacket )
{

	// �ε��� / ����
	PACKET_GUARD_VOID( rkPacket.Write(rkCharacter.dwCharIndex) );

	// ĳ���� ���� - �ʿ� ����

	// ��� ����
	PACKET_GUARD_VOID( rkPacket.Write(rkCharacter.dwDisplayEtcMotion) );

	int i = 0;
	// ������ ���� 
	if( bCreate )
	{
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
		{
			ITEM_DATA kItemData = rkCharacter.kEquipItem[i];
			if( kItemData.m_item_code == 0 )
			{
				int iCode = (100000 * i) + g_PowerUpMgr.ConvertPowerUpTypeCharToItem(rkCharacter.kCharInfo.m_byReinforceGrade) + rkCharacter.kCharInfo.m_class_type;
				const ItemInfo *pItemInfo = g_ItemInfoMgr.GetItemInfo( iCode );
				if( pItemInfo )
				{
					kItemData.m_item_code = iCode;
				}
				else
					kItemData.m_item_code = ( 100000 * i ) + rkCharacter.kCharInfo.m_class_type;
			}

			// ������ ����
			ioItem *pNewItem = m_pCreator->CreateItem( kItemData, rkCharacter.szName );
			if( pNewItem )
			{
				ioItem *pPreItem = EquipItem( rkCharacter.dwCharIndex, i, pNewItem );
				SAFEDELETE( pPreItem );			
			}
		}
	}
	FillCharEquipItem( rkCharacter.dwCharIndex, rkPacket );

	// �޴� - �ʿ� ����

	// ���� - �ʿ� ����
}

void HeadquartersMode::ProcessPlay()
{
	CheckCharacterCreate( false );
	ProcessRevival();
	CheckFieldItemLiveTime();
	CheckItemSupply( m_dwStateChangeTime );	
	CheckBallSupply( m_dwStateChangeTime );
	CheckMachineSupply( m_dwStateChangeTime );
	CheckNeedSendPushStruct();
}

void HeadquartersMode::ProcessRevival()
{
	Mode::ProcessRevival();

	DWORD dwCurTime = TIMEGETTIME();
	int iCharacterCnt = m_vCharacterList.size();
	for(int i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCurDieTime == 0 ) continue;
		if( rkCharacter.eState != RS_DIE ) continue;
		if( rkCharacter.bAI && m_dwCharState != STATE_DISPLAY_CHAR ) continue;

		if( rkCharacter.dwCurDieTime + 5000 < dwCurTime )
		{
			rkCharacter.dwCurDieTime = 0;
			rkCharacter.eState	= RS_PLAY;
			rkCharacter.szSyncUser= SearchCharacterSyncUser();
			if( rkCharacter.dwNPCIndex == 0 )
				rkCharacter.dwNPCIndex = GetUniqueMonsterIDGenerate();

			// �ΰ����� ž�� ���ʹ� ��ü�Ѵ�.
			if( rkCharacter.bAI )
			{
				// NPC �ڵ�
				if( m_MonsterTable.m_MonsterCodeList.size() > 1 )
					std::random_shuffle( m_MonsterTable.m_MonsterCodeList.begin(), m_MonsterTable.m_MonsterCodeList.end() ); 

				rkCharacter.dwCode = *m_MonsterTable.m_MonsterCodeList.begin();

				// NPC �̸��� ������ �ڵ����� �����.
				char szNpcName[MAX_PATH] = "";
				sprintf_s( szNpcName, " -N%d- ", ++m_iCharacterCreateNameCount );        // �¿쿡 ������ �־ ���� �г��Ӱ� �ߺ����� �ʵ��� �Ѵ�.
				rkCharacter.szName = szNpcName;
			}

			SP2Packet kPacket( STPK_MONSTER_REVIVAL );
#ifdef ANTIHACK
			kPacket << rkCharacter.dwNPCIndex << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#else
			kPacket << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#endif
			
			
			if( rkCharacter.dwCode == 0 )
			{
				if( m_dwCharState == STATE_OPEN_CHAR )
					FillCharacterSimpleInfo( rkCharacter, true, kPacket );
				else
					FillCharacterInfo( rkCharacter, true, kPacket );
			}

			SendRoomAllUser( kPacket );
		}
	}
}

void HeadquartersMode::UpdateRoundRecord()
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
				pRecord->pUser->DeleteExpiredBonusCash();
			}
		}
	}

	UpdateUserRank();	
}

void HeadquartersMode::CheckCharacterDeleteByAI()
{
	// ����ִ� AI ���� ���� ���� ó��.
	ioHashStringVec vDieMonsterName;	
	HeadquartersCharRecordList::iterator iter = m_vCharacterList.begin();
	for(;iter != m_vCharacterList.end();)
	{
		HeadquartersCharRecord &rkCharacter = *iter;
		if( !rkCharacter.bAI )
		{
			iter++;
			continue;
		}

		HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
		if( pSyncUserRecord )
			pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

		vDieMonsterName.push_back( rkCharacter.szName );

		AddTeamType( rkCharacter.eTeam );
		iter = m_vCharacterList.erase( iter );
	}

	if( vDieMonsterName.empty() )
		return;

	// ���� ���� ����
	int iDieSize = vDieMonsterName.size();
	SP2Packet kPacket( STPK_MONSTER_FORCE_DIE );
	kPacket << iDieSize;
	for(int i = 0;i < iDieSize;i++)
	{
		kPacket << vDieMonsterName[i];
	}
	SendRoomAllUser( kPacket );
}

void HeadquartersMode::CheckCharacterCreate( bool bForceCreate )
{
	if( m_bCharacterCreate && !bForceCreate ) return;
	if( m_vCharacterList.empty() ) return;

	m_bCharacterCreate = true;
	CheckCharacterDeleteByAI();

	SP2Packet kPacket( STPK_TURN_START );
	kPacket << m_dwCharState;
	switch( m_dwCharState )
	{
	case STATE_OPEN_CHAR:
		{
			// ��� �뺴 / ������ K ����, �� �뺴 ����
			int i = 0;
			int iMaxCharacter = 0;
			int iSelectClassType = 0;

			// �������� �뺴�� ��� ���� 
			HeadquartersRecord *pMasterRecord = FindHeadquartersRecord( m_szMasterName );
			if( pMasterRecord && pMasterRecord->pUser )
			{
				iSelectClassType = pMasterRecord->pUser->GetSelectClassType();
			}

			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
				if( pSyncUserRecord )
					pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

				rkCharacter.eState         = RS_LOADING;
				rkCharacter.szSyncUser.Clear();
				
				if( rkCharacter.dwCode != 0 || rkCharacter.kCharInfo.m_class_type != iSelectClassType )
					iMaxCharacter++;
			}

			int iStartTime = 0;
			kPacket << iMaxCharacter;
			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				if( rkCharacter.dwCode == 0 && rkCharacter.kCharInfo.m_class_type == iSelectClassType ) continue;

				rkCharacter.eState         = RS_PLAY;
				rkCharacter.szSyncUser     = SearchCharacterSyncUser();
				rkCharacter.dwStartTime    = 300 * ++iStartTime;
				if( rkCharacter.dwNPCIndex == 0 )
					rkCharacter.dwNPCIndex	= GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
				kPacket << rkCharacter.dwNPCIndex  << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#else
				kPacket << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#endif
				

				if( rkCharacter.dwCode == 0 )
				{
					FillCharacterSimpleInfo( rkCharacter, true, kPacket );
				}
			}
		}
		break;
	case STATE_UNITE_CHAR:
		{
			// ������K�� ����
			int i = 0;
			int iMaxCharacter = 0;
			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
				if( pSyncUserRecord )
					pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

				rkCharacter.eState         = RS_LOADING;
				rkCharacter.szSyncUser.Clear();

				if( rkCharacter.dwCode != 0 )
					iMaxCharacter++;
			}

			int iStartTime = 0;
			kPacket << iMaxCharacter;
			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				if( rkCharacter.dwCode == 0 ) continue;

				rkCharacter.eState         = RS_PLAY;
				rkCharacter.szSyncUser     = SearchCharacterSyncUser();
				rkCharacter.dwStartTime    = 300 * ++iStartTime;
				if( rkCharacter.dwNPCIndex == 0 )
					rkCharacter.dwNPCIndex     = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
				kPacket << rkCharacter.dwNPCIndex << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#else
				kPacket << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#endif
				
			}
		}
		break;
	case STATE_DISPLAY_CHAR:
		{
			// ������K + ���� �뺴 ����
			int i = 0;
			int iMaxCharacter = 0;

			// AI ���� �߰�
			if( !m_MonsterTable.m_MonsterCodeList.empty() )
			{
				int iCreateMonster = min( m_MonsterTable.m_iMonsterCreateCount, (int)m_MonsterTable.m_StartXPosList.size() );
				for(i = 0;i < iCreateMonster;i++)
				{
					HeadquartersCharRecord kCharacter;

					// NPC �ڵ�
					if( m_MonsterTable.m_MonsterCodeList.size() > 1 )
						std::random_shuffle( m_MonsterTable.m_MonsterCodeList.begin(), m_MonsterTable.m_MonsterCodeList.end() ); 

					kCharacter.dwCode = *m_MonsterTable.m_MonsterCodeList.begin();

					// NPC ��� �ð�
					kCharacter.dwStartTime = 300 * (i + 1 );

					// NPC ���� ��ġ
					kCharacter.fStartXPos = m_MonsterTable.m_StartXPosList[i];
					kCharacter.fStartZPos = m_MonsterTable.m_StartZPosList[i];

					// NPC ��
					kCharacter.eTeam = GetNextTeamType();
					RemoveTeamType( kCharacter.eTeam );

					// NPC �̸��� ������ �ڵ����� �����.
					char szNpcName[MAX_PATH] = "";
					sprintf_s( szNpcName, " -N%d- ", ++m_iCharacterCreateNameCount );        // �¿쿡 ������ �־ ���� �г��Ӱ� �ߺ����� �ʵ��� �Ѵ�.
					kCharacter.szName = szNpcName;

					// AI
					kCharacter.bAI = true;

					m_vCharacterList.push_back( kCharacter );
				}
			}

			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
				if( pSyncUserRecord )
					pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

				rkCharacter.eState         = RS_LOADING;
				rkCharacter.szSyncUser.Clear();

				if( rkCharacter.dwCode != 0 || rkCharacter.bDisplayCharacter )
					iMaxCharacter++;
			}

			int iStartTime = 0;
			kPacket << iMaxCharacter;
			for(i = 0;i < (int)m_vCharacterList.size();i++)
			{
				HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];

				if( rkCharacter.dwCode == 0 && rkCharacter.bDisplayCharacter == false ) continue;

				rkCharacter.eState         = RS_PLAY;
				rkCharacter.szSyncUser     = SearchCharacterSyncUser();
				rkCharacter.dwStartTime    = 300 * ++iStartTime;
				if( rkCharacter.dwNPCIndex == 0 )
					rkCharacter.dwNPCIndex     = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
				kPacket << rkCharacter.dwNPCIndex << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#else
				kPacket << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser 
					<< rkCharacter.dwStartTime << rkCharacter.fStartXPos << rkCharacter.fStartZPos << (int)rkCharacter.eTeam;
#endif
				

				if( rkCharacter.dwCode == 0 )
				{
					FillCharacterInfo( rkCharacter, true, kPacket );
				}
			}
		}
		break;
	}		
	SendRoomAllUser( kPacket );
}

const ioHashString &HeadquartersMode::SearchCharacterSyncUser()
{
	static ioHashString szError = "����ȭ��������";
	if( m_vRecordList.empty() ) 
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::SearchCharacterSyncUser(%d) : None User Record", m_pCreator->GetRoomIndex() );
		return szError;
	}

	HeadquartersRecord *pReturnRecord = NULL;
	HeadquartersRecordList::iterator iter = m_vRecordList.begin();
	for(;iter != m_vRecordList.end();iter++)
	{
		HeadquartersRecord &rkRecord = *iter;
		if( rkRecord.pUser == NULL ) continue;
		if( !pReturnRecord )
		{
			pReturnRecord = &rkRecord;
			continue;
		}		
		if( rkRecord.eState != RS_PLAY ) continue;

		int iPrevPoint = pReturnRecord->pUser->GetPingStep() + pReturnRecord->iMonsterSyncCount;
		int iNextPoint = rkRecord.pUser->GetPingStep() + rkRecord.iMonsterSyncCount;
		if( iPrevPoint > iNextPoint )
			pReturnRecord = &rkRecord;
	}

	if( pReturnRecord == NULL || pReturnRecord->pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::SearchCharacterSyncUser(%d) : None Return Record : %d", m_pCreator->GetRoomIndex(), m_vRecordList.size() );
		return szError;
	}

	// ���� �Ѹ��� ����ȭ �߰�~
	pReturnRecord->iMonsterSyncCount++;
	return pReturnRecord->pUser->GetPublicID();
}

void HeadquartersMode::RemoveRecordChangeCharacterSync( const ioHashString &rkRemoveName )
{
	if( m_vRecordList.empty() ) return;
	if( rkRemoveName.IsEmpty() ) return;

	HeadquartersCharRecordList vSyncRecord;
	// ���� NPC�� ����ȭ �Ű澲�� �ʰ� ����ִ� NPC�� üũ�Ͽ� �����鿡�� ����
	int i;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.eState != RS_PLAY ) continue;
		if( rkCharacter.szSyncUser != rkRemoveName ) continue;

		rkCharacter.szSyncUser = SearchCharacterSyncUser();
		vSyncRecord.push_back( rkCharacter );
	}

	if( vSyncRecord.empty() ) return;

	int iSyncSize = vSyncRecord.size();
	SP2Packet kPacket( STPK_MONSTER_SYNC_CHANGE );
	kPacket << iSyncSize;
	for(i = 0;i < iSyncSize;i++)
	{
		HeadquartersCharRecord &rkCharacter = vSyncRecord[i];
		kPacket << rkCharacter.szName << rkCharacter.szSyncUser;
	}
	SendRoomAllUser( kPacket );
	vSyncRecord.clear();
}

void HeadquartersMode::PlayCharacterSync( HeadquartersRecord *pSendRecord )
{
	if( pSendRecord == NULL || pSendRecord->pUser == NULL ) return;

	// �÷������� NPC�� ����ȭ ���� ����.
	int i;
	HeadquartersCharRecordList vSyncRecord;           // ����ȭ ����
	int iMonsterCnt = m_vCharacterList.size();
	for(i = 0;i < iMonsterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.eState != RS_PLAY ) continue;

		vSyncRecord.push_back( rkCharacter );
	}

	SP2Packet kPacket( STPK_MONSTER_INFO_SYNC );
	kPacket << m_dwCharState;

	int iSyncSize = vSyncRecord.size();	
	kPacket << iSyncSize;
	for(i = 0;i < iSyncSize;i++)
	{
		HeadquartersCharRecord &rkCharacter = vSyncRecord[i];
		if( rkCharacter.dwNPCIndex == 0 )
			rkCharacter.dwNPCIndex = GetUniqueMonsterIDGenerate();

#ifdef ANTIHACK
		kPacket << rkCharacter.dwNPCIndex << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser << (int)rkCharacter.eTeam;
#else
		kPacket << rkCharacter.dwCode << rkCharacter.szName << rkCharacter.szSyncUser << (int)rkCharacter.eTeam;
#endif
		
		
		if( rkCharacter.dwCode == 0 )
		{
			if( m_dwCharState == STATE_OPEN_CHAR )
				FillCharacterSimpleInfo( rkCharacter, false, kPacket );
			else
				FillCharacterInfo( rkCharacter, false, kPacket );
		}
	}
	pSendRecord->pUser->SendMessage( kPacket ); 
	vSyncRecord.clear();
}

ModeType HeadquartersMode::GetModeType() const
{
	return MT_HEADQUARTERS;
}

void HeadquartersMode::GetCharModeInfo( SP2Packet &rkPacket, const ioHashString &szName, bool bDieCheck )
{
	HeadquartersRecord *pRecord = FindHeadquartersRecord( szName );
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

bool HeadquartersMode::ProcessTCPPacket( User *pSend, SP2Packet &rkPacket )
{
	if( Mode::ProcessTCPPacket( pSend, rkPacket ) )
		return true;
	
	switch( rkPacket.GetPacketID() )
	{
	case CTPK_HEADQUARTERS_STATE_CHANGE:
		OnCharStateChange( pSend, rkPacket );
		return true;
	case CTPK_HEADQUARTERS_USER_INVITE:
		OnUserInvite( pSend, rkPacket );
		return true;
	}
	return false;
}

void HeadquartersMode::OnCharStateChange( User *pSend, SP2Packet &rkPacket )
{
	if( pSend == NULL ) return;
	
	DWORD dwCharState;
	rkPacket >> dwCharState;

	if( pSend->GetPublicID() == m_szMasterName )
	{
		// ���� ����� ������ ������ ���� ���� �ȵȴ�.
		if( GetRecordCnt() + m_pCreator->GetReserveUserSize() == 1 && dwCharState != m_dwCharState )
		{
			SP2Packet kPacket( STPK_HEADQUARTERS_STATE_CHANGE );
			kPacket << true << dwCharState;
			pSend->SendMessage( kPacket );

			SetCharState( dwCharState );
		}
		else 
		{
			SP2Packet kPacket( STPK_HEADQUARTERS_STATE_CHANGE );
			kPacket << false;
			pSend->SendMessage( kPacket );
		}
	}
	else
	{
		SP2Packet kPacket( STPK_HEADQUARTERS_STATE_CHANGE );
		kPacket << false;
		pSend->SendMessage( kPacket );
	}
}

void HeadquartersMode::OnUserInvite( User *pSend, SP2Packet &rkPacket )
{
	if( m_pCreator->IsRoomFull() )
		return;

	int i = 0;
	int iInviteCount;
	rkPacket >> iInviteCount;

	SP2Packet kPacket( STPK_HEADQUARTERS_USER_INVITE );
	kPacket << m_szMasterName;
	kPacket << GetMaxPlayer();
	
	int iRecordCnt = GetRecordCnt();
	kPacket << iRecordCnt;
	for(i = 0;i < iRecordCnt;i++)
	{
		ModeRecord *pRecord = FindModeRecord( i );
		if( !pRecord || !pRecord->pUser )
		{
			kPacket << "" << 0 << 0;
			continue;
		}
		if( pRecord->pUser->IsObserver() || pRecord->pUser->IsStealth() )
		{
			kPacket << "" << 0 << 0;
			continue;
		}
		
		kPacket << pRecord->pUser->GetPublicID() << pRecord->pUser->GetGradeLevel() << pRecord->pUser->GetPingStep();
	}

    
	for(i = 0;i < iInviteCount;i++)
	{
		ioHashString szInvitedID;
		rkPacket >> szInvitedID;

		UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( szInvitedID );
		if( pUser )
		{			
			pUser->RelayPacket( kPacket );
		}
	}
}

int HeadquartersMode::GetRecordCnt() const
{
	return m_vRecordList.size();
}

const char* HeadquartersMode::GetModeINIFileName() const
{
	return "config/headquartersmode.ini";
}

void HeadquartersMode::GetModeInfo( SP2Packet &rkPacket )
{
	rkPacket << GetModeType();
	rkPacket << m_szMasterName;
	rkPacket << m_bJoinLock;
	rkPacket << GetMaxPlayer();

	int iPosCnt = m_TeamPosArray.size();
	rkPacket << iPosCnt;
	for( int i=0; i < iPosCnt; ++i )
		rkPacket << m_TeamPosArray[i];
}

TeamType HeadquartersMode::GetNextTeamType()
{
	if( m_TeamList.empty() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::GetNextTeamType Not Team : %d", (int)m_vRecordList.size() );
		return TEAM_NONE;
	}

	return (TeamType)m_TeamList[0];
}

int HeadquartersMode::GetUserKickVoteLimit( const ioHashString &szKickUserName )
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
	return 0;
}

bool HeadquartersMode::IsMasterJoin()
{
	ModeRecord *pRecord = FindModeRecord( m_szMasterName );
	if( pRecord )
		return true;
	return false;
}

ModeRecord* HeadquartersMode::FindModeRecord( const ioHashString &rkName )
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

ModeRecord* HeadquartersMode::FindModeRecord( User *pUser )
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

ModeRecord* HeadquartersMode::FindModeRecord( int iIdx )
{
	if( COMPARE( iIdx, 0, GetRecordCnt() ) )
		return &m_vRecordList[iIdx];

	return NULL;
}

HeadquartersRecord* HeadquartersMode::FindHeadquartersRecord( const ioHashString &rkName )
{
	return (HeadquartersRecord*)FindModeRecord( rkName );
}

HeadquartersRecord* HeadquartersMode::FindHeadquartersRecord( User *pUser )
{
	return (HeadquartersRecord*)FindModeRecord( pUser );
}

HeadquartersRecord* HeadquartersMode::FindHeadquartersRecordByUserID( const ioHashString &rkUserID )
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

HeadquartersCharRecord* HeadquartersMode::FindCharacterInfo( const ioHashString &rkName )
{
	int i;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord *pCharacter = &m_vCharacterList[i];
		if( pCharacter == NULL ) continue;
		if( pCharacter->szName == rkName )
			return pCharacter;
	}
	return NULL;
}

void HeadquartersMode::UpdateUserDieTime( User *pDier )
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

void HeadquartersMode::InitObjectGroupList()
{
	ioINILoader &rkLoader = g_ModeINIMgr.GetINI( GetModeINIFileName() );

	int iSubNum = GetModeSubNum();
	int iGroupNum = GetModeMapNum();
	char szBuf[MAX_PATH], szTitle[MAX_PATH];

	m_vPushStructList.clear();
	m_iPushStructIdx = 0;

	wsprintf( szTitle, "headquarters%d_object_group%d", iSubNum, iGroupNum );
	rkLoader.SetTitle( szTitle );

	int iPushStructCnt = rkLoader.LoadInt( "push_struct_cnt", 0 );
	m_vPushStructList.reserve( iPushStructCnt );

	//Push Struct
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
	vObjectItemList.reserve( iObjectItemCnt );

	//Object Item
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

	int iNewItemCnt = vItemList.size();
	SP2Packet kPacket( STPK_FIELD_ITEM_SUPPLY );
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

Vector3 HeadquartersMode::GetRandomItemPos(ioItem *pItem)
{
	if( m_vItemCreatePosList.empty() )
		return Vector3( 0.0f, 0.0f, 0.0f );

	Vector3 vPos;
	if( !m_vItemShufflePosList.empty() )
	{
		vPos = m_vItemShufflePosList.front();
		m_vItemShufflePosList.pop_front();
		return vPos;
	}

	m_vItemShufflePosList.clear();
	m_vItemShufflePosList.insert( m_vItemShufflePosList.begin(),
								  m_vItemCreatePosList.begin(),
								  m_vItemCreatePosList.end() );

	std::random_shuffle( m_vItemShufflePosList.begin(),
						 m_vItemShufflePosList.end() );

	vPos = m_vItemShufflePosList.front();
	m_vItemShufflePosList.pop_front();
	return vPos;
}

void HeadquartersMode::UpdateMonsterDieRecord( const ioHashString &szAttacker, const ioHashString &szBestAttacker )
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


void HeadquartersMode::OnMonsterDropItemPos( Vector3Vec &rkPosList, float fRange )
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

void HeadquartersMode::OnMonsterDieToItemDrop( float fDropX, float fDorpZ, const ioHashString &rkDropName )
{
	if( m_MonsterTable.m_dwDropItemIndex == 0 ) return;

	Vector3Vec vRandPos;
	OnMonsterDropItemPos( vRandPos, 150.0f );
	std::random_shuffle( vRandPos.begin(), vRandPos.end() );
	//

	ITEM_DATA kItem;
	kItem.Initialize();
	g_MonsterMapLoadMgr.GetMonsterDropItem( m_MonsterTable.m_dwDropItemIndex, kItem );
	if( kItem.m_item_code == 0 ) return;

	ioItem *pItem = m_pCreator->CreateItem( kItem, rkDropName );
	if( pItem )
	{
		Vector3 vPos( fDropX + vRandPos[0].x, 0.0f, fDorpZ + vRandPos[0].z );
		pItem->SetItemPos( vPos );
		pItem->SetDropTime( TIMEGETTIME() );
		m_pCreator->AddFieldItem( pItem );

		SP2Packet kPacket( STPK_FIELD_ITEM_SUPPLY );
		kPacket << 1;

		kPacket << pItem->GetItemCode();
		kPacket << pItem->GetItemReinforce();
		kPacket << pItem->GetItemMaleCustom();
		kPacket << pItem->GetItemFemaleCustom();
		kPacket << pItem->GetGameIndex();
		kPacket << pItem->GetItemPos();
		kPacket << pItem->GetOwnerName();
		kPacket << "";
		SendRoomAllUser( kPacket );
	}		
}

void HeadquartersMode::OnDropDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	HeadquartersCharRecord *pDieCharacter = FindCharacterInfo( rkDieName );
	if( !pDieCharacter ) return;
	if( pDieCharacter->eState != RS_PLAY ) return;

	float fDiePosX, fDiePosZ;
	rkPacket >> fDiePosX >> fDiePosZ;

	int iLastAttackerTeam;
	ioHashString szLastAttackerName;
	ioHashString szLastAttackerSkillName;
	DWORD dwLastAttackerWeaponItemCode;
	rkPacket >> szLastAttackerName >> szLastAttackerSkillName >> dwLastAttackerWeaponItemCode >> iLastAttackerTeam;

	// ���� ���� ó��
	{
		pDieCharacter->eState = RS_DIE;
		pDieCharacter->dwCurDieTime = TIMEGETTIME();
		HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( pDieCharacter->szSyncUser );
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
	kReturn << pDieCharacter->szName;
	kReturn << szLastAttackerName;
	kReturn << szLastAttackerSkillName;
	kReturn << dwLastAttackerWeaponItemCode;
	kReturn << iLastAttackerTeam;
	kReturn << szBestAttackerName;
	kReturn << fLastRate;
	kReturn << fBestRate;
	GetCharModeInfo( kReturn, pDieCharacter->szName );
	GetCharModeInfo( kReturn, szLastAttackerName );
	SendRoomAllUser( kReturn );
}

void HeadquartersMode::OnWeaponDieNpc( const ioHashString &rkDieName, SP2Packet &rkPacket )
{
	HeadquartersCharRecord *pDieCharacter = FindCharacterInfo( rkDieName );
	if( !pDieCharacter )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnWeaponDieNpc(%d) - None Npc : %s", m_pCreator->GetRoomIndex(), rkDieName.c_str() );
		return;
	}

	if( pDieCharacter->eState != RS_PLAY )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnWeaponDieNpc(%d) - None Live Npc : %s", m_pCreator->GetRoomIndex(), rkDieName.c_str() );
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
		pDieCharacter->eState = RS_DIE;
		pDieCharacter->dwCurDieTime = TIMEGETTIME();
		HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( pDieCharacter->szSyncUser );
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
	kReturn << pDieCharacter->szName;
	kReturn << szLastAttackerName;
	kReturn << szLastAttackerSkillName;
	kReturn << dwLastAttackerWeaponItemCode;
	kReturn << iLastAttackerTeam;
	kReturn << szBestAttackerName;
	kReturn << fLastRate;
	kReturn << fBestRate;
	GetCharModeInfo( kReturn, pDieCharacter->szName );
	GetCharModeInfo( kReturn, szLastAttackerName );
	SendRoomAllUser( kReturn );

	// ������ ���
	if( pDieCharacter->bAI )
	{
		OnMonsterDieToItemDrop( fDiePosX, fDiePosZ, pDieCharacter->szName );
	}
}

void HeadquartersMode::OnDropItemNpc( User *pSendUser, const ioHashString &rkOwnerID, SP2Packet &rkPacket )
{
	HeadquartersCharRecord *pCharacter = FindCharacterInfo( rkOwnerID );
	if( !pCharacter )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnDropItemNpc(%d) - None Npc : %s", m_pCreator->GetRoomIndex(), rkOwnerID.c_str() );
		return;
	}

	int iGameIndex, iItemCode, iSlot;
	rkPacket >> iGameIndex >> iItemCode >> iSlot;

	ioItem *pPreItem = ReleaseItem( pCharacter->dwCharIndex, iGameIndex, iItemCode );

	if( !pPreItem || !IsCanPickItemState() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnDropItemNpc - %s Not Has Item %d", rkOwnerID.c_str(), iItemCode );
		
		if( pSendUser )
		{
			SP2Packet kFailReturn( STPK_DROP_ITEM_FAIL );
			kFailReturn << rkOwnerID;
			kFailReturn << iSlot;
			pSendUser->SendMessage( kFailReturn );
		}
		return;
	}

	float fCurGauge;
	rkPacket >> fCurGauge;
	pPreItem->SetCurItemGauge( fCurGauge );

	int iCurBullet;
	rkPacket >> iCurBullet;

	Vector3 vDropPos;
	rkPacket >> vDropPos;
	pPreItem->SetItemPos( vDropPos );

	bool bDropZone;
	rkPacket >> bDropZone;

	if( pPreItem->IsNotDeleteItem() && pPreItem->GetCrownItemType() == ioItem::MCT_NONE && bDropZone )
	{
		pPreItem->SetEnableDelete( false );
	}
	m_pCreator->DropItemOnField( rkOwnerID, pPreItem, iSlot, fCurGauge, iCurBullet );
}

void HeadquartersMode::OnDropMoveItemNpc( User *pSendUser, const ioHashString &rkOwnerID, SP2Packet &rkPacket )
{
	HeadquartersCharRecord *pCharacter = FindCharacterInfo( rkOwnerID );
	if( !pCharacter )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnDropMoveItemNpc(%d) - None Npc : %s", m_pCreator->GetRoomIndex(), rkOwnerID.c_str() );
		return;
	}

	int iGameIndex, iItemCode, iSlot;
	rkPacket >> iGameIndex >> iItemCode >> iSlot;

	ioItem *pPreItem = ReleaseItem( pCharacter->dwCharIndex, iGameIndex, iItemCode );

	if( !pPreItem || !IsCanPickItemState() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnDropMoveItemNpc - %s Not Has Item %d", rkOwnerID.c_str(), iItemCode );
		
		if( pSendUser )
		{
			SP2Packet kFailReturn( STPK_DROP_ITEM_FAIL );
			kFailReturn << rkOwnerID;
			kFailReturn << iSlot;
			pSendUser->SendMessage( kFailReturn );
		}
		return;
	}

	float fCurGauge;
	rkPacket >> fCurGauge;
	pPreItem->SetCurItemGauge( fCurGauge );

	int iCurBullet;
	rkPacket >> iCurBullet;

	// Drop Info
	ioHashString szAttacker, szSkillName;
	Vector3 vStartPos, vDropPos;
	float fMoveSpeed;

	rkPacket >> szAttacker >> szSkillName >> vStartPos >> vDropPos >> fMoveSpeed;

	pPreItem->SetItemPos( vStartPos );

	m_pCreator->MoveDropItemOnField( rkOwnerID, pPreItem, iSlot, fCurGauge, iCurBullet, szAttacker, szSkillName, vDropPos, fMoveSpeed );
	m_pCreator->NotifyDropItemToMode( pPreItem );
}

bool HeadquartersMode::OnModeChangeChar( User *pSend, int iCharArray, bool bWait, int iSelectCharArray, DWORD dwCharChangeIndex )
{
	if( m_dwCharState != STATE_OPEN_CHAR )
		return false;

	if( !pSend ) return false;
	if( pSend->GetPublicID() != m_szMasterName ) return false;

	if( !COMPARE( iCharArray, 0, pSend->GetCharCount() ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnModeChangeChar - %s Char Overflow(%d)", pSend->GetPublicID().c_str(), iCharArray );
		return false;
	}
	pSend->SetSelectCharArray( iCharArray );

	// ���ĳ��� �뺴�� ��ü
	int i = 0;
	ioHashString kLeaveCharName;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCode != 0 ) continue;

		if( rkCharacter.kCharInfo.m_class_type == pSend->GetSelectClassType() )
		{		
			// ������ ��ü
			ioCharacter *pCharacter = pSend->GetCharacter( pSend->GetSelectChar() );
			if( pCharacter == NULL )
				return false;

			if( rkCharacter.eState == RS_LOADING )
			{
				pSend->EquipDBItemToAllChar();
			}
			else
			{
				for(int k = 0;k < MAX_CHAR_DBITEM_SLOT;k++)
				{
					ioItem *pPrevItem = pCharacter->ReleaseItem( k );
					SAFEDELETE( pPrevItem );

					//
					pPrevItem = ReleaseItem( rkCharacter.dwCharIndex, k );
					if( pPrevItem )
					{
						// �ٲ����� �뺴�� ���� ������ ������ �ִ´�.
						pCharacter->EquipItem( k, pPrevItem );
					}
				}

				// ĳ���ʹ� ��� ���·� ��ȯ
				HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
				if( pSyncUserRecord )
					pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

				rkCharacter.eState         = RS_LOADING;
				rkCharacter.szSyncUser.Clear();
				kLeaveCharName = rkCharacter.szName;
			}
		}
	}

	//ĳ���� ���� ����.
	{
		SP2Packet kPacket( STPK_CHANGE_CHAR );
		PACKET_GUARD_bool( kPacket.Write(pSend->GetPublicID()) );
		PACKET_GUARD_bool( kPacket.Write(CHANGE_CHAR_OK) );
		PACKET_GUARD_bool( kPacket.Write(pSend->GetCountRSoldier()) );
		PACKET_GUARD_bool( kPacket.Write(pSend->GetCountOfSpecialSoldier(SST_GFRIEND)) );
		PACKET_GUARD_bool( kPacket.Write(bWait) );
		PACKET_GUARD_bool( kPacket.Write(dwCharChangeIndex) );

		pSend->FillChangeCharData( kPacket );
		pSend->FillEquipMedalItem( kPacket );
		pSend->FillGrowthLevelData( kPacket );
 
		//���� ���� Īȣ ���� GET
		if( !pSend->FillEquipTitleData(kPacket) )
		{
			PACKET_GUARD_bool( kPacket.Write(pSend->GetPublicID()) );
			PACKET_GUARD_bool( kPacket.Write(0) );
			PACKET_GUARD_bool( kPacket.Write((BYTE)0) );
			PACKET_GUARD_bool( kPacket.Write(0) );
		}
		
		int iClassType = 0;
		ioCharacter *rkChar = pSend->GetCharacter( pSend->GetSelectChar() );
		if( rkChar )	iClassType = rkChar->GetCharInfo().m_class_type;
		pSend->FillExMedalSlotByClassType( iClassType, kPacket );
		pSend->FillEquipPetData( kPacket );
		
		// ������ ������ �뺴�� ������
		int iPrevCharType = pSend->GetCharClassType( iSelectCharArray );
		if( iPrevCharType == 0 )
			iPrevCharType = pSend->GetSelectClassType();
		NotifyChangeChar( pSend, iCharArray, iPrevCharType );
		m_pCreator->RoomSendPacketTcpSenderLast( kPacket, pSend );
	}

	// NPC�� �뺴 ��ü
	if( !kLeaveCharName.IsEmpty() )
	{
		SP2Packet kPacket( STPK_CHANGE_CHAR_BY_NPC );		
		PACKET_GUARD_bool( kPacket.Write(kLeaveCharName) );     // ������ ĳ����

		// �߰��� ĳ����
		for(i = 0;i < iCharacterCnt;i++)
		{
			HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
			if( rkCharacter.dwCode != 0 ) continue;
			if( rkCharacter.szName == kLeaveCharName ) continue;
			if( rkCharacter.eState != RS_LOADING ) continue;
			
			rkCharacter.eState         = RS_PLAY;
			rkCharacter.szSyncUser     = SearchCharacterSyncUser();
			rkCharacter.dwStartTime    = 0;
			if( rkCharacter.dwNPCIndex == 0 )
				rkCharacter.dwNPCIndex     = GetUniqueMonsterIDGenerate();
			
#ifdef ANTIHACK
			PACKET_GUARD_bool( kPacket.Write(rkCharacter.dwNPCIndex) );
#endif	
			PACKET_GUARD_bool( kPacket.Write(rkCharacter.dwCode) );
			PACKET_GUARD_bool( kPacket.Write(rkCharacter.szName) ); 
			PACKET_GUARD_bool( kPacket.Write(rkCharacter.szSyncUser) );
			PACKET_GUARD_bool( kPacket.Write((int)rkCharacter.eTeam) );

			if( rkCharacter.dwCode == 0 )
			{
				FillCharacterSimpleInfo( rkCharacter, true, kPacket );
			}
			break;
		}

		m_pCreator->RoomSendPacketTcpSenderLast( kPacket, pSend );
	}
	return true;
}

void HeadquartersMode::OnModeChangeDisplayMotion( User *pSend, DWORD dwEtcItem, int iClassType )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;
	
	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.kCharInfo.m_class_type != iClassType) continue;
		
		rkCharacter.dwDisplayEtcMotion = dwEtcItem;

		SP2Packet kPacket( STPK_HEADQUARTERS_COMMAND );
		kPacket << HEADQUARTERS_CMD_MOTION_CHANGE << iClassType << dwEtcItem;
		SendRoomAllUser( kPacket );
	}
}

void HeadquartersMode::OnModeCharDecoUpdate( User *pSend, ioCharacter *pCharacter )
{
	if( pSend == NULL ) return;
	if( pCharacter == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex != pCharacter->GetCharIndex() ) continue;
	
		//
		rkCharacter.kCharInfo   = (CHARACTER)pCharacter->GetCharInfo();
		// ġ���� Ŭ���̾�Ʈ���� UDP�� ����ȭ �ϹǷ� TCP ����ȭ �ʿ����
	}
}

void HeadquartersMode::OnModeCharExtraItemUpdate( User *pSend, DWORD dwCharIndex, int iSlot, int iNewIndex )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;
	if( !COMPARE( iSlot, 0, MAX_CHAR_DBITEM_SLOT ) ) return;

	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex != dwCharIndex ) continue;

		if( iNewIndex == 0 )	// ����
		{
			// �⺻ ���
			rkCharacter.kEquipItem[iSlot].Initialize();		
		}
		else
		{
			// ��� ����
			ioUserExtraItem::EXTRAITEMSLOT kSlot;
			ioUserExtraItem *pUserExtraItem = pSend->GetUserExtraItem();
			if( pUserExtraItem == NULL )
				rkCharacter.kEquipItem[iSlot].Initialize();
			else if( !pUserExtraItem->GetExtraItem( iNewIndex, kSlot ) )
				rkCharacter.kEquipItem[iSlot].Initialize();
			else
			{		
				rkCharacter.kEquipItem[iSlot].m_item_code = kSlot.m_iItemCode;
				rkCharacter.kEquipItem[iSlot].m_item_reinforce = kSlot.m_iReinforce;
				rkCharacter.kEquipItem[iSlot].m_item_male_custom = kSlot.m_dwMaleCustom;
				rkCharacter.kEquipItem[iSlot].m_item_female_custom = kSlot.m_dwFemaleCustom;
			}
		}

		if( rkCharacter.eState != RS_PLAY ) 
			break;

		// ��� ��ü
		ioItem *pPreItem = ReleaseItem( rkCharacter.dwCharIndex, iSlot );
		if( pPreItem )
		{
			SAFEDELETE( pPreItem );  // ���� ��� ����

			ITEM_DATA kItemData = rkCharacter.kEquipItem[iSlot];
			if( kItemData.m_item_code == 0 )
			{
				int iCode = (100000 * iSlot) + g_PowerUpMgr.ConvertPowerUpTypeCharToItem(rkCharacter.kCharInfo.m_byReinforceGrade) + rkCharacter.kCharInfo.m_class_type;
				const ItemInfo *pItemInfo = g_ItemInfoMgr.GetItemInfo( iCode );
				if( pItemInfo )
				{
					kItemData.m_item_code = iCode;
				}
				else
					kItemData.m_item_code = ( 100000 * iSlot ) + rkCharacter.kCharInfo.m_class_type;
			}

			// ������ ����
			ioItem *pNewItem = m_pCreator->CreateItem( kItemData, rkCharacter.szName );
			if( pNewItem )
			{
				ioItem *pPreItem = EquipItem( rkCharacter.dwCharIndex, iSlot, pNewItem );
				SAFEDELETE( pPreItem );			

				SP2Packet kPacket( STPK_MODE_EXTRAITEM_UPDATE );
				kPacket << rkCharacter.szName << pNewItem->GetGameIndex() << kItemData.m_item_code << kItemData.m_item_reinforce 
						<< kItemData.m_item_male_custom << kItemData.m_item_female_custom;
				SendRoomAllUser( kPacket );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "HeadquartersMode::OnModeCharExtraItemUpdate(%s) : %d Item Create Fail", m_szMasterName.c_str(), kItemData.m_item_code );

				SP2Packet kPacket( STPK_MODE_EXTRAITEM_UPDATE );
				kPacket << rkCharacter.szName << -1 << kItemData.m_item_code << kItemData.m_item_reinforce 
						<< kItemData.m_item_male_custom << kItemData.m_item_female_custom;
				SendRoomAllUser( kPacket );
			}
		}
		break;
	}
}

void HeadquartersMode::OnModeCharMedalUpdate( User *pSend, DWORD dwCharIndex, int iMedalType, bool bEquip )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex != dwCharIndex ) continue;
		
		if( bEquip )
			rkCharacter.vEquipMedal.push_back( iMedalType );
		else
		{
			int iMedalSize = (int)rkCharacter.vEquipMedal.size();
			for(int k = 0;k < iMedalSize;k++)
			{
				if( rkCharacter.vEquipMedal[k] == iMedalType )
				{
					rkCharacter.vEquipMedal.erase( rkCharacter.vEquipMedal.begin() + k );
					break;
				}
			}
		}

		if( rkCharacter.eState != RS_PLAY ) 
			break;

		int iMedalSize = (int)rkCharacter.vEquipMedal.size();
		
		SP2Packet kPacket( STPK_MODE_MEDAL_UPDATE );
		kPacket << rkCharacter.szName << iMedalType << bEquip;
		SendRoomAllUser( kPacket, pSend );
		break;
	}
}

void HeadquartersMode::OnModeCharGrowthUpdate( User *pSend, int iClassType, int iSlot, bool bItem, int iUpLevel )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.kCharInfo.m_class_type != iClassType ) continue;

		if( bItem )
		{	
			if( COMPARE( iSlot, 0, MAX_ITEM_GROWTH ) )
				rkCharacter.kItemGrowth[iSlot] = max( 0, rkCharacter.kItemGrowth[iSlot] + (BYTE)iUpLevel );
		}
		else
		{
			if( COMPARE( iSlot, 0, MAX_CHAR_GROWTH ) )
				rkCharacter.kCharGrowth[iSlot] = max( 0, rkCharacter.kItemGrowth[iSlot] + (BYTE)iUpLevel );
		}
		break;
	}
}

void HeadquartersMode::OnModeCharInsert( User *pSend, ioCharacter *pCharacter )
{
	if( pSend == NULL ) return;
	if( pCharacter == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	bool bInsert = true;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex == pCharacter->GetCharIndex() )
		{
			// ������ ������ ����		
			bInsert = false;

			rkCharacter.iClassLevel = pSend->GetClassLevelByType( pCharacter->GetCharInfo().m_class_type, false );
			rkCharacter.kCharInfo   = (CHARACTER)pCharacter->GetCharInfo();

			int k = 0;
			// ���
			ioUserExtraItem *pExtraItem = pSend->GetUserExtraItem();
			if( pExtraItem )
			{
				for(k = 0;k < MAX_CHAR_DBITEM_SLOT;k++)
				{
					ioUserExtraItem::EXTRAITEMSLOT kSlot;
					if( pExtraItem->GetExtraItem( rkCharacter.kCharInfo.m_extra_item[k], kSlot) )
					{
						rkCharacter.kEquipItem[k].m_item_code = kSlot.m_iItemCode;
						rkCharacter.kEquipItem[k].m_item_reinforce = kSlot.m_iReinforce;
						rkCharacter.kEquipItem[k].m_item_male_custom = kSlot.m_dwMaleCustom;
						rkCharacter.kEquipItem[k].m_item_female_custom = kSlot.m_dwFemaleCustom;

						// �������� ������ ����
						ioItem *pPreItem = ReleaseItem( rkCharacter.dwCharIndex, k );
						SAFEDELETE( pPreItem );
					}
				}
			}

			// �޴�
			rkCharacter.vEquipMedal.clear();
			pSend->GetEquipMedalClassTypeArray( rkCharacter.kCharInfo.m_class_type, rkCharacter.vEquipMedal );

			// ����
			ioUserGrowthLevel *pGrowthLevel = pSend->GetUserGrowthLevel();
			if( pGrowthLevel )
			{
				for(k = 0;k < MAX_CHAR_GROWTH;k++)
					rkCharacter.kCharGrowth[k] = pGrowthLevel->GetCharGrowthLevel( rkCharacter.kCharInfo.m_class_type, k, false );
				for(k = 0;k < MAX_ITEM_GROWTH;k++)
					rkCharacter.kItemGrowth[k] = pGrowthLevel->GetItemGrowthLevel( rkCharacter.kCharInfo.m_class_type, k, false );
			}
			break;
		}
	}

	//
	if( bInsert )
	{
		InsertCharacter( pSend, pCharacter );
	}

	if( m_dwCharState != STATE_OPEN_CHAR )
		return;

	// ĳ���� ����
	for(i = 0;i < (int)m_vCharacterList.size();i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex == pCharacter->GetCharIndex() )
		{
			rkCharacter.eState         = RS_PLAY;
			rkCharacter.szSyncUser     = SearchCharacterSyncUser();
			rkCharacter.dwStartTime    = 0;
			if( rkCharacter.dwNPCIndex == 0 )
				rkCharacter.dwNPCIndex     = GetUniqueMonsterIDGenerate();

			SP2Packet kPacket( STPK_MODE_CHAR_INSERT_DELETE );

			PACKET_GUARD_VOID( kPacket.Write(true) );
#ifdef ANTIHACK
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.dwNPCIndex) );
#endif	
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.dwCode) );
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.szName) );
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.szSyncUser) );
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.dwStartTime) );
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.fStartXPos) );
			PACKET_GUARD_VOID( kPacket.Write(rkCharacter.fStartZPos) );
			PACKET_GUARD_VOID( kPacket.Write((int)rkCharacter.eTeam) );

			FillCharacterSimpleInfo( rkCharacter, true, kPacket );
			SendRoomAllUser( kPacket );
			break;
		}
	}
}

void HeadquartersMode::OnModeCharDelete( User *pSend, DWORD dwCharIndex )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	ioHashString kDeleteName;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex == dwCharIndex )
		{
			// ������ ������ ����		
			HeadquartersRecord *pSyncUserRecord = FindHeadquartersRecord( rkCharacter.szSyncUser );
			if( pSyncUserRecord )
				pSyncUserRecord->iMonsterSyncCount = max( 0, pSyncUserRecord->iMonsterSyncCount - 1 );

			AddTeamType( rkCharacter.eTeam );

			for(int k = 0;k < MAX_CHAR_DBITEM_SLOT;k++)
			{
				// �������� ������ ����
				ioItem *pPreItem = ReleaseItem( rkCharacter.dwCharIndex, k );
				SAFEDELETE( pPreItem );
			}		
			kDeleteName = rkCharacter.szName;
			DeleteEquipSlotMap( rkCharacter.dwCharIndex );
			m_vCharacterList.erase( m_vCharacterList.begin() + i );
			break;
		}
	}

	if( kDeleteName.IsEmpty() ) return;
	if( m_dwCharState == STATE_UNITE_CHAR ) return;

	SP2Packet kPacket( STPK_MODE_CHAR_INSERT_DELETE );
	kPacket << false << kDeleteName;
	SendRoomAllUser( kPacket );
}

void HeadquartersMode::OnModeCharDisplayUpdate( User *pSend, DWORD dwCharIndex )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	int i = 0;
	int iCharacterCnt = m_vCharacterList.size();
	for(i = 0;i < iCharacterCnt;i++)
	{
		HeadquartersCharRecord &rkCharacter = m_vCharacterList[i];
		if( rkCharacter.dwCharIndex == dwCharIndex )
		{
			rkCharacter.fStartXPos = 0.0f; 
			rkCharacter.fStartZPos = 0.0f;
			rkCharacter.bDisplayCharacter = pSend->GetDisplayCharacter( dwCharIndex, rkCharacter.fStartXPos, rkCharacter.fStartZPos );  // ���� On / Off
			rkCharacter.dwDisplayEtcMotion= pSend->GetDisplayCharMotion( dwCharIndex );
			break;
		}
	}
}

void HeadquartersMode::OnModeJoinLockUpdate( User *pSend, bool bJoinLock )
{
	if( pSend == NULL ) return;
	if( m_szMasterName != pSend->GetPublicID() ) return;

	m_bJoinLock = bJoinLock;

	SP2Packet kPacket( STPK_HEADQUARTERS_COMMAND );
	kPacket << HEADQUARTERS_CMD_JOINLOCK_CHANGE << m_bJoinLock;
	SendRoomAllUser( kPacket );
}

void HeadquartersMode::OnModeLogoutAlarm( const ioHashString &rkMasterName )
{
	if( rkMasterName != m_szMasterName ) return;

	SP2Packet kPacket( STPK_HEADQUARTERS_COMMAND );
	kPacket << HEADQUARTERS_CMD_MASTER_LOGOUT;
	SendRoomAllUser( kPacket );
}

HeadquartersCharRecord* HeadquartersMode::GetCharacterInfo( int iCharCode )
{
	int iSize = m_vCharacterList.size();

	for( int i = 0; i < iSize; i++ )
	{
		if( m_vCharacterList[i].kCharInfo.m_class_type == iCharCode )
			return &m_vCharacterList[i];
	}

	return NULL;
}

void HeadquartersMode::SetCharacterReinforceInfo( int iCode, int iReinforce )
{
	HeadquartersCharRecord* pInfo = GetCharacterInfo(iCode);
	if( !pInfo )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[error][headquater] no exist char info : [code:%d]", iCode);
		return;
	}

	pInfo->kCharInfo.m_byReinforceGrade = iReinforce;
}