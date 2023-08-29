#include "stdafx.h"

//#include "../Window.h"
#include "../MainProcess.h"

#include "../Network/GameServer.h"
#include "../Network/ioPacketQueue.h"
#include "../DataBase/DBClient.h"
#include "../MainServerNode/MainServerNode.h"
#include "../EtcHelpFunc.h"
#include "../NodeInfo/ioPresentHelper.h"
#include "../NodeInfo/TradeInfoManager.h"
#include "../NodeInfo/GuildRoomsBlockManager.h"
#include "../NodeInfo/TradeSyncManger.h"

#include "ioEtcItemManager.h"
#include "Room.h"
#include "RoomParent.h"
#include "ChannelNode.h"
#include "ServerNodeManager.h"
#include "RoomNodeManager.h"
#include "BattleRoomManager.h"
#include "LadderTeamManager.h"
#include "ShuffleRoomManager.h"
#include "ChannelNodeManager.h"
#include "MemoNodeManager.h"
#include "HeroRankManager.h"
#include "ShuffleRoomReserveMgr.h"

#include "ServerNode.h"

#include "ioSaleManager.h"
#include "../Local/ioLocalParent.h"
#include "../Local/ioLocalManager.h"
#include "../BillingRelayServer/BillingRelayServer.h"

#include "../QueryData/QueryResultData.h"

#include <strsafe.h>

extern CLog EventLOG;
extern CLog TradeLOG;

ServerNode::ServerNode( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize ) : CConnectNode( s, dwSendBufSize, dwRecvBufSize ), m_bBlockState(false), m_iPartitionIndex(0)
{
	InitData();
	InitUserMemoryPool();
	InitRoomMemoryPool();
	InitBattleRoomMemoryPool();
	InitChannelMemoryPool();
	InitLadderTeamMemoryPool();
	InitShuffleRoomMemoryPool();
	m_eSessionState = SS_DISCONNECT;
}

ServerNode::~ServerNode()
{	
	ReleaseUserMemoryPool();
	ReleaseRoomMemoryPool();
	ReleaseBattleRoomMemoryPool();
	ReleaseChannelMemoryPool();
	ReleaseLadderTeamMemoryPool();
	ReleaseShuffleRoomMemoryPool();
}

void ServerNode::InitUserMemoryPool()
{
	// Get MaxConnection
	int	MaxConnection	= g_UserNodeManager.GetMaxConnection();

	// MemPooler
	m_UserMemNode.CreatePool( 0, MaxConnection, TRUE );
	for( int i = 0 ; i < MaxConnection ; i++ )
		m_UserMemNode.Push( new UserCopyNode );
}

void ServerNode::ReleaseUserMemoryPool()
{
	ReturnUserMemoryPool();
	m_UserMemNode.DestroyPool();
}

void ServerNode::ReturnUserMemoryPool()
{
	uUserCopyNode_iter iter, iEnd;
	iEnd = m_UserCopyNode.end();
	for(iter = m_UserCopyNode.begin();iter != iEnd; ++iter)
	{
		UserCopyNode *pUserNode = iter->second;
		if( pUserNode )
		{
			//������ / ������ ����
			g_BattleRoomManager.RemoveUserCopyNode( pUserNode->GetUserIndex(), pUserNode->GetPublicID() );
			g_LadderTeamManager.RemoveUserCopyNode( pUserNode->GetUserIndex(), pUserNode->GetPublicID() );
			g_ShuffleRoomManager.RemoveUserCopyNode( pUserNode->GetUserIndex(), pUserNode->GetPublicID() );
		}

		g_UserNodeManager.RemoveCopyUser( pUserNode->GetUserIndex() );

		pUserNode->OnDestroy();
		m_UserMemNode.Push( pUserNode );
	}	
	m_UserCopyNode.clear();
}

UserCopyNode *ServerNode::CreateNewUser( DWORD dwIndex )
{
	UserCopyNode *NewNode = GetUserNode( dwIndex );
	if( NewNode )
		return NewNode;

	//NewNode = (UserCopyNode*)m_UserMemNode.OutPoolAddr();
	NewNode = (UserCopyNode*)m_UserMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewUser MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetUserIndex( dwIndex );
	m_UserCopyNode.insert( make_pair(dwIndex, NewNode) );
	return NewNode;
}

UserCopyNode *ServerNode::GetUserNode( DWORD dwIndex )
{
	uUserCopyNode_iter it = m_UserCopyNode.find( dwIndex );
	if(it != m_UserCopyNode.end())
	{
		return it->second;
	}
	return NULL;
}

//UserCopyNode *ServerNode::GetUserNode( const ioHashString &szUserID )
//{
//	uUserCopyNode_iter iter, iEnd;
//	iEnd = m_UserCopyNode.end();
//	for(iter = m_UserCopyNode.begin();iter != iEnd;++iter)
//	{
//		UserCopyNode *pUserNode = *iter;
//		if( pUserNode->GetPublicID() == szUserID )
//			return pUserNode;
//	}	
//	return NULL;
//}

void ServerNode::RemoveUserNode( DWORD dwIndex )
{
	uUserCopyNode_iter it = m_UserCopyNode.find( dwIndex );
	if(it != m_UserCopyNode.end())
	{
		UserCopyNode *pUserNode = it->second;
		if(!pUserNode)
		{
			m_UserCopyNode.erase( it );
			return;
		}

		g_UserNodeManager.RemoveCopyUser( pUserNode->GetUserIndex() );
		m_UserCopyNode.erase( it );
		pUserNode->OnDestroy();
		m_UserMemNode.Push( pUserNode );
	}
}

void ServerNode::RemoveUserNode( const ioHashString &szUserID )
{
	uUserCopyNode_iter iter, iEnd;
	iEnd = m_UserCopyNode.end();
	for(iter = m_UserCopyNode.begin();iter != iEnd;++iter)
	{
		UserCopyNode *pUserNode = iter->second;
		if( pUserNode->GetPublicID() == szUserID )
		{
			g_UserNodeManager.RemoveCopyUser( pUserNode->GetUserIndex() );
			pUserNode->OnDestroy();
			m_UserMemNode.Push( pUserNode );
			break;;
		}		
	}	
}

//int ServerNode::ExceptionRemoveUserNode( DWORD dwIndex )
//{
//	int iReturnCnt = 0;
//	uUserCopyNode_iter iter, iter_Prev;
//	for(iter = m_UserCopyNode.begin();iter != m_UserCopyNode.end();)
//	{
//		iter_Prev = iter;
//		UserCopyNode *pUserCopyNode = *iter_Prev;
//		if( pUserCopyNode->GetUserIndex() == dwIndex )
//		{
//			pUserCopyNode->OnDestroy();
//			iter = m_UserCopyNode.erase( iter_Prev );
//
//			m_UserMemNode.Push( pUserCopyNode );
//			iReturnCnt++;
//		}		
//		else
//		{
//			iter++;
//		}
//	}	
//
//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ExceptionRemoveUserNode(%d) : %d ���� - %d�� ����", m_dwServerIndex, dwIndex, iReturnCnt );
//	return iReturnCnt;
//}

void ServerNode::InitRoomMemoryPool()
{
	ioINILoader kLoader( "ls_config_game.ini" );
	int iMaxRoomNodePool = kLoader.LoadInt( "MemoryPool", "room_pool", 1000 );

	// MemPooler
	m_RoomMemNode.CreatePool( 0, iMaxRoomNodePool, TRUE );
	for( int i = 0 ; i < iMaxRoomNodePool ; i++ )
		m_RoomMemNode.Push( new RoomCopyNode );
 
	strcpy_s(m_szPublicIP,g_App.GetPublicIP().c_str());
}

void ServerNode::ReleaseRoomMemoryPool()
{
	ReturnRoomMemoryPool();
	m_RoomMemNode.DestroyPool();
}

void ServerNode::ReturnRoomMemoryPool()
{
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomNode = *iter;
		g_RoomNodeManager.RemoveCopyRoom( pRoomNode );
		pRoomNode->OnDestroy();

		m_RoomMemNode.Push( pRoomNode );
	}	
	m_RoomCopyNode.clear();
}

RoomCopyNode *ServerNode::CreateNewRoom( int iIndex )
{
	RoomCopyNode *NewNode = GetRoomNode( iIndex );
	if( NewNode )
		return NewNode;

	//NewNode = (RoomCopyNode*)m_RoomMemNode.OutPoolAddr();
	NewNode = (RoomCopyNode*)m_RoomMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewRoom MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetRoomIndex( iIndex );
	m_RoomCopyNode.push_back( NewNode );
	return NewNode;
}

RoomCopyNode *ServerNode::GetRoomNode( int iIndex )
{
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomNode = *iter;
		if( pRoomNode->GetRoomIndex() == iIndex )
			return pRoomNode;
	}	
	return NULL;
}

void ServerNode::RemoveRoomNode( int iIndex )
{
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomNode = *iter;
		if( pRoomNode->GetRoomIndex() == iIndex )
		{
			g_RoomNodeManager.RemoveCopyRoom( pRoomNode );
			pRoomNode->OnDestroy();
			m_RoomCopyNode.erase( iter );

			m_RoomMemNode.Push( pRoomNode );
			return;
		}		
	}	
}

void ServerNode::RemoveRoomNode( RoomCopyNode *pRoomNode )
{
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomCopyNode = *iter;
		if( pRoomCopyNode == pRoomNode )
		{
			g_RoomNodeManager.RemoveCopyRoom( pRoomNode );
			pRoomCopyNode->OnDestroy();
			m_RoomCopyNode.erase( iter );

			m_RoomMemNode.Push( pRoomCopyNode );
			return;
		}		
	}	
}

void ServerNode::RemoveRoomCopyNodeAll()
{
	for(vRoomCopyNode_iter iter = m_RoomCopyNode.begin() ; iter != m_RoomCopyNode.end() ; ++iter)
	{
		RoomCopyNode *pRoomCopyNode = *iter;
		if( pRoomCopyNode )
		{
			g_RoomNodeManager.RemoveCopyRoom( pRoomCopyNode );

			pRoomCopyNode->OnDestroy();
			m_RoomMemNode.Push( pRoomCopyNode );
		}		
	}	
	m_RoomCopyNode.clear();
}

void ServerNode::InitBattleRoomMemoryPool()
{
	ioINILoader kLoader( "ls_config_game.ini" );
	int iMaxBattleRoomSize = kLoader.LoadInt( "MemoryPool", "battleroom_pool", 3000 );

	// MemPooler
	m_BattleRoomMemNode.CreatePool( 0, iMaxBattleRoomSize, TRUE );
	for( int i = 0 ; i < iMaxBattleRoomSize ; i++ )
		m_BattleRoomMemNode.Push( new BattleRoomCopyNode );
}

void ServerNode::ReleaseBattleRoomMemoryPool()
{
	ReturnBattleRoomMemoryPool();
	m_BattleRoomMemNode.DestroyPool();
}

void ServerNode::ReturnBattleRoomMemoryPool()
{
	vBattleRoomCopyNode_iter iter, iEnd;
	iEnd = m_BattleRoomCopyNode.end();
	for(iter = m_BattleRoomCopyNode.begin();iter != iEnd;++iter)
	{
		BattleRoomCopyNode *pBattleRoomNode = *iter;
		if( pBattleRoomNode )
			g_UserNodeManager.RemoveBattleRoomCopyNode( pBattleRoomNode->GetIndex() );

		g_BattleRoomManager.RemoveBattleCopyRoom( pBattleRoomNode );
		pBattleRoomNode->OnDestroy();

		m_BattleRoomMemNode.Push( pBattleRoomNode );
	}	
	m_BattleRoomCopyNode.clear();
}

BattleRoomCopyNode *ServerNode::CreateNewBattleRoom( DWORD dwIndex )
{
	BattleRoomCopyNode *NewNode = GetBattleRoomNode( dwIndex );
	if( NewNode )
		return NewNode;

	//NewNode = (BattleRoomCopyNode*)m_BattleRoomMemNode.OutPoolAddr();
	NewNode = (BattleRoomCopyNode*)m_BattleRoomMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewBattleRoom MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetIndex( dwIndex );
	m_BattleRoomCopyNode.push_back( NewNode );
	return NewNode;
}

BattleRoomCopyNode *ServerNode::GetBattleRoomNode( DWORD dwIndex )
{
	vBattleRoomCopyNode_iter iter, iEnd;
	iEnd = m_BattleRoomCopyNode.end();
	for(iter = m_BattleRoomCopyNode.begin();iter != iEnd;++iter)
	{
		BattleRoomCopyNode *pBattleRoomNode = *iter;
		if( pBattleRoomNode->GetIndex() == dwIndex )
			return pBattleRoomNode;
	}	
	return NULL;
}

void ServerNode::RemoveBattleRoomNode( DWORD dwIndex )
{
	vBattleRoomCopyNode_iter iter, iEnd;
	iEnd = m_BattleRoomCopyNode.end();
	for(iter = m_BattleRoomCopyNode.begin();iter != iEnd;++iter)
	{
		BattleRoomCopyNode *pBattleRoomNode = *iter;
		if( pBattleRoomNode->GetIndex() == dwIndex )
		{
			g_BattleRoomManager.RemoveBattleCopyRoom( pBattleRoomNode );
			
			pBattleRoomNode->OnDestroy();
			m_BattleRoomMemNode.Push( pBattleRoomNode );

			m_BattleRoomCopyNode.erase( iter );
			return;
		}		
	}	
}

void ServerNode::RemoveBattleRoomNode( BattleRoomCopyNode *pBattleRoomNode )
{
	vBattleRoomCopyNode_iter iter, iEnd;
	iEnd = m_BattleRoomCopyNode.end();
	for(iter = m_BattleRoomCopyNode.begin();iter != iEnd;++iter)
	{
		BattleRoomCopyNode *pBattleRoomCopyNode = *iter;
		if( pBattleRoomCopyNode == pBattleRoomNode )
		{
			g_BattleRoomManager.RemoveBattleCopyRoom( pBattleRoomNode );

			pBattleRoomCopyNode->OnDestroy();
			m_BattleRoomMemNode.Push( pBattleRoomCopyNode );

			m_BattleRoomCopyNode.erase( iter );
			return;
		}		
	}	
}

void ServerNode::RemoveBattleRoomCopyNodeAll()
{
	for(vBattleRoomCopyNode_iter iter = m_BattleRoomCopyNode.begin() ; iter != m_BattleRoomCopyNode.end() ; ++iter)
	{
		BattleRoomCopyNode *pBattleRoomCopyNode = *iter;
		if( pBattleRoomCopyNode )
		{
			g_BattleRoomManager.RemoveBattleCopyRoom( pBattleRoomCopyNode );

			pBattleRoomCopyNode->OnDestroy();
			m_BattleRoomMemNode.Push( pBattleRoomCopyNode );
		}		
	}	

	m_BattleRoomCopyNode.clear();
}

//////////////////////////////////////////////////////////////////////////
void ServerNode::InitShuffleRoomMemoryPool()
{
	ioINILoader kLoader( "ls_config_game.ini" );
	int iMaxShuffleRoomSize = kLoader.LoadInt( "MemoryPool", "shuffleroom_pool", 3000 );

	// MemPooler
	m_ShuffleRoomMemNode.CreatePool( 0, iMaxShuffleRoomSize, TRUE );
	for( int i = 0 ; i < iMaxShuffleRoomSize ; i++ )
		m_ShuffleRoomMemNode.Push( new ShuffleRoomCopyNode );
}

void ServerNode::ReleaseShuffleRoomMemoryPool()
{
	ReturnShuffleRoomMemoryPool();
	m_ShuffleRoomMemNode.DestroyPool();
}

void ServerNode::ReturnShuffleRoomMemoryPool()
{
	vShuffleRoomCopyNode_iter iter, iEnd;
	iEnd = m_ShuffleRoomCopyNode.end();
	for(iter = m_ShuffleRoomCopyNode.begin();iter != iEnd;++iter)
	{
		ShuffleRoomCopyNode *pShuffleRoomNode = *iter;
		if( pShuffleRoomNode )
			g_UserNodeManager.RemoveShuffleRoomCopyNode( pShuffleRoomNode->GetIndex() );

		g_ShuffleRoomManager.RemoveShuffleCopyRoom( pShuffleRoomNode );
		pShuffleRoomNode->OnDestroy();

		m_ShuffleRoomMemNode.Push( pShuffleRoomNode );
	}	
	m_ShuffleRoomCopyNode.clear();
}

ShuffleRoomCopyNode *ServerNode::CreateNewShuffleRoom( DWORD dwIndex )
{
	ShuffleRoomCopyNode *NewNode = GetShuffleRoomNode( dwIndex );
	if( NewNode )
		return NewNode;

	NewNode = (ShuffleRoomCopyNode*)m_ShuffleRoomMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewShuffleRoom MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetIndex( dwIndex );
	m_ShuffleRoomCopyNode.push_back( NewNode );
	return NewNode;
}

ShuffleRoomCopyNode *ServerNode::GetShuffleRoomNode( DWORD dwIndex )
{
	vShuffleRoomCopyNode_iter iter, iEnd;
	iEnd = m_ShuffleRoomCopyNode.end();
	for( iter = m_ShuffleRoomCopyNode.begin() ; iter != iEnd ; ++iter )
	{
		ShuffleRoomCopyNode *pShuffleRoomNode = *iter;
		if( pShuffleRoomNode->GetIndex() == dwIndex )
			return pShuffleRoomNode;
	}	
	return NULL;
}

void ServerNode::RemoveShuffleRoomNode( DWORD dwIndex )
{
	vShuffleRoomCopyNode_iter iter, iEnd;
	iEnd = m_ShuffleRoomCopyNode.end();
	for( iter = m_ShuffleRoomCopyNode.begin() ; iter != iEnd ; ++iter )
	{
		ShuffleRoomCopyNode *pShuffleRoomNode = *iter;
		if( pShuffleRoomNode->GetIndex() == dwIndex )
		{
			g_ShuffleRoomManager.RemoveShuffleCopyRoom( pShuffleRoomNode );

			pShuffleRoomNode->OnDestroy();
			m_ShuffleRoomMemNode.Push( pShuffleRoomNode );

			m_ShuffleRoomCopyNode.erase( iter );
			return;
		}		
	}	
}

void ServerNode::RemoveShuffleRoomNode( ShuffleRoomCopyNode *pShuffleRoomNode )
{
	vShuffleRoomCopyNode_iter iter, iEnd;
	iEnd = m_ShuffleRoomCopyNode.end();
	for( iter = m_ShuffleRoomCopyNode.begin() ; iter != iEnd ; ++iter )
	{
		ShuffleRoomCopyNode *pShuffleRoomCopyNode = *iter;
		if( pShuffleRoomCopyNode == pShuffleRoomNode )
		{
			g_ShuffleRoomManager.RemoveShuffleCopyRoom( pShuffleRoomNode );

			pShuffleRoomCopyNode->OnDestroy();
			m_ShuffleRoomMemNode.Push( pShuffleRoomCopyNode );

			m_ShuffleRoomCopyNode.erase( iter );
			return;
		}		
	}	
}

void ServerNode::RemoveShuffleRoomCopyNodeAll()
{
	for( vShuffleRoomCopyNode_iter iter = m_ShuffleRoomCopyNode.begin() ; iter != m_ShuffleRoomCopyNode.end() ; ++iter )
	{
		ShuffleRoomCopyNode *pShuffleRoomCopyNode = *iter;
		if( pShuffleRoomCopyNode )
		{
			g_ShuffleRoomManager.RemoveShuffleCopyRoom( pShuffleRoomCopyNode );

			pShuffleRoomCopyNode->OnDestroy();
			m_ShuffleRoomMemNode.Push( pShuffleRoomCopyNode );
		}		
	}	

	m_ShuffleRoomCopyNode.clear();
}

//////////////////////////////////////////////////////////////////////////
void ServerNode::InitChannelMemoryPool()
{
	ioINILoader kLoader( "ls_config_game.ini" );
	int iMaxRoomNodePool = kLoader.LoadInt( "MemoryPool", "channel_pool", 10000 );

	// MemPooler
	m_ChannelMemNode.CreatePool( 0, iMaxRoomNodePool, TRUE );
	for( int i = 0 ; i < iMaxRoomNodePool ; i++ )
		m_ChannelMemNode.Push( new ChannelCopyNode );
}

void ServerNode::ReleaseChannelMemoryPool()
{
	ReturnChannelMemoryPool();
	m_ChannelMemNode.DestroyPool();
}

void ServerNode::ReturnChannelMemoryPool()
{
	vChannelCopyNode_iter iter, iEnd;
	iEnd = m_ChannelCopyNode.end();
	for(iter = m_ChannelCopyNode.begin();iter != iEnd;++iter)
	{
		ChannelCopyNode *pChannelNode = *iter;
		if( pChannelNode )
			g_UserNodeManager.RemoveChannelCopyNode( pChannelNode->GetIndex() );

		g_ChannelNodeManager.RemoveCopyChannel( pChannelNode->GetIndex() );
		pChannelNode->OnDestroy();

		m_ChannelMemNode.Push( pChannelNode );
	}	
	m_ChannelCopyNode.clear();
}

ChannelCopyNode *ServerNode::CreateNewChannel( DWORD dwIndex )
{
	ChannelCopyNode *NewNode = GetChannelNode( dwIndex );
	if( NewNode )
		return NewNode;

	//NewNode = (ChannelCopyNode*)m_ChannelMemNode.OutPoolAddr();
	NewNode = (ChannelCopyNode*)m_ChannelMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewChannel MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetIndex( dwIndex );
	m_ChannelCopyNode.push_back( NewNode );
	return NewNode;
}

ChannelCopyNode *ServerNode::GetChannelNode( DWORD dwIndex )
{
	vChannelCopyNode_iter iter, iEnd;
	iEnd = m_ChannelCopyNode.end();
	for(iter = m_ChannelCopyNode.begin();iter != iEnd;++iter)
	{
		ChannelCopyNode *pChannelNode = *iter;
		if( pChannelNode->GetIndex() == dwIndex )
			return pChannelNode;
	}	
	return NULL;
}

void ServerNode::RemoveChannelNode( DWORD dwIndex )
{
	vChannelCopyNode_iter iter, iEnd;
	iEnd = m_ChannelCopyNode.end();
	for(iter = m_ChannelCopyNode.begin();iter != iEnd;++iter)
	{
		ChannelCopyNode *pChannelNode = *iter;
		if( pChannelNode->GetIndex() == dwIndex )
		{
			g_ChannelNodeManager.RemoveCopyChannel( pChannelNode->GetIndex() );
			pChannelNode->OnDestroy();
			m_ChannelCopyNode.erase( iter );

			m_ChannelMemNode.Push( pChannelNode );
			return;
		}		
	}	
}

void ServerNode::RemoveChannelNode( ChannelCopyNode *pChannelNode )
{
	vChannelCopyNode_iter iter, iEnd;
	iEnd = m_ChannelCopyNode.end();
	for(iter = m_ChannelCopyNode.begin();iter != iEnd;++iter)
	{
		ChannelCopyNode *pChannelCopyNode = *iter;
		if( pChannelCopyNode == pChannelNode )
		{
			g_ChannelNodeManager.RemoveCopyChannel( pChannelNode->GetIndex() );
			pChannelCopyNode->OnDestroy();
			m_ChannelCopyNode.erase( iter );

			m_ChannelMemNode.Push( pChannelCopyNode );
			return;
		}		
	}	
}

void ServerNode::InitLadderTeamMemoryPool()
{
	ioINILoader kLoader( "ls_config_game.ini" );
	int iMaxLadderTeamSize = kLoader.LoadInt( "MemoryPool", "ladderteam_pool", 3000 );

	// MemPooler
	m_LadderTeamMemNode.CreatePool( 0, iMaxLadderTeamSize, TRUE );
	for( int i = 0 ; i < iMaxLadderTeamSize ; i++ )
		m_LadderTeamMemNode.Push( new LadderTeamCopyNode );
}

void ServerNode::ReleaseLadderTeamMemoryPool()
{
	ReturnLadderTeamMemoryPool();
	m_LadderTeamMemNode.DestroyPool();
}

void ServerNode::ReturnLadderTeamMemoryPool()
{
	vLadderTeamCopyNode_iter iter, iEnd;
	iEnd = m_LadderTeamCopyNode.end();
	for(iter = m_LadderTeamCopyNode.begin();iter != iEnd;++iter)
	{
		LadderTeamCopyNode *pLadderTeamNode = *iter;
		if( pLadderTeamNode )
			g_UserNodeManager.RemoveLadderTeamCopyNode( pLadderTeamNode->GetIndex() );

		g_LadderTeamManager.RemoveLadderTeamCopy( pLadderTeamNode );
		pLadderTeamNode->OnDestroy();

		m_LadderTeamMemNode.Push( pLadderTeamNode );
	}	
	m_LadderTeamCopyNode.clear();
}

LadderTeamCopyNode *ServerNode::CreateNewLadderTeam( DWORD dwIndex )
{
	LadderTeamCopyNode *NewNode = GetLadderTeamNode( dwIndex );
	if( NewNode )
		return NewNode;

	//NewNode = (LadderTeamCopyNode*)m_LadderTeamMemNode.OutPoolAddr();
	NewNode = (LadderTeamCopyNode*)m_LadderTeamMemNode.Remove();
	if( !NewNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL,"ServerNode::CreateNewLadderTeam MemPool Zero!");
		return NULL;
	}

	NewNode->OnCreate( this );
	NewNode->SetIndex( dwIndex );
	m_LadderTeamCopyNode.push_back( NewNode );
	return NewNode;
}

LadderTeamCopyNode *ServerNode::GetLadderTeamNode( DWORD dwIndex )
{
	vLadderTeamCopyNode_iter iter, iEnd;
	iEnd = m_LadderTeamCopyNode.end();
	for(iter = m_LadderTeamCopyNode.begin();iter != iEnd;++iter)
	{
		LadderTeamCopyNode *pLadderTeamNode = *iter;
		if( pLadderTeamNode->GetIndex() == dwIndex )
			return pLadderTeamNode;
	}	
	return NULL;
}

void ServerNode::RemoveLadderTeamNode( DWORD dwIndex )
{
	vLadderTeamCopyNode_iter iter, iEnd;
	iEnd = m_LadderTeamCopyNode.end();
	for(iter = m_LadderTeamCopyNode.begin();iter != iEnd;++iter)
	{
		LadderTeamCopyNode *pLadderTeamNode = *iter;
		if( pLadderTeamNode->GetIndex() == dwIndex )
		{
			g_LadderTeamManager.RemoveLadderTeamCopy( pLadderTeamNode );
			pLadderTeamNode->OnDestroy();
			m_LadderTeamCopyNode.erase( iter );

			m_LadderTeamMemNode.Push( pLadderTeamNode );
			return;
		}		
	}	
}

void ServerNode::RemoveLadderTeamNode( LadderTeamCopyNode *pLadderTeamNode )
{
	vLadderTeamCopyNode_iter iter, iEnd;
	iEnd = m_LadderTeamCopyNode.end();
	for(iter = m_LadderTeamCopyNode.begin();iter != iEnd;++iter)
	{
		LadderTeamCopyNode *pLadderTeamCopyNode = *iter;
		if( pLadderTeamCopyNode == pLadderTeamNode )
		{
			g_LadderTeamManager.RemoveLadderTeamCopy( pLadderTeamNode );
			pLadderTeamCopyNode->OnDestroy();
			m_LadderTeamCopyNode.erase( iter );

			m_LadderTeamMemNode.Push( pLadderTeamCopyNode );
			return;
		}		
	}	
}

void ServerNode::SetBlockState(const bool bBlockState)
{ 
	m_bBlockState = bBlockState; 

	// ���� ������������ ������ ����
	for(vBattleRoomCopyNode_iter iter = m_BattleRoomCopyNode.begin() ;iter != m_BattleRoomCopyNode.end() ; ++iter)
	{
		BattleRoomCopyNode *pBattleRoomCopyNode = *iter;
		pBattleRoomCopyNode->SetBlockFlag( bBlockState );
	}	

	// ���� ���ù������� ������ ����
	for(vShuffleRoomCopyNode_iter iter = m_ShuffleRoomCopyNode.begin() ;iter != m_ShuffleRoomCopyNode.end() ; ++iter)
	{
		ShuffleRoomCopyNode *pShuffleRoomCopyNode = *iter;
		pShuffleRoomCopyNode->SetBlockFlag( bBlockState );
	}	

	// ���� ��(����)������ ������ ����
	for(vRoomCopyNode_iter iter = m_RoomCopyNode.begin() ; iter != m_RoomCopyNode.end() ; ++iter)
	{
		RoomCopyNode *pRoomCopyNode = *iter;
		if( pRoomCopyNode->GetRoomStyle() == RSTYLE_PLAZA )
		{
			pRoomCopyNode->SetBlockFlag( bBlockState );
		}	
	}
}

bool ServerNode::IsBusy()
{
	if( !IsConnectWorkComplete() )	return true;
	if( IsBlocked() )				return true;
	if( IsFull() )					return true;
	if( IsRoomCreateClose() )		return true;
	return false;
}

bool ServerNode::IsRoomCreateClose()
{
	if( m_dwRecvPingTime == 0 ) return false;

	DWORD dwPingGap = TIMEGETTIME() - m_dwRecvPingTime;
	if( dwPingGap >= g_ServerNodeManager.GetServerRoomCreateClosePingTime() ) 
		return true;          // 10�ʸ��� �� üũ�� �ϹǷ� 15�ʵ��� ������ �ȿ� ������ ������� �Ͻ� ����.
	return false;
}

bool ServerNode::IsFull()
{ 
	if( GetUserNodeSize() >= g_UserNodeManager.GetStableConnection() )
	{
		return true;
	}

	return false; 
}

bool ServerNode::IsMyPartition(const int iPartitionIndex)
{
	if( GetPartitionIndex() == iPartitionIndex )
	{
		return true;
	}
	return false;
}

int ServerNode::GetPlazaRoomCount()
{
	int iCount = 0;
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomCopyNode = *iter;
		if( pRoomCopyNode->GetRoomStyle() == RSTYLE_PLAZA )
		{
			if( pRoomCopyNode->GetModeType() == MT_TRAINING ) 
				iCount++;
		}	
	}	
	return iCount;
}

int ServerNode::GetHeadquartersRoomCount()
{
	int iCount = 0;
	vRoomCopyNode_iter iter, iEnd;
	iEnd = m_RoomCopyNode.end();
	for(iter = m_RoomCopyNode.begin();iter != iEnd;++iter)
	{
		RoomCopyNode *pRoomCopyNode = *iter;
		if( pRoomCopyNode->GetRoomStyle() == RSTYLE_HEADQUARTERS )
		{
			if( pRoomCopyNode->GetModeType() == MT_HEADQUARTERS || pRoomCopyNode->GetModeType() == MT_HOUSE ) 
				iCount++;
		}	
	}	
	return iCount;
}

int ServerNode::GetBattleRoomCount()
{
	return (int)m_BattleRoomCopyNode.size();
}

int ServerNode::GetLadderTeamCount( bool bHeroMode )
{
	if( bHeroMode )
		return GetLadderHeroModeCount();

	int iCount = (int)m_LadderTeamCopyNode.size() - GetLadderHeroModeCount();

	return iCount;
}

int ServerNode::GetLadderHeroModeCount()
{
	int iReturnNodeSize = 0;
	vLadderTeamCopyNode_iter iter, iEnd;
	iEnd = m_LadderTeamCopyNode.end();
	for(iter = m_LadderTeamCopyNode.begin();iter != iEnd;++iter)
	{
		LadderTeamCopyNode *pLadderTeamNode = *iter;
		if( pLadderTeamNode == NULL ) continue;
		if( pLadderTeamNode->IsHeroMatchMode() == false ) continue;

		iReturnNodeSize++;
	}	
	return iReturnNodeSize;
}

int ServerNode::GetShuffleRoomCount()
{
	return (int)m_BattleRoomCopyNode.size();
}

void ServerNode::InitData()
{
	m_eNodeType				= NODE_TYPE_NONE;
	m_eNodeRole				= NODE_ROLE_NONE;
	m_bBlockState			= false;
	m_dwServerIndex			= 0;
	m_iServerPort			= 0;
	m_iClientPort			= 0;
	m_bConnectWorkComplete	= false;
	m_dwPingTIme			= 0;
	m_dwRecvPingTime		= 0;
	m_eSessionState			= SS_DISCONNECT;
	m_iUserCount			= 0;
	m_iRelayIndex			= 0;
	m_iPartitionIndex		= 0;

	m_szServerIP.Clear();
	m_szClientMoveIP.Clear();
	m_ConnectWorkingPacket.clear();

	ZeroMemory(&m_RelayInfo,sizeof(m_RelayInfo));
}

void ServerNode::OnCreate()
{
	CConnectNode::OnCreate();
	InitData();
	m_eSessionState = SS_CONNECT;
}

void ServerNode::OnDestroy()
{
	g_CriticalError.CheckGameServerDisconnect( m_szServerIP, m_iServerPort );

	CConnectNode::OnDestroy();
	m_eSessionState = SS_DISCONNECT;
}

void ServerNode::OnSessionDestroy()
{
	ReturnUserMemoryPool();
	ReturnRoomMemoryPool();
	ReturnBattleRoomMemoryPool();
	ReturnChannelMemoryPool();
	ReturnLadderTeamMemoryPool();
	ReturnShuffleRoomMemoryPool();
}

void ServerNode::SessionClose(BOOL safely)
{
	if(!safely)
	{
		g_CriticalError.CheckGameServerExceptionDisconnect( GetServerIP(), GetServerPort(), GetLastError() );
	}

	CPacket packet(SSTPK_CLOSE);
	ReceivePacket( packet );
}

bool ServerNode::SendMessage( CPacket &rkPacket )
{
	g_ProcessChecker.ServerSendMessage( rkPacket.GetPacketID(), rkPacket.GetBufferSize() );
	return CConnectNode::SendMessage( rkPacket );
}

bool ServerNode::CheckNS( CPacket &rkPacket )
{
	return true;             //��Ʈ�� ���� �ʿ����.
}

int ServerNode::GetConnectType()
{
	return CONNECT_TYPE_SERVER;
}

void ServerNode::RequestJoin()
{
	// ���� ���� ���� ����
	SP2Packet kPacket( SSTPK_CONNECT_INFO );
	kPacket << g_ServerNodeManager.GetServerIndex() << g_App.GetPrivateIP() << g_App.GetClientMoveIP() << g_App.GetSSPort() << g_App.GetCSPort();
	SendMessage( kPacket );
}

void ServerNode::RequestSync()
{
	// �������� ��带 ����ȭ �Ѵ�. ���� - > �� - > ���� �� - > ä�� - > ����� - > ���÷� - > ����ȭ �Ϸ� ������ ����.
	g_UserNodeManager.ConnectServerNodeSync( this );
	g_RoomNodeManager.ConnectServerNodeSync( this );
	g_BattleRoomManager.ConnectServerNodeSync( this );
	g_ChannelNodeManager.ConnectServerNodeSync( this );
	g_LadderTeamManager.ConnectServerNodeSync( this );
	g_ShuffleRoomManager.ConnectServerNodeSync( this );

	SP2Packet kPacket( SSTPK_CONNECT_SYNC );
	kPacket << SSTPK_CONNECT_SYNC_COMPLETE;
	SendMessage( kPacket );
}

void ServerNode::AfterJoinProcess()
{
	g_ServerNodeManager.CalculateMaxNode();
}

void ServerNode::ProcessPing()
{
	if( !IsActive() ) return;
	if( m_dwPingTIme == 0 ) return;
	
	if( TIMEGETTIME() - m_dwPingTIme > SERVER_PING_CHECK_TIME )       
	{
		m_dwPingTIme = TIMEGETTIME();
		SP2Packet kPacket( SSTPK_PING );
		SendMessage( kPacket );
	}
}

void ServerNode::SetNodeType( ServerNode::NodeTypes eType )
{
	m_eNodeType = eType;
}

void ServerNode::SetNodeRole( ServerNode::NodeRoles eType )
{
	m_eNodeRole = eType;
}

User *ServerNode::GetUserOriginalNode( DWORD dwUserIndex, SP2Packet &rkPacket )
{
	User *pUser = g_UserNodeManager.GetUserNode( dwUserIndex );
	if( pUser )
		return pUser;

	// ������ ������ �̵��߰ų� ���� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( pUserParent && !pUserParent->IsUserOriginal() )
	{
		UserCopyNode *pCopyNode = (UserCopyNode*)pUserParent;
		pCopyNode->SendMessage( rkPacket );
	}
	
	// ���� ����� �׳� NULL
	return NULL;
}

User *ServerNode::GetUserOriginalNode( const ioHashString &szPublicID, SP2Packet &rkPacket )
{
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
		return pUser;

	// ������ ������ �̵��߰ų� ���� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( szPublicID );
	if( pUserParent && !pUserParent->IsUserOriginal() )
	{
		UserCopyNode *pCopyNode = (UserCopyNode*)pUserParent;
		pCopyNode->SendMessage( rkPacket );
	}

	// ���� ����� �׳� NULL
	return NULL;
}

User * ServerNode::GetUserOriginalNodeByPrivateID( const ioHashString &szPrivateID, SP2Packet &rkPacket )
{
	User *pUser = g_UserNodeManager.GetUserNodeByPrivateID( szPrivateID );
	if( pUser )
		return pUser;

	// ������ ������ �̵��߰ų� ���� ����
	UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNodeByPrivateID( szPrivateID );
	if( pUserParent && !pUserParent->IsUserOriginal() )
	{
		UserCopyNode *pCopyNode = (UserCopyNode*)pUserParent;
		pCopyNode->SendMessage( rkPacket );
	}

	// ���� ����� �׳� NULL
	return NULL;
}

void ServerNode::ReceivePacket( CPacket &packet )
{
	g_RecvQueue.InsertQueue( (DWORD)this, packet, PK_QUEUE_SESSION );
}

void ServerNode::PacketParsing( CPacket &packet )
{
	SP2Packet &kPacket = (SP2Packet&)packet;

	g_ProcessChecker.ServerPacketProcess( kPacket.GetPacketID() );

	FUNCTION_TIME_CHECKER( 500000.0f, kPacket.GetPacketID() );          // 0.5 �� �̻� �ɸ���α� ����
	
	if( OnConnectWorkingPacket( kPacket ) )
		return;

	if( OnUserTransferTCP( kPacket ) )
		return;

	if( OnWebUDPParse( kPacket ) )
		return;

	if( OnBillingParse( kPacket ) )
		return;

	switch( kPacket.GetPacketID() )
	{

	case SSTPK_CLOSE:
		OnClose( kPacket );
		break;
	case SSTPK_CONNECT_INFO:
		OnConnectInfo( kPacket );
		break;
	case SSTPK_CONNECT_SYNC:
		OnConnectSync( kPacket );
		break;
	case SSTPK_PING:
		OnPing( kPacket );
		break;
	case SSTPK_ROOM_SYNC:
		OnRoomSync( kPacket );
		break;
	case SSTPK_MOVING_ROOM:
		OnMovingRoom( kPacket );
		break;
	case SSTPK_MOVING_ROOM_RESULT:
		OnMovingRoomResult( kPacket );
		break;
	case SSTPK_USER_DATA_MOVE:
		OnUserDataMove( kPacket );
		break;
	case SSTPK_INVENTORY_DATA_MOVE_RESULT:
		OnUserInventoryMoveResult( kPacket );
		break;
	case SSTPK_EXTRAITEM_DATA_MOVE_RESULT:
		OnUserExtraItemMoveResult( kPacket );
		break;
	case SSTPK_QUEST_DATA_MOVE_RESULT:
		OnUserQuestMoveResult( kPacket );
		break;
	case SSTPK_USER_DATA_MOVE_RESULT:
		OnUserDataMoveResult( kPacket );
		break;
	case SSTPK_USER_SYNC:
		OnUserSync( kPacket );
		break;
	case SSTPK_USER_MOVE:
		OnUserMove( kPacket );
		break;
	case SSTPK_USER_FRIEND_MOVE:
		OnUserFriendMove( kPacket );
		break;
	case SSTPK_USER_FRIEND_MOVE_RESULT:
		OnUserFriendMoveResult( kPacket );
		break;
	case SSTPK_FOLLOW_USER:
		OnFollowUser( kPacket );
		break;
	case SSTPK_USER_POS_INDEX:
		OnUserPosIndex( kPacket );
		break;
	case SSTPK_CHANNEL_SYNC:
		OnChannelSync( kPacket );
		break;
	case SSTPK_CHANNEL_INVITE:
		OnChannelInvite( kPacket );
		break;
	case SSTPK_CHANNEL_CHAT:
		OnChannelChat( kPacket );
		break;
	case SSTPK_BATTLEROOM_SYNC:
		OnBattleRoomSync( kPacket );
		break;
	case SSTPK_BATTLEROOM_TRANSFER:
		OnBattleRoomTransfer( kPacket );
		break;
	case SSTPK_BATTLEROOM_JOIN_RESULT:
		OnBattleRoomJoinResult( kPacket );
		break;
	case SSTPK_BATTLEROOM_FOLLOW:
		OnBattleRoomFollow( kPacket );
		break;
	case SSTPK_PLAZAROOM_TRANSFER:
		OnPlazaRoomTransfer( kPacket );
		break;
	case SSTPK_USER_INFO_REFRESH:
		OnUserInfoRefresh( kPacket );
		break;
	case SSTPK_SIMPLE_USER_INFO_REFRESH:
		OnSimpleUserInfoRefresh( kPacket );
		break;
	case SSTPK_USER_CHAR_INFO_REFRESH:
		OnUserCharInfoRefresh( kPacket );
		break;
	case SSTPK_USER_CHAR_SUB_INFO_REFRESH:
		OnUserCharSubInfoRefresh( kPacket );
		break;
	case SSTPK_BATTLEROOM_KICK_OUT:
		OnBattleRoomKickOut( kPacket );
		break;
	case SSTPK_OFFLINE_MEMO:
		OnOfflineMemo( kPacket );
		break;
	case SSTPK_RESERVE_CREATE_ROOM:
		OnReserveCreateRoom( kPacket );
		break;
	case SSTPK_RESERVE_CREATE_BATTLEROOM:
		OnReserveCreateBattleRoom( kPacket );
		break;
	case SSTPK_RESERVE_CREATE_BATTLEROOM_RESULT:
		OnReserveCreateBattleRoomResult( kPacket );
		break;
	case SSTPK_EXCEPTION_BATTLEROOM_LEAVE:
		OnExceptionBattleRoomLeave( kPacket );
		break;
	case SSTPK_CREATE_GUILD_RESULT:
		OnCreateGuildResult( kPacket );
		break;
	case SSTPK_CREATE_GUILD_COMPLETE:
		OnCreateGuildComplete( kPacket );
		break;
	case SSTPK_GUILD_ENTRY_AGREE:
		OnGuildEntryAgree( kPacket );
		break;
	case SSTPK_GUILD_MEMBER_LIST_EX:
		OnGuildMemberListEx( kPacket );
		break;
	case SSTPK_GUILD_INVITATION:
		OnGuildInvitation( kPacket );
		break;
	case SSTPK_GUILD_MASTER_CHANGE:
		OnGuildMasterChange( kPacket );
		break;
	case SSTPK_GUILD_POSITION_CHANGE:
		OnGuildPositionChange( kPacket );
		break;
	case SSTPK_GUILD_KICK_OUT:
		OnGuildKickOut( kPacket );
		break;
	case SSTPK_FRIEND_DELETE:
		OnFriendDelete( kPacket );
		break;
	case SSTPK_GUILD_MARK_CHANGE:
		OnGuildMarkChange( kPacket );
		break;
	case SSTPK_GUILD_USER_DELETE:
		OnGuildUserDelete( kPacket );
		break;
	case SSTPK_LADDERTEAM_SYNC:
		OnLadderTeamSync( kPacket );
		break;
	case SSTPK_LADDERTEAM_TRANSFER:
		OnLadderTeamTransfer( kPacket );
		break;
	case SSTPK_EXCEPTION_LADDERTEAM_LEAVE:
		OnExceptionLadderTeamLeave( kPacket );
		break;
	case SSTPK_RESERVE_CREATE_LADDERTEAM:
		OnReserveCreateLadderTeam( kPacket );
		break;
	case SSTPK_RESERVE_CREATE_LADDERTEAM_RESULT:
		OnReserveCreateLadderTeamResult( kPacket );
		break;
	case SSTPK_LADDERTEAM_JOIN_RESULT:
		OnLadderTeamJoinResult( kPacket );
		break;
	case SSTPK_LADDERTEAM_KICK_OUT:
		OnLadderTeamKickOut( kPacket );
		break;
	case SSTPK_LADDERTEAM_FOLLOW:
		OnLadderTeamFollow( kPacket );
		break;
	case SSTPK_LADDERTEAM_ENTER_ROOM_USER:
		OnLadderTeamEnterRoomUser( kPacket );
		break;
	case SSTPK_CAMP_SEASON_BONUS:
		OnCampSeasonBonus( kPacket );
		break;
	case SSTPK_GUILD_NAME_CHANGE:
		OnGuildNameChange( kPacket );
		break;
	case SSTPK_GUILD_NAME_CHANGE_RESULT:
		OnGuildNameChangeResult( kPacket );
		break;
	case SSTPK_WHOLE_CHAT:
		OnWholeChat( kPacket );
		break;
	case SSTPK_UDP_RECV_TIMEOUT:
		OnUDPRecvTimeOut( kPacket );
		break;
	case SSTPK_SERVER_ALARM_MENT_UDP:
		OnServerAlarmMent( kPacket );
		break;
	case SSTPK_PRESENT_SELECT:
		OnPresentSelect( kPacket );
		break;
	case SSTPK_SUBSCRIPTION_SELECT:
		OnSubscriptionSelect( kPacket );
		break;
	case SSTPK_USER_HERO_DATA:
		OnUserHeroData( kPacket );
		break;
	case SSTPK_HERO_MATCH_OTHER_INFO:
		OnHeroMatchOtherInfo( kPacket );
		break;
	case SSTPK_USER_CHAR_RENTAL_AGREE:
		OnUserCharRentalAgree( kPacket );
		break;

	case SSTPK_TRADE_CREATE:
		OnTradeCreate( kPacket );
		break;
	case SSTPK_TRADE_CREATE_COMPLETE:
		OnTradeCreateComplete( kPacket );
		break;
	case SSTPK_TRADE_CREATE_FAIL:
		OnTradeCreateFail( kPacket );
		break;
	case SSTPK_TRADE_ITEM_COMPLETE:
		OnTradeItemComplete( kPacket );
		break;
	case SSTPK_TRADE_CANCEL:
		OnTradeCancel( kPacket );
		break;
	case SSTPK_TRADE_TIME_OUT:
		OnTradeTimeOut( kPacket );
		break;
	case SSTPK_EVENT_ITEM_INITIALIZE:
		OnEventItemInitialize( kPacket );
		break;

	case SSTPK_JOIN_HEADQUARTERS_USER:
		OnJoinHeadquartersUser( kPacket );
		break;
	case SSTPK_HEADQUARTERS_INFO:
		OnHeadquartersInfo( kPacket );
		break;
	case SSTPK_HEADQUARTERS_ROOM_INFO:
		OnHeadquartersRoomInfo( kPacket );
		break;
	case SSTPK_HEADQUARTERS_JOIN_AGREE:
		OnHeadquartersJoinAgree( kPacket );
		break;
	case SSTPK_LOGOUT_ROOM_ALARM:
		OnLogoutRoomAlarm( kPacket );
		break;
	case SSTPK_DISCONNECT_ALREADY_ID:
		OnDisconnectAlreadyID( kPacket );
		break;
	case SSTPK_ALCHEMIC_DATA_MOVE_RESULT:
		OnUserAlchemicMoveResult( kPacket );
		break;
	case LSPTK_CONNECT_REQUEST: // �α��μ������� ó�� ���ӽ� ������ ��������
		OnLoginConnect( kPacket );
		break;
	case LSTPK_STATUS_REQUEST: // �α��� ������ ���Ӽ��� ���¸� �ֱ������� ��û�ϴ� ��������
		OnLoginRequestStatus();
		break;
	case LSPTK_BLOCK_REQUEST: // �α��� ������ ���Ӽ��� ����� ��û�ϴ� ��������
		OnLoginBlockRequest( kPacket );
		break;
	case LSPTK_PARTITION_REQUEST: // �α��� ������ ���Ӽ����� ��Ƽ�� ó���� ��û�ϴ� ��������
		OnLoginPartitionRequest( kPacket );
		break;
	case SSTPK_TOURNAMENT_TEAM_CREATE:
		OnTournamentCreateTeam( kPacket );
		break;
	case SSTPK_TOURNAMENT_TEAM_ENTRY_AGREE_OK:
		OnTournamentTeamAgreeOK( kPacket );
		break;
	case SSTPK_TOURNAMENT_TEAM_JOIN:
		OnTournamentTeamJoin( kPacket );
		break;
	case SSTPK_TOURNAMENT_TEAM_LEAVE:
		OnTournamentTeamLeave( kPacket );
		break;
	case SSTPK_PRESENT_INSERT:
		OnPresentInsert( kPacket );
		break;
	case SSTPK_CLOVER_SEND:	// ģ���� ����. ������
		OnCloverSend( kPacket );
		break;
	case RSPTK_ON_CONNECT:
		OnRelayServerConnect( kPacket);
		break;
		//relay ��� ����!!
 	case RSTPK_ON_CONTROL:
 		OnRelayControl(kPacket);
 		break;
	case SSTPK_ETC_ITEM_SEND_PRESENT:
		OnPresentInsertByEtcItem( kPacket );
		break;
	case SSTPK_SHUFFLEROOM_TRANSFER:
		OnShuffleRoomTransfer( kPacket );
		break;
	case SSTPK_SHUFFLEROOM_JOIN_RESULT:
		OnShuffleRoomJoinResult( kPacket );
		break;
	case SSTPK_SHUFFLEROOM_SYNC:
		OnShuffleRoomSync( kPacket );
		break;
	case SSTPK_SHUFFLEROOM_KICK_OUT:
		OnShuffleRoomKickOut( kPacket );
		break;
	case SSTPK_EXCEPTION_SHUFFLEROOM_LEAVE:
		OnExceptionShuffleRoomLeave( kPacket );
		break;
	case SSTPK_SHUFFLEROOM_GLOBAL_CREATE:
		OnShuffleRoomGlobalCreate( kPacket );
		break;
	case SSTPK_PET_DATA_MOVE_RESULT:
		OnUserPetMoveResult( kPacket );
		break;
	case SSTPK_CHAR_AWAKE_MOVE_RESULT:
		OnUserCharAwakeMoveResult( kPacket );
		break;
	case SSTPK_NEW_PET_DATA_INFO:
		OnMoveNewPetData( kPacket ); 
		break;
	case SSTPK_SOLDIER_DATA_MOVE_RESULT:
		OnMoveSoldierData( kPacket );
		break;
	case SSTPK_ETCITEM_DATA_MOVE_RESULT:
		OnMoveEtcItemData( kPacket );
		break;
	case SSTPK_MEDALITEM_DATA_MOVE_RESULT:
		OnMoveMedalItemData( kPacket );
		break;
	case SSTPK_RAINBOW_WHOLE_CHAT:
		OnRainbowWholeChat( kPacket );
		break;
	case SSTPK_COSTUME_DATA_MOVE_RESULT:
		OnMoveCostumeData(kPacket);
		break;
	case SSTPK_COSTUME_ADD:
		OnCostumeAdd(kPacket);
		break;
	case SSTPK_ACCESSORY_ADD:
		OnAccessoryAdd(kPacket);
		break;
	case SSTPK_MISSION_DATA_MOVE_RESULT:
		OnMoveMissionData(kPacket);
		break;
	case SSTPK_ROOLBOOK_DATA_MOVE_RESULT:
		OnMoveRollBookData(kPacket);
		break;
	case SSTPK_ACCESSORY_DATA_MOVE_RESULT:
		OnMoveAccessroyData(kPacket);
		break;
	case SSTPK_RESERVE_CREATE_GUILD_ROOM:
		OnReserveCreateGuildRoom(kPacket);
		break;
	case SSTPK_ACTIVE_GUILD_ROOM:
		OnChangeGuildRoomStatus(kPacket);
		break;
	case SSTPK_JOIN_PERSONAL_HQ_USER:
		OnJoinPersonalHQUser( kPacket );
		break;
	case SSTPK_PERSONAL_HQ_INFO:
		OnPersonalHQInfo( kPacket );
		break;
	case SSTPK_PERSONAL_HQ_ROOM_INFO:
		OnPersonalHQRoomInfo( kPacket );
		break;
	case SSTPK_PERSONAL_HQ_JOIN_AGREE:
		OnPersonalHQJoinAgree(kPacket);
		break;
	case SSTPK_PERSONAL_HQ_DATA_MOVE_RESULT:
		OnMovePersonalHQData(kPacket);
		break;
	case SSTPK_PERSONAL_HQ_ADD_BLOCK:
		OnPersonalHQAddBlock(kPacket);
		break;
	case SSTPK_TIMECASH_DATA_MOVE_RESULT:
		OnMoveTimeCashData(kPacket);
		break;
	case SSTPK_UPDATE_TIME_CASH:
		OnUpdateTimeCashInfo(kPacket);
		break;
	case SSTPK_TITLE_MOVE_RESULT:
		OnMoveTitleData(kPacket);
		break;
	case SSTPK_TITLE_UPDATE:
		OnTitleUpdate(kPacket);
		break;
	case SSTPK_BONUS_CASH_MOVE_RESULT:
		OnMoveBonusCashData(kPacket);
		break;
	case SSTPK_BONUS_CAHSH_ADD:
		OnResultBonusCashAdd(kPacket);
		break;
	case SSTPK_BONUS_CASH_UPDATE:
		OnResultBonusCashUpdate(kPacket);
		break;
	default:
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::PacketParsing �˼����� ��Ŷ : 0x%x", kPacket.GetPacketID() );
		break;
	}
}

bool ServerNode::OnConnectWorkingPacket( SP2Packet &rkPacket )
{
	if( m_bConnectWorkComplete )			return false;
	if( IsLoginNode() || IsRelayNode() )	return false;

	//���� ���� ����ȭ�� �޾Ƽ� ó���ؾ��� ��Ŷ
	switch( rkPacket.GetPacketID() )
	{
	case SSTPK_CONNECT_INFO:
	case SSTPK_CONNECT_SYNC:
	case SSTPK_PING:
	case LSPTK_CONNECT_REQUEST:
	case RSPTK_ON_CONNECT:
		return false;
	}

	//���� ���� ����ȭ�� �޴� ��Ŷ�� ����ȭ ������ ó��
	m_ConnectWorkingPacket.push_back( rkPacket );
	return true;
}

bool ServerNode::OnUserTransferTCP( SP2Packet &rkPacket )
{
	/* ���߰� */
	if( COMPARE( rkPacket.GetPacketID(), STPK_RESERVE_LOGOUT, STPK_ABSTRACT + 1 ) )
	{
		DWORD dwUserIndex;
		rkPacket >> dwUserIndex;
		User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
		if( pUser )
		{
			// ���� �ε��� ����
			SP2Packet kPacket( rkPacket.GetPacketID() );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
			pUser->SendMessage( kPacket );
		}
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserTransferTCP Not Node PacketID: 0x%x", rkPacket.GetPacketID() );
		return true;
	}	
	else if( COMPARE( rkPacket.GetPacketID(), CUPK_CONNECT, 0x5000 ) )
	{
		DWORD dwUserIndex;
		rkPacket >> dwUserIndex;
		User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
		if( pUser )
		{
			// ���� �ε��� ����
			SP2Packet kPacket( rkPacket.GetPacketID() );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + sizeof( DWORD ), rkPacket.GetDataSize() - sizeof( DWORD ) );
			pUser->SendMessage( kPacket );
		}
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserTransferTCP Not Node PacketID: 0x%x", rkPacket.GetPacketID() );
		return true;
	}	
	return false;
}

void ServerNode::OnClose( SP2Packet &rkPacket )
{
	if(IsRelayNode())
	{
		g_Relay.DelRelayServerInfo(this);
		g_UserNodeManager.SendNotUseRelayMessageAll(m_iRelayIndex);
	}
	if( !IsDisconnectState() )
	{
		OnDestroy();
		OnSessionDestroy();
		
		g_ServerNodeManager.RemoveNode( this );
	}
}

void ServerNode::OnConnectInfo( SP2Packet &rkPacket )
{
	rkPacket >> m_dwServerIndex >> m_szServerIP >> m_szClientMoveIP >> m_iServerPort >> m_iClientPort;
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]OnConnect :[%d] [%s] [%s] [%d] [%d] [%d]", m_dwServerIndex, m_szServerIP.c_str(), m_szClientMoveIP.c_str(), m_iServerPort, m_iClientPort, GetNodeRole() );
	
	if( GetNodeRole() == NODE_ROLE_CLIENT )
	{
		//Information( "Ŭ���̾�Ʈ ��� %s:%d\r\n", m_szServerIP.c_str(), m_iServerPort );
		SetNodeType( NODE_TYPE_GAME );
		RequestJoin();
		AfterJoinProcess();
	}	
	else if( GetNodeRole() == NODE_ROLE_SERVER )
	{
		//Information( "���� ��� %s:%d\r\n", m_szServerIP.c_str(), m_iServerPort );
		SetNodeType( NODE_TYPE_GAME );
		RequestSync();
		AfterJoinProcess();

		m_bConnectWorkComplete = true;
	}
	
	m_dwPingTIme = TIMEGETTIME();	
}

void ServerNode::OnConnectSync( SP2Packet &rkPacket )
{
	int iSyncType;
	rkPacket >> iSyncType;
	switch( iSyncType )
	{
	case SSTPK_CONNECT_SYNC_USER:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				DWORD dwUserIndex;
				rkPacket >> dwUserIndex;
				UserCopyNode *pNewNode = CreateNewUser( dwUserIndex );
				if( pNewNode )
				{
					pNewNode->ApplySyncCreate( rkPacket );
					g_UserNodeManager.AddCopyUser( pNewNode );
				}
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ gameServer UserInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_ROOM:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				int iRoomIndex;
				rkPacket >> iRoomIndex;
				RoomCopyNode *pNewNode = CreateNewRoom( iRoomIndex );
				if( pNewNode )
				{
					pNewNode->ApplySyncCreate( rkPacket );
					g_RoomNodeManager.AddCopyRoom( pNewNode );
				}
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ gameServer RoomInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_BATTLEROOM:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				int iBattleRoomIndex;
				rkPacket >> iBattleRoomIndex;
				BattleRoomCopyNode *pNewNode = CreateNewBattleRoom( iBattleRoomIndex );
				if( pNewNode )
				{
					pNewNode->ApplySyncCreate( rkPacket );
					g_BattleRoomManager.AddCopyBattleRoom( pNewNode );
				}
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ gameServer BattleRoomInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_CHANNEL:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				DWORD dwChannelIndex;
				rkPacket >> dwChannelIndex;
				ChannelCopyNode *pNewNode = CreateNewChannel( dwChannelIndex );
				if( pNewNode )
				{
					g_ChannelNodeManager.AddCopyChannel( pNewNode );
				}
			}			
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ GameServer ChannelInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_LADDERTEAM:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				DWORD dwLadderTeamIndex;
				rkPacket >> dwLadderTeamIndex;
				LadderTeamCopyNode *pNewNode = CreateNewLadderTeam( dwLadderTeamIndex );
				if( pNewNode )
				{
					pNewNode->ApplySyncCreate( rkPacket );
					g_LadderTeamManager.AddCopyLadderTeam( pNewNode );
				}
			}
			g_LadderTeamManager.SortLadderTeamRank( true );
			g_LadderTeamManager.SortLadderTeamRank( false );
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ GameServer LadderTeamInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_SHUFFLEROOM:
		{
			int iSize;
			rkPacket >> iSize;
			for(int i = 0;i < iSize;i++)
			{
				int iShuffleRoomIndex;
				rkPacket >> iShuffleRoomIndex;
				ShuffleRoomCopyNode *pNewNode = CreateNewShuffleRoom( iShuffleRoomIndex );
				if( pNewNode )
				{
					pNewNode->ApplySyncCreate( rkPacket );
					g_ShuffleRoomManager.AddCopyShuffleRoom( pNewNode );
				}
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ GameServer ShuffleRoomInfo : [%d] [%d]", m_dwServerIndex, iSize );
		}
		break;
	case SSTPK_CONNECT_SYNC_COMPLETE:
		{
			// ����ȭ �Ϸ�
			m_bConnectWorkComplete = true;
			g_ServerNodeManager.IsConnectWorkComplete();

			// ����ȭ�� ���� ��Ŷ ó��
			if( !m_ConnectWorkingPacket.empty() )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][server]Differ GameServer Connect Sync Complete PacketParsing : [%d] [%d]", m_dwServerIndex, (int)m_ConnectWorkingPacket.size() );
				SP2PacketList::iterator iter;
				for( iter=m_ConnectWorkingPacket.begin() ; iter!=m_ConnectWorkingPacket.end() ; ++iter )
				{
					PacketParsing( *iter );
				}
				m_ConnectWorkingPacket.clear();
			}
		}
		break;
	}
}

void ServerNode::OnPing( SP2Packet &rkPacket )
{
	m_dwRecvPingTime = TIMEGETTIME();
}

void ServerNode::OnRoomSync( SP2Packet &rkPacket )
{
	int   iSyncType, iRoomIndex;
	rkPacket >> iSyncType >> iRoomIndex;

	switch( iSyncType )
	{
	case RoomSync::RS_MODE:
		{
			RoomCopyNode *pCopyNode = GetRoomNode( iRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncMode( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnRoomSync Not Room : %d - %d - %s:%d", (int)RoomSync::RS_MODE, iRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case RoomSync::RS_CURUSER:
		{
			RoomCopyNode *pCopyNode = GetRoomNode( iRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncCurUser( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnRoomSync Not Room : %d - %d - %s:%d", (int)RoomSync::RS_CURUSER, iRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case RoomSync::RS_PLAZAINFO:
		{
			RoomCopyNode *pCopyNode = GetRoomNode( iRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncPlazaInfo( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnRoomSync Not Room : %d - %d - %s:%d", (int)RoomSync::RS_PLAZAINFO, iRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case RoomSync::RS_CREATE:
		{
			RoomCopyNode *pNewNode = CreateNewRoom( iRoomIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCreate( rkPacket );
				g_RoomNodeManager.AddCopyRoom( pNewNode );
			}
		}
		break;
	case RoomSync::RS_DESTROY:
		RemoveRoomNode( iRoomIndex );
		break; 
	}
}

void ServerNode::OnMovingRoom( SP2Packet &rkPacket )
{
	int iMoveType, iRoomIndex, iUserIndex;
	rkPacket >> iMoveType >> iRoomIndex >> iUserIndex;

	switch( iMoveType )
	{
	case SS_MOVING_ROOM_JOIN_PLAZA:
		{ 
			int iMatchingLevel;
			rkPacket >> iMatchingLevel;
			Room *pRoom = NULL;
			if( iRoomIndex == -1 )
				pRoom = g_RoomNodeManager.GetJoinPlazaNode( iMatchingLevel, NULL );
			else
				pRoom = g_RoomNodeManager.GetJoinPlazaNodeByNum( iRoomIndex );

			if( pRoom )
			{
				if( pRoom->IsRoomFull() )       
				{
					// �ο� ����
					SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
					kPacket << SS_MOVING_ROOM_ERROR << iMoveType << SEARCH_TRAINING_ERROR_3 << iRoomIndex << iUserIndex;
					SendMessage( kPacket );					
				}
				else
				{
					// �ӽ� ������ �Ǹ� JoinRoom �����Ͽ� ���� �̵��� �����Ѵ�.
					pRoom->EnterReserveUser( iUserIndex );
					SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
					kPacket << SS_MOVING_ROOM_JOIN << iMoveType << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << iUserIndex;	
					kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum() << (int)pRoom->GetPlazaModeType() << pRoom->GetRoomNumber();
					SendMessage( kPacket );
				}
			}
			else
			{
				// �� ��������
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_ERROR << iMoveType << SEARCH_TRAINING_ERROR_4 << iRoomIndex << iUserIndex;
				SendMessage( kPacket );
			}
		}
		break;
	case SS_MOVING_ROOM_EXIT_PLAZA:
		{
			int iNewResult;
			bool bPenalty;
			rkPacket >> iNewResult >> bPenalty;

			Room *pRoom = g_RoomNodeManager.GetJoinPlazaNodeByNum( iRoomIndex );			
			if( pRoom && !pRoom->IsRoomFull() )
			{
				// �ӽ� ������ �Ǹ� JoinRoom �����Ͽ� ���� �̵��� �����Ѵ�.
				pRoom->EnterReserveUser( iUserIndex );
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_JOIN << iMoveType << iNewResult << bPenalty << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << iUserIndex;	
				kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum() << (int)pRoom->GetPlazaModeType() << pRoom->GetRoomNumber();
				SendMessage( kPacket );
			}
			else
			{
				// �� ��������
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_ERROR << iMoveType << iNewResult << bPenalty << iUserIndex;
				SendMessage( kPacket );
			}
		}
		break;
	case SS_MOVING_ROOM_JOIN_HEADQUARTERS:
		{ 
			bool bInvited;
			int iMapIndex;
			DWORD dwOwnerUser;
			rkPacket >> dwOwnerUser >> iMapIndex >> bInvited;
			RoomParent *pRoomParent = g_RoomNodeManager.GetHeadquartersGlobalNode( iRoomIndex );
			if( pRoomParent )
			{				
				if( pRoomParent->IsRoomFull() )       
				{
					// �ο� ����
					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pUserParent )
					{
						SP2Packet kPacket( STPK_JOIN_HEADQUARTERS );
						kPacket << JOIN_HEADQUARTERS_ROOM_FULL;
						pUserParent->RelayPacket( kPacket );
					}
				}
				else if( pRoomParent->IsRoomOriginal() )
				{
					Room *pRoom = static_cast< Room * >( pRoomParent );

					// ����
					UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pRequestUser == NULL )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovingRoom - %d - pRequestUser NULL(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
					}
					else if( pRequestUser->IsUserOriginal() )
					{				
						User *pUser = static_cast< User * >( pRequestUser );

						SP2Packet kPacket( STPK_JOIN_ROOM );   
						kPacket << JOIN_ROOM_OK;
						kPacket << pRoom->GetModeType();
						kPacket << pRoom->GetModeSubNum();
						kPacket << pRoom->GetModeMapNum();
						kPacket << pRoom->GetRoomNumber();
						kPacket << (int)pRoom->GetPlazaModeType();
						pUser->SendMessage( kPacket );
						pUser->EnterRoom( pRoom );
					}
					else
					{
						pRoom->EnterReserveUser( pRequestUser->GetUserIndex() );

						// �ش� �������� ���� �˸�
						UserCopyNode *pUser = static_cast< UserCopyNode * >( pRequestUser );

						SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
						kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_HEADQUARTERS << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << pUser->GetUserIndex();	
						kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum();
						pUser->SendMessage( kPacket );
					}
				}
				else
				{
					// ���� ���̴� ���� ó��
					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pUserParent )
					{
						SP2Packet kPacket( STPK_JOIN_HEADQUARTERS );
						kPacket << JOIN_HEADQUARTERS_EXCEPTION;
						pUserParent->RelayPacket( kPacket );
					}
				}
			}
			else
			{
				// �� ��������
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_ERROR << iMoveType << iRoomIndex << iUserIndex << dwOwnerUser << iMapIndex << bInvited;
				SendMessage( kPacket );
			}
		}
		break;
	case SS_MOVING_ROOM_JOIN_HEADQUARTERS_AGREE:
		{ 
			RoomParent *pRoomParent = g_RoomNodeManager.GetHeadquartersGlobalNode( iRoomIndex );
			if( pRoomParent )
			{				
				if( pRoomParent->IsRoomFull() )       
				{
					// �ο� ����
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_HEADQUARTERS_AGREE Full(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
				}
				else if( pRoomParent->IsRoomOriginal() )
				{
					Room *pRoom = static_cast< Room * >( pRoomParent );

					// ����
					UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pRequestUser == NULL )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovingRoom - %d - pRequestUser NULL(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
					}
					else if( pRequestUser->IsUserOriginal() )
					{				
						User *pUser = static_cast< User * >( pRequestUser );

						SP2Packet kPacket( STPK_JOIN_ROOM );   
						kPacket << JOIN_ROOM_OK;
						kPacket << pRoom->GetModeType();
						kPacket << pRoom->GetModeSubNum();
						kPacket << pRoom->GetModeMapNum();
						kPacket << pRoom->GetRoomNumber();
						kPacket << (int)pRoom->GetPlazaModeType();
						pUser->SendMessage( kPacket );
						pUser->EnterRoom( pRoom );
					}
					else
					{
						pRoom->EnterReserveUser( pRequestUser->GetUserIndex() );

						// �ش� �������� ���� �˸�
						UserCopyNode *pUser = static_cast< UserCopyNode * >( pRequestUser );

						SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
						kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_HEADQUARTERS << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << pUser->GetUserIndex();	
						kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum();
						pUser->SendMessage( kPacket );
					}
				}
				else
				{					
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_HEADQUARTERS_AGREE -6.", iUserIndex );
				}
			}
			else
			{
				// �� ��������
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_HEADQUARTERS_AGREE.", iUserIndex );
			}
		}
		break;
	case SS_MOVING_ROOM_JOIN_PERSONAL_HQ:
		{ 
			bool bInvited		= false;
			int iMapIndex		= 0;
			DWORD dwOwnerUser	= 0;

			PACKET_GUARD_VOID( rkPacket.Read(dwOwnerUser) );
			PACKET_GUARD_VOID( rkPacket.Read(iMapIndex) );
			PACKET_GUARD_VOID( rkPacket.Read(bInvited) );

			RoomParent *pRoomParent = g_RoomNodeManager.GetHeadquartersGlobalNode( iRoomIndex );
			if( pRoomParent )
			{				
				if( pRoomParent->IsRoomFull() )       
				{
					// �ο� ����
					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pUserParent )
					{
						SP2Packet kPacket( STPK_JOIN_PERSONAL_HQ );
						PACKET_GUARD_VOID( kPacket.Write(JOIN_PERSONAL_HQ_ROOM_FULL) );
						pUserParent->RelayPacket( kPacket );
					}
				}
				else if( pRoomParent->IsRoomOriginal() )
				{
					Room *pRoom = static_cast< Room * >( pRoomParent );

					// ����
					UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pRequestUser == NULL )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovingRoom - %d - pRequestUser NULL(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
					}
					else if( pRequestUser->IsUserOriginal() )
					{				
						User *pUser = static_cast< User * >( pRequestUser );

						SP2Packet kPacket( STPK_JOIN_ROOM );   
						kPacket << JOIN_ROOM_OK;
						kPacket << pRoom->GetModeType();
						kPacket << pRoom->GetModeSubNum();
						kPacket << pRoom->GetModeMapNum();
						kPacket << pRoom->GetRoomNumber();
						kPacket << (int)pRoom->GetPlazaModeType();
						pUser->SendMessage( kPacket );
						pUser->EnterRoom( pRoom );
					}
					else
					{
						pRoom->EnterReserveUser( pRequestUser->GetUserIndex() );

						// �ش� �������� ���� �˸�
						UserCopyNode *pUser = static_cast< UserCopyNode * >( pRequestUser );

						SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
						kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_PERSONAL_HQ << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << pUser->GetUserIndex();	
						kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum();
						pUser->SendMessage( kPacket );
					}
				}
				else
				{
					// ���� ���̴� ���� ó��
					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pUserParent )
					{
						SP2Packet kPacket( STPK_JOIN_PERSONAL_HQ );
						kPacket << JOIN_PERSONAL_HQ_EXCEPTION;
						pUserParent->RelayPacket( kPacket );
					}
				}
			}
			else
			{
				// �� ��������
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_ERROR << iMoveType << iRoomIndex << iUserIndex << dwOwnerUser << iMapIndex << bInvited;
				SendMessage( kPacket );
			}
		}
		break;
	case SS_MOVING_ROOM_JOIN_PERSONAL_HQ_AGREE:
		{ 
			RoomParent *pRoomParent = g_RoomNodeManager.GetHeadquartersGlobalNode( iRoomIndex );
			if( pRoomParent )
			{				
				if( pRoomParent->IsRoomFull() )       
				{
					// �ο� ����
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_PERSONAL_HQ_AGREE Full(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
				}
				else if( pRoomParent->IsRoomOriginal() )
				{
					Room *pRoom = static_cast< Room * >( pRoomParent );

					// ����
					UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( iUserIndex );
					if( pRequestUser == NULL )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovingRoom - %d - pRequestUser NULL(%d).", pRoomParent->GetRoomIndex(), iUserIndex );
					}
					else if( pRequestUser->IsUserOriginal() )
					{				
						User *pUser = static_cast< User * >( pRequestUser );

						SP2Packet kPacket( STPK_JOIN_ROOM );   
						kPacket << JOIN_ROOM_OK;
						kPacket << pRoom->GetModeType();
						kPacket << pRoom->GetModeSubNum();
						kPacket << pRoom->GetModeMapNum();
						kPacket << pRoom->GetRoomNumber();
						kPacket << (int)pRoom->GetPlazaModeType();
						pUser->SendMessage( kPacket );
						pUser->EnterRoom( pRoom );
					}
					else
					{
						pRoom->EnterReserveUser( pRequestUser->GetUserIndex() );

						// �ش� �������� ���� �˸�
						UserCopyNode *pUser = static_cast< UserCopyNode * >( pRequestUser );

						SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
						kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_PERSONAL_HQ << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << pUser->GetUserIndex();	
						kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum();
						pUser->SendMessage( kPacket );
					}
				}
				else
				{					
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_PERSONAL_HQ_AGREE -6.", iUserIndex );
				}
			}
			else
			{
				// �� ��������
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnMovingRoom - %d - SS_MOVING_ROOM_JOIN_PERSONAL_HQ_AGREE.", iUserIndex );
			}
		}
		break;
	default:
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovingRoom �˼����� ��û : %d", iMoveType );
		}
		break;
	}
}

void ServerNode::_OnMovingRoomOK( int iMoveType, SP2Packet &rkPacket )
{
	switch( iMoveType )
	{
	case SS_MOVING_ROOM_JOIN_PLAZA:
		{
			int iModeType, iRoomIndex, iUserIndex;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				if( pUser->GetCharCount() == 0 )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�뺴�� ���� ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_PLAZA : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_NOT_CHAR, false );
				}
				else if( pUser->GetEntryType() == ET_TERMINATION )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�ӽð��� ���� ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_PLAZA : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_ENTRY_NOT_FORMALITY, false );
				}
				else if( pUser->IsNewPublicID() )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�̸����� ������ ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_PLAZA : %d|%s", pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_RESERVED_CHANGE_ID, false );
				}
				else
				{

#ifdef SRC_LATIN
					// ���� �̵� ������ ���� �� Ŭ���̾�Ʈ CHCEnd() ȣ�� ���� G �޽��� ����.	JCLEE 140310
					LOG.PrintTimeAndLog( 0, "Apex G Msg Send, ServerNode::_OnMovingRoomOK(), SS_MOVING_ROOM_JOIN_PLAZA" );
					g_ioApex.NoticeApexProxy_UserLogout( pUser->GetUserIndex(), pUser->GetPrivateID() );
					
#endif

					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMovingRoomOK Plaza : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetClientMoveIP().c_str(), g_App.GetCSPort(), 
											GetServerIP().c_str(), GetClientPort() );
					int iModeSubNum, iModeMapNum, iRoomNumber, iPlazaType;
					rkPacket >> iModeSubNum >> iModeMapNum >> iPlazaType >> iRoomNumber;

					int iMovingValue = pUser->SetServerMoving();
					SP2Packet kPacket( STPK_MOVING_SERVER );   
					kPacket << iModeType << iModeSubNum << iModeMapNum << iPlazaType << iRoomNumber << iRoomIndex;
					kPacket << GetClientMoveIP() << GetClientPort();
					kPacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
					pUser->SendMessage( kPacket );			
				}				
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_PLAZA: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_EXIT_PLAZA:
		{
			bool bPenalty;
			int iNewResult, iModeType, iRoomIndex, iUserIndex;
			rkPacket >> iNewResult >> bPenalty >> iModeType >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				if( pUser->GetCharCount() == 0 )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�뺴�� ���� ���η� �̵���Ŵ SS_MOVING_ROOM_EXIT_PLAZA : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_NOT_CHAR, false );
				}
				else if( pUser->GetEntryType() == ET_TERMINATION )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�ӽð��� ���� ������ ���η� �̵���Ŵ SS_MOVING_ROOM_EXIT_PLAZA : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_ENTRY_NOT_FORMALITY, false );
				}
				else if( pUser->IsNewPublicID() )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�̸����� ������ ������ ���η� �̵���Ŵ SS_MOVING_ROOM_EXIT_PLAZA : %d|%s", pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_RESERVED_CHANGE_ID, false );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMovingRoomOK  Plaza : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetClientMoveIP().c_str(), g_App.GetCSPort(), 
											GetServerIP().c_str(), GetClientPort() );
					int iModeSubNum, iModeMapNum, iRoomNumber, iPlazaType;
					rkPacket >> iModeSubNum >> iModeMapNum >> iPlazaType >> iRoomNumber;

					SP2Packet kPacket1( STPK_EXIT_ROOM );
					kPacket1 << iNewResult << iModeSubNum << bPenalty << iRoomNumber << iModeMapNum << iPlazaType;
					pUser->SendMessage( kPacket1 );

#ifdef SRC_LATIN
					// ���� �̵� ������ ���� �� Ŭ���̾�Ʈ CHCEnd() ȣ�� ���� G �޽��� ����.	JCLEE 140310
					LOG.PrintTimeAndLog( 0, "Apex G Msg Send, ServerNode::_OnMovingRoomOK(), SS_MOVING_ROOM_EXIT_PLAZA" );
					g_ioApex.NoticeApexProxy_UserLogout( pUser->GetUserIndex(), pUser->GetPrivateID() );
				
#endif

					int iMovingValue = pUser->SetServerMoving();
					SP2Packet kPacket2( STPK_MOVING_SERVER );   
					kPacket2 << iModeType << iModeSubNum << iModeMapNum << iPlazaType << iRoomNumber << iRoomIndex << GetClientMoveIP() << GetClientPort();
					kPacket2 << iMovingValue << g_ServerNodeManager.GetServerIndex();
					pUser->SendMessage( kPacket2 );			
				}				
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_EXIT_PLAZA: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_BATTLE:
		{
			int iModeType, iRoomIndex, iUserIndex;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				if( pUser->GetCharCount() == 0 )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�뺴�� ���� ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_BATTLE : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_NOT_CHAR, false );
				}
				else if( pUser->GetEntryType() == ET_TERMINATION )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�ӽð��� ���� ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_BATTLE : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_ENTRY_NOT_FORMALITY, false );
				}
				else if( pUser->IsNewPublicID() )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�̸����� ������ ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_BATTLE : %d|%s", pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_RESERVED_CHANGE_ID, false );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMovingRoomOK Battle : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetClientMoveIP().c_str(), g_App.GetCSPort(), 
											GetServerIP().c_str(), GetClientPort() );

					int iModeSubNum, iModeMapNum, iRoomNumber, iPlazaType;
					rkPacket >> iModeSubNum >>  iModeMapNum >> iPlazaType >> iRoomNumber;

					{
						// ���� �ε� ���� ����
						SP2Packet kPacket( STPK_BATTLEROOM_COMMAND );
						kPacket << BATTLEROOM_READY_GO_OK << iModeType << iModeSubNum << iModeMapNum;
						pUser->SendMessage( kPacket );
					}
#ifdef SRC_LATIN
					// ���� �̵� ������ ���� �� Ŭ���̾�Ʈ CHCEnd() ȣ�� ���� G �޽��� ����.	JCLEE 140310
					LOG.PrintTimeAndLog( 0, "Apex G Msg Send, ServerNode::_OnMovingRoomOK(), SS_MOVING_ROOM_JOIN_BATTLE" );
					g_ioApex.NoticeApexProxy_UserLogout( pUser->GetUserIndex(), pUser->GetPrivateID() );
				
#endif
					// ���� �̵� ��ȣ ���� 
					int iMovingValue = pUser->SetServerMoving();
					SP2Packet kMovePacket( STPK_MOVING_SERVER );   
					kMovePacket << iModeType << iModeSubNum << iModeMapNum << iPlazaType << iRoomNumber << iRoomIndex;
					kMovePacket << GetClientMoveIP() << GetClientPort();
					kMovePacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
					pUser->SendMessage( kMovePacket );			
				}
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_BATTLE: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_LADDER:
		{
			int iModeType, iRoomIndex, iUserIndex;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				if( pUser->GetCharCount() == 0 )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�뺴�� ���� ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_LADDER : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_NOT_CHAR, false );
				}
				else if( pUser->GetEntryType() == ET_TERMINATION )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�ӽð��Ը��� ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_LADDER : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_ENTRY_NOT_FORMALITY, false );
				}
				else if( pUser->IsNewPublicID() )
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "�̸����� ������ ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_LADDER : %d|%s", pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_RESERVED_CHANGE_ID, false );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMovingRoomOK LadderBattle : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetClientMoveIP().c_str(), g_App.GetCSPort(), 
											GetServerIP().c_str(), GetClientPort() );

					bool bReadyGoSend = false;
					int iModeSubNum = 0;
					int iModeMapNum = 0;
					int iRoomNumber = 0;
					int iPlazaType = 0;
					int iCampType = 0;
					int iTeamType = 0;

					PACKET_GUARD_VOID( rkPacket.Read(iModeSubNum) );
					PACKET_GUARD_VOID( rkPacket.Read(iModeMapNum) );
					PACKET_GUARD_VOID( rkPacket.Read(iPlazaType) );
					PACKET_GUARD_VOID( rkPacket.Read(iRoomNumber) );
					PACKET_GUARD_VOID( rkPacket.Read(bReadyGoSend) );
					PACKET_GUARD_VOID( rkPacket.Read(iCampType) );
					PACKET_GUARD_VOID( rkPacket.Read(iTeamType) );
					
					if( bReadyGoSend )
					{
						// ���� �ε� ���� ����
						SP2Packet kPacket( STPK_LADDERTEAM_MACRO );
						PACKET_GUARD_VOID( kPacket.Write(LADDERTEAM_MACRO_MODE_READY_GO) );
						PACKET_GUARD_VOID( kPacket.Write(iModeType) );
						PACKET_GUARD_VOID( kPacket.Write(iModeSubNum) );
						PACKET_GUARD_VOID( kPacket.Write(iModeMapNum) );
						PACKET_GUARD_VOID( kPacket.Write(iCampType) );
						PACKET_GUARD_VOID( kPacket.Write(iTeamType) );

						pUser->SendMessage( kPacket );
					}
#ifdef SRC_LATIN
					// ���� �̵� ������ ���� �� Ŭ���̾�Ʈ CHCEnd() ȣ�� ���� G �޽��� ����.	JCLEE 140310
					LOG.PrintTimeAndLog( 0, "Apex G Msg Send, ServerNode::_OnMovingRoomOK(), SS_MOVING_ROOM_JOIN_LADDER" );
					g_ioApex.NoticeApexProxy_UserLogout( pUser->GetUserIndex(), pUser->GetPrivateID() );
				
#endif

					// ���� �̵� ��ȣ ���� 
					int iMovingValue = pUser->SetServerMoving();
					SP2Packet kMovePacket( STPK_MOVING_SERVER );   
					kMovePacket << iModeType << iModeSubNum << iModeMapNum << iPlazaType << iRoomNumber << iRoomIndex;
					kMovePacket << GetClientMoveIP() << GetClientPort();
					kMovePacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
					pUser->SendMessage( kMovePacket );			
				}
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_LADDER: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_HEADQUARTERS:
		{
			int iModeType, iRoomIndex, iUserIndex, iModeSubNum, iModeMapNum;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex >> iModeSubNum >> iModeMapNum;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
#ifdef SRC_LATIN
				// ���� �̵� ������ ���� �� Ŭ���̾�Ʈ CHCEnd() ȣ�� ���� G �޽��� ����.	JCLEE 140310
				LOG.PrintTimeAndLog( 0, "Apex G Msg Send, ServerNode::_OnMovingRoomOK(), SS_MOVING_ROOM_JOIN_HEADQUARTERS" );
				g_ioApex.NoticeApexProxy_UserLogout( pUser->GetUserIndex(), pUser->GetPrivateID() );
			
#endif

				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "_OnMovingRoomOK Headquarters : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetPublicIP().c_str(), g_App.GetCSPort(), 
																					          GetServerIP().c_str(), GetClientPort() );
				int iMovingValue = pUser->SetServerMoving();
				SP2Packet kPacket( STPK_MOVING_SERVER );   
				kPacket << iModeType << iModeSubNum << iModeMapNum << 0 << 0 << iRoomIndex;
				kPacket << GetClientMoveIP() << GetClientPort();
				kPacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
				pUser->SendMessage( kPacket );
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_HEADQUARTERS: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_SHUFFLE:
		{
			int iModeType, iRoomIndex, iUserIndex;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				if( pUser->GetCharCount() == 0 )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�뺴�� ���� ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_SHUFFLE : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_NOT_CHAR, false );
				}
				else if( pUser->GetEntryType() == ET_TERMINATION )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�ӽð��� ���� ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_SHUFFLE : %s", pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_ENTRY_NOT_FORMALITY, false );
				}
				else if( pUser->IsNewPublicID() )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "�̸����� ������ ������ ���η� �̵���Ŵ SS_MOVING_ROOM_JOIN_SHUFFLE : %d|%s", pUser->GetUserIndex(), pUser->GetPublicID().c_str() );
					pUser->ExitRoomToTraining( EXIT_ROOM_RESERVED_CHANGE_ID, false );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Shuffle : %s, %s:%d - > %s:%d", pUser->GetPublicID().c_str(), g_App.GetClientMoveIP().c_str(), g_App.GetCSPort(), GetServerIP().c_str(), GetClientPort() );

					int iModeSubNum, iModeMapNum, iRoomNumber, iPlazaType;
					rkPacket >> iModeSubNum >>  iModeMapNum >> iPlazaType >> iRoomNumber;

					{
						// ���� �ε� ���� ����
						SP2Packet kPacket( STPK_SHUFFLEROOM_COMMAND );
						kPacket << SHUFFLEROOM_READY_GO_OK << iModeType << iModeSubNum << iModeMapNum;
						pUser->SendMessage( kPacket );
					}
					// ���� �̵� ��ȣ ���� 
					int iMovingValue = pUser->SetServerMoving();
					SP2Packet kMovePacket( STPK_MOVING_SERVER );   
					kMovePacket << iModeType << iModeSubNum << iModeMapNum << iPlazaType << iRoomNumber << iRoomIndex;
					kMovePacket << GetClientMoveIP() << GetClientPort();
					kMovePacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
					pUser->SendMessage( kMovePacket );
				}
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_SHUFFLE: %d", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_PERSONAL_HQ:
		{
			int iModeType	= 0, iRoomIndex	= 0, iUserIndex	= 0, iModeSubNum	= 0, iModeMapNum	= 0;
			rkPacket >> iModeType >> iRoomIndex >> iUserIndex >> iModeSubNum >> iModeMapNum;

			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				int iMovingValue = pUser->SetServerMoving();
				SP2Packet kPacket( STPK_MOVING_SERVER );   
				kPacket << iModeType << iModeSubNum << iModeMapNum << 0 << 0 << iRoomIndex;
				kPacket << GetClientMoveIP() << GetClientPort();
				kPacket << iMovingValue << g_ServerNodeManager.GetServerIndex();
				pUser->SendMessage( kPacket );
			}		
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomOK Not User SS_MOVING_ROOM_JOIN_HEADQUARTERS: %d", iUserIndex );
		}
		break;
	}        
}

void ServerNode::_OnMovingRoomError( int iMoveType, SP2Packet &rkPacket )
{
	switch( iMoveType )
	{
	case SS_MOVING_ROOM_JOIN_PLAZA:
		{
			int iResult, iRoomIndex, iUserIndex;
			rkPacket >> iResult >> iRoomIndex >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				SP2Packet kPacket( STPK_SEARCH_PLAZA_ROOM );
				kPacket << iResult;
				pUser->SendMessage( kPacket );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomError SS_MOVING_ROOM_JOIN_PLAZA :%d Not User", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_EXIT_PLAZA:
		{
			int  iNewResult, iUserIndex;
			bool bPenalty;
			rkPacket >> iNewResult >> bPenalty >> iUserIndex;
			User *pUser = GetUserOriginalNode( iUserIndex, rkPacket );
			if( pUser )
			{
				// �� ��Ż �� ���(���Ƽ/����)���� ������ �־�����Ѵ�.
				Room *pRoom = g_RoomNodeManager.GetExitRoomJoinPlazaNode( pUser->GetKillDeathLevel() );
				if( pRoom )
				{
					SP2Packet kPacket( STPK_EXIT_ROOM );
					kPacket << iNewResult;
					kPacket << pRoom->GetModeSubNum();
					kPacket << bPenalty;
					kPacket << pRoom->GetRoomNumber();
					pUser->SendMessage( kPacket );
					pUser->EnterRoom( pRoom );
				}
				else      //����
				{
					pUser->ExitRoomToTraining( EXIT_ROOM_LOBBY, false );
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "_OnMovingRoomError SS_MOVING_ROOM_EXIT_PLAZA :%d Not User", iUserIndex );
		}
		break;
	case SS_MOVING_ROOM_JOIN_HEADQUARTERS:
		{
			bool bInvited;
			DWORD dwRequestUser, dwOwnerUser;
			int iRequestRoomIndex, iMapIndex;
			rkPacket >> iRequestRoomIndex >> dwRequestUser >> dwOwnerUser >> iMapIndex >> bInvited;

			RoomParent *pRoomParent = g_RoomNodeManager.GetHeadquartersGlobalNode( iRequestRoomIndex );
			if( pRoomParent )
			{
				// -_-; ���� ���ٰ��ؼ� �Դµ� ����� �ִܴ�....���� ó��
				UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
				if( pUserParent )
				{
					SP2Packet kPacket( STPK_JOIN_HEADQUARTERS );
					kPacket << JOIN_HEADQUARTERS_EXCEPTION;
					pUserParent->RelayPacket( kPacket );
				}
			}
			else
			{
				UserParent *pRequestParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
				if( pRequestParent == NULL )
					return;

				UserParent *pOwnerParent = g_UserNodeManager.GetGlobalUserNode( dwOwnerUser );
				if( pOwnerParent == NULL )
				{
					// ���ʰ� ��������
					SP2Packet kPacket( STPK_JOIN_HEADQUARTERS );
					kPacket << JOIN_HEADQUARTERS_OWNER_OFFLINE;
					pRequestParent->RelayPacket( kPacket );
				}
				else if( pOwnerParent->IsUserOriginal() )
				{
					User *pUser = static_cast< User * >( pOwnerParent );
					pUser->_OnJoinHeadquarters( pRequestParent, iMapIndex, bInvited );
				}
				else
				{
					UserCopyNode *pUser = static_cast< UserCopyNode * >( pOwnerParent );

					SP2Packet kPacket( SSTPK_JOIN_HEADQUARTERS_USER );
					kPacket << pRequestParent->GetUserIndex() << pUser->GetUserIndex() << iMapIndex << bInvited;
					pUser->SendMessage( kPacket );
				}
			}
		}
		break;
	}
}

void ServerNode::OnMovingRoomResult( SP2Packet &rkPacket )
{
	int iMovingResult, iMoveType;
	rkPacket >> iMovingResult >> iMoveType;
	switch( iMovingResult )
	{
	case SS_MOVING_ROOM_ERROR:
		_OnMovingRoomError( iMoveType, rkPacket );
		break;
	case SS_MOVING_ROOM_JOIN:
		_OnMovingRoomOK( iMoveType, rkPacket );
		break;
	}
}

void ServerNode::SendDecoMoveData( int iMovingValue, User *pUser   )
{
	int iStartIndex = 0;
	int iPageCount = pUser->GetDecoPageCount();

	if( iPageCount > 0 )
	{
		ioInventory* pInventory = pUser->GetInventory();

		if( !pInventory )
			return;

		int iAddIndex = DB_DECO_SELECT_COUNT;

		for( int i = 0; i < iPageCount; i++ )
		{
			SP2Packet kPacket( SSTPK_INVENTORY_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );

			pUser->FillInventoryMoveData( kPacket, iStartIndex );
			SendMessage( kPacket );

			iStartIndex += iAddIndex; 
		}
	}

}

void ServerNode::SendExtraItemMoveData( int iMovingValue, User *pUser )
{
	int iStartRow = 0;
	int iPageCount = pUser->GetExtraItemPageCount();

	if( iPageCount > 0 )
	{
		ioUserExtraItem* pInventory = pUser->GetUserExtraItem();

		if( !pInventory )
			return;

		int iAddRow = DB_EXTRAITEM_SELECT_COUNT;

		for( int i = 0; i < iPageCount; i++ )
		{
			SP2Packet kPacket( SSTPK_EXTRAITEM_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );

			pUser->FillExtraItemMoveData( kPacket, iStartRow );
			SendMessage( kPacket );

			iStartRow += iAddRow; 
		}
	}
	else
	{
		SP2Packet kPacket( SSTPK_EXTRAITEM_DATA_MOVE_RESULT );
		PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
		PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
		pUser->FillExtraItemMoveData( kPacket, iStartRow );
		SendMessage( kPacket );
	}
}

void ServerNode::OnUserDataMove( SP2Packet &rkPacket )
{
	int iUserIndex = 0, iMovingValue = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );

	User *pUser = g_UserNodeManager.GetMoveUserNode( iUserIndex, iMovingValue );
	if(!pUser)
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserDataMove NULL %d - %d", iUserIndex, iMovingValue );
		return;
	}

	if( pUser && pUser->GetServerMovingValue() == iMovingValue )
	{
		{	
			// ���� ġ�� ���� ����
			SendDecoMoveData( iMovingValue, pUser );	
		}

		{	// ���� ���� ���� ����
			SP2Packet kPacket( SSTPK_ALCHEMIC_DATA_MOVE_RESULT );
			kPacket << iMovingValue << pUser->GetPublicID();
			pUser->FillAlchemicInvenMoveData( kPacket );
			SendMessage( kPacket );
		}

		{	// ���� ��� ���� ����
			SendExtraItemMoveData( iMovingValue, pUser );
		}

		{	// ���� ����Ʈ ���� ����
			SP2Packet kPacket( SSTPK_QUEST_DATA_MOVE_RESULT );
			kPacket << iMovingValue << pUser->GetPublicID();
			pUser->FillQuestMoveData( kPacket );
			SendMessage( kPacket );
		}

		{
			// ���� ETC������ ���� ����
			SP2Packet kPacket( SSTPK_ETCITEM_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(iMovingValue) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
			pUser->FillEtcItemMoveData( kPacket );
			SendMessage( kPacket );
		}

		{
			// ���� �޴� ������ ���� ����
			SP2Packet kPacket( SSTPK_MEDALITEM_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(iMovingValue) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
			pUser->FillMedalItemMoveData( kPacket );
			SendMessage( kPacket );
		}

		{
			// ���� �뺴���� ����
			SP2Packet kPacket( SSTPK_SOLDIER_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(iMovingValue) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
			pUser->FillSoldierMoveData( kPacket );
			SendMessage( kPacket );

		}

		{	// ���� �⺻ & ��Ÿ ���� ����
			SP2Packet kPacket( SSTPK_USER_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write(iMovingValue) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->GetPublicID()) );
			pUser->FillMoveData( kPacket );
			SendMessage( kPacket );
		}

		{
			//���� �� ���� ����
			SP2Packet kPacket( SSTPK_PET_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillPetMoveData( kPacket );
			SendMessage( kPacket );
		}

		{
			//���� �������� ���� ����
			SP2Packet kPacket( SSTPK_CHAR_AWAKE_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillAwakeMoveData( kPacket );
			SendMessage( kPacket );	
		}

		{
			//���� �ڽ�Ƭ ���� ����
			SP2Packet kPacket( SSTPK_COSTUME_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillCostumeMoveData(kPacket);
			SendMessage(kPacket);
		}

		{
			//���� �̼� ���� ����
			SP2Packet kPacket( SSTPK_MISSION_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillMissionMoveData(kPacket);
			SendMessage(kPacket);
		}

		{
			//���� �⼮�� ���� ����
			SP2Packet kPacket( SSTPK_ROOLBOOK_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillRollBookMoveData(kPacket);
			SendMessage(kPacket);
		}

		{
			// ���κ��� �κ� ������
			SP2Packet kPacket( SSTPK_PERSONAL_HQ_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillPersonalHQData(kPacket);
			SendMessage(kPacket);
		}

		{
			// ���ʽ� ĳ��.
			SP2Packet kPacket( SSTPK_BONUS_CASH_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillBonusCashData(kPacket);
			SendMessage(kPacket);
		}

		{
			// �Ⱓ ĳ�� �ڽ� ������.
			SP2Packet kPacket( SSTPK_TIMECASH_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillTimeCashDate(kPacket);
			SendMessage(kPacket);
		}

		{
			// Īȣ ���� 
			SP2Packet kPacket( SSTPK_TITLE_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillTitleMoveData(kPacket);
			SendMessage(kPacket);
		}

		{
			// �Ǽ��縮 ����
			SP2Packet kPacket( SSTPK_ACCESSORY_DATA_MOVE_RESULT );
			PACKET_GUARD_VOID( kPacket.Write( iMovingValue ) );
			PACKET_GUARD_VOID( kPacket.Write( pUser->GetPublicID() ) );
			pUser->FillAccessoryMoveData(kPacket);
			SendMessage(kPacket);
		}

		g_LogDBClient.OnInsertTime( pUser, LogDBClient::TT_MOVE_SERVER );
		pUser->SetStartTimeLog( 0 ); // �ʱ�ȭ

		// ���� ������ ���������Ƿ� ���ݺ��� DB���� �޴� �����ʹ� �����ϱ����� ��带 �����Ѵ�.
		g_UserNodeManager.MoveUserNode( pUser );

		// ���� ���纻 ����
		UserCopyNode *pNewNode = CreateNewUser( pUser->GetUserIndex() );
		if( pNewNode )
		{
			pNewNode->ApplyUserNode( (UserParent*)pUser );
			g_UserNodeManager.AddCopyUser( pNewNode );
		}

		// ��� ������ ���� �̵� ����
		SP2Packet kPacket1( SSTPK_USER_MOVE );
		kPacket1 << GetServerIndex();
		pUser->FillUserLogin( kPacket1 );
		g_ServerNodeManager.SendMessageAllNode( kPacket1, GetServerIndex() );

		pUser->BeforeLogoutProcess();

		//kyg���� �̶� ������ �̵� ���� �˰� 
		// ���ĺ��� ������ ��ġ�� ���� ������ �ƴ϶� �̵��� �����̴�.
		if( IsSameBilling( GetServerIndex()) )
		{
			//kyg ���� ���漭���� ���� �����̳� �ƴϳĸ� �Ǵ� 
		}
		else
		{
			//���� ���� �ʴٸ� ���� LogOut ��Ŷ�� ������. 
			LOG.PrintTimeAndLog(0,"[info][ServerMove]Send logout packet : [%lu]", pUser->GetUserIndex());
			pUser->SetSendLogOutUserState(true);

			pUser->SendSessionLogout();
		}
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserDataMove Not Node %d - %d", iUserIndex, iMovingValue );

	
	if(pUser)
	{
		
	}
	
}

void ServerNode::OnUserInventoryMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	rkPacket >> iMovingValue >> szPublicID;

	// ���� �̵��ϴ� �������̹Ƿ� ��Ŷ�� �������� �� ������ �ٸ������� �ִٴ°� ���� �ȵȴ�.
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyInventoryMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserInventoryMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserExtraItemMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	rkPacket >> iMovingValue >> szPublicID;

	// ���� �̵��ϴ� �������̹Ƿ� ��Ŷ�� �������� �� ������ �ٸ������� �ִٴ°� ���� �ȵȴ�.
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyExtraItemMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserExtraItemMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserAlchemicMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	rkPacket >> iMovingValue >> szPublicID;

	// ���� �̵��ϴ� �������̹Ƿ� ��Ŷ�� �������� �� ������ �ٸ������� �ִٴ°� ���� �ȵȴ�.
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyAlchemicInvenData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserAlchemicMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserPetMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	PACKET_GUARD_VOID( rkPacket.Read( iMovingValue ) );
	PACKET_GUARD_VOID( rkPacket.Read( szPublicID ) );
	
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyPetData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserPetMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserCharAwakeMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	PACKET_GUARD_VOID( rkPacket.Read( iMovingValue ) );
	PACKET_GUARD_VOID( rkPacket.Read( szPublicID ) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyAwakeMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserPetMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserQuestMoveResult( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;
	rkPacket >> iMovingValue >> szPublicID;

	// ���� �̵��ϴ� �������̹Ƿ� ��Ŷ�� �������� �� ������ �ٸ������� �ִٴ°� ���� �ȵȴ�.
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyQuestMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserQuestMoveResult Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUserDataMoveResult( SP2Packet &rkPacket )
{//kyg �̋� ������ ���� �̵� ������ �ν��� .
	int iMovingValue;
	ioHashString szPublicID;
	rkPacket >> iMovingValue >> szPublicID;

	// ���� �̵��ϴ� �������̹Ƿ� ��Ŷ�� �������� �� ������ �ٸ������� �ִٴ°� ���� �ȵȴ�.
	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		
		pUser->ApplyMoveData( rkPacket );	

		pUser->AfterLoginProcess( true );
		//���⼭ �õ� ���� �ľ� ���� gk

		// ģ�� ��� ��û.
		SP2Packet kPacket( SSTPK_USER_FRIEND_MOVE );
		kPacket << pUser->GetUserIndex() << iMovingValue << FRIEND_LIST_MOVE_COUNT << 0;
		SendMessage( kPacket );
		
		// ���� �̵��Ǿ����� ���� ����.
		RemoveUserNode( pUser->GetUserIndex() );
		g_DBClient.OnUpdateUserMoveServerID( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), g_App.GetGameServerID() );		

		pUser->m_bOnPopup = false;
		g_EventMgr.NotifySpecificEventInfoWhenLogin(pUser);
		//pUser->OnConnectPopupProcess();

		// ���� �� �����ڼ� ����
		SP2Packet kTotalRegUser( STPK_TOTAL_REG_USER_CNT );
		kTotalRegUser << g_MainServer.GetTotalUserRegCount();
		pUser->SendMessage( kTotalRegUser );

		// ���� ���� ���� ����
		g_SaleMgr.SendLastActiveDate( pUser );

	}
	else
	{
		RemoveUserNode( szPublicID );

		// ���� �̵��߿� ������ Disconnect�Ǿ����� ������忡 �ְ� �ٷ� Disconnect���Ѽ� �������� �α׾ƿ� ���μ����� �۵� ��Ų��.
		g_UserNodeManager.ServerMovingPassiveLogOut( rkPacket );
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnUserDataMoveResult Exception Node %s = �α׾ƿ� ��Ŵ", szPublicID.c_str() );
	}
}

void ServerNode::OnUserSync( SP2Packet &rkPacket )
{
	DWORD dwSyncType, dwUserIndex;
	rkPacket >> dwSyncType >> dwUserIndex;
	switch( dwSyncType )
	{
	case USER_SYNC_LOGIN:
		{
			UserCopyNode *pNewNode = CreateNewUser( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCreate( rkPacket );
				g_UserNodeManager.AddCopyUser( pNewNode );

				//
				g_HeroRankManager.CheckLogIn( pNewNode->GetUserIndex(), pNewNode->GetPublicID() );
			}
		}
		break;
	case USER_SYNC_POS:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncPos( rkPacket );
			}
			//else
			//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_UPDATE:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncUpdate( rkPacket );
			}
			//else
			//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_GUILD:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncGuild( rkPacket );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_CAMP:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCamp( rkPacket );

				//
				g_HeroRankManager.CheckCamp( pNewNode->GetUserIndex(), pNewNode->GetUserCampPos() );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_LOGOUT:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				RemoveUserNode( dwUserIndex );
			}			
			else
			{
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
				
				// ���� ��忡 �ִ� ������ �α׾ƿ� ��Ų��.
				ServerNode *pServerNode = g_ServerNodeManager.GetUserIndexToServerNode( dwUserIndex );
				if( pServerNode )
				{
					pServerNode->RemoveUserNode( dwUserIndex );
					LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnUserSync Exception LogOut : %d - %s:%d", dwUserIndex, GetServerIP().c_str(), GetServerPort() );
				}
			}

			//
			g_HeroRankManager.CheckLogOut( dwUserIndex );
		}		
		break;
	case USER_SYNC_PUBLICID:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncPublicID( rkPacket );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_BESTFRIEND:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncBestFriend( rkPacket );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserSync Not User : %d - %d - %s:%d", dwSyncType, dwUserIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case USER_SYNC_SHUFFLE:
		{
			UserCopyNode *pNewNode = GetUserNode( dwUserIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncShuffle( rkPacket );
			}			
		}
		break;
	}
}

void ServerNode::_OnMoveUserNode( DWORD dwUserIndex, SP2Packet &rkPacket )
{
	// �̵��� ������ �������� �߰�.
	UserCopyNode *pNewNode = CreateNewUser( dwUserIndex );
	if( pNewNode )
	{
		pNewNode->ApplySyncCreate( rkPacket );
		g_UserNodeManager.AddCopyUser( pNewNode );
	}
}

void ServerNode::OnUserMove( SP2Packet &rkPacket )
{
	DWORD dwMoveServerIndex, dwUserIndex;
	rkPacket >> dwMoveServerIndex >> dwUserIndex;

	RemoveUserNode( dwUserIndex );
	{
		ServerNode *pServerNode = g_ServerNodeManager.GetServerNode( dwMoveServerIndex );
		if( pServerNode )
			pServerNode->_OnMoveUserNode( dwUserIndex, rkPacket );
		else
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserMove Not Node : %d", dwMoveServerIndex );
	}		
}

void ServerNode::OnUserFriendMove( SP2Packet &rkPacket )
{
	int iUserIndex, iMovingValue;
	rkPacket >> iUserIndex >> iMovingValue;
	User *pUser = g_UserNodeManager.GetMoveUserNode( iUserIndex, iMovingValue );
	if( pUser )
	{
		int iFriendCount, iLastIndex;
		rkPacket >> iFriendCount >> iLastIndex;

		SP2Packet kPacket( SSTPK_USER_FRIEND_MOVE_RESULT );
		pUser->SendFriendServerMove( iLastIndex, iFriendCount, kPacket );
		SendMessage( kPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnUserFriendMove Not Node : %d", iUserIndex );
		SP2Packet kPacket( SSTPK_USER_FRIEND_MOVE_RESULT );
		kPacket << iUserIndex << 0 << iMovingValue;
		SendMessage( kPacket );
	}
}

void ServerNode::OnUserFriendMoveResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwFriendCount, dwMovingValue;
	rkPacket >> dwUserIndex >> dwFriendCount >> dwMovingValue;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{		
		for(int i = 0;i < (int)dwFriendCount;i++)
		{
			int  iFriendIndex = 0;
			DWORD dwFriendUserIndex = 0;
			ioHashString szName;
			int iGradeLevel, iCampPosition;
			DWORD dwRegTime;
			int iSendCount = 0;
			DWORD dwSendDate = 0;
			int iReceiveCount = 0;
			int iBeforeReceiveCount = 0;
			DWORD dwReceiveDate = 0;
			bool bSave = false;

			rkPacket >> iFriendIndex >> dwFriendUserIndex >> szName >> iGradeLevel >> iCampPosition >> dwRegTime
				>> iSendCount >> dwSendDate >> iReceiveCount >> dwReceiveDate >> iBeforeReceiveCount >> bSave;
			pUser->InsertFriend( iFriendIndex, dwFriendUserIndex, szName, iGradeLevel, iCampPosition, dwRegTime
								, iSendCount, dwSendDate, iReceiveCount, dwReceiveDate, iBeforeReceiveCount, bSave );
		}

		if( dwFriendCount != FRIEND_LIST_MOVE_COUNT )
		{

#if defined( SRC_LATIN )
			// ���� �̵� �Ϸ� ������ ���� �� Ŭ���̾�Ʈ CHCStart() ȣ�� ���� L �޽��� ����.	JCLEE 140310
			LOG.PrintTimeAndLog( 0, "Apex L Msg Send, ServerNode::OnUserFriendMoveResult() " );
			g_ioApex.NoticeApexProxy_UserLogIn( pUser->GetUserIndex(), pUser->GetPrivateID(), pUser->GetPublicIP());
#endif

			// ���� �̵� �Ϸ� �˸�
			SP2Packet kPacket( STPK_MOVING_SERVER_COMPLETE );
			pUser->FillExerciseIndex( kPacket );
			pUser->SendMessage( kPacket );
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OnUserFriendMoveResult Complete : %s : %d", pUser->GetPublicID().c_str(), pUser->GetFriendSize() );

			//kyg ���� ���� �Ϸ� ���� ������ 2���� �پ��ִ� ���� 
			//���޽��� ������ ���� Ŭ���̾�Ʈ�� ���� �������� ������ ���� 
			//kyg����

			if(IsSameBilling(pUser->GetOldServerIndex()))
			{
				//�α��� �Ⱥ���
			}
			else
			{
				//�α��� ��Ŷ ���� 
				pUser->SendSessionLogin();
				LOG.PrintTimeAndLog(0,"[info][ServerMove]Send login packet : [%lu]",pUser->GetUserIndex());
				//pUser->AfterLoginProcess( true );
			}
		}
		else 
		{
			// ���� ģ�� ��� ��û.
			SP2Packet kPacket( SSTPK_USER_FRIEND_MOVE );
			kPacket << dwUserIndex << dwMovingValue << FRIEND_LIST_MOVE_COUNT << pUser->GetFriendLastIndex();
			SendMessage( kPacket );
		}
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserFriendMoveResult Not Node : %d", dwUserIndex );
}

void ServerNode::OnFollowUser( SP2Packet &rkPacket )
{
	ioHashString szUserName;
	DWORD dwUserIndex, dwRequestUserIndex;
	int   iUserPos, iAbilityLevel, iMyRoomIndex, iLeaveRoomIndex, iLeaveBattleRoomIndex;
	bool  bDeveloper;
	rkPacket >> dwUserIndex >> dwRequestUserIndex >> iUserPos >> iMyRoomIndex >> iLeaveRoomIndex >> iLeaveBattleRoomIndex >> szUserName >> bDeveloper >> iAbilityLevel;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		UserParent *pRequestUser = GetUserNode( dwRequestUserIndex );
		if( !pRequestUser )			
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnFollowUser Request User Not Node." );
			return;
		}

		if( pUser->_OnFollowUser( pRequestUser, iUserPos, iMyRoomIndex, iLeaveRoomIndex, iLeaveBattleRoomIndex, bDeveloper, iAbilityLevel ) )
		{
			switch( iUserPos )
			{
			case UP_TRAINING:
				{
					Room *pRoom = pUser->GetMyRoom();
					if( pRoom )
					{
						pRoom->EnterReserveUser( dwRequestUserIndex );
						SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
						kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_PLAZA << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << dwRequestUserIndex;	
						kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum() << (int)pRoom->GetPlazaModeType() << pRoom->GetRoomNumber();
						SendMessage( kPacket );
					}
				}
				break;
			case UP_BATTLE_ROOM:
				{
					BattleRoomParent *pUserBattleRoom = pUser->GetMyBattleRoom();
					if( pUserBattleRoom  )
					{
						// ��û �������� ��Ƽ ���� ����
						SP2Packet kPacket( SSTPK_BATTLEROOM_FOLLOW );
						kPacket << pRequestUser->GetUserIndex() << pUserBattleRoom->GetIndex() << pUser->GetUserPos();
						SendMessage( kPacket );
					}
				}
				break;
			case UP_LADDER_TEAM:
				{
					LadderTeamParent *pUserLadderTeam = pUser->GetMyLadderTeam();
					if( pUserLadderTeam )
					{
						SP2Packet kPacket( SSTPK_LADDERTEAM_FOLLOW );
						kPacket << pRequestUser->GetUserIndex() << pUserLadderTeam->GetIndex() << pUser->GetUserPos();
						SendMessage( kPacket );
					}
				}
				break;
			}
		}		
	}
	else if( !g_UserNodeManager.IsConnectUser( szUserName ) )
	{
		UserParent *pRequestUser = GetUserNode( dwRequestUserIndex );
		if( pRequestUser )
		{
			SP2Packet kPacket( STPK_FOLLOW_USER );
			kPacket << FOLLOW_USER_ERROR_3 << szUserName << iUserPos << iUserPos;
			pRequestUser->RelayPacket( kPacket );
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnFollowUser ��û �� ��� ���� ����." );
		}
	}
}

void ServerNode::OnUserPosIndex( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwRequestUserIndex;
	int   iUserPos, iPrevBattleIndex;
	ioHashString szUserName;
	bool bSafetyLevel;
	rkPacket >> dwUserIndex >> dwRequestUserIndex >> iUserPos >> szUserName >> bSafetyLevel >> iPrevBattleIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		UserParent *pRequestUser = GetUserNode( dwRequestUserIndex );
		if( !pRequestUser )			
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserPosIndex Request User Not Node." );
			return;
		}
		pUser->_OnUserPosIndex( pRequestUser, bSafetyLevel, iUserPos, iPrevBattleIndex );
	}
	else if( !g_UserNodeManager.IsConnectUser( szUserName ) )
	{
		UserParent *pRequestUser = GetUserNode( dwRequestUserIndex );
		if( pRequestUser )
		{
			SP2Packet kPacket( STPK_FOLLOW_USER );
			kPacket << FOLLOW_USER_ERROR_3 << szUserName << iUserPos << iUserPos;
			pRequestUser->RelayPacket( kPacket );
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserPosIndex ��û �� ��� ���� ����." );
		}
	}
}

void ServerNode::OnChannelSync( SP2Packet &rkPacket )
{
	int iSyncType;
	DWORD dwChannelIndex;
	rkPacket >> iSyncType >> dwChannelIndex;
	switch( iSyncType )
	{
	case ChannelParent::CHANNEL_CREATE:
		{
			ChannelCopyNode *pNewNode = CreateNewChannel( dwChannelIndex );
			if( pNewNode )
			{
				g_ChannelNodeManager.AddCopyChannel( pNewNode );		
			}
		}
		break;
	case ChannelParent::CHANNEL_DESTROY:
		{
			RemoveChannelNode( dwChannelIndex );
		}
		break;
	case ChannelParent::CHANNEL_ENTER:
		{
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				DWORD dwUserIndex;
				ioHashString szEnterUser;
				rkPacket >> dwUserIndex >> szEnterUser;
				pChannel->EnterChannel( dwUserIndex, szEnterUser );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelSync �����Ϸ��� ä�� ���� %d", dwChannelIndex );
		}
		break;
	case ChannelParent::CHANNEL_LEAVE:
		{
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				DWORD dwUserIndex;
				ioHashString szLeaveUser;
				rkPacket >> dwUserIndex >> szLeaveUser;
				pChannel->LeaveChannel( dwUserIndex, szLeaveUser );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelSync �����Ϸ��� ä�� ���� %d", dwChannelIndex );
		}
		break;
	case ChannelParent::CHANNEL_TRANSFER:
		{
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				DWORD dwUserIndex, dwPacketID;
				rkPacket >> dwUserIndex >> dwPacketID;				
				int iMoveSize = sizeof(int) + ( sizeof(DWORD) * 3 );
				// ��Ŷ ����
				SP2Packet kPacket( dwPacketID );
				kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
				pChannel->SendPacketTcp( kPacket, dwUserIndex );
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelSync �߰��Ϸ��� ä�� ���� %d", dwChannelIndex );
		}
		break;
	}
}

void ServerNode::OnChannelInvite( SP2Packet &rkPacket )
{
	int iTransferType;
	DWORD dwChannelIndex;
	rkPacket >> iTransferType >> dwChannelIndex;
	switch( iTransferType )
	{
	case INVITE_CHANNEL_TRANSFER:
		{
			ioHashString szInvitedID;
			DWORD dwInveteUserIndex;
			rkPacket >> szInvitedID >> dwInveteUserIndex;
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( szInvitedID );
				if( !pUserParent )   // ���� ����
				{
					UserCopyNode *pInviteUser = GetUserNode( dwInveteUserIndex );
					if( !pInviteUser )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_TRANSFER 1 �ʴ� ���� ���� : %d", dwInveteUserIndex );
						return;
					}

					SP2Packet kPacket( STPK_CHANNEL_INVITE );
					kPacket << CHANNEL_INVITE_NOT_USER << dwChannelIndex << szInvitedID;
					pInviteUser->RelayPacket( rkPacket );
				}
				else if( pUserParent->IsUserOriginal() )
				{
					User *pUser = (User*)pUserParent;
					int iResult = pUser->_OnChannelInviteException( dwChannelIndex );
					if( iResult != CHANNEL_INVITE_OK )
					{
						UserCopyNode *pInviteUser = GetUserNode( dwInveteUserIndex );
						if( !pInviteUser )
						{
							LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_TRANSFER 2 �ʴ� ���� ���� : %d", dwInveteUserIndex );
							return;
						}

						SP2Packet kPacket( STPK_CHANNEL_INVITE );
						kPacket << iResult << dwChannelIndex << szInvitedID;
						pInviteUser->RelayPacket( kPacket );

						CRASH_GUARD();
						if( pChannel->GetManToManID() == szInvitedID )
							pChannel->SetManToManID( "" );
					}
					else
					{
						UserCopyNode *pInviteUser = GetUserNode( dwInveteUserIndex );
						if( !pInviteUser )
						{
							LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_TRANSFER 3 �ʴ� ���� ���� : %d", dwInveteUserIndex );
							return;
						}

						User *pUser = (User*)pUserParent;
						pUser->EnterChannel( pChannel );
						SP2Packet kPacket( STPK_CHANNEL_INVITE );
						kPacket << CHANNEL_INVITE_OK << pChannel->GetIndex() << pInviteUser->GetPublicID() << pUser->GetPublicID();
						pChannel->SendPacketTcp( kPacket );			
					}
				}
				else      // ���纻 ���� ó��
				{
					UserCopyNode *pCopyUser = (UserCopyNode*)pUserParent;
					SP2Packet kPacket( SSTPK_CHANNEL_INVITE );
					kPacket << INVITE_CHANNEL_USER_TRANSFER << dwChannelIndex << szInvitedID << dwInveteUserIndex;
					pCopyUser->SendMessage( kPacket );
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_TRANSFER ���� ä�ο��� �ʴ� : %d", dwInveteUserIndex );
		}
		break;
	case INVITE_CHANNEL_TRANSFER_RESULT:
		break;
	case INVITE_CHANNEL_USER_TRANSFER:
		{
			ioHashString szInvitedID;
			DWORD dwInveteUserIndex;
			rkPacket >> szInvitedID >> dwInveteUserIndex;
			ChannelParent *pChannelParent = g_ChannelNodeManager.GetGlobalChannelNode( dwChannelIndex );
			if( pChannelParent )
			{
				User *pUser = GetUserOriginalNode( szInvitedID, rkPacket );
				if( !pUser )   // ���� ����
				{
					UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( dwInveteUserIndex );
					if( !pInviteUser )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_USER_TRANSFER 1 �ʴ� ���� ���� : %d", dwInveteUserIndex );
						return;
					}

					SP2Packet kPacket( STPK_CHANNEL_INVITE );
					kPacket << CHANNEL_INVITE_NOT_USER << dwChannelIndex << szInvitedID;
					pInviteUser->RelayPacket( kPacket );
				}
				else 
				{
					int iResult = pUser->_OnChannelInviteException( dwChannelIndex );
					if( iResult != CHANNEL_INVITE_OK )
					{
						UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( dwInveteUserIndex );
						if( !pInviteUser )
						{
							LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_USER_TRANSFER 2 �ʴ� ���� ���� : %d", dwInveteUserIndex );
							return;
						}

						SP2Packet kPacket( STPK_CHANNEL_INVITE );
						kPacket << iResult << dwChannelIndex << szInvitedID;
						pInviteUser->RelayPacket( kPacket );
						
						if( pChannelParent->IsChannelOriginal() )
						{
							ChannelNode *pNode = (ChannelNode*)pChannelParent;
							CRASH_GUARD();
							if( pNode->GetManToManID() == szInvitedID )
								pNode->SetManToManID( "" );
						}
						else
						{
							// �ʴ� ���� �����Ǿ��̵� ����
							ChannelCopyNode *pCopyNode = (ChannelCopyNode*)pChannelParent;
							SP2Packet kPacket( SSTPK_CHANNEL_INVITE );
							kPacket << INVITE_CHANNEL_USER_TRANSFER_RESULT << dwChannelIndex << szInvitedID;							
							pCopyNode->SendMessage( kPacket );							
						}
					}
					else
					{
						UserParent *pInviteUser = g_UserNodeManager.GetGlobalUserNode( dwInveteUserIndex );
						if( !pInviteUser )
						{
							LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_USER_TRANSFER 3 �ʴ� ���� ���� : %d", dwInveteUserIndex );
							return;
						}

                        pUser->EnterChannel( pChannelParent );
						SP2Packet kPacket( STPK_CHANNEL_INVITE );
						kPacket << CHANNEL_INVITE_OK << pChannelParent->GetIndex() << pInviteUser->GetPublicID() << pUser->GetPublicID();
						pChannelParent->SendPacketTcp( kPacket );			
					}
				}
			}				
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelInvite INVITE_CHANNEL_USER_TRANSFER ���� ä�ο��� �ʴ� : %d", dwInveteUserIndex );
		}
		break;
	case INVITE_CHANNEL_USER_TRANSFER_RESULT:
		{
			ioHashString szInvitedID;
			rkPacket >> szInvitedID;
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				CRASH_GUARD();
				if( pChannel->GetManToManID() == szInvitedID )
					pChannel->SetManToManID( "" );
			}
		}
		break;
	}
}

void ServerNode::OnChannelChat( SP2Packet &rkPacket )
{
	DWORD dwTransferType, dwChannelIndex;
	rkPacket >> dwTransferType >> dwChannelIndex;
	switch( dwTransferType )
	{
	case CHAT_CHANNEL_TRANSFER:
		{
			ChannelNode *pChannel = g_ChannelNodeManager.GetChannelNode( dwChannelIndex );
			if( pChannel )
			{
				DWORD dwUserIndex;
				ioHashString szSendUser, szChat;	
				rkPacket >> szSendUser >> szChat >> dwUserIndex;
				if( pChannel->GetManToManID().IsEmpty() )
				{
					SP2Packet kPacket( STPK_CHANNEL_CHAT );
					kPacket << dwChannelIndex << szSendUser << szChat;
					pChannel->SendPacketTcp( kPacket, dwUserIndex );	
				}
				else 
				{
					// ������ ������ ���� ���� ��Ų�� ä�� �߻�
					UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( pChannel->GetManToManID() );
					if( !pUserParent )     //���� ����
					{
						UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( szSendUser );
						if( pSendUser )
						{
							SP2Packet kPacket( STPK_CHANNEL_INVITE );
							kPacket << CHANNEL_INVITE_NOT_USER << pChannel->GetIndex() << pChannel->GetManToManID();
							if( pSendUser->IsUserOriginal() )
							{
								User *pUser = (User*)pSendUser;
								pUser->RelayPacket( kPacket );
							}
							else
							{
								UserCopyNode *pUser = (UserCopyNode*)pSendUser;
								pUser->RelayPacket( kPacket );
							}

						}						
						pChannel->SetManToManID( "" );
					}
					else if( pUserParent->IsUserOriginal() )
					{
						User *pUser = (User*)pUserParent;
						int iResult = pUser->_OnChannelInviteException( dwChannelIndex );
						if( iResult != CHANNEL_INVITE_OK )
						{
							SP2Packet kPacket( STPK_CHANNEL_INVITE );
							kPacket << iResult << dwChannelIndex << pChannel->GetManToManID();
							SendMessage( kPacket );
							pChannel->SetManToManID( "" );
						}
						else
						{
							pUser->EnterChannel( pChannel );				
						}

						// ä�� ����
						SP2Packet kPacket( STPK_CHANNEL_CHAT );
						kPacket << dwChannelIndex << szSendUser << szChat;
						pChannel->SendPacketTcp( kPacket, dwUserIndex );	
					}
					else    //���� �������� ���� �� ó��
					{
						UserCopyNode *pCopyNode = (UserCopyNode*)pUserParent;
						SP2Packet kPacket( SSTPK_CHANNEL_CHAT );
						kPacket << CHAT_CHANNEL_MANTOMAN_TRANSFER << dwChannelIndex << pCopyNode->GetUserIndex() << szSendUser << szChat << dwUserIndex;
						pCopyNode->SendMessage( kPacket );			
					}
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelChat CHAT_CHANNEL_TRANSFER Not Node : %d", dwChannelIndex );
		}
		break;
	case CHAT_CHANNEL_MANTOMAN_TRANSFER:
		{
			DWORD dwUserIndex;
			rkPacket >> dwUserIndex;
			User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
			if( pUser )
			{
				DWORD dwSendUserIndex;
				ioHashString szSendUser, szChat;	
				rkPacket >> szSendUser >> szChat >> dwSendUserIndex;

				int iResult = pUser->_OnChannelInviteException( dwChannelIndex );
				if( iResult != CHANNEL_INVITE_OK )
				{
					UserParent *pInvitedUser = g_UserNodeManager.GetGlobalUserNode( dwSendUserIndex );
					if( !pInvitedUser )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelChat CHAT_CHANNEL_MANTOMAN_TRANSFER 1 �ʴ� ���� ���� : %s", szSendUser.c_str() );
						return;
					}

					ChannelParent *pChannelParent = g_ChannelNodeManager.GetGlobalChannelNode( dwChannelIndex );
					if( !pChannelParent )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelChat CHAT_CHANNEL_MANTOMAN_TRANSFER 1 �ʴ� ä�� ���� : %s", szSendUser.c_str() );
						return;
					}
					
					if( !pInvitedUser->IsUserOriginal() )
					{
						UserCopyNode *pUserCopyNode = (UserCopyNode*)pInvitedUser;
						SP2Packet kPacket( STPK_CHANNEL_INVITE );
						kPacket << iResult << dwChannelIndex << pUser->GetPublicID();
						pUserCopyNode->RelayPacket( kPacket );
					}

					if( pChannelParent->IsChannelOriginal() )
					{
						ChannelNode *pNode = (ChannelNode*)pChannelParent;
						CRASH_GUARD();
						if( pNode->GetManToManID() == pUser->GetPublicID() )
							pNode->SetManToManID( "" );
					}
					else
					{
						// �ʴ� ���� �����Ǿ��̵� ����
						ChannelCopyNode *pCopyNode = (ChannelCopyNode*)pChannelParent;
						SP2Packet kPacket( SSTPK_CHANNEL_INVITE );
						kPacket << INVITE_CHANNEL_USER_TRANSFER_RESULT << dwChannelIndex << pUser->GetPublicID();							
						pCopyNode->SendMessage( kPacket );		
					}
				}	
				else
				{
					ChannelParent *pChannelParent = g_ChannelNodeManager.GetGlobalChannelNode( dwChannelIndex );
					if( !pChannelParent )
					{
						LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelChat CHAT_CHANNEL_MANTOMAN_TRANSFER 2 �ʴ� ä�� ���� : %s", szSendUser.c_str() );
						return;
					}

					pUser->EnterChannel( pChannelParent );

					// ä�� ����
					SP2Packet kPacket( STPK_CHANNEL_CHAT );
					kPacket << pChannelParent->GetIndex() << szSendUser << szChat;
					pChannelParent->SendPacketTcp( kPacket, dwSendUserIndex );	
				}
			}
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnChannelChat CHAT_CHANNEL_MANTOMAN_TRANSFER Not Node : %d", dwUserIndex );
		}
		break;
	}
}

void ServerNode::OnBattleRoomSync( SP2Packet &rkPacket )
{
	int iSyncType, iBattleRoomIndex;
	rkPacket >> iSyncType >> iBattleRoomIndex;

	switch( iSyncType )
	{
	case BattleRoomSync::BRS_SELECTMODE:
		{
			BattleRoomCopyNode *pCopyNode = GetBattleRoomNode( iBattleRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncSelectMode( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomSync Not Room : %d - %d - %s:%d", iSyncType, iBattleRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case BattleRoomSync::BRS_PLAY:
		{
			BattleRoomCopyNode *pCopyNode = GetBattleRoomNode( iBattleRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncPlay( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomSync Not Room : %d - %d - %s:%d", iSyncType, iBattleRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;	
	case BattleRoomSync::BRS_CHANGEINFO:
		{
			BattleRoomCopyNode *pCopyNode = GetBattleRoomNode( iBattleRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncChangeInfo( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomSync Not Room : %d - %d - %s:%d", iSyncType, iBattleRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case BattleRoomSync::BRS_CREATE:
		{
			BattleRoomCopyNode *pNewNode = CreateNewBattleRoom( iBattleRoomIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCreate( rkPacket );
				g_BattleRoomManager.AddCopyBattleRoom( pNewNode );
			}
		}
		break;
	case BattleRoomSync::BRS_DESTROY:
		RemoveBattleRoomNode( iBattleRoomIndex );
		break; 
	}
}

void ServerNode::OnBattleRoomTransfer( SP2Packet &rkPacket )
{
	// ��� �������� �������̴�
	int iTransferType, iBattleRoomIndex;
	rkPacket >> iTransferType >> iBattleRoomIndex;

	BattleRoomNode *pBattleRoom = g_BattleRoomManager.GetBattleRoomNode( iBattleRoomIndex );	
	switch( iTransferType )
	{
	case BattleRoomParent::ENTER_USER:
		{
			DWORD dwUserIndex;
			bool  bSafetyLevel, bObserver;
			int   iGradeLevel, iAbilityLevel, iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> iGradeLevel >> iAbilityLevel >> bSafetyLevel >> bObserver
					 >> szPublicIP >> szPrivateIP >> szTransferIP >> iClientPort >> iTransferPort;

			if( pBattleRoom == NULL )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_NOT_NODE << iBattleRoomIndex;
				SendMessage( kPacket );				
			}
			else if( !bObserver && pBattleRoom->IsFull() )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_FULL_USER << iBattleRoomIndex << pBattleRoom->GetPlayUserCnt()
						<< pBattleRoom->GetMaxPlayerBlue() << pBattleRoom->GetMaxPlayerRed() << pBattleRoom->GetMaxObserver();
				SendMessage( kPacket );						
			}
			else if( !bObserver && pBattleRoom->IsMapLimitPlayerFull() )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_MAP_LIMIT_PLAYER << iBattleRoomIndex << pBattleRoom->GetPlayUserCnt()
						<< pBattleRoom->GetMaxPlayerBlue() << pBattleRoom->GetMaxPlayerRed() << pBattleRoom->GetMaxObserver();
				SendMessage( kPacket );
			}
			else if( !bObserver && pBattleRoom->IsMapLimitGrade( iGradeLevel ) )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_MAP_LIMIT_GRADE << iBattleRoomIndex << pBattleRoom->GetPlayUserCnt()
					<< pBattleRoom->GetMaxPlayerBlue() << pBattleRoom->GetMaxPlayerRed() << pBattleRoom->GetMaxObserver();
				SendMessage( kPacket );
				return;
			}
			else if( bObserver && pBattleRoom->IsObserverFull() )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "bObserver && pBattleRoom->IsObserverFull() : %d:%s", dwUserIndex, szPublicID.c_str());

				bool bDeveloper = g_UserNodeManager.IsDeveloper( szPublicID.c_str() );
				if( bDeveloper == true )
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "bObserver && pBattleRoom->IsObserverFull()-v1 : %d:%s", dwUserIndex, szPublicID.c_str());
					pBattleRoom->EnterUser( dwUserIndex, szPublicID, iGradeLevel, iAbilityLevel, bSafetyLevel, true, szPublicIP, szPrivateIP, szTransferIP, iClientPort, iTransferPort );
				}
				else
				{
					SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
					kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_FULL_USER << iBattleRoomIndex << pBattleRoom->GetPlayUserCnt()
						<< pBattleRoom->GetMaxPlayerBlue() << pBattleRoom->GetMaxPlayerRed() << pBattleRoom->GetMaxObserver();
					SendMessage( kPacket );
				}
			}	
			else if( pBattleRoom->IsBattleTimeClose() && !pBattleRoom->IsInviteUser( dwUserIndex ) )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_TIME_CLOSE << iBattleRoomIndex;
				SendMessage( kPacket );								
			}
			else if( !bObserver && pBattleRoom->IsStartRoomEnterX() )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_START_ROOM_ENTER_X << iBattleRoomIndex;
				SendMessage( kPacket );	
			}
			else if( !bObserver && pBattleRoom->IsNoChallenger() )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_NO_CHALLENGER << iBattleRoomIndex;
				SendMessage( kPacket );	
			}
			else if( !bObserver && pBattleRoom->GetSelectModeTerm() == BMT_TEAM_SURVIVAL_FIRST && !bSafetyLevel )
			{
				SP2Packet kPacket( SSTPK_BATTLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_BATTLEROOM_JOIN_SAFETY_ROOM << iBattleRoomIndex;
				SendMessage( kPacket );	
			}
			else
			{				
				pBattleRoom->EnterUser( dwUserIndex, szPublicID, iGradeLevel, iAbilityLevel, bSafetyLevel, bObserver, szPublicIP, szPrivateIP, szTransferIP, iClientPort, iTransferPort );
			}			
		}
		break;
	case BattleRoomParent::LEAVE_USER:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer LEAVE_USER �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			ioHashString szPublicID;
			rkPacket >> dwUserIndex >> szPublicID;
			pBattleRoom->LeaveUser( dwUserIndex, szPublicID );
		}
		break;
	case BattleRoomParent::ROOM_INFO:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer ROOM_INFO �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			int iPrevBattleIndex;
			DWORD dwUserIndex;
			rkPacket >> dwUserIndex >> iPrevBattleIndex;
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomParent::ROOM_INFO ���� NULL : %d", dwUserIndex );
				return;
			}
			pBattleRoom->OnBattleRoomInfo( pUser, iPrevBattleIndex );
		}
		break;
	case BattleRoomParent::USER_INFO:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer USER_INFO �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}
			
			DWORD dwUserIndex;
			bool  bSafetyLevel;
			int   iGradeLevel, iAbilityLevel, iClientPort, iTransferPort;
			ioHashString szTransferIP;
			rkPacket >> dwUserIndex >> iGradeLevel >> iAbilityLevel >> bSafetyLevel >> iClientPort >> szTransferIP >> iTransferPort;
			pBattleRoom->UserInfoUpdate( dwUserIndex, iGradeLevel, iAbilityLevel, bSafetyLevel, iClientPort, szTransferIP, iTransferPort );
		}
		break;
	case BattleRoomParent::UDP_CHANGE:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer UDP_CHANGE �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			int   iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> szPublicIP >> iClientPort >> szPrivateIP >> szTransferIP >> iTransferPort;
			pBattleRoom->UserUDPChange( dwUserIndex, szPublicID, szPublicIP, iClientPort, szPrivateIP, szTransferIP, iTransferPort );
		}
		break;
	case BattleRoomParent::TRANSFER_PACKET:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer TRANSFER_PACKET �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pBattleRoom->SendPacketTcp( kPacket, dwUserIndex );			
		}
		break;
	case BattleRoomParent::TRANSFER_PACKET_USER:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer TRANSFER_PACKET_USER �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			ioHashString szSenderName;
			DWORD dwPacketID;
			rkPacket >> szSenderName >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 1 ) + lstrlen( szSenderName.c_str() ) + 1;
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pBattleRoom->SendPacketTcpUser( kPacket, szSenderName );			
		}
		break;
	case BattleRoomParent::USER_PACKET_TRANSFER:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer USER_PACKET_TRANSFER �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;	
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "BattleRoomParent::USER_PACKET_TRANSFER ���� NULL : %d - 0x%x", dwUserIndex, dwPacketID );
				return;
			}
			// ��Ŷ ����
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize, true );
			pBattleRoom->OnProcessPacket( kPacket, pUser );
		}
		break;
	case BattleRoomParent::P2P_RELAY_INFO:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer P2P_RELAY_INFO �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}
			DWORD dwUserIndex, dwRelayUserIndex;
			bool  bServerRelay;
			rkPacket >> dwUserIndex >> dwRelayUserIndex >> bServerRelay;
			pBattleRoom->UserP2PRelayInfo( dwUserIndex, dwRelayUserIndex, bServerRelay );
		}
		break;
	case BattleRoomParent::TRANSFER_PACKET_UDP:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer TRANSFER_PACKET_UDP �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pBattleRoom->SendPacketUdp( kPacket, dwUserIndex );			
		}
		break;
	case BattleRoomParent::TOURNAMENT_INFO:
		{
			if( !pBattleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomTransfer TOURNAMENT_INFO �߰��Ϸ��� ������ ���� %d", iBattleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwTeamIndex;
			rkPacket >> dwUserIndex >> dwTeamIndex;
			pBattleRoom->TournamentInfo( dwUserIndex, dwTeamIndex );
		}
		break;
	}
}

void ServerNode::OnPlazaRoomTransfer( SP2Packet &rkPacket )
{
	// ��� �������� �������̴�
	int iTransferType, iRoomIndex;
	rkPacket >> iTransferType >> iRoomIndex;

	RoomParent *pNode = g_RoomNodeManager.GetPlazaNodeByNum( iRoomIndex );
	switch( iTransferType )
	{
	case RoomParent::ROOM_INFO:
		{
			if( !pNode )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnPlazaRoomTransfer ROOM_INFO �߰��Ϸ��� ������ ���� %d", iRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			rkPacket >> dwUserIndex;
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "RoomParent::ROOM_INFO ���� NULL : %d", dwUserIndex );
				return;
			}
			pNode->OnPlazaRoomInfo( pUser );
		}
		break;
	}
}

void ServerNode::OnBattleRoomJoinResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex = 0;
	DWORD dwBattleRoomIndex = 0;
	int   iResultType = 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iResultType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwBattleRoomIndex) );

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomJoinResult ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->_OnBattleRoomJoinResult( iResultType, false, 0, true );

	// ������� ����
	BattleRoomCopyNode *pBattleRoom = GetBattleRoomNode( dwBattleRoomIndex );
	if( pBattleRoom )
	{
		switch( iResultType )
		{
		case USER_BATTLEROOM_JOIN_FULL_USER:
			{
				int iPlayUserCnt, iMaxPlayerBlue, iMaxPlayerRed, iMaxObserver;
				rkPacket >> iPlayUserCnt >> iMaxPlayerBlue >> iMaxPlayerRed >> iMaxObserver;
				pBattleRoom->SetJoinUserCnt( iPlayUserCnt );
				pBattleRoom->SetMaxPlayer( iMaxPlayerBlue, iMaxPlayerRed, iMaxObserver );
			}
			break;
		case USER_BATTLEROOM_JOIN_TIME_CLOSE:
			{
				pBattleRoom->SetTimeClose( true );
			}
			break;
		case USER_BATTLEROOM_JOIN_START_ROOM_ENTER_X:
			{
				pBattleRoom->SetStartRoomEnterX( true );
			}
			break;
		case USER_BATTLEROOM_JOIN_NO_CHALLENGER:
			{
				pBattleRoom->SetNoChallenger( true );
			}
			break;
		}
	}
}

void ServerNode::OnBattleRoomFollow( SP2Packet &rkPacket )
{
	int iNextPos;
	DWORD dwUserIndex, dwBattleRoomIndex;
	rkPacket >> dwUserIndex >> dwBattleRoomIndex >> iNextPos;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomFollow ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->_OnBattleRoomFollow( g_BattleRoomManager.GetGlobalBattleRoomNode( dwBattleRoomIndex ), iNextPos );	
}

void ServerNode::OnUserInfoRefresh( SP2Packet &rkPacket )
{
	DWORD dwTargetIndex, dwSendIndex;
	rkPacket >> dwTargetIndex >> dwSendIndex;
	
	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserInfoRefresh Ÿ�� ���� NULL : %d", dwTargetIndex );
		return;
	}

	UserCopyNode *pSendUser = GetUserNode( dwSendIndex );
	if( pSendUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserInfoRefresh ��û ���� NULL : %d", dwSendIndex );
		return;
	}
	
	// ���� ����
	SP2Packet kPacket( STPK_USER_INFO_REFRESH );
	pTargetUser->FillToolTipInfo( kPacket );
	pSendUser->RelayPacket( kPacket );

	// ��� ����
	ioUserGuild *pUserGuild = pTargetUser->GetUserGuild();
	if( pUserGuild )			
	{
		// �������� ����
		SP2Packet kPacket( STPK_GUILD_SIMPLE_DATA );
		kPacket << pTargetUser->GetPublicID() << pUserGuild->GetGuildIndex() << pUserGuild->GetGuildName()
				<< pUserGuild->GetGuildMark();
		pSendUser->RelayPacket( kPacket );
	}			
}

void ServerNode::OnSimpleUserInfoRefresh( SP2Packet &rkPacket )
{
	DWORD dwTargetIndex, dwSendIndex;
	rkPacket >> dwTargetIndex >> dwSendIndex;

	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnSimpleUserInfoRefresh Ÿ�� ���� NULL : %d", dwTargetIndex );
		return;
	}

	UserCopyNode *pSendUser = GetUserNode( dwSendIndex );
	if( pSendUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnSimpleUserInfoRefresh ��û ���� NULL : %d", dwSendIndex );
		return;
	}

	// ���� ����
	SP2Packet kPacket( STPK_SIMPLE_USER_INFO_REFRESH );
	pTargetUser->FillSimpleToolTipInfo( kPacket );
	pSendUser->RelayPacket( kPacket );
}

void ServerNode::OnUserCharInfoRefresh( SP2Packet &rkPacket )
{
	DWORD dwTargetIndex, dwSendIndex;
	rkPacket >> dwTargetIndex >> dwSendIndex;

	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserCharInfoRefresh Ÿ�� ���� NULL : %d", dwTargetIndex );
		return;
	}

	UserCopyNode *pSendUser = GetUserNode( dwSendIndex );
	if( pSendUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserCharInfoRefresh ��û ���� NULL : %d", dwSendIndex );
		return;
	}

	int iStartArray, iSyncCount;
	rkPacket >> iStartArray >> iSyncCount;

	SP2Packet kPacket( STPK_USER_CHAR_INFO_REFRESH );
	pTargetUser->FillUserCharListInfo( kPacket, iStartArray, iSyncCount );

	pSendUser->RelayPacket( kPacket );
}

void ServerNode::OnUserCharSubInfoRefresh( SP2Packet &rkPacket )
{
	DWORD dwTargetIndex, dwSendIndex;
	rkPacket >> dwTargetIndex >> dwSendIndex;

	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserCharSubInfoRefresh Ÿ�� ���� NULL : %d", dwTargetIndex );
		return;
	}

	UserCopyNode *pSendUser = GetUserNode( dwSendIndex );
	if( pSendUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserCharSubInfoRefresh ��û ���� NULL : %d", dwSendIndex );
		return;
	}

	int iClassType;
	rkPacket >> iClassType;
	SP2Packet kPacket( STPK_USER_CHAR_SUB_INFO_REFRESH );
	pTargetUser->FillUserCharSubInfo( kPacket, iClassType );
	pSendUser->RelayPacket( kPacket );
}

void ServerNode::OnBattleRoomKickOut( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomKickOut ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->BattleRoomKickOut();
}

void ServerNode::OnOfflineMemo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwSendCount;
	rkPacket >> dwUserIndex >> dwSendCount;

	UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnOfflineMemo ���� ���� : %d", dwUserIndex );
		return;
	}
	g_MemoNodeManager.SendMemo( pUser, dwSendCount );
}

void ServerNode::OnReserveCreateRoom( SP2Packet &rkPacket )
{
	bool bSafetyRoom;
	int  iModeType, iSubType, iModeMapNum, iUserSize;
	rkPacket >> bSafetyRoom >> iModeType >> iSubType >> iModeMapNum;

	if( iModeType == MT_TRAINING )       //����
	{
		Room *pRoom = g_RoomNodeManager.CreateNewPlazaRoom( iSubType, iModeMapNum );
		if( pRoom )
		{
			ioHashString szPlazaName, szPlazaPW;
			int iMaxPlayer, iPlazaType;
			rkPacket >> szPlazaName >> szPlazaPW >> iMaxPlayer >> iPlazaType;
			if( !szPlazaName.IsEmpty() )
				pRoom->SetRoomName( szPlazaName );
			if( !szPlazaPW.IsEmpty() )
				pRoom->SetRoomPW( szPlazaPW );
			pRoom->SetMaxPlayer( iMaxPlayer );
			pRoom->SetPlazaModeType( (PlazaType)iPlazaType );
			pRoom->SetSubState( false );

			//������ ����
			rkPacket >> iUserSize;
			for(int i = 0;i < iUserSize;i++)
			{
				ioHashString szUserID;
				DWORD dwUserIndex;
				rkPacket >> dwUserIndex >> szUserID;
				if( i == 0 )
					pRoom->SetRoomMasterID( szUserID );

				pRoom->EnterReserveUser( dwUserIndex );
				SP2Packet kPacket( SSTPK_MOVING_ROOM_RESULT );
				kPacket << SS_MOVING_ROOM_JOIN << SS_MOVING_ROOM_JOIN_PLAZA << (int)pRoom->GetModeType() << pRoom->GetRoomIndex() << dwUserIndex;	
				kPacket << pRoom->GetModeSubNum() << pRoom->GetModeMapNum() << (int)pRoom->GetPlazaModeType() << pRoom->GetRoomNumber();

				UserCopyNode *pUser = GetUserNode( dwUserIndex );
				if( pUser )
					pUser->SendMessage( kPacket );
				else
				{
					SendMessage( kPacket );
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateRoom Not User Copy Node Plaza : %d:%s", dwUserIndex, szUserID.c_str() );
				}
			}
		}
		else
		{
			//���� ����.
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateRoom Not Room Create Plaza");
		}
	}	
}

void ServerNode::OnReserveCreateBattleRoom( SP2Packet &rkPacket )
{
	bool bUserCreate;
	ioHashString szBattleRoomName, szBattleRoomPW;
	int iBlueCount, iRedCount, iObserver, iUserCount, iSelectTerm;
	rkPacket >> bUserCreate >> szBattleRoomName >> szBattleRoomPW >> iBlueCount >> iRedCount >> iObserver >> iSelectTerm >> iUserCount;

	// ��Ƽ ����
	BattleRoomNode *pBattleRoom = g_BattleRoomManager.CreateNewBattleRoom();
	if( !pBattleRoom )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateBattleRoom Not BattleRoom Node" );
		return;
	}

	if( !szBattleRoomName.IsEmpty() )
		pBattleRoom->SetName( szBattleRoomName );
	if( !szBattleRoomPW.IsEmpty() )
		pBattleRoom->SetPW( szBattleRoomPW );
	pBattleRoom->SetMaxPlayer( iBlueCount, iRedCount, iObserver );
	pBattleRoom->SetDefaultMode( iSelectTerm );

	// ���� �������� ��� ����ȭ �ߵ�
	pBattleRoom->SyncRealTimeCreate();

	for(int i = 0;i < iUserCount;i++)
	{
		DWORD dwUserIndex;
		ioHashString szPublicID;
		rkPacket >> dwUserIndex >> szPublicID;
		
		UserCopyNode *pUser = GetUserNode( dwUserIndex );
		if( pUser )
		{
			SP2Packet kPacket( SSTPK_RESERVE_CREATE_BATTLEROOM_RESULT );
			kPacket << dwUserIndex << pBattleRoom->GetIndex() << bUserCreate;
			SendMessage( kPacket );			
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateBattleRoom Not User Copy Node : %d:%s", dwUserIndex, szPublicID.c_str() );
		}		
	}
}

void ServerNode::OnReserveCreateBattleRoomResult( SP2Packet &rkPacket )
{
	bool bUserCreate;
	DWORD dwUserIndex, dwBattleRoomIndex;
	rkPacket >> dwUserIndex >> dwBattleRoomIndex >> bUserCreate;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateBattleRoomResult ���� NULL : %d", dwUserIndex );
		return;
	}

	pUser->ReserveBattleRoom( false );
	if( !pUser->EnterBattleRoom( dwBattleRoomIndex, this ) )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnReserveCreateBattleRoomResult ��û�� ������ ���� ����: %s : %d", pUser->GetPublicID().c_str(), dwBattleRoomIndex );
		if( bUserCreate )
		{
			SP2Packet kPacket( STPK_CREATE_BATTLEROOM );
			kPacket << CREATE_BATTLEROOM_NOT;
			pUser->SendMessage( kPacket );
		}
	}
	else if( bUserCreate )
	{
		SP2Packet kPacket( STPK_CREATE_BATTLEROOM );
		kPacket << CREATE_BATTLEROOM_OK;
		pUser->SendMessage( kPacket );
	}
}

void ServerNode::OnExceptionBattleRoomLeave( SP2Packet &rkPacket )
{
	int iBattleRoomIndex;
	rkPacket >> iBattleRoomIndex;
	BattleRoomNode *pBattleRoom = g_BattleRoomManager.GetBattleRoomNode( iBattleRoomIndex );	
	if( !pBattleRoom ) return;

	DWORD dwUserIndex;
	ioHashString szPublicID;
	rkPacket >> dwUserIndex >> szPublicID;
	pBattleRoom->LeaveUser( dwUserIndex, szPublicID );

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnExceptionBattleRoomLeave(%d) : %s", iBattleRoomIndex, szPublicID.c_str() );
}

void ServerNode::OnCreateGuildResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwItemTableIndex;
	int iResult, iItemFieldNum;
	rkPacket >> dwUserIndex >> iResult >> dwItemTableIndex >> iItemFieldNum;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwUserIndex ) == NULL )
		{
			// ���� �α� �ƿ�
			if( iResult == CREATE_GUILD_OK )
			{
				// ��� ������ �����ߴµ� ������ �α׾ƿ� ���¶�� DB �������� ���� �����Ѵ�.
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OnCreateGuildResult User Find Not CreateGuild OK: %d", dwUserIndex );
				g_DBClient.OnUpdateGuildEtcItemDelete( 0, 0, dwUserIndex, iItemFieldNum, dwItemTableIndex );
			}
		}
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnCreateGuildResult ���� NULL : %d", dwUserIndex );
		return;
	}

	if( iResult == CREATE_GUILD_OK )       //����
	{
		g_DBClient.OnSelectCreateGuildInfo( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), dwUserIndex );

		//��� ���� ���� ������ ����
		ioUserEtcItem *pUserEtcItem = pUser->GetUserEtcItem();
		if( pUserEtcItem )
		{
			pUserEtcItem->DeleteEtcItem( ioEtcItem::EIT_ETC_GUILD_CREATE, LogDBClient::ET_DEL );
		}
		SP2Packet kSuccess( STPK_ETCITEM_USE );
		kSuccess << ETCITEM_USE_OK;
		kSuccess << (int)ioEtcItem::EIT_ETC_GUILD_CREATE;
		pUser->SendMessage( kSuccess );
	}
	else
	{
		// ���� �˸�
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnCreateGuildResult CreateGuild Fail: %s - %I64d - E(%d)", pUser->GetPublicID().c_str(), pUser->GetMoney(), iResult );

		SP2Packet kPacket( STPK_CREATE_GUILD );
		kPacket << iResult;
		pUser->SendMessage( kPacket );
	}
}

void ServerNode::OnCreateGuildComplete( SP2Packet &rkPacket )
{
	DWORD dwUserIndex		= 0;
	DWORD dwAttendRcvDate	= 0;
	DWORD dwRankRcvDate		= 0;
	DWORD dwJoinDate		= 0;
	BOOL bActive			= FALSE;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnCreateGuildComplete ���� NULL : %d", dwUserIndex );
		return;
	}

	DWORD dwGuildIndex	= 0;
	ioHashString szGuildName, szGuildPos;
	int iGuildMark	= 0;
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(szGuildName) );
	PACKET_GUARD_VOID( rkPacket.Read(iGuildMark) );
	PACKET_GUARD_VOID( rkPacket.Read(szGuildPos) );
	
	SP2Packet kPacket( STPK_CREATE_GUILD );
	kPacket << CREATE_GUILD_OK << pUser->GetMoney() << dwGuildIndex << szGuildName << iGuildMark << szGuildPos;
	pUser->SendMessage( kPacket );

	PACKET_GUARD_VOID( rkPacket.Read(dwAttendRcvDate) );
	PACKET_GUARD_VOID( rkPacket.Read(dwRankRcvDate) );
	PACKET_GUARD_VOID( rkPacket.Read(dwJoinDate) );
	PACKET_GUARD_VOID( rkPacket.Read(bActive) );

	ioUserGuild *pUserGuild = pUser->GetUserGuild();
	if( pUserGuild )
	{
		pUserGuild->SetGuildData( dwGuildIndex, szGuildName, szGuildPos, iGuildMark, true );
		pUserGuild->SetRecvAttendRewardDate(dwAttendRcvDate);
		pUserGuild->SetRecvRankRewardDate(dwRankRcvDate);
		pUserGuild->SetJoinDate(dwJoinDate);
		pUserGuild->SetGuildRoomState(bActive);
	}
}

void ServerNode::OnGuildEntryAgree( SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0, dwGuildIndex	= 0;
	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwUserIndex ) == NULL )
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnGuildEntryAgree ����(%d) ������ ���� �����Ͽ� ���� ����Ʈ ��û", dwUserIndex );
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildEntryAgree ���� NULL : %d", dwUserIndex );
		}
		return;
	}

	ioUserGuild *pUserGuild = pUser->GetUserGuild();
	if( pUserGuild )
	{
		ioHashString szGuildName, szGuildPos;
		int iGuildMark	= 0, iGuildEvent	= 0;
		DWORD dwAttendRcvDate	= 0;
		DWORD dwRankRcvDate		= 0;
		DWORD dwJoinDate		= 0;
		BOOL bActive			= FALSE;

		PACKET_GUARD_VOID( rkPacket.Read(szGuildName) );
		PACKET_GUARD_VOID( rkPacket.Read(szGuildPos) );
		PACKET_GUARD_VOID( rkPacket.Read(iGuildMark) );
		PACKET_GUARD_VOID( rkPacket.Read(iGuildEvent) );
		PACKET_GUARD_VOID( rkPacket.Read(dwAttendRcvDate) );
		PACKET_GUARD_VOID( rkPacket.Read(dwRankRcvDate) );
		PACKET_GUARD_VOID( rkPacket.Read(dwJoinDate) );
		PACKET_GUARD_VOID( rkPacket.Read(bActive) );

		pUserGuild->SetGuildDataEntryAgree( dwGuildIndex, szGuildName, szGuildPos, iGuildMark, true );
		pUserGuild->SetRecvAttendRewardDate(dwAttendRcvDate);
		pUserGuild->SetRecvRankRewardDate(dwRankRcvDate);
		pUserGuild->SetJoinDate(dwJoinDate);
		pUserGuild->SetGuildRoomState(bActive);

		CTime cCurTime = CTime::GetCurrentTime();
		DWORD dwCurTime	= cCurTime.GetTime();

		//�������� ��� ��Ŷ ����
		SP2Packet kPacket( STPK_MY_GUILD_INFO );
		PACKET_GUARD_VOID(kPacket.Write(dwGuildIndex));
		PACKET_GUARD_VOID(kPacket.Write(szGuildName));
		PACKET_GUARD_VOID(kPacket.Write(szGuildPos));
		PACKET_GUARD_VOID(kPacket.Write(iGuildMark));
		PACKET_GUARD_VOID(kPacket.Write(iGuildEvent));
		PACKET_GUARD_VOID(kPacket.Write(pUserGuild->GetRecvAttendRewardDate()));
		PACKET_GUARD_VOID(kPacket.Write(pUserGuild->GetGuildJoinDate()));
		PACKET_GUARD_VOID(kPacket.Write(dwCurTime));
		PACKET_GUARD_VOID(kPacket.Write(bActive));
		pUser->SendMessage( kPacket );

		g_DBClient.OnUpdateGuildMemberEvent( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), dwUserIndex, dwGuildIndex );
		g_DBClient.OnSelectGuildMemberListEx( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), dwUserIndex, dwGuildIndex, 0, false );
	}	
}

void ServerNode::OnGuildMemberListEx( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildMemberListEx ���� NULL : %d", dwUserIndex );
		return;
	}

	if( !pUser->IsGuild() )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildMemberListEx ��� ������ �ƴ� : %d", dwUserIndex );
		return;
	}

	ioUserGuild *pUserGuild = pUser->GetUserGuild();
	if( pUserGuild )
	{
		int iCommand;
		rkPacket >> iCommand;
		switch( iCommand )
		{
		case SSTPK_GUILD_MEMBER_LIST_ALL:
			pUserGuild->GuildEntryAgreeSync();
			break;
		case SSTPK_GUILD_MEMBER_LIST_ME:
			g_DBClient.OnSelectGuildMemberListEx( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), pUserGuild->GetGuildIndex(), 0, pUserGuild->IsGuildMaster() );
			break;
		}
	}	
}

void ServerNode::OnGuildInvitation( SP2Packet &rkPacket )
{
	ioHashString szUserID;
	DWORD dwSendIndex, dwRecvIndex;
	rkPacket >> dwSendIndex >> dwRecvIndex >> szUserID;

	User *pRecvUser = GetUserOriginalNode( dwRecvIndex, rkPacket );
	if( pRecvUser == NULL )
	{
		UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( dwSendIndex );
		if( pSendUser )
		{
			// ���� �������� �˸�
			SP2Packet kPacket( STPK_GUILD_INVITATION );
			kPacket << GUILD_INVITATION_OFFLINE << szUserID;
			pSendUser->RelayPacket( kPacket );
		}		
	}
	else
	{
		UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( dwSendIndex );
		if( pSendUser )
		{
			int   iGuildMark;
			DWORD dwGuildIndex;
			ioHashString szGuildName;
			rkPacket >> dwGuildIndex >> iGuildMark >> szGuildName;
			pRecvUser->_OnGuildInvitation( pSendUser, dwGuildIndex, iGuildMark, szGuildName );
		}
	}
}

void ServerNode::OnGuildMasterChange( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwTargetIndex, dwGuildIndex;
	rkPacket >> dwUserIndex >> dwTargetIndex >> dwGuildIndex;

	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnGuildMasterChange Not User : %d", dwTargetIndex );
		UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
		if( pSendUser )
		{
			// ���� �̵��� ��� �� �ٽ� �õ� �˸�
			SP2Packet kPacket( STPK_GUILD_MASTER_CHANGE );
			kPacket << GUILD_MASTER_CHANGE_DELAY;
			pSendUser->RelayPacket( kPacket );
		}		
		return;
	}
	else
	{
		ioUserGuild *pUserGuild = pTargetUser->GetUserGuild();
		if( pUserGuild )			
		{
			if( pUserGuild->GetGuildIndex() == dwGuildIndex )        //���� ���� ���� ���� ����
			{
				ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
				if( pLocal )
					pUserGuild->SetGuildPosition( ioHashString( pLocal->GetGuildMasterPostion() ) );	
				g_DBClient.OnUpdateGuildMasterChange( pTargetUser->GetUserDBAgentID(), pTargetUser->GetAgentThreadID(), dwUserIndex, dwTargetIndex, dwGuildIndex );
			}		
			else            //�̹� Ż���� ���� �˸�
			{
				UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
				if( pSendUser )
				{
					SP2Packet kPacket( SSTPK_GUILD_MASTER_CHANGE );
					kPacket << GUILD_MASTER_CHANGE_LEAVEUSER;
					pSendUser->RelayPacket( kPacket );
				}
			}
		}						
	}	
}

void ServerNode::OnGuildPositionChange( SP2Packet &rkPacket )
{
	ioHashString szGuildPos;
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex >> szGuildPos;
	
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildPositionChange Not User : %d : %s", dwUserIndex, szGuildPos.c_str() );
	}
	else
	{
		ioUserGuild *pUserGuild = pUser->GetUserGuild();
		if( pUserGuild )
			pUserGuild->SetGuildPosition( szGuildPos );
	}
}

void ServerNode::OnGuildKickOut( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwSendIndex, dwGuildIndex;
	rkPacket >> dwUserIndex >> dwSendIndex >> dwGuildIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildKickOut Not User : %d", dwUserIndex );
	}
	else
	{
		ioUserGuild *pUserGuild = pUser->GetUserGuild();
		if( pUserGuild )			
		{
			if( pUserGuild->GetGuildIndex() == dwGuildIndex )        //���� ���� ���� ����� ����
			{
				// DB ����
				g_DBClient.OnDeleteGuildLeaveUser( pUser->GetUserDBAgentID(), pUser->GetAgentThreadID(), pUser->GetUserIndex(), pUserGuild->GetGuildIndex() );

				// ���� ������ �˸�
				SP2Packet kMainPacket( MSTPK_GUILD_LEAVE );
				kMainPacket << pUserGuild->GetGuildIndex();
				g_MainServer.SendMessage( kMainPacket );

				SP2Packet kPacket( STPK_GUILD_KICK_OUT );
				kPacket << GUILD_KICK_OUT_OK << pUserGuild->GetGuildIndex() << pUser->GetPublicID() << false;
				pUser->SendMessage( kPacket );

				// ��� ���� �ʱ�ȭ
				pUserGuild->LeaveGuildInitInfo( true );
			}		
			else            
			{
				UserParent *pSendUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
				if( pSendUser )
				{
					SP2Packet kPacket( STPK_GUILD_KICK_OUT );
					kPacket << GUILD_KICK_OUT_LEAVEUSER;
					pSendUser->RelayPacket( kPacket );
				}
			}
		}						
	}
}

void ServerNode::OnFriendDelete( SP2Packet &rkPacket )
{
	ioHashString szUserID, szFriendID;
	rkPacket >> szUserID >> szFriendID;

	User *pUser = GetUserOriginalNode( szUserID, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnFriendDelete Not User : %s", szUserID.c_str() );
	}
	else
	{
		pUser->_OnBestFriendDismiss( szFriendID, false );
		pUser->DeleteFriend( szFriendID );
		SP2Packet kPacket( STPK_FRIEND_DELETE );
		kPacket << szFriendID;
		pUser->SendMessage( kPacket );
		pUser->CheckRoomBonusTable();
	}
}

void ServerNode::OnGuildMarkChange( SP2Packet &rkPacket )
{
	bool bBlock;
	DWORD dwUserIndex, dwGuildIndex, dwGuildMark;
	rkPacket >> dwUserIndex >> dwGuildIndex >> dwGuildMark >> bBlock;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildMarkChange Not User : %d", dwUserIndex );
	}
	else
	{
		pUser->_OnGuildMarkChange( dwGuildIndex, dwGuildMark, bBlock );
	}
}

void ServerNode::OnGuildUserDelete( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildUserDelete Not User : %d", dwUserIndex );
	}
	else
	{
		DWORD dwGuildIndex;
		ioHashString szUserID;
		rkPacket >> szUserID >> dwGuildIndex;

		ioUserGuild *pUserGuild = pUser->GetUserGuild();
		if( pUserGuild )
		{
			if( pUserGuild->GetGuildIndex() == dwGuildIndex )
			{
				pUserGuild->DeleteGuildUser( szUserID );

				// �������� ����
				DWORD dwRelayPacketID;
				int iResultType;
				rkPacket >> dwRelayPacketID >> iResultType;

				SP2Packet kPacket( dwRelayPacketID );
				kPacket << iResultType << dwGuildIndex << szUserID;
				pUser->SendMessage( kPacket );
			}
		}
	}
}

void ServerNode::OnLadderTeamSync( SP2Packet &rkPacket )
{
	// ���� - > ī�Ǻ����� ���� ����ȭ
	int iSyncType;
	rkPacket >> iSyncType;

	switch( iSyncType )
	{
	case LadderTeamSync::LTS_CHANGEINFO:
		{
			DWORD dwLadderTeamIndex;
			rkPacket >> dwLadderTeamIndex;
			LadderTeamCopyNode *pCopyNode = GetLadderTeamNode( dwLadderTeamIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncChangeInfo( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamSync Not Room : %d - %d - %s:%d", iSyncType, dwLadderTeamIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;	
	case LadderTeamSync::LTS_CHANGERECORD:
		{
			DWORD dwLadderTeamIndex;
			rkPacket >> dwLadderTeamIndex;
			LadderTeamCopyNode *pCopyNode = GetLadderTeamNode( dwLadderTeamIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncChangeRecord( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamSync Not Room : %d - %d - %s:%d", iSyncType, dwLadderTeamIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case LadderTeamSync::LTS_CREATE:
		{
			DWORD dwLadderTeamIndex;
			rkPacket >> dwLadderTeamIndex;
			LadderTeamCopyNode *pNewNode = CreateNewLadderTeam( dwLadderTeamIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCreate( rkPacket );
				g_LadderTeamManager.AddCopyLadderTeam( pNewNode );
			}
		}
		break;
	case LadderTeamSync::LTS_DESTROY:
		{
			DWORD dwLadderTeamIndex;
			rkPacket >> dwLadderTeamIndex;
			RemoveLadderTeamNode( dwLadderTeamIndex );
		}		
		break; 
	}
}

void ServerNode::OnLadderTeamTransfer( SP2Packet &rkPacket )
{
	// ī�Ǻ� - > ���� ���� ����
	int iTransferType	= 0;
	DWORD dwLadderTeamIndex = 0;

	PACKET_GUARD_VOID( rkPacket.Read(iTransferType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwLadderTeamIndex) );

	LadderTeamNode *pLadderTeam = g_LadderTeamManager.GetLadderTeamNode( dwLadderTeamIndex );	
	switch( iTransferType )
	{
	case LadderTeamParent::ENTER_USER:
		{
			DWORD dwUserIndex, dwGuildIndex, dwGuildMark;
			int   iGradeLevel, iAbilityLevel, iHeroMatchPoint, iLadderPoint, iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> iGradeLevel >> iAbilityLevel >> iHeroMatchPoint >> iLadderPoint >> dwGuildIndex >> dwGuildMark
					 >> szPublicIP >> szPrivateIP >> szTransferIP >> iClientPort >> iTransferPort;
			if( pLadderTeam == NULL )
			{
				SP2Packet kPacket( SSTPK_LADDERTEAM_JOIN_RESULT );
				kPacket << dwUserIndex << LADDERTEAM_JOIN_NOT_LADDERTEAM << dwLadderTeamIndex;
				SendMessage( kPacket );				
			}
			else if( pLadderTeam->IsMatchPlay() )
			{
				SP2Packet kPacket( SSTPK_LADDERTEAM_JOIN_RESULT );
				kPacket << dwUserIndex << LADDERTEAM_JOIN_MATCH_PLAY << dwLadderTeamIndex;
				SendMessage( kPacket );
			}
			else if( pLadderTeam->IsFull() )
			{
				SP2Packet kPacket( SSTPK_LADDERTEAM_JOIN_RESULT );
				kPacket << dwUserIndex << LADDERTEAM_JOIN_FULL << dwLadderTeamIndex << pLadderTeam->GetJoinUserCnt() << pLadderTeam->GetMaxPlayer();
				SendMessage( kPacket );						
			}					
			else
			{				
				pLadderTeam->EnterUser( dwUserIndex, szPublicID, iGradeLevel, iAbilityLevel, iHeroMatchPoint, iLadderPoint, dwGuildIndex, dwGuildMark, szPublicIP, szPrivateIP, szTransferIP, iClientPort, iTransferPort );
			}			
		}
		break;
	case LadderTeamParent::LEAVE_USER:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer LEAVE_USER �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex;
			ioHashString szPublicID;
			rkPacket >> dwUserIndex >> szPublicID;
			pLadderTeam->LeaveUser( dwUserIndex, szPublicID );
		}
		break;
	case LadderTeamParent::TEAM_STATE:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TEAM_STATE �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}
			DWORD dwTeamState;
			rkPacket >> dwTeamState;
			pLadderTeam->SetTeamState( dwTeamState );
		}
		break;
	case LadderTeamParent::MATCH_ROOM_REQUEST:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TEAM_STATE �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwRequestIndex = 0, dwRequestSelectMode = 0;
			rkPacket >> dwRequestIndex >> dwRequestSelectMode;

			//����� ���� ��� 
			pLadderTeam->MatchRoomRequest( dwRequestIndex );
		}
		break;
	case LadderTeamParent::MATCH_ROOM_REQUEST_JOIN:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer MATCH_ROOM_REQUEST_JOIN �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}
			
			DWORD dwOtherTeamIndex	= 0;
			int iModeType = 0;
			int iModeSubNum = 0;
			int iModeMapNum = 0; 
			int iCampType = 0;
			int iTeamType = 0;

			PACKET_GUARD_VOID( rkPacket.Read(dwOtherTeamIndex) );
			PACKET_GUARD_VOID( rkPacket.Read(iModeType) );
			PACKET_GUARD_VOID( rkPacket.Read(iModeSubNum) );
			PACKET_GUARD_VOID( rkPacket.Read(iModeMapNum) );
			PACKET_GUARD_VOID( rkPacket.Read(iCampType) );
			PACKET_GUARD_VOID( rkPacket.Read(iTeamType) );

            pLadderTeam->MatchRoomRequestJoin( dwOtherTeamIndex, (ModeType)iModeType, iModeSubNum, iModeMapNum, iCampType, iTeamType );
		}
		break;
	case LadderTeamParent::MATCH_ROOM_RESERVE_CANCEL:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer MATCH_ROOM_RESERVE_CANCEL �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			pLadderTeam->MatchReserveCancel();
		}
		break;
	case LadderTeamParent::MATCH_ROOM_END_SYNC:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer MATCH_ROOM_END_SYNC �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			pLadderTeam->MatchPlayEndSync();
		}
		break;
	case LadderTeamParent::TEAM_INFO:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TEAM_INFO �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex;
			rkPacket >> dwUserIndex;
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamParent::TEAM_INFO ���� NULL : %d", dwUserIndex );
				return;
			}
			pLadderTeam->OnLadderTeamInfo( pUser );
		}
		break;
	case LadderTeamParent::USER_INFO:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer USER_INFO �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex, dwGuildIndex, dwGuildMark;
			int   iGradeLevel, iAbilityLevel, iHeroMatchPoint, iLadderPoint, iClientPort, iTransferPort;
			ioHashString szTransferIP;
			rkPacket >> dwUserIndex >> iGradeLevel >> iAbilityLevel >> iHeroMatchPoint >> iLadderPoint >> dwGuildIndex >> dwGuildMark >> iClientPort >> szTransferIP >> iTransferPort;
			pLadderTeam->UserInfoUpdate( dwUserIndex, iGradeLevel, iAbilityLevel, iHeroMatchPoint, iLadderPoint, dwGuildIndex, dwGuildMark,  iClientPort, szTransferIP, iTransferPort );
		}
		break;
	case LadderTeamParent::UPDATE_RECORD:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer UPDATE_RECORD �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}
			int iWinTeam, iPlayTeam;
			rkPacket >> iWinTeam >> iPlayTeam;
			pLadderTeam->UpdateRecord( (TeamType)iPlayTeam, (TeamType)iWinTeam );
			g_LadderTeamManager.SortLadderTeamRank( pLadderTeam->IsHeroMatchMode() ); //��ŷ ����
		}
		break;
	case LadderTeamParent::UPDATE_GUILDMARK:
		{
		}
		break;
	case LadderTeamParent::UDP_CHANGE:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer UDP_CHANGE �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex;
			int   iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> szPublicIP >> iClientPort >> szPrivateIP >> szTransferIP >> iTransferPort;
			pLadderTeam->UserUDPChange( dwUserIndex, szPublicID, szPublicIP, iClientPort, szPrivateIP, szTransferIP, iTransferPort );
		}
		break;
	case LadderTeamParent::TRANSFER_PACKET:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TRANSFER_PACKET �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pLadderTeam->SendPacketTcp( kPacket, dwUserIndex );			
		}
		break;
	case LadderTeamParent::TRANSFER_PACKET_USER:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TRANSFER_PACKET_USER �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			ioHashString szSenderName;
			DWORD dwPacketID;
			rkPacket >> szSenderName >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 1 ) + lstrlen( szSenderName.c_str() ) + 1;
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pLadderTeam->SendPacketTcpUser( kPacket, szSenderName );			
		}
		break;
	case LadderTeamParent::USER_PACKET_TRANSFER:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer USER_PACKET_TRANSFER �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;	
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "LadderTeamParent::USER_PACKET_TRANSFER ���� NULL : %d - 0x%x", dwUserIndex, dwPacketID );
				return;
			}
			// ��Ŷ ����
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize, true );
			pLadderTeam->OnProcessPacket( kPacket, pUser );
		}
		break;
	case LadderTeamParent::TRANSFER_PACKET_UDP:
		{
			if( !pLadderTeam )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamTransfer TRANSFER_PACKET_UDP �߰��Ϸ��� ����� ���� %d", dwLadderTeamIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;				
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pLadderTeam->SendPacketUdp( kPacket, dwUserIndex );			
		}
		break;
	}
}

void ServerNode::OnExceptionLadderTeamLeave( SP2Packet &rkPacket )
{
	DWORD dwLadderTeamIndex;
	
	PACKET_GUARD_VOID( rkPacket.Read(dwLadderTeamIndex) ) ; 

	LadderTeamNode *pLadderTeam = g_LadderTeamManager.GetLadderTeamNode( dwLadderTeamIndex );	
	if( !pLadderTeam ) return;

	DWORD dwUserIndex;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) ); 
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	pLadderTeam->LeaveUser( dwUserIndex, szPublicID );

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnExceptionLadderTeamLeave(%d) : %s", dwLadderTeamIndex, szPublicID.c_str() );
}

void ServerNode::OnReserveCreateLadderTeam( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	LadderTeamNode *pLadderTeam = g_LadderTeamManager.CreateNewLadderTeam();
	if( !pLadderTeam )
	{
		UserCopyNode *pUser = GetUserNode( dwUserIndex );
		if( pUser )
		{
			SP2Packet kPacket( STPK_CREATE_LADDERTEAM );
			kPacket << LADDERTEAM_CREATE_NOT;
			pUser->RelayPacket( kPacket );			
			return;
		}
	}	

	DWORD dwJoinGuildIndex;
	int iCampType, iModeSelectType, iLadderMaxPlayer;
	ioHashString szTeamName, szPassword;
	bool bHeroMatchMode;

	int iSize = 0;
	rkPacket >> iCampType >> iModeSelectType >> szTeamName >> szPassword >> iLadderMaxPlayer >> dwJoinGuildIndex >> bHeroMatchMode;

	if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_TAIWAN )
	{
		// ������ �ϰ�츸 ���� �̵��ÿ� DB ���� ��ȸ�� �����ߴ� ���� ����Ʈ ������
		if( bHeroMatchMode == true)
		{
			rkPacket >> iSize;
			if(iSize != 0)
			{
				int iUser = 0;
				for(int i = 0; i<iSize; i++)
				{	
					rkPacket >> iUser;
					pLadderTeam->SetCompetitorsList( iUser );
				}
			}
			pLadderTeam->SetUserIndex( dwUserIndex );		// �� ���� �ε���
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateLadderTeam() %d competitor Count:%d", dwUserIndex, iSize );

		}	
	}


	pLadderTeam->SetCampType( iCampType );
	pLadderTeam->SetTeamName( szTeamName );
	pLadderTeam->SetTeamPW( szPassword );
	pLadderTeam->SetMaxPlayer( iLadderMaxPlayer );
	pLadderTeam->SetJoinGuildIndex( dwJoinGuildIndex );
	pLadderTeam->SelectMode( iModeSelectType );
	pLadderTeam->SetHeroMatchMode( bHeroMatchMode );
	pLadderTeam->SyncRealTimeCreate();
	UserCopyNode *pUser = GetUserNode( dwUserIndex );
	if( pUser )
	{
		SP2Packet kPacket( SSTPK_RESERVE_CREATE_LADDERTEAM_RESULT );
		kPacket << dwUserIndex << pLadderTeam->GetIndex();
		SendMessage( kPacket );	
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateLadderTeam Not User Copy Node : %d", dwUserIndex );
	}
}

void ServerNode::OnReserveCreateLadderTeamResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwLadderTeamIndex;
	rkPacket >> dwUserIndex >> dwLadderTeamIndex;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnReserveCreateLadderTeamResult ���� NULL : %d", dwUserIndex );
		return;
	}

	pUser->ReserveLadderTeam( false );
	if( !pUser->EnterLadderTeam( dwLadderTeamIndex, this ) )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnReserveCreateLadderTeamResult ��û�� ������ ���� ����: %s : %d", pUser->GetPublicID().c_str(), dwLadderTeamIndex );
		
		SP2Packet kPacket( STPK_CREATE_LADDERTEAM );
		kPacket << LADDERTEAM_CREATE_NOT;
		pUser->SendMessage( kPacket );
	}
}

void ServerNode::OnLadderTeamJoinResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwLadderTeamIndex;
	int   iResultType;
	rkPacket >> dwUserIndex >> iResultType >> dwLadderTeamIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamJoinResult ���� NULL : %d", dwUserIndex );
		return;
	}		
	
	switch( iResultType )
	{
	case LADDERTEAM_JOIN_NOT_LADDERTEAM:
	case LADDERTEAM_JOIN_MATCH_PLAY:
		{
			pUser->LeaveLadderTeam();
			SP2Packet kPacket( STPK_JOIN_LADDERTEAM );
			kPacket << iResultType;
			pUser->SendMessage( kPacket );	
		}
		break;
	case LADDERTEAM_JOIN_FULL:
		{
			pUser->LeaveLadderTeam();
			//�ο� �ʰ�
			SP2Packet kPacket( STPK_JOIN_LADDERTEAM );
			kPacket << iResultType;
			pUser->SendMessage( kPacket );

			LadderTeamCopyNode *pLadderTeam = GetLadderTeamNode( dwLadderTeamIndex );
			if( pLadderTeam )
			{
				int iJoinUserCount, iMaxPlayer;
				rkPacket >> iJoinUserCount >> iMaxPlayer;
				pLadderTeam->SetJoinUserCnt( iJoinUserCount );
				pLadderTeam->SetMaxPlayer( iMaxPlayer );
			}
		}
		break;
	}
}

void ServerNode::OnLadderTeamKickOut( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamKickOut ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->LadderTeamKickOut();
}

void ServerNode::OnLadderTeamFollow( SP2Packet &rkPacket )
{
	int iNextPos;
	DWORD dwUserIndex, dwLadderTeamIndex;
	rkPacket >> dwUserIndex >> dwLadderTeamIndex >> iNextPos;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamFollow ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->_OnLadderTeamFollow( g_LadderTeamManager.GetGlobalLadderTeamNode( dwLadderTeamIndex ), iNextPos );	
}

void ServerNode::OnLadderTeamEnterRoomUser( SP2Packet &rkPacket )
{
	DWORD dwLadderTeamIndex;
	rkPacket >> dwLadderTeamIndex;
	LadderTeamNode *pLadderTeam = g_LadderTeamManager.GetLadderTeamNode( dwLadderTeamIndex );
	if( pLadderTeam )
	{
		if( !pLadderTeam->MatchRequstTeamEnterRoom( rkPacket ) )
		{
			LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnLadderTeamEnterRoomUser[%d] �� ���� ����!!", dwLadderTeamIndex );
		}
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLadderTeamEnterRoomUser pLadderTeam == NULL : %d", dwLadderTeamIndex );
	}
}

void ServerNode::OnCampSeasonBonus( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwBonusIndex;
	int iBlueCampPoint, iBlueCampBonusPoint, iBlueEntry, iRedCampPoint, iRedCampBonusPoint, iRedEntry, iMyCampType, iMyCampPoint, iMyCampRank;

	rkPacket >> dwUserIndex >> dwBonusIndex >> iBlueCampPoint >> iBlueCampBonusPoint >> iBlueEntry 
             >> iRedCampPoint >> iRedCampBonusPoint >> iRedEntry >> iMyCampType >> iMyCampPoint >> iMyCampRank;
	
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		pUser->AddCampSeasonBonus( dwBonusIndex, iBlueCampPoint, iBlueCampBonusPoint, iBlueEntry,
								   iRedCampPoint, iRedCampBonusPoint, iRedEntry, iMyCampType, iMyCampPoint, iMyCampRank );
	}
}

void ServerNode::OnGuildNameChange( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwMasterIndex, dwGuildIndex;
	ioHashString szNewGuildName;
	rkPacket >> dwUserIndex >> dwMasterIndex >> dwGuildIndex >> szNewGuildName;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		ioUserGuild *pUserGuild = pUser->GetUserGuild();
		if( pUserGuild && pUserGuild->GetGuildIndex() == dwGuildIndex )
		{
			pUserGuild->SetGuildName( szNewGuildName );
			SP2Packet kUserPacket( STPK_GUILD_NAME_CHANGE );
			kUserPacket << GUILD_NAME_CHANGE_OK << dwMasterIndex << pUserGuild->GetGuildIndex() << pUserGuild->GetGuildName();
			pUser->SendMessage( kUserPacket );		
		}
	}
}

void ServerNode::OnGuildNameChangeResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwGuildIndex, dwItemTableIndex;
	int iResult, iItemFieldNum;	
	char szGuildName[GUILD_NAME_NUM_PLUS_ONE] = "";
	rkPacket >> dwUserIndex >> iResult >> dwGuildIndex >> szGuildName >> dwItemTableIndex >> iItemFieldNum;
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwUserIndex ) == NULL )
		{
			// ���� �α� �ƿ�
			if( iResult == GUILD_NAME_CHANGE_OK )
			{
				// ���� ������ �����ߴµ� ������ �α׾ƿ� ���¶�� DB �������� ���� �����Ѵ�.
				LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "OnGuildNameChangeResult User Find Not GuildNameChange OK: %d", dwUserIndex );
				g_DBClient.OnUpdateGuildEtcItemDelete( 0, 0, dwUserIndex, iItemFieldNum, dwItemTableIndex );
			}
		}
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnGuildNameChangeResult ���� NULL : %d", dwUserIndex );
		return;
	}

	if( iResult == GUILD_NAME_CHANGE_OK )       //����
	{
		ioUserGuild *pUserGuild = pUser->GetUserGuild();
		if( !pUserGuild || !pUser->IsGuild() )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "������ ������ ���� ����  ġ������ ���� 2: %s - %d:%s", pUser->GetPublicID().c_str(), dwGuildIndex, szGuildName );
			return;
		}

		//���� ���� ���� ������ ����
		ioUserEtcItem *pUserEtcItem = pUser->GetUserEtcItem();
		if( pUserEtcItem )
		{
			pUserEtcItem->DeleteEtcItem( ioEtcItem::EIT_ETC_GUILD_NAME_CHANGE, LogDBClient::ET_DEL );
		}
		// ���� ������ ����
		SP2Packet kSuccess( STPK_ETCITEM_USE );
		kSuccess << ETCITEM_USE_OK;
		kSuccess << ioEtcItem::EIT_ETC_GUILD_NAME_CHANGE;
		pUser->SendMessage( kSuccess );

		// �����鿡�� ���� ����ȭ
		pUserGuild->SetGuildName( szGuildName );
		SP2Packet kUserPacket( STPK_GUILD_NAME_CHANGE );
		kUserPacket << GUILD_NAME_CHANGE_OK << pUser->GetUserIndex() << pUserGuild->GetGuildIndex() << pUserGuild->GetGuildName();
		pUser->SendMessage( kUserPacket );
		pUserGuild->GuildNameChangeSync();
	}
	else
	{
		// ���� �˸�
		SP2Packet kPacket( STPK_GUILD_NAME_CHANGE );
		kPacket << iResult << pUser->GetUserIndex();
		pUser->SendMessage( kPacket );
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "GuildNameChange Fail: %s - E(%d)", pUser->GetPublicID().c_str(), iResult );
	}
}

void ServerNode::OnPresentSelect( SP2Packet &rkPacket )
{
	DWORD dwUserIndex  = 0;
	int   iSelectCount = 0;
	rkPacket >> dwUserIndex >> iSelectCount;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnPresentSelect ���� NULL : %d", dwUserIndex );
		return;
	}

	pUser->_OnSelectPresent( iSelectCount );
}

void ServerNode::OnSubscriptionSelect( SP2Packet &rkPacket )
{
	DWORD dwUserIndex  = 0;
	int   iSelectCount = 0;
	rkPacket >> dwUserIndex >> iSelectCount;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnSubscriptionSelect ���� NULL : %d", dwUserIndex );
		return;
	}

	pUser->_OnSelectSubscription( iSelectCount );
}

void ServerNode::OnUserHeroData( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) ); //rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUserHeroData ���� NULL : %d", dwUserIndex );
		return;
	}

	UserHeroData kUserHeroData;
	PACKET_GUARD_VOID( rkPacket.Read(kUserHeroData.m_iHeroTitle) );
	PACKET_GUARD_VOID( rkPacket.Read(kUserHeroData.m_iHeroTodayRank) );
	for(int i = 0;i < HERO_SEASON_RANK_MAX;i++)
	{
		PACKET_GUARD_VOID( rkPacket.Read(kUserHeroData.m_iHeroSeasonRank[i]) );
	}
	pUser->SetUserHeroData( kUserHeroData );
}

void ServerNode::OnHeroMatchOtherInfo( SP2Packet &rkPacket )
{
	DWORD dwTargetIndex, dwSendIndex;
	rkPacket >> dwTargetIndex >> dwSendIndex;

	User *pTargetUser = GetUserOriginalNode( dwTargetIndex, rkPacket );
	if( pTargetUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnHeroMatchOtherInfo Ÿ�� ���� NULL : %d", dwTargetIndex );
		return;
	}

	UserCopyNode *pSendUser = GetUserNode( dwSendIndex );
	if( pSendUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnHeroMatchOtherInfo ��û ���� NULL : %d", dwSendIndex );
		return;
	}

	// ���� ����
	SP2Packet kPacket( STPK_HERO_MATCH_OTHER_INFO );
	pTargetUser->FillHeroMatchInfo( kPacket );
	pSendUser->RelayPacket( kPacket );
}

void ServerNode::OnUserCharRentalAgree( SP2Packet &rkPacket )
{
	DWORD dwAgreeDBAgentID;
	ioHashString szTargetID, szAgreeID;
	rkPacket >> szTargetID >> szAgreeID >> dwAgreeDBAgentID;

	User *pTargetUser = GetUserOriginalNode( szTargetID, rkPacket );
	if( pTargetUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( szTargetID ) == NULL )
		{
			CHARACTER kCharInfo;
			rkPacket >> kCharInfo;

			RentalData kRentalData;
			kRentalData.ApplyData( rkPacket );

			// Agree ���� �뺴 ��ȯ 
			UserParent *pAgreeUserParent = g_UserNodeManager.GetGlobalUserNode( szAgreeID );
			if( pAgreeUserParent )
			{
				SP2Packet kPacket( STPK_USER_CHAR_RENTAL_AGREE );
				kPacket << USER_CHAR_RENTAL_AGREE_NONE_USER << kCharInfo.m_class_type;
				pAgreeUserParent->RelayPacket( kPacket );
			}
			g_DBClient.OnUpdateCharRentalTime( dwAgreeDBAgentID, szAgreeID.GetHashCode(), kRentalData.m_dwCharIndex, 0 );
		}
		return;
	}

	CHARACTER kCharInfo;
	rkPacket >> kCharInfo;

	RentalData kRentalData;
	kRentalData.ApplyData( rkPacket );

	pTargetUser->_OnUserCharRentalAgree( g_UserNodeManager.GetGlobalUserNode( szAgreeID ), szAgreeID, dwAgreeDBAgentID, kCharInfo, kRentalData );
}

void ServerNode::OnJoinHeadquartersUser( SP2Packet &rkPacket )
{
	bool bInvited;
	int iMapIndex;
	DWORD dwRequestUser, dwOwnerUser;
	rkPacket >> dwRequestUser >> dwOwnerUser >> iMapIndex >> bInvited;

	User *pUser = GetUserOriginalNode( dwOwnerUser, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwOwnerUser ) == NULL )
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
			if( pUserParent )
			{
				SP2Packet kPacket( STPK_JOIN_HEADQUARTERS );
				kPacket << JOIN_HEADQUARTERS_OWNER_OFFLINE;
				pUserParent->RelayPacket( kPacket );
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnJoinHeadquartersUser User NULL(%d)", dwOwnerUser );
		}
		return;
	}
	pUser->_OnJoinHeadquarters( g_UserNodeManager.GetGlobalUserNode( dwRequestUser ), iMapIndex, bInvited );
}

void ServerNode::OnHeadquartersInfo( SP2Packet &rkPacket )
{
	ioHashString szOwnerName;
	DWORD dwRequestUser, dwOwnerUser;
	rkPacket >> dwRequestUser >> dwOwnerUser >> szOwnerName;

	User *pUser = GetUserOriginalNode( dwOwnerUser, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwOwnerUser ) == NULL )
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
			if( pUserParent )
			{
				SP2Packet kPacket( STPK_HEADQUARTERS_INFO );
				kPacket << szOwnerName << (MAX_PLAYER / 2) << false << 0;       // - 
				pUserParent->RelayPacket( kPacket );
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnHeadquartersInfo User NULL(%d)", dwOwnerUser );
		}
		return;
	}
	pUser->_OnHeadquartersInfo( g_UserNodeManager.GetGlobalUserNode( dwRequestUser ) );
}

void ServerNode::OnHeadquartersRoomInfo( SP2Packet &rkPacket )
{
	int iRoomIndex;
	rkPacket >> iRoomIndex;

	bool bLock;
	DWORD dwRequestUser;
	ioHashString szMasterName;
	rkPacket >> szMasterName >> bLock >> dwRequestUser;
	
	Room *pRoom = g_RoomNodeManager.GetHeadquartersNode( iRoomIndex );
	if( pRoom == NULL )
	{
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
		if( pUserParent )
		{
			SP2Packet kPacket( STPK_HEADQUARTERS_INFO );
			kPacket << szMasterName << (MAX_PLAYER / 2) << bLock << 0;       // - 
			pUserParent->RelayPacket( kPacket );
		}
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnHeadquartersRoomInfo Room NULL(%d)", iRoomIndex );
	}
	else
	{
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
		if( pUserParent )
		{
			SP2Packet kPacket( STPK_HEADQUARTERS_INFO );
			pRoom->FillHeadquartersInfo( szMasterName, bLock, kPacket );
			pUserParent->RelayPacket( kPacket );
		}
	}
}

void ServerNode::OnHeadquartersJoinAgree( SP2Packet &rkPacket )
{
	DWORD dwUserIndex, dwRoomIndex;
	rkPacket >> dwUserIndex >> dwRoomIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		return;
	}

	pUser->_OnHeadquartersJoinAgree( dwRoomIndex );
}

void ServerNode::OnLogoutRoomAlarm( SP2Packet &rkPacket )
{
	int iRoomIndex;
	ioHashString szMasterName;
	rkPacket >> iRoomIndex >> szMasterName;
	Room *pRoom = g_RoomNodeManager.GetHeadquartersNode( iRoomIndex );
	if( pRoom )
	{
		pRoom->OnModeLogoutAlarm( szMasterName );
	}
}

void ServerNode::OnUDPRecvTimeOut( SP2Packet &rkPacket )
{
	ioHashString szSenderID, szTargetID;
	DWORD dwSenderIndex, dwSenderBattleRoom;
	rkPacket >> szSenderID >> dwSenderIndex >> szTargetID >> dwSenderBattleRoom;

	User *pUser = GetUserOriginalNode( szTargetID, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( szTargetID ) == NULL )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUDPRecvTimeOut User NULL(%s)", szTargetID.c_str() );
		}
		return;
	}

	if( pUser->IsRoomLoadingOrServerMoving() )
	{
		// �ε����̹Ƿ� �ٽ� ���� ��û
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnUDPRecvTimeOut User Loading... : %s", szTargetID.c_str() );

		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwSenderIndex );
		if( pUserParent )
		{
			SP2Packet kPacket( STPK_UDP_RECV_TIMEOUT_RECHECK );
			kPacket << szTargetID;
			pUserParent->RelayPacket( kPacket );
		}
		else
		{

			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnUDPRecvTimeOut Sender User NULL(%s)", szSenderID.c_str() );
		}
	}
	else
	{
		SP2Packet kPacket( STPK_UDP_RECV_TIMEOUT );
		kPacket << szSenderID;
		pUser->SendMessage( kPacket );

		// ������ ī��Ʈ ����
		BattleRoomParent *pBattleParent = g_BattleRoomManager.GetGlobalBattleRoomNode( dwSenderBattleRoom );
		if( pBattleParent )
		{			
			pBattleParent->UserP2PRelayInfo( dwSenderIndex, pUser->GetUserIndex(), true );
		}
	}
}

void ServerNode::OnServerAlarmMent( SP2Packet &rkPacket )
{
	SP2Packet kPacket( SUPK_SERVER_ALARM_MENT );
	kPacket.SetDataAdd( (char*)rkPacket.GetData(), rkPacket.GetDataSize() );
	g_UserNodeManager.SendUDPMessageAll( kPacket );
}

bool ServerNode::OnWebUDPParse( SP2Packet &rkPacket )
{
	switch( rkPacket.GetPacketID() )
	{
	case SSTPK_WEB_EVENT:
	case SSTPK_WEB_REFRESH_BLOCK:
	case SSTPK_WEB_GET_CASH:
	case SSTPK_WEB_REFRESH_USER_ENTRY:
		break; // �Ʒ���
	default:
		return false;
	}
	
	DWORD dwUserIndex = 0;
	rkPacket >> dwUserIndex;
	User *pUser =  GetUserOriginalNode( dwUserIndex, rkPacket );
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnWebUDPParse - Not User :%d", dwUserIndex );
		return true;
	}

	switch( rkPacket.GetPacketID() )
	{
	case SSTPK_WEB_EVENT:
		pUser->OnWebEvent( rkPacket );
		return true;
	case SSTPK_WEB_REFRESH_BLOCK:
		pUser->OnWebRefreshBlock( rkPacket );
		return true;
	case SSTPK_WEB_GET_CASH:
		pUser->OnWebGetCash( rkPacket );
		return true;
	case SSTPK_WEB_REFRESH_USER_ENTRY:
		pUser->OnWebRefreshUserEntry( rkPacket );
		return true;
	}

	return false;
}

bool ServerNode::OnBillingParse( SP2Packet &rkPacket )
{
	switch( rkPacket.GetPacketID() )
	{
	case SSTPK_GET_CASH_RESULT:
	case SSTPK_OUTPUT_CASH_RESULT:
	case SSTPK_BILLING_LOGIN_RESULT:
	case SSTPK_BILLING_REFUND_CASH_RESULT:
	case SSTPK_BILLING_AUTOUPGRADE_LOGIN_RESULT:
	case SSTPK_BILLING_PCROOM_RESULT:
	case SSTPK_BILLING_OTP_RESULT:
	case SSTPK_BILLING_GET_MILEAGE_RESULT:
	case SSTPK_BILLING_ADD_MILEAGE_RESULT:
	case SSTPK_BILLING_IPBONUS_RESULT:
	case SSTPK_BILLING_ADD_CASH_RESULT:
	case SSTPK_BILLING_FILL_CASH_URL_RESULT:
	case SSTPK_BILLING_SUBSCRIPTION_RETRACT_CHECK_RESULT:
	case SSTPK_BILLING_SUBSCRIPTION_RETRACT_RESULT:
	case SSTPK_SESSION_CONTROL_RESULT:
	case SSTPK_TIMEOUT_BILLINGGUID:
	case SSTPK_BSTPK_GA_ID_RESULT:
	case SSTPK_REQUEST_TIME_CASH_RESULT:
		break; // �Ʒ���
	default:
		return false;
	}
	
	DWORD dwUserIndex = 0;
	ioHashString szPrivateID;
	User *pUser       = NULL;
	if( rkPacket.GetPacketID() == SSTPK_BILLING_LOGIN_RESULT     || 
		rkPacket.GetPacketID() == SSTPK_BSTPK_GA_ID_RESULT )
	{
		rkPacket >> szPrivateID;
		pUser =  GetUserOriginalNodeByPrivateID( szPrivateID, rkPacket );

		//HRYOON 20150112
//#if SRC_TH
//		rkPacket >> szPrivateID;
//		pUser =  g_UserNodeManager.GetUserNodeByGUID( szPrivateID );
//#else 
//		rkPacket >> szPrivateID;
//		pUser =  GetUserOriginalNodeByPrivateID( szPrivateID, rkPacket );
//#endif
	}
	else if( rkPacket.GetPacketID() == SSTPK_BILLING_USER_INFO_RESULT ||
		 rkPacket.GetPacketID() == SSTPK_BILLING_AUTOUPGRADE_LOGIN_RESULT || 
		 rkPacket.GetPacketID() == SSTPK_BILLING_OTP_RESULT || 
		 rkPacket.GetPacketID() == SSTPK_BILLING_IPBONUS_RESULT )
	{
		rkPacket >> szPrivateID;
		pUser =  GetUserOriginalNodeByPrivateID( szPrivateID, rkPacket );
	}
	else
	{
		rkPacket >> dwUserIndex;
		pUser =  GetUserOriginalNode( dwUserIndex, rkPacket );
	}

	if( !pUser )
	{
		ioHashString szBillingGUID;
		rkPacket >> szBillingGUID;
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBillingParse - Not User :%d:%s:0x%x", dwUserIndex, szBillingGUID.c_str(), rkPacket.GetPacketID() );
		return true;
	}

	switch( rkPacket.GetPacketID() )
	{
	case SSTPK_GET_CASH_RESULT:
		pUser->OnBillingGetCash( rkPacket );
		return true;
	case SSTPK_OUTPUT_CASH_RESULT:
		pUser->OnBillingOutputCash( rkPacket );
		return true;
	case SSTPK_BILLING_LOGIN_RESULT:
		pUser->OnBillingLogin( rkPacket );
		return true;
	case SSTPK_BILLING_REFUND_CASH_RESULT:
		pUser->OnBillingRefundCash( rkPacket );
		return true;
	case SSTPK_BILLING_USER_INFO_RESULT:
		pUser->OnBillingUserInfo( rkPacket );
		return true;
	case SSTPK_BILLING_AUTOUPGRADE_LOGIN_RESULT:
		pUser->OnBillingAutoUpgradeLogin( rkPacket );
		return true;
	case SSTPK_BILLING_PCROOM_RESULT:
		pUser->OnBillingPCRoom( rkPacket );
		return true;
	case SSTPK_BILLING_OTP_RESULT:
		pUser->OnBillingAutoUpgradeOTP( rkPacket );
		return true;
	case SSTPK_BILLING_GET_MILEAGE_RESULT:
		pUser->OnBillingGetMileage( rkPacket );
		return true;
	case SSTPK_BILLING_ADD_MILEAGE_RESULT:
		pUser->OnBillingAddMileage( rkPacket );
		return true;
	case SSTPK_BILLING_IPBONUS_RESULT:
		pUser->OnBillingIPBonus( rkPacket );
		return true;
	case SSTPK_BILLING_ADD_CASH_RESULT:
		pUser->OnBillingAddCash( rkPacket );
		return true;
	case SSTPK_BILLING_FILL_CASH_URL_RESULT:
		pUser->OnBillingFillCashUrl( rkPacket );
		return true;
	case SSTPK_BILLING_SUBSCRIPTION_RETRACT_CHECK_RESULT:
		pUser->OnBillingSubscriptionRetractCheck( rkPacket );
		return true;
	case SSTPK_BILLING_SUBSCRIPTION_RETRACT_RESULT:
		pUser->OnBillingSubscriptionRetract( rkPacket );
		return true;
	case SSTPK_SESSION_CONTROL_RESULT:
		pUser->OnSessionControl(rkPacket);
		return true;
	case SSTPK_TIMEOUT_BILLINGGUID:
		pUser->OnBillingTimeoutBillingGUID( rkPacket );
		return true;
	case SSTPK_BSTPK_GA_ID_RESULT:
		pUser->OnBillingDecodeGarenaToken( rkPacket );
		return true;
	case SSTPK_REQUEST_TIME_CASH_RESULT:
		pUser->OnBillingTimeCashResult(rkPacket);
		return true;
	}

	return false;
}

void ServerNode::OnWholeChat( SP2Packet &rkPacket )
{
	ioHashString szID;
	ioHashString szChat;
	rkPacket >> szID;
	rkPacket >> szChat;

	SP2Packet kPacket( STPK_WHOLE_CHAT );
	kPacket << szID;
	kPacket << szChat;
	g_UserNodeManager.SendMessageAll( kPacket );	
}

void ServerNode::OnTradeCreate( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeCreate ���� NULL : %d", dwUserIndex );
		return;
	}

	int iExtraValue;
	rkPacket >> iExtraValue;

	ioUserExtraItem *pExtraItem = pUser->GetUserExtraItem();
	if( pExtraItem )
	{
		pExtraItem->DeleteExtraItem( iExtraValue );

		SP2Packet kPacket( STPK_TRADE_CREATE );
		kPacket << TRADE_CREATE_DEL;
		kPacket << iExtraValue;
		pUser->SendMessage( kPacket );
	}
}

void ServerNode::OnTradeCreateComplete( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;

//	rkPacket >> dwUserIndex;
	PACKET_GUARD_VOID( rkPacket.Read( dwUserIndex ) );

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeCreateComplete ���� NULL : %d", dwUserIndex );
		return;
	}

	DWORD dwTradeIndex, dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	DWORD dwRegisterPeriod, dwDate1, dwDate2;

// 	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom >> iItemPrice
// 			 >> dwRegisterPeriod >> dwDate1 >> dwDate2;

	PACKET_GUARD_VOID( rkPacket.Read( dwTradeIndex ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwItemType ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwItemMagicCode ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwItemValue ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwItemMaleCustom ) );

	PACKET_GUARD_VOID( rkPacket.Read( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( rkPacket.Read( iItemPrice ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwDate1 ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwDate2 ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwRegisterPeriod ) );

	SP2Packet kPacket( STPK_TRADE_CREATE );
// 	kPacket << TRADE_CREATE_OK;
// 	kPacket << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom << iItemPrice;

	PACKET_GUARD_VOID( kPacket.Write( TRADE_CREATE_OK ) );

	PACKET_GUARD_VOID( kPacket.Write( dwUserIndex ) );
	PACKET_GUARD_VOID( kPacket.Write( dwTradeIndex ) );
	PACKET_GUARD_VOID( kPacket.Write( dwItemType ) );
	PACKET_GUARD_VOID( kPacket.Write( dwItemMagicCode ) );
	PACKET_GUARD_VOID( kPacket.Write( dwItemValue ) );

	PACKET_GUARD_VOID( kPacket.Write( dwItemMaleCustom ) );
	PACKET_GUARD_VOID( kPacket.Write( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( kPacket.Write( iItemPrice ) );
	PACKET_GUARD_VOID( kPacket.Write( dwDate1 ) );
	PACKET_GUARD_VOID( kPacket.Write( dwDate2 ) );

	PACKET_GUARD_VOID( kPacket.Write( dwRegisterPeriod ) );

	pUser->SendMessage( kPacket );
}

void ServerNode::OnTradeCreateFail( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeCreateFail ���� NULL : %d", dwUserIndex );
		return;
	}

	SP2Packet kPacket( STPK_TRADE_CREATE );
	kPacket << TRADE_CREATE_FAIL;
	pUser->SendMessage( kPacket );
}

void ServerNode::OnTradeItemComplete( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	int iType;
	rkPacket >> dwUserIndex >> iType;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeItemComplete ���� NULL : %d", dwUserIndex );
		return;
	}

	switch( iType )
	{
	case TRADE_S_GET_INFO_OK:
		OnTradeGetInfoOK( pUser, rkPacket );
		break;
	case TRADE_S_GET_INFO_FAIL:
		OnTradeGetInfoFail( pUser, rkPacket );
		break;
	case TRADE_S_BUY_COMPLETE:
		OnTradeBuyComplete( pUser, rkPacket );
		break;
	case TRADE_S_SELL_COMPLETE:
		OnTradeSellComplete( pUser, rkPacket );
		break;
	}
}

void ServerNode::OnTradeGetInfoOK( User *pUser, SP2Packet &rkPacket )
{
	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex, dwBuyUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex >> dwBuyUserIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue;
	rkPacket >> dwItemMaleCustom >> dwItemFemaleCustom >> iItemPrice;

	// �������
	__int64 iCurPeso = pUser->GetMoney();
	__int64 iResultPeso = iItemPrice + (iItemPrice * g_TradeInfoMgr.GetBuyTexRate());		// ������ ���

	if( iCurPeso < iResultPeso )
	{
		// ���μ����� ��������
		SP2Packet kPacket( MSTPK_TRADE_ITEM_TRADE );
		PACKET_GUARD_VOID( kPacket.Write( TRADE_ITEM_TRADE_FAIL ) );
		PACKET_GUARD_VOID( kPacket.Write( dwTradeIndex ) );

		g_MainServer.SendMessage( kPacket );

		// Ŭ���̾�Ʈ�� ��������
		SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );
		kSuccess << TRADE_BUY_PESO;
		pUser->SendMessage( kSuccess );
		return;
	}
	else
	{
		// ��� ����
		pUser->RemoveMoney( iResultPeso );
		g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_PESO_CONSUME, pUser, 0, 0, LogDBClient::LET_ITEM, LogDBClient::PT_TRADE, PRESENT_EXTRAITEM, dwItemMagicCode, iResultPeso, NULL);
	}

	// DB�� ������û
	g_DBClient.OnTradeItemComplete( pUser->GetUserDBAgentID(),
									pUser->GetAgentThreadID(),
									dwBuyUserIndex,
									dwRegisterUserIndex,
									szRegisterUserName,
									dwTradeIndex,
									dwItemType,
									dwItemMagicCode,
									dwItemValue,
									dwItemMaleCustom,
									dwItemFemaleCustom,
									iItemPrice );

	char szNote[MAX_PATH]="";
	StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

	g_LogDBClient.OnInsertTrade( dwBuyUserIndex, pUser->GetPublicID(),
								 dwTradeIndex,
								 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_BUY,
								 pUser->GetPublicIP(), szNote );
}

void ServerNode::OnTradeGetInfoFail( User *pUser, SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;

	int iFailType = TRADE_BUY_ERROR;
	switch( iResult )
	{
	case TRADE_ITEM_NO_ITEM:
		iFailType = TRADE_BUY_NO_ITEM;
		break;
	case TRADE_ITEM_PESO:
		iFailType = TRADE_BUY_PESO;
		break;
	case TRADE_ITEM_OWNER:
		iFailType = TRADE_BUY_OWNER;
		break;
	}

	SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );
	kSuccess << iFailType;
	pUser->SendMessage( kSuccess );
}

void ServerNode::OnTradeBuyComplete( User *pUser, SP2Packet &rkPacket )
{
	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex, dwBuyUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex >> dwBuyUserIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue;
	rkPacket >> dwItemMaleCustom >> dwItemFemaleCustom >> iItemPrice;

	g_PresentHelper.SendPresentByTradeItemBuy( pUser->GetUserDBAgentID(), 
											   pUser->GetAgentThreadID(),
											   dwRegisterUserIndex,
											   dwBuyUserIndex,
											   dwItemType,
											   dwItemMagicCode,
											   dwItemValue,
											   dwItemMaleCustom,
											   dwItemFemaleCustom,
											   szRegisterUserName );

	// DB���� ���� ���� ���������ϰ�, ���� ����.
	pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

	// ���μ����� ���� ��û
	SP2Packet kPacket( MSTPK_TRADE_ITEM_TRADE );
	PACKET_GUARD_VOID( kPacket.Write( TRADE_ITEM_DEL ) );
	PACKET_GUARD_VOID( kPacket.Write( dwTradeIndex ) );

	g_MainServer.SendMessage( kPacket );

	// ������ ���
	__int64 iResultPeso = iItemPrice;
	SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );

// 	kSuccess << TRADE_BUY_OK;
// 	kSuccess << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
// 	kSuccess << iResultPeso;

	PACKET_GUARD_VOID( kSuccess.Write( TRADE_BUY_OK ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwTradeIndex ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemType ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMagicCode ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemValue ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( iResultPeso ) );

	pUser->SendMessage( kSuccess );
}

void ServerNode::OnTradeSellComplete( User *pUser, SP2Packet &rkPacket )
{
	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex, dwBuyUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex >> dwBuyUserIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
	rkPacket >> iItemPrice;

	// ���������� ��� ���
	int iTradePrice = iItemPrice;
	g_PresentHelper.SendPresentByTradeItemSell( pUser->GetUserDBAgentID(),
												pUser->GetAgentThreadID(),
												dwBuyUserIndex,
												dwRegisterUserIndex,
												iTradePrice,
												szRegisterUserName );

	// DB���� ���� ���� ���������ϰ�, Ŭ���̾�Ʈ�� ���� ����.
	pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

	SP2Packet kSuccess( STPK_TRADE_ITEM_COMPLETE );

// 	kSuccess << TRADE_SELL_OK;
// 	kSuccess << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
// 	kSuccess << iTradePrice;

	PACKET_GUARD_VOID( kSuccess.Write( TRADE_SELL_OK ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwTradeIndex ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemType ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMagicCode ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemValue ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( iTradePrice ) );

	pUser->SendMessage( kSuccess );
}

void ServerNode::OnTradeCancel( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	int iType;
	rkPacket >> dwUserIndex >> iType;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeCancel ���� NULL : %d", dwUserIndex );
		return;
	}

	switch( iType )
	{
	case TRADE_CANCEL_S_GET_INFO_OK:
		OnTradeCancelInfoOK( pUser, rkPacket );
		break;
	case TRADE_CANCEL_S_GET_INFO_FAIL:
		OnTradeCancelInfoFail( pUser, rkPacket );
		break;
	case TRADE_CANCEL_S_CANCEL_COMPLETE:
		OnTradeCancelComplete( pUser, rkPacket );
		break;
	}
}

void ServerNode::OnTradeCancelInfoOK( User *pUser, SP2Packet &rkPacket )
{
	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
	rkPacket >> iItemPrice;

	TradeLOG.PrintTimeAndLog( LOG_TEST_LEVEL, "TradeCancel - [TradeDelete] [RequestUser:%d] [RegisterUser:%d] [TradeIndex:%d] [Type:%d] [Code:%d] [Value:%d] [Custom:%d.%d]",
								  pUser->GetUserIndex(), dwRegisterUserIndex, dwTradeIndex, dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom );

	// DB�� ������û
	g_DBClient.OnTradeItemCancel( pUser->GetUserDBAgentID(),
								  pUser->GetAgentThreadID(),
								  dwRegisterUserIndex,
								  szRegisterUserName,
								  dwTradeIndex,
								  dwItemType,
								  dwItemMagicCode,
								  dwItemValue,
								  dwItemMaleCustom,
								  dwItemFemaleCustom,
								  iItemPrice );

	char szNote[MAX_PATH]="";
	StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

	g_LogDBClient.OnInsertTrade( dwRegisterUserIndex, pUser->GetPublicID(),
								 dwTradeIndex,
								 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_CANCEL,
								 pUser->GetPublicIP(), szNote );
}

void ServerNode::OnTradeCancelInfoFail( User *pUser, SP2Packet &rkPacket )
{
	int iResult;
	rkPacket >> iResult;

	int iFailType = TRADE_CANCEL_ERROR;
	switch( iResult )
	{
	case TRADE_ITEM_CANCEL_NO_ITEM:
		iFailType = TRADE_CANCEL_NO_ITEM;
		break;
	case TRADE_ITEM_CANCEL_NOT_OWNER:
		iFailType = TRADE_CANCEL_NOT_OWNER;
		break;
	}

	SP2Packet kSuccess( STPK_TRADE_CANCEL );
	kSuccess << iFailType;
	pUser->SendMessage( kSuccess );
}

void ServerNode::OnTradeCancelComplete( User *pUser, SP2Packet &rkPacket )
{
	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	__int64 iItemPrice;
	ioHashString szRegisterUserName;

	rkPacket >> dwTradeIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
	rkPacket >> iItemPrice;

	g_PresentHelper.SendPresentByTradeCancel( pUser->GetUserDBAgentID(),
											  pUser->GetAgentThreadID(),
											  dwRegisterUserIndex,
											  dwRegisterUserIndex,
											  dwItemType,
											  dwItemMagicCode,
											  dwItemValue,
											  dwItemMaleCustom,
											  dwItemFemaleCustom,
											  szRegisterUserName );

	// DB���� ���� ���� ���������ϰ�, Ŭ���̾�Ʈ�� ���� ����.
	pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

	// ���μ����� ���� ��û
	SP2Packet kPacket( MSTPK_TRADE_ITEM_CANCEL );
	kPacket << TRADE_ITEM_CANCEL_DEL;
	kPacket << dwTradeIndex;
	g_MainServer.SendMessage( kPacket );

	SP2Packet kSuccess( STPK_TRADE_CANCEL );

// 	kSuccess << TRADE_CANCEL_COMPLETE;
// 	kSuccess << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
// 	kSuccess << iItemPrice;

	PACKET_GUARD_VOID( kSuccess.Write( TRADE_CANCEL_COMPLETE ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwTradeIndex ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemType ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMagicCode ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemValue ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( iItemPrice ) );

	pUser->SendMessage( kSuccess );
}

void ServerNode::OnTradeTimeOut( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;

	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnTradeTimeOut ���� NULL : %d", dwUserIndex );
		return;
	}

	DWORD dwTradeIndex;
	DWORD dwRegisterUserIndex;
	DWORD dwItemType, dwItemMagicCode, dwItemValue, dwItemMaleCustom, dwItemFemaleCustom;
	ioHashString szRegisterUserName;
	__int64 iItemPrice;

	rkPacket >> dwTradeIndex;
	rkPacket >> dwRegisterUserIndex >> szRegisterUserName;
	rkPacket >> dwItemType >> dwItemMagicCode >> dwItemValue >> dwItemMaleCustom >> dwItemFemaleCustom;
	rkPacket >> iItemPrice;

	g_PresentHelper.SendPresentByTradeTimeOut( pUser->GetUserDBAgentID(),
											   pUser->GetAgentThreadID(),
											   dwRegisterUserIndex,
											   dwRegisterUserIndex,
											   dwItemType,
											   dwItemMagicCode,
											   dwItemValue,
											   dwItemMaleCustom,
											   dwItemFemaleCustom,
											   szRegisterUserName );

	char szNote[MAX_PATH]="";
	StringCbPrintf( szNote, sizeof( szNote ) , "[%d-1 : %d,%d]", dwTradeIndex, dwItemMaleCustom, dwItemFemaleCustom );

	g_LogDBClient.OnInsertTrade( dwRegisterUserIndex, pUser->GetPublicID(),
								 dwTradeIndex,
								 dwItemType, dwItemMagicCode, dwItemValue, iItemPrice, LogDBClient::TST_TIMEOUT,
								 pUser->GetPublicIP(), szNote );


	// DB���� ���� ���� ���������ϰ�, Ŭ���̾�Ʈ�� ���� ����.
	pUser->_OnSelectPresent( 30 );     // ���� ���� ���� �ִ� 30�� ��û

	SP2Packet kSuccess( STPK_TRADE_TIME_OUT );

// 	kSuccess << dwItemType << dwItemMagicCode << dwItemValue << dwItemMaleCustom << dwItemFemaleCustom;
// 	kSuccess << iItemPrice;

	PACKET_GUARD_VOID( kSuccess.Write( dwTradeIndex ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemType ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMagicCode ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemValue ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemMaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( dwItemFemaleCustom ) );
	PACKET_GUARD_VOID( kSuccess.Write( iItemPrice ) );

	pUser->SendMessage( kSuccess );
}

void ServerNode::OnDisconnectAlreadyID( SP2Packet &rkPacket )
{
	
	DWORD        dwUserIndex = 0;
	ioHashString szRequestGUID;
	rkPacket >> dwUserIndex;
	rkPacket >> szRequestGUID;

	
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s User NULL : %d", __FUNCTION__, dwUserIndex );
		return;
	}
	//hr �߰�
	pUser->SetLogoutType( 7 );
	pUser->CloseConnection();
	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "DisconnectAlreadyID[3]-%s-%s", pUser->GetPrivateID().c_str(), szRequestGUID.c_str() );
}

void ServerNode::OnEventItemInitialize( SP2Packet &rkPacket )
{
	int iType = 0;
	rkPacket >> iType;
	g_UserNodeManager.InitUserEventItem( iType );
}

void ServerNode::OnLoginConnect(SP2Packet &rkPacket)
{
	SetNodeType( NODE_TYPE_LOGIN );
}

void ServerNode::OnLoginRequestStatus() 
{
	int iServerIndex = g_ServerNodeManager.GetServerIndex();
	int iBlockState	= g_ServerNodeManager.IsBlocked() ? 1 : 0;
	int iUserCount	= g_UserNodeManager.GetNodeSize();

	SP2Packet kPacket(LSTPK_STATUS_RESPONSE); //���⼭ public ip/userport�� ���� 
	kPacket << iServerIndex;
	kPacket << iBlockState;
	kPacket << iUserCount;
	kPacket << m_szPublicIP;
	kPacket << g_App.GetCSPort();

	SendMessage(kPacket);
}

void ServerNode::OnLoginBlockRequest(SP2Packet &rkPacket)
{
	// �α��μ����� ��û���� �ش缭���� ����Ѵ�
	int iServerIndex = 0, iBlockState = 0;
	rkPacket >> iServerIndex;
	rkPacket >> iBlockState;

	bool bBlockFlag = (iBlockState == 1) ? true : false;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLoginBlockRequest : %d, %d", iServerIndex, iBlockState );
	if( 0 == iServerIndex ) // ��缭�� ��
	{
		// ���缭�� ��
		g_ServerNodeManager.SetBlockState( bBlockFlag );

		// �ٸ����� ����� ��
		static vServerIndex vServerIndexes;
		vServerIndexes.clear();

		g_ServerNodeManager.GetServerNodes(true, vServerIndexes);

		for(vServerIndex::iterator it = vServerIndexes.begin() ; it != vServerIndexes.end() ; ++it)
		{
			ServerNode *pServerNode = g_ServerNodeManager.GetServerNode( *it );
			if( !pServerNode ) continue;

			pServerNode->SetBlockState( bBlockFlag );
		}
	}
	else
	{
		if(g_ServerNodeManager.GetServerIndex() == iServerIndex)
		{
			g_ServerNodeManager.SetBlockState( bBlockFlag );

			// ��� �������� ��ϻ��¸� �˸�
			g_UserNodeManager.AllUserLeaveServer(bBlockFlag);
		}
		else
		{
			ServerNode *pServerNode = g_ServerNodeManager.GetServerNode( iServerIndex );
			if( pServerNode )
			{
				pServerNode->SetBlockState( bBlockFlag );
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnLoginBlockRequest not exist : %d, %d", iServerIndex, iBlockState );
			}
		}
	}
}

void ServerNode::OnLoginPartitionRequest(SP2Packet &rkPacket)
{
	int iPartitionCount = 0;
	rkPacket >> iPartitionCount;

	// �ּ� 1���� ��Ƽ���� �����ؾ� �Ѵ�
	if(iPartitionCount > 1 )
	{
		// ��Ƽ�� �籸�� �۾�
		g_ServerNodeManager.RearrangePartition( iPartitionCount );
	}
}

void ServerNode::OnTournamentCreateTeam( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		SHORT Position;
		ioHashString kTeamName;
		int iCheerPoint, iLadderPoint;
		BYTE MaxPlayer, CampPos, TourPos;
		DWORD dwTourIndex, dwTeamIndex, dwOwnerIndex;
		rkPacket >> dwTourIndex >> dwTeamIndex >> kTeamName >> dwOwnerIndex;
		rkPacket >> Position >> MaxPlayer >> iCheerPoint >> TourPos >> iLadderPoint >> CampPos;

		ioUserTournament *pTournament = pUser->GetUserTournament();
		if( pTournament )
		{
			pTournament->CreateTeamData( dwTourIndex, dwTeamIndex, dwOwnerIndex, kTeamName, Position, TourPos );

			//
			SP2Packet kPacket( STPK_TOURNAMENT_TEAM_CREATE );
			kPacket << MS_TOURNAMENT_TEAM_CREATE_OK << dwTourIndex << dwTeamIndex << kTeamName << dwOwnerIndex 
					<< Position << MaxPlayer << iCheerPoint << TourPos << iLadderPoint << CampPos;
			pUser->SendMessage( kPacket );
		}
	}
}

void ServerNode::OnTournamentTeamAgreeOK( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		ioUserTournament *pUserTournament = pUser->GetUserTournament();
		if( pUserTournament )
		{
			DWORD dwTeamIndex;
			ioUserTournament::TeamUserData kUserData;
			rkPacket >> dwTeamIndex >> kUserData.m_dwTableIndex >> kUserData.m_dwUserIndex >> kUserData.m_szNick;
			rkPacket >> kUserData.m_iGradeLevel >> kUserData.m_iLadderPoint >> kUserData.m_dwGuildIndex; 
			pUserTournament->AddTeamUserData( dwTeamIndex, kUserData );
		}
	}
}

void ServerNode::OnTournamentTeamJoin( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		ioUserTournament *pUserTournament = pUser->GetUserTournament();
		if( pUserTournament )
		{
			ioHashString szTeamName;
			BYTE LeaguePos, TourPos;
			DWORD dwTourIndex, dwTeamIndex, dwOwnerIndex;
			rkPacket >> dwTourIndex >> dwTeamIndex >> szTeamName >> dwOwnerIndex >> LeaguePos >> TourPos;				
			if( pUserTournament->JoinTeamData( dwTourIndex, dwTeamIndex, dwOwnerIndex, szTeamName, LeaguePos, TourPos ) )
			{
				SP2Packet kPacket( STPK_TOURNAMENT_TEAM_JOIN );
				kPacket << dwTourIndex << dwTeamIndex << szTeamName << dwOwnerIndex << LeaguePos << TourPos;					
				pUser->SendMessage( kPacket );
			}
		}
	}
}

void ServerNode::OnTournamentTeamLeave( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		ioUserTournament *pUserTournament = pUser->GetUserTournament();
		if( pUserTournament )
		{
			DWORD dwSenderIndex, dwTeamIndex, dwLeaveIndex;
			rkPacket >> dwSenderIndex >> dwTeamIndex >> dwLeaveIndex;

			pUserTournament->LeaveTeamUser( dwSenderIndex, dwTeamIndex, dwLeaveIndex );
		}
	}
}

void ServerNode::OnPresentInsert( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		ioHashString kSendID, kLogMent;
		SHORT        iPresentType, iPresentMent, iPresentState;
		int          iPresentValue1, iPresentValue2, iPresentDay;
		rkPacket >> kSendID >> iPresentType >> iPresentValue1 >> iPresentValue2 >> iPresentMent >> iPresentDay >> iPresentState >> kLogMent;

		CTimeSpan cPresentGapTime( iPresentDay, 0, 0, 0 );
		CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;
		pUser->AddPresentMemory( kSendID, iPresentType, iPresentValue1, iPresentValue2, 0, 0, iPresentMent, kPresentTime, iPresentState );
		g_LogDBClient.OnInsertPresent( 0, kSendID, g_App.GetPublicIP().c_str(), dwUserIndex, iPresentType, iPresentValue1, iPresentValue2, 0, 0, LogDBClient::PST_RECIEVE, kLogMent.c_str() );
		pUser->SendPresentMemory();
	}
}

void ServerNode::OnPresentInsertByEtcItem( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	rkPacket >> dwUserIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser )
	{
		DWORD dwSendUserIndex;
		ioHashString kSendUserID, kSendUserIP, kLogMent;
		int iPresentType, iPresentMent, iPresentState, iPresentPeriod;
		int iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4;

		rkPacket >> dwSendUserIndex;
		rkPacket >> kSendUserID;
		rkPacket >> kSendUserIP;
		rkPacket >> iPresentPeriod;
		rkPacket >> iPresentType;
		rkPacket >> iPresentValue1;
		rkPacket >> iPresentValue2;
		rkPacket >> iPresentValue3;
		rkPacket >> iPresentValue4;
		rkPacket >> iPresentMent;
		rkPacket >> iPresentState;
		rkPacket >> kLogMent;

		CTimeSpan cPresentGapTime( iPresentPeriod, 0, 0, 0 );
		CTime kPresentTime = CTime::GetCurrentTime() + cPresentGapTime;

		pUser->AddPresentMemory( kSendUserID, iPresentType, iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4, iPresentMent, kPresentTime, iPresentState );

		g_LogDBClient.OnInsertPresent( dwSendUserIndex, kSendUserID, kSendUserIP.c_str(), pUser->GetUserIndex(), iPresentType,
									   iPresentValue1, iPresentValue2, iPresentValue3, iPresentValue4,
									   LogDBClient::PST_RECIEVE, kLogMent.c_str() );
		pUser->SendPresentMemory();
	}
}

//! ģ���� �ٸ��������� Ŭ�ι� ����.
void ServerNode::OnCloverSend( SP2Packet& rkPacket )
{
	// Declare) A -> B. 
	DWORD dwSendUserIndex;
	DWORD dwFriendIndex;
	int iReceiveDate;

	// UserIndex : A / B
	rkPacket >> dwSendUserIndex >> dwFriendIndex >> iReceiveDate;

	// Get User : B
	User* pFriendUser = GetUserOriginalNode( dwFriendIndex, rkPacket );
	if( pFriendUser )
	{
		// Friend
		ioFriend* pTargetFriend = pFriendUser->GetFriend();
		pTargetFriend->SetFriendBeforeReceiveCloverData( dwSendUserIndex, Help::GetCloverSendCount(), iReceiveDate );

		// ���⼭ ó��.
		pFriendUser->CloverFriendInfo( dwSendUserIndex, ioClover::FRIEND_CLOVER_COME_TO_FRIEND );
	}
}

void ServerNode::OnRelayServerConnect(SP2Packet& kPacket)
{
	SetNodeType( NODE_TYPE_RELAY );
	int nSize = 0;
	kPacket >> m_szPublicIP;
	kPacket >> m_iServerPort; // ���� �� ���� ȣȯ�� 
	kPacket >> nSize;
	if(nSize == 0)
	{
		LOG.PrintTimeAndLog(0,"OnRelayServerConnect Error nsize Is NULL (%s)",m_szPublicIP);
	//	return;
		m_relayPorts.push_back(m_iServerPort); // ���� �ڵ�� ȣȯ���� ���� 
	}
	
	for(int i=0; i< nSize; ++i)
	{
		int nPortTemp = 0;
		kPacket >> nPortTemp;
		m_relayPorts.push_back(nPortTemp);
		
	}
	
	 
	g_Relay.AddRelayServerInfo(this);
	m_iRelayIndex = g_Relay.GetRelayServerIndex();

	SP2Packet pk(RSPTK_ON_CONNECT);

	pk << Help::IsWholeChatOn();
	pk << m_iRelayIndex;
	pk << g_ServerNodeManager.GetServerIndex();

	CConnectNode::SendMessage(pk,TRUE);

	g_Relay.SetUseRelayServer(TRUE);

	LOG.PrintTimeAndLog(0,"OnRelayServerConnect(%s:%d)",m_szPublicIP,m_iServerPort);
}

void ServerNode::OnRelayControl( SP2Packet & kPacket )
{
	LOG.PrintTimeAndLog( 0, "[err] relay svr error - %u", kPacket.GetPacketID() );
	return;

	if( !IsRelayNode() ) return;

	int ctype;
	kPacket >> ctype;
	switch(ctype)
	{
	case RS_INFO:
		{
			kPacket >> m_RelayInfo;
		}
		break;
	case RS_ON_ADD_USER:
		{
			RequestRelayServerConnect(kPacket);
		}
		break;
	case RS_CHANGE_ADDR:
		{
			OnChangeAddress(kPacket);
		}
		break;
	case RS_HACK_ANNOUNCE:
		{
			OnHackAnnounce(kPacket);
		}
		break;
	case RS_USER_GHOST:
		{
			OnUserGhost(kPacket);
		}
		break;
	case RS_RESERVER_ROOM_JOIN:
		{
			DWORD userIndex;
			kPacket >> userIndex;
			User* node = g_UserNodeManager.GetUserNode(userIndex);
			if(node)
				node->OnReserveRoomJoin(kPacket);
			else
				LOG.PrintTimeAndLog(0,"Error RS_RESERVER_ROOM_JOIN! UserIdnex NotFound(%d)",userIndex);

		}
		break;
	}
}

void ServerNode::RequestRelayServerConnect( SP2Packet & kPacket )
{
	char publicID[ID_NUM_PLUS_ONE];
	kPacket >> publicID;
//	LOG.PrintTimeAndLog(0,"Relay::%s:%d[%s]\n",m_szPublicIP,m_iServerPort,publicID);
//	Information("Relay::%s:%d[%s]\n",m_szPublicIP,m_iServerPort,publicID);
	RequestRelayServerConnect(publicID);
}

void ServerNode::RequestRelayServerConnect(const char* publicID)
{ //kyg ���⼭ �ϴ� �ѹ� �ٲ� 
	if(m_iServerPort != 0 )
	{
		User* node = g_UserNodeManager.GetUserNodeByPublicID(publicID);
		if(node)
		{
			Room* pRoom  = node->GetMyRoom();
			int nPort  = 0 ;
			if(pRoom)
			{
				Debug("RQ:%d :: \n",node->GetMyRoom()->GetRoomIndex());
				nPort = GetRelayServerPort(pRoom->GetRoomIndex());
			}
			nPort = (nPort != 0) ? nPort : GetRelayServerPort(0);
			SP2Packet pk(STPK_ON_CONTROL);
			int ctype = RC_USE_RELAYSVR;
			pk << ctype;
			pk << m_szPublicIP;
			pk << nPort;
			Debug("RelayServer Port Send : %d\n",nPort);
			node->SetRelayServerID(m_iRelayIndex);
			node->SendMessage(pk);
		}
	}
}

void ServerNode::OnChangeAddress( SP2Packet & kPacket )
{
	char szPublicID[ID_NUM_PLUS_ONE];
	char szIpaddr[STR_IP_MAX];	
	int  iPort;
	DWORD dwUserIndex;
	kPacket >> szPublicID;
	kPacket >> szIpaddr;
	kPacket >> iPort;
	kPacket >> dwUserIndex;
	User* node = g_UserNodeManager.GetUserNode(dwUserIndex);
	if(node)
	{
		char back_iip[16] = "";
		int  back_port = node->GetUDP_port();
		strcpy_s( back_iip, node->GetPublicIP() );
		LOG.PrintTimeAndLog(0,"OnChangeAddress::%s:%d to %s:%d\n",back_iip,back_port,szIpaddr,iPort);
		node->SetClientAddressForRelay(szIpaddr,iPort);
		node->SetClientAddr(back_iip,back_port);
	}
	else
	{
		LOG.PrintTimeAndLog(LOG_DEBUG_LEVEL,"Error RS_CAHNGE_ADDR userID: [%s] NotFound Node!",szPublicID);
	}
}

void ServerNode::OnHackAnnounce( SP2Packet & kPacket )
{
	DWORD userIndex = 0;
	int ht_speed = 0;
	int maxAnswer = 0;
	DWORD clientAnswerTime =0;
	int iFirstOperand = 0;
	int iSecondOperand = 0;
	int iOperator = 0; 

	kPacket >> userIndex >> ht_speed >> maxAnswer >> clientAnswerTime >> 
		iFirstOperand >> iSecondOperand >> iOperator;
	Debug("HACK::userindex:%d FirstOperand:%d\n",userIndex,iFirstOperand);
	User* node = g_UserNodeManager.GetUserNode(userIndex);
	if(node)
	{
		SP2Packet sp(STPK_HACK_ANNOUNCE);
		sp << ht_speed;
		sp << maxAnswer;
		sp << clientAnswerTime;
		sp << iFirstOperand;
		sp << iSecondOperand;
		sp << iOperator;
		node->SendMessage(sp);
	}
}

void ServerNode::OnUserGhost( SP2Packet & kPacket )
{
	DWORD userIndex;
	int CheckTime;
	kPacket >> userIndex;
	kPacket >> CheckTime;

	User* node = g_UserNodeManager.GetUserNode(userIndex);
	if(node)
		node->TimeOutClose(CheckTime);
}

int ServerNode::GetRelayServerPort( int num )
{
	int nIndex = 0;
	if(num == 0)
		nIndex = rand() % m_relayPorts.size();
	else
		nIndex = num % m_relayPorts.size();

	return m_relayPorts[nIndex];
}

bool ServerNode::IsSameBilling( DWORD serverIndex )
{
	int myIndex = g_ServerNodeManager.GetServerIndex() % g_BillingRelayServer.GetBillingServerCount();
	int moveIndex = serverIndex % g_BillingRelayServer.GetBillingServerCount();

	if(myIndex == moveIndex) return true;
	else return false;
}

void ServerNode::OnShuffleRoomTransfer( SP2Packet &rkPacket )
{
	int iTransferType, iShuffleRoomIndex;
	rkPacket >> iTransferType >> iShuffleRoomIndex;

	ShuffleRoomNode *pShuffleRoom = g_ShuffleRoomManager.GetShuffleRoomNode( iShuffleRoomIndex );	
	switch( iTransferType )
	{
	case ShuffleRoomParent::ENTER_USER:
		{
			DWORD dwUserIndex;
			int   iGradeLevel, iAbilityLevel, iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> iGradeLevel >> iAbilityLevel
					 >> szPublicIP >> szPrivateIP >> szTransferIP >> iClientPort >> iTransferPort;

			if( pShuffleRoom == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - ENTER_USER �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );

				SP2Packet kPacket( SSTPK_SHUFFLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_SHUFFLEROOM_JOIN_NOT_NODE << iShuffleRoomIndex;
				SendMessage( kPacket );
			}
			else if( pShuffleRoom->IsFull() )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - ENTER_USER �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );

				SP2Packet kPacket( SSTPK_SHUFFLEROOM_JOIN_RESULT );
				kPacket << dwUserIndex << USER_SHUFFLEROOM_JOIN_FULL_USER << iShuffleRoomIndex << pShuffleRoom->GetPlayUserCnt()
						<< pShuffleRoom->GetMaxPlayerBlue() << pShuffleRoom->GetMaxPlayerRed();
				SendMessage( kPacket );						
			}
			else
			{
				pShuffleRoom->EnterUser( dwUserIndex, szPublicID, iGradeLevel, iAbilityLevel, szPublicIP, szPrivateIP, szTransferIP, iClientPort, iTransferPort );
			}
		}
		break;
	case ShuffleRoomParent::LEAVE_USER:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - LEAVE_USER �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			ioHashString szPublicID;
			rkPacket >> dwUserIndex >> szPublicID;
			pShuffleRoom->LeaveUser( dwUserIndex, szPublicID );
		}
		break;
	case ShuffleRoomParent::TRANSFER_PACKET:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - TRANSFER_PACKET �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pShuffleRoom->SendPacketTcp( kPacket, dwUserIndex );
		}
		break;
	case ShuffleRoomParent::TRANSFER_PACKET_USER:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - TRANSFER_PACKET_USER �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			ioHashString szSenderName;
			DWORD dwPacketID;
			rkPacket >> szSenderName >> dwPacketID;
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 1 ) + lstrlen( szSenderName.c_str() ) + 1;
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pShuffleRoom->SendPacketTcpUser( kPacket, szSenderName );			
		}
		break;
	case ShuffleRoomParent::TRANSFER_PACKET_UDP:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - TRANSFER_PACKET_UDP �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			// ��Ŷ ����
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize );
			pShuffleRoom->SendPacketUdp( kPacket, dwUserIndex );
		}
		break;
	case ShuffleRoomParent::USER_INFO:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - USER_INFO �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			int   iGradeLevel, iAbilityLevel, iClientPort, iTransferPort;
			ioHashString szTransferIP;
			rkPacket >> dwUserIndex >> iGradeLevel >> iAbilityLevel >> iClientPort >> szTransferIP >> iTransferPort;
			pShuffleRoom->UserInfoUpdate( dwUserIndex, iGradeLevel, iAbilityLevel, iClientPort, szTransferIP, iTransferPort );
		}
		break;
	case ShuffleRoomParent::UDP_CHANGE:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - UDP_CHANGE �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex;
			int   iClientPort, iTransferPort;
			ioHashString szPublicID, szPublicIP, szPrivateIP, szTransferIP;
			rkPacket >> dwUserIndex >> szPublicID >> szPublicIP >> iClientPort >> szPrivateIP >> szTransferIP >> iTransferPort;
			pShuffleRoom->UserUDPChange( dwUserIndex, szPublicID, szPublicIP, iClientPort, szPrivateIP, szTransferIP, iTransferPort );
		}
		break;
	case ShuffleRoomParent::P2P_RELAY_INFO:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - P2P_RELAY_INFO �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}
			DWORD dwUserIndex, dwRelayUserIndex;
			bool  bServerRelay;
			rkPacket >> dwUserIndex >> dwRelayUserIndex >> bServerRelay;
			pShuffleRoom->UserP2PRelayInfo( dwUserIndex, dwRelayUserIndex, bServerRelay );
		}
		break;
	case ShuffleRoomParent::USER_PACKET_TRANSFER:
		{
			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - USER_PACKET_TRANSFER �߰��Ϸ��� ���÷� ���� %d", iShuffleRoomIndex );
				return;
			}

			DWORD dwUserIndex, dwPacketID;
			rkPacket >> dwUserIndex >> dwPacketID;	
			UserParent *pUser = g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
			if( pUser == NULL )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - USER_PACKET_TRANSFER ���� NULL : %d - 0x%x", dwUserIndex, dwPacketID );
				return;
			}
			// ��Ŷ ����
			int iMoveSize = ( sizeof(int) * 2 ) + ( sizeof(DWORD) * 2 );
			SP2Packet kPacket( dwPacketID );
			kPacket.SetDataAdd( (char*)rkPacket.GetData() + iMoveSize, rkPacket.GetDataSize() - iMoveSize, true );
			pShuffleRoom->OnProcessPacket( kPacket, pUser );
		}
		break;
	case ShuffleRoomParent::REJOIN_TRY:
		{
			DWORD dwUserIndex;
			ioHashString szPublicID;
			PACKET_GUARD_VOID( rkPacket.Read( dwUserIndex ) );
			PACKET_GUARD_VOID( rkPacket.Read( szPublicID ) );

			if( !pShuffleRoom )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomTransfer - REJOIN_TRY, %s, �߰��Ϸ��� ���÷� ���� %d", szPublicID.c_str(), iShuffleRoomIndex );
				return;
			}

			pShuffleRoom->ReJoinTry( dwUserIndex, szPublicID );
		}
		break;
	}
}

void ServerNode::OnShuffleRoomJoinResult( SP2Packet &rkPacket )
{
	DWORD dwUserIndex;
	int   iResultType, iShuffleRoomIndex;
	rkPacket >> dwUserIndex >> iResultType >> iShuffleRoomIndex;

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomJoinResult ���� NULL : %d", dwUserIndex );
		return;
	}

	pUser->_OnShuffleRoomJoinResult( iResultType );

	// ������� ����
	ShuffleRoomCopyNode *pShuffleRoom = GetShuffleRoomNode( iShuffleRoomIndex );
	if( pShuffleRoom )
	{
		switch( iResultType )
		{
		case USER_SHUFFLEROOM_JOIN_FULL_USER:
			{
				int iPlayUserCnt, iMaxPlayerBlue, iMaxPlayerRed;
				rkPacket >> iPlayUserCnt >> iMaxPlayerBlue >> iMaxPlayerRed;
				pShuffleRoom->SetJoinUserCnt( iPlayUserCnt );
				pShuffleRoom->SetMaxPlayer( iMaxPlayerBlue, iMaxPlayerRed );
			}
			break;
		}
	}
}

void ServerNode::OnShuffleRoomSync( SP2Packet &rkPacket )
{
	int iSyncType, iShuffleRoomIndex;
	rkPacket >> iSyncType >> iShuffleRoomIndex;

	switch( iSyncType )
	{
	case ShuffleRoomSync::BRS_PLAY:
		{
			ShuffleRoomCopyNode *pCopyNode = GetShuffleRoomNode( iShuffleRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncPlay( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomSync Not Room : %d - %d - %s:%d", iSyncType, iShuffleRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;	
	case ShuffleRoomSync::BRS_CHANGEINFO:
		{
			ShuffleRoomCopyNode *pCopyNode = GetShuffleRoomNode( iShuffleRoomIndex );
			if( pCopyNode )
				pCopyNode->ApplySyncChangeInfo( rkPacket );
			else
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnShuffleRoomSync Not Room : %d - %d - %s:%d", iSyncType, iShuffleRoomIndex, GetServerIP().c_str(), GetServerPort() );
		}
		break;
	case ShuffleRoomSync::BRS_CREATE:
		{
			ShuffleRoomCopyNode *pNewNode = CreateNewShuffleRoom( iShuffleRoomIndex );
			if( pNewNode )
			{
				pNewNode->ApplySyncCreate( rkPacket );
				g_ShuffleRoomManager.AddCopyShuffleRoom( pNewNode );
			}
		}
		break;
	case ShuffleRoomSync::BRS_DESTROY:
		{
			RemoveShuffleRoomNode( iShuffleRoomIndex );
		}
		break; 
	}
}

void ServerNode::OnShuffleRoomKickOut( SP2Packet &rkPacket )
{
	BYTE eKickType = (BYTE)RoomParent::RLT_NORMAL;
	PACKET_GUARD_VOID( rkPacket.Read( eKickType ) );
	
	DWORD dwUserIndex;
	PACKET_GUARD_VOID( rkPacket.Read( dwUserIndex ) );
		
	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnBattleRoomKickOut ���� NULL : %d", dwUserIndex );
		return;
	}
	pUser->ShuffleRoomKickOut();
	pUser->ShuffleRoomKickOut( eKickType );
}

void ServerNode::OnExceptionShuffleRoomLeave( SP2Packet &rkPacket )
{
	int iShuffleRoomIndex;
	rkPacket >> iShuffleRoomIndex;
	ShuffleRoomNode *pShuffleRoom = g_ShuffleRoomManager.GetShuffleRoomNode( iShuffleRoomIndex );	
	if( !pShuffleRoom ) return;

	DWORD dwUserIndex;
	ioHashString szPublicID;
	rkPacket >> dwUserIndex >> szPublicID;
	pShuffleRoom->LeaveUser( dwUserIndex, szPublicID );

	LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "ServerNode::OnExceptionBattleRoomLeave(%d) : %s", iShuffleRoomIndex, szPublicID.c_str() );
}

void ServerNode::OnShuffleRoomGlobalCreate( SP2Packet &rkPacket )
{
	DWORD dwUserIdx        = 0;
	DWORD dwShuffleRoomIdx = 0;
	PACKET_GUARD_VOID( rkPacket.Read( dwUserIdx ) );
	PACKET_GUARD_VOID( rkPacket.Read( dwShuffleRoomIdx ) );
	
	User* pUser = g_UserNodeManager.GetUserNode( dwUserIdx );
	if( pUser && pUser->IsUserShuffleRoomJoin() )
	{
		ShuffleRoomParent* pRoom = g_ShuffleRoomManager.GetGlobalShuffleRoomNode( dwShuffleRoomIdx );
		if( pRoom )
		{
			pUser->EnterShuffleRoom( pRoom );			
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - pRoom == NULL!!!", __FUNCTION__ );
		}
	}
}

void ServerNode::OnMoveNewPetData( SP2Packet &rkPacket )
{
	DWORD dwUserIdx = 0;
	CQueryResultData query_data;

	PACKET_GUARD_VOID( rkPacket.Read( dwUserIdx ) );

	User* pUser = g_UserNodeManager.GetUserNode( dwUserIdx );
	if( !pUser )
		return;

	PACKET_GUARD_VOID( rkPacket.Read( query_data ) );

	g_UserNodeManager.ProcessInsertPetData( pUser, query_data );

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Before Recv Pet Insert Packet, User Move Server.  : %s", __FUNCTION__, pUser->GetPublicID().c_str() );
}

void ServerNode::OnMoveSoldierData( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplySoldierMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveSoldierData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnMoveEtcItemData( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyEtcItemMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveEtcItemData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnMoveMedalItemData( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyMedalItemMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveMedalItemData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnMoveCostumeData( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyCostumeMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveCostumeData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnMoveAccessroyData(SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyAccessoryMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveCostumeData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnAccessoryAdd(SP2Packet &rkPacket)
{
	DWORD dwUserIndex	= 0;
	int iIndex			= 0;
	DWORD dwCode		= 0;
	BYTE byPeriodType	= 0;
	int iYMD			= 0;
	int iHM				= 0;
	BYTE byInsertType	= 0;
	int iValue1			= 0;
	int iValue2			= 0;
	int iValue3			= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwCode) );
	PACKET_GUARD_VOID( rkPacket.Read(byPeriodType) );
	PACKET_GUARD_VOID( rkPacket.Read(iYMD) );
	PACKET_GUARD_VOID( rkPacket.Read(iHM) );
	PACKET_GUARD_VOID( rkPacket.Read(byInsertType) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue1) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue2) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue3) );

	User* pUser = GetUserOriginalNode(dwUserIndex, rkPacket);
	if( pUser )
	{
		pUser->AddAccessoryItem(iIndex, dwCode,byPeriodType, iYMD, iHM, iValue3);
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] servernode accessory add : [code:%d value:%d]", dwCode, iValue3);

		if( byInsertType == 1 )
		{
			int iYMD	= 0;
			int iHM		= 0;

			//��ǰ ����ó��
			SP2Packet kPacket( STPK_ACCESSORY_BUY );
			PACKET_GUARD_VOID(kPacket.Write(ITEM_BUY_OK));
			PACKET_GUARD_VOID(kPacket.Write(iIndex));
			PACKET_GUARD_VOID(kPacket.Write(dwCode));
			PACKET_GUARD_VOID(kPacket.Write(byPeriodType));
			PACKET_GUARD_VOID(kPacket.Write(iYMD));
			PACKET_GUARD_VOID(kPacket.Write(iHM));
			PACKET_GUARD_VOID(kPacket.Write(iValue1));
			PACKET_GUARD_VOID(kPacket.Write(iValue2));		//��ǰ �Ⱓ��
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetMoney()));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetCash()));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetChannelingCash()));
			PACKET_GUARD_VOID(kPacket.Write(iValue3));

			pUser->SendMessage(kPacket);
			int iPeriod = iValue2 * 60;
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_ITEM_BUY, pUser, 0, PRESENT_ACCESSORY, 1, iValue1, iValue3, 1, iPeriod, NULL);
		}
		else if( byInsertType == 2 )
		{
			//������ recv
			SP2Packet kPacket( STPK_ACCESSORY_PRESENT );
			PACKET_GUARD_VOID(kPacket.Write(dwCode));
			PACKET_GUARD_VOID(kPacket.Write(iIndex));
			PACKET_GUARD_VOID(kPacket.Write(byPeriodType));
			PACKET_GUARD_VOID(kPacket.Write(iYMD));
			PACKET_GUARD_VOID(kPacket.Write(iHM));
			PACKET_GUARD_VOID(kPacket.Write(iValue3));
			pUser->SendMessage(kPacket);
		}
	}
}

void ServerNode::OnRainbowWholeChat( SP2Packet &rkPacket )
{
	ioHashString szID;
	ioHashString szChat;

	PACKET_GUARD_VOID( rkPacket.Read(szID) );
	PACKET_GUARD_VOID( rkPacket.Read(szChat) );

	SP2Packet kPacket( STPK_RAINBOW_WHOLE_CHAT );
	PACKET_GUARD_VOID( kPacket.Write(szID) );
	PACKET_GUARD_VOID( kPacket.Write(szChat) );
	g_UserNodeManager.SendMessageAll( kPacket );	
}

void ServerNode::OnCostumeAdd(SP2Packet &rkPacket)
{
	DWORD dwUserIndex	= 0;
	int iIndex			= 0;
	DWORD dwCode		= 0;
	BYTE byPeriodType	= 0;
	int iYMD			= 0;
	int iHM				= 0;
	BYTE byInsertType	= 0;
	int iValue1			= 0;
	int iValue2			= 0;
	int iValue3			= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwCode) );
	PACKET_GUARD_VOID( rkPacket.Read(byPeriodType) );
	PACKET_GUARD_VOID( rkPacket.Read(iYMD) );
	PACKET_GUARD_VOID( rkPacket.Read(iHM) );
	PACKET_GUARD_VOID( rkPacket.Read(byInsertType) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue1) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue2) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue3) );

	User* pUser = GetUserOriginalNode(dwUserIndex, rkPacket);
	if( pUser )
	{
		pUser->AddCostumeItem(iIndex, dwCode,byPeriodType, iValue1, iValue2);

		if( byInsertType == 1 )
		{
			int iYMD	= 0;
			int iHM		= 0;

			//��ǰ ����ó��
			SP2Packet kPacket( STPK_COSTUME_BUY );
			PACKET_GUARD_VOID(kPacket.Write(ITEM_BUY_OK));
			PACKET_GUARD_VOID(kPacket.Write(iIndex));
			PACKET_GUARD_VOID(kPacket.Write(dwCode));
			PACKET_GUARD_VOID(kPacket.Write(byPeriodType));
			PACKET_GUARD_VOID(kPacket.Write(iYMD));
			PACKET_GUARD_VOID(kPacket.Write(iHM));
			PACKET_GUARD_VOID(kPacket.Write(0));
			PACKET_GUARD_VOID(kPacket.Write(0));
			PACKET_GUARD_VOID(kPacket.Write(iValue2));		//��ǰ �Ⱓ��
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetMoney()));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetCash()));
			PACKET_GUARD_VOID(kPacket.Write(pUser->GetChannelingCash()));
			PACKET_GUARD_VOID(kPacket.Write(iValue1));		//��ǰ �ڵ�

			pUser->SendMessage(kPacket);
			int iPeriod = iValue2 * 60;
			g_LogDBClient.OnInsertGameLogInfo(LogDBClient::GLT_ITEM_BUY, pUser, 0, PRESENT_COSTUME, 1, iValue1, iValue3, 1, iPeriod, NULL);
		}
		else if( byInsertType == 2 )
		{
			//������ recv
			SP2Packet kPacket( STPK_COSTUME_PRESENT );
			PACKET_GUARD_VOID(kPacket.Write(dwCode));
			PACKET_GUARD_VOID(kPacket.Write(iIndex));
			PACKET_GUARD_VOID(kPacket.Write(byPeriodType));
			PACKET_GUARD_VOID(kPacket.Write(iYMD));
			PACKET_GUARD_VOID(kPacket.Write(iHM));
			PACKET_GUARD_VOID(kPacket.Write(0));			//���� ġ��
			PACKET_GUARD_VOID(kPacket.Write(0));			//����
			pUser->SendMessage(kPacket);

			//g_LogDBClient.OnInsertCostumeInfo(pUser, dwCode, 1, LogDBClient::COT_PRESENT);
		}
	}

}

void ServerNode::OnMoveMissionData( SP2Packet &rkPacket )
{
	int iMovingValue;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyMissionMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveMissionData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnMoveRollBookData( SP2Packet &rkPacket )
{
	int iMovingValue	= 0;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyRollBookMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveRollBookData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnReserveCreateGuildRoom(SP2Packet &rkPacket)
{
	int iSubType		= 0;
	DWORD dwGuildIndex	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iSubType) );

	Room *pRoom = g_RoomNodeManager.CreateNewPlazaRoom( iSubType, 1 );
	if( pRoom )
	{
		ioHashString szPlazaName, szPlazaPW;
		int iMax	= 0;
		DWORD dwUserIndex	= 0;
		
		PACKET_GUARD_VOID( rkPacket.Read(szPlazaName) );
		PACKET_GUARD_VOID( rkPacket.Read(szPlazaPW) );
		PACKET_GUARD_VOID( rkPacket.Read(iMax) );
		PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );

		if( !szPlazaName.IsEmpty() )
			pRoom->SetRoomName( szPlazaName );
		if( !szPlazaPW.IsEmpty() )
			pRoom->SetRoomPW( szPlazaPW );

		pRoom->SetMaxPlayer( iMax );
		pRoom->SetPlazaModeType( PT_GUILD );
		pRoom->SetSubState( false );

		g_DBClient.OnSelectGuildBlocksInfos(dwUserIndex, dwGuildIndex, pRoom->GetRoomIndex());
	}
}

void ServerNode::OnChangeGuildRoomStatus(SP2Packet &rkPacket)
{
	DWORD dwUserIndex	= 0;
	DWORD dwGuildIndex	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwGuildIndex) );

	User* pUser	= g_UserNodeManager.GetUserNode( dwUserIndex );
	if( !pUser )
		return;

	if( !pUser->IsGuild() )
		return;

	if( pUser->GetGuildIndex() != dwGuildIndex )
		return;

	pUser->ActiveUserGuildRoom();
}

void ServerNode::OnJoinPersonalHQUser( SP2Packet &rkPacket )
{
	bool bInvited		= false;
	int iMapIndex		= 0;
	DWORD dwRequestUser	= 0;
	DWORD dwOwnerUser	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwRequestUser) );
	PACKET_GUARD_VOID( rkPacket.Read(dwOwnerUser) );
	PACKET_GUARD_VOID( rkPacket.Read(iMapIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(bInvited) );

	User *pUser = GetUserOriginalNode( dwOwnerUser, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwOwnerUser ) == NULL )
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
			if( pUserParent )
			{
				SP2Packet kPacket( STPK_JOIN_PERSONAL_HQ );
				PACKET_GUARD_VOID( kPacket.Write(JOIN_PERSONAL_HQ_OWNER_OFFLINE) );
				pUserParent->RelayPacket( kPacket );
			}
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnJoinPersonalHQUser User NULL(%d)", dwOwnerUser );
		}
		return;
	}

	pUser->_OnJoinPersonalHQ( g_UserNodeManager.GetGlobalUserNode( dwRequestUser ), iMapIndex, bInvited );
}

void ServerNode::OnPersonalHQInfo( SP2Packet &rkPacket )
{
	ioHashString szOwnerName;
	DWORD dwRequestUser	= 0, dwOwnerUser	= 0;
	PACKET_GUARD_VOID( rkPacket.Read(dwRequestUser) );
	PACKET_GUARD_VOID( rkPacket.Read(dwOwnerUser) );
	PACKET_GUARD_VOID( rkPacket.Read(szOwnerName) );

	User *pUser = GetUserOriginalNode( dwOwnerUser, rkPacket );
	if( pUser == NULL )
	{
		if( g_UserNodeManager.GetGlobalUserNode( dwOwnerUser ) == NULL )
		{
			UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
			if( pUserParent )
			{
				SP2Packet kPacket( STPK_PERSONAL_HQ_INFO );
				PACKET_GUARD_VOID( kPacket.Write(szOwnerName) );
				PACKET_GUARD_VOID( kPacket.Write((MAX_PLAYER / 2)) );
				PACKET_GUARD_VOID( kPacket.Write(0) );
				pUserParent->RelayPacket( kPacket );
			}
		}
		return;
	}
	pUser->_OnPersonalHQInfo( g_UserNodeManager.GetGlobalUserNode( dwRequestUser ) );
}

void ServerNode::OnPersonalHQRoomInfo( SP2Packet &rkPacket )
{
	int iRoomIndex	= 0;
	PACKET_GUARD_VOID( rkPacket.Read(iRoomIndex) );

	bool bLock = false;
	DWORD dwRequestUser = 0;
	ioHashString szMasterName;

	PACKET_GUARD_VOID( rkPacket.Read(szMasterName) );
	PACKET_GUARD_VOID( rkPacket.Read(bLock) );
	PACKET_GUARD_VOID( rkPacket.Read(dwRequestUser) );
	
	Room *pRoom = g_RoomNodeManager.GetHeadquartersNode( iRoomIndex );
	if( pRoom == NULL )
	{
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
		if( pUserParent )
		{
			SP2Packet kPacket( STPK_PERSONAL_HQ_INFO );
			PACKET_GUARD_VOID( kPacket.Write(szMasterName) );
			PACKET_GUARD_VOID( kPacket.Write((MAX_PLAYER / 2)) );
			PACKET_GUARD_VOID( kPacket.Write(0) );
		
			pUserParent->RelayPacket( kPacket );
		}
		//LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnHeadquartersRoomInfo Room NULL(%d)", iRoomIndex );
	}
	else
	{
		UserParent *pUserParent = g_UserNodeManager.GetGlobalUserNode( dwRequestUser );
		if( pUserParent )
		{
			SP2Packet kPacket( STPK_PERSONAL_HQ_INFO );
			pRoom->FillHeadquartersInfo( szMasterName, bLock, kPacket );
			pUserParent->RelayPacket( kPacket );
		}
	}
}

void ServerNode::OnPersonalHQJoinAgree( SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0, dwRoomIndex	= 0;
	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwRoomIndex) );

	User *pUser = GetUserOriginalNode( dwUserIndex, rkPacket );
	if( pUser == NULL )
		return;

	pUser->_OnPersonalHQJoinAgree( dwRoomIndex );
}

void ServerNode::OnMovePersonalHQData(SP2Packet &rkPacket )
{
	int iMovingValue	= 0;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyPersonalHQMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMovePersonalHQData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnPersonalHQAddBlock(SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0;
	DWORD dwItemCode	= 0;
	int iCount			= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwItemCode) );
	PACKET_GUARD_VOID( rkPacket.Read(iCount) );

	UserParent* pUserParent	= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
		return;

	if( pUserParent->IsUserOriginal() )
	{
		User* pUser	= static_cast<User*>(pUserParent);

		pUser->AddPersonalInvenItem(dwItemCode, iCount);
	}
	else
	{
		UserCopyNode *pUser = static_cast<UserCopyNode*>(pUserParent);

		SP2Packet kPacket(SSTPK_PERSONAL_HQ_ADD_BLOCK);
		PACKET_GUARD_VOID( kPacket.Write(dwUserIndex) );
		PACKET_GUARD_VOID( kPacket.Write(dwItemCode) );
		PACKET_GUARD_VOID( kPacket.Write(iCount) );
		pUser->SendMessage(kPacket);
	}
}

void ServerNode::OnMoveTimeCashData( SP2Packet &rkPacket )
{
	int iMovingValue	= 0;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyTimeCashDate( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveTimeCashData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnUpdateTimeCashInfo( SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0;
	int iResultType		= 0;
	DWORD dwCode		= 0;
	DWORD dwReceiveDate	= 0;
	ioHashString szBillingGUID;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(dwCode) );
	PACKET_GUARD_VOID( rkPacket.Read(iResultType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwReceiveDate) );
	PACKET_GUARD_VOID( rkPacket.Read(szBillingGUID) );

	UserParent* pUserParent	= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
		return;

	if( pUserParent->IsUserOriginal() )
	{
		User* pUser	= static_cast<User*>(pUserParent);

		pUser->UpdateCashTable(dwCode, iResultType, dwReceiveDate, szBillingGUID);
	}
	else
	{
		UserCopyNode *pUser = static_cast<UserCopyNode*>(pUserParent);

		SP2Packet kPacket(SSTPK_UPDATE_TIME_CASH);
		PACKET_GUARD_VOID( kPacket.Write(dwUserIndex) );
		PACKET_GUARD_VOID( kPacket.Write(dwCode) );
		PACKET_GUARD_VOID( kPacket.Write(iResultType) );
		PACKET_GUARD_VOID( kPacket.Write(dwReceiveDate) );
		PACKET_GUARD_VOID( kPacket.Write(szBillingGUID) );
		pUser->SendMessage(kPacket);
	}
}

void ServerNode::OnMoveTitleData( SP2Packet &rkPacket )
{
	int iMovingValue	= 0;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyTitleMoveData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveTitleData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnTitleUpdate(SP2Packet &rkPacket )
{
	DWORD dwUserIndex	= 0;
	DWORD dwCode		= 0;
	__int64 iValue		= 0;
	int iLevel			= 0;
	BYTE byPremium		= 0;
	BYTE byEquip		= 0;
	BYTE byStatus		= 0;
	BYTE byActionType	= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(byActionType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwCode) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue) );
	PACKET_GUARD_VOID( rkPacket.Read(iLevel) );
	PACKET_GUARD_VOID( rkPacket.Read(byPremium) );
	PACKET_GUARD_VOID( rkPacket.Read(byEquip) );
	PACKET_GUARD_VOID( rkPacket.Read(byStatus) );

	UserParent* pUserParent	= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
		return;

	if( pUserParent->IsUserOriginal() )
	{
		User* pUser	= static_cast<User*>(pUserParent);

		pUser->UpdateUserTitle(dwCode, iValue, iLevel, byPremium, byEquip, byStatus, byActionType);
	}
	else
	{
		UserCopyNode *pUser = static_cast<UserCopyNode*>(pUserParent);

		SP2Packet kPacket(SSTPK_TITLE_UPDATE);
		PACKET_GUARD_VOID( kPacket.Write(dwUserIndex) );
		PACKET_GUARD_VOID( kPacket.Write(byActionType) );
		PACKET_GUARD_VOID( kPacket.Write(dwCode) );
		PACKET_GUARD_VOID( kPacket.Write(iValue) );
		PACKET_GUARD_VOID( kPacket.Write(iLevel) );
		PACKET_GUARD_VOID( kPacket.Write(byPremium) );
		PACKET_GUARD_VOID( kPacket.Write(byEquip) );
		PACKET_GUARD_VOID( kPacket.Write(byStatus) );

		pUser->SendMessage(kPacket);
	}
}

void ServerNode::OnMoveBonusCashData(SP2Packet &rkPacket )
{
	int iMovingValue	= 0;
	ioHashString szPublicID;

	PACKET_GUARD_VOID( rkPacket.Read(iMovingValue) );
	PACKET_GUARD_VOID( rkPacket.Read(szPublicID) );

	User *pUser = g_UserNodeManager.GetUserNodeByPublicID( szPublicID );
	if( pUser )
	{
		pUser->ApplyBonusCashData( rkPacket );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ServerNode::OnMoveBonusCashData Exception Node %s", szPublicID.c_str() );
	}
}

void ServerNode::OnResultBonusCashAdd(SP2Packet &rkPacket )
{
	DWORD dwUserIndex		= 0;
	int iAmount				= 0;
	DWORD dwExpirationDate	= 0;
	DWORD dwIndex			= 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iAmount) );
	PACKET_GUARD_VOID( rkPacket.Read(dwExpirationDate) );
	PACKET_GUARD_VOID( rkPacket.Read(dwIndex) );

	UserParent* pUserParent	= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
		return;

	if( pUserParent->IsUserOriginal() )
	{
		User* pUser	= static_cast<User*>(pUserParent);

		pUser->InsertUserBonusCash(dwIndex, iAmount, dwExpirationDate);
	}
	else
	{
		UserCopyNode *pUser = static_cast<UserCopyNode*>(pUserParent);

		SP2Packet kPacket(SSTPK_BONUS_CAHSH_ADD);
		PACKET_GUARD_VOID( kPacket.Write(dwUserIndex) );
		PACKET_GUARD_VOID( kPacket.Write(iAmount) );
		PACKET_GUARD_VOID( kPacket.Write(dwExpirationDate) );
		PACKET_GUARD_VOID( kPacket.Write(dwIndex) );
		pUser->SendMessage(kPacket);
	}
}

void ServerNode::OnResultBonusCashUpdate(SP2Packet &rkPacket )
{
	DWORD dwUserIndex		= 0;
	BYTE byType				= 0;
	DWORD dwCashIndex		= 0;
	DWORD dwStatus			= 0;
	int iAmount				= 0;
	int iUsedAmount			= 0;
	int iType				= 0;
	int iValue1				= 0;
	int iValue2				= 0;
	ioHashString szGUID;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(byType) );
	PACKET_GUARD_VOID( rkPacket.Read(dwStatus) );
	PACKET_GUARD_VOID( rkPacket.Read(dwCashIndex) );
	PACKET_GUARD_VOID( rkPacket.Read(iAmount) );
	PACKET_GUARD_VOID( rkPacket.Read(iUsedAmount) );
	PACKET_GUARD_VOID( rkPacket.Read(iType) );
	PACKET_GUARD_VOID( rkPacket.Read(iValue1) );

	PACKET_GUARD_VOID( rkPacket.Read(iValue2) );	
	PACKET_GUARD_VOID( rkPacket.Read(szGUID) );	

	UserParent* pUserParent	= g_UserNodeManager.GetGlobalUserNode( dwUserIndex );
	if( !pUserParent )
		return;

	if( pUserParent->IsUserOriginal() )
	{
		User* pUser	= static_cast<User*>(pUserParent);


		pUser->UpdateUserBonusCash(static_cast<BonusCashUpdateType>(byType), dwStatus, dwCashIndex, iAmount, iUsedAmount, iType, iValue1, iValue2, szGUID.c_str());
	}
	else
	{
		UserCopyNode *pUser = static_cast<UserCopyNode*>(pUserParent);

		SP2Packet kPacket(SSTPK_BONUS_CASH_UPDATE);
		PACKET_GUARD_VOID( kPacket.Write(dwUserIndex) );
		PACKET_GUARD_VOID( kPacket.Write(byType) );
		PACKET_GUARD_VOID( rkPacket.Read(dwStatus) );
		PACKET_GUARD_VOID( kPacket.Write(dwCashIndex) );
		PACKET_GUARD_VOID( kPacket.Write(iAmount) );
		PACKET_GUARD_VOID( rkPacket.Read(iUsedAmount) );
		PACKET_GUARD_VOID( kPacket.Write(iType) );
		PACKET_GUARD_VOID( kPacket.Write(iValue1) );
		PACKET_GUARD_VOID( kPacket.Write(iValue2) );
		PACKET_GUARD_VOID( rkPacket.Read(szGUID) );	

		pUser->SendMessage(kPacket);
	}
}

#if 0
bool ServerNode::IsMyRoom( DWORD roomIndex )
{
	auto resultVal = m_roomSet.find(roomIndex);
	if( resultVal != m_roomSet.end())
	{
		return true;
	}
	else
		return false;

}

void ServerNode::AddRoom( DWORD roomIndex )
{
	m_roomSet.insert(roomIndex);

}

void ServerNode::DelRoom( DWORD roomIndex )
{
	m_roomSet.erase(roomIndex);
}




#endif