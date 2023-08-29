#pragma once
#include "../../iocpSocketDLL/SocketModules/BufferPool.h"
#include <atlcoll.h>

class MPSCQueue;
class ServerNode;
class Room;
class SP2Packet;
class ioRelayGroupInfoMgr;

class ioGuardianMgr
{	
public:
	ioGuardianMgr(void);
	virtual ~ioGuardianMgr(void);

#ifdef ANTIHACK

	bool PacketParsing( SP2Packet& rkPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	DWORD GetUserIndexByAllSendPacket( SP2Packet &rkPacket );
	void GetRUDPInfo( SP2Packet& rkPacket, DWORD& dwUserSeed, DWORD& dwHostTime, DWORD& dwNPCIndex );

	std::set<DWORD> m_setRUDPList;
	void InitRUDPList();
	bool IsRUDPPacket( SP2Packet& kPacket );

	void OnSkillUse( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	// 스킬마다 다름!! 주사위 스킬만 처리함!!
	void OnSkillExtraInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	void OnSkillAntihack( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	void OnAntiHackHit( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	void OnAntiHackWounded( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	void OnAntiHackDefence( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );

	void OnAntiHackBulletInfo( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
	void OnAntiHackReloadBullet( DWORD dwRoomIndex, DWORD dwUserIndex, DWORD dwNPCIndex, DWORD dwHostTime, SP2Packet& kPacket, ioRelayGroupInfoMgr* pRelayGroupInfoMgr );
#endif
};

#define g_Guardian		(*cSingleton<ioGuardianMgr>::GetInstance())