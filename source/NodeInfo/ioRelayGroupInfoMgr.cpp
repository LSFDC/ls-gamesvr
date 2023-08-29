#include "stdafx.h"
#include "ioRelayGroupInfoMgr.h"
#include "ioRelayRoomInfoMgr.h"
#include "SchedulerNode.h"
#ifdef ANTIHACK
#include "../Network/ioSkillInfoMgr.h"
#endif

bool bDetailLog = false;

#ifndef ANTIHACK
ioRelayGroupInfoMgr::ioRelayGroupInfoMgr(void)
{
}

ioRelayGroupInfoMgr::~ioRelayGroupInfoMgr(void)
{
}

void ioRelayGroupInfoMgr::InitData(int iSeedRoom)
{
	for(int i=0 ; i < iSeedRoom ; i++)
	{
		RelayGroup* pRelayGroup = new RelayGroup;
		if(pRelayGroup)
		{
			pRelayGroup->m_dwRoomIndex = -1;
			pRelayGroup->m_RelayUserList.reserve(MAX_PLAYER);
			m_vRelayGroupPool.Push(pRelayGroup);
		}
	}
}

void ioRelayGroupInfoMgr::InsertRoom( RelayHeader* pRelayHeader )
{
	InsertData* pData = reinterpret_cast<InsertData*>(pRelayHeader);
	InsertData &rkData = *pData;

	{
		//LOG.PrintTimeAndLog(0,"ioRelayGroupInfoMgr::InsertRoom Start: %d", GetCurrentThreadId());
		ThreadSync ts(this);
		if(!InsertUser(rkData.m_dwRoomIndex, rkData.m_dwUserIndex, rkData.m_iClientPort, rkData.m_szPublicIP))
		{
			// ����
			CreateRelayGroup(rkData.m_dwRoomIndex, rkData.m_dwUserIndex, rkData.m_iClientPort, rkData.m_szPublicIP);
		}
	}

	//LOG.PrintTimeAndLog(0,"ioRelayGroupInfoMgr::InsertRoom End: %d", GetCurrentThreadId());
	g_RelayRoomInfoMgr->InsertRoomInfo(rkData.m_dwUserIndex, rkData.m_dwRoomIndex);
}

void ioRelayGroupInfoMgr::RemoveRoom( RelayHeader* pRelayHeader )
{
	RemoveData* pData = reinterpret_cast<RemoveData*>(pRelayHeader);
	RemoveData &rkData = *pData;

	if( rkData.m_dwUserIndex == 0 )
	{
		// �� ��°�� ����
		//LOG.PrintTimeAndLog(0,"TEST RemoveRoom : %d %d", rkData.m_dwUserIndex, rkData.m_dwRoomIndex);//������
		RelayGroup *pRelayGroup = RemoveRelayGroupByRoom( rkData.m_dwRoomIndex );
		if(pRelayGroup)
		{
			for(DWORD i=0; i<pRelayGroup->m_RelayUserList.size(); i++)
			{
				g_RelayRoomInfoMgr->RemoveRoomInfo(pRelayGroup->m_RelayUserList[i].m_dwUserIndex);
			}

			pRelayGroup->m_dwRoomIndex = 0;
			pRelayGroup->m_RelayUserList.clear();

			m_vRelayGroupPool.Push(pRelayGroup);
			//LOG.PrintTimeAndLog(0,"TEST RemoveRoom m_vRelayGroupPool push");//������
		}
	}
	else
	{
		// ������ ����
		//LOG.PrintTimeAndLog(0,"TEST RemoveUser : %d %d", rkData.m_dwUserIndex, rkData.m_dwRoomIndex);
		RemoveUser(rkData.m_dwRoomIndex, rkData.m_dwUserIndex);

		g_RelayRoomInfoMgr->RemoveRoomInfo(rkData.m_dwUserIndex);
	}
}

BOOL ioRelayGroupInfoMgr::SendRelayPacket( DWORD dwUserIndex, SP2Packet& kPacket )
{
	RelayGroup::RelayGroups vGroupUsers;
	if(!GetRelayUserList( dwUserIndex, vGroupUsers ))
		return FALSE;

	for(unsigned int i=0; i< vGroupUsers.size(); i++)
	{
		UserData& rkUser = vGroupUsers.at(i);
		if(rkUser.m_dwUserIndex != dwUserIndex)
		{
			g_UDPNode.SendMessage(rkUser.m_szPublicIP, rkUser.m_iClientPort, kPacket);
		}
	}
	return TRUE;
}

BOOL ioRelayGroupInfoMgr::CreateRelayGroup(DWORD dwRoomIndex, DWORD dwUserIndex, int iPort, char* szIP)
{
	RelayGroup* pRelayGroup = m_vRelayGroupPool.Pop();
	if( pRelayGroup )
	{
		pRelayGroup->m_dwRoomIndex = dwRoomIndex;
		pRelayGroup->AddUser(dwUserIndex, iPort, szIP);

		m_vRelayGroups.SetAt( dwRoomIndex, pRelayGroup );
		//LOG.PrintTimeAndLog( 0, "Test - RelayGroup Create : %d, %d", (int)dwRoomIndex, (int)dwUserIndex );
		return TRUE;
	}

	LOG.PrintTimeAndLog(0, "CreateRelayGroup failed");
	return FALSE;
}

RelayGroup* ioRelayGroupInfoMgr::RemoveRelayGroupByRoom( const DWORD dwRoomIndex )
{
	ThreadSync ts(this);

	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		m_vRelayGroups.RemoveKey( dwRoomIndex );

		return pRelayGroup;
	}
	return NULL;
}

RelayGroup* ioRelayGroupInfoMgr::GetRelayGroupByRoom( const DWORD dwRoomIndex )
{
	PGROUPRESULT prData = m_vRelayGroups.Lookup(dwRoomIndex);
	if(prData)
	{
		return prData->m_value;
	}
	return NULL;
}

BOOL ioRelayGroupInfoMgr::InsertUser( const DWORD dwRoomIndex, const DWORD dwUserIndex, const int iPort, const char* szIP )
{

	//LOG.PrintTimeAndLog( 0, "Test - RelayGroup InsertUser : %d, %d", (int)dwRoomIndex, (int)dwUserIndex );
	RelayGroup *pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if( pRelayGroup )
	{
		//	LOG.PrintTimeAndLog( 0, "Test - RelayGroup InsertUser : AddUser OK : %d", (int)dwUserIndex );
		pRelayGroup->AddUser(dwUserIndex, iPort, szIP);
		return TRUE;
	}
	return FALSE;
}

BOOL ioRelayGroupInfoMgr::RemoveUser( const DWORD dwRoomIndex, const DWORD dwUserIndex )
{
	ThreadSync ts(this);

	RelayGroup *pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		pRelayGroup->RemoveUser(dwUserIndex);
		return TRUE;
	}
	return FALSE;
}

BOOL ioRelayGroupInfoMgr::GetRelayUserList(const DWORD dwUserIndex, RelayGroup::RelayGroups& vGroupUsers)
{
	DWORD dwRoomIndex = g_RelayRoomInfoMgr->GetRoomIndexByUser(dwUserIndex);
	if(dwRoomIndex != 0)
	{
		ThreadSync ts(this);

		RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
		if(pRelayGroup) 
		{
			pRelayGroup->GetUserLists(vGroupUsers);
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
#else


#include "../Network/ioSkillInfoMgr.h"
extern CLog CheatLOG;

//�׽�Ʈ��
bool bDetailSkillLog = false;

ioRelayGroupInfoMgr::ioRelayGroupInfoMgr(void)
{
	m_bRoomLog = m_bIsWrite = false;
	m_dwAntiWaitTime = 1000;
	m_fAntiErrorRate = 0.8f;
	m_dwPenguinCount = 8;
	m_dwKickCount = 50;
	m_iTimeDecrease = 30;

	m_dwSkillPenguinCount = 5;
	m_dwSkillKickCount = 15;
	m_iSkillTimeDecrease = 30000;
}

ioRelayGroupInfoMgr::~ioRelayGroupInfoMgr(void)
{
}

void ioRelayGroupInfoMgr::InitData(int iSeedRoom, DWORD dwAntiWaitTime, float fAntiErrorRate, int iPenguinCount, int iKickCount, int iTimeDecrease, int iSkillHackCount, int iSkillKickCount, int iSkillTimeDrease, std::vector<int>& vecExceptList )
{
	m_dwAntiWaitTime = dwAntiWaitTime;
	m_fAntiErrorRate = fAntiErrorRate;

	m_dwPenguinCount = iPenguinCount;
	m_dwKickCount = iKickCount;
	m_iTimeDecrease = iTimeDecrease;

	m_dwSkillPenguinCount = iSkillHackCount;
	m_dwSkillKickCount = iSkillKickCount;
	m_iSkillTimeDecrease = iSkillTimeDrease;

	m_vecExceptID = vecExceptList;

	for(int i=0 ; i < iSeedRoom ; i++)
	{
		RelayGroup* pRelayGroup = new RelayGroup;
		if(pRelayGroup)
		{
			pRelayGroup->m_dwRoomIndex = -1;
			pRelayGroup->m_bWriteLog = false;
			pRelayGroup->m_RelayUserList.reserve(MAX_PLAYER);
			m_vRelayGroupPool.Push(pRelayGroup);
		}
	}
}

void ioRelayGroupInfoMgr::InsertRoom( RelayHeader* pRelayHeader )
{
	InsertData* pData = reinterpret_cast<InsertData*>(pRelayHeader);
	InsertData &rkData = *pData;

	if(!InsertUser(rkData))
	{
		// ����
		CreateRelayGroup(rkData);
	}

	g_RelayRoomInfoMgr->InsertRoomInfo(rkData.m_dwUserIndex, rkData.m_dwRoomIndex);
}

void ioRelayGroupInfoMgr::RemoveRoom( RelayHeader* pRelayHeader )
{
	RemoveData* pData = reinterpret_cast<RemoveData*>(pRelayHeader);
	RemoveData &rkData = *pData;

	if( rkData.m_dwUserIndex == 0 )
	{
		// �� ��°�� ����
		RelayGroup *pRelayGroup = RemoveRelayGroupByRoom( rkData.m_dwRoomIndex );
		if(pRelayGroup)
		{
			for(DWORD i=0; i<pRelayGroup->m_RelayUserList.size(); i++)
			{
				g_RelayRoomInfoMgr->RemoveRoomInfo(pRelayGroup->m_RelayUserList[i].m_dwUserIndex);
			}

			pRelayGroup->InitData();

			m_vRelayGroupPool.Push(pRelayGroup);
		}
	}
	else
	{
		// ������ ����
		RemoveUser(rkData.m_dwRoomIndex, rkData.m_dwUserIndex);

		g_RelayRoomInfoMgr->RemoveRoomInfo(rkData.m_dwUserIndex);
	}
}

BOOL ioRelayGroupInfoMgr::CreateRelayGroup( InsertData& rkData )
{
	RelayGroup* pRelayGroup = m_vRelayGroupPool.Pop();
	if( pRelayGroup )
	{
		if( IsRoomLog() )
		{
			if( IsWriteLog() == false )
			{
				SetWriteLog( true );
				pRelayGroup->SetWriteLog( true );
			}
			else
				pRelayGroup->SetWriteLog( false );
		}
		else
			pRelayGroup->SetWriteLog( false );

		pRelayGroup->m_dwRoomIndex = rkData.m_dwRoomIndex;
		pRelayGroup->m_iCoolType = rkData.m_iCoolType;
		pRelayGroup->m_iModeType = rkData.m_iModeType;
		pRelayGroup->m_iRoomStyle = rkData.m_iRoomStyle;
		pRelayGroup->AddUser(rkData.m_dwUserIndex, rkData.m_iClientPort, rkData.m_szPublicIP, rkData.m_dwUserSeed, rkData.m_dwNPCSeed, rkData.m_iTeamType );

		pRelayGroup->InitMode();

		m_vRelayGroups.SetAt( rkData.m_dwRoomIndex, pRelayGroup );

		//LOG.PrintTimeAndLog( 0, "Test - RelayGroup Create : %d, %d", (int)dwRoomIndex, (int)dwUserIndex );
		return TRUE;
	}

	LOG.PrintTimeAndLog(0, "[relay] CreateRelayGroup failed");
	return FALSE;
}

RelayGroup* ioRelayGroupInfoMgr::RemoveRelayGroupByRoom( const DWORD dwRoomIndex )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		if( IsRoomLog() )
		{
			if( IsWriteLog() )
			{
				if( pRelayGroup->IsWriteLog() )
				{
					pRelayGroup->SetWriteLog( false );
					SetWriteLog( false );
				}
			}
		}


		m_vRelayGroups.RemoveKey( dwRoomIndex );

		return pRelayGroup;
	}
	return NULL;
}

RelayGroup* ioRelayGroupInfoMgr::GetRelayGroupByRoom( const DWORD dwRoomIndex )
{
	PGROUPRESULT prData = m_vRelayGroups.Lookup(dwRoomIndex);
	if(prData)
	{
		return prData->m_value;
	}
	return NULL;
}

BOOL ioRelayGroupInfoMgr::InsertUser( InsertData& rkData )
{
	RelayGroup *pRelayGroup = GetRelayGroupByRoom( rkData.m_dwRoomIndex );
	if( pRelayGroup )
	{
		if( pRelayGroup->m_iCoolType != rkData.m_iCoolType ||
			pRelayGroup->m_iModeType != rkData.m_iModeType || 
			pRelayGroup->m_iRoomStyle != rkData.m_iRoomStyle )
			LOG.PrintTimeAndLog( 0, "[relay] TEST - InsertUser CoolTypeCheck Error roomIndex(%u), uIdx(%u), CoolType(%d/%d),ModeType(%d/%d),RoomStyle(%d/%d)",
			pRelayGroup->m_dwRoomIndex, rkData.m_dwRoomIndex, rkData.m_dwUserIndex, pRelayGroup->m_iCoolType, rkData.m_iCoolType, pRelayGroup->m_iModeType, rkData.m_iModeType, pRelayGroup->m_iRoomStyle, rkData.m_iRoomStyle );

		pRelayGroup->AddUser(rkData.m_dwUserIndex, rkData.m_iClientPort, rkData.m_szPublicIP, rkData.m_dwUserSeed, rkData.m_dwNPCSeed, rkData.m_iTeamType );
		return TRUE;
	}
	return FALSE;
}

BOOL ioRelayGroupInfoMgr::RemoveUser( const DWORD dwRoomIndex, const DWORD dwUserIndex )
{
	RelayGroup *pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		UserData* pUserData = pRelayGroup->GetUserData(dwUserIndex);
		if( pUserData )
		{
			DWORD dwTotalCnt = pUserData->m_dwUserSeed - pUserData->m_dwUserSeedOri;
			dwTotalCnt += pUserData->m_dwNPCSeed - pUserData->m_dwNPCSeedOri;
			if( dwTotalCnt != 0 )
			{
				CheatLOG.PrintTimeAndLog( 0, "[RUDP][INFO] RudpInfo uid:%u, RoomIndex:%u, modeType:%d, total:%u, req:%u, ack:%u, dup:%u, err:%u, loss:%u, del:%u",
					dwUserIndex, dwRoomIndex, (int)pRelayGroup->m_iModeType, dwTotalCnt,
					pUserData->m_dwReqCount, pUserData->m_dwAckCount, pUserData->m_dwAckCountDupl, pUserData->m_dwErrCount, pUserData->m_dwLossCount, pUserData->m_dwEraseCount );

				CheatLOG.PrintTimeAndLog( 0, "[ANTI][INFO] AntiInfo - uid:%u, RoomIndex:%u, modeType:%d, gamePlayValue(%u), Hit Total(%u), Hit Penalty(%u), Skill Total(%u), Skill Penalty(%u)",
					dwUserIndex, dwRoomIndex, (int)pRelayGroup->m_iModeType, dwTotalCnt, pUserData->m_dwAntiHitCount, pUserData->m_dwAntiHitCount2, pUserData->m_dwAntiSkillCount, pUserData->m_dwAntiSkillCount2 );
			}			
		}

		if( IsRoomLog() )
		{
			if( IsWriteLog() )
			{
				if( pRelayGroup->IsWriteLog() )
				{
					pRelayGroup->SetWriteLog( false );
					SetWriteLog( false );
				}
			}
		}

		pRelayGroup->RemoveUser(dwUserIndex);

		if( pRelayGroup->GetUserCount() == 0 )
		{
			m_vRelayGroups.RemoveKey( dwRoomIndex );

			pRelayGroup->InitData();

			m_vRelayGroupPool.Push(pRelayGroup);
		}
		return TRUE;
	}
	return FALSE;
}

RelayGroup::RelayGroups* ioRelayGroupInfoMgr::GetRelayUserList( const DWORD dwUserIndex )
{
	DWORD dwRoomIndex = g_RelayRoomInfoMgr->GetRoomIndexByUser(dwUserIndex);
	if(dwRoomIndex != 0)
	{
		RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
		if(pRelayGroup) 
		{
			return pRelayGroup->GetUsers();
		}
	}

	return NULL;
}

BOOL ioRelayGroupInfoMgr::BroadcastPacket( DWORD dwUserIndex, SP2Packet& kPacket )
{
	RelayGroup::RelayGroups* pUsers = GetRelayUserList( dwUserIndex );

	if( pUsers == NULL )
		return FALSE;

	for(unsigned int i=0; i< pUsers->size(); i++)
	{
		kPacket.SetPosBegin();
		UserData& rkUser = pUsers->at(i);
		if(rkUser.m_dwUserIndex != dwUserIndex)
		{
			g_UDPNode.SendMessage(rkUser.m_szPublicIP, rkUser.m_iClientPort, kPacket);
		}
	}
	return TRUE;
}

bool ioRelayGroupInfoMgr::CheckControlSeed( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwSeed, DWORD dwHostTime, bool bUser )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( pRelayGroup )
	{
		UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
		if( pUserData )
		{
			DWORD& dwUpdateSeed = (bUser == true) ? pUserData->m_dwUserSeed : pUserData->m_dwNPCSeed;
			std::vector<DWORD>& dwVecID = (bUser == true) ? pUserData->m_vecRUDP : pUserData->m_vecRUDPNPC;
			std::vector<DWORD>& dwVecIDErase = bUser == true ? pUserData->m_vecRUDPErase : pUserData->m_vecRUDPEraseNPC;
			std::vector<DWORD>& dwVecIDTest = (bUser == true) ? pUserData->m_vecRUDPTest : pUserData->m_vecRUDPTestNPC;

			//min üũ�� rudp seed �� �� �ִ� ��� ��������, max üũ�� ���� ���ǵ� �� ��Ŷ�� �� ���� ����
			DWORD dwSeedCheckValue = (bUser == true) ? MAX_PASS_SEED_VALUE_USER : MAX_PASS_SEED_VALUE_NPC;
			if( dwSeed < (dwUpdateSeed-dwSeedCheckValue) ||
				(dwUpdateSeed+(dwSeedCheckValue/2)) < dwSeed )
			{
				// �̻� �õ� ��
				//CheatLOG.PrintTimeAndLog( 0, "[RUDP][seed] user seed check err userIndex(%u), seed(%u,%u)", pUserData->m_dwUserIndex, dwUpdateSeed, dwSeed );
				dwUpdateSeed = dwSeed;
				return false;
			}

			bool bSend = false;
			if( dwUpdateSeed < dwSeed )
			{
				for( DWORD i = dwUpdateSeed + 1; i < dwSeed; ++i )
				{
					bSend = true;
					dwVecID.push_back( i );
					pUserData->m_dwReqCount++;
				}

				//�õ� ����
				if( dwUpdateSeed < dwSeed )
					dwUpdateSeed = dwSeed;

				//���� �ƴϸ� pass
				if( bUser == false )
					return true;

				//����� �ʹ� Ŀ����..
				if( dwVecID.size() > MAX_SEED_CHECK_COUNT )
				{
					int vSize = dwVecID.size();
					int delSize = vSize - MAX_SEED_CHECK_COUNT;

					//����� ���� ���� �ְ�
					for( int i = 0; i < delSize; ++i )
						dwVecIDErase.push_back( dwVecID[i] );

					dwVecID.erase( dwVecID.begin(), dwVecID.begin() + delSize );
					pUserData->m_dwLossCount += delSize;
				}

				if( dwVecIDErase.size() > MAX_SEED_CHECK_COUNT )
				{
					int vSize = dwVecIDErase.size();
					int delSize = vSize - MAX_SEED_CHECK_COUNT;
					dwVecIDErase.erase( dwVecIDErase.begin(), dwVecIDErase.begin() + delSize );
				}

				//�ε����� �ʹ� ���� ���̳��� ���� �ؾ���..
				DWORD dwSeedLiveCnt = bUser == true ? MAX_PASS_SEED_VALUE_USER : MAX_PASS_SEED_VALUE_NPC;
				if( !dwVecID.empty() )
				{
					//ù��° �������� ���� �����Ȱ��ε�, �̰� ���� ���� ���� �� üũ.
					if( (dwUpdateSeed-dwVecID[0]) > dwSeedLiveCnt )
					{
						auto it = dwVecID.begin();
						while( it != dwVecID.end() )
						{
							if( (dwUpdateSeed - *it) > dwSeedLiveCnt )
							{
								it = dwVecID.erase(it);
								pUserData->m_dwLossCount++;
							}
							else
								break;
						}

					}
				}

				//��û, ��� seed�� ������ �������� �����µ�.. NPC�� ��û �� �ϰ� ����
				if( !dwVecID.empty() && bSend )
				{
					SP2Packet kPacket( SUPK_RUDP );
					//����� ������..
					//PACKET_GUARD_BOOL( kPacket.W
					DWORD dwSize = dwVecID.size();
					PACKET_GUARD_BOOL( kPacket.Write( dwSize ));
					for( DWORD i = 0; i < dwSize; ++i )
						PACKET_GUARD_BOOL( kPacket.Write( dwVecID[i] ) );

					g_UDPNode.SendMessage( pUserData->m_szPublicIP, pUserData->m_iClientPort, kPacket);
				}
			}
			//�����ϰ��
			else
			{
				auto it = std::find( dwVecID.begin(), dwVecID.end(), dwSeed );
				if( it != dwVecID.end() )
				{
					pUserData->m_dwAckCount++;
					dwVecID.erase( it );
					dwVecIDTest.push_back( dwSeed );
					return true;
				}
				else
				{
					//������ ��Ŷ�� �ι��̻� �ð��..
					auto it = std::find( dwVecIDTest.begin(), dwVecIDTest.end(), dwSeed );
					if( it != dwVecIDTest.end() )
					{
						pUserData->m_dwAckCountDupl++;
					}
					//���� �ڿ� �ð���ε� üũ��..
					it = std::find( dwVecIDErase.begin(), dwVecIDErase.end(), dwSeed );
					if( it != dwVecIDErase.end() )
					{
						pUserData->m_dwEraseCount++;
					}
					else
					{
						pUserData->m_dwErrCount++;
					}
					return false;
				}
			}
		}
		else
		{
			LOG.PrintTimeAndLog( 0, "[relay][seed] TEST - checkcontrolseed user index error - room(%u), user(%u)", dwRoomIndex, dwUserIndex );
			return false;
		}
	}
	else
	{
		LOG.PrintTimeAndLog( 0, "[relay][seed] TEST - checkcontrolseed room index error - room(%u), user(%u)", dwRoomIndex, dwUserIndex );
		return false;
	}
	return true;
}

void ioRelayGroupInfoMgr::NotUseControlSeed( DWORD dwRoomIndex, DWORD dwUserIndex, SP2Packet& rkPacket )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if(pRelayGroup == NULL)
		return;
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
		return;

	//�ϴ� ������
	bool bUser = true;
	DWORD dwSize;
	rkPacket >> dwSize;
	MAX_GUARD(dwSize, 100);
	std::vector<DWORD>& dwVecID = (bUser == true) ? pUserData->m_vecRUDP : pUserData->m_vecRUDPNPC;
	for( DWORD i = 0; i < dwSize; ++i )
	{
		DWORD dwID;
		rkPacket >> dwID;

		auto it = std::find( dwVecID.begin(), dwVecID.end(), dwID );
		if( it != dwVecID.end() )
		{
			dwVecID.erase( it );
			pUserData->m_dwLossCount++;
		}
	}
}

float GetCoolTypebyRoomOption( int iCoolType )
{
	float fResult = 1.f;
	switch( iCoolType )
	{
	case 4 :
		fResult = 1.f;
		break;
	case 0 :
		fResult = 0.f;
		break;
	case 1 :
		fResult = 0.25f;
		break;
	case 2 :
		fResult = 0.5f;
		break;
	case 3 :
		fResult = 0.75f;
		break;
	case 5 :
		fResult = 1.25f;
		break;
	case 6 :
		fResult = 1.5f;
		break;
	case 7 :
		fResult = 2.f;
		break;
	case 8 :
		fResult = 3.f;
		break;
	case 9:
		fResult = 5.f;
		break;
	}
	return fResult;
}

void ioRelayGroupInfoMgr::OnSkillUse( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, ioHashString& hsSkillName, int iLevel, float fGauge )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pUserData NULL" );
		return;
	}
	ioSkillInfoMgr* pSkillMgr = g_SkillInfoMgr;
	if( !pSkillMgr )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pSkillMgr NULL" );
		return;
	}

	SkillInfo* pSkillInfo = pSkillMgr->GetSkillInfo( hsSkillName );
	if( !pSkillInfo )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pSkillInfo NULL, SkillName(%s)", hsSkillName.c_str() );
		return;
	}

	if( pSkillInfo->iEquipType < 0 || pSkillInfo->iEquipType > 3 )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - iEquipType error %d", pSkillInfo->iEquipType );
		return;
	}

	//except ����Ʈ �˻�!!
	if( IsExceptID(pSkillInfo->iSkillID) )
		return;

	if( bDetailSkillLog )
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - OnSkillUse iSlot(%d) room/user(%u/%u)",pSkillInfo->iEquipType, dwRoomIndex, dwUserIndex );

	if( IsCheckSkillTime( pUserData, pSkillInfo )  )	
		CheckSkillTime( pUserData, pRelayGroup, pSkillInfo, iLevel, dwHostTime );

	OnAfterSkill( pUserData, pSkillInfo, dwHostTime );
}

void ioRelayGroupInfoMgr::UpdateRelayGroupWin( DWORD dwRoomIndex, int iRedTeamWinCnt, int iBlueTeamWindCnt )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		pRelayGroup->m_iWinCntRed = iRedTeamWinCnt;
		pRelayGroup->m_iWinCntBlue = iBlueTeamWindCnt;
		pRelayGroup->CalculateTeamWinCnt();
	}
	else
		;//CheatLOG.PrintTimeAndLog( 0, "[relay] TEST - UpdateRelayGroupWin RoomIndex(%u) not exist", dwRoomIndex );
}

void ioRelayGroupInfoMgr::UpdateRelayGroupScore( DWORD dwRoomIndex, int iTeamType )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		// ���� �������� �Ѿ��
		// RED = 1, BLUE= 2
		if( iTeamType == 1 )
			pRelayGroup->IncTeamScoreBlue();
		else if( iTeamType == 2 )
			pRelayGroup->IncTeamScoreRed();

		pRelayGroup->CalculateTeamScore();
	}
	else
		;//CheatLOG.PrintTimeAndLog( 0, "[relay] TEST - UpdateRelayGroupScore RoomIndex(%u) not exist", dwRoomIndex );
}

void ioRelayGroupInfoMgr::OnSkillAstRandomGenerate( UserData* pUser, SkillInfo* pSkillInfo, float fRate, float fUseTimeDiff )
{
	//�⺻������ üũ�� ����ϰ� �ٷ� ����Ͽ������� üũ��
	// ���ڵ�� ���۰� ���ÿ� ��� �����ؼ� �ּ� ��� �ð���ŭ���ε�..������ ��� 1.5�� �������� ��������!
	fRate *= 1.5f;

	// ���� �����ð��� 200ms��
	float fLowestTime = 0.2f;
	float fUsingTime = pSkillInfo->fNeedGauge + fLowestTime + 1.f;	//���� �ٷ� 1.f ����
	fUsingTime /= fRate;
	fUsingTime *= 1000.f;
	// �� ����� ���۰� ���ÿ� ����ϰ� �ٷ� Ǯ���� �� ��� �ε�, ��ǻ� �Ұ�����.. ���� �����ϰ� �ص� �ɵ�?
	// fUsingTime ����ŭ ä���� ��
	if( fUsingTime > fUseTimeDiff ) 
	{
		AddPenaltySkill( pUser, pSkillInfo->hsSkillName, fUseTimeDiff, fUsingTime );
// 		CheatLOG.PrintTimeAndLog( 0, "[skill][hack] Check - Uidx(%u), SkillName(%s), fCheckTiem(%f), fUseTimeDiff(%f)"	
// 			, pUser->m_dwUserIndex, hsSkillName.c_str(), fUsingTime, fUseTimeDiff );
	}
}

void ioRelayGroupInfoMgr::OnSkillBasic( UserData* pUser, ioHashString& hsSkillName, float fIncMaxCoolTime, float fUseTimeDiff, float fErrorRate )
{
	if( fIncMaxCoolTime > fUseTimeDiff )
	{
		// 20���� ������ ������ ���
		if( fIncMaxCoolTime * fErrorRate > fIncMaxCoolTime - fUseTimeDiff )
		{
			/*// ��������� �α�..
			CheatLOG.PrintTimeAndLog( 0, "[skill][warn] Check - Uidx(%u), SkillName(%s), SkillUseTimeDiff(%f), fAllowTime(%f)"	
				, pUser->m_dwUserIndex, hsSkillName.c_str(), fUseTimeDiff, fIncMaxCoolTime * fErrorRate );*/
		}
		else
		{
			//ġ�� �ǽ�
			AddPenaltySkill( pUser, hsSkillName, fUseTimeDiff, fIncMaxCoolTime );			
// 			CheatLOG.PrintTimeAndLog( 0, "[skill][hack] Check - Uidx(%u), SkillName(%s), SkillUseTimeDiff(%f), fAllowTime(%f)"	
// 				, pUser->m_dwUserIndex, hsSkillName.c_str(), fUseTimeDiff, fIncMaxCoolTime * fErrorRate );
		}
	}
}

void ioRelayGroupInfoMgr::OnSkillBuff( UserData* pUser, SkillInfo* pSkillInfo, float fIncMaxCoolTime, float fUseTimeDiff, float fErrorRate )
{	
	if( fIncMaxCoolTime > fUseTimeDiff )
	{
		// ��ų ��� ���� �������� �Ҹ�Ǵ� ���µ�..
		// ������ڸ��� �ٷ� ��ҵǾ��ٰ� �����ϰ�..

		float fRemainGauge = pSkillInfo->fMaxGauge - pSkillInfo->fNeedGauge;
		float fRemainIncTime = fIncMaxCoolTime - (fRemainGauge / pSkillInfo->fMaxGauge) * fIncMaxCoolTime;
		float fChecktime = fRemainIncTime;
		if( fChecktime > fUseTimeDiff )
		{
			//���⼭���� �ϴ� �ǽ�.
			fChecktime = fChecktime * (1.f - fErrorRate);
			float fDiffTime = fRemainIncTime - fUseTimeDiff;
			float fAllowTime = fIncMaxCoolTime * fErrorRate;
			if( fChecktime > fDiffTime )
			{
				/*// �ϴ� ����ϴ°ɷ�
				CheatLOG.PrintTimeAndLog( 0, "[skill][warn] TEST - WARN %s, fChecktime(%f), fDiffTime(%f)", pSkillInfo->hsSkillName.c_str(), fChecktime, fDiffTime );*/
			}
			else
			{
				AddPenaltySkill( pUser, pSkillInfo->hsSkillName, fUseTimeDiff, fChecktime );
				// ġ�� ���� �� �� ������ ���!
// 				CheatLOG.PrintTimeAndLog( 0, "[skill][hack] Check - Uidx(%u), SkillName(%s), fChecktime(%f), fDiffTime(%f)"
// 					, pUser->m_dwUserIndex, pSkillInfo->hsSkillName.c_str(), fChecktime, fDiffTime );
			}
		}
	}
}

void ioRelayGroupInfoMgr::OnSkillCount( UserData* pUser, SkillInfo* pSkillInfo, DWORD dwCurTime )
{
	// ��Ȯ�� üũ�� �Ұ�����. �׳� �����ϰ� �����ð��� �� �� �ִ� ���� + �������� �� �� ���� �ð� 2���� üũ��.
	std::vector<DWORD>& vecTimer = pUser->m_dwVecMultiCountSkill[pSkillInfo->iEquipType];
	vecTimer.push_back( dwCurTime );

	//���� �ð� �̻�Ȱ� ����
	int nMaxCount = pSkillInfo->iMaxCount / pSkillInfo->iNeedCount;
	float fNeedTime = pSkillInfo->fNeedGauge * pSkillInfo->iTickTime;
	float fNotCheckTime = fNeedTime * nMaxCount;

	auto it = vecTimer.begin();
	while( it != vecTimer.end() )
	{
		if( (dwCurTime - *it) > fNotCheckTime )
		{
			it = vecTimer.erase( it );
		}
		else
			break;
	}

	//�������� �� �� �ִ� ����			
	int nCurSize = vecTimer.size();
	if( nCurSize <= nMaxCount )
	{
		//�ϴ� ó�� 3���� ������ �ް�
	}
	else
	{
		// 2���� ���̽� �˻�.
		// 1. �ִ� �޽��Ŀ� 3�� ������� �ٷ� �ٷ� ����� ��� 3-4 �������� �ּ� ������ �ð��� �����ؾ���
		// 2. 1���� ���������� �ɸ� �ð���� ��밡��Ƚ�� üũ..���� ����Ȯ������?

		const float fLowestAnitime = 150.f;
		float fLastedTime = dwCurTime - vecTimer[0];	
		float fCheckTime = (nCurSize-nMaxCount) * fNeedTime;
		if( fLastedTime < fCheckTime )
		{
			AddPenaltySkill( pUser, pSkillInfo->hsSkillName, fLastedTime * 0.001f, fCheckTime * 0.001f );
			/*CheatLOG.PrintTimeAndLog( 0, "[relay] hack Check Count Skill - Uidx(%u), SkillName(%s), LastTime(%.2f), fCheckTime(%.2f)",
				pUser->m_dwUserIndex, pSkillInfo->hsSkillName.c_str(), fLastedTime, fCheckTime );*/
		}
	}
}

void ioRelayGroupInfoMgr::OnAntiHackHit( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, DWORD dwHitUserIndex, DWORD dwWeaponIndex, BYTE eAttackType )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwHitUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pUserData NULL" );
		return;
	}

	DWORD dwTarget = dwNPCIndex != 0 ? dwNPCIndex : dwUserIndex;

	/*if( bDetailLog )
		CheatLOG.PrintTimeAndLog( 0, "Test OnAntiHackHit - send(%u) -  %u >> %u", dwUserIndex, dwTarget, dwHitUserIndex );*/

	pUserData->AddHitData( dwTarget, dwWeaponIndex/*, pRelayGroup->IsWriteLog()*/ );
}

void ioRelayGroupInfoMgr::OnAntiHackWounded( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, DWORD dwAttackerIndex, DWORD dwWeaponIndex, BYTE eAttackType )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pUserData NULL" );
		return;
	}

	/*if( bDetailLog )
		CheatLOG.PrintTimeAndLog( 0, "Test - OnAntiHackWounded send(%u) -  %u << %u", dwUserIndex, dwUserIndex, dwAttackerIndex );*/

	pUserData->AddWoundedData( dwAttackerIndex, dwWeaponIndex/*, pRelayGroup->IsWriteLog()*/ );
}

void ioRelayGroupInfoMgr::OnAntiHackDefence( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, DWORD dwAttackerIndex, DWORD dwWeaponIndex, BYTE eAttackType )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[anti] TEST - pUserData NULL" );
		return;
	}

	pUserData->AddWoundedData( dwAttackerIndex, dwWeaponIndex/*, pRelayGroup->IsWriteLog()*/ );
}

void ioRelayGroupInfoMgr::CheckAntiHackData()
{
	DWORD dwCurTime = TIMEGETTIME();

	POSITION pos = m_vRelayGroups.GetStartPosition();
	while(pos)
	{
		RelayGroup* pRelayGroup = m_vRelayGroups.GetValueAt(pos);
		if( pRelayGroup )
		{
			RelayGroup::RelayGroups* pRelayGroups = pRelayGroup->GetUsers();
			if( pRelayGroups )
			{
				for( DWORD i = 0; i < pRelayGroups->size(); ++i )
				{
					UserData& rUserData = pRelayGroups->at(i);

					int iHackCount = 0;
					int nSize = rUserData.m_vecAntiData.size();
					int nCheckSize = 0;

					auto it = rUserData.m_vecAntiData.begin();
					while( it != rUserData.m_vecAntiData.end() )
					{
						if( dwCurTime-it->dwLastTime >= m_dwAntiWaitTime )
						{
							++nCheckSize;

							if( it->dwHitCount > it->dwWDCount )
								iHackCount++;

							it = rUserData.m_vecAntiData.erase( it );
						}
						else
							break;
					}			

					if( nCheckSize > 0 && ((float)iHackCount/(float)nCheckSize >= m_fAntiErrorRate) )
					{
						int iPenaltyPoint = iHackCount * iHackCount / nCheckSize;
						if( iPenaltyPoint < 1 ) iPenaltyPoint = 1;
						//���.
						AddPenaltyPoint( &rUserData, iPenaltyPoint );
					}
				}
			}
		}

		m_vRelayGroups.GetNext(pos);
	}
}

int ioRelayGroupInfoMgr::GetModeType( DWORD dwRoomIndex )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if( pRelayGroup )
		return pRelayGroup->GetModeType();

	return 0;
}

int ioRelayGroupInfoMgr::GetModeStyle( DWORD dwRoomIndex )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if( pRelayGroup )
		return pRelayGroup->GetModeStyle();

	return 0;
}

void ioRelayGroupInfoMgr::AddPenaltySkill( UserData* pUser, ioHashString& hsSkillName, float fTime1, float fTime2 )
{
	DWORD dwCurTime = TIMEGETTIME();
	pUser->m_dwPenaltySkill++;
	if( pUser->m_dwLastSkillPenaltyTime != 0 )
	{
		int iPenaltyDecreaseTime = 1000 * m_iSkillTimeDecrease;

		DWORD dwPastTime = dwCurTime - pUser->m_dwLastSkillPenaltyTime;
		DWORD dwCnt = dwPastTime / iPenaltyDecreaseTime;
		if( pUser->m_dwPenaltySkill < dwCnt )
		{
			// clean penalty
			pUser->m_dwPenaltySkill = 0;
		}
		else
		{
			pUser->m_dwPenaltySkill -= dwCnt;
		}
	}
	pUser->m_dwLastSkillPenaltyTime = dwCurTime;

	if( pUser->m_dwPenaltySkill < m_dwSkillKickCount )
	{
		if( pUser->m_dwPenaltySkill > m_dwSkillPenguinCount )
		{
			pUser->m_dwAntiSkillCount++;

			SP2Packet kPacket(SUPK_CLIENT_MSG);
			PACKET_GUARD_VOID( kPacket.Write((BYTE)1) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->m_dwPenaltySkill) );
			g_UDPNode.SendMessage( pUser->m_szPublicIP, pUser->m_iClientPort, kPacket );
		}
	}
	else
	{
		pUser->m_dwAntiSkillCount2 = 1;
		// Kick..
		SP2Packet iPacket( ITPK_UDP_ANTIHACK_PENALTY );
		PACKET_GUARD_VOID( iPacket.Write(pUser->m_dwUserIndex) );
		g_InternalNode->ReceivePacket( iPacket );
		//CheatLOG.PrintTimeAndLog( 0, "[anti][Kick] User Skill Penalty over - Uidx(%u), SkillName(%s)", pUser->m_dwUserIndex, hsSkillName.c_str() );
	}

	

	/*SP2Packet kPacket(SUPK_CLIENT_MSG);
	PACKET_GUARD_VOID( kPacket.Write((BYTE)0) );
	PACKET_GUARD_VOID( kPacket.Write(hsSkillName) );
	PACKET_GUARD_VOID( kPacket.Write(fTime1) );
	PACKET_GUARD_VOID( kPacket.Write(fTime2) );
	PACKET_GUARD_VOID( kPacket.Write(pUser->m_dwPenaltySkill) );
	g_UDPNode.SendMessage( pUser->m_szPublicIP, pUser->m_iClientPort, kPacket );

	CheatLOG.PrintTimeAndLog( 0, "[skill][penalty] User Penalty - Uidx(%u), SkillName(%s), Penalty(%u)"	
		, pUser->m_dwUserIndex, hsSkillName.c_str(), pUser->m_dwPenaltySkill );*/
}

void ioRelayGroupInfoMgr::AddPenaltyPoint( UserData* pUser, int iPenaltyPoint )
{
	//pUser->m_dwAntiHitCount++;

	//���Ƽ
	DWORD dwCurTime = TIMEGETTIME();
	pUser->m_dwPenaltyCount += iPenaltyPoint;
	if( pUser->m_dwLastPenaltyTime != 0 )
	{
		//Ư�� �ð����� ���Ƽ ���Ҹ� ���� �ϴ� 1�п� 1�� �ٿ���
		int iPenaltyDecreaseTime = 1000 * m_iTimeDecrease;

		DWORD dwPastTime = dwCurTime - pUser->m_dwLastPenaltyTime;
		DWORD dwCnt = dwPastTime / iPenaltyDecreaseTime;
		if( pUser->m_dwPenaltyCount < dwCnt )
		{
			// clean penalty
			pUser->m_dwPenaltyCount = 0;
		}
		else
		{
			pUser->m_dwPenaltyCount -= dwCnt;
		}
	}
	pUser->m_dwLastPenaltyTime = dwCurTime;

	if( pUser->m_dwPenaltyCount < m_dwKickCount )
	{
		if( pUser->m_dwPenaltyCount > m_dwPenguinCount )
		{
			pUser->m_dwAntiHitCount++;

			SP2Packet kPacket(SUPK_CLIENT_MSG);
			PACKET_GUARD_VOID( kPacket.Write((BYTE)2) );
			PACKET_GUARD_VOID( kPacket.Write(pUser->m_dwPenaltyCount) );
			g_UDPNode.SendMessage( pUser->m_szPublicIP, pUser->m_iClientPort, kPacket );

			
		}
	}
	else
	{
		pUser->m_dwAntiHitCount2 = 1;
		// Kick..
		SP2Packet iPacket( ITPK_UDP_ANTIHACK_PENALTY );
		PACKET_GUARD_VOID( iPacket.Write(pUser->m_dwUserIndex) );
		g_InternalNode->ReceivePacket( iPacket );
		//CheatLOG.PrintTimeAndLog( 0, "[anti][Kick] User Hit Penalty over - Uidx(%u)", pUser->m_dwUserIndex );
	}

	/*// �⺻ ���Ƽ ������ �������� �˷��ش�
	SP2Packet kPacket(SUPK_CLIENT_MSG);
	PACKET_GUARD_VOID( kPacket.Write((BYTE)2) );
	PACKET_GUARD_VOID( kPacket.Write(pUser->m_dwPenaltyCount) );
	g_UDPNode.SendMessage( pUser->m_szPublicIP, pUser->m_iClientPort, kPacket );*/
}

DWORD ioRelayGroupInfoMgr::GetLastSkillTime( UserData* pUserData, SkillInfo* pSkillInfo )
{
	if( pSkillInfo->iSkillType == ST_COUNT )
		return 0;

	if( pSkillInfo->iSkillSubType == AST_RANDOM_GENERATE )
		return pUserData->m_dwSkillUseTime2[pSkillInfo->iEquipType];

	return pUserData->m_dwSkillUseTime[pSkillInfo->iEquipType];
}

bool ioRelayGroupInfoMgr::IsCheckSkillTime( UserData* pUserData, SkillInfo* pSkillInfo )
{
	DWORD dwLastSkillTime = GetLastSkillTime( pUserData, pSkillInfo );
	if( dwLastSkillTime == 0 )
		return false;

	return true;
}

void ioRelayGroupInfoMgr::CheckSkillTime( UserData* pUserData, RelayGroup* pRelayGroup, SkillInfo* pSkillInfo,  int iLevel, DWORD dwHostTime )
{
	ioSkillInfoMgr* pSkillMgr = g_SkillInfoMgr;
	if( !pSkillMgr )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pSkillMgr NULL" );
		return;
	}

	//����ġ�� ����� ��Ÿ��
	float fMaxCoolTime = pSkillMgr->GetMaxSkillCoolTime( pSkillInfo->hsSkillName, iLevel );
	if( fMaxCoolTime == 0 )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - Skill Max CoolTime error. SkillName(%s)", pSkillInfo->hsSkillName.c_str() );
		return;
	}

	DWORD dwLastTime = 0;
	DWORD dwCurTime = 0;
	//���� �ð��� ���� �ð��� ���̷� ���..
	if( pSkillInfo->iSkillType != ST_COUNT )
	{
		dwLastTime = GetLastSkillTime( pUserData, pSkillInfo );
		dwCurTime = dwHostTime;
		if( dwLastTime > dwCurTime )
		{
			// ���� ��Ȳ�̰ų� �������� ���� �ǽ�..
			//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - SkillTime Cheat Doubt Last/Cur(%u/%u)", dwLastTime, dwCurTime );
			//������ ����� ���� ���ͼ� ���⼭�� ���Ƽ ������
			AddPenaltySkill( pUserData, pSkillInfo->hsSkillName, 0.f, 0.f );
			return;
		}
	}

	float fErrorRate = pSkillMgr->GetErrorRate();

	// ��ų ����
	DWORD dwUseDiffTime = dwCurTime - dwLastTime;
	float fUseDifftime = dwUseDiffTime;// * 0.1f;

	// ���� �ɼ��� �ݿ��� ��갪
	//���⼭ ��ɼ�, ���, �⿩�� ����� ���� �򰡷� ��Ÿ�� �������� ��..
	float fRate = 1.f;

	//��ɼ�
	fRate *= GetCoolTypebyRoomOption(pRelayGroup->m_iCoolType);
	// �ο���
	// ���ھ�
	// ��庰 ���� ����
	if( pUserData->m_iTeamType == TEAM_RED )
	{
		fRate *= pRelayGroup->GetTeamBalanceRed();
		fRate *= pRelayGroup->GetWinBalanceRed();
		fRate *= pRelayGroup->GetModeBalanceRed();
	}
	else if( pUserData->m_iTeamType == TEAM_BLUE )
	{
		fRate *= pRelayGroup->GetTeamBalanceBlue();
		fRate *= pRelayGroup->GetWinBalanceBlue();
		fRate *= pRelayGroup->GetModeBalanceBlue();
	}
	else
	{
		//������
		fRate *= pRelayGroup->GetModeBalanceRed();
	}
	//////////////////////////////////////////////////////////////////////////
	//������κ�			
	//����Ʈ Ŭ���Ͻ� 1~1.3f * 1~1.3f
	//�ұ����÷��� �Ͻ� 0~0.5f?
	//////////////////////////////////////////////////////////////////////////

	// �ƽ� ��Ÿ�ӿ� �����ӵ��� ���ؼ� ��� ��������
	float fIncMaxCoolTime = fMaxCoolTime / fRate;
	fIncMaxCoolTime *= 1000.f;
	switch( pSkillInfo->CheckSkillType() )
	{
	case SCT_BASIC:
		OnSkillBasic( pUserData, pSkillInfo->hsSkillName, fIncMaxCoolTime, fUseDifftime, fErrorRate );
		break;
	case SCT_NEED:
		OnSkillBuff( pUserData, pSkillInfo, fIncMaxCoolTime, fUseDifftime, fErrorRate );
		break;
	case SCT_COUNT:
		OnSkillCount( pUserData, pSkillInfo, dwCurTime );
		break;
	case SCT_AST:
		OnSkillAstRandomGenerate( pUserData, pSkillInfo, fRate, fUseDifftime );
		break;
	case SCT_RETURN:
		//����
		break;
	}
}

void ioRelayGroupInfoMgr::OnAfterSkill( UserData* pUserData, SkillInfo* pSkillInfo, DWORD dwHostTime )
{
	// �ð� ���
	SetLastSkillTime( pUserData, pSkillInfo, dwHostTime );
	// ȸ����ų�� ���..CUPK_ANTIHACK_SKILL ��ų�� �߰� ���� �Լ�
	if( pSkillInfo->bRecoverGauge )
	{
		Debug( "OnAfterSkill Start- SkillIdx(%d), Time(%u)", pSkillInfo->iSkillID, dwHostTime );
		pUserData->OnRecvoerSkill( dwHostTime );
	}
	else if( pSkillInfo->bReloadBullet )
	{
		//���� ������ ������ ��� �� �� �ֳ�?
		UserData::sBulletData* pBulletData = pUserData->GetBulletData( pSkillInfo->dwExtraValue );
		pBulletData->Clear();
	}
	//�ֻ��� ��ų�� �����ڵ�
}

void ioRelayGroupInfoMgr::SetLastSkillTime( UserData* pUserData, SkillInfo* pSkillInfo, DWORD dwHostTime )
{
	if( pSkillInfo->iSkillType == ST_COUNT )
		return;

	if( pSkillInfo->iSkillSubType == AST_RANDOM_GENERATE )
		pUserData->m_dwSkillUseTime2[pSkillInfo->iEquipType] = dwHostTime;
	else
		pUserData->m_dwSkillUseTime[pSkillInfo->iEquipType] = dwHostTime;
}

void ioRelayGroupInfoMgr::OnSkillAntihack( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, int nSize, int* iSlots, DWORD* dwLatedTime )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pUserData NULL" );
		return;
	}

	Debug( "OnSkillAntihack - uidx(%u), Time(%u)", dwUserIndex, dwHostTime );
	
	pUserData->OnRecoverSkillExtraInfo( dwHostTime, nSize, iSlots, dwLatedTime );
}

void ioRelayGroupInfoMgr::OnSkillExtraInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, int iSize, DWORD* dwUserIndexs )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom(dwRoomIndex);
	if( !pRelayGroup )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pRelayGroup NULL" );
		return;
	}
	UserData* pUserData = pRelayGroup->GetUserData( dwUserIndex );
	if( !pUserData )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] TEST - pUserData NULL" );
		return;
	}

	Debug( "OnSkillExtraInfo - uidx(%u), Time(%u)", dwUserIndex, dwHostTime );

	// �ش� �������� ��Ŀ���� �����ϴٰ� ��ȣ�� ��
	for( int i = 0; i < iSize; ++i )
	{
		UserData* pTargetUser = pRelayGroup->GetUserData( dwUserIndexs[i] );
		if( pTargetUser )
		{
			pTargetUser->OnRecvoerSkill( dwHostTime );
		}
	}
}

void ioRelayGroupInfoMgr::ReloadAntiHack( float fAntiErrorRate, int iAntiWaitTime, int iPenguinCount, int iKickCount, int iTimeDecrease, int iSkillHackCount, int iSkillKickCount, int iSkillTimeDecrease, int* iArray )
{
	m_dwAntiWaitTime = iAntiWaitTime;
	m_fAntiErrorRate = fAntiErrorRate;

	m_dwPenguinCount = iPenguinCount;
	m_dwKickCount = iKickCount;
	m_iTimeDecrease = iTimeDecrease;

	m_dwSkillPenguinCount = iSkillHackCount;
	m_dwSkillKickCount = iSkillKickCount;
	m_iSkillTimeDecrease = iSkillTimeDecrease;

	int nSize = m_vecExceptID.size();

	for( int i = 0; i < 10; ++i )
	{
		if( iArray[i] == 0 )
			break;

		int iExceptid = iArray[i];

		for( int j = 0; j < nSize; ++j )
		{
			if( m_vecExceptID[j] == iExceptid )
				break;
		}

		m_vecExceptID.push_back( iExceptid );
	}
}

bool ioRelayGroupInfoMgr::IsExceptID( int iSkillid )
{
	int nSize = m_vecExceptID.size();
	for( int i = 0; i < nSize; ++i )
		if( m_vecExceptID[i] == iSkillid )
			return true;
	return false;
}

void ioRelayGroupInfoMgr::UpdateUserSPPotion( DWORD dwRoomIndex, DWORD dwUserIndex )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		DWORD dwCurTime = TIMEGETTIME();
		UserData* pUser = pRelayGroup->GetUserData( dwUserIndex );
		if( pUser )
			pUser->OnRecvoerSkill( dwCurTime );
	}
}

void ioRelayGroupInfoMgr::OnAnthHackBulletInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, DWORD dwItemCode, DWORD dwWasteCount, DWORD dwWasteTime )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		UserData* pUser = pRelayGroup->GetUserData( dwUserIndex );
		if( pUser )
		{
			UserData::sBulletData* pBulletData = pUser->GetBulletData( dwItemCode );
			if( !pBulletData )
			{	
				sWeaponInfo* pWeapon = GetWeaponInfo( dwItemCode );
				if( !pWeapon )
					return;
				pBulletData = pUser->InsertBulletData( dwItemCode, pWeapon->dwMaxCount );
				if( !pBulletData )
					return;
			}
			pBulletData->WasteBullet( dwWasteCount, dwWasteTime, dwHostTime );
			CheckBulletData( pUser, dwItemCode );
		}
	}
}

void ioRelayGroupInfoMgr::OnAnthHackReloadBullet( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, DWORD dwItemCode )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		UserData* pUser = pRelayGroup->GetUserData( dwUserIndex );
		if( pUser )
		{
			UserData::sBulletData* pBulletData = pUser->GetBulletData( dwItemCode );
			{
				if( pBulletData )
				{
					sWeaponInfo* pWeapon = GetWeaponInfo( dwItemCode );
					if( pWeapon )
					{
						//reload �ð�üũ
						DWORD dwReloadInterval = dwHostTime-pBulletData->dwReloadTime;
						//20���� ���� ����
						if( dwReloadInterval * 1.2 < pWeapon->dwReloadTime )
						{
							CheatLOG.PrintTimeAndLog( 0, "[hack][bullet] OnAnthHackReloadBullet -  bullet reload time check over time(%u/%u) interval(%d)", pBulletData->dwReloadTime, dwHostTime, dwReloadInterval );
							AddPenaltyPoint( pUser, 5 );
							pBulletData->Clear();
						}
					}
					pBulletData->Clear();
					pBulletData->dwReloadTime = dwHostTime;
				}
				else
				{
					//CheatLOG.PrintTimeAndLog( 0, "[hack][bullet] OnAnthHackReloadBullet  time(%u) interval pBulletData is NULL", dwHostTime );
				}
				
			}
		}
	}
}

void ioRelayGroupInfoMgr::CheckBulletData( UserData* pUser, DWORD dwItemCode )
{
	if( !pUser )
		return;

	UserData::sBulletData* pData = pUser->GetBulletData( dwItemCode );
	if( !pData )
		return;

	int iSize = pData->vecRecvTime.size();
	if( iSize < 3 )
		return;

	sWeaponInfo* pWeapon = GetWeaponInfo( pData->dwItemCode );
	if( !pWeapon ||  pWeapon->dwMaxCount == 0 )
		return;

	//2���� üũ
	//1. ���� ( �ϴ� 20�������� �� �����ϰ� )
	if( pData->dwCurCount > pData->dwMaxCount + (pWeapon->dwMaxCount*0.2)  )
	{
		pData->dwCurCount = 0;
		AddPenaltyPoint( pUser, 5 );
		CheatLOG.PrintTimeAndLog( 0, "[hack][bullet] CheckBulletData - BulletCount check over count(%u/%u)", pData->dwCurCount, pData->dwMaxCount );
		return;
	}

	//2.�߻� �ӵ� �˻�
	int iCheatCnt = 0;
	for( int i = 0; i < iSize; ++i )
	{
		DWORD dwWasteCount = pData->vecWasteBullet[i];
		if( dwWasteCount == 1 )
			continue;
		DWORD dwWasteTime = pData->vecWasteTime[i];
		DWORD dwWasteInterval = dwWasteTime / dwWasteCount;
		if( dwWasteInterval < pWeapon->dwIntervalBullet )
		{
			iCheatCnt++;
			CheatLOG.PrintTimeAndLog( 0, "[hack][bullet] CheckBulletData - BulletWasteTime check over time(%u/%u)", dwWasteInterval, pWeapon->dwIntervalBullet );
		}
	}
	if( iCheatCnt > 0 )
	{	
		int iDefaultValue = 10;
		int iPenalty = (iDefaultValue * iCheatCnt) / iSize;
		AddPenaltyPoint( pUser, iPenalty );
	}

	
	//3.�߻� ���� �˻�
	iCheatCnt = 0;
	for( int i = 1; i < iSize; ++i )
	{
		DWORD dwPrevTime = pData->vecRecvTime[i-1];
		DWORD dwNextTime = pData->vecRecvTime[i];
		DWORD dwInterVal = dwNextTime - dwPrevTime;
		if( dwInterVal < pWeapon->dwIntervalMotion )
		{
			iCheatCnt++;
			CheatLOG.PrintTimeAndLog( 0, "[hack][bullet] CheckBulletData - BulletRecvTime check over time(%u/%u)", dwInterVal, pWeapon->dwIntervalMotion );
		}
	}
	if( iCheatCnt > 0 )
	{	
		int iDefaultValue = 10;
		int iPenalty = (iDefaultValue * iCheatCnt) / iSize;
		AddPenaltyPoint( pUser, iPenalty );
	}
}

sWeaponInfo* ioRelayGroupInfoMgr::GetWeaponInfo( DWORD dwItemCode )
{
	static sWeaponInfo sWeapon;

	auto it = m_mapWeaponInfo.find( dwItemCode );
	if( it == m_mapWeaponInfo.end() )
	{
		//�̱��濡�� ������
		sWeaponInfo* pWeapon = g_SkillInfoMgr->GetWeaponInfo( dwItemCode );
		m_mapWeaponInfo.insert( std::map<DWORD,sWeaponInfo*>::value_type( dwItemCode, pWeapon ) );
	}
	
	it = m_mapWeaponInfo.find( dwItemCode );
	if( it == m_mapWeaponInfo.end() )
		return &sWeapon;

	return it->second;
}

void ioRelayGroupInfoMgr::UpdateDieState( DWORD dwRoomIndex, DWORD dwUserIndex, BOOL bState )
{
	RelayGroup* pRelayGroup = GetRelayGroupByRoom( dwRoomIndex );
	if(pRelayGroup)
	{
		UserData* pUser = pRelayGroup->GetUserData(dwUserIndex);
		if( pUser )
		{
			pUser->ClearBulletData();
		}
	}
}

#endif

//buff ����Ÿ�԰��
/*
void ioRelayGroupInfoMgr::OnSkillBuff( UserData* pUser, SkillInfo* pSkillInfo, float fIncMaxCoolTime, float fUseTimeDiff, float fErrorRate )
{	
if( fIncMaxCoolTime > fUseTimeDiff )
{
// ��ų ��� ���� �������� �Ҹ�Ǵ� ���µ�..
// ������ڸ��� �ٷ� ��ҵǾ��ٰ� �����ϰ�..

float fRemainGauge = pSkillInfo->fMaxGauge - pSkillInfo->fNeedGauge;
float fLowestUsingTime = 0.2f;
int nCnt = fLowestUsingTime / pSkillInfo->iTickTime;
//����ƮŬ���� ������..
if( iModeType == MT_FIGHT_CLUB )
nCnt = fLowestUsingTime / pSkillInfo->iFCTickTime;

float fLowestTime = pSkillInfo->fGaugePerTick * nCnt;					
float fRemainIncTime = fIncMaxCoolTime - (fRemainGauge / pSkillInfo->fMaxGauge) * fIncMaxCoolTime;
float fChecktime = fRemainIncTime - fLowestTime;
if( fChecktime > fUseTimeDiff )
{
//���⼭���� �ϴ� �ǽ�.
fChecktime = fChecktime * (1.f - fErrorRate);
float fDiffTime = fRemainIncTime - fUseTimeDiff;
float fAllowTime = fIncMaxCoolTime * fErrorRate;
if( fChecktime > fDiffTime )
{
// �ϴ� ����ϴ°ɷ�
CheatLOG.PrintTimeAndLog( 0, "[skill][warn] TEST - WARN %s, fChecktime(%f), fDiffTime(%f)", pSkillInfo->hsSkillName.c_str(), fChecktime, fDiffTime );
}
else
{
AddPenaltySkill( pUser, pSkillInfo->hsSkillName, fUseTimeDiff, fChecktime );
// ġ�� ���� �� �� ������ ���!
CheatLOG.PrintTimeAndLog( 0, "[skill][hack] Check - Uidx(%u), SkillName(%s), fChecktime(%f), fDiffTime(%f)"
, pUser->m_dwUserIndex, pSkillInfo->hsSkillName.c_str(), fChecktime, fDiffTime );

return;
}
}
}

return true;
}
*/