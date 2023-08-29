// RoomNodeManager.h: interface for the RoomNodeManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Room.h"
#include "RoomCopyNode.h"
#include "BlockNode.h"

using namespace std;

#define PLAZA_ROOM_SORT_HALF_POINT             10000000

typedef vector<Room*> vRoom;
typedef vRoom::iterator vRoom_iter;

typedef struct tagSortRoom
{
	RoomParent *m_pRoomParent;
	int         m_iRoomPoint;
	tagSortRoom()
	{
		m_pRoomParent= NULL;
		m_iRoomPoint = PLAZA_ROOM_SORT_HALF_POINT;
	}
}SortRoom;
typedef vector< SortRoom > vSortRoom;
typedef vSortRoom::iterator vSortRoom_iter;
class RoomInfoSort : public std::binary_function< const SortRoom&, const SortRoom&, bool >
{
public:
	bool operator()( const SortRoom &lhs , const SortRoom &rhs ) const
	{
		if( lhs.m_iRoomPoint < rhs.m_iRoomPoint )
		{
			return true;
		}				
		return false;
	}
};

typedef struct tagSortPlazaRoom
{
	enum
	{
		PRS_ACTIVE				= 1,
		PRS_FULL_USER			= 2,
		PRS_NOT_MIN_LEVEL_MATCH = 3,
		PRS_NOT_MAX_LEVEL_MATCH = 4,
	};

	enum
	{
		PRS_SUB_NONE = 0,
		PRS_SUB_NPC_EVENT = 1,
	};

	RoomParent *m_pNode;
	int	        m_iPoint;
	int         m_iState;
	int			m_iSubState;
	tagSortPlazaRoom()
	{
		m_pNode  = NULL;
		m_iPoint = 0;
		m_iSubState = 0;
		m_iState = PRS_ACTIVE;
	}
}SortPlazaRoom;
typedef vector< SortPlazaRoom > vSortPlazaRoom;
typedef vSortPlazaRoom::iterator vSortPlazaRoom_iter;
class PlazaRoomSort : public std::binary_function< const SortPlazaRoom&, const SortPlazaRoom&, bool >
{
public:
	bool operator()( const SortPlazaRoom &lhs , const SortPlazaRoom &rhs ) const
	{
		if( lhs.m_iPoint < rhs.m_iPoint )
			return true;
		return false;
	}
};

class DamageTableSort : public std::binary_function< const DamageTable&, const DamageTable&, bool >
{
public:
	bool operator()( const DamageTable &lhs , const DamageTable &rhs ) const
	{
		if( lhs.iDamage > rhs.iDamage )
		{
			return true;
		}
		//else if( lhs.iDamage == rhs.iDamage )
		//{
		//	int iCmpValue = _stricmp( lhs.szName.c_str(), rhs.szName.c_str() );
		//	if( iCmpValue < 0 )
		//		return true;
		//}
		return false;
	}
};

class RoomNodeManager : public BlockNode
{
private:
	static RoomNodeManager *sg_Instance;

	// ���� �� �޸� Ǯ(���� + ����)
	MemPooler< Room >	m_MemNode;

	// ���� ���� ��
	vRoom      m_vRoomNode;

	// ���� ���� ��
	vRoom      m_vPlazaNode;

	// ���� ���� ��
	vRoom      m_vHeadquartersNode;

	// ���� ���� ��
	vRoomCopyNode m_vRoomCopyNode;

	// ���� ���� ��
	vRoomCopyNode m_vPlazaCopyNode;

	// ���� ���� ��
	vRoomCopyNode m_vHeadquartersCopyNode;


	// �� ���μ���(���� + ����)
	DWORD	   m_dwRoomProcessTime;

	// ���� ���� ����
	typedef vector< ioHashString > vioHashString;
	vioHashString m_vPlazaNameList;

	// ���� �� �ѹ� ����
	int m_iPlazaRoomNumber;

	DWORD m_dwGuildRoomLifeTime;	

public:
	static RoomNodeManager &GetInstance();
	static void ReleaseInstance();

public:
	void InitPlazaInfo();
	void InitMemoryPool( const DWORD dwServerIndex );
	void ReleaseMemoryPool();

public:
	int RemainderNode(){ return m_MemNode.GetCount(); }
	bool IsAfford();

public:
	int GetRoomNodeSize(){ return m_vRoomNode.size(); }
	int GetPlazaNodeSize(){ return m_vPlazaNode.size(); }
	int GetHeadquartersNodeSize(){ return m_vHeadquartersNode.size(); }
	int GetCopyRoomNodeSize(){ return m_vRoomCopyNode.size(); }
	int GetCopyPlazaNodeSize(){ return m_vPlazaCopyNode.size(); }
	int GetCopyHeadquartersNodeSize(){ return m_vHeadquartersCopyNode.size(); }
	
public:  // ����
	int GetSafetySurvivalRoomUserCount(); // �ʺ����� �����̹� ������
	int GetPlazaUserCount();
	int GetHeadquartersUserCount();
	void SendCurLadderRoomList( User *pUser, int iPage, int iMaxCount ); //������ ���� �������ֱ�
	void SendLadderRoomJoinInfo( UserParent *pUserParent, DWORD dwIndex, int iPrevBattleIndex );
	ioHashString GetPlazaRandomName();
	Room *GetRoomNode( int iRoomIndex );

public:  // ���� �� ����
	Room* CreateNewRoom();
	Room* CreateNewPlazaRoom( int iSubType = -1, int iModeMapNum = -1 );
	Room* CreateNewHeadquartersRoom( int iSubType = -1, int iModeMapNum = -1 );

	void RemoveRoom( Room *pRoom );
	void RemovePlazaRoom( Room *pRoom );
	void RemoveHeadquartersRoom( Room *pRoom );

	void AddCopyRoom( RoomCopyNode *pRoom );
	void RemoveCopyRoom( RoomCopyNode *pRoom );

public:  // ���� ����ȭ
	void ConnectServerNodeSync( ServerNode *pServerNode );

public: //���� �� ��Ī���̺�
	void CreateMatchingTable();

public:
	

	// ���� �� ����
	Room* GetExitRoomJoinPlazaNode( int iMyLevel );								//���� ������ ���� ��( ������ �����Ͽ� ���� ) �� ��Ż�� 

	// ���� ����Ʈ
	int GetSortPlazaRoomPoint( SortPlazaRoom &rkSortRoom, int iKillDeathLevel );

	// ���� ���� ���� ����
	Room* GetJoinPlazaNode( int iMyLevel, Room *pMyRoom );                      //���� ������ ���� ��( ������ �����Ͽ� ���� ) ���� ������
	Room* GetAutoCreatePlazaNode( int iMyLevel );
	Room* GetJoinPlazaNodeByNum( int iRoomIndex );
		
	// ���� Ÿ ���� ����
	RoomCopyNode* GetJoinPlazaCopyNode( int iMyLevel );
	RoomCopyNode* GetJoinPlazaCopyNodeByNum( int iRoomIndex );

	// ���� ��� ���� ����
	RoomParent* GetJoinGlobalPlazaNode( int iMyLevel, RoomParent *pMyRoom );
	RoomParent* GetJoinGlobalPlazaNodeByNum( int iRoomIndex );
	RoomParent* GetExitRoomJoinGlobalPlazaNode( int iMyLevel, RoomParent *pMyRoom ); //���� ������ ���� ��( ������ �����Ͽ� ���� ) �� ��Ż�� 

	Room*		GetGlobalLadderRoomNode( DWORD dwRoomIndex );

	//
	Room *GetHeadquartersNode( int iRoomIndex );
	RoomParent* GetHeadquartersGlobalNode( int iRoomIndex );          // ����

	//
	RoomParent* GetGlobalPlazaNodeByNum( int iRoomIndex );
	RoomParent* GetPlazaNodeByNum( int iRoomIndex );

	void SendPlazaRoomList( User *pUser, int iCurPage, int iMaxCount );         //���� �� ����Ʈ ����
	void SendPlazaRoomJoinInfo( UserParent *pUserParent, DWORD dwRoomIndex );
	
	DWORD GetGuildRoomLifeTime();

	Room * CreateNewPersonalHQRoom( int iSubType = -1, int iModeMapNum = -1 );

protected:
	void INILoadToGuildRoomLifeTime();

public:
	void NagleOptionChange( bool bNagleAlgorithm, bool bPlazaNagleAlgorithm );

public:
	void RoomProcess();
	//�ؿ� ������ ����
	int GetSortLadderRoomPoint( SortRoom& rkSortRoom );
	void FillLadderRoomInfo( SP2Packet& rkPacket, Room& rkRoomNode );
	void EnterUserToLadderRoom( User* pUser, Room& rkRoomNode, bool bObserver );
private:     	/* Singleton Class */
	RoomNodeManager();
	virtual ~RoomNodeManager();
};

inline Room* ToRoomOriginal( Room *pRoom )
{
	if( !pRoom || !pRoom->IsRoomOriginal() )
		return NULL;

	return static_cast< Room* >( pRoom );
}

#define g_RoomNodeManager RoomNodeManager::GetInstance()

