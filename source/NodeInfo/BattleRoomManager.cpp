#include "stdafx.h"

//#include "../Window.h"
#include "../MainProcess.h"

#include "User.h"
#include "BattleRoomManager.h"
#include "LevelMatchManager.h"
#include "ServerNodeManager.h"

BattleRoomManager* BattleRoomManager::sg_Instance = NULL;
BattleRoomManager::BattleRoomManager()
{
	m_dwCurTime = 0;
	m_iMaxBattleRoomSize = 0;
	m_iBattleDefaultCatchModeLevel = 11;
	m_iEnableExtraOption = 0;

	InitTotalModeList();
}

BattleRoomManager::~BattleRoomManager()
{
}

BattleRoomManager &BattleRoomManager::GetInstance()
{
	if( sg_Instance == NULL )
		sg_Instance = new BattleRoomManager;
	return *sg_Instance;
}

void BattleRoomManager::ReleaseInstance()
{
	SAFEDELETE( sg_Instance );
}

void BattleRoomManager::InitMemoryPool( const DWORD dwServerIndex )
{
	{
		ioINILoader kLoader( "ls_config_game.ini" );
		kLoader.SetTitle( "MemoryPool" );
		m_iMaxBattleRoomSize = kLoader.LoadInt( "battleroom_pool", 3000 );
	}
	
	int iStartIndex      = dwServerIndex * m_iMaxBattleRoomSize;
	int i = 0;

	m_MemNode.CreatePool( 0, m_iMaxBattleRoomSize, FALSE );
	for(i = 1;i <= m_iMaxBattleRoomSize;i++)
	{
		m_MemNode.Push( new BattleRoomNode( i + iStartIndex ) );
	}

	char szBuf[MAX_PATH] = "";
	char szKey[MAX_PATH] = "";
	ioINILoader kLoader1( "config/sp2_battleroom.ini" );

	kLoader1.SetTitle( "reserve_create" );

	int iMaxName   = kLoader1.LoadInt( "MAX_NAME", 0 );
	for(i = 0;i < iMaxName;i++)
	{
		sprintf_s( szKey, "NAME_%d", i );
		kLoader1.LoadString( szKey, "", szBuf, MAX_PATH );
		m_vNameList.push_back( szBuf );
	}

	m_iBattleDefaultCatchModeLevel = kLoader1.LoadInt( "default_catchmode_level", 10 );

	// Ŀ���� �ɼ� ����(���� ����)
	kLoader1.SetTitle( "ExtraOption" );

	m_iEnableExtraOption = kLoader1.LoadInt( "enable_level", 0 );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][battleroom]Init memory pool : [%d ~ %d]", iStartIndex + 1, iStartIndex + m_iMaxBattleRoomSize );
}

void BattleRoomManager::ReleaseMemoryPool()
{
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pNode = *iter;
		pNode->OnDestroy();
		m_MemNode.Push( pNode );
	}
	m_vBattleRoomNode.clear();
	m_vBattleRoomCopyNode.clear();
	m_MemNode.DestroyPool();
}

BattleRoomNode *BattleRoomManager::CreateNewBattleRoom()
{
	BattleRoomNode* pNewNode = ( BattleRoomNode* )m_MemNode.Remove();
	if( !pNewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"BattleRoomManager::CreateNewBattleRoom MemPool Zero!");
		return NULL;
	}

	m_vBattleRoomNode.push_back( pNewNode );
	pNewNode->OnCreate();

	//�����ϰ� ������ �̸� �߰�.
	if( !m_vNameList.empty() )
	{
		int iArray = rand()%(int)m_vNameList.size();
		pNewNode->SetName( m_vNameList[iArray] );
	}
	return pNewNode;
}

void BattleRoomManager::RemoveBattleRoom( BattleRoomNode *pNode )
{	
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pCursor = *iter;
		if( pCursor == pNode )
		{
			pCursor->OnDestroy();
			m_MemNode.Push( pCursor );
			m_vBattleRoomNode.erase( iter );
			break;
		}
	}
}

BattleRoomNode *BattleRoomManager::GetBattleRoomNode( DWORD dwIndex )
{
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pCursor = *iter;
		if( pCursor->GetIndex() == dwIndex )
		{
			return pCursor;
		}
	}

	return NULL;
}

void BattleRoomManager::AddCopyBattleRoom( BattleRoomCopyNode *pBattleRoom )
{
	vBattleRoomCopyNode_iter iter,iEnd;
	iEnd = m_vBattleRoomCopyNode.end();
	for( iter=m_vBattleRoomCopyNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomCopyNode *pCursor = *iter;
		if( pCursor == pBattleRoom )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomManager::AddCopyBattleRoom �̹� ����Ʈ�� �ִ� ���纻��(%d)", pCursor->GetIndex() );
			return;
		}
	}
	m_vBattleRoomCopyNode.push_back( pBattleRoom );
}

void BattleRoomManager::RemoveBattleCopyRoom( BattleRoomCopyNode *pBattleRoom )
{
	vBattleRoomCopyNode_iter iter,iEnd;
	iEnd = m_vBattleRoomCopyNode.end();
	for( iter=m_vBattleRoomCopyNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomCopyNode *pCursor = *iter;

		if( pCursor == pBattleRoom )
		{
			m_vBattleRoomCopyNode.erase( iter );
			return;
		}
	}
}

void BattleRoomManager::RemoveUserCopyNode( DWORD dwUserIndex, const ioHashString &rkName )
{
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pCursor = *iter;
		if( pCursor->LeaveUser( dwUserIndex, rkName ) )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomManager::RemoveUserCopyNode : Leave User (%d) - %s(%d)", pCursor->GetIndex(), rkName.c_str(), dwUserIndex );
			return;
		}
	}
}

void BattleRoomManager::ConnectServerNodeSync( ServerNode *pServerNode )
{
	if( pServerNode == NULL ) return;

	static vBattleRoomNode vBattleRoom;
	vBattleRoom.clear();

	LOOP_GUARD();
	vBattleRoomNode_iter iter = m_vBattleRoomNode.begin();
	while( iter != m_vBattleRoomNode.end() )
	{
		BattleRoomNode *pNode = *iter++;
		vBattleRoom.push_back( pNode );		
	}
	LOOP_GUARD_CLEAR();

	// �������� ������ ������ N���� ��� ����
	LOOP_GUARD();
	while( true )
	{
		int iMaxSize = min( SSTPK_CONNECT_SYNC_BATTLEROOM_MAX, (int)vBattleRoom.size() );
		if( iMaxSize == 0 )
			break;

		SP2Packet kPacket( SSTPK_CONNECT_SYNC );
		kPacket << SSTPK_CONNECT_SYNC_BATTLEROOM << iMaxSize;
		for(int i = 0;i < iMaxSize;i++)
		{
			BattleRoomNode *pNode  = vBattleRoom[0];
			pNode->FillSyncCreate( kPacket );
			vBattleRoom.erase( vBattleRoom.begin() );
		}
		pServerNode->SendMessage( kPacket );
	}
	LOOP_GUARD_CLEAR();
}

BattleRoomParent* BattleRoomManager::GetGlobalBattleRoomNode( DWORD dwIndex )
{
	BattleRoomParent *pReturn = (BattleRoomParent*)GetBattleRoomNode( dwIndex );
	if( pReturn )
		return pReturn;

	vBattleRoomCopyNode_iter iter,iEnd;
	iEnd = m_vBattleRoomCopyNode.end();
	for( iter=m_vBattleRoomCopyNode.begin() ; iter!=iEnd ; ++iter )
	{
		BattleRoomCopyNode *pCursor = *iter;
		if( pCursor->GetIndex() == dwIndex )
		{
			return pCursor;
		}
	}
	return NULL;
}

void BattleRoomManager::InitTotalModeList()
{
	m_vTotalRandomModeList.clear();

	ioINILoader kLoader( "config/sp2_mode_select.ini" );

	int i, j;
	char szKey[MAX_PATH] = "";
	char szBuf[MAX_PATH] = "";
	kLoader.SetTitle( "ModeTypeInfo" );
	int iModeCnt = kLoader.LoadInt( "mode_cnt", 0 );
	for( i = 0; i < iModeCnt; ++i )
	{
		sprintf_s( szKey, "mode%d", i+1 );
		kLoader.SetTitle( szKey );

		ioHashString szModeTitle;
		kLoader.LoadString( "mode_title", "", szBuf, MAX_PATH );
		szModeTitle = szBuf;

		int iModeType = kLoader.LoadInt( "mode_type", 0 );

		BattleRandomMode kRandomMode;
		kRandomMode.m_Title = szModeTitle;
		kRandomMode.m_ModeType = (ModeType)iModeType;
		kRandomMode.m_bSafetyMode = kLoader.LoadBool( "mode_safety", false );
		kRandomMode.m_bBroadcastMode = kLoader.LoadBool( "mode_broadcast", false );

		int iSubCnt = kLoader.LoadInt( "sub_cnt", 0 );
		for( j = 0; j < iSubCnt; ++j )
		{	
			BattleMapInfo kPartyMapInfo;

			sprintf_s( szKey, "sub%d_active", j+1 );
			kPartyMapInfo.m_bActive = kLoader.LoadBool( szKey, true );

			sprintf_s( szKey, "sub%d_title", j+1 );
			kLoader.LoadString( szKey, "", szBuf, MAX_PATH );
			kPartyMapInfo.m_Title = szBuf;

			sprintf_s( szKey, "sub%d_type", j+1 );
			kPartyMapInfo.m_iSubType = kLoader.LoadInt( szKey, 0 );

			sprintf_s( szKey, "sub%d_limit_player", j+1 );
			kPartyMapInfo.m_iLimitPlayer = kLoader.LoadInt( szKey, MAX_PLAYER );
            
			sprintf_s( szKey, "sub%d_limit_grade", j+1 );
			kPartyMapInfo.m_iLimitGrade = kLoader.LoadInt( szKey, 0 );

			kRandomMode.m_SubList.push_back( kPartyMapInfo );
		}

		// inactive sub mode ó��, ���� ������ ������ array�� �����ϱ� ���ؼ�
		int iSize = kRandomMode.m_SubList.size();
		IntVec vActiveArray;
		vActiveArray.reserve( iSize );
		for (int i = 0; i < iSize ; i++)
		{
			BattleMapInfo &rCurInfo = kRandomMode.m_SubList[i];
			if( rCurInfo.m_bActive )
				vActiveArray.push_back( i );
		}
		int iActiveArraySize = vActiveArray.size();
		int iActiveCnt = 0;
		for (int i = 0; i < iSize ; i++)
		{
			BattleMapInfo &rInActivekInfo = kRandomMode.m_SubList[i];
			if( rInActivekInfo.m_bActive )
				continue;
			if( iActiveArraySize <= 0 )
				break;
			int iArray = vActiveArray[iActiveCnt];
			if( !COMPARE( iArray, 0, iSize ) )
				break;
			BattleMapInfo &rActivekInfo = kRandomMode.m_SubList[iArray];
			rInActivekInfo = rActivekInfo;
			rInActivekInfo.m_bActive = false;
			iActiveCnt++;
			if( iActiveCnt >= iActiveArraySize )
				iActiveCnt = 0;
		}
		vActiveArray.clear();
		//

		m_vTotalRandomModeList.push_back( kRandomMode );
	}
}

bool BattleRoomManager::CheckBattleRandomMode( int iModeIndex, int iMapIndex, ModeType &eModeType, int &iSubType )
{
	// ���
	int iModeCnt = m_vTotalRandomModeList.size();
	if( COMPARE( iModeIndex - 1, 0, iModeCnt ) )
	{
		eModeType = m_vTotalRandomModeList[iModeIndex - 1].m_ModeType;
	}
	else
        return false;
	
	// ��
	int iSubCnt = m_vTotalRandomModeList[iModeIndex - 1].m_SubList.size();
	if( COMPARE( iMapIndex - 1, 0, iSubCnt ) )
	{
		iSubType = m_vTotalRandomModeList[iModeIndex - 1].m_SubList[iMapIndex - 1].m_iSubType;		
	}	
	return true;
}

int BattleRoomManager::GetBattleModeToIndex( ModeType eModeType, bool bSafetyMode, bool bBroadcastMode )
{
	int iRandomCnt = m_vTotalRandomModeList.size();
	for( int i=0; i < iRandomCnt; ++i )
	{
		if( m_vTotalRandomModeList[i].m_ModeType == eModeType &&
			m_vTotalRandomModeList[i].m_bSafetyMode == bSafetyMode &&
			m_vTotalRandomModeList[i].m_bBroadcastMode == bBroadcastMode )
		{
			return i + 1;
		}
	}
	return -1;
}

int BattleRoomManager::GetBattleMapToLimitPlayer( int iModeIndex, int iMapIndex )
{
	int iReturnValue = MAX_PLAYER;
	// ��� & ��
	int iModeCnt = m_vTotalRandomModeList.size();
	if( COMPARE( iModeIndex - 1, 0, iModeCnt ) )
	{
        int iSubCnt = m_vTotalRandomModeList[iModeIndex - 1].m_SubList.size();
		if( COMPARE( iMapIndex - 1, 0, iSubCnt ) )
		{
			iReturnValue = m_vTotalRandomModeList[iModeIndex - 1].m_SubList[iMapIndex - 1].m_iLimitPlayer;		
		}	
	}
	return iReturnValue;
}

int BattleRoomManager::GetBattleMapToLimitGrade( int iModeIndex, int iMapIndex )
{
	int iReturnValue = 0;
	// ��� & ��
	int iModeCnt = m_vTotalRandomModeList.size();
	if( COMPARE( iModeIndex - 1, 0, iModeCnt ) )
	{
		int iSubCnt = m_vTotalRandomModeList[iModeIndex - 1].m_SubList.size();
		if( COMPARE( iMapIndex - 1, 0, iSubCnt ) )
		{
			iReturnValue = m_vTotalRandomModeList[iModeIndex - 1].m_SubList[iMapIndex - 1].m_iLimitGrade;		
		}	
	}
	return iReturnValue;
}

int BattleRoomManager::GetBattleRoomUserCount()
{
	int iCount = 0;
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pNode = *iter;
		iCount += pNode->GetJoinUserCnt();
	}
	return iCount;
}

int BattleRoomManager::GetBattleRoomPlayUserCount()
{
	int iCount = 0;
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pNode = *iter;
		if( pNode->IsBattleModePlaying() )
			iCount += pNode->GetJoinUserCnt();
	}
	return iCount;
}

void BattleRoomManager::CreateMatchingTable()
{
	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	g_LevelMatchMgr.InitPartyLevelMatch();

	// ���� ������ ������ �������� ����
	if( GetNodeSize() + GetCopyNodeSize() < g_LevelMatchMgr.GetPartyLevelCheckMinRoom() )
		return;

	static vSortBattleRoom vBattleRoomList;
	vBattleRoomList.clear();

	// ����
	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pNode = *iter;
		if( pNode == NULL ) continue;
		if( pNode->IsLevelMatchIgnore() ) continue;

		SortBattleRoom sbr;
		sbr.m_pNode = (BattleRoomParent*)pNode;
		sbr.m_iPoint= pNode->GetAbilityMatchLevel();
		vBattleRoomList.push_back( sbr );
	}
	// ���纻
	vBattleRoomCopyNode_iter iterCopy, iEndCopy;
	iEndCopy = m_vBattleRoomCopyNode.end();
	for(iterCopy = m_vBattleRoomCopyNode.begin();iterCopy != iEndCopy;++iterCopy)
	{
		BattleRoomCopyNode *pNode = *iterCopy;
		if( pNode == NULL ) continue;
		if( pNode->IsLevelMatchIgnore() ) continue;

		SortBattleRoom sbr;
		sbr.m_pNode = (BattleRoomParent*)pNode;
		sbr.m_iPoint= pNode->GetAbilityMatchLevel();
		vBattleRoomList.push_back( sbr );
	}

	int iMaxList = vBattleRoomList.size();
	if( iMaxList == 0 ) return;

	std::sort( vBattleRoomList.begin(), vBattleRoomList.end(), BattleRoomSort() );

	int i = 0;
	int iLimitCount = max( 1, (float)iMaxList * g_LevelMatchMgr.GetPartyEnterLimitCount() );
	for(i = 0;i < g_LevelMatchMgr.GetRoomEnterLevelMax();i++)
	{		
		int rIndex = 0;
		for(;rIndex < iMaxList;rIndex++)
		{
			SortBattleRoom &kNode = vBattleRoomList[rIndex];
			if( kNode.m_iPoint >= i )
				break;
		}

		int iMaxRemain = max( 0, ( rIndex + iLimitCount ) - iMaxList );
		int iMinRemain = max( 0, iLimitCount - rIndex );
		int iLowIndex  = max( rIndex - iLimitCount - iMaxRemain, 0 );
		int iHighIndex = min( iLimitCount + iMinRemain + rIndex, iMaxList );

		//�ڽ������� ������ ����� �Ѵ� �ε����� Ȥ�� ������ �������....-_-;
		if( iLowIndex < 0 )
			iLowIndex = 0;
		else if( iLowIndex >= iMaxList )
			iLowIndex = iMaxList - 1;

		if( iHighIndex < 0 )
			iHighIndex = 0;
		else if( iHighIndex > iMaxList )
			iHighIndex = iMaxList;

		SortBattleRoom &kLowNode = vBattleRoomList[iLowIndex];
		SortBattleRoom &kHighNode = vBattleRoomList[iHighIndex - 1];
		g_LevelMatchMgr.InsertPartyLevelMatch( min( i, kLowNode.m_iPoint ), max( i, kHighNode.m_iPoint ) );
	}	
}

bool BattleRoomManager::IsBattleRoomSameModeTerm( int iModeTermA, int iModeTermB )
{
	switch( iModeTermA )
	{
	case BMT_ALL_MODE:
		if( iModeTermB == BMT_ALL_MODE )
			return true;
		break;
	case BMT_RANDOM:
	case BMT_RANDOM_BOSS:
		if( iModeTermB == BMT_RANDOM || iModeTermB == BMT_RANDOM_BOSS )
			return true;
		break;
	case BMT_CATCH:
	case BMT_CATCH_BOSS:
		if( iModeTermB == BMT_CATCH || iModeTermB == BMT_CATCH_BOSS )
			return true;
		break;
	case BMT_CATCH_RUNNINGMAN:
	case BMT_CATCH_RUNNINGMAN_BOSS:
		if( iModeTermB == BMT_CATCH_RUNNINGMAN || iModeTermB == BMT_CATCH_RUNNINGMAN_BOSS )
			return true;
		break;
	case BMT_USER_CUSTOM:
		if( iModeTermB == BMT_USER_CUSTOM )
			return true;
		break;
	case BMT_STONE:
	case BMT_STONE_BOSS:
		if( iModeTermB == BMT_STONE || iModeTermB == BMT_STONE_BOSS )
			return true;
		break;
	case BMT_KING:
	case BMT_KING_BOSS:
		if( iModeTermB == BMT_KING || iModeTermB == BMT_KING_BOSS )
			return true;
		break;
	case BMT_SURVIVAL:
		if( iModeTermB == BMT_SURVIVAL )
			return true;
		break;
	case BMT_FIGHT_CLUB:
		if( iModeTermB == BMT_FIGHT_CLUB )
			return true;
		break;
	case BMT_UNDERWEAR:
		if( iModeTermB == BMT_UNDERWEAR )
			return true;
		break;
	case BMT_CBT:
		if( iModeTermB == BMT_CBT )
			return true;
		break;
	case BMT_TEAM_SURVIVAL:
	case BMT_TEAM_SURVIVAL_BOSS:
		if( iModeTermB == BMT_TEAM_SURVIVAL || iModeTermB == BMT_TEAM_SURVIVAL_BOSS )
			return true;
		break;
	case BMT_TEAM_SURVIVAL_FIRST:
	case BMT_TEAM_SURVIVAL_FIRST_BOSS:
		if( iModeTermB == BMT_TEAM_SURVIVAL_FIRST || iModeTermB == BMT_TEAM_SURVIVAL_FIRST_BOSS )
			return true;
		break;
	case BMT_BOSS:
		if( iModeTermB == BMT_BOSS )
			return true;
		break;
	case BMT_MONSTER_SURVIVAL_EASY:
		if( iModeTermB == BMT_MONSTER_SURVIVAL_EASY )
			return true;
		break;
	case BMT_MONSTER_SURVIVAL_NORMAL:
		if( iModeTermB == BMT_MONSTER_SURVIVAL_NORMAL )
			return true;
		break;
	case BMT_MONSTER_SURVIVAL_HARD:
		if( iModeTermB == BMT_MONSTER_SURVIVAL_HARD )
			return true;
		break;
	case BMT_FOOTBALL:
	case BMT_FOOTBALL_BOSS:
		if( iModeTermB == BMT_FOOTBALL || iModeTermB == BMT_FOOTBALL_BOSS )
			return true;
		break;
	case BMT_GANGSI:
		if( iModeTermB == BMT_GANGSI )
			return true;
		break;
	case BMT_DUNGEON_A_EASY:
		if( iModeTermB == BMT_DUNGEON_A_EASY )
			return true;
		break;

	case BMT_TOWER_DEFENSE_EASY:
	case BMT_TOWER_DEFENSE_NORMAL:
	case BMT_TOWER_DEFENSE_HARD:
	case BMT_TOWER_DEFENSE_CHALLENGE:
	case BMT_DARK_XMAS_EASY:
	case BMT_DARK_XMAS_NORMAL:
	case BMT_DARK_XMAS_HARD:
	case BMT_DARK_XMAS_CHALLENGE:
	case BMT_FIRE_TEMPLE_EASY:
	case BMT_FIRE_TEMPLE_NORMAL:
	case BMT_FIRE_TEMPLE_HARD:
	case BMT_FIRE_TEMPLE_CHALLENGE:
	case BMT_FACTORY_EASY:    
	case BMT_FACTORY_NORMAL:
	case BMT_FACTORY_HARD:
	case BMT_FACTORY_CHALLENGE:
	case BMT_TEAM_SURVIVAL_AI_EASY:
	case BMT_TEAM_SURVIVAL_AI_HARD:
	case BMT_RAID:
		if( iModeTermB == iModeTermA)
			return true;
		break;
	case BMT_DOBULE_CROWN:
	case BMT_DOBULE_CROWN_BOSS:
		if( iModeTermB == BMT_DOBULE_CROWN || iModeTermB == BMT_DOBULE_CROWN_BOSS )
			return true;
		break;
	}
	return false;
}

int BattleRoomManager::GetSortBattleRoomPoint( SortBattleRoom &rkSortRoom, int iKillDeathLevel, int iSelectTerm, int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer )
{
	if( rkSortRoom.m_pNode == NULL )
		return BATTLE_ROOM_SORT_HALF_POINT;

	int iReturnPoint = 0;

/*	// �̺�Ʈ �������� ������ ��ġ�Ѵ�.
	if( rkSortRoom.m_pNode->GetBattleEventType() == BET_BROADCAST_AFRICA ||
		rkSortRoom.m_pNode->GetBattleEventType() == BET_BROADCAST_MBC )
		return iReturnPoint;
*/
	// ���� �Ұ����� ��
	if( rkSortRoom.m_iState != SortBattleRoom::BRS_ACTIVE )         
		iReturnPoint = BATTLE_ROOM_SORT_HALF_POINT;

	// ��庰 ����
	if( !IsBattleRoomSameModeTerm( rkSortRoom.m_pNode->GetSelectModeTerm(), iSelectTerm ) )
		iReturnPoint += 50000000;

	// ����� ����
	if( rkSortRoom.m_pNode->IsPassword() )
	{
		iReturnPoint += 1000000;
	}

	// ���� ���� �ο� / �ٸ� �ο� üũ
	if( bSameTeamPlayer )
	{
		// ������ �ο��� �ٸ��� ���Ŀ��� �Ʒ��� ������.
		if( rkSortRoom.m_pNode->GetMaxPlayerBlue() != rkSortRoom.m_pNode->GetMaxPlayerRed() )
			iReturnPoint += 1000000;
	}

	// �ο��̳��� ���� �ƴϸ� �Ʒ���
	if( !COMPARE( rkSortRoom.m_pNode->GetMaxPlayer(), iMinPlayer, iMaxPlayer+1 ) )
		iReturnPoint += 1000000;
		
	// �ο��� ���� ����
	// �ּ��ο� + ( �ִ��ο� - �ּ��ο� / 2 ) - �� �ִ��ο� = ���� ���� �� �켱 ����
	iReturnPoint += abs( ( iMinPlayer + ( abs( iMaxPlayer - iMinPlayer ) / 2 ) ) - rkSortRoom.m_pNode->GetPlayUserCnt() ) * 10000;

	// �Ƿ� ���� 
	iReturnPoint += rkSortRoom.m_pNode->GetSortLevelPoint( iKillDeathLevel );

	return iReturnPoint;
}

int BattleRoomManager::GetSortBattleRoomState( BattleRoomParent *pBattleParent, UserParent *pUserParent, DWORD dwPrevBattleIndex )
{
	if( !pBattleParent || !pUserParent )
		return SortBattleRoom::BRS_TIME_CLOSE;

	int iReturnState = SortBattleRoom::BRS_ACTIVE;

	if( pBattleParent->IsNoChallenger() )
		iReturnState = SortBattleRoom::BRS_NO_CHALLENGER;
	else if( pBattleParent->IsStartRoomEnterX() )
		iReturnState = SortBattleRoom::BRS_ENTER_X;
	else if( pBattleParent->IsBattleTimeClose() || pBattleParent->IsEmptyBattleRoom() )
		iReturnState = SortBattleRoom::BRS_TIME_CLOSE;
	else if( pBattleParent->IsFull() )
		iReturnState = SortBattleRoom::BRS_FULL_USER;
	else if( pBattleParent->GetIndex() == dwPrevBattleIndex )
		iReturnState = SortBattleRoom::BRS_ALREADY_ROOM;
	else if( pBattleParent->IsMapLimitGrade( pUserParent->GetGradeLevel() ) )
		iReturnState = SortBattleRoom::BRS_MAP_LIMIT_GRADE;
	else if( pBattleParent->IsUseExtraOption() && m_iEnableExtraOption > pUserParent->GetGradeLevel() )
	{
		if(pBattleParent->GetSelectModeTerm() != BMT_UNDERWEAR && pBattleParent->GetSelectModeTerm() != BMT_CBT)
			iReturnState = SortBattleRoom::BRS_OPTION_LIMIT_GRADE;
	}
	else if( !pBattleParent->IsLevelMatchIgnore() && !g_LevelMatchMgr.IsPartyLevelJoin( pBattleParent->GetAbilityMatchLevel(), pUserParent->GetKillDeathLevel(), JOIN_CHECK_MIN_LEVEL ) )
		iReturnState = SortBattleRoom::BRS_NOT_MIN_LEVEL_MATCH;
	else if( !pBattleParent->IsLevelMatchIgnore() && !g_LevelMatchMgr.IsPartyLevelJoin( pBattleParent->GetAbilityMatchLevel(), pUserParent->GetKillDeathLevel(), JOIN_CHECK_MAX_LEVEL ) )
		iReturnState = SortBattleRoom::BRS_NOT_MAX_LEVEL_MATCH;
	else if( pBattleParent->GetSelectModeTerm() == BMT_TEAM_SURVIVAL_FIRST && !pUserParent->IsSafetyLevel() )
		iReturnState = SortBattleRoom::BRS_NOT_MIN_LEVEL_MATCH;

	return iReturnState;
}

BattleRoomParent *BattleRoomManager::GetJoinBattleRoomNode( int iPrevLeaveBattleRoom, bool bSafetyLevel, int iGradeLevel, int iAverageLevel, int iSearchTerm, int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer )
{
	vBattleRoomParent vBattleRoomList;

	if(!IsBlocked())
	{
		vBattleRoomNode_iter iter, iEnd;
		iEnd = m_vBattleRoomNode.end();
		for( iter=m_vBattleRoomNode.begin() ; iter!=iEnd ; ++iter )
		{
			BattleRoomNode *pNode = *iter;
			vBattleRoomList.push_back( (BattleRoomParent*)pNode );
		}
	}

	vBattleRoomCopyNode_iter iterCopy, iEndCopy;
	iEndCopy = m_vBattleRoomCopyNode.end();
	for( iterCopy=m_vBattleRoomCopyNode.begin() ; iterCopy!=iEndCopy ; ++iterCopy )
	{
		BattleRoomCopyNode *pNode = *iterCopy;
		if(!pNode->IsBlocked())
		{
			vBattleRoomList.push_back( (BattleRoomParent*)pNode );
		}
	}

	static vSortBattleRoom vSortList;
	vSortList.clear();

	vBattleRoomParent_iter iterParent, iEndParent;
	iEndParent = vBattleRoomList.end();
	for(iterParent = vBattleRoomList.begin();iterParent != iEndParent;++iterParent)
	{
		BattleRoomParent *pNode = *iterParent;
		if( pNode == NULL ) continue;
		if( pNode->GetIndex() == iPrevLeaveBattleRoom ) continue;
		if( pNode->IsPassword() ) continue;
		if( pNode->IsLevelMatchIgnore() ) continue;
		if( !g_LevelMatchMgr.IsPartyLevelJoin( pNode->GetAbilityMatchLevel(), iAverageLevel, JOIN_CHECK_MINMAX_LEVEL ) ) continue;
		if( pNode->IsFull() ) continue;
		if( pNode->IsMapLimitPlayerFull() ) continue;
		if( pNode->IsMapLimitGrade( iGradeLevel ) ) continue;
		if( pNode->IsEmptyBattleRoom() ) continue;
		if( pNode->IsBattleTimeClose() ) continue;
		if( pNode->IsStartRoomEnterX() ) continue;
		if( pNode->IsUseExtraOption() ) continue;
		if( pNode->IsNoChallenger() ) continue;

		if( pNode->GetSelectModeTerm() != BMT_SURVIVAL && pNode->GetSelectModeTerm() != BMT_BOSS && pNode->GetSelectModeTerm() != BMT_GANGSI && pNode->GetSelectModeTerm() != BMT_FIGHT_CLUB && !pNode->IsRandomTeamMode() ) continue;         //������ �ɼ� ���õ� �븸 ���
		if( iSearchTerm != BMT_ALL_MODE && pNode->GetSelectModeTerm() != iSearchTerm ) continue;
		if( pNode->GetSelectModeTerm() == BMT_TEAM_SURVIVAL_FIRST || pNode->GetSelectModeTerm() == BMT_TEAM_SURVIVAL_FIRST_BOSS )
		{
			// �ʺ����ε� �ڽ��� �ʺ������� �ƴϸ� �н�
			if( !bSafetyLevel )
				continue;
		}

		// ���� ���� �ο� üũ
		if( bSameTeamPlayer )
		{
			if( pNode->GetMaxPlayerBlue() != pNode->GetMaxPlayerRed() )
				continue;
		}

		// �ּ� ~ �ִ� �ο��� ���Ե��� �ʴ� ���� ����
		if( !COMPARE( pNode->GetMaxPlayer(), iMinPlayer, iMaxPlayer + 1 ) )
			continue;

		SortBattleRoom sbr;
		sbr.m_pNode   = pNode;
		sbr.m_iPoint  = GetSortBattleRoomPoint( sbr, iAverageLevel, iSearchTerm, iMinPlayer, iMaxPlayer, bSameTeamPlayer );
		vSortList.push_back( sbr );
	}

	int iMaxList = vSortList.size();
	if( iMaxList == 0 ) return NULL;
	
	std::sort( vSortList.begin(), vSortList.end(), BattleRoomSort() );

	SortBattleRoom &kNode = vSortList[0];
	BattleRoomParent *pReturn = kNode.m_pNode;
	vBattleRoomList.clear();
	return pReturn;	
}

void BattleRoomManager::SendCurBattleRoomList( User *pUser, int iPage, int iMaxCount, int iSelectTerm, int iPrevLeaveBattleRoom,
											   int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer )
{
	if( pUser == NULL ) return;
	
	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	int  iMyLevel    = pUser->GetKillDeathLevel();
	bool bSafetyLevel= pUser->IsSafetyLevel();
	
	static vSortBattleRoom vBattleRoomList;
	vBattleRoomList.clear();

	if(!IsBlocked())
	{
		vBattleRoomNode_iter iter, iEnd;
		iEnd = m_vBattleRoomNode.end();
		for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
		{
			BattleRoomNode *pNode = *iter;
			if( pNode == NULL ) continue;
			if( pNode->IsHidden(pUser->GetMyBattleRoomIndex()) ) continue;
		
			SortBattleRoom sbr;
			sbr.m_pNode  = (BattleRoomParent*)pNode;
			sbr.m_iState = GetSortBattleRoomState( (BattleRoomParent*)pNode, (UserParent*)pUser, iPrevLeaveBattleRoom );		
			sbr.m_iPoint = GetSortBattleRoomPoint( sbr, iMyLevel, iSelectTerm, iMinPlayer, iMaxPlayer, bSameTeamPlayer );
			vBattleRoomList.push_back( sbr );
		}
	}

	vBattleRoomCopyNode_iter iterCopy, iEndCopy;
	iEndCopy = m_vBattleRoomCopyNode.end();
	for( iterCopy=m_vBattleRoomCopyNode.begin() ; iterCopy!=iEndCopy ; ++iterCopy )
	{
		BattleRoomCopyNode *pNode = *iterCopy;
		if( pNode == NULL ) continue;
		if( pNode->IsHidden(pUser->GetMyBattleRoomIndex()) ) continue;

		SortBattleRoom sbr;
		sbr.m_pNode  = (BattleRoomParent*)pNode;
		sbr.m_iState = GetSortBattleRoomState( (BattleRoomParent*)pNode, (UserParent*)pUser, iPrevLeaveBattleRoom );
		sbr.m_iPoint = GetSortBattleRoomPoint( sbr, iMyLevel, iSelectTerm, iMinPlayer, iMaxPlayer, bSameTeamPlayer );
		vBattleRoomList.push_back( sbr );
	}

	int iMaxList = vBattleRoomList.size();
	if( iMaxList == 0 )
	{
		SP2Packet kPacket( STPK_BATTLEROOM_LIST );
		kPacket << 0 << 0;
		pUser->SendMessage( kPacket );
		return;
	}
	std::sort( vBattleRoomList.begin(), vBattleRoomList.end(), BattleRoomSort() );

	int iStartPos = iPage * iMaxCount;
	int iCurSize  = max( 0, min( iMaxList - iStartPos, iMaxCount ) );

	SP2Packet kPacket( STPK_BATTLEROOM_LIST );
	kPacket << iMaxList << iCurSize;
	for(int i = iStartPos; i < iStartPos + iCurSize;i++)
	{
		SortBattleRoom &kSortNode	= vBattleRoomList[i];
		BattleRoomParent *pBattleNode = kSortNode.m_pNode;
		if( pBattleNode )
		{
			kPacket << pBattleNode->GetIndex() << pBattleNode->GetRoomLevel() << pBattleNode->GetName()
					<< pBattleNode->GetJoinUserCnt() << pBattleNode->GetPlayUserCnt()
					<< pBattleNode->GetMaxPlayerBlue() << pBattleNode->GetMaxPlayerRed() << pBattleNode->GetMaxObserver()
					<< pBattleNode->GetSelectModeTerm() << pBattleNode->IsPassword() << kSortNode.m_iState
					<< pBattleNode->IsUseExtraOption() << pBattleNode->IsNoChallenger();

					/*
					<< pBattleNode->IsUseExtraOption() << pBattleNode->GetChangeCharType()
					<< pBattleNode->GetCoolTimeType() << pBattleNode->GetRedHPType() << pBattleNode->GetBlueHPType()
					<< pBattleNode->GetDropDamageType() << pBattleNode->GetGravityType()
					<< pBattleNode->GetPreSetModeType() << pBattleNode->GetTeamAttackType() << pBattleNode->GetGetUpType()
					<< pBattleNode->GetKOType() << pBattleNode->GetRedBlowType() << pBattleNode->GetBlueBlowType()
					<< pBattleNode->GetRedMoveSpeedType() << pBattleNode->GetBlueMoveSpeedType() << pBattleNode->GetKOEffectType()
					<< pBattleNode->GetRedEquipType() << pBattleNode->GetBlueEquipType();
					*/
		}		
		else    //����
		{
			kPacket << 0 << 0 << ""
					<< 0 << 0
					<< 0 << 0 << 0
					<< 0 << false << SortBattleRoom::BRS_TIME_CLOSE
					<< false;

					/*
					<< false << 0
					<< 0 << 0 << 0
					<< 0 << 0
					<< 0 << 0 << 0
					<< 0 << 0 << 0
					<< 0 << 0 << 0
					<< 0 << 0;
					*/
		}
	}
	pUser->SendMessage( kPacket );
	vBattleRoomList.clear();
}

void BattleRoomManager::SendBattleRoomJoinInfo( UserParent *pUserParent, DWORD dwIndex, int iPrevBattleIndex )
{
	if( !pUserParent )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomManager::SendBattleRoomJoinInfo Null User Pointer!!(%d)", dwIndex );
		return;
	}

	BattleRoomParent *pNode = GetGlobalBattleRoomNode( dwIndex );
	if( pNode )
	{
		pNode->OnBattleRoomInfo( pUserParent, iPrevBattleIndex );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomManager::SendBattleRoomJoinInfo Null Battle Pointer!!(%s:%d)", pUserParent->GetPublicID().c_str(), dwIndex );
	}
}

void BattleRoomManager::UpdateProcess()
{
	if( TIMEGETTIME() - m_dwCurTime < 1000 ) return;
	m_dwCurTime = TIMEGETTIME();


	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	static vBattleRoomNode vDestroyNode;
	vDestroyNode.clear();

	vBattleRoomNode_iter iter, iEnd;
	iEnd = m_vBattleRoomNode.end();
	for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
	{
		BattleRoomNode *pNode = *iter;
		if( pNode->IsReserveTimeOver() )
			vDestroyNode.push_back( pNode );
		else
			pNode->Process();
	}

	if( !vDestroyNode.empty() )
	{
		iEnd = vDestroyNode.end();
		for(iter = vDestroyNode.begin();iter != iEnd;++iter)
		{
			RemoveBattleRoom( *iter );
		}
		vDestroyNode.clear();
	}
}

bool BattleRoomManager::CheckEnableExtraOptionLevel( int iLevel )
{
	if( m_iEnableExtraOption > iLevel )
		return false;

	return true;
}

/***
�뼭Ī ����� ���� �������� �˻��Ͽ� ����� �����մϴ�.
m_vBattleRoomNode ���� ������ ������ ����
m_vBattleRoomCopyNode �ٸ� ������ ������ ����
***/
bool BattleRoomManager::SearchRoomIndex( User *pUser, int iRoomIndex )
{
	if( pUser == NULL ) return false;

	FUNCTION_TIME_CHECKER( 100000.0f, 0 );          // 0.1 �� �̻� �ɸ���α� ����

	if(!IsBlocked())
	{
		vBattleRoomNode_iter iter, iEnd;
		iEnd = m_vBattleRoomNode.end();
		for(iter = m_vBattleRoomNode.begin();iter != iEnd;++iter)
		{
			BattleRoomNode *pNode = *iter;
			if( pNode == NULL ) continue;
			//if( pNode->IsHidden(pUser->GetMyBattleRoomIndex()) ) continue;
		
			int iRoomNum = pNode->GetIndex();
			if( iRoomIndex == pNode->GetIndex() )
				return true;
		}
	}

	vBattleRoomCopyNode_iter iterCopy, iEndCopy;
	iEndCopy = m_vBattleRoomCopyNode.end();
	for( iterCopy=m_vBattleRoomCopyNode.begin() ; iterCopy!=iEndCopy ; ++iterCopy )
	{
		BattleRoomCopyNode *pNode = *iterCopy;
		if( pNode == NULL ) continue;
		//if( pNode->IsHidden(pUser->GetMyBattleRoomIndex()) ) continue;

		int iRoomNum = pNode->GetIndex();
		if( iRoomIndex == pNode->GetIndex() )
			return true;
	}

	SP2Packet kPacket( STPK_SEARCH_ROOM_RESLUT );
	kPacket << RS_SEARCH_FAIL;
	pUser->SendMessage( kPacket );

	//�������� ���� ��� false �� ��Ŷ ����
	LOG.PrintTimeAndLog( 0, "%s - %d�� ���� ������ ã�� �� �����ϴ�. | userIdx: %d, userId: %s", __FUNCTION__, iRoomIndex, pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
	return false;
}

