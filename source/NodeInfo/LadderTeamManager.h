#ifndef _LadderTeamManager_h_
#define _LadderTeamManager_h_

#include "LadderTeamNode.h"
#include "LadderTeamCopyNode.h"
#include "boost\unordered_map.hpp"
#include "LevelMatchManager.h"

typedef vector< LadderTeamNode* > vLadderTeamNode;
typedef vLadderTeamNode::iterator vLadderTeamNode_iter;

typedef boost::unordered_map<DWORD, LadderTeamNode*> mLadderTeamNode;
typedef mLadderTeamNode::iterator mLadderTeamNode_iter;

typedef boost::unordered_map<DWORD, LadderTeamCopyNode*> mLadderTeamCopyNode;
typedef mLadderTeamCopyNode::iterator mLadderTeamCopyNode_iter;

#define SORT_LADDERTEAM_POINT      10000000
typedef struct tagSortLadderTeam
{
	enum
	{
		LTS_ACTIVE				= 1,
		LTS_FULL_USER			= 2,
		LTS_NOT_MIN_LEVEL_MATCH = 3,
		LTS_NOT_MAX_LEVEL_MATCH = 4,
		LTS_MATCH_PLAY			= 5,
		LTS_GUILD_TEAM_JOIN		= 6,
		LTS_CAMP_NOT_MATCH		= 7,
	};

	LadderTeamParent *m_pNode;
	int               m_iState;
	int		  	      m_iPoint;
	tagSortLadderTeam()
	{
		m_pNode  = NULL;
		m_iState = LTS_ACTIVE;
		m_iPoint = SORT_LADDERTEAM_POINT;
	}
}SortLadderTeam;
typedef vector< SortLadderTeam > vSortLadderTeam;
typedef vSortLadderTeam::iterator vSortLadderTeam_iter;
class LadderTeamSort : public std::binary_function< const SortLadderTeam&, const SortLadderTeam&, bool >
{
public:
	bool operator()( const SortLadderTeam &lhs , const SortLadderTeam &rhs ) const
	{
		if( lhs.m_iPoint < rhs.m_iPoint )
			return true;
		else if( lhs.m_iPoint == rhs.m_iPoint )
		{
			if( lhs.m_pNode && rhs.m_pNode )
			{
				if( lhs.m_pNode->GetIndex() < rhs.m_pNode->GetIndex() )
					return true;
			}
		}
		return false;
	}
};

typedef struct tagLadderTeamMapInfo
{
	bool         m_bActive;
	ioHashString m_Title;
	int          m_iSubType;

	tagLadderTeamMapInfo()
	{
		m_bActive = true;
		m_iSubType = -1;
	}
}LadderTeamMapInfo;
typedef std::vector< LadderTeamMapInfo > LadderTeamMapInfoList;

typedef struct tagLadderTeamRandomMode
{
	ioHashString m_Title;
	ModeType     m_ModeType;
	LadderTeamMapInfoList m_SubList;
	tagLadderTeamRandomMode()
	{
		m_ModeType = MT_NONE;
	}
}LadderTeamRandomMode;
typedef std::vector< LadderTeamRandomMode > LadderTeamRandomModeList;

class LadderTeamManager
{
	static LadderTeamManager *sg_Instance;
	
protected:
	// ����
	//vLadderTeamNode        m_vLadderTeamNode;	
	mLadderTeamNode			m_mLadderTeamNode;

	MemPooler< LadderTeamNode >	m_MemNode;

	DWORDVec				m_vSearchingTeamList;		//��Ī �õ����� ������ �� �ε���
	DWORDVec				m_vSearchingLadderList;		//��Ī �õ����� ������ �ε���

	// ���纻
	//vLadderTeamCopyNode    m_vLadderTeamCopyNode;
	mLadderTeamCopyNode		m_mLadderTeamCopyNode;

	DWORDVec				m_vCopyNodeSearchingTeamList;		//��Ī �õ����� ������ �� �ε���
	DWORDVec				m_vCopyNodeSearchingLadderList;		//��Ī �õ����� ������ �ε���

	DWORD                  m_dwCurTime;
	bool                   m_bCampBattlePlay;

protected:
	LadderTeamRandomModeList m_vTotalRandomModeList;

	int m_iLadderBattleWinPoint;
	int m_iLadderBattleDrawPoint;
	int m_iLadderBattleLosePoint;

	float m_fSeasonEndDecreaseRate;

	//Search
	DWORD m_dwSearchMatchFullTime;
	int   m_iSearchMatchSendSec;
	DWORD m_dwLastMatchDelayTime;

protected:
	//
	int m_iMaxLadderTeamSize;
	int m_iAverageHeroMatchPoint;

public:
	static LadderTeamManager &GetInstance();
	static void ReleaseInstance();

public:
	void InitMemoryPool( const DWORD dwServerIndex );
	void ReleaseMemoryPool();

public:
	void CreateTeamMatchingTable();
	void CreateHeroMatchingPoint();
	void CreateHeroMatchingTable();

public:
	DWORD GetSearchMatchFullTime(){ return m_dwSearchMatchFullTime; }
	int GetSearchMatchSendSec(){ return m_iSearchMatchSendSec; }
	int GetLadderBattleWinPoint(){ return m_iLadderBattleWinPoint; }
	int GetLadderBattleDrawPoint(){ return m_iLadderBattleDrawPoint; }
	int GetLadderBattleLosePoint(){ return m_iLadderBattleLosePoint; }
	float GetSeasonEndDecreaseRate(){ return m_fSeasonEndDecreaseRate; }
	DWORD GetLastMatchDelayTime(){ return m_dwLastMatchDelayTime; }
	int GetHeroMatchAveragePoint(){ return m_iAverageHeroMatchPoint; }

public:
	bool IsCampBattlePlay(){ return m_bCampBattlePlay; }
	void SetCampBattlePlay( bool bCampBattlePlay );

public:
	int RemainderNode(){ return m_MemNode.GetCount(); }

public:
	int GetNodeSize(){ return m_mLadderTeamNode.size(); } 
	int GetCopyNodeSize(){ return m_mLadderTeamCopyNode.size(); }
	int GetHeroNodeSize();

public:
	LadderTeamNode *CreateNewLadderTeam();	
	void RemoveLadderTeam( LadderTeamNode *pNode );

	void AddCopyLadderTeam( LadderTeamCopyNode *pLadderTeam );
	void RemoveLadderTeamCopy( LadderTeamCopyNode *pLadderTeam );	

	// Ÿ������ ������ ���� ��尡 ������ �� ó��
	void RemoveUserCopyNode( DWORD dwUserIndex, const ioHashString &rkName );

public:  // ���� ����ȭ
	void ConnectServerNodeSync( ServerNode *pServerNode );

public:
	LadderTeamNode* GetLadderTeamNode( DWORD dwIndex );
	LadderTeamParent* GetGlobalLadderTeamNode( DWORD dwIndex );
	
public:
	int GetLadderTeamUserCount();

protected:
	void InitTotalModeList();

public:
	bool CheckLadderTeamRandomMode( int iModeIndex, int iMapIndex, ModeType &eModeType, int &iSubType );

public:
	bool SearchBattleMatch( LadderTeamNode *pRequestTeam );

public:
	int GetSortLadderTeamPoint( SortLadderTeam &rkLadderTeam, UserParent *pUserParent );
	int GetSortLadderTeamState( LadderTeamParent *pLadderTeamParent, UserParent *pUserParent );
	void SendCurLadderTeamList( User *pUser, int iPage, int iMaxCount, bool bHeroMatch );			//�������� ��Ƽ ���	
	void SendLadderTeamJoinInfo( UserParent *pUserParent, DWORD dwIndex );

public:
	void RemoveSearchingList(DWORD dwIndex,  bool bLadder);
	void AddSearchingList(DWORD dwIndex,  bool bLadder);

	void RemoveCopyNodeSearchingList(DWORD dwIndex,  bool bLadder);
	void AddCopyNodeSearchingList(DWORD dwIndex,  bool bLadder);

protected:
	void RemoveSearchingTeamList(DWORD dwIndex);
	void RemoveSearchingLadderList(DWORD dwIndex);
	void RemoveCopyNodeSearchingTeamList(DWORD dwIndex);
	void RemoveCopyNodeSearchingLadderList(DWORD dwIndex);

	void AddSearchingTeamList(DWORD dwIndex);
	void AddSearchingLadderList(DWORD dwIndex);
	void AddCopyNodeSearchingTeamList(DWORD dwIndex);
	void AddCopyNodeSearchingLadderList(DWORD dwIndex);

	bool SearchingLadderMode(LadderTeamNode *pRequestTeam);
	bool SearchingTeamMode(LadderTeamNode *pRequestTeam);

public:
	void SortLadderTeamRank( bool bHeroMatch, User *pSendUser = NULL, DWORD dwTeamIndex = 0, int iCurPage = 0, int iMaxCount = 0 );

public:
	void UpdateProcess();

private:
	LadderTeamManager();
	virtual ~LadderTeamManager();	

public:
	template <class T>
	bool DoMatching(LadderTeamNode* pRequestTeam, T* pNode)
	{
		bool bSameCampSearch = false;

		if( pRequestTeam->GetSearchCount() >= 3 )
			bSameCampSearch = true;

		if( pNode == NULL ) return false;
		if( pRequestTeam->GetIndex() == pNode->GetIndex() ) return false;
		if( pNode->IsEmptyUser() ) return false;
		if( pRequestTeam->IsHeroMatchMode() != pNode->IsHeroMatchMode() ) return false;  //������ ���� ������ ���� ������.
		if( !bSameCampSearch )
		{
			if( pRequestTeam->GetCampType() == pNode->GetCampType() ) return false;			//���� �������� ���� X
		}
		if( !pNode->IsHeroMatchMode() && pNode->GetJoinUserCnt() <= 1 ) return false;    //������ 1�� �÷��� �ȵ�
		if( pRequestTeam->IsBadPingKick() != pNode->IsBadPingKick() ) return false;      //��Ʈ��ũ �ҷ� �ɼ��� ���ƾ���
		if( pRequestTeam->IsMatchReserve( pNode->GetIndex() ) ) return false;
		if( !pNode->IsSearching() ) return false;

		// HRYOON 
		if( pRequestTeam->IsHeroMatchMode() )
		{
			if(ioLocalManager::GetLocalType() == ioLocalManager::LCT_TAIWAN )
			{

				// HRYOON LADDER ��Ī �� �ֱ� ������ �Բ� �ߴ� ������ �ִ��� ã�´�.

				UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( pRequestTeam->GetTeamName() );
				UserParent *pNodeUser = g_UserNodeManager.GetGlobalUserNode( pNode->GetTeamName() );

				if( pRequestUser  != NULL && pNodeUser != NULL ) 
				{
					bool bMatch = true;
					bMatch = pRequestTeam->FindMatchingUser( pRequestUser->GetUserIndex(), pNodeUser->GetUserIndex() );
					if( bMatch == false ) return false;
				}
				else
					return false;
			}
		}

		// ��û�� �ɼ� üũ
		// ���ظ�Ī �˻� �ɼ� üũ
		if( pRequestTeam->IsSearchLevelMatch() )
		{
			if( pRequestTeam->IsHeroMatchMode() )
			{
				if( !g_LevelMatchMgr.IsLadderHeroLevelJoin( pRequestTeam->GetAbilityMatchLevel(), pNode->GetAbilityMatchLevel(), JOIN_CHECK_MINMAX_LEVEL ) ) return false;
			}
			else
			{
				if( !g_LevelMatchMgr.IsLadderLevelJoin( pRequestTeam->GetAbilityMatchLevel(), pNode->GetAbilityMatchLevel(), JOIN_CHECK_MINMAX_LEVEL ) ) return false;
			}
		}

		// ���� �ο� �ɼ� üũ
		if( pRequestTeam->IsSearchSameUser() )
		{
			if( pRequestTeam->GetJoinUserCnt() != pNode->GetJoinUserCnt() ) return false;
		}		
		
		// ����� �ɼ� üũ
		if( pNode->IsSearchLevelMatch() )
		{
			if( pNode->IsHeroMatchMode() )
			{
				if( !g_LevelMatchMgr.IsLadderHeroLevelJoin( pNode->GetAbilityMatchLevel(), pRequestTeam->GetAbilityMatchLevel(), JOIN_CHECK_MINMAX_LEVEL ) ) return false;
			}
			else
			{
				if( !g_LevelMatchMgr.IsLadderLevelJoin( pNode->GetAbilityMatchLevel(), pRequestTeam->GetAbilityMatchLevel(), JOIN_CHECK_MINMAX_LEVEL ) ) return false;
			}
		}
		// ���� �ο� �ɼ� üũ
		if( pNode->IsSearchSameUser() )
		{
			if( pRequestTeam->GetJoinUserCnt() != pNode->GetJoinUserCnt() ) return false;
		}
		
		// ��� üũ
		if( pRequestTeam->GetSelectMode() != -1 && pNode->GetSelectMode() != -1 )
		{
			if( pNode->GetSelectMode() != pRequestTeam->GetSelectMode() ) return false;
		}
		// �� üũ
		if( pRequestTeam->GetSelectMap() != -1 && pNode->GetSelectMap() != -1 )
		{
			if( pNode->GetSelectMap() != pRequestTeam->GetSelectMap() ) return false;
		}

		//HRYOON LADDER
		//�������ΰ�� ������ �ε����� �����Ѵ�. 

		if(ioLocalManager::GetLocalType() == ioLocalManager::LCT_TAIWAN )
		{
			if( pNode->IsHeroMatchMode() )
			{

				UserParent *pRequestUser = g_UserNodeManager.GetGlobalUserNode( pRequestTeam->GetTeamName() );
				UserParent *pNodeUser = g_UserNodeManager.GetGlobalUserNode( pNode->GetTeamName() );

				if( pRequestUser  != NULL && pNodeUser != NULL ) 
				{	

					if( pRequestTeam->GetCompetitorIndex() == 0)
					{
						pRequestTeam->SetCompetitorIndex( pNodeUser->GetUserIndex() );	//���� �ε����� ����

						pNode->SetCompetitorIndex( pRequestUser->GetUserIndex() );		//�� �ε����� ���濡�� ����

						pRequestUser->InsertUserLadderList( pNodeUser->GetUserIndex(), pRequestTeam->GetIndex() );
						pNodeUser->InsertUserLadderList( pRequestUser->GetUserIndex(), pNode->GetIndex() );
					}
				}
			}
		}

	

		return true;
	}

};
#define g_LadderTeamManager LadderTeamManager::GetInstance()
#endif
