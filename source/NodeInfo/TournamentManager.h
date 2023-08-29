#ifndef _TournamentManager_h_
#define _TournamentManager_h_

enum
{
	TOURNAMENT_REGULAR_INDEX = 1,
};

class SP2Packet;
class RegularTournamentReward
{
protected:
	ioHashString m_szTitle;
	DWORD m_dwTournamentStartDate;              // ������ - INI�� [1210081500]���õ�

protected:
	int		 m_iBattleRewardPeso;
	DWORDVec m_RoundRewardIndex;                   // 
	DWORD    m_dwCampWinRewardIndexB;               // Blue
	DWORD    m_dwCampLoseRewardIndexB;              // Blue
	DWORD    m_dwCampWinRewardIndexR;               // Red
	DWORD    m_dwCampLoseRewardIndexR;              // Red
	DWORD    m_dwCampDrawRewardIndex;              //

public:
	DWORD GetStartDate(){ return m_dwTournamentStartDate; }
	int   GetBattleRewardPeso(){ return m_iBattleRewardPeso; }
	DWORD GetRoundRewardIndex( int iRound );
	DWORD GetCampWinRewardIndexB(){ return m_dwCampWinRewardIndexB; }
	DWORD GetCampLoseRewardIndexB(){ return m_dwCampLoseRewardIndexB; }
	DWORD GetCampWinRewardIndexR(){ return m_dwCampWinRewardIndexR; }
	DWORD GetCampLoseRewardIndexR(){ return m_dwCampLoseRewardIndexR; }
	DWORD GetCampDrawRewardIndex(){ return m_dwCampDrawRewardIndex; }

public:
	void LoadReward( ioINILoader &rkLoader );

public:
	RegularTournamentReward( const ioHashString &rkTitle );
	virtual ~RegularTournamentReward();
};
typedef std::vector< RegularTournamentReward * > RegularTournamentRewardVec;


struct RegularRewardDBData
{
	DWORD m_dwTableIndex;
	DWORD m_dwStartDate;

	BYTE  m_TourPos;

	int   m_iMyCampPos;
	int   m_iWinCampPos;
	int   m_iLadderBonusPeso;
	int   m_iLadderRank;
	int   m_iLadderPoint;

	RegularRewardDBData()
	{
		Clear();
	}

	void Clear()
	{
		m_dwTableIndex		= 0;
		m_dwStartDate		= 0;

		m_TourPos			= 0;
		m_iMyCampPos		= CAMP_NONE;
		m_iWinCampPos		= CAMP_NONE;
		m_iLadderBonusPeso	= 0;
		m_iLadderRank		= 0;
		m_iLadderPoint		= 0;
	}
};

//////////////////////////////////////////////////////////////////////////
class TournamentManager : public Singleton< TournamentManager >
{
public:
	// ��ʸ�Ʈ Ÿ��
	enum
	{
		TYPE_REGULAR = 1,                // ���� ����
		TYPE_CUSTOM,                     // ���� ���� 
	};

	// ��ʸ�Ʈ ����
	enum
	{
		STATE_WAITING = 0,         //���� �Ⱓ������ ��� ����
		STATE_TEAM_APP,            //�� ��� �Ⱓ ( ���� �Ⱓ )
		STATE_TEAM_DELAY,          //�� ��� �Ⱓ ( ����  �Ⱓ )
		STATE_TOURNAMENT,          //��ʸ�Ʈ �Ⱓ ( ��/�� ���� �Ⱓ ) 
		// �� �Ʒ��� ����ϸ� �ȵ� - ��ʸ�Ʈ ���尡 ����Ǹ鼭 ������. 
	};

protected:
	struct RoundData
	{
		DWORD m_dwBlueIndex;
		ioHashString m_szBlueName;
		BYTE  m_BlueCamp;

		DWORD m_dwRedIndex;
		ioHashString m_szRedName;
		BYTE  m_RedCamp;

		DWORD m_dwBattleRoomIndex;

		DWORD m_dwInviteTimer;
		RoundData()
		{
			m_dwBlueIndex = 0;
			m_BlueCamp    = 0;

			m_dwRedIndex  = 0;
			m_RedCamp     = 0;

			m_dwBattleRoomIndex = 0;			

			m_dwInviteTimer = 0;
		}
	};
	typedef std::vector< RoundData > RoundDataVec;
	struct RoundBattleRoomData
	{
		RoundDataVec m_RoomList;

		RoundBattleRoomData()
		{
		}
	};
	typedef std::map< DWORD, RoundBattleRoomData > RoundBattleRoomDataMap;

protected:
	struct Tournament 
	{
		DWORD m_dwIndex;
		BYTE  m_Type;
		BYTE  m_State;
		bool  m_bDisableTournament;

		DWORD m_dwStartDate;
		DWORD m_dwEndDate;

		BYTE m_PrevChampTeamCamp;
		ioHashString m_PrevWinChampName;
		
		RoundBattleRoomDataMap m_BattleRoom;
		Tournament()
		{
			m_dwIndex = 0;
			m_Type    = 0;
			m_State   = 0;
			m_dwStartDate = 0;
			m_dwEndDate   = 0;
			m_bDisableTournament = false;

			m_PrevChampTeamCamp = CAMP_NONE;
		}
	};
	typedef std::vector< Tournament > TournamentVec;
	TournamentVec m_TournamentList;

protected:
	RegularTournamentRewardVec m_RegularRewardList;

protected:
	ioHashString m_SenderID;
	int		     m_iCampPesoRewardMent;
	int          m_iCampPesoRewardPeriod;

	int		     m_iCheerPesoRewardMent;
	int          m_iCheerPesoRewardPeriod;

	struct RewardPresent
	{
		short m_iPresentType;
		short m_iPresentMent;
		int   m_iPresentPeriod;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		RewardPresent()
		{
			m_iPresentType = m_iPresentMent = 0;
			m_iPresentPeriod = m_iPresentValue1 = m_iPresentValue2 = 0;
		}
	};
	typedef std::vector< RewardPresent > RewardPresentVec;
	struct RegularRewardTable
	{
		//
		RewardPresentVec m_PresentList;
		RegularRewardTable()
		{
		}
	};
	typedef std::map< DWORD, RegularRewardTable > RegularRewardTableMap;
	RegularRewardTableMap m_RegularRewardTableMap;

protected:     // ���� ��ȸ ���� ���� ���� ( ��ȸ ���� )
	short m_iCustomRewardPresentMent;
	int   m_iCustomRewardPresentPeriod;
	typedef std::map< DWORD, int > CustomRewardEtcItemPriceMap;
	CustomRewardEtcItemPriceMap m_CustomRewardEtcItemPriceMap;

protected:
	RegularRewardDBData m_RegularRewardDBData;

protected:
	void ClearRegularReward();

public:
	void ApplyTournament( SP2Packet &rkPacket );
	void ApplyTournamentEnd( SP2Packet &rkPacket );
	void ApplyTournamentRoundStart( SP2Packet &rkPacket );
	void ApplyTournamentBattleRoomInvite( SP2Packet &rkPacket );
	void ApplyTournamentBattleTeamChange( SP2Packet &rkPacket );

	void ApplyTournamentPrevChampSync( SP2Packet &rkPacket );

protected:
	TournamentManager::Tournament &GetTournament( DWORD dwTourIndex );
	TournamentManager::RoundBattleRoomData &GetTournamentBattleRoomData( DWORD dwTourIndex, DWORD dwServerIndex );
	RegularTournamentReward *GetRegularTournamentReward( DWORD dwStartDate );
	TournamentManager::RegularRewardTable &GetRegularRewardTable( DWORD dwTable );

public:
	void SendTournamentRoomList( User *pUser, DWORD dwTourIndex, int iCurPage, int iMaxCount );

public:
	DWORD GetRegularTournamentIndex();
	int   GetRegularTournamentBattleRewardPeso( DWORD dwTourIndex );
	
public:
	bool IsRegularTournamentTeamEntryAgreeState();
	bool IsTournamentTeamEntryAgreeState( DWORD dwTourIndex );

public:
	bool IsRegularTournament( DWORD dwTourIndex );

public:
	void CheckTournamentBattleInvite( User *pUser, DWORD dwTourIndex, DWORD dwTeamIndex );

protected:
	bool _InsertCustomRewardPresent( User *pUser, const ioHashString &rkNickName, DWORD dwEtcItem, int iCurrentRound );

public:
	void InsertRegularTournamentReward( User *pUser, DWORD dwTableIndex, DWORD dwStartDate, BYTE TourPos, int iMyCampPos, int iWinCampPos, int iLadderBonusPeso, int iLadderRank, int iLadderPoint );
	void InsertRegularTournamentCheerReward( User *pUser, DWORD dwTourTableIdx, DWORD dWCheerTableIdx, DWORD dwStartDate, BYTE TourPos, int iMyCampPos, int iWinCampPos, int iLadderBonusPeso, int iLadderRank, int iLadderPoint, DWORD dwCheerPeso );

	void InsertCustomTournamentReward( User *pUser, DWORD dwTableIndex, const ioHashString &rkNickName, const ioHashString &rkTourName, int iCurrentRound, short MaxRound, DWORD dwReward1, DWORD dwReward2, DWORD dwReward3, DWORD dwReward4 );
	
public:
	int  GetCustomRewardEtcItemPrice( DWORD dwEtcItem );

public:
	BYTE  GetTournamentState( DWORD dwTourIdx );
	DWORD GetTournamentStartDate( DWORD dwTourIdx );
	void  GetRegularTournamentPrevChampInfo( BYTE& PrevChampCamp, ioHashString& PrevChampName );

public:
	void LoadINI();


public:
	static TournamentManager& GetSingleton();

public:
	TournamentManager();
	virtual ~TournamentManager();
};
#define g_TournamentManager TournamentManager::GetSingleton()
#endif