#pragma once

#include "../Util/Singleton.h"
#include "MissionTriggerBase.h"

class Mission;
class ioMission;
class User;

class MissionManager : public Singleton< MissionManager >
{
public:
	MissionManager();
	virtual ~MissionManager();

	void Init();
	void Destroy();

	void ProcessMission();

public:
	static MissionManager& GetSingleton();

public:
	BOOL LoadINI(BOOL bReload = FALSE);
	void LoadMissionTypeIniFile(ioINILoader& kLoader, int iType);

	void ReloadMissionTypeIniFile(ioINILoader& kLoader, int iType);

	DWORD GetStartDateFromINI(ioINILoader& kLoader, const int iIndex);
	void  SetDateFromINI(ioINILoader& kLoader, const int iIndex);
public:
	BOOL IsAlive(const DWORD dwCode);
	void SendReward(User* pUser, const DWORD dwMissionCode);

	Mission* GetActiveMission(const DWORD dwCode);
	void DoTrigger(const MissionClasses eMissionClass, User* pUser, DWORDVec& vValues, BOOL bMacro = FALSE);

protected:
	typedef boost::unordered_map<DWORD, Mission*> mCurMissionTable;
	void DeleteDailyMissionTable();
	void DeleteWeeklyMissionTable();
	void DeleteMonthlyMissionTable();

	void CreateActiveMissionTable(const int iType);

	void ChangeActiveMissionData(const int iType);
	void ChangeActiveMissionDate(const int iType);

	void FillActiveMissionData(ioMission* pUserMission, const int iMissionType);

public:
	DWORD GetNextMissionDate(const int iMissionType);
	DWORD GetNextMissionDate( const int iMissionType, int iIndex );
	DWORD GetActiveMissionDate(const int iMissionType);
	int GetActiveIndex(const int iMissionType);
	int GetMaxIndex(const int iMissionType);
	int GetMissionClass(const DWORD dwCode);

	void SetActiveMissionDate(const int iMissionType, DWORD dwDate);
	void SetNextMissionDate(const int iMissionType, DWORD dwDate);
	void SetActiveIndex(const int iMissionType, int iIndex);
	
	BOOL IsComplete(MissionData* pMissionData);

	void GetResetMissionType(const DWORD dDate, IntVec& vMissionType);

	void TurnCurDataIntoNextData(ioMission* pUserMission, int iResetType);

	void FillAllActiveMissionData(ioMission* pUserMission);

	BOOL IsPrevMissionData(int iType, int iIndex);

	BOOL IsTimeMission(const DWORD dwCode);

	DWORD GetMostRapidNextActiveDate();

	DWORD GetPresentID(DWORD dwCode);

public:
	//�׽�Ʈ ��ũ��
	void TestChangeNextDate( SP2Packet& kPacket );

protected:
	DWORD m_dwTimerCheckTime;
	//���Ӱ� ������
	IntVec m_vActiveIndex;		//mission.ini�� [date x] �� x����
	IntVec m_vMaxIndex;			//mission �� �ִ� �ε����� ����. max date x
	IntVec m_vRelMissionIndex;	//mission�� �ε��� ����
	IntVec m_vResetHour;
	DWORDVec m_vBaseTime;
	
	DWORDVec m_vActiveDate;
	DWORDVec m_vNextActiveDate;
	
	mCurMissionTable m_mDailyMissionTable;
	mCurMissionTable m_mMonthlyMissionTable;
	mCurMissionTable m_mWeeklyMissionTable;

private:
	int GetCurrentMissionNumber( IN int iType, IN int iBaseDate , IN int iRotation );
	DWORD GetStartDate( IN int iType, IN int iResetHour );
	DWORD GetNextActiveDate( IN int iType, IN int iResetHour );
};

#define g_MissionMgr MissionManager::GetSingleton()