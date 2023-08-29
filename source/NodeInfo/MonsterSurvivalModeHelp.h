#pragma once


struct MonsterSurvivalRecord : public ModeRecord         // User Record
{
	int  iMonsterSyncCount;                               // �ڽ��� ����ȭ�� �ð� �ִ� ���� ����
	bool bPrisoner;
	bool bClientViewState;
	int  iTreasureCardCount;
	bool bStartCoinDec;

	// for raid
	// ȹ���� ������
	int m_iHunterCoinCount;


	// ���� �Ȼ��� ī�� ��ȣ��.
	std::vector<int> m_vTreasureCard;

	virtual void AddKillCount( RoomStyle eRoomStyle, int iKill )
	{
		int iClassType = pUser->GetCharClassType( pUser->GetSelectChar() );
		if( iClassType > 0 )
		{
			KillDeathInfoMap::iterator iter = iKillInfoMap.find( iClassType );
			if( iter != iKillInfoMap.end() )
			{
				iter->second += iKill;
			}
			else
			{
				iKillInfoMap.insert( KillDeathInfoMap::value_type( iClassType, iKill ) );
			}
		}
	}

	virtual void AddDeathCount( RoomStyle eRoomStyle, int iDeath )
	{
		int iClassType = pUser->GetCharClassType( pUser->GetSelectChar() );
		if( iClassType > 0 )
		{
			KillDeathInfoMap::iterator iter = iDeathInfoMap.find( iClassType );
			if( iter != iDeathInfoMap.end() )
			{
				iter->second += iDeath;
			}
			else
			{
				iDeathInfoMap.insert( KillDeathInfoMap::value_type( iClassType, iDeath ) );
			}
		}
	}

	MonsterSurvivalRecord()
	{
		iMonsterSyncCount = 0;
		bPrisoner = false;
		bClientViewState = false;
		iTreasureCardCount = 0;
		bStartCoinDec = false;
		m_iHunterCoinCount = 0;
	}
};
typedef std::vector< MonsterSurvivalRecord > MonsterSurvivalRecordList;

struct MonsterRecord                                      // Npc Record
{
	DWORD        dwCode;
	DWORD		 dwNPCIndex;
	ioHashString szName;
	float        fStartXPos;
	float        fStartZPos;
	DWORD        dwStartTime;
	DWORD        dwBossStartTime;
	DWORD        dwDieType;             // �׾��� �� ó��
	bool         bBossTurnShow;
	int          iCreateLimitTurn;
	int          iHighTurnIndex;
	int          iLowTurnIndex;
	DWORD        dwCurDieTime;
	DWORD        dwCreateRandValue;
	bool         bCreateUniqueType;
	RecordState  eState;
	ioHashString szSyncUser;
	TeamType     eTeam;
	int			 nGrowthLvl;
	int			 nGroupIdx;
	int			 nNpcType;
	bool		 bGroupBoss;
	bool		 bEndBoss;
	DWORDVec     vDropItemList;
	DWORDVec     vDropRewardItemList;
	DWORD        dwPresentCode;
	int          iExpReward;
	int          iPesoReward;
	DWORD        dwDiceTable;
	DWORD        dwAliveTime;

	MonsterRecord& operator = ( const MonsterRecord &rhs )
	{
		dwCode		= rhs.dwCode;
		dwNPCIndex  = rhs.dwNPCIndex;
		szName		= rhs.szName;
		fStartXPos	= rhs.fStartXPos;
		fStartZPos	= rhs.fStartZPos;
		dwStartTime = rhs.dwStartTime;
		dwBossStartTime = rhs.dwBossStartTime;
		dwDieType   = rhs.dwDieType;
		bBossTurnShow = rhs.bBossTurnShow;
		eState		= rhs.eState;
		szSyncUser	= rhs.szSyncUser;
		eTeam		= rhs.eTeam;
		nGrowthLvl  = rhs.nGrowthLvl;
		nGroupIdx	= rhs.nGroupIdx;
		nNpcType	= rhs.nNpcType;
		bGroupBoss  = rhs.bGroupBoss;
		bEndBoss	= rhs.bEndBoss;
		dwPresentCode = rhs.dwPresentCode;
		iCreateLimitTurn = rhs.iCreateLimitTurn;
		iHighTurnIndex= rhs.iHighTurnIndex;
		iLowTurnIndex = rhs.iLowTurnIndex;
		dwCurDieTime  = rhs.dwCurDieTime;
		dwCreateRandValue = rhs.dwCreateRandValue;
		bCreateUniqueType = rhs.bCreateUniqueType;
		iExpReward  = rhs.iExpReward;
		iPesoReward = rhs.iPesoReward;
		dwDiceTable = rhs.dwDiceTable;
		dwAliveTime = rhs.dwAliveTime;

		int i = 0;
		for(i = 0;i < (int)rhs.vDropItemList.size();i++)
		{
			vDropItemList.push_back( rhs.vDropItemList[i] );
		}
		for(i = 0;i < (int)rhs.vDropRewardItemList.size();i++)
		{
			vDropRewardItemList.push_back( rhs.vDropItemList[i] );
		}

		return *this;
	}

	MonsterRecord()
	{
		dwCode		= 0;
		dwNPCIndex  = 0;
		fStartXPos  = 0.0f;
		fStartZPos  = 0.0f;
		dwStartTime = 0;
		dwBossStartTime  = 0;
		dwDieType   = 0;
		bBossTurnShow = true;
		iCreateLimitTurn = 0;
		iHighTurnIndex = 0;
		iLowTurnIndex  = 0;
		eState      = RS_LOADING;
		eTeam       = TEAM_RED;
		nGroupIdx	= 0;
		nNpcType	= 0;
		bGroupBoss  = false;
		bEndBoss	= false;
		dwCurDieTime= 0;
		dwCreateRandValue = 0;
		bCreateUniqueType = false;
		dwPresentCode = 0;
		iExpReward  = 0;
		iPesoReward = 0;
		dwDiceTable = 0;
		nGrowthLvl = 0;
		dwAliveTime = 0;
	}	
};
typedef std::vector< MonsterRecord > MonsterRecordList;
