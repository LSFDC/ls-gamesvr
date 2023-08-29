// User.h: interface for the User class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USER_H__03E2797C_74A5_433B_B34D_2E60C0784CEF__INCLUDED_)
#define AFX_USER_H__03E2797C_74A5_433B_B34D_2E60C0784CEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UserParent.h"
#include "RoomParent.h"
#include "Item.h"
#include "ioEquipSlot.h"
#include "NodeHelpStructDefine.h"

#include "ioCharacter.h"
#include "ioInventory.h"
#include "ioAward.h"
#include "ioClassExpert.h"
#include "ioFriend.h"
#include "ioUserGuild.h"
#include "ioUserRecord.h"
#include "ioUserGrowthLevel.h"
#include "ChannelParent.h"
#include "../HackCheck.h"
#include "ioEventUserManager.h"
#include "ioUserEtcItem.h"
#include "ioUserPresent.h"
#include "ioUserSubscription.h"
#include "ioUserFishingItem.h"
#include "ioQuest.h"
#include "ioUserExtraItem.h"
#include "ioUserMedalItem.h"
#include "ioUserExpandMedalSlot.h"
#include "ioCharRentalData.h"
#include "ioUserSelectShutDown.h"
#include "ioAlchemicInventory.h"
#include "ioUserTournament.h"
#include "ioClover.h"
#include "ioBingo.h"
#include "ioExerciseCharIndexManager.h"
#include "ioUserPcRoom.h"
#include "ioUserAttendance.h"
#include "ioEtcItemHelp.h"
#include "ioUserPet.h"
#include "ioTimeStamp.h"
#include "ioUserCostume.h"
#include "ioMission.h"
#include "ioUserRollBook.h"
#include "PersonalHQInven.h"
#include "UserTimeCashTable.h"
#include "UserTitleInven.h"
#include "ioPirateRoulette.h"
#include "ioUserAccessory.h"
#include "UserBonusCash.h"
#include "ioUserCoin.h"

#ifdef XTRAP
#include "../Xtrap/ioXtrap.h"
#endif

#ifdef NPROTECT
#ifdef NPROTECT_CSAUTH3
	#include "../nProtect/ggsrv30.h"
	#ifdef _DEBUG
		#pragma comment( lib, "ggsrv30lib_x86_MD.lib" )
	#else
		#pragma comment( lib, "ggsrv30lib_x86_MT.lib" )
	#endif
#endif
#endif // NPROTECT

#ifdef HACKSHIELD
#include "../HackShield/ioHackShield.h"
#endif

#ifdef SRC_LATIN
#include "../Apex/ioApex.h"
#endif

class Room;
class BattleRoomParent;
class LadderTeamParent;
class ShuffleRoomParent;
class CConnectNode;
class ServerNode;
class User;
class ioEtcItem;
class ioSetItemInfo;

struct UserSyncTable
{
	int  m_iGradeLevel;
	int  m_iUserPos;
	int  m_iKillDeathLevel;
	int  m_iLadderPoint;
	bool m_bSafetyLevel;

	UserSyncTable()
	{
		Initialize();
	}

	void Initialize()
	{
		m_iGradeLevel		= -1;
		m_iUserPos			= -1;
		m_iKillDeathLevel   = -1;
		m_iLadderPoint      = 0;
		m_bSafetyLevel      = false;
	}
};

// �뿡�� ���� �����³Ŀ� ���� �ǽð����� ����Ǹ� �� ������ �� �ʱ�ȭ�Ѵ�.
struct UserBonusTable
{
	bool m_bPCRoomBonus;
	bool m_bGuildBonus;
	bool m_bFriendBonus;
	UserBonusTable()
	{
		Init();
	}

	void Init()
	{
		m_bPCRoomBonus = m_bGuildBonus = m_bFriendBonus = false;
	}
};

// ģ�� ��õ �̺�Ʈ�� ������
struct FriendRecommendEvent
{
	DWORD m_dwTableIndex;
	DWORD m_dwRecommendIndex;

	FriendRecommendEvent()
	{
		Init();
	}

	void Init()
	{
		m_dwTableIndex = 0;
		m_dwRecommendIndex = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////

class User : public CConnectNode,
			 public UserParent
{
	friend class UserNodeManager;

public:
	enum ChatType
	{
		CT_NONE = -1,
		CT_TEAM,
		CT_ALL,
		CT_PRIVATE,
		CT_ANNOUNCE,
		CT_PARTY,
		CT_LADDER,
		CT_SERVER_LOBBY,
		CT_WHOLE_SERVER,
	};

	// ������带 ������ ���� �ʰ� �Ʒ��� ������Ʈ�� ��������� ������ ó���Ѵ�. �� ũ��Ƽ�� ������ �ʿ��������.
	enum SessionState
	{
		SS_DISCONNECT	= 0,
		SS_CONNECT		= 1,
		SS_SERVERMOVING = 2,
	};

private:
	int		m_entity;
	SessionState m_eSessionState;
	bool m_bSessionDestroySave;

private:
	USERDATA m_user_data;                   //������ ��� ����.
	UserRelativeGradeData m_user_relative_grade_data;
	DWORD    m_dwDBAgentID;                 //����� DBAgent ID
	DWORD    m_save_time;                   //���� �ð�.
	DWORD    m_dwSaveCheckTime;             //���� �ð� ���� + 0 ~ 1800000

private:
	DWORD	 m_sync_time;                   //����ȭ �ð�
	DWORD    m_dwSyncCheckTime;             //���ɼ��� ���� �ð� 120000 ~ 300000.
	DWORD    m_dwConnectTime;               //���� �ð�.

	int		 m_ping_total_send_index;		//���ݱ��� ������ ���� �ε���
	int      m_ping_less_error_count;		//����ȭ �ð����� ���� ����
	int		 m_ping_over_error_count;		//����ȭ �ð����� ���� ����
	int		 m_total_ping_error_count;		//�� ����ȭ ����
	DWORD	 m_prev_over_ping_time;			//�����޼����� �ʰ� �°���� �ð���
	bool	 m_first_heart_beat;			//ù��° Ping ����ȭ
	DWORD    m_dwPingStep;                  //Ping �ܰ�. 0(����) ~~~ N(����)
	bool     m_ladder_point_change;         //��������Ʈ�� ����Ǿ���.
	bool     m_hero_expert_change;          //������ ����ġ�� ����Ǿ���.
	bool     m_first_login_user;            //ù ���� ����

	int		 m_iUserIPType;					//���� IPŸ�� ( �Ϲ�, ������, Ŭ���� )

private:
	HackCheck::CheckProblem m_SpeedHackQuiz;
	DWORD	m_dwSpeedHackQuizLimitTime;
	int     m_iCurSpeedHackAnswerChance;

private:
	HackCheck::CheckProblem m_AbuseQuiz;
	DWORD	m_dwAbuseQuizLimitTime;
	int     m_iCurAbuseAnswerChance;
protected://relay ���� 
	int     m_iRelayServerID;
	int     m_iDropKing;


public://get/set//relay
	int DropKing() const { return m_iDropKing; }
	void DropKing(int val) { m_iDropKing = val; }
	int RelayServerID() const { return m_iRelayServerID; }
	void SetRelayServerID(int val) { m_iRelayServerID = val; }
	bool IsRelayUse() { if(m_iRelayServerID != 0)
							return true ;
						else
							return false;
					  }
	void LeaveProcess();
private:
	bool    m_bDeveloper;

private:
	// ��
	Room	*m_pMyRoom;                 
	int      m_iLeaveRoomIndex;                 // ��Ż�� �� �ε���
	DWORD    m_dwLeaveRoomTime;                 // �� ��Ż �ð�
	DWORD    m_dwMemoryLeaveRoomTime;           // ��Ż �� �ε��� ��� �ð�.
	bool     m_bJoinServerLobby;                // ���� �κ� �ִ� ����.
	DWORD    m_dwMyHeadquartersIndex;           // �� ���� �� �ε���

	UserBonusTable m_BonusTable;                // �뿡�� ������ ������ ���ʽ� ���� �߻�
	UserBonusTable m_BonusTableBackup;

	// ���� ��
	DWORD    m_dwMyBattleRoom;
	int      m_iSearchBatleRoomIndex;           // -1�̸� �ڵ�
	int      m_iLeaveBattleRoomIndex;           // ��Ż�� ������ �ε���
	DWORD    m_dwLeaveBattleRoomTime;           // ������ ��Ż �ð�
	DWORD    m_dwMemoryLeaveBattleRoomTime;     // ��Ż ������ �ε��� ��� �ð�.

	// ���÷�
	DWORD    m_dwMyShuffleRoom;

	// �����
	DWORD    m_dwMyLadderTeam;
	int	     m_iMyVictories;

	// ������
	int m_iCompetitorIndex;

	// ���� ���
	int      m_iModeConsecutivelyCnt;
	
	static int m_iLimiteMaxCharSlot;
	static int m_iDefaultMaxCharSlot;
	static int m_RefCount;
	int m_iCurMaxCharSlot;

	int	 m_select_char;						//���� ���õ� ĳ����.
	typedef std::vector< ioCharacter * > vCharList;
	vCharList m_CharList;	

	ioInventory		       m_Inventory;
	ioUserEtcItem          m_UserEtcItem;
	ioClassExpert	       m_ClassExpert;
	ioAward			       m_Award;
	ioFriend               m_Friend;
	ioQuest                m_Quest;
	ioUserRecord           m_UserRecord;
	ioUserGuild            m_UserGuild;
	ioUserPresent          m_UserPresent;
	ioUserSubscription     m_UserSubscription;
	ioUserGrowthLevel      m_UserGrowthLevel;
	ioUserFishingItem      m_UserFishingItem;
	ioUserExtraItem        m_UserExtraItem;
	ioUserMedalItem        m_UserMedalItem;
	ioUserExpandMedalSlot  m_UserExpandMedalSlot;
	UserHeroData           m_UserHeroData;
	ioCharRentalData       m_CharRentalData;
	UserHeadquartersOption m_UserHeadquartersData;
	ioUserSelectShutDown   m_UserSelectShutDown;
	ioAlchemicInventory    m_AlchemicInventory;
	ioUserTournament       m_UserTournament;
	ioClover		       m_Clover;
	ioBingo			       m_Bingo;
	ioUserPcRoom	       m_UserPcRoom;
	ioUserAttendance       m_UserAttendance;
	ioUserPet			   m_UserPetItem;	
	ioTimeStamp			   m_Timestamp;
	ioUserCostume		   m_UserCostume;
	ioUserAccessory		   m_UserAccessory;
	ioMission			   m_UserMission;
	ioUserRollBook		   m_UserRollBook;
	PersonalHQInven		   m_PersonalHQInven;
	UserTimeCashTable	   m_UserTimeCashTabel;
	UserTitleInven		   m_TitleInven;
	ioPirateRoulette	   m_PirateRoulette;
	UserBonusCash		   m_UserBonusCash;
	ioUserCoin			   m_UserCoin;

	DWORDVec        m_vChannelNode;	
	UserSyncTable   m_SyncTable;

	DWORD           m_dwMyIP;                        //4����Ʈ�� ����� �ڱ� ������
 
	int m_PreRoomNum;
	DWORD    m_dwStartTimeLog;

	//
	DWORD m_dwFishingStartTime;
	DWORD m_dwFishingLoopTime;

	int m_iFishingRodType;
	int m_iFishingBaitType;
	DWORD m_dwFisheryCode;
	//

private:
	TeamType m_Team;                        //����, �Ʒ��� �� Ÿ��
	TeamType m_ShamBattleTeam;              //��Ƽ �����ϱ��� �� Ÿ��.

private:                                    //������ ó��
	bool	 m_bExitRoomReserved;
	DWORD    m_dwExitRoomTime;
	DWORD    m_dwCurExitRoomTime;	
	bool     m_bExitLobby;                  //�κ��

private:
	bool     m_bMovieCapturing;
	DWORD    m_dwPCRoomNumber;

protected:
	bool m_bStealth;
	
	bool m_bNeedSendPushStruct;				// ���ε��ͼ� ������ ���� ������� �ϴ��� ����
	int m_iNeedSendPushStructMaxIndex;		// ��������ϴ� �������� ������ index
	int m_iSendPushStructIndex;				// ���������� ���� ������ Index
	DWORD m_dwSendPushStructCheckTime;		// ���������� ���� �ð�.

protected:									// ��Ʈ��ũ ���µ��� �հ���� üũ ����
	int m_iCurCheckKingIndex;
	int m_iCurRecvKingPingCnt;

private:
	ioHashString *m_pEncLoginKey;

public:
	static bool m_bUseSecurity;
	static int  m_iSecurityOneSecRecv;
	static int 	m_iRevision;

protected:      //���� �̵� ����
	int  m_iServerMovingValue;
	bool m_bReserveServerMoving;            //���� �̵� ����
	bool m_bReserveBattleRoom;              //������ ���� ����
	bool m_bReserveLadderTeam;              //����� ���� ����

	bool m_bShuffleGlobalSearch;
	bool m_bReserveShuffleRoom;

protected:      //��� ��ũ ���� �������� ��ȣ
	DWORD m_dwGuildMarkChangeKeyValue;

#ifdef XTRAP
	BYTE  m_XtrapSessionBuf[ioXtrap::MAX_SESSION_BUF];
	DWORD m_dwXtrapCheckTime;
#endif

#ifdef NPROTECT
#ifdef NPROTECT_CSAUTH3
	CCSAuth3 m_NProtectAuth;
#else
	CCSAuth2 *m_pNProtectAuth;
#endif
	DWORD     m_dwNProtectCheckTime;
	int       m_iSentNProtectCheckCnt;
#endif // NPROTECT

#ifdef HACKSHIELD
	AHNHS_CLIENT_HANDLE m_hHackShield;
	DWORD               m_dwHackShieldCheckTime;
#endif

	bool  m_bFirstChangeID;

	DWORD m_dwExcavatingTime;
	DWORD m_dwTryExcavatedTime;

	bool  m_bSendCheckPCRoom;
	DWORD m_dwOldServerIndex;
	bool m_bSendLogOutUserState;

	float m_fExpBonusEventValue;

public://get set 
	DWORD GetOldServerIndex() const { return m_dwOldServerIndex; } // 
	bool GetSendLogOutUserState() const { return m_bSendLogOutUserState; }
	void SetSendLogOutUserState(bool val) { m_bSendLogOutUserState = val; }

	void SetExpBonusEventValue(float fExpBonusValue) { m_fExpBonusEventValue = fExpBonusValue; }
	float GetExpBonusEventValue() { return m_fExpBonusEventValue; }
	float GetExpBonusEvent();

protected:
	bool  m_bShutDownUser;

protected:  // ģ�� ��õ �̺�Ʈ��
	FriendRecommendEvent m_FriendRecommendData;
	
	CTime m_cLoginTime;

protected:
	//enum { MAX_SELECT_EXTRA_ITEM_CODE = 10, };
	int  m_iSelectExtraItemCodes[MAX_SELECT_EXTRA_ITEM_CODE];
	void ClearSelectExtraItemCodes();
public://������ ���� 
	bool TimeOutClose(int checkTime);
	void SendUserLogOut();
public:
	void GetSelectExtraItemCodes( IntVec &rvInt );

public:
	int	 GetEntity()	{ return m_entity; }

	void InitServerMovingValue();
	int  GetServerMovingValue();
	int  SetServerMoving();
	bool IsServerMoving();	

	void ReserveServerMoving();
	bool IsReserveServerMoving();

	inline void ReserveBattleRoom( bool bReserve ) { m_bReserveBattleRoom = bReserve; }
	inline bool IsReserveBattleRoom() { return m_bReserveBattleRoom; }

	inline void ReserveLadderTeam( bool bReserve ) { m_bReserveLadderTeam = bReserve; }
	inline bool IsReserveLadderTeam() { return m_bReserveLadderTeam; }
	
	inline void SetShuffleGlboalSearch( bool bReserve ) { m_bShuffleGlobalSearch = bReserve; }
	inline bool IsShuffleGlboalSearch() { return m_bShuffleGlobalSearch; }

private:
	int  m_iCreateCharCount;                  //�뺴 DB Insert ī����

public:
	ioHashString	m_szMacAddress;
	ioHashString	m_szPremiumKey;
	ioHashString	m_szAuthInfo;
	DWORD			m_dwSessionID;

private:
	struct LevelUPData
	{
		int m_iType;
		int m_iLevel;
		LevelUPData()
		{
			m_iType = m_iLevel = 0;
		}
	};
	typedef std::vector< LevelUPData > vLevelUPData;
	vLevelUPData m_vLevelUpClass; 
	DWORDVec m_vLevelUpAndPresentCreateClass; // �������̳� ������ ���� �뺴�� DB�� Insert�Ǳ� ���� ���
	ioHashString m_szGUID;			          // ���ӽ� ���� ���Ӱ� �ο��Ǵ� ������ id, DB�� ����Ҷ� user�� �ĺ��Ѵ�. 

	EventUserManager m_EventUserMgr;
	ioHashString m_szBillingGUID; // ����,�ܾ���ȸ�Ǹ��� ���Ӱ� ���ŵǴ� ID

	ioHashString m_szNewPublicID;
	ioHashString m_szPublicIPForNewPublicID;

	ioHashString m_szBillingUserKey;


	ControlKeys  m_kControlKeys;

	// ��ƾ�� �Ⱓ�� �뺴�� ���� ���� ���� ������ �����صδ� ���� (DB �����)	JCLEE 140416
	int	m_iLimitSecForLatin;

	// ���� ���� ������ ���� �����
	int m_iBeforeMonsterCoin;

protected:	
	SelectGashaponValueVec  m_SelectGashaponItem;

public:
	void ConnectProcessSelectChar();
	void ConnectProcessComplete();
	bool IsConnectProcessComplete(){ return m_user_data.m_bSavePossible; }
	
public:
	int  GetCreateCharCount(){ return m_iCreateCharCount; }
	void SetCreateCharCount( int iCreateCharCount ){ m_iCreateCharCount = iCreateCharCount; }
	bool IsCharCreating(){ return ( GetCreateCharCount() > 0 ); }

	bool IsCharIndex( DWORD dwCharIndex );

public:
	static void LoadHackCheckValue();
	static void LoadCharSlotValue();
	static void	LoadNagleReferenceValue( int refCount = 0 );

public:
	ioHashString * GetEncLoginKey() const;
	void ClearEncLoginKey();

protected:
	void InitData();

public:
	void InitCharList();

	void SaveData();
	void SaveUserData();            //���� ���� ����
	void SaveUserLadderData();      //���� ���� ����Ʈ
	void SaveUserHeroExpert();      //���� ������ ����ġ
	void SaveClassExpert();         //Ŭ���� ���� ���� ����
	void SaveCharacter();           //ĳ���� ����
	void SaveInventory();           //�κ��丮 ����
	void SaveEtcItem();             //Ư�������� ����
	void SaveAward();               //�û󳻿� ����
	void SaveQuest();               //����Ʈ ����
	void SaveRecord();              //���ڵ� ����
	void SaveGrowth();				//���� ����
	void SaveFishItem();			//���þ����� ����
	void SaveExtraItem();			//�߰���� ����
	void SaveControlKeys();			//����Ű ����
	void SaveMedalItem();			//�޴� ������ ����
	void SaveExpandMedalSlot();		//�޴�Ȯ�彽�� ����
	void SaveBestFriend();          //��ģ ����
	void SaveUserHeadquarters();    //���� ����
	void SaveAlchemicInventory();	//���� ����
	void SaveClover();				// Ŭ�ι� ����.
	void SaveFriendClover();		// ģ����� Ŭ�ι� ����.
	void SaveBingo();				// ���� ����.
	void SavePet();					//�� ����
	void SaveTimeMission();			// �ð� �̼� ����.
	void SavePirateRoulette();		// �����귿 ����.

public:
	virtual void SessionClose( BOOL safely=TRUE );
	virtual bool SendMessage( CPacket &rkPacket );
	virtual void ReceivePacket( CPacket &packet );
	virtual void PacketParsing(CPacket &packet);
	virtual void SetClientAddr(sockaddr_in client_addr);
	void SetClientAddr( const char* back_iip, const int back_port );
	void SetClientAddressForRelay(char* szIpaddr,int iPort);


	bool RoomBroadCast( SP2Packet &rkPacket );

public:
	virtual void OnCreate();       //�ʱ�ȭ
	virtual void OnDestroy();
	void OnSessionDestroy();       //���� ���� & �ʱ�ȭ
	void OnNameChageSync();        //���̵� ���� �� ó��
	virtual bool CheckNS( CPacket &rkPacket );
	virtual int  GetConnectType();
	
public:
	bool IsSessionDestroySave(){ return m_bSessionDestroySave; }
	void SetSessionState( User::SessionState eState ){ m_eSessionState = eState; }
	bool IsDisconnectState(){ return ( m_eSessionState == SS_DISCONNECT ); }
	bool IsConnectState(){ return ( m_eSessionState == SS_CONNECT ); }

	//HRYOON
	virtual void InsertUserLadderList( int competitorIndex, int );

public:
	const int GetUserEventType() const { return m_user_data.m_user_event_type; }

public:

#ifdef ANTIHACK
	//rudp
	DWORD m_dwUserSeed;
	DWORD m_dwNPCSeed;
	void	UpdateSeed( DWORD dwRand );
	DWORD	GetUserSeed(){ return m_dwUserSeed; }
	DWORD	GetNPCSeed(){ return m_dwNPCSeed; }

	void UpdateSpPotion();
	void SetDieState();
#endif
	void EnterRoom(Room *pRoom);	
	void LeaveRoom( BYTE eType = (BYTE)RoomParent::RLT_NORMAL );
	void NetworkCloseLeaveRooom();
	void NetworkCloseLogout();
	void SetLeaveRoomTime( int iIndex );
	void EnterRoomSelectChar();
	void CheckLeaveRoomIndex();

	void InitBonusTable();
	void SetBonusTable( UserBonusTable &kBonusTable );
	bool ChangeBonusTable();
	void BackupBonusTable();
	void CheckRoomBonusTable();
	inline const UserBonusTable &GetBonusTable(){ return m_BonusTable; }

	void ExitRoomToTraining( int iResult, bool bPenalty );
	void PrivatelyLeaveRoomToTraining();
	bool IsPartyPossible();	
			
public:
	void LeaveServer( const int iServerIndex );

	void EnterBattleRoom( BattleRoomParent *pBattleRoom, bool bObserver );
	bool EnterBattleRoom( DWORD dwBattleRoom, ServerNode *pBattleRoomServer );
	void LeaveBattleRoom( bool bJoinFailed = false );
	void LeaveBattleRoomException( DWORD dwNodeIndex );
	bool IsBattleRoom();
	void ExitRoomToBattleRoomJoin( BattleRoomParent *pBattleRoom, bool bObserver, bool bMovePesoPenalty, int iPenaltyPeso );
	void SetLeaveBattleRoomTime( int iIndex );
	void CheckLeaveBattleRoomIndex();

public:
	void EnterShuffleRoom( ShuffleRoomParent *pShuffleRoomNode, bool bReJoinCheck = true );
	void LeaveShuffleRoom( bool bJoinFailed = false );
	void LeaveShuffleRoomException( DWORD dwNodeIndex );
	bool IsShuffleRoom();

public:
	void EnterLadderTeam( LadderTeamParent *pLadderTeam );
	bool EnterLadderTeam( DWORD dwLadderTeam, ServerNode *pLadderTeamServer );
	void LeaveLadderTeam();
	void LeaveLadderTeamException( DWORD dwNodeIndex );
	bool IsLadderTeam();
	void ExitRoomToLadderTeamJoin( LadderTeamParent *pLadderTeam );
	void LeaveGuildToLeaveGuildPlaza(DWORD dwGuildIndex);
	void LeaveGuildToQuestClear();

public:
	void EnterChannel( ChannelParent *pNode );
	void LeaveChannel( ChannelParent *pNode = NULL );
	void LeaveChannelException( DWORD dwNodeIndex );
	ChannelParent *FindChannel( DWORD dwIndex );

public:
	void EquipDBItemToChar( int iCharArray );
	void EquipDBItemToAllChar();
	void EquipGangsiItemToAllChar();
	void EquipDBItemToLiveChar();
	void AllCharReEquipDBItem( int iSlotIndex );
	void ReleaseEquipAllChar();
	void ReleaseAllItemSelectChar();
	void ClearRealEquipItemOwner( const ioHashString &rkOutUser );
	ioItem* ReleaseItem( int iSlot );
	bool IsSlotEquiped( int iSlot ) const;
	bool IsEquipedItem();
	const ioItem* GetItem( int iSlot ) const;
	const int GetEquipItemCount();

	ioCharacter* GetCharacter( int iArray );
	ioCharacter* GetLeaderCharacter();
	void CreateSelectCharData( ioCharacter *pChar );
	void StartCharLimitDate( DWORD dwCheckSecond, const ioHashString &szCallFunName, int iLine );
	void StartCharLimitDate( ioCharacter *pChar );
	bool UpdateCharLimitDate();

	// Ex Info
	ioInventory*      GetInventory();
	ioUserEtcItem*    GetUserEtcItem();
	ioClassExpert*    GetClassExpert();
	ioAward*          GetAward(); 
	ioQuest*          GetQuest();
	ioUserRecord*     GetUserRecord();
	ioUserGuild*      GetUserGuild();
	ioUserPresent*    GetUserPresent();
	ioFriend*		  GetFriend();
	ioClover*		  GetClover();
	ioBingo*		  GetBingo();
	ioPirateRoulette* GetPirateRoulette();

	ioUserSubscription*    GetUserSubscription();	
	ioUserGrowthLevel*     GetUserGrowthLevel();
	ioUserFishingItem*     GetUserFishingItem();
	ioUserExtraItem*       GetUserExtraItem();
	ioUserMedalItem*       GetUserMedalItem();
	ioUserExpandMedalSlot* GetUserExpandMedalSlot();
	ioCharRentalData*      GetCharRentalData();
	ioAlchemicInventory*   GetAlchemicInventory();
	ioUserTournament*      GetUserTournament();
	ioUserAttendance*      GetUserAttendance();
	ioUserPet*			   GetUserPetItem();
	ioUserCostume*		   GetUserCostume() { return &m_UserCostume; }
	ioUserAccessory*	   GetUserAccessory() { return &m_UserAccessory; }
	ioMission*			   GetUserMission()	{ return &m_UserMission; }
	ioUserRollBook*		   GetUserRollBook() { return &m_UserRollBook; }
	PersonalHQInven*	   GetUserPersonalHQInven() { return &m_PersonalHQInven; }
	UserTimeCashTable*	   GetUserTimeCashTable() { return &m_UserTimeCashTabel; }
	UserTitleInven*		   GetUserTitleInfo() { return &m_TitleInven; }
	UserBonusCash*		   GetUserBonusCash() { return &m_UserBonusCash; }		//?
	//
	void  GetEquipMedalClassTypeArray( int iClassType, IntVec &kMedal );
	bool  GetDisplayCharacter( DWORD dwCharIndex, float &rkXPos, float &rkZPos );
	DWORD GetDisplayCharMotion( DWORD dwCharIndex );
	bool IsHeadquartersLock();

public: // ģ��

	void User::InsertFriend( int iIndex, DWORD dwUserIndex, const ioHashString &szName, int iGradeLevel, int iCampPos, DWORD dwRegTime
						, int iSendCount, DWORD dwSendDate, int iReceiveCount, DWORD dwReceiveDate, int iBeforeReceiveCount, bool bSave );

	bool DeleteFriend( const ioHashString &szName );
	bool IsFriend( const ioHashString &szName );
	bool IsFriendRegHourCheck( const ioHashString &szName, DWORD dw24HourBefore );
	int  GetFriendOnlineUserCount( DWORD dw24HourBefore );
	int  GetFriendLastIndex();
	int  GetFriendSize(){ return m_Friend.GetFriendSize(); }

	void InsertBestFriend( DWORD dwTableIndex, DWORD dwFriendUserIndex, DWORD dwState, DWORD dwMagicDate );
	void UpdateBestFriend( DWORD dwUserIndex, DWORD dwState, DWORD dwMagicDate );

	void SendFriendAndGuildUserLogOut();
	void SendFriendPacket( int iLastIndex, bool bSendLogin );
	void SendBestFriendPacket( int iLastIndex );
	bool SendFriendServerMove( int iLastIndex, int iSendCount, SP2Packet &rkPacket );

#ifdef XTRAP
	bool SendXtrapStep1();
#endif
#ifdef NPROTECT
	bool SendNProtectCheck();
#endif 
#ifdef HACKSHIELD
	bool SendHackShieldCheck();
#endif 

	// Ex Info Fun
	void  AddAward( int iCategory, int iPoint );
	bool  IsPCRoomAuthority();
	DWORD GetPCRoomNumber() const;
	void  SetPCRoomNumber( DWORD dwPCRoomNumber ) { m_dwPCRoomNumber = dwPCRoomNumber; }
	void  ChildrenDayEventEndProcess();
	bool  IsRoomLoadingOrServerMoving();
	
public:
	virtual DWORD GetUserIndex() const { return m_user_data.m_user_idx; }
	virtual DWORD GetUserDBAgentID() const;
	virtual const DWORD GetAgentThreadID() const { return m_user_data.m_private_id.GetHashCode(); }
	virtual const ioHashString& GetPrivateID() const { return m_user_data.m_private_id; }
	virtual const ioHashString& GetPublicID() const { return m_user_data.m_public_id; }
	virtual int GetUserCampPos() const { return m_user_data.m_camp_position; }
	virtual int GetGradeLevel();
	virtual int GetUserPos();
	virtual int GetKillDeathLevel();
	virtual int GetLadderPoint() { return m_user_data.m_ladder_point; }
	virtual bool IsSafetyLevel();
	virtual const bool IsUserOriginal(){ return true; }
	virtual bool RelayPacket( SP2Packet &rkPacket );
	virtual ModeType GetModeType();
	virtual const bool IsDeveloper(){ return m_bDeveloper; }	
	virtual int GetUserRanking(){ return m_user_data.m_user_rank; }
	virtual DWORD GetPingStep(){ return m_dwPingStep; }
	virtual bool  IsGuild();
	virtual DWORD GetGuildIndex();
	virtual DWORD GetGuildMark();
	virtual bool IsBestFriend( DWORD dwUserIndex );
	virtual void GetBestFriend( DWORDVec &rkUserIndexList );
	bool IsGeneralGrade();
	
	bool IsObserver();
	bool IsStealth(){ return m_bStealth; }
	BlockType GetBlockType(){ return m_user_data.m_eBlockType; }

	// Ÿ ������ �ڽ��� ���� ����ȭ
public:
	void SyncUserLogin();
	void FillUserLogin( SP2Packet &rkPacket );
	void SyncUserLogout();
	void SyncUserUpdate();	
	void SyncUserPos();
	void SyncUserGuild();
	void SyncUserCamp();
	void SyncUserPublicID();
	void SyncUserBestFriend();
	void SyncUserShuffle();
	void FillUserBestFriend( SP2Packet &rkPacket );

public:
	int GetGradeExpert();
	int GetGradeExpRate();
	int GetBackupGradeExp();
	int GetClassLevel( int iSelectChar, bool bOriginal );
	int GetClassLevelByType( int iClassType, bool bOriginal );
	bool IsLeaveSafeRoom();

public:
	void SetMoney( __int64 i64Money );
	void AddMoney( __int64 i64Money );
	void RemoveMoney( __int64 i64Money );
	void SetCash( int iCash );
	void AddCash( int iCash );
	__int64 DecreasePenaltyPeso( __int64 i64Money );
	void SetChannelingCash( int iCash );
	void AddChannelingCash( int iCash );
	void SetPurchasedCash( int iPurchasedCash );
	void AddPurchasedCash( int iPurchasedCash );

	__int64 GetMoney() const { return m_user_data.m_money; }
	__int64 GetFirstMoney() const { return m_user_data.m_iFirstPeso; }
	int		GetFirstExp() const { return m_user_data.m_iFirstExp; }

	void    SetFirstWinCount( int iWin ) { m_user_data.m_iFirstWinCount = iWin; }
	int		GetFistWinCount() const { return m_user_data.m_iFirstWinCount; }
	void    SetFirstLoseCount( int iLose ) { m_user_data.m_iFistLoseCount = iLose; }
	int		GetFirstLoseCount() const { return m_user_data.m_iFistLoseCount; } 


	void    IncreaseWinCount() { m_user_data.m_iWinCount++; }
	int		GetWinCount() const { return m_user_data.m_iWinCount; }

	void    IncreaseLoseCount() { m_user_data.m_iLoseCount++; }
	int		GetLoseCount() const { return m_user_data.m_iLoseCount; } 

	int GetCash() const { return m_user_data.m_cash; }
	int GetPurchasedCash() const { return m_user_data.m_purchased_cash; }
	int GetUserState() const { return m_user_data.m_user_state; }
	int GetJoinChannelSize(){ return m_vChannelNode.size(); }

	ChannelingType      GetChannelingType() const;
	const ioHashString &GetChannelingUserID() const;
	const ioHashString &GetChannelingUserNo() const;
	int       GetChannelingCash() const;
	BlockType GetBlockType() const;
	void      SetBlockType( BlockType eBlockType );
	float     GetBlockPointPer(); 
	CTime     GetBlockTime() const;
	void      SetBlockTime( CTime &rkTime );

	bool  IsFirstJoinUser() const { return m_first_login_user; }
	CTime GetLastLogOutTime() const { return m_user_data.m_connect_time; }

	//������ ��û�ϴ� ��Ŷ
	void SetOutPutCashPacket( SP2Packet &rkPacket, const int& iBuyCash, const int& iMachineCode, const int& iType );
	
public:
	bool AddGradeExp( int iExp );
	void AddClassExp( int iClassType, int iExp );
	int  GradeNClassUPBonus();
	
	void AddKillCount( RoomStyle eRoomStyle, ModeType eModeType, int iCount );
	void AddDeathCount( RoomStyle eRoomStyle, ModeType eModeType, int iCount );
	void AddWinCount( RoomStyle eRoomStyle, ModeType eModeType, int iCount );
	void AddLoseCount( RoomStyle eRoomStyle, ModeType eModeType, int iCount );

public:
	DWORD GetEtcMotionAni( DWORD dwMotionType );

	bool InitEtcItemUseBattleRecord();
	bool InitEtcItemUseLadderRecord();
	bool InitEtcItemUseHeroRecord();
	
	bool InitHeroSeasonRecord();
	bool HeroSeasonEndDecreaseRate();
	void AddHeroExpert( int iHeroExpert );
	void AddLadderPoint( int iLadderPoint );
	bool SetLadderPoint( int iLadderPoint );
	void AddCampSeasonBonus( DWORD dwBonusIndex, int iBlueCampPoint, int iBlueCampBonusPoint, int iBlueEntry, 
							 int iRedCampPoint, int iRedCampBonusPoint, int iRedEntry, int iMyCampType, int iMyCampPoint, int iMyCampRank );

	void InitUserLadderPointNRecord();
public:
	inline const int GetConnectCnt() const { return m_user_data.m_connect_count; }
	inline DWORD GetDwordMyIP() const { return m_dwMyIP; }

	DWORD GetCharIndex(int array) const;
	int GetCharClassType(int array) const; 
	int GetTopLevelClassType();
	int GetCharArray( DWORD dwIndex ) const;
	bool IsCharMortmain( int iArray ) const;
	bool IsCharClassType( int iClassType ) const;
	bool IsActiveRentalClassType( int iClassType );
	
	int GetCharCount() const { return m_CharList.size(); }         // ���� �뺴 + ü�� �뺴
	int GetActiveCharCount();
	int GetBuyCharCount();                                         // ���� �뺴
	int GetActiveBuyCharCount();                                   // ���� �뺴 + ������� �뺴
	int GetExerciseCharCount();                                    // ü�� �뺴
	int GetExerciseCharCount( int chExercise );
	int GetActiveRentalCount();

	inline DWORD GetSaveTime() const { return m_save_time; }
	inline DWORD GetSaveCheckTime() const { return m_dwSaveCheckTime; }
	inline DWORD GetSyncTime() const { return m_sync_time; }
	inline DWORD GetSyncCheckTime() const { return m_dwSyncCheckTime; }
#ifdef XTRAP
	inline DWORD GetXtrapCheckTime() const { return m_dwXtrapCheckTime; }
#endif
#ifdef NPROTECT
	inline DWORD GetNProtectCheckTime() const { return m_dwNProtectCheckTime; }
	inline int   GetSentNProtectCheckCnt() const { return m_iSentNProtectCheckCnt; }
#endif 
#ifdef HACKSHIELD
	inline DWORD GetHackShieldCheckTime() const { return m_dwHackShieldCheckTime; }
#endif
	inline TeamType GetTeam() const { return m_Team; }
	inline TeamType GetShamBattleTeam() const { return m_ShamBattleTeam; }
	inline int GetSelectChar() const { return m_select_char; }
	inline void SetSelectCharArray( int iSelectArray ){ m_select_char = iSelectArray; }
	int GetSelectClassType();

	__int64 GetPartyExp();
	int GetPartyLevel();
	__int64 GetLadderExp();
	int GetLadderLevel();
	int GetHeroExp();
	int GetHeroMatchPoint();
	int GetHeroTitle();
	ModeType GetPlayingMode();
	
public:
	LadderTeamParent  *GetMyLadderTeam();
	BattleRoomParent *GetMyBattleRoom();
	ShuffleRoomParent *GetMyShuffleRoom();
	DWORD GetMyBattleRoomIndex(){ return m_dwMyBattleRoom; }
	DWORD GetMyShuffleRoomIndex(){ return m_dwMyShuffleRoom; }
	Room *GetMyRoom() { return m_pMyRoom; }
	bool IsServerLobby(){ return m_bJoinServerLobby; }
	
	int GetMyVictories() { return m_iMyVictories; }
	void IncreaseMyVictories( bool bIncrease );

public:
	float GetModeConsecutivelyBonus();
	void  SetModeConsecutively( ModeType eModeType );
	inline int GetModeConsecutivelyCnt(){ return m_iModeConsecutivelyCnt; }

public:
	void SendAllCharInfo();
	void SendFreeDayEvent(DWORD dwPCRoom);

public:
	void ClearCharJoinedInfo();
	void SetCharJoined( int iArray, bool bJoined );
	bool IsCharJoined( int iArray );
	bool IsClassType( int iClassType );
	bool IsClassTypeExceptExercise( int iClassType );
	bool IsClassTypeExerciseStyle( int iClassType, int chExercise );
	bool IsBuyActiveChar( int iClassType );        //������ �뺴�� Active
	bool IsActiveChar( int iClassType );           //PC�� ü�� �뺴�� Active
	int  GetCharArrayByClass( int iClassType );
	int  GetExerciseCharArrayByClass( int iClassType );	
	void CheckCharSlot( ioCharacter *pChar );
	void IntegrityCharSlot();
	int  GetCharSlotIndexToArray( int iSlotIndex );
	bool IsCharPeriodTime( int iClassType );

public:
	// Monster Coin
	bool UseModeStartMonsterCoin( int iUseCnt, bool bUseGoldCoin = true);
	bool UseGoldMonsterCoin();
	bool UseRouletteCoin( const int iUseCount, const BOOL bOnlyCheck );
	int  GetEtcMonsterCoin();
	int  GetEtcGoldMonsterCoin();
	int  AddEtcMonsterCoin( int iAddCoin );
	int  AddRefillSeconds( DWORD dwAddSeconds );
	int  GetRefillSeconds();
	void CheckRefillMonsterCoin( DWORD dwProcessSec, bool bFirstSync = false );

	void SetBeforeMonsterCoin(int iCnt) { m_iBeforeMonsterCoin = iCnt; }
	int GetBeforeMonsterCoin() { return m_iBeforeMonsterCoin; }

public:
	float GetSoldierExpBonus( int iClassType );	

public:
	void SetUserData( const USERDATA &user_data, bool first_login_user );
	void SetUserHeroData( const UserHeroData &rkData );
	void SetUserRelativeGradeData( int iInitTime, bool bReward, int iBackupExp );
	void SetUserHeadquartersData( const UserHeadquartersOption &rkData );
	void SetFriendRecommendData( DWORD dwTableIndex, DWORD dwRecommendIndex );

	ioCharacter* AddCharDataToPointer();
	void SetPrivateID( const ioHashString &szID ) { m_user_data.m_private_id = szID; }
	void SetPublicID( const ioHashString &rszPublicID ) { m_user_data.m_public_id = rszPublicID; }

	/*
	ioHashString   m_Country;
	ioHashString   m_Gender;
	ioHashString   m_Cafe;
	ioHashString   m_Age;
	*/
	//[HRYOON����] ������ ���� ���� ���� ó��
	bool m_UserOutputStatus;
	void SetOutputStatus(bool userStatus ) { m_UserOutputStatus = userStatus; }
	bool GetOutputStatus() { return m_UserOutputStatus; }


	//hr �߰� ��ƾ
	void SetCountry( const ioHashString &szCountry ) { m_user_data.m_Country = szCountry; }
	void SetGender( const ioHashString &rszGender ) { m_user_data.m_Gender = rszGender; }
	void SetCafeLevel( const ioHashString &rszCafe ) { m_user_data.m_Cafe = rszCafe; }
	void SetAge( const ioHashString &rszAge ) { m_user_data.m_Age = rszAge; }
	
	void SetLatinConnTime( const ioHashString &rszTime ) { m_user_data.m_LatinConnTime = rszTime; }
	virtual void IncreaseGiveupCount() { m_user_data.m_iGiveUp++; }
	virtual int  GetGiveupCount() { return m_user_data.m_iGiveUp; }

	virtual void SetLogoutType( int type ) { m_user_data.m_iClientLogoutType = type; }
	virtual int  GetLogoutType() { return m_user_data.m_iClientLogoutType; }

//virtual const ioHashString& GetPrivateID() const { return m_user_data.m_private_id; }
	virtual const ioHashString& GetCountry() const { return m_user_data.m_Country; }
	virtual const ioHashString& GetGender() const { return m_user_data.m_Gender; }
	const ioHashString& GetCafeLevel() const { return m_user_data.m_Cafe; }
	const ioHashString& GetAge() const { return m_user_data.m_Age; }
	
	virtual const ioHashString& GetLatinConnTime() const { return m_user_data.m_LatinConnTime; }

 //hr EU
	const ioHashString& GetNexonEUID() const { return m_user_data.m_NexonEUID; }
	void SetNexonEUID( const ioHashString &szEUID ) { m_user_data.m_NexonEUID = szEUID; }	
	
	void SetNexonSN( __int64 iSN ) { m_user_data.m_iNexonSN = iSN; } 
	__int64 GetNexonSN() const { return m_user_data.m_iNexonSN; }

	void SetEUGender( int iGender ) { m_user_data.m_iEUGender = iGender; } 
	int GetEUGender() const { return m_user_data.m_iEUGender; }

	void SetEUAge( int iAge ) { m_user_data.m_iEUAge = iAge; } 
	int GetEUAge() const { return m_user_data.m_iEUAge; }

	/*void SetEUCountryType( const ioHashString& szType ) { m_user_data.m_dwEUContryType = szType ; } 
	const ioHashString& GetEUCountryType() const { return m_user_data.m_dwEUContryType; }*/
	virtual void SetEUCountryType( DWORD eType ) { m_user_data.m_dwEUContryType = eType; } 
	DWORD GetEUCountryType() const { return m_user_data.m_dwEUContryType; }
	
//hr US
	virtual void SetUSMemberIndex( DWORD eMemberIndex ) { m_user_data.m_dwUSMemberIndex = eMemberIndex; } 
	DWORD GetUSMemberIndex() const { return m_user_data.m_dwUSMemberIndex; }

	void SetUSMemberID( const ioHashString &szUSID ) { m_user_data.m_USMemberID = szUSID; }			//�������� �����
	const ioHashString& GetUSMemberID() const { return m_user_data.m_USMemberID; }
	
	virtual void SetUSMemberType( DWORD eMemberType ) { m_user_data.m_USMemberType = eMemberType; } 
	DWORD GetUSMemberType() const { return m_user_data.m_USMemberType; }


public:
	void SetTeam( TeamType eTeam );
	void SetShamBattleTeam( TeamType eTeam );

	void InitCharDie();
	void SetCharDie( bool bCharDie );

	bool ToggleExitRoomReserve();
	bool IsExitRoomDelay();
	void SetExitRoomDelay( DWORD dwExitTime );
	bool ExitRoomTimeOver();
	void SetExitPosition( bool bHeadquarters );
	void SetExperienceChar( SP2Packet &rkPacket );

	void ClearExitRoomReserve();
	bool IsExitRoomReserved() const { return m_bExitRoomReserved; }

	bool CheckPreRoomNum( ModeType eModeType );
	void ClearPreRoomNum();
	int GetPreRoomNum() { return m_PreRoomNum; }

    void BattleRoomKickOut( BYTE eType = (BYTE)RoomParent::RLT_NORMAL );
	void BattleRoomMyInfo();
	
	void LadderTeamKickOut();
	void LadderTeamMyInfo();

	void ShuffleRoomKickOut( BYTE eType = (BYTE)RoomParent::RLT_NORMAL );
	void ShuffleRoomMyInfo();

	bool IsUserShuffleRoomJoin();

	void UserVoteRoomKickOut( const ioHashString &rkReason );           //��ǥ�� �����

public:
	void SetGuildMarkChangeKeyValue( DWORD dwKeyValue ){ m_dwGuildMarkChangeKeyValue = dwKeyValue; }
	bool IsGuildUser( const ioHashString &rkName );

protected:
	bool AddNewItemToDBSlot( int iCharArray, DWORD dwItemCode, int iItemIndex  );
	
public:
	void FillConnectUserData( SP2Packet &rkPacket );
	void FillEquipItemData( SP2Packet &rkPacket );
	void FillJoinUserData( SP2Packet &rkPacket, bool bExperienceChar );

	void SetTransferAddress( ServerNode* &node, ioHashString &ipAddr, int &port );

	void FillChangeCharData( SP2Packet &rkPacket );
	void FillExperienceChar( SP2Packet &rkPacket );
	void FillFinalRoundResult( RoomStyle eRoomStyle, ModeType eModeType, SP2Packet &kPacket );
	void FillItemSlotData( SP2Packet &rkPacket );
	void FillClassData( int iClassType, bool bCurrentEquip, SP2Packet &rkPacket );
	void FillRoomAndUserInfo( SP2Packet &rkPacket );
	int  FillGradeNClassLevelUPBonus( SP2Packet &rkPacket, int iUpClassType, int iUpLevel );
	void FillToolTipInfo( SP2Packet &rkPacket );
	void FillSimpleToolTipInfo( SP2Packet &rkPacket );
	void FillTotalGrowthLevelDataByClassType( IN int iSelectClassType, OUT SP2Packet &rkPacket );
	void FillEquipMedalItem( SP2Packet &rkPacket );
	void FillEquipMedalItemByClassType( IN int iClassType, OUT SP2Packet &rkPacket );
	void FillExMedalSlotByClassType( IN int iClassType, OUT SP2Packet &rkPacket );
	void FillGrowthLevelData( SP2Packet &rkPacket );
	void FillGrowthLevelDataByClassType( int iClassType, SP2Packet &rkPacket );
	void FillHeroMatchInfo( SP2Packet &rkPacket );
	void FillUserCharListInfo( SP2Packet &rkPacket, int iStartArray, int iSyncCount );
	void FillUserCharSubInfo( SP2Packet &rkPacket, int iClassType );

public:
	void SetSelectCharDB( int iClassType );
	int SetPresentChar( int iClassType, int iLimitTime );
	int SetPresentEtcItem( int iEtcItemType, int iServerValue );
	int SetPresentExtraItem( int iExtraItemCode, int iExtraItemReinforce, int iExtraItemLimitDate, int iExtraItemPeriodType,
							 DWORD dwItemMaleCustom, DWORD dwItemFemaleCustom, int iMentType );
	int SetPresentExtraItemBox( int iExtraItemMachine, int iPesoArray, bool bCash );
	int SetPresentMedalItem( int iMedalItemType, int iMedalItemLimitTime );

	int SetPresentAlchemicItem( int iCode, int iCount );

	int SetPresentPetItem( int iPetCode, int iPetLevel, int iPetRank, DWORD dwIndex, DWORD dwSlotIndex );

	int SetPresentCostume( int iCode, int iLimitDate, DWORD dwPresentIndex, DWORD dwPresentSlotIndex );
	int SetPresentBonusCash( int iAmount, int iLimitDate, DWORD dwPresentIndex, DWORD dwPresentSlotIndex );
	int SetPresentAccessory( int iCode, int iLimitDate, DWORD dwPresentIndex, DWORD dwPresentSlotIndex );

	// ���� �̵�
public:
	void FillMoveData( SP2Packet &rkPacket );
	void FillInventoryMoveData( SP2Packet &rkPacket );
	void FillInventoryMoveData( SP2Packet &rkPacket, int iStartIndex );

	void FillExtraItemMoveData( SP2Packet &rkPacket );
	void FillExtraItemMoveData( SP2Packet &rkPacket, int iStartRow );

	void FillQuestMoveData( SP2Packet &rkPacket );
	void FillAlchemicInvenMoveData( SP2Packet &rkPacket );
	void FillPetMoveData( SP2Packet &rkPacket ); 
	void FillAwakeMoveData( SP2Packet &rkPacket ); 
	void FillSoldierMoveData( SP2Packet &rkPacket ); 
	void FillEtcItemMoveData( SP2Packet &rkPacket );
	void FillMedalItemMoveData( SP2Packet &rkPacket );
	void FillCostumeMoveData( SP2Packet &rkPacket );
	void FillMissionMoveData( SP2Packet &rkPacket );
	void FillRollBookMoveData( SP2Packet &rkPacket );
	void FillPersonalHQData( SP2Packet &rkPacket );
	void FillTimeCashDate( SP2Packet &rkPacket );
	void FillTitleMoveData( SP2Packet &rkPacket );
	void FillBonusCashData( SP2Packet &rkPacket );
	void FillAccessoryMoveData( SP2Packet &rkPacket );

	void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false );
	void ApplyInventoryMoveData( SP2Packet &rkPacket );
	void ApplyExtraItemMoveData( SP2Packet &rkPacket );
	void ApplyQuestMoveData( SP2Packet &rkPacket );
	void ApplyAlchemicInvenData( SP2Packet &rkPacket );
	void ApplyPetData( SP2Packet &rkPacket );
	void ApplyAwakeMoveData( SP2Packet &rkPacket );
	void ApplySoldierMoveData( SP2Packet &rkPacket );
	void ApplyEtcItemMoveData( SP2Packet &rkPacket );
	void ApplyMedalItemMoveData( SP2Packet &rkPacket );
	void ApplyCostumeMoveData( SP2Packet &rkPacket );
	void ApplyMissionMoveData( SP2Packet &rkPacket );
	void ApplyRollBookMoveData( SP2Packet &rkPacket );
	void ApplyPersonalHQMoveData(SP2Packet &rkPacket);
	void ApplyTimeCashDate( SP2Packet &rkPacket );
	void ApplyTitleMoveData(SP2Packet &rkPacket);
	void ApplyBonusCashData( SP2Packet &rkPacket );
	void ApplyAccessoryMoveData( SP2Packet &rkPacket );

	void FillExerciseIndex( SP2Packet &rkPacket );

public:  // �� ������� ���� ����ȭ
	void SendGradeSync();
	void SyncModeEtcItem( IN IntVec &rvModeEtcItemList, OUT IntVec &rvReturnEtcItem );

	//TCP
public:
	bool OnRoomProcessPacket( SP2Packet &rkPacket );
	bool OnBattleRoomProcessPacket( SP2Packet &rkPacket );
	bool OnLadderTeamProcessPacket( SP2Packet &rkPacket );

	void OnClose( SP2Packet &packet );
	void OnConnect( SP2Packet &packet );
	
	//HRYOON 20150102 �±� ��ū ��ȣȭ ��� ����
	void OnThailandTokenDecodeReq( SP2Packet &rkPacket );
	void OnMovingServer( SP2Packet &rkPacket );
	void OnJoinServerLobbyInfo( SP2Packet &rkPacket );

	void OnFriendList( SP2Packet &rkPacket );
	void OnFriendRequestList( SP2Packet &rkPacket );
	void OnFriendApplication( SP2Packet &rkPacket );
	void OnFriendCommand( SP2Packet &rkPacket );
	void OnFriendDelete( SP2Packet &rkPacket );

	void OnBestFriendInsert( SP2Packet &rkPacket );
	void OnBestFriendInsertFailed( SP2Packet &rkPacket );
	void _OnBestFriendDismiss( const ioHashString &rkFriendName, bool bResult );
	void OnBestFriendDismiss( SP2Packet &rkPacket );
	void OnBestFriendExceptionList( SP2Packet &rkPacket );

	void OnUserPosRefresh( SP2Packet &rkPacket );
	void OnUserLogin( SP2Packet &rkPacket );
	void OnRegisteredUser( SP2Packet &rkPacket );
	void OnDeleteFriendByWeb( SP2Packet &rkPacket );

	void _OnLeaderCreate( int iClassType );
	void OnCharCreate( SP2Packet &packet );
	void OnServiceChar( SP2Packet &rkPacket );
	void _OnCharDelete( int iCharArray );
	void OnCharDelete( SP2Packet &packet );
	void OnChangeLeaderChar( SP2Packet &rkPacket );
	int _OnSetMyRentalCharException( DWORD dwCharIndex );
	void OnSetMyRentalChar( SP2Packet &rkPacket );	
	void OnCharDisassemble( SP2Packet &packet );

	void OnJoinRoom( SP2Packet &packet );
	void OnDropItem( SP2Packet &rkPacket );
	void OnItemMoveDrop( SP2Packet &rkPacket );
	void OnPickItem( SP2Packet &rkPacket );
	void OnSearchPlazaRoom( SP2Packet &rkPacket );
	void OnPlazaRoomList( SP2Packet &rkPacket );
	void OnCreatePlaza( SP2Packet &rkPacket );
	
	void AutoCreatePlaza( SP2Packet &rkPacket );
	bool CreatePlazaInThisSvr( ioHashString& szPlazaName, ioHashString& szPlazaPW, int iMaxPlayer, int iSubIndex, int iPlazaType, bool bAuto );
	int CheckPlazaCreateDefaultErr();

	void SetPlazaInfoWithPacket( SP2Packet &rkPacket, ioHashString& szPlazaName, ioHashString& szPlazaPW, int& iMaxPlayer, int& iSubIndex, int& iPlazaType );
	void SetPlazaInfoWithDefault( int& iMaxPlayer, int& iSubIndex, int& iPlazaType );

	void OnPlazaCommand( SP2Packet &rkPacket );
	void OnPlazaInviteList( SP2Packet &rkPacket );
	void OnPlazaInvite( SP2Packet &rkPacket );

	void OnHeadquartersInviteList( SP2Packet &rkPacket );
	void _OnJoinHeadquarters( UserParent *pRequestUser, int iMapIndex, bool bInvited );
	void OnJoinHeadquarters( SP2Packet &rkPacket );
	void _OnHeadquartersInfo( UserParent *pRequestUser );
	void OnHeadquartersInfo( SP2Packet &rkPacket );
	void _OnHeadquartersJoinAgree( DWORD dwHeadquartersModeIndex );
	void OnHeadquartersJoinAgree( SP2Packet &rkPacket );
	
	void OnChangeSingleChar( SP2Packet &rkPacket );
	void OnChangeChar( SP2Packet &rkPacket );
	bool SetChangeChar( int iCharArray, bool bWait, int iSelectCharArray, DWORD dwCharChangeIndex );	

	void OnUseItem( SP2Packet &rkPacket );
	void OnBuyItem( SP2Packet &rkPacket );
	void OnEquipItem( SP2Packet &rkPacket );
	void OnIncreaseStat( SP2Packet &rkPacket );
	void OnInitStat( SP2Packet &rkPacket );

	void OnCharSlotChange( SP2Packet &rkPacket );

	void OnLogOut( SP2Packet &rkPacket );
	
	void OnBattleRoomReserveDelete( SP2Packet &rkPacket );
	void OnJoinBattleRoomList( SP2Packet &rkPacket );
	void OnCreateBattleRoom( SP2Packet &rkPacket );
	void _OnBattleRoomJoinResult( int iResultType, bool bMovePenalty, int iPenaltyPeso, bool bServerMove = false );
	void OnUserBattleRoomJoin( SP2Packet &rkPacket );
	void OnUserBattleRoomLeave( SP2Packet &rkPacket );
	void OnBattleRoomInviteList( SP2Packet &rkPacket ); 

	void OnUserShuffleRoomJoin( SP2Packet &rkPacket );
	void OnUserShuffleRoomJoinCancel( SP2Packet &rkPacket );
	void OnUserShuffleRoomLeave( SP2Packet &rkPacket );
	void _OnShuffleRoomJoinResult( int iResultType );
	void OnLevelUpItem( SP2Packet &rkPacket );
	
	void OnTutorialStep( SP2Packet &rkPacket );

	void OnCharLimitCheck( SP2Packet &rkPacket );
	void OnCharDecorationBuy( SP2Packet &rkPacket );
	int  _OnEtcItemMaxCheck( ioEtcItem* pEtcItem, ioUserEtcItem::ETCITEMSLOT &kEtcItemSlot, int iServerValue, bool bMortmain );
	DWORD _OnEtcItemSetting( ioEtcItem* pEtcItem, ioUserEtcItem::ETCITEMSLOT &kEtcItemSlot, int iServerValue, bool bMortmain );
	void OnEtcItemBuy( SP2Packet &rkPacket );
	void OnEtcItemUse( SP2Packet &rkPacket );
	void OnEtcItemMotionOption( SP2Packet &rkPacket );
	void OnEtcItemSell( SP2Packet &rkPacket );
	int _OnEtcItemPackageDecoInsert( SP2Packet &rkPacket, DWORD dwEtcItemType, bool bSoldierPackage = false );
	void OnEtcItemSoldierPackage( SP2Packet &rkPacket, DWORD dwEtcItemType, int iClassType, int eActiveFilter );
	void OnEtcItemDecorationPackage( SP2Packet &rkPacket, DWORD dwEtcItemType );
	void OnCharExtend( SP2Packet &rkPacket );
	void OnCharCharge( SP2Packet &rkPacket );
	void OnCharChangePeriod( SP2Packet &rkPacket );

	int _OnEtcItemPreSetDecoInsert( CHARACTER kCharInfo, ITEMSLOTVec vItemSlot, DWORD dwEtcItemType );
	bool OnEtcItemPreSetPackage( int iClassType, int iLimitTime, ITEMSLOTVec vItemSlot, DWORD dwEtcItemType, bool bPresent );

	void OnPartyUserInviteAnswer( SP2Packet &rkPacket );
	void OnRelayChat( SP2Packet &rkPacket );
	void OnBattleRoomInfo( SP2Packet &rkPacket );
	void OnServerLobbyChat( SP2Packet &rkPacket );
	void OnMyRoomServerChange( SP2Packet &rkPacket );
	void OnPlazaRoomInfo( SP2Packet &rkPacket );
	void OnTrial( SP2Packet &rkPacket );
	void OnHackQuiz( SP2Packet &rkPacket );
	void OnAbuseQuizStart( SP2Packet &rkPacket );
	void OnAbuseQuiz( SP2Packet &rkPacket );
	bool _OnFollowUser( UserParent *pRequestUser, int iUserPos, int iRoomIndex, int iLeaveRoomIndex, int iLeaveBattleRoomIndex, bool bDeveloper, int iAbilityLevel );
	void _OnBattleRoomFollow( BattleRoomParent *pBattleRoom, int iNextPos );
	void _OnLadderTeamFollow( LadderTeamParent *pLadderTeam, int iNextPos );
	void OnFollowUser( SP2Packet &rkPacket );
	void _OnUserPosIndex( UserParent *pRequestUser, bool bSafetyLevel, int iUserPos, int iPrevBattleIndex );
	void OnUserPosIndex( SP2Packet &rkPacket );
	void OnDeveloperMacro( SP2Packet &rkPacket );
	void OnExerciseCharCreate( SP2Packet &packet );
	void OnPresentTestSend( SP2Packet &rkPacket );

	void OnChannelCreate( SP2Packet &rkPacket );
	void OnChannelInvite( SP2Packet &rkPacket );
	int  _OnChannelInviteException( int iIndex );
	void OnChannelLeave( SP2Packet &rkPacket );
	void OnChannelChat( SP2Packet &rkPacket );
	void OnUserInfoRefresh( SP2Packet &rkPacket );
	void OnSimpleUserInfoRefresh( SP2Packet &rkPacket );
	void OnUserCharInfoRefresh( SP2Packet &rkPacket );
	void OnUserCharSubInfoRefresh( SP2Packet &rkPacket );
	void OnUserCharRentalRequest( SP2Packet &rkPacket );
	void _OnUserCharRentalAgree( UserParent *pOwnerParent, const ioHashString &rkOwnerID, DWORD dwOwnerDBAgentID, const CHARACTER &rkCharInfo, RentalData &rkRentalData );
	void OnUserCharRentalAgree( SP2Packet &rkPacket );
	void OnUserCharRentalTimeEnd( SP2Packet &rkPacket );
	void OnUserInfoExist( SP2Packet &rkPacket );
	void OnMemoMsg( SP2Packet &rkPacket );
	void OnOfflineMemoList( SP2Packet &rkPacket );
	void OnBankruptcyPeso( SP2Packet &rkPacket );
	void OnUserEntryRefresh( SP2Packet &rkPacket );
	void OnFirstChangeID( SP2Packet &rkPacket );
	void OnAbstract( SP2Packet &rkPacket );

	void OnGrowthLevelUp( SP2Packet &rkPacket );
	void OnGrowthLevelInit( SP2Packet &rkPacket );
	void OnFishingState( SP2Packet &rkPacket );
	void OnEtcItemAction( SP2Packet &rkPacket );
	void OnEtcItemSwitch( SP2Packet &rkPacket );
	void OnRouletteState( SP2Packet &rkPacket );

	void OnGuildRankList( SP2Packet &rkPacket );
	void OnGuildInfo( SP2Packet &rkPacket );
	void OnGuildUserList( SP2Packet &rkPacket );
	void OnGuildJoinerChange( SP2Packet &rkPacket );
	void OnGuildEntryApp( SP2Packet &rkPacket );
	void OnGuildEntryCancel( SP2Packet &rkPacket );
	void OnGuildEntryDelayMember( SP2Packet &rkPacket );
	void OnGuildEntryAgree( SP2Packet &rkPacket );
	void OnGuildEntryRefuse( SP2Packet &rkPacket );
	void _OnGuildInvitation( UserParent *pSendUser, DWORD dwGuildIndex, int iGuildMark, const ioHashString szGuildName );
	void OnGuildInvitation( SP2Packet &rkPacket );
	void OnGuildLeave( SP2Packet &rkPacket );
	void OnGuildTitleChange( SP2Packet &rkPacket );
	void OnGuildMasterChange( SP2Packet &rkPacket );
	void OnGuildPositionChange( SP2Packet &rkPacket );
	void OnGuildKickOut( SP2Packet &rkPacket );
	void _OnGuildMarkChange( DWORD dwGuildIndex, DWORD dwGuildMark, bool bBlock = false );
	void OnGuildChat( SP2Packet &rkPacket );
	void OnGuildMarkChangeKeyValue( SP2Packet &rkPacket );
	void OnGuildMarkChangeKeyValueDelete( SP2Packet &rkPacket );
	void OnGuildTitleSync( SP2Packet &rkPacket );
	void OnGuildExist( SP2Packet &rkPacket );
	void OnLadderTeamList( SP2Packet &rkPacket );
	void OnCreateLadderTeam( SP2Packet &rkPacket );
	void OnCreateHeroMatchLadderTeam( ioHashString szTeamName, ioHashString szPassword, int iladderMaxPlayer, int ijoinguildIndex, int imodeSelectType, bool mode );
	void OnJoinLadderTeam( SP2Packet &rkPacket );
	void OnLadderTeamJoinInfo( SP2Packet &rkPacket );
	void OnLadderTeamLeave( SP2Packet &rkPacket );
	void OnLadderTeamInviteList( SP2Packet &rkPacket );
	void OnLadderOtherTeamNameChange( SP2Packet &rkPacket );
	void OnLadderBattleRequestAgree( SP2Packet &rkPacket );
	void OnLadderUserHQMove( SP2Packet &rkPacket );
	void OnLadderTeamRanking( SP2Packet &rkPacket );
	void OnLadderTeamRankList( SP2Packet &rkPacket );

	void OnMovieControl( SP2Packet &rkPacket );
	void _OnSelectPresent( DWORD dwSelectCount );
	void OnPresentRequest( SP2Packet &rkPacket );
	void OnPresentRecv( SP2Packet &rkPacket );
	void OnPresentSell( SP2Packet &rkPacket );
	void OnPresentBuy( SP2Packet &rkPacket );
	void _OnDBPresentBuy( DWORD dwRecvUserIndex, int iRecvPresentCnt, const char *szRecvPrivateID, const char *szRecvPublicID, short iPresentType, int iBuyValue1, int iBuyValue2 );

	void OnVoiceInfo( SP2Packet &rkPacket );
	void OnProtectCheck( SP2Packet &rkPacket );

	// û��
	void OnSubscriptionRequest( SP2Packet &rkPacket );

	void OnSubscriptionBuy( SP2Packet &rkPacket );
	void OnSubscriptionRecv( SP2Packet &rkPacket );
	void OnSubscriptionRetr( SP2Packet &rkPacket );
	void OnSubscriptionRetrCheck( SP2Packet &rkPacket );

	bool CheckSubscriptionRetr( SP2Packet &rkPacket, DWORD dwIndex, const ioHashString& szSubscriptionID );
	bool SetSubscriptionRetr( DWORD dwIndex, const ioHashString& szSubscriptionID );

	void _OnSelectSubscription( DWORD dwSelectCount );
	void _OnDBSubscriptionBuy( DWORD dwRecvUserIndex, const char *szRecvPrivateID, short iPresentType, int iBuyValue1, int iBuyValue2 );

	//
	void OnGetCash( SP2Packet &rkPacket );
	void OnHoleSendComplete( SP2Packet &rkPacket );
	void OnUDPRecvTimeOut( SP2Packet &rkPacket );
	void OnEventDataUpdate( SP2Packet &rkPacket );
	void OnServerLobbyInfo( SP2Packet &rkPacket );
	void OnCampDataSync( SP2Packet &rkPacket );
	void OnCampChangePos( SP2Packet &rkPacket );
	void OnCampBattleEndLeaveTeam( SP2Packet &rkPacket );
	void OnAllItemDrop( SP2Packet &rkPacket );
	void OnChangeGangsi( SP2Packet &rkPacket );
	void OnGashaponList( SP2Packet &rkPacket );
	void OnExcavationCommand( SP2Packet &rkPacket );
	
	void OnServerAlarmMsg( SP2Packet &rkPacket );
	void OnControlKeys( SP2Packet &rkPacket );

	// ExtraItem
	BOOL IsWearingExtraItem( int iIndex, int iTargetClass );
	void PutonExtraItem( const int iCharArray, int iSlot, int iNewIndex );
	void TakeoffExtraItem( const int iCharArray, int iSlot );
	void OnExtraItemBuy( SP2Packet &rkPacket );
	void OnExtraItemChange( SP2Packet &rkPacket );
	void OnExtraItemSell( SP2Packet &rkPacket );
	void OnExtraItemDisassemble( SP2Packet &rkPacket );
	int	 GetEquipedClassWithExtraItem(int iItemIndex);

	// MedalItem
	void OnMedalItemSell( SP2Packet &rkPacket );

	// CheckBaseValue
	void OnCheckBaseValue( SP2Packet &rkPacket );

	void OnShutdownDate( SP2Packet &rkPacket );

	// Alchemic
	void OnAlchemicFunc( SP2Packet &rkPacket );

	// Pet
	bool FillEquipPetData( SP2Packet &rkPacket );
	bool OnPetEggUse( SP2Packet &rkPacket, bool &bCash, int &iType );
	void OnPetChange( SP2Packet &rkPacket );
	void OnPetSell( SP2Packet &rkPacket ); 
	void OnPetNurture( SP2Packet &rkPacket ); //�� ����
	void OnPetComPound( SP2Packet &rkPacket );
	void OnPetEquipInfo( SP2Packet &rkPacket );


	// Quest 
	void OnQuestOccur( SP2Packet &rkPacket );
	void OnQuestAttain( SP2Packet &rkPacket );
	void OnQuestAlarm( SP2Packet &rkPacket );
	void OnQuestReward( SP2Packet &rkPacket );
	bool _OnQuestDirectReward( int iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, int iPresentValue4, int iMentType );
	bool _OnQuestRewardPresent( const ioHashString &rkSendID, int iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, int iPresentValue4, 
								int iPresentState, int iPresentMent, int iPresentPeriod, bool bDirectPresent );
	void OnQuestAllDelete( SP2Packet &rkPacket );
	
	void OnPresentAllDelete( SP2Packet &rkPacket );

	void OnHeroTop100Data( SP2Packet &rkPacket );
	void OnHeroMatchOtherInfo( SP2Packet &rkPacket );

	void OnTradeCreate( SP2Packet &rkPacket );
	void OnTradeList( SP2Packet &rkPacket );
	void OnTradeItem( SP2Packet &rkPacket );
	void OnTradeCancel( SP2Packet &rkPacket );

	void OnEventShopGoodsList( SP2Packet &rkPacket );
	void _OnEventShopGoodsBuy( int iBuyState, SP2Packet &rkPacket );   // ���� �������� ���� ����
	void OnEventShopGoodsBuy( SP2Packet &rkPacket );    // Ŭ���̾�Ʈ���� ���� ����
	void OnEventShopState( SP2Packet &rkPacket );
	void OnEventShopBuyUserClear( SP2Packet &rkPacket );
	void OnEventNpcClose( SP2Packet &rkPacket );

	// billing msg
	void OnBillingGetCash( SP2Packet &rkPacket );
	void OnBillingOutputCash( SP2Packet &rkPacket );
	void OnBillingLogin( SP2Packet &rkPacket );
	void OnBillingRefundCash( SP2Packet &rkPacket );
	void OnBillingUserInfo( SP2Packet &rkPacket );
	void OnBillingAutoUpgradeLogin( SP2Packet &rkPacket );
	void OnBillingPCRoom( SP2Packet &rkPacket );
	void OnBillingAutoUpgradeOTP( SP2Packet &rkPacket );
	void OnBillingGetMileage( SP2Packet &rkPacket );
	void OnBillingAddMileage( SP2Packet &rkPacket );
	void OnBillingIPBonus( SP2Packet &rkPacket );
	void OnBillingAddCash( SP2Packet &rkPacket );
	void OnBillingFillCashUrl( SP2Packet &rkPacket );
	void OnBillingTimeoutBillingGUID( SP2Packet &rkPacket );
	void OnBillingDecodeGarenaToken( SP2Packet &rkPacket );
	void OnBillingGarenaWebEvent( SP2Packet &rkPacket );

	// û��öȸ
	void OnBillingSubscriptionRetractCheck(SP2Packet& rkPacket );
	void OnBillingSubscriptionRetract(SP2Packet& rkPacket );

	//���Ǽ��� ��Ʈ�� 
	void OnSessionControl( SP2Packet& rkPacket );

	void OnNexonTerminateMessage( SP2Packet& rkPacket, int controlType );

	// autoupgrade msg
	void OnAutoUpgradeLogin( SP2Packet &rkPacket );
	void OnAutoUpgradeOTP( SP2Packet &rkPacket );

	void OnMedalItemChange(SP2Packet &rkPacket);

	void OnNProtectCheck( SP2Packet &rkPacket );

	void OnRoomStealthEnter( SP2Packet &rkPacket );
	void OnCampSeasonBonus( SP2Packet &rkPacket );
	void OnCustomItemSkinUniqueIndex( SP2Packet &rkPacket );
	void OnCustomCostumeSkinUniqueIndex( SP2Packet &rkPacket );
	void OnCustomItemSkinDelete( SP2Packet &rkPacket );
	void OnCustomCostumeSkinDelete( SP2Packet &rkPacket );

	void OnGetMileage( SP2Packet &rkPacket );

	void OnHeadquartersOptionCmd( SP2Packet &rkPacket );
	void OnHeadquartersCommand( SP2Packet &rkPacket );

	void OnDisconnectAlreadyID( SP2Packet &rkPacket );
	void OnPCInfo( SP2Packet &rkPacket );

	void OnBuySelectExtraGashapon( SP2Packet &rkPacket );

	//
	void OnTournamentRegularRequest( SP2Packet &rkPacket );
	void OnTournamentMainInfo( SP2Packet &rkPacket );
	void OnTournamentListRequest( SP2Packet &rkPacket );
	void OnTournamentTeamCreate( SP2Packet &rkPacket );
	void OnTournamentTeamInfo( SP2Packet &rkPacket );
	void OnTournamentTeamUserList( SP2Packet &rkPacket );
	void OnTournamentTeamInvitation( SP2Packet &rkPacket );
	void OnTournamentTeamEntryDelayMember( SP2Packet &rkPacket );
	void OnTournamentTeamEntryApp( SP2Packet &rkPacket );
	void OnTournamentTeamEntryRefuse( SP2Packet &rkPacket );
	void OnTournamentTeamEntryAgree( SP2Packet &rkPacket );
	void OnTournamentTeamLeave( SP2Packet &rkPacket );
	void OnTournamentScheduleInfo( SP2Packet &rkPacket );
	void OnTournamentRoundTeamData( SP2Packet &rkPacket );
	void OnTournamentRoomList( SP2Packet &rkPacket );
	void OnTournamentTeamAllocateList( SP2Packet &rkPacket );
	void OnTournamentTeamAllocateData( SP2Packet &rkPacket );
	void OnTournamentJoinConfirmCheck( SP2Packet &rkPacket );
	void OnTournamentJoinConfirmRequest( SP2Packet &rkPacket );
	void OnTournamentJoinConfirmCommand( SP2Packet &rkPacket );
	void OnTournamentAnnounceChange( SP2Packet &rkPacket );
	void OnTournamentTotalTeamList( SP2Packet &rkPacket );
	void OnTournamentCustomStateStart( SP2Packet &rkPacket );
	void OnTournamentCustomRewardList( SP2Packet &rkPacket );
	void OnTournamentCustomRewardBuy( SP2Packet &rkPacket );
	void OnTournamentCheerDecision( SP2Packet &rkPacket );	
	void OnTournamentCheerDecisionOK( SP2Packet &rkPacket );

	// s -> c
	void CloverRefill( CTime& CurrentTime );	// ����
	void GiftCloverInfo( const int iType );
	void CloverFriendInfo( const DWORD dwFriendIndex, const int iType );

	// c -> s
	void OnCloverChargeReq( SP2Packet& kPacket );		//! ����
	void OnCloverSendReq( SP2Packet& kPacket );			//! ������
	void OnCloverReceiveReq( SP2Packet& kPacket );		//! �ޱ�
	void OnCloverReceiveDelete( SP2Packet& kPacket );	//! �Ⱓ���� Ŭ�ι� ����.

	void OnBingoStart( SP2Packet& kPacket );
	void OnBingoNumberInitialize( SP2Packet& kPacket );
	void OnBingoALLInitialize( SP2Packet& kPacket );
	void OnBingoData( BYTE (*pArrayNumber)[ ioBingo::MAX ], BYTE* pArrayPresent, SP2Packet& rkPacket, const int iRollingType );

	void OnSendSuperGashponPackage( DWORD dwEtcItemType, DWORD dwPackageIndex, int iUseType );
	void OnSendSuperGashponSubPackage( DWORD dwEtcItemType, int iUseType );
	void OnSendSuperGashponLimitInfo(  DWORD dwEtcItemType, DWORD dwLimit );

	void OnFillCashUrl( SP2Packet& kPacket );

	void OnPirateRouletteInfoRequest( SP2Packet& kPacket );
	void OnPirateRouletteUseSwordRequest( SP2Packet& kPacket );
	void OnPirateRouletteResetRequest( SP2Packet& kPacket );	
	//�ؿ� �߰� ��Ŷ
	void OnSearchRoom( SP2Packet &rkPacket );
	void OnJoinLadderRoomList( SP2Packet &rkPacket );
	void OnLadderRoomInfo( SP2Packet &rkPacket );
	void OnLadderRoomObserverJoin( SP2Packet &rkPacket );
	void ExitRoomToLobby();

protected:
	// billing msg
	void _OnBillingOutputCashSoldier( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID, int& iBuyType, int& iValue1, int& iValue2 );
	void _OnBillingOutputCashDeco( SP2Packet &rkPacket, int iReturnItemPrice, int iPayAmt, int iTransactionID, int& iBuyType, int& iValue1, int& iValue2 );
	void _OnBillingOutputCashSoldierExtend( SP2Packet &rkPacket, int iReturnItemPrice, int iPayAmt, int iTransactionID );
	void _OnBillingOutputCashEtc( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID, int& iBuyType, int& iValue1, int& iValue2 );
	void _OnBillingOutputCashSoldierChangePeriod( SP2Packet &rkPacket, int iReturnItemPrice, int iPayAmt, int iTransactionID );
	void _OnBillingOutputCashExtra( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID, int& iBuyType, int& iValue1, int& iValue2 );
	void _OnBillingOutputCashPresent( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID );
	void _OnBillingOutputCashSubscription( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID, ioHashString szSubscriptionID = "" );
	void _OnBillingOutputCashCostume( SP2Packet &rkPacket, int iReturnItemPrice,int iPayAmt, int iTransactionID, int& iBuyType, int& iValue1, int& iValue2 );
	void _OnBillingOutputCashPopup( SP2Packet &rkPacket, int iReturnItemPrice, int iPayAmt, int iTransactionID );

protected:
	bool CheckHackCount( DWORD dwCurGap );
	void PrintHackLog( DWORD dwCurGap );

	//UDP
public:
	void OnUDPConnect( SP2Packet &rkPacket );
	void _OnCheckPingStep( DWORD dwClientTime );
	void OnSyncTime( SP2Packet &rkPacket );
	void OnReserveRoomJoin( SP2Packet &rkPacket );
	void OnWebEvent( SP2Packet &rkPacket );
	void OnWebRefreshBlock( SP2Packet &rkPacket );
	void OnWebGetCash( SP2Packet &rkPacket );
	void OnWebRefreshUserEntry( SP2Packet &rkPacket );
	
	void OnCheckKingUserPing( SP2Packet &rkPacket );

public:
	void  SetStartTimeLog(DWORD dwStartTimeLog);
	DWORD GetStartTimeLog();
	inline const ioHashString& GetGUID() const { return m_szGUID; }
	EventUserManager& GetEventUserMgr();

	EntryType GetEntryType() const;
	bool IsEntryFormality() const;
	void SetEntryType( EntryType eEntryType );

	void ImmediatelyEquipItem( ioItem *pItem, const ioHashString& szItemName, int eObjectCreateType, SP2Packet &rkPacket );
	void SetDefaultDecoItem( const CHARACTER &rkCharInfo );

private:
	void DeleteCharData( int iCharArray );
	void FixSelectChar();
	bool IsDeleteExerciseChar( Room *pRoom, byte chExerciseType ); 
	bool IsBuyExerciseChar( int iExerciseStyle );

	bool IsCanBuyItem( const ioSetItemInfo *pSetItemInfo );
	bool IsCanBuyItemBySameGradeLevel( const ioSetItemInfo *pSetItemInfo );

	void DeleteGuildMarkChangeKeyValue();

	ITEMSLOT GetItemSlot( int iDecoType, const CHARACTER &rkCharInfo );
	RaceDetailType GetRaceDetailType( const CHARACTER &rkInfo );

	int  GetFriendSlotSize();

public:
	bool DeleteExerciseChar( byte chExerciseStyle );
	bool DeleteExercisePCRoomChar( DWORD dwCharIdx );

	void DeleteExerciseRentalChar( int iCharArray, bool bReturnPacket );
	void DeleteExerciseRentalCharAll();
	void CheckExerciseRentalCharDeleteTime();
	void BuyRentalCharacterProcess( ioCharacter *pCharacter );

public:
	inline bool IsBillingWait() { return !m_szBillingGUID.IsEmpty(); }
	inline const ioHashString& GetBillingGUID() const { return m_szBillingGUID; }
	void SetBillingGUID( const char* szGUID );
	void ClearBillingGUID();

	void StartEtcItemTime( const char *szFunction, int iType = 0/*ioEtcItem::EIT_NONE*/ ); // ioEtcItem.h �߰����� �ʱ� ���ؼ�
	void UpdateEtcItemTime( const char *szFunction, int iType = 0/*ioEtcItem::EIT_NONE*/ );
	void LeaveRoomEtcItemTime(); 
	void DeleteEtcItemPassedDate();

	bool IsNewPublicID() const { return !m_szNewPublicID.IsEmpty(); }
	void SetNewPublicID( const char *szNewPublicID ) { m_szNewPublicID = szNewPublicID; }
	void SetPublicIPForNewPublicID( const char *szPublicIPForNewPublicID) { m_szPublicIPForNewPublicID = szPublicIPForNewPublicID; }

	inline const ioHashString& GetBillingUserKey() const { return m_szBillingUserKey; }
	inline void SetBillingUserKey( const ioHashString &rszBillingUserKey ) { m_szBillingUserKey = rszBillingUserKey; }

	void SendEventData();

	// Growth
	void OnCheckTimeGrowth( SP2Packet &rkPacket );
	void OnAddTimeGrowth( SP2Packet &rkPacket );
	void OnRemoveTimeGrowth( SP2Packet &rkPacket );

	void CheckTimeGrowth();

	void ClearKingPingCnt();
	inline int GetCurKingPingCnt() { return m_iCurRecvKingPingCnt; }

	// Extend CharSlot
	inline int GetCurMaxCharSlot() { return m_iCurMaxCharSlot; }
	void CheckCurMaxCharSlot();

	// ExtraItem
	int FindExtraItemEquipChar( int iSlotIndex );

	void DeleteExtraItemPassedDate( bool bImmediately );

	// Item Compound
	bool OnItemCompound( SP2Packet &rkPacket, DWORD dwType );
	bool OnItemMaterialCompound( SP2Packet &rkPacket, DWORD dwType );
	bool OnMultipleItemCompound( SP2Packet &rkPacket, DWORD dwType );

	// Item GrowthCatalyst
	bool OnItemGrowthCatalyst( SP2Packet &rkPacket, DWORD dwType );

	bool OnItemCompoundEx( SP2Packet &rkPacket, DWORD dwType, int iRandValue );

	void SetFirstChangeID(bool bFirstChangeID) { m_bFirstChangeID = bFirstChangeID; }

	// Expand Medal
	bool OnExpandMedalSlotOpen( SP2Packet &rkPacket, DWORD dwType );
	void DeleteExMedalSlotPassedDate();

	//Plaza
	void OnSpawnNpcInPlaza(SP2Packet &rkPacket);

	//SuperGashapon
	void OnSuperGashaponGetAll(SP2Packet &rkPacket);
	void OnSuperGashaponGet(SP2Packet &rkPacket);
	void OnSuperGashaponInfoGet(SP2Packet &rkPacket);
	void OnSuperGashaponInfoReset(SP2Packet &rkPacket);

	//
	void OnMacroAttendanceAddDay(SP2Packet &rkPacket);
	void OnMacroAttendancePrevMonth(SP2Packet &rkPacket);
	void OnMacroAttendanceDateModify(SP2Packet &rkPacket);

public:
	bool IsFishingState();
	bool CheckFishingInfo();

	int GetFishingLevel();
	int GetFishingExpert();
	bool AddFishingExp( int iExp );
	
	int GetFishingSlotExtendItem();
	int GetFishingExtraType( int iEtcType );

	int GetFishingRodType();
	int GetFishingBaitType();

	bool HasEtcItem( int iEtcType );

	int ChanceMortmainCharEventLotto( int &iMortmainCnt );

public:
	void  SetEquipExcavating( bool bEquip, int iEtcItemType );

	bool  IsRealExcavating();
	DWORD GetExcavatingTime() const { return m_dwExcavatingTime; }
	void  SetExcavatingTime( DWORD dwExcavatingTime ) { m_dwExcavatingTime = dwExcavatingTime; }

	DWORD GetTryExcavatedTime() const { return m_dwTryExcavatedTime; }
	void  SetTryExcavatedTime(DWORD dwTryExcavatedTime) { m_dwTryExcavatedTime = dwTryExcavatedTime; }

	int  GetExcavationLevel();
	int  GetExcavationExp();
	bool AddExcavationExp( int iExp );

	void SetAllCharAllDecoration();

	void SetDBAgentID( DWORD dwDBAgentID ) { m_dwDBAgentID = dwDBAgentID; }

public:
	void DeleteMedalItemPassedDate( bool bImmediately );
	bool SendBillingAddMileage( int iPresentType, int iPresentValue1, int iPresentValue2, int iResellPeso, bool bPresent );

protected:
	void EquipSlotItem( SP2Packet &rkPacket );
	void ReleaseSlotItem( SP2Packet &rkPacket );

	void SendBilligLogOut();
	void SavePlayTimeLog();
public:
	inline void SetNeedSendPushStruct( bool bNeed ) { m_bNeedSendPushStruct = bNeed; }
	inline bool IsNeedSendPushStruct() const { return m_bNeedSendPushStruct; }

	inline void SetNeedSendPushStructIndex( int iIndex ) { m_iNeedSendPushStructMaxIndex = iIndex; }
	inline int GetNeedSendPushStructIndex() const { return m_iNeedSendPushStructMaxIndex; }

	inline void SetSendPushStructIndex( int iIndex ) { m_iSendPushStructIndex = iIndex; }
	inline int GetSendPushStructIndex() const { return m_iSendPushStructIndex; }

	inline void SetSendPushStructCheckTime( DWORD dwTime ) { m_dwSendPushStructCheckTime = dwTime; }
	inline DWORD GetSendPushStructCheckTime() const { return m_dwSendPushStructCheckTime; }

public:
	void AddPresentMemory( const ioHashString &szSendName, short iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, 
						   int iPresentValue4, short iPresentMent, CTime &rkLimitTime, short iPresentState );
	void SendPresentMemory();
	void LogoutMemoryPresentInsert();     // ���� �α� �ƿ��ÿ� ����

	// û��
	void AddSubscriptionMemory( const ioHashString &szSubscriptionID, int iSubscriptionGold,
								short iPresentType, int iPresentValue1, int iPresentValue2,
								CTime &rkLimitTime );
	void SendSubscriptionMemory();
	void LogoutMemorySubscriptionInsert();		// ���� �α� �ƿ��ÿ� ����
public: //���Ǽ��� ����
	void SendSessionLogin();
	void SendSessionLogout();

public: // InsertPlayTimeLOG
	void InsertLoginRecord(int recordType = 0);//LogDBClient::RCT_LOGIN
	void InsertLoginPcRoom(int recordType = 0);
	void InsertLogoutRecord();

public:
	bool IsShutDownUser() const { return m_bShutDownUser; }
	void SetShutDownUser(bool bShutDownUser) { m_bShutDownUser = bShutDownUser; }

	ioUserSelectShutDown &GetUserSelectShutDown();

public:
	DWORD GetFriendRecommendTableIndex(){ return m_FriendRecommendData.m_dwTableIndex; }
	
	bool GetPeerIP(char* remoteIP, const int size, int& remotePort); // �ӽ�

	void EventProcessTime();

public:
	void UpdateRelativeGrade( DWORD dwUniqueCode );

public: //kyg Subscription ���� 
	void SetSubscriptionRetract(const DWORD dwIndex, const ioHashString& szSubscriptionID, int iGold, SP2Packet& kPacket, bool bDefaultGold=false );
	void SetRetractGold( const DWORD dwIndex, const ioHashString& szSubscriptionID, int iGold );
	
	int GetSubscriptionGold( const DWORD dwIndex, const ioHashString& szSubscriptionID );
	int GetRetractGold( const DWORD dwIndex, const ioHashString& szSubscriptionID );
	int GetDBRetractGold( const DWORD dwIndex, const ioHashString& szSubscriptionID );

public:
	void SendShutDown(SP2Packet& rkPacket);
	void SendSelectUserShutDown(SP2Packet& rkPacket);
	void OnNexonPCRoom(SP2Packet& rkPacket);

public:
	void OnPcRoomCharCreate( SP2Packet& rkPacket );

	ioCharacter* GetPCRoomChar( int iClassType );
	ioCharacter* CreatePCRoomChar( int iClassType );

	void AllocPCRoomChar( const PcRoomCharInfoVec& InfoVec );

	void DeletePCRoomCharBySlot();
	void DeleteExpiredChar( const PcRoomCharInfoVec& InfoVec );
	
public:
	void OnAttedanceCheck( SP2Packet& rkPacket );

public:
	void ClearSelectGashapon();
	const SelectGashaponValueVec& GetSelectGashapon();

	void OnBuySelectGashapon( SP2Packet &rkPacket );

//ĳ���� ����
protected:
	std::map < int, int > m_CharAwakeDataMap; // < ĳ����Index, ���� �ð� >
	bool SetCharDateType(CHARACTER &kCharInfo);		// �Ⱓ�� �뺴�� limitTime �� date �������� ���� (0403201720) int �� 14�������� 04�� ǥ��.

public:
	void EraseCharAwakeDataMap( std::map < int, int >::iterator &iter, std::map< int, BYTE > &mAwakePassedDateMap );

	int GetCalcCharAwakeEndDate( int iAddDay );
	int GetCalcCharAwakeExtendDate( int iAddDay, int iEndDate );

	void SendAwakeFailPacket( BYTE chFailType );
	void SendAwakeExtendFailPacket( BYTE chFailType );

	void DeleteCharAwakePassedDate( );
	//void DeleteLoginAwakePassedDate( CHARACTER &char_data, const int iCharIndex, std::map<int, BYTE> & mAwakeMap);
	void SendCharAwakeEndPacket( std::map< int, BYTE > &mAwakeMap );

	void OnCharAwake( SP2Packet &rkPacket ); 
	void OnCharAwakeExtend( SP2Packet &rkPacket );

	void InsertAwakeDataMap( int iCharIndex, int iAwakeEndDate );

	bool CheckAwakePassedDate( int iAwakeEndDate ); 
	int GetINTtypeCurDate();

	//////Test
	void OnTestAwakeTimeSet( SP2Packet &rkPacket ); 

#if defined( SRC_OVERSEAS )
	// �Ϲ� ��í ��Ű�� ���� ������ (��ũ��)		JCLEE 140718
	void OnGetGashaAll( SP2Packet &rkPacket ); 
#endif

public:
	bool IsRightAwakeChange( const int iCharArray, const int iAwakeType );
	bool CharAwakeErrorCheck( int iCharArray, int iAwakeType, int iAwakeDay, int &iMaterialCode, int &iMaterialNeedCount );
	bool CharAwakeExtendErrorCheck( int iCharArray, int iAwakeDay, int &iMaterialCode, int &iMaterialNeedCount );
public:
	inline int GetUserIPType() { return m_iUserIPType; }

public:
	void AfterLoginProcess( bool bMove=false );
	void BeforeLogoutProcess();

public:
	//���� �̵��� 100���� ���̺� �� �����ϱ� ���ؼ� ȣ��( �ѹ��� ġ�� 2õ���� ���� )
	int GetDecoPageCount( );
	int GetExtraItemPageCount();

	//Ŭ�� �󿡼� �Ⱥ��̴� ���� ���� ����
	void OnPlayingUserDataInfo( SP2Packet &rkPacket ); 
	void FillUserCharInfo( SP2Packet &kPacket );

	void ModeExitRoom(bool bMovePenalty, int iPenaltyPeso);
	void SetRoomEntryType( bool bServerMove, BYTE& byEntryType );

	void LeaveAllRoom();

	//recv �ð� ����
	bool IsEnableInterval(const int ID, const DWORD dwInterval);

	//�뺴��ȭ
	void TakeOffCharExtraItem(ioCharacter* pCharacter, int iEquipType);	//�������� ������ ����

	void CustomItemSkinDelete(int iSlotIndex, BYTE byDeleteType);
	void CustomCostumeSkinDelete(int iSlotIndex, BYTE byDeleteType);
	void PowerUpItemSkinDelete( ioUserExtraItem::EXTRAITEMSLOT& kSlot );
	
	//�ڽ�Ƭ
	void OnCostumeBuy(SP2Packet &rkPacket );
	void OnCostumeChange( SP2Packet &rkPacket );
	void OnCostumeSell( SP2Packet &rkPacket );
	void OnCostumeDisassemble( SP2Packet &rkPacket );
	void DeleteCostumePassedDate();
	ioCharacter* FindCostumeEquipChar(int iCostumeIndex);
	void AddCostumeItem(int iIndex, DWORD dwCode, int iPeriodType, SYSTEMTIME &sysTime);
	void AddCostumeItem(int iIndex, DWORD dwCode, int iPeriodType, int iValue1, int iValue2);

	//�Ǽ��縮
	void OnAccessorySell( SP2Packet &rkPacket );
	void OnAccessoryChange(SP2Packet &rkPacket );
	void AddAccessoryItem(int iIndex, DWORD dwCode, int iPeriodType, SYSTEMTIME &sysTime);
	void AddAccessoryItem(int iIndex, DWORD dwCode, int iPeriodType, int iValue1, int iValue2, int iValue3);
	void DeleteAccessoryPassedDate();
	ioCharacter* FindAccessoryEquipChar(int iAccessoryIndex);

	//����
	void CashItemBuyProcess(int iITemType, IntVec& vDataValue, DWORD dwPacketID);
	void PesoItemBuyProcess(int iITemType, int iCode, int iArray, DWORD dwPacketID);

	void BuyCostumeWithPeso(int iCode, int iPeso, int iPeriod );

	int GetItemCashValue( int iItemType, IntVec& vDataValue );
	int GetItemBuyPesoValue( int iItemType, int iCode, int iArray );
	int GetItemBuyPeriodValue( int iItemType, int iCode, int iArray );

	int GetOutPutType(int iItemType);
	bool SetItemBuyVariableValue(int iItemType, IntVec& vData, SP2Packet &rkPacket);

	//Ư�� ��ǰ ���� ó��
	void SendSpeialGoodsBuyResult(int iResult, int iItemType);
	void SpecialGoodsBuy(DWORD dwEtcType);
	void SpecialGoodsPresent(const int iPresentType, const int iBuyValue1, const int iBuyValue2, ioHashString szReceiverPublicID);

	void _OnSpecialGoodsBuy(int iBuyState, SP2Packet &rkPacket );
	bool SpecialGoodsCashProcess(int iItemType);
	void OnSpecialShopGoodsList(SP2Packet &rkPacket );

	void PresendCashProcess(const int iPresentType, const int iBuyValue1, const int iBuyValue2, ioHashString szReceiverPublicID);

	//�̼� ����
	void OnMissionInfoRequest( SP2Packet &kPacket );
	void OnMissionCompensationRecv( SP2Packet &kPacket );
	void OnMissionTimeCheck( SP2Packet &kPacket );
	DWORD GetUserLoginTime() { return (DWORD)m_user_data.m_login_time.GetTime(); }
	void DoAdditiveMission(int iCount, int iUseType);
	
	void OnTestMissionSetDate( SP2Packet &kPacket );
	void OnTestMissionSetValue( SP2Packet &kPacket );

	//�⼮��
	void SetUserRollBookType(int iType);
	void OnRollBookRenewal( SP2Packet &rkPacket );
	void OnTestRollBookProgress( SP2Packet &rkPacket );

	//��� �⼮
	void OnGuildAttend( SP2Packet &rkPacket );
	void OnRecvGuildAttendReward(SP2Packet &rkPacket);
	void SendGuildAttendReward(int iCount);
	void GetAttendStandardDate(SYSTEMTIME& sYesterDay, SYSTEMTIME& sToday);
	void OnRenewalGuildMemberAttendInfo(SP2Packet &rkPacket);

	//���� �ϰ� �ִ��� ���������� üũ
	bool DoHaveAItem(const int iType, const DWORD dwCode, bool bPermanentItem, int iSearchingType);
	bool IsExistMedalItem(const DWORD dwCode, bool bPermanentItem, int iSearchingType);
	bool IsExistCostumeItem(const DWORD dwIndex, bool bPermanentItem, int iSearchingType);
	bool IsExistMedalItemAsCount(const DWORD dwCode,  bool bPermanentItem, int iSearchingType, int iCount);

	bool DeleteItem(int iType, DWORD dwCode);

	void OnDaumAdultCheckRsp(SP2Packet &rkPacket );

	void LogCashItemBuyInfo(const int iItemPrice, const int iPreTotalCash, const int iPreRealCash, const int iCurTotalCash, const int iCurRealCash, const int iItemType, const int iItemCode, const int iItemCount);

public:
	User( SOCKET s=INVALID_SOCKET, DWORD dwSendBufSize=0, DWORD dwRecvBufSize=0 );
	virtual ~User();

public:
	// ��ƾ�� �Ⱓ�� �뺴�� ���� ���� ���� ������ �����صδ� ���� (DB �����)	JCLEE 140416
	int	GetLimitSecForLatin(){ return m_iLimitSecForLatin; }

	

	//popup store
	bool m_bOnPopup;
	DWORD m_dwTotalMoney;
	DWORD m_dwMonthMoney;
	DWORD m_dwTotalPlayTime;
	std::vector<int> m_vecUsePopupIndex;	// ������ �˾� ����Ʈ
	std::vector<int> m_vecSendPopupIndex;		// ���� �˾� ����Ʈ
	
	//int m_iSendPopupIndex;
	void OnConnectPopupProcess();
	void CheckPopupStoreIndex();
	void SetUserSpentMoney( DWORD dwTotal, DWORD dwMonth, DWORD dwTotalPlayTime );
	void SetUserPopupIndex( std::vector<int>& vecPopupIndex );

	void AddUsePopupIndex( int iBuyPopupIndex );
	void OnPopupItemBuy(SP2Packet &rkPacket );


	// ���� ���� ġ�� ���� ����
	bool m_bUsedCoin;

	// rising gashapon
	int m_nRisingBuyCount;
	std::vector<int> m_vRisingGetIndex;
	void OnBuyRisingGashapon(SP2Packet &rkPacket );
	void OnInitRisingGashapon(SP2Packet &rkPacket );
	
	void InitRisingBuyCount() { m_nRisingBuyCount = 0; }
	int GetRisingBuyCount() const { return m_nRisingBuyCount; }
	void IncreaseRisingBuyCount() { m_nRisingBuyCount++; }

	std::vector<int> GetRisingGetIndex() const { return m_vRisingGetIndex; }
	void AddRisingGetIndex(int index) { m_vRisingGetIndex.push_back(index); IncreaseRisingBuyCount(); }
	void InitRisingGetIndex() { m_vRisingGetIndex.clear(); InitRisingBuyCount(); }

	int GetCountRSoldier();

	bool IsRSoldier(ioCharacter *pChar);

	void OnBillingTimeCashResult(SP2Packet &rkPacket );
	void _OnBillingPresentCash(SP2Packet &rkPacket);

	int GetCountOfSpecialSoldier(int iSpecialType);
	bool IsSpecialSoldier(ioCharacter *pChar, int iSpecialType, BOOL bCheckMortmain = TRUE );
	int GetSpecialSoldierType(int iClassType);
	int GetStartNumOfSpecialSoldier(int iSpecialType);
	int GetEndNumOfSpecialSoldier(int iSpecialType);

public:
	void EnterGuildRoom(const DWORD dwRoomIndex, const bool bInvite);

	void OnEnterGuildRoom(SP2Packet &kPacket );
	void OnConstructBlock(SP2Packet &kPacket );
	void OnRetrieveBlock(SP2Packet &kPacket );
	
	void ResultGuildRoomReq(SP2Packet &kPacket );

	void CreateGuildRoom(DWORD dwMapIndex);
	void SetPlazaInfoWithGuild(int& iMaxPlayer, int& iPlazaType);

	void OnConstructMode( SP2Packet &rkPacket );
	void OnSendGuildInvenInfo( SP2Packet &rkPacket );

	void OnTestCheckHousingInfo( SP2Packet &rkPacket );

	void SetFisheryBlockInfo(int val) { m_dwFisheryCode = val; }
	DWORD GetGuildFisheryCode() { return m_dwFisheryCode; }

	void ActiveUserGuildRoom();

	//���� ����
	void OnJoinPersonalHQ(SP2Packet &rkPacket);
	void _OnJoinPersonalHQ( UserParent *pRequestUser, int iMapIndex, bool bInvited );
	
	void OnPersonalHQInviteList( SP2Packet &rkPacket );

	void OnPersonalHQJoinAgree( SP2Packet &rkPacket );
	void _OnPersonalHQJoinAgree( DWORD dwRoomIndex );

	void OnPersonalHQInfo( SP2Packet &rkPacket );
	void _OnPersonalHQInfo( UserParent *pRequestUser );

	void OnPersonalHQCommand( SP2Packet &rkPacket );

	void GuildConstructMode(BYTE byType);
	void PersonalHQConstructMode(BYTE byType);

	void GuildConstructBlock(SP2Packet &kPacket);
	void PersonalHQConstructBlock(SP2Packet &kPacket);

	void GuildRetrieveBlock(SP2Packet &kPacket);
	void PersonalHQRetrieveBlock(SP2Packet &kPacket);

	void AddPersonalInvenItem(const DWORD dwItemCode, const int iCount);
	void DecreasePersonalInvenItem(const DWORD dwItemCode, const int iCount);

	void OnReqPersonalHQInvenData(SP2Packet &kPacket);
	//�Ⱓ�� ĳ�� �ڽ�
	void LoginSelectCashTable(CQueryResultData *query_data);
	void InsertCashTable(CQueryResultData *query_data);
	void UpdateCashTable(const DWORD dwCode, const int iResult, const DWORD dwReceiveDate, ioHashString& szBillingGUID);
	void RequestTimeCash(const DWORD dwCode, ioHashString& szBillingGUID, BOOL bFirst);
	
	//��ʸ�Ʈ ��ũ��
	void OnTestTournamentMacro(SP2Packet &rkPacket);
	//Īȣ
	void UpdateUserTitle(const DWORD dwCode, const __int64 iValue, const int iLevel, const BYTE byPremium, const BYTE byEquip, const BYTE byStatus, const BYTE byActionType);
	void UpdateUserTitleStatus();
	void SelectUserTitle(CQueryResultData *query_data);
	void OnTitleChange(SP2Packet &rkPacket);
	void OnSyncTimeEquipTitle(SP2Packet &rkPacket);
	void OnNewTitleConfirm(SP2Packet &rkPacket);
	bool FillEquipTitleData(SP2Packet &rkPacket);

	//����ȭ
	void GiveWomanGender(int iClassType);

	void DeleteExpiredPcRoomChar(int iClassType);
	
	
	//���ʽ� ĳ��
	void DeleteExpiredBonusCash();
	int GetAmountOfBonusCash();
	void InsertUserBonusCash(const DWORD dwIndex, const int iAmount, const DWORD dwExpirationDate);
	void UpdateUserBonusCash(BonusCashUpdateType eType, const DWORD dwStatus, const DWORD dwCashIndex, const int iAmount, const int iUsedAmount, const int iType, const int iValue1, const int iValue2, const char *szBillingGUID);
	void GivePCRoomRodAndBait();
	
	// �ŷ��� ������ ����Ʈ ��û
	void ReqTradeItemList( SP2Packet &kPacket );

public:
	BOOL IsTutorialUser() { return bTutorial; } 
	void CreateTutorialChar();

protected:
	//������ Ʃ�丮��
	BOOL bTutorial;

	// �������� ���� ������������ ���̵�����(Ƽ��)������.
	ioUserCoin * GetUserCoin();
public:
	// ���̵� Ƽ�� ����
	bool GetLastRaidTicketTime(CTime & outLastRefillTime);
	void UpdateRaidTicketTime();
	void CheckRefillRaidTicket(bool bLogin);
	int AddEtcRaidTicket(int iAddTicket);
	int GetRaidTicket();

	//������ ���� ����
	typedef std::vector<int> LADDER_USER_LIST;
	LADDER_USER_LIST m_ladderList;
	virtual void ClearLadderUser() { m_ladderList.clear();  }
	void AddLadderUser(int competitorIndex) { m_ladderList.push_back(competitorIndex);  }
	void CopyLadderUserList( std::vector<int>& vLadderList );
	void FillUserNetworkInfo( SP2Packet &rkPacket );
	void SetStealth( bool bStealth ) { m_bStealth = bStealth; };
};

#endif // !defined(AFX_USER_H__03E2797C_74A5_433B_B34D_2E60C0784CEF__INCLUDED_)
