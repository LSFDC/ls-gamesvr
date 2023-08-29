#pragma once

#include "BattleRoomNode.h"
#include "BattleRoomCopyNode.h"
#include "BlockNode.h"

#define BATTLE_ROOM_SORT_HALF_POINT          100000000         //1��

class BattleRoomNode;
typedef vector<BattleRoomNode*> vBattleRoomNode;
typedef vBattleRoomNode::iterator vBattleRoomNode_iter;

typedef struct tagSortBattleRoom
{
	enum
	{
		BRS_ACTIVE				= 1,
		BRS_FULL_USER			= 2,
		BRS_TIME_CLOSE			= 3,
		BRS_ENTER_X				= 4,
		BRS_NOT_MIN_LEVEL_MATCH = 5,
		BRS_NOT_MAX_LEVEL_MATCH = 6,
		BRS_ALREADY_ROOM		= 7,
		BRS_FULL_OBSERVER		= 8,
		BRS_MAP_LIMIT_GRADE     = 9,
		BRS_OPTION_LIMIT_GRADE  = 10,
		BRS_NO_CHALLENGER		= 11,
	};

	BattleRoomParent *m_pNode;
	int				  m_iPoint;
	int               m_iState;
	tagSortBattleRoom()
	{
		m_pNode  = NULL;
		m_iPoint = BATTLE_ROOM_SORT_HALF_POINT;
		m_iState = BRS_ACTIVE;
	}
}SortBattleRoom;
typedef vector< SortBattleRoom > vSortBattleRoom;
typedef vSortBattleRoom::iterator vSortBattleRoom_iter;
class BattleRoomSort : public std::binary_function< const SortBattleRoom&, const SortBattleRoom&, bool >
{
public:
	bool operator()( const SortBattleRoom &lhs , const SortBattleRoom &rhs ) const
	{
		if( lhs.m_iPoint < rhs.m_iPoint )
			return true;
		return false;
	}
};

typedef struct tagBattleMapInfo
{
	bool         m_bActive;
	ioHashString m_Title;
	int          m_iSubType;
	int          m_iLimitPlayer;
	int          m_iLimitGrade;
	tagBattleMapInfo()
	{
		m_bActive  = true;
		m_iSubType = -1;
		m_iLimitPlayer = MAX_PLAYER;
		m_iLimitGrade  = 0;
	}
}BattleMapInfo;
typedef std::vector< BattleMapInfo > BattleMapInfoList;

typedef struct tagBattleRandomMode
{
	ioHashString m_Title;
	ModeType     m_ModeType;
	bool         m_bSafetyMode;
	bool         m_bBroadcastMode;
	BattleMapInfoList m_SubList;
	tagBattleRandomMode()
	{
		m_bSafetyMode = false;
		m_bBroadcastMode = false;
		m_ModeType = MT_NONE;
	}
}BattleRandomMode;
typedef std::vector< BattleRandomMode > BattleRandomModeList;

class BattleRoomManager : public BlockNode
{
	static BattleRoomManager *sg_Instance;

protected:
	// ����
	vBattleRoomNode         m_vBattleRoomNode;	
	MemPooler< BattleRoomNode >	m_MemNode;

	// ���纻
	vBattleRoomCopyNode     m_vBattleRoomCopyNode;
	
	DWORD                   m_dwCurTime;
	
	//���� �� �̸�
	typedef vector< ioHashString > vioHashString;
	vioHashString m_vNameList;

	//Ŀ���� �ɼ� ���� ����
	int m_iEnableExtraOption;

protected:
	BattleRandomModeList m_vTotalRandomModeList;

protected:
	//
	int m_iMaxBattleRoomSize;
	int m_iBattleDefaultCatchModeLevel;

public:
	static BattleRoomManager &GetInstance();
	static void ReleaseInstance();

public:
	void InitMemoryPool( const DWORD dwServerIndex );
	void ReleaseMemoryPool();

public:
	int	RemainderNode(){ return m_MemNode.GetCount(); }

public:
	int GetNodeSize(){ return m_vBattleRoomNode.size(); } 
	int GetCopyNodeSize(){ return m_vBattleRoomCopyNode.size(); }

public:
	BattleRoomNode *CreateNewBattleRoom();
	void RemoveBattleRoom( BattleRoomNode *pNode );

	void AddCopyBattleRoom( BattleRoomCopyNode *pBattleRoom );
	void RemoveBattleCopyRoom( BattleRoomCopyNode *pBattleRoom );	

	// Ÿ������ ������ ���� ��尡 ������ �� ó��
	void RemoveUserCopyNode( DWORD dwUserIndex, const ioHashString &rkName );

public:  // ���� ����ȭ
	void ConnectServerNodeSync( ServerNode *pServerNode );

public:
	BattleRoomNode* GetBattleRoomNode( DWORD dwIndex );
	BattleRoomParent* GetGlobalBattleRoomNode( DWORD dwIndex );

public:
	int GetBattleRoomUserCount();
	int GetBattleRoomPlayUserCount();
	int GetBattleDefaultCatchModeLevel(){ return m_iBattleDefaultCatchModeLevel; }

protected:
	void InitTotalModeList();

public:
	bool CheckBattleRandomMode( int iModeIndex, int iMapIndex, ModeType &eModeType, int &iSubType );
	int  GetBattleModeToIndex( ModeType eModeType, bool bSafetyMode, bool bBroadcastMode );
	int  GetBattleMapToLimitPlayer( int iModeIndex, int iMapIndex );
	int  GetBattleMapToLimitGrade( int iModeIndex, int iMapIndex );

public:
	void CreateMatchingTable();

public:
	bool IsBattleRoomSameModeTerm( int iModeTermA, int iModeTermB );
	int GetSortBattleRoomPoint( SortBattleRoom &rkSortRoom, int iKillDeathLevel, int iSelectTerm, int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer );
	int GetSortBattleRoomState( BattleRoomParent *pBattleParent, UserParent *pUserParent, DWORD dwPrevBattleIndex );
	BattleRoomParent *GetJoinBattleRoomNode( int iPrevLeaveBattleRoom, bool bSafetyLevel, int iGradeLevel, int iAverageLevel, int iSearchTerm, 
											 int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer );   //�ڵ� ���� ��Ƽ
	void SendCurBattleRoomList( User *pUser, int iPage, int iMaxCount, int iSelectTerm, int iPrevLeaveBattleRoom,
								int iMinPlayer, int iMaxPlayer, bool bSameTeamPlayer  );			//�������� ��Ƽ ���
	
	void SendBattleRoomJoinInfo( UserParent *pUserParent, DWORD dwIndex, int iPrevBattleIndex );

	bool CheckEnableExtraOptionLevel( int iLevel );

public:
	void UpdateProcess();
	bool SearchRoomIndex( User *pUser, int iRoomIndex );

private:
	BattleRoomManager();
	virtual ~BattleRoomManager();	
};

#define g_BattleRoomManager BattleRoomManager::GetInstance()
