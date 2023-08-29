#pragma once
#include <vector>

class User;

struct RewardInfo
{
	int iDelimiter;		//�⼮, ��ũ���� ���� ������ ���� ��(��� �⼮���� ��� / �ش� ��ũ )
	int iType;
	int iValue1;
	int iValue2;
	int iPresentPeriod;
	int iMent;

	RewardInfo()
	{
		iDelimiter		= 0;
		iType			= 0;
		iValue1			= 0;
		iValue2			= 0;
		iPresentPeriod	= 0;
		iMent			= 0;
	}
};

class GuildRewardManager :public Singleton< GuildRewardManager >
{
public:
	GuildRewardManager();
	virtual ~GuildRewardManager();

	void Init();
	void Destroy();
	void LoadINI();

public:
	BOOL SendAttendReward(User *pUser, int iAttendCount);
	BOOL SendRankReward(User *pUser, int iRank);

public:
	void GetAttendReward(int iAttendCount, RewardInfo& stReward);
	void GetRankReward(int iRank, RewardInfo& stReward);

	int GetRenewalHour() { return m_iRenewalHour; }
	DWORD GetActiveCampDate() { return m_dwActiveCampDate; }

	void SetActiveCampDate(DWORD dwDate);
	DWORD ConvertCampDateToDwordDate( DWORD dwDate);

public:
	BOOL IsRecvGuildRank(int iGuildRank);
	DWORD RecvDateConvertDBTIMESTAMPToDWORD(DBTIMESTAMP& RecvDate);

public:
	static GuildRewardManager& GetSingleton();

protected:
	typedef std::vector<RewardInfo> AttendanceRewardList;
	typedef	std::vector<RewardInfo> RankRewardList;

protected:
	AttendanceRewardList m_vAttendanceRewardList;
	RankRewardList	m_vRankRewardList;

	ioHashString m_szPresentSendID;

	DWORD m_dwActiveCampDate;
	int m_iRenewalHour;
};

#define g_GuildRewardMgr GuildRewardManager::GetSingleton()