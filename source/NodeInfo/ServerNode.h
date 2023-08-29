#ifndef _ServerNode_h_
#define _ServerNode_h_

#include "UserCopyNode.h"
#include "RoomCopyNode.h"
#include "ShuffleRoomCopyNode.h"
#include "BattleRoomCopyNode.h"
#include "ChannelCopyNode.h"
#include "LadderTeamCopyNode.h"
#include <unordered_set>

#define FRIEND_LIST_MOVE_COUNT             50
#define SERVER_PING_CHECK_TIME             10000
#define SERVER_ROOM_CREATE_CLOSE_TIME      15000

class User;
class CConnectNode;
class SP2Packet;

class ServerNode : public CConnectNode
{
	friend class ServerNodeManager;

public:
	enum NodeTypes
	{
		NODE_TYPE_NONE = 0,
		NODE_TYPE_GAME,
		NODE_TYPE_LOGIN,
		NODE_TYPE_RELAY,
		NODE_TYPE_END
	};
	enum NodeRoles
	{
		NODE_ROLE_NONE = 0,
		NODE_ROLE_SERVER = 1, //���� �����̴�
		NODE_ROLE_CLIENT = 2, //���� Ŭ���̾�Ʈ�̴�.
		NODE_ROLE_END
	};

	enum SessionState
	{
		SS_DISCONNECT	= 0,
		SS_CONNECT		= 1,
	};

protected:
	NodeTypes	 m_eNodeType;
	NodeRoles	 m_eNodeRole;
	SessionState m_eSessionState;

	bool		 m_bBlockState;
	DWORD        m_dwServerIndex;
	ioHashString m_szServerIP;     // �缳IP
	ioHashString m_szClientMoveIP; // ����IP
	TCHAR        m_szPublicIP[STR_IP_MAX]; //���ۿ� 
	int          m_iServerPort;
	int          m_iClientPort;
	int          m_iUserCount;
	int			 m_iRelayIndex;
	int			 m_iPartitionIndex;
 
	DWORD        m_dwPingTIme;
	DWORD        m_dwRecvPingTime;
	std::vector<int> m_relayPorts;
	SendRelayInfo_ m_RelayInfo;

	typedef std::vector< SP2Packet > SP2PacketList;
	SP2PacketList m_ConnectWorkingPacket;
	bool          m_bConnectWorkComplete;
 
protected:			  // User Copy Node
	uUserCopyNode m_UserCopyNode;
	MemPooler< UserCopyNode >	m_UserMemNode;

protected:
	void InitUserMemoryPool();
	void ReleaseUserMemoryPool();
	void ReturnUserMemoryPool();
	UserCopyNode *CreateNewUser( DWORD dwIndex );
	UserCopyNode *GetUserNode( DWORD dwIndex );
	//UserCopyNode *GetUserNode( const ioHashString &szUserID );
	void RemoveUserNode( DWORD dwIndex );
	void RemoveUserNode( const ioHashString &szUserID );
	//int  ExceptionRemoveUserNode( DWORD dwIndex );

protected:            // Room Copy Node
	vRoomCopyNode m_RoomCopyNode;
	MemPooler< RoomCopyNode >	m_RoomMemNode;
	
	void InitRoomMemoryPool();
	void ReleaseRoomMemoryPool();
	void ReturnRoomMemoryPool();
	RoomCopyNode *CreateNewRoom( int iIndex );
	RoomCopyNode *GetRoomNode( int iIndex );
	void RemoveRoomNode( int iIndex );
	void RemoveRoomNode( RoomCopyNode *pRoomNode );
	void RemoveRoomCopyNodeAll();
	
protected:           // BattleRoom Copy Node
	vBattleRoomCopyNode m_BattleRoomCopyNode;
	MemPooler< BattleRoomCopyNode >	m_BattleRoomMemNode;

	void InitBattleRoomMemoryPool();
	void ReleaseBattleRoomMemoryPool();
	void ReturnBattleRoomMemoryPool();
	BattleRoomCopyNode *CreateNewBattleRoom( DWORD dwIndex );
	BattleRoomCopyNode *GetBattleRoomNode( DWORD dwIndex );
	void RemoveBattleRoomNode( DWORD dwIndex );
	void RemoveBattleRoomNode( BattleRoomCopyNode *pBattleRoomNode );
	void RemoveBattleRoomCopyNodeAll();

protected:           // ShuffleRoom Copy Node
	vShuffleRoomCopyNode m_ShuffleRoomCopyNode;
	MemPooler< ShuffleRoomCopyNode >	m_ShuffleRoomMemNode;

	void InitShuffleRoomMemoryPool();
	void ReleaseShuffleRoomMemoryPool();
	void ReturnShuffleRoomMemoryPool();
	ShuffleRoomCopyNode *CreateNewShuffleRoom( DWORD dwIndex );
	ShuffleRoomCopyNode *GetShuffleRoomNode( DWORD dwIndex );
	void RemoveShuffleRoomNode( DWORD dwIndex );
	void RemoveShuffleRoomNode( ShuffleRoomCopyNode *pShuffleRoomNode );
	void RemoveShuffleRoomCopyNodeAll();

protected:			// Channel Copy Node
	vChannelCopyNode m_ChannelCopyNode;
	MemPooler< ChannelCopyNode >	m_ChannelMemNode;

	void InitChannelMemoryPool();
	void ReleaseChannelMemoryPool();
	void ReturnChannelMemoryPool();
	ChannelCopyNode *CreateNewChannel( DWORD dwIndex );
	ChannelCopyNode *GetChannelNode( DWORD dwIndex );
	void RemoveChannelNode( DWORD dwIndex );
	void RemoveChannelNode( ChannelCopyNode *pChannelNode );

protected:         // LadderTeam Copy Node
	vLadderTeamCopyNode m_LadderTeamCopyNode;
	MemPooler< LadderTeamCopyNode >	m_LadderTeamMemNode;

	void InitLadderTeamMemoryPool();
	void ReleaseLadderTeamMemoryPool();
	void ReturnLadderTeamMemoryPool();
	LadderTeamCopyNode *CreateNewLadderTeam( DWORD dwIndex );
	LadderTeamCopyNode *GetLadderTeamNode( DWORD dwIndex );
	void RemoveLadderTeamNode( DWORD dwIndex );
	void RemoveLadderTeamNode( LadderTeamCopyNode *pLadderTeamNode );

public:
	virtual void SessionClose( BOOL safely=TRUE );
	virtual bool SendMessage( CPacket &rkPacket );
	virtual void ReceivePacket( CPacket &packet );
	virtual void PacketParsing( CPacket &packet );

protected:
	void OnRelayControl( SP2Packet &kPacket );
	void OnUserGhost( SP2Packet &kPacket );
	void OnHackAnnounce( SP2Packet &kPacket );
	void OnChangeAddress( SP2Packet &kPacket );

	void RequestRelayServerConnect( SP2Packet &kPacket );
	void RequestRelayServerConnect(const char* publicID);

	void OnRelayServerConnect( SP2Packet &kPacket );

public:
	void SetSessionState(ServerNode::SessionState eState)	{ m_eSessionState = eState; }
	void SetBlockState(const bool bBlockState);

	bool IsDisconnectState()		{ return ( m_eSessionState == SS_DISCONNECT ); }
	bool IsConnectState()			{ return ( m_eSessionState == SS_CONNECT ); }
	
	bool IsBusy();
	bool IsConnectWorkComplete()	{ return m_bConnectWorkComplete; }
	bool IsBlocked()				{ return m_bBlockState; }
	bool IsRoomCreateClose();
	bool IsFull();
	bool IsMyPartition(const int iPartitionIndex);

public:
	virtual void OnCreate();       //�ʱ�ȭ
	virtual void OnDestroy();
	void OnSessionDestroy();
	virtual bool CheckNS( CPacket &rkPacket );	
	virtual int  GetConnectType();

	int GetUserCount()			{ return m_iUserCount; }
	void SetUserCount(int val)	{ m_iUserCount = val; }

	int GetNodeType()			{ return m_eNodeType; }
	int GetNodeRole()			{ return m_eNodeRole; }

	bool IsGameNode()			{ return (GetNodeType() == NODE_TYPE_GAME) ? true : false; }
	bool IsLoginNode()			{ return (GetNodeType() == NODE_TYPE_LOGIN) ? true : false; }
	bool IsRelayNode()			{ return (GetNodeType() == NODE_TYPE_RELAY) ? true : false; }

protected:
	void InitData();

	// ���� ���� ��Ŷ�� �����ϴ°ſ� ���ÿ� ������ ������ �̵��ع����� ���� ��Ŷ�� �սǵǹǷ� �̵��� ������ �������Ѵ�.
	// ServerNode���� ����ϴ� ��κ��� Get�� �Ʒ� �Լ��� ���.
	User *GetUserOriginalNode( DWORD dwUserIndex, SP2Packet &rkPacket );
	User *GetUserOriginalNode( const ioHashString &szPublicID, SP2Packet &rkPacket );
	User *GetUserOriginalNodeByPrivateID( const ioHashString &szPrivateID, SP2Packet &rkPacket );
	 
public:
	void SetNodeType( ServerNode::NodeTypes eType );
	void SetNodeRole( ServerNode::NodeRoles eType );
	void SetServerIndex( DWORD dwServerIndex ){ m_dwServerIndex = dwServerIndex; }

public:
	void RequestJoin();
	void RequestSync();
	void AfterJoinProcess();

	void ProcessPing();

public:
	const DWORD &GetServerIndex()			{ return m_dwServerIndex; }
	const ioHashString &GetServerIP()		{ return m_szServerIP; }          // �缳IP
	const ioHashString &GetClientMoveIP()	{ return m_szClientMoveIP; }      // ����IP
	const int	&GetServerPort()			{ return m_iServerPort; }
	const int   &GetClientPort()			{ return m_iClientPort; }
	const int   GetUserNodeSize()			{ return m_UserCopyNode.size(); }

public:
	int GetPlazaRoomCount();
	int GetHeadquartersRoomCount();
	int GetBattleRoomCount();
	int GetLadderTeamCount( bool bHeroMode );
	int GetLadderHeroModeCount();
	int GetShuffleRoomCount();

public:
	int RelayServerIndex() const				{ return m_iRelayIndex; }
	void SetRelayServerIndex(int iRelayIndex)	{ m_iRelayIndex = iRelayIndex; }
	SendRelayInfo_& RelayInfo()					{ return m_RelayInfo; }
	TCHAR* SZPublicIP()							{ return m_szPublicIP; }

	int GetRelayServerPort(int num);

	void SetPartitionIndex(const int iPartitionIndex)	{ m_iPartitionIndex = iPartitionIndex; }
	int GetPartitionIndex()								{ return m_iPartitionIndex; }

public:
	void OnClose( SP2Packet &rkPacket );
	void OnConnectInfo( SP2Packet &rkPacket );
	void OnConnectSync( SP2Packet &rkPacket );
	void OnPing( SP2Packet &rkPacket );
	void OnRoomSync( SP2Packet &rkPacket );
	void OnMovingRoom( SP2Packet &rkPacket );
	void _OnMovingRoomOK( int iMoveType, SP2Packet &rkPacket );
	void _OnMovingRoomError( int iMoveType, SP2Packet &rkPacket );
	void OnMovingRoomResult( SP2Packet &rkPacket );
	void OnUserDataMove( SP2Packet &rkPacket );
	void OnUserInventoryMoveResult( SP2Packet &rkPacket );
	void OnUserExtraItemMoveResult( SP2Packet &rkPacket );
	void OnUserQuestMoveResult( SP2Packet &rkPacket );
	void OnUserDataMoveResult( SP2Packet &rkPacket );
	void OnUserSync( SP2Packet &rkPacket );
	void _OnMoveUserNode( DWORD dwUserIndex, SP2Packet &rkPacket );
	void OnUserMove( SP2Packet &rkPacket );
	void OnUserFriendMove( SP2Packet &rkPacket );
	void OnUserFriendMoveResult( SP2Packet &rkPacket );
	void OnFollowUser( SP2Packet &rkPacket );
	void OnUserPosIndex( SP2Packet &rkPacket );
	void OnChannelSync( SP2Packet &rkPacket );
	void OnChannelInvite( SP2Packet &rkPacket );
	void OnChannelChat( SP2Packet &rkPacket );
	void OnBattleRoomSync( SP2Packet &rkPacket );
	void OnBattleRoomTransfer( SP2Packet &rkPacket );
	void OnBattleRoomJoinResult( SP2Packet &rkPacket );
	void OnBattleRoomFollow( SP2Packet &rkPacket );
	void OnPlazaRoomTransfer( SP2Packet &rkPacket );
	void OnUserInfoRefresh( SP2Packet &rkPacket );
	void OnSimpleUserInfoRefresh( SP2Packet &rkPacket );
	void OnUserCharInfoRefresh( SP2Packet &rkPacket );
	void OnUserCharSubInfoRefresh( SP2Packet &rkPacket );
	void OnBattleRoomKickOut( SP2Packet &rkPacket );
	void OnOfflineMemo( SP2Packet &rkPacket );
	void OnReserveCreateRoom( SP2Packet &rkPacket );
	void OnReserveCreateBattleRoom( SP2Packet &rkPacket );
	void OnReserveCreateBattleRoomResult( SP2Packet &rkPacket );
	void OnExceptionBattleRoomLeave( SP2Packet &rkPacket );
	void OnFriendDelete( SP2Packet &rkPacket );
	void OnWholeChat( SP2Packet &rkPacket );
	void OnRainbowWholeChat( SP2Packet &rkPacket );
	void OnUserCharRentalAgree( SP2Packet &rkPacket );
	void OnUserAlchemicMoveResult( SP2Packet &rkPacket );
	void OnUserPetMoveResult( SP2Packet &rkPacket );
	void OnUserCharAwakeMoveResult( SP2Packet &rkPacket );

	//GUILD
	void OnCreateGuildResult( SP2Packet &rkPacket );
	void OnCreateGuildComplete( SP2Packet &rkPacket );
	void OnGuildEntryAgree( SP2Packet &rkPacket );
	void OnGuildMemberListEx( SP2Packet &rkPacket );
	void OnGuildInvitation( SP2Packet &rkPacket );
	void OnGuildMasterChange( SP2Packet &rkPacket );
	void OnGuildPositionChange( SP2Packet &rkPacket );
	void OnGuildKickOut( SP2Packet &rkPacket );
	void OnGuildMarkChange( SP2Packet &rkPacket );
	void OnGuildUserDelete( SP2Packet &rkPacket );
	void OnLadderTeamSync( SP2Packet &rkPacket );
	void OnLadderTeamTransfer( SP2Packet &rkPacket );
	void OnExceptionLadderTeamLeave( SP2Packet &rkPacket );
	void OnReserveCreateLadderTeam( SP2Packet &rkPacket );
	void OnReserveCreateLadderTeamResult( SP2Packet &rkPacket );
	void OnLadderTeamJoinResult( SP2Packet &rkPacket );
	void OnLadderTeamKickOut( SP2Packet &rkPacket );
	void OnLadderTeamFollow( SP2Packet &rkPacket );
	void OnLadderTeamEnterRoomUser( SP2Packet &rkPacket );
	void OnCampSeasonBonus( SP2Packet &rkPacket );
	void OnGuildNameChange( SP2Packet &rkPacket );
	void OnGuildNameChangeResult( SP2Packet &rkPacket );
	void OnPresentSelect( SP2Packet &rkPacket );
	void OnSubscriptionSelect( SP2Packet &rkPacket );

	// Trade
	void OnTradeCreate( SP2Packet &rkPacket );
	void OnTradeCreateComplete( SP2Packet &rkPacket );
	void OnTradeCreateFail( SP2Packet &rkPacket );
	void OnTradeItemComplete( SP2Packet &rkPacket );
	void OnTradeCancel( SP2Packet &rkPacket );
	void OnTradeTimeOut( SP2Packet &rkPacket );

	void OnTradeGetInfoOK( User *pUser, SP2Packet &rkPacket );
	void OnTradeGetInfoFail( User *pUser, SP2Packet &rkPacket );
	void OnTradeBuyComplete( User *pUser, SP2Packet &rkPacket );
	void OnTradeSellComplete( User *pUser, SP2Packet &rkPacket );

	void OnTradeCancelInfoOK( User *pUser, SP2Packet &rkPacket );
	void OnTradeCancelInfoFail( User *pUser, SP2Packet &rkPacket );
	void OnTradeCancelComplete( User *pUser, SP2Packet &rkPacket );


	//
	void OnUserHeroData( SP2Packet &rkPacket );
	void OnHeroMatchOtherInfo( SP2Packet &rkPacket );
	void OnEventItemInitialize( SP2Packet &rkPacket );

	//
	void OnJoinHeadquartersUser( SP2Packet &rkPacket );
	void OnHeadquartersInfo( SP2Packet &rkPacket );
	void OnHeadquartersRoomInfo( SP2Packet &rkPacket );
	void OnHeadquartersJoinAgree( SP2Packet &rkPacket );
	void OnLogoutRoomAlarm( SP2Packet &rkPacket );
	void OnDisconnectAlreadyID( SP2Packet &rkPacket );

	//
	void OnUDPRecvTimeOut( SP2Packet &rkPacket );
	void OnServerAlarmMent( SP2Packet &rkPacket );

	bool OnConnectWorkingPacket( SP2Packet &rkPacket );
	bool OnUserTransferTCP( SP2Packet &rkPacket );

	// web
	bool OnWebUDPParse( SP2Packet &rkPacket );
	// billing
	bool OnBillingParse( SP2Packet &rkPacket );

	//login
	void OnLoginConnect(SP2Packet &rkPacket);
	void OnLoginRequestStatus();
	void OnLoginBlockRequest(SP2Packet &rkPacket);
	void OnLoginPartitionRequest(SP2Packet &rkPacket);

	//
	void OnTournamentCreateTeam( SP2Packet &rkPacket );
	void OnTournamentTeamAgreeOK( SP2Packet &rkPacket );
	void OnTournamentTeamJoin( SP2Packet &rkPacket );
	void OnTournamentTeamLeave( SP2Packet &rkPacket );
	void OnCloverSend( SP2Packet& rkPacket );

	// ���� ����
	void OnPresentInsert( SP2Packet &rkPacket );
	void OnPresentInsertByEtcItem( SP2Packet &rkPacket );

	// ����
	void OnShuffleRoomTransfer( SP2Packet &rkPacket );
	void OnShuffleRoomJoinResult( SP2Packet &rkPacket );
	void OnShuffleRoomSync( SP2Packet &rkPacket );
	void OnShuffleRoomKickOut( SP2Packet &rkPacket );
	void OnExceptionShuffleRoomLeave( SP2Packet &rkPacket );
	void OnShuffleRoomGlobalCreate( SP2Packet &rkPacket );

	// ��
	void OnMoveNewPetData( SP2Packet &rkPacket );

	// �뺴,ETC, �޴�
	void OnMoveSoldierData( SP2Packet &rkPacket );
	void OnMoveEtcItemData( SP2Packet &rkPacket );
	void OnMoveMedalItemData( SP2Packet &rkPacket );

	//�ڽ�Ƭ
	void OnMoveCostumeData( SP2Packet &rkPacket );
	void OnCostumeAdd(SP2Packet &rkPacket);

	//�̼�
	void OnMoveMissionData( SP2Packet &rkPacket );

	//�⼮��
	void OnMoveRollBookData( SP2Packet &rkPacket );

	//��� ����
	void OnReserveCreateGuildRoom(SP2Packet &rkPacket);
	void OnChangeGuildRoomStatus(SP2Packet &rkPacket);

	//���� ����
	void OnJoinPersonalHQUser(SP2Packet &rkPacket);
	void OnPersonalHQInfo( SP2Packet &rkPacket );
	void OnPersonalHQRoomInfo( SP2Packet &rkPacket );
	void OnPersonalHQJoinAgree(SP2Packet &rkPacket );
	void OnMovePersonalHQData(SP2Packet &rkPacket );
	void OnPersonalHQAddBlock(SP2Packet &rkPacket );

	//�Ⱓ ĳ�� �ڽ�
	void OnMoveTimeCashData( SP2Packet &rkPacket );
	void OnUpdateTimeCashInfo( SP2Packet &rkPacket );

	//Īȣ
	void OnTitleUpdate(SP2Packet &rkPacket );
	void OnMoveTitleData( SP2Packet &rkPacket );

	//���ʽ� ĳ��
	void OnMoveBonusCashData(SP2Packet &rkPacket );
	void OnResultBonusCashAdd(SP2Packet &rkPacket );
	void OnResultBonusCashUpdate(SP2Packet &rkPacket );

	//�Ǽ��縮
	void OnMoveAccessroyData(SP2Packet &rkPacket );
	void OnAccessoryAdd(SP2Packet &rkPacket);

public:
	//���� �̵��� ������ ���� ����.
	void SendDecoMoveData( int iMovingValue, User *pUser );			// ġ��
	void SendExtraItemMoveData( int iMovingValue, User *pUser );	// ���

public:
	bool IsSameBilling(DWORD serverIndex);// �̵� �Ϸ� �� 

public:
	ServerNode( SOCKET s = INVALID_SOCKET, DWORD dwSendBufSize = 0, DWORD dwRecvBufSize = 0 );
	virtual ~ServerNode();
};

#endif
