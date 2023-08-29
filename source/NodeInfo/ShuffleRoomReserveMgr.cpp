#include "stdafx.h"

#include "../MainProcess.h"

#include "UserNodeManager.h"

#include "ShuffleRoomReserveMgr.h"
#include "ShuffleRoomManager.h"
#include "ServerNodeManager.h"
#include "LevelMatchManager.h"

ShuffleRoomReserveMgr *ShuffleRoomReserveMgr::sg_Instance = NULL;
ShuffleRoomReserveMgr::ShuffleRoomReserveMgr()
{
}

ShuffleRoomReserveMgr::~ShuffleRoomReserveMgr()
{
}

ShuffleRoomReserveMgr &ShuffleRoomReserveMgr::GetInstance()
{
	if( sg_Instance == NULL )
		sg_Instance = new ShuffleRoomReserveMgr;

	return *sg_Instance;
}

void ShuffleRoomReserveMgr::ReleaseInstance()
{
	SAFEDELETE( sg_Instance );
}

void ShuffleRoomReserveMgr::Initialize()
{
}

void ShuffleRoomReserveMgr::AddShuffleQueue( User *pUser, int iGlobalSearchingTryCount )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - pUser == NULL!", __FUNCTION__ );
		return;
	}

	//�ٷ� ���� ������ ���� �ִ��� �˻�
	ShuffleRoomParent *pNode = g_ShuffleRoomManager.GetJoinShuffleRoomNode( pUser->GetKillDeathLevel(), iGlobalSearchingTryCount );
	if( pNode )
	{
		pUser->EnterShuffleRoom( pNode );
		
		LOG.PrintTimeAndLog( LOG_SHUFFLE, "%s : %d�� ��(%d) �߰����� : %s (%d)", __FUNCTION__, pNode->GetIndex(), pNode->GetAbilityMatchLevel(), pUser->GetPublicID().c_str(), pUser->GetKillDeathLevel() );
		return;
	}

	GlobalSearchingShuffleUser( pUser, iGlobalSearchingTryCount );
}

void ShuffleRoomReserveMgr::GlobalSearchingShuffleUser( User *pSearchUser, int iGlobalSearchingTryCount )
{
	if( !pSearchUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s - pSearchUser == NULL!", __FUNCTION__ );
		return;
	}

	//�����߿� ��Ī ������ �´� ������ ã�´�(������ Ƚ�� ��ŭ �õ�)
	if( iGlobalSearchingTryCount < g_ShuffleRoomManager.GetMatchCheckMaxCount( pSearchUser->GetKillDeathLevel() ) )
	{
		DWORDVec vUserGroup;
		g_UserNodeManager.FindShuffleGlobalQueueUser( pSearchUser, iGlobalSearchingTryCount, vUserGroup );

		if( vUserGroup.empty() )
		{
			//���ǿ� �´� ������ ��ã�������� ��Ī ���
			pSearchUser->SetShuffleGlboalSearch( true );
			pSearchUser->SyncUserShuffle();

			LOG.PrintTimeAndLog( LOG_SHUFFLE, "%s : %s(ų��������:%d)�� ���ǿ� �´� ���� ��Ī ���� �� ��Ī ���(�˻� �õ� Ƚ��:%d)", __FUNCTION__, pSearchUser->GetPublicID().c_str(), pSearchUser->GetKillDeathLevel(), iGlobalSearchingTryCount );
		}
		else
		{
			//���� �÷����� ���� �׷����
			UserGroup kGroup;
			kGroup.Init();
			kGroup.Add( pSearchUser->GetUserIndex(), pSearchUser->GetKillDeathLevel() );

			for( DWORDVec::iterator iter = vUserGroup.begin(); iter != vUserGroup.end(); ++iter )
			{
				UserParent* pUser = g_UserNodeManager.GetGlobalUserNode( *iter );
				if( pUser )
				{
					kGroup.Add( pUser->GetUserIndex(), pUser->GetKillDeathLevel() );
					LOG.PrintTimeAndLog( LOG_SHUFFLE, "%s : %s(ų��������:%d)���� %s���� ���� �׷쿡 �߰�(�˻� �õ� Ƚ��:%d)", __FUNCTION__, pSearchUser->GetPublicID().c_str(), pSearchUser->GetKillDeathLevel(), pUser->GetPublicID().c_str(), iGlobalSearchingTryCount );
				}
				else
				{
					LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s : %s(ų��������:%d)�� ���� �׷������ ���� �߰� ����(�˻� �õ� Ƚ��:%d)", __FUNCTION__, pSearchUser->GetPublicID().c_str(), pSearchUser->GetKillDeathLevel(), iGlobalSearchingTryCount );
				}
			}

			ShuffleRoomNode* pRoomNode = CreateShuffleRoom( kGroup );
			if( pRoomNode )
			{
				//CreateShuffleRoom���� ModeList�� �����ϱ⋚���� RETURN���� ���� ����
				pRoomNode->ShuffleRoomReadyGo();
			}
			else
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s : %s(ų��������:%d)�� ���÷� ��� ���� ����(�˻� �õ� Ƚ��:%d)", __FUNCTION__, pSearchUser->GetPublicID().c_str(), pSearchUser->GetKillDeathLevel(), iGlobalSearchingTryCount );
				pSearchUser->SetShuffleGlboalSearch( false );
				pSearchUser->SyncUserShuffle();
			}
		}
	}
	else
	{
		//��Ī ���� - ��� ���µ� ����
		pSearchUser->SetShuffleGlboalSearch( false );
		pSearchUser->SyncUserShuffle();

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s : %s�� (ų��������:%d) ��Ī �˻� Ƚ�� �ʰ��� ���ǿ� �´� ���� ��Ī ����(�˻� �õ� Ƚ��:%d)", __FUNCTION__, pSearchUser->GetPublicID().c_str(), pSearchUser->GetKillDeathLevel(), iGlobalSearchingTryCount );
	}
}

void ShuffleRoomReserveMgr::DeleteShuffleQueue( User *pUser )
{
	if( pUser )
	{
		pUser->SetShuffleGlboalSearch( false );
		pUser->SyncUserShuffle();

		LOG.PrintTimeAndLog( LOG_SHUFFLE, "[info][shuffle]User stop searching : [%d]", pUser->GetUserIndex() );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][shuffle]Invalid user stop searching");
	}
}

ShuffleRoomNode* ShuffleRoomReserveMgr::CreateShuffleRoom( const UserGroup& kGroup )
{
	ShuffleRoomNode *pRoomNode = g_ShuffleRoomManager.CreateNewShuffleRoom();
	if( !pRoomNode )
		return NULL;

	if( !pRoomNode->HasModeList() )
		return NULL;

	CRASH_GUARD();

	pRoomNode->SyncRealTimeCreate();

	LOOP_GUARD();

	int iSize = kGroup.GetUserCnt();
	for( int i = 0; i < iSize; ++i )
	{
		UserParent *pParent = g_UserNodeManager.GetGlobalUserNode( kGroup.m_vUserIndex[i] );
		if( !pParent )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s : %d�� �� �� �� ���� ���� ��� �߻� : %d", __FUNCTION__, pRoomNode->GetIndex(), kGroup.m_vUserIndex[i] );
			continue;
		}

		if( pParent->IsUserOriginal() )
		{
			User* pUser = static_cast<User*>( pParent );
			pUser->EnterShuffleRoom( pRoomNode );
			LOG.PrintTimeAndLog( LOG_SHUFFLE, "%s : %d�� �� �ű����� : %s�� (%d)", __FUNCTION__, pRoomNode->GetIndex(), pUser->GetPublicID().c_str(), pUser->GetKillDeathLevel() );
		}
		else
		{	
			UserCopyNode* pUser = static_cast<UserCopyNode*>( pParent );
			SP2Packet kPacket( SSTPK_SHUFFLEROOM_GLOBAL_CREATE );
			PACKET_GUARD_NULL( kPacket.Write( pUser->GetUserIndex() ) );
			PACKET_GUARD_NULL( kPacket.Write( pRoomNode->GetIndex() ) );
			pUser->SendMessage( kPacket );

			LOG.PrintTimeAndLog( LOG_SHUFFLE, "%s : %d�� �� �ű����� : %s�� (%d)", __FUNCTION__, pRoomNode->GetIndex(), pUser->GetPublicID().c_str(), pUser->GetKillDeathLevel() );
		}
	}
	LOOP_GUARD_CLEAR();

	return pRoomNode;
}