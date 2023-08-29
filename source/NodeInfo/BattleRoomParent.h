#ifndef _BattleRoomParent_h_
#define _BattleRoomParent_h_

#include "BlockNode.h"

// ������ ��� ���� Ÿ��
enum
{
	BMT_ALL_MODE                = 0,  //��ȣ������
	BMT_RANDOM					= 1,  //���� ���
	BMT_CATCH					= 2,  //����Ż��
	BMT_CATCH_BOSS				= 3,  //����Ż�� - ����
	BMT_STONE					= 4,  //��¡��
	BMT_KING					= 5,  //���� ũ���
	BMT_SURVIVAL				= 6,  //������ġ
	BMT_TEAM_SURVIVAL			= 7,  //�� ������ġ
	BMT_TEAM_SURVIVAL_FIRST		= 8,  //�� ������ġ - �ʺ�
	BMT_STONE_BOSS				= 9,  //��¡�� - ����
	BMT_KING_BOSS				= 10, //���� ũ��� - ����
	BMT_TEAM_SURVIVAL_BOSS		= 11, //�� ������ġ - ����
	BMT_TEAM_SURVIVAL_FIRST_BOSS= 12, //�� ������ġ - �ʺ� - ����
	BMT_RANDOM_BOSS             = 13, //���� ��� - ����
	BMT_BOSS                    = 14, //�������
	BMT_MONSTER_SURVIVAL_EASY   = 15, //PvE��� - ����
	BMT_MONSTER_SURVIVAL_NORMAL = 16, //PvE��� - ����
	BMT_MONSTER_SURVIVAL_HARD   = 17, //PvE��� - �����
	BMT_USER_CUSTOM             = 18, //������� - ��������ġ & ����Ż��
	BMT_FOOTBALL				= 19, //�౸���
	BMT_FOOTBALL_BOSS			= 20, //�౸��� - ����
	BMT_GANGSI                  = 21, //���ø��
	BMT_DUNGEON_A_EASY			= 22, //������� - ���
	BMT_CATCH_RUNNINGMAN		= 23, //����Ż�� ���׸�
	BMT_CATCH_RUNNINGMAN_BOSS	= 24, //����Ż�� ���׸� - ����
	BMT_FIGHT_CLUB				= 25, //����ƮŬ��
	BMT_TOWER_DEFENSE_EASY		= 26, //tower defense
	BMT_TOWER_DEFENSE_NORMAL	= 27, 
	BMT_TOWER_DEFENSE_HARD		= 28,  //UNDONE : tower easy normal hard
	BMT_TOWER_DEFENSE_CHALLENGE	= 29,
	BMT_DARK_XMAS_EASY			= 30,
	BMT_DARK_XMAS_NORMAL		= 31,
	BMT_DARK_XMAS_HARD			= 32,
	BMT_DARK_XMAS_CHALLENGE		= 33,
	BMT_FIRE_TEMPLE_EASY		= 34,
	BMT_FIRE_TEMPLE_NORMAL		= 35,
	BMT_FIRE_TEMPLE_HARD		= 36,
	BMT_FIRE_TEMPLE_CHALLENGE	= 37,
	BMT_DOBULE_CROWN			= 38, //���� ũ���(����ũ��� ������)
	BMT_DOBULE_CROWN_BOSS		= 39, //���� ũ���(����ũ��� ������) ����	
	BMT_FACTORY_EASY			= 40, //��а���
	BMT_FACTORY_NORMAL			= 41, 
	BMT_FACTORY_HARD			= 42,
	BMT_FACTORY_CHALLENGE		= 43,
	BMT_TEAM_SURVIVAL_AI_EASY	= 44, // ������ AI ���
	BMT_TEAM_SURVIVAL_AI_HARD	= 45, // ������ AI ���
	BMT_UNDERWEAR				= 46,
	BMT_CBT						= 47,
	BMT_RAID					= 48, // ���̵���
};

// ������ �̺�Ʈ Ÿ��
enum
{
	BET_NORMAL = 0,             // �Ϲ� ������
	BET_BROADCAST_AFRICA = 1,   // AFRICA ��� ��
	BET_BROADCAST_MBC = 2,      // MBC ��� ��
	BET_TOURNAMENT_BATTLE = 3,  // ��ȸ ��� ������
};

class UserParent;
class SP2Packet;
class BattleRoomParent : public BlockNode
{
	public:    
	enum
	{
		ENTER_USER = 100,
		LEAVE_USER,
		ROOM_INFO,
		USER_INFO,
		KICK_OUT,
		UDP_CHANGE,
		TRANSFER_PACKET,
		TRANSFER_PACKET_USER,
		USER_PACKET_TRANSFER,
		P2P_RELAY_INFO,
		TRANSFER_PACKET_UDP,
		TOURNAMENT_INFO,
	};

	//�����Լ��� �⿬
	public:
	virtual const DWORD GetIndex() = 0;
	virtual void EnterUser( const DWORD dwUserIndex, const ioHashString &szPublicID, const int iGradeLevel, const int iAbilityLevel, const bool bSafetyLevel, const bool bObserver,
							const ioHashString &szPublicIP, const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iClientPort, const int iTransferPort ) = 0;
	virtual bool LeaveUser( const DWORD dwUserIndex, const ioHashString &szPublicID ) = 0;
	virtual void SendPacketTcp( SP2Packet &rkPacket, const DWORD dwUserIndex = 0 ) = 0;
	virtual void SendPacketTcpUser( SP2Packet &rkPacket, const ioHashString &rkSenderName ) = 0;
	virtual void SendPacketUdp( SP2Packet &rkPacket, const DWORD dwUserIndex = 0 ) = 0;
	virtual void UserInfoUpdate( const DWORD dwUserIndex, const int iGradeLevel, const int iAbilityLevel, const bool bSafetyLevel, const int iClientPort, const ioHashString &szTransferIP, const int iTransferPort ) = 0;
	virtual void UserUDPChange( const DWORD dwUserIndex, const ioHashString &szPublicID, const ioHashString &szPublicIP, const int iClientPort,
								const ioHashString &szPrivateIP, const ioHashString &szTransferIP, const int iTransferPort ) = 0;
	virtual void UserP2PRelayInfo( const DWORD dwUserIndex, const DWORD dwRelayUserIndex, bool bRelay ) = 0;
	virtual void TournamentInfo( const DWORD dwUserIndex, const DWORD dwTeamIndex ) = 0;
	virtual const bool IsOriginal() = 0;
	
    virtual int	 GetJoinUserCnt() const = 0;
	virtual int  GetPlayUserCnt() = 0;
	virtual int  GetMaxPlayer() const = 0;
	virtual int  GetMaxPlayerBlue() const = 0;
	virtual int  GetMaxPlayerRed() const = 0;
	virtual int  GetMaxObserver() const = 0;
	virtual ioHashString GetName() const = 0;
	virtual	ioHashString GetPW() const = 0;
	virtual ioHashString GetOwnerName() const = 0;
	virtual bool IsBattleTimeClose() = 0;
	virtual bool IsBattleModePlaying() = 0;
	virtual bool IsFull() = 0;
	virtual bool IsMapLimitPlayerFull() = 0;
	virtual bool IsMapLimitGrade( int iGradeLevel ) = 0;
	virtual bool IsObserverFull() = 0;
	virtual bool IsRandomTeamMode() = 0;
	virtual bool IsStartRoomEnterX() = 0;
	virtual bool IsEmptyBattleRoom() = 0;
	virtual int  GetPlayModeType() = 0;
	virtual int  GetSelectModeTerm() = 0;
	virtual bool IsPassword() = 0;
	virtual int  GetAbilityMatchLevel() = 0;
	virtual int  GetRoomLevel() = 0;
	virtual int  GetSortLevelPoint( int iMyLevel ) = 0;
	virtual int  GetBattleEventType() = 0;
	virtual bool IsLevelMatchIgnore() = 0;
	virtual DWORD GetTournamentIndex() = 0;

	virtual bool IsNoChallenger() = 0;
	virtual bool IsHidden(int iIndex) = 0;

	// Custom Option
	virtual bool IsUseExtraOption() = 0;

	virtual int GetTeamAttackType() = 0;
	virtual int GetChangeCharType() = 0;
	virtual int GetCoolTimeType() = 0;
	virtual int GetRedHPType() = 0;
	virtual int GetBlueHPType() = 0;
	virtual int GetDropDamageType() = 0;
	virtual int GetGravityType() = 0;
	virtual int GetGetUpType() = 0;
	virtual int GetKOType() = 0;
	virtual int GetKOEffectType() = 0;
	virtual int GetRedBlowType() = 0;
	virtual int GetBlueBlowType() = 0;
	virtual int GetRedMoveSpeedType() = 0;
	virtual int GetBlueMoveSpeedType() = 0;
	virtual int GetRedEquipType() = 0;
	virtual int GetBlueEquipType() = 0;

	virtual int GetPreSetModeType() = 0;

	virtual int GetCatchModeRoundType() = 0;
	virtual int GetCatchModeRoundTimeType() = 0;

	virtual int GetGrowthUseType() = 0;
	virtual int GetExtraItemUseType() = 0;

	public:
	virtual void SyncPlayEnd( bool bAutoStart ) = 0;

	public:        //��Ŷ ó��
	virtual void OnBattleRoomInfo( UserParent *pUser, int iPrevBattleIndex ) = 0;
	virtual bool OnProcessPacket( SP2Packet &rkPacket, UserParent *pUser ) = 0;	
	
	public:
	BattleRoomParent();
	virtual ~BattleRoomParent();
};
typedef std::vector< BattleRoomParent * > vBattleRoomParent;
typedef vBattleRoomParent::iterator vBattleRoomParent_iter;
#endif