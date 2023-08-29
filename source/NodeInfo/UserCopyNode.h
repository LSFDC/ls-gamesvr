#pragma once

#include "UserParent.h"
#include "CopyNodeParent.h"
#include <boost/unordered/unordered_map.hpp>

class UserCopyNode : public UserParent,
					 public CopyNodeParent					 
{
protected:
	// 기본 정보
	DWORD		 m_dwUserIndex;
	DWORD        m_dwDBAgentID;
	ioHashString m_szPrivateID;
	ioHashString m_szPublicID;
	int          m_iCampType;
	int			 m_iGradeLevel;
	int          m_iUserPos;
	int          m_iKillDeathLevel;
	int          m_iLadderPoint;
	bool         m_bSafetyLevel;
	ModeType     m_eModeType;
	bool         m_bDeveloper;
	int          m_iUserRank;
	DWORD        m_dwPingStep;
	DWORD        m_dwGuildIndex;
	DWORD        m_dwGuildMark;
	DWORDVec     m_vBestFriend;
	bool		 m_bShuffleGlobalSearch;
	ioHashString m_szCountry;
	ioHashString m_szConnTime;
	ioHashString m_szGender;
	int			 m_iFirstWinCount;
	int			 m_iFistLoseCount;	

	int			 m_iWinCount;
	int			 m_iLoseCount;	

	__int64			 m_iFirstPeso;		//라틴 : 로그아웃 시 페소 - 첫 로긴시 페소
	int			 m_iGiveUp;			//게임포기 횟수
	int			 m_iFirstExp;		//로긴시 경험치
	int			 m_iClientLogoutType;	//유저 접속 끊을 때 상태
	DWORD		 m_dwEUContryType;

protected:
	void InitData();

public:
	virtual void OnCreate( ServerNode *pCreator );
	virtual void OnDestroy();

public:
	virtual DWORD GetUserIndex() const { return m_dwUserIndex; }
	virtual DWORD GetUserDBAgentID() const { return m_dwDBAgentID; }
	virtual const DWORD GetAgentThreadID() const { return m_szPrivateID.GetHashCode(); }
	virtual const ioHashString& GetPrivateID() const { return m_szPrivateID; }
	virtual const ioHashString& GetPublicID() const { return m_szPublicID; }
	virtual int GetUserCampPos() const { return m_iCampType; }
	virtual int GetGradeLevel() { return m_iGradeLevel; }
	virtual int GetUserPos() { return m_iUserPos; }
	virtual int GetKillDeathLevel() { return m_iKillDeathLevel; }
	virtual int GetLadderPoint(){ return m_iLadderPoint; }
	virtual bool IsSafetyLevel() { return m_bSafetyLevel; }
	virtual const bool IsUserOriginal(){ return false; }
	virtual bool RelayPacket( SP2Packet &rkPacket );
	virtual ModeType GetModeType(){ return m_eModeType; }
	virtual const bool IsDeveloper(){ return m_bDeveloper; }
	virtual int GetUserRanking(){ return m_iUserRank; }
	virtual DWORD GetPingStep(){ return m_dwPingStep; }
	virtual bool  IsGuild();
	virtual DWORD GetGuildIndex();
	virtual DWORD GetGuildMark();
	virtual bool IsBestFriend( DWORD dwUserIndex );
	virtual void GetBestFriend( DWORDVec &rkUserIndexList );
	virtual bool IsShuffleGlboalSearch();

	//hr 추가
	virtual const	ioHashString& GetCountry() const { return m_szCountry; }
	virtual const	ioHashString& GetLatinConnTime() const { return m_szConnTime; }
	virtual __int64 GetFirstMoney() const { return m_iFirstPeso; }
	virtual int		GetFirstExp() const { return m_iFirstExp; }
	
	virtual int		GetFistWinCount() const { return m_iFirstWinCount; }
	virtual int		GetFirstLoseCount() const { return m_iFistLoseCount; } 

	virtual int		GetWinCount() const { return m_iWinCount; }
	virtual int		GetLoseCount() const { return m_iLoseCount; } 
	virtual DWORD   GetEUCountryType() const { return m_dwEUContryType; }

	virtual void    IncreaseWinCount() { m_iWinCount++; }
	virtual void    IncreaseLoseCount() { m_iLoseCount++; }


	virtual void	IncreaseGiveupCount() { m_iGiveUp++; }
	virtual int		GetGiveupCount() { return m_iGiveUp; }
	virtual void	SetLogoutType( int type ) { m_iClientLogoutType = type; }
	virtual int		GetLogoutType() { return m_iClientLogoutType; }
	virtual const	ioHashString& GetGender() const { return m_szGender; }

public:
	void SetUserIndex( DWORD dwIndex ){ m_dwUserIndex = dwIndex; }

public:
	void ApplySyncCreate( SP2Packet &rkPacket );
	void ApplySyncUpdate( SP2Packet &rkPacket );
	void ApplySyncPos( SP2Packet &rkPacket );
	void ApplySyncGuild( SP2Packet &rkPacket );
	void ApplySyncCamp( SP2Packet &rkPacket );
	void ApplySyncPublicID( SP2Packet &rkPacket );
	void ApplySyncBestFriend( SP2Packet &rkPacket );
	void ApplyUserNode( UserParent *pUser );
	void ApplySyncShuffle( SP2Packet &rkPacket );

	//HRYOON
private:
	// 래더전
	int m_iCompetitorIndex;
	typedef std::vector<int> LADDER_USER_LIST;
	LADDER_USER_LIST m_ladderList;

public:
	virtual void InsertUserLadderList( int competitor, int );
	void CopyLadderUserList( std::vector<int>& vLadderList );
	virtual void clearladderuser() { m_ladderList.clear();  }
	virtual void AddLadderUser(int competitorIndex) { m_ladderList.push_back(competitorIndex);  }
public:
	UserCopyNode();
	virtual ~UserCopyNode();
};

typedef boost::unordered_map<DWORD, UserCopyNode*> uUserCopyNode;
typedef uUserCopyNode::iterator uUserCopyNode_iter;

typedef boost::unordered_map<std::string, DWORD> uUserCopyNodeTable;
typedef uUserCopyNodeTable::iterator uUserCopyNodeTable_iter;