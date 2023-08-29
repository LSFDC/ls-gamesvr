#include "stdafx.h"
#include "ioGuardianMgr.h"

#include "../MainProcess.h"
#include "../Network/ioPacketQueue.h"
#include "../Network/GameServer.h"
#include "../EtcHelpFunc.h"
#include "../NodeInfo/ServerNode.h" 
#include "../NodeInfo/Room.h"
#include "../NodeInfo/RoomNodeManager.h"
#include "../NodeInfo/ioMedalItemInfoManager.h"
#include "../NodeInfo/ServerNodeManager.h"
#include "../NodeInfo/ioRelayRoomInfoMgr.h"
#include "../NodeInfo/ioRelayGroupInfoMgr.h"
#include "../EtcHelpFunc.h"

extern CLog CriticalLOG;

ioGuardianMgr::ioGuardianMgr( void )
{
#ifdef ANTIHACK
	InitRUDPList();
#endif
}

ioGuardianMgr::~ioGuardianMgr( void )
{

}
#ifdef ANTIHACK
bool ioGuardianMgr::PacketParsing( SP2Packet& rkPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	rkPacket.SetPosBegin();

	if( IsRUDPPacket(rkPacket) )
	{
		DWORD dwUserIndex = GetUserIndexByAllSendPacket( rkPacket );
		DWORD dwRoomIndex = g_RelayRoomInfoMgr->GetRoomIndexByUser(dwUserIndex);

		// 유저 종료 루틴에서 udp relaygroup 먼저 제거 하고 후에 tcp를 보내기 때문에..
		if( dwRoomIndex == 0 )
			return false;

		//////////////////////////////////////////////////////////////////////////
/*
		// 일단 적용대상을 전체 유저의 5%만
#ifdef SRC_ID
 		if( dwUserIndex%100 <= 5 )
 			return true;
#endif*/

		int iModeStyle = pRelayGroupInfoMgr->GetModeStyle( dwRoomIndex );
		if( iModeStyle == RSTYLE_PLAZA ||
			iModeStyle == RSTYLE_HEADQUARTERS )
			return true;

		int iModeType = pRelayGroupInfoMgr->GetModeType( dwRoomIndex );
		if( iModeType == MT_HEADQUARTERS || 
			iModeType == MT_TRAINING ||
			iModeType == MT_FOOTBALL ||
			iModeType == MT_DUNGEON_A ||
			iModeType == MT_FACTORY ||
			iModeType == MT_FIRE_TEMPLE ||
			iModeType == MT_TOWER_DEFENSE ||
			iModeType == MT_DARK_XMAS ||
			iModeType == MT_BOSS )
			return true;

		if( rkPacket.GetPacketID() == CUPK_RUDP_NOT_USE_ID )
		{
			pRelayGroupInfoMgr->NotUseControlSeed( dwRoomIndex, dwUserIndex, rkPacket );
			return false;
		}

		DWORD dwSeed, dwHostTime, dwNPCIndex;
		GetRUDPInfo( rkPacket, dwSeed, dwHostTime, dwNPCIndex );

		bool bUser = dwNPCIndex == 0 ? true : false;
		if( !pRelayGroupInfoMgr->CheckControlSeed( dwRoomIndex, dwUserIndex, dwSeed, dwHostTime, bUser ) )
		{
			return false;
		}

		DWORD dwAttackerUserIndex = bUser == true ? dwUserIndex : dwNPCIndex;
	
		switch( rkPacket.GetPacketID() )
		{
		case CUPK_USE_SKILL:
			if( !bUser )
				return true;
			OnSkillUse(dwRoomIndex,dwUserIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_SKILL_EXTRA_INFO:
			if( !bUser )
				return true;
			OnSkillExtraInfo( dwRoomIndex, dwUserIndex, dwHostTime, rkPacket, pRelayGroupInfoMgr );
			break;
		case CUPK_ANTIHACK_SKILL:
			if( !bUser )
				return true;
			OnSkillAntihack(dwRoomIndex,dwUserIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_CHAR_HIT:
			OnAntiHackHit(dwRoomIndex,dwUserIndex,dwNPCIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_CHAR_WOUNDED:
			OnAntiHackWounded(dwRoomIndex,dwUserIndex,dwNPCIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_CHAR_DEFENCE:
			OnAntiHackDefence(dwRoomIndex,dwUserIndex,dwNPCIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_BULLET_INFO:
			OnAntiHackBulletInfo(dwRoomIndex,dwUserIndex,dwNPCIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		case CUPK_ANTIHACK_BULLET_RELOAD:
			OnAntiHackReloadBullet(dwRoomIndex,dwUserIndex,dwNPCIndex,dwHostTime,rkPacket,pRelayGroupInfoMgr);
			break;
		}
	}
	return true;
}

DWORD ioGuardianMgr::GetUserIndexByAllSendPacket( SP2Packet &rkPacket )
{
	DWORD dwIP = 0, dwPort = 0, dwUserIndex = 0;
	int iIndex = 0;

	PACKET_GUARD_NULL( rkPacket.Read(dwIP) );
	PACKET_GUARD_NULL( rkPacket.Read(dwPort) );
	PACKET_GUARD_NULL( rkPacket.Read(dwUserIndex) );
	PACKET_GUARD_NULL( rkPacket.Read(iIndex) );

	return dwUserIndex;
}

void ioGuardianMgr::GetRUDPInfo( SP2Packet& rkPacket, DWORD& dwUserSeed, DWORD& dwHostTime, DWORD& dwNPCIndex )
{
	dwUserSeed = dwHostTime = dwNPCIndex = 0;

	PACKET_GUARD_VOID( rkPacket.Read(dwUserSeed) );
	PACKET_GUARD_VOID( rkPacket.Read(dwHostTime) );
	PACKET_GUARD_VOID( rkPacket.Read(dwNPCIndex) );
}

void ioGuardianMgr::InitRUDPList()
{
	//대상 리스트를 여기 추가
	DWORD dwRUDPList[] = 
	{
		CUPK_CHAR_CONTROL,
		CUPK_USE_SKILL,
		CUPK_ANTIHACK_CHAR_HIT,
		CUPK_ANTIHACK_CHAR_WOUNDED,
		CUPK_ANTIHACK_CHAR_DEFENCE,
		CUPK_ANTIHACK_SKILL,
		CUPK_ANTIHACK_SKILL_EXTRA_INFO,
		CUPK_ANTIHACK_BULLET_INFO,
		CUPK_ANTIHACK_BULLET_RELOAD,
		CUPK_RUDP_NOT_USE_ID	// 이 패킷은 실제 시드값은 없음. 시드값 제거용으로 쓰임
	};

	int rudpSize = sizeof(dwRUDPList);
	int rudpCount = rudpSize/4;
	for( int i = 0; i < rudpCount; ++ i )
		m_setRUDPList.insert( dwRUDPList[i] );
}

bool ioGuardianMgr::IsRUDPPacket( SP2Packet& kPacket )
{
	bool bRet = m_setRUDPList.find( kPacket.GetPacketID() ) == m_setRUDPList.end() ? false : true;
	return bRet;
}

void ioGuardianMgr::OnSkillUse( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	if( !pRelayGroupInfoMgr )
		return;

	ioHashString szKillName;
	int iLevel;
	HALF half;

	const DWORD Pos = 12;
	kPacket.MovePointer( Pos );
	PACKET_GUARD_VOID( kPacket.Read(szKillName) );
	PACKET_GUARD_VOID( kPacket.Read(iLevel) );
	PACKET_GUARD_VOID( kPacket.Read(half) );

	// 여기서 가디언 서버로 전송이지만! 현재 없으니, 검증작업할 스레드를 relayGroupInfo 쪽으로..	
	pRelayGroupInfoMgr->OnSkillUse( dwRoomIndex, dwUserIndex, dwHostTime, szKillName, iLevel, HALFToFloat(half) );
}

void ioGuardianMgr::OnSkillExtraInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	if( !pRelayGroupInfoMgr )
		return;

	int iSize;
	DWORD dwUserIndexs[16] = { 0, };
	PACKET_GUARD_VOID( kPacket.Read(iSize) );
	MAX_GUARD(iSize, 16);
	for( int i = 0; i < iSize; ++i )
	{
		PACKET_GUARD_VOID( kPacket.Read(dwUserIndexs[i]) );
	}

	// 여기서 가디언 서버로 전송이지만! 현재 없으니, 검증작업할 스레드를 relayGroupInfo 쪽으로..	
	pRelayGroupInfoMgr->OnSkillExtraInfo( dwRoomIndex, dwUserIndex, dwHostTime, iSize, dwUserIndexs );
}

void ioGuardianMgr::OnSkillAntihack( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	if( !pRelayGroupInfoMgr )
		return;

	//스킬 회복에 관련된 정보
	int nSize;	
	PACKET_GUARD_VOID( kPacket.Read(nSize) );
	MAX_GUARD(nSize, 4);
	
	int iSlot[4];
	DWORD dwLatedTime[4];
	for( int i = 0; i < nSize; ++i )
	{
		PACKET_GUARD_VOID( kPacket.Read(iSlot[i]) );
		PACKET_GUARD_VOID( kPacket.Read(dwLatedTime[i]) );
	}

	pRelayGroupInfoMgr->OnSkillAntihack( dwRoomIndex, dwUserIndex, dwHostTime, nSize, iSlot, dwLatedTime );
}
void ioGuardianMgr::OnAntiHackHit( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	DWORD dwHitUserIndex;
	DWORD dwWeaponIndex;
	BYTE eAttackType;

	PACKET_GUARD_VOID( kPacket.Read(dwHitUserIndex) );
	PACKET_GUARD_VOID( kPacket.Read(dwWeaponIndex) );
	PACKET_GUARD_VOID( kPacket.Read(eAttackType) );

	pRelayGroupInfoMgr->OnAntiHackHit( dwRoomIndex, dwUserIndex, dwNPCIndex, dwHostTime, dwHitUserIndex, dwWeaponIndex, eAttackType );
}

void ioGuardianMgr::OnAntiHackWounded( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	//npc는 무시
	if( dwNPCIndex )
		return;

	DWORD dwAttackerUserIndex;
	DWORD dwWeaponIndex;
	BYTE eAttackType;

	PACKET_GUARD_VOID( kPacket.Read(dwAttackerUserIndex) );
	PACKET_GUARD_VOID( kPacket.Read(dwWeaponIndex) );
	PACKET_GUARD_VOID( kPacket.Read(eAttackType) );
	pRelayGroupInfoMgr->OnAntiHackWounded( dwRoomIndex, dwUserIndex, dwNPCIndex, dwHostTime, dwAttackerUserIndex, dwWeaponIndex, eAttackType );
}

void ioGuardianMgr::OnAntiHackDefence( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	//npc는 무시
	if( dwNPCIndex )
		return;

	DWORD dwAttackerUserIndex;
	DWORD dwWeaponIndex;
	BYTE eAttackType;

	PACKET_GUARD_VOID( kPacket.Read(dwAttackerUserIndex) );
	PACKET_GUARD_VOID( kPacket.Read(dwWeaponIndex) );
	PACKET_GUARD_VOID( kPacket.Read(eAttackType) );
	pRelayGroupInfoMgr->OnAntiHackDefence( dwRoomIndex, dwUserIndex, dwNPCIndex, dwHostTime, dwAttackerUserIndex, dwWeaponIndex, eAttackType );
}

void ioGuardianMgr::OnAntiHackBulletInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	if( dwNPCIndex )
		return;

	DWORD dwItemCode;
	DWORD dwWasteCount;
	DWORD dwWasteTime;

	PACKET_GUARD_VOID( kPacket.Read(dwItemCode) );
	PACKET_GUARD_VOID( kPacket.Read(dwWasteCount) );
	PACKET_GUARD_VOID( kPacket.Read(dwWasteTime) );
	pRelayGroupInfoMgr->OnAnthHackBulletInfo( dwRoomIndex, dwUserIndex, dwNPCIndex, dwHostTime, dwItemCode, dwWasteCount, dwWasteTime );
}

void ioGuardianMgr::OnAntiHackReloadBullet( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr )
{
	if( dwNPCIndex )
		return;

	DWORD dwItemCode;

	PACKET_GUARD_VOID( kPacket.Read(dwItemCode) );
	pRelayGroupInfoMgr->OnAnthHackReloadBullet( dwRoomIndex, dwUserIndex, dwNPCIndex, dwHostTime, dwItemCode );
}

#endif