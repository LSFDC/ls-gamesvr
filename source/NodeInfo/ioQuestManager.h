#ifndef _ioQuestManager_h_
#define _ioQuestManager_h_

#include "../Util/Singleton.h"
#include "QuestVariety.h"

/**************************************************************************************/
/* 퀘스트의 발생은 클라이언트에서 체크 함											  */
/* 퀘스트의 완료는 최대한 클라이언트와 서버 모두 체크해야 어뷰즈를 막을 수 있으나	  */
/* 기능에 따라서 서버 체크가 부담스러울 수 있다. 이 경우 판단을 1회만하면 완료되는	  */
/* (낚시하기) 퀘스트는 클라이언트에서 체크해도 될 듯하다.							  */
/**************************************************************************************/

enum QuestPerform       // 퀘스트 수행 방식
{
	QP_NORMAL = 1,      // 기본 퀘스트
	QP_EVENT  = 2,      // 이벤트 퀘스트
};

enum QuestState
{
	QS_PROGRESS = 0,    // 진행중 - 받은 상태
	QS_ATTAIN,			// 달성 - 퀘스트 달성했지만 보상은 받지 않았음
	QS_COMPLETE,        // 완료 - 보상 받았음.
};

class User;
class ioQuestManager : public Singleton< ioQuestManager >
{
protected:
	typedef std::vector< QuestParent * > vQuestVariety;
	vQuestVariety m_QuestVariety;

	// 보상 리스트 - 선물로 지급함.
protected:
	struct QuestReward
	{
		ioHashString m_szPresentSendID;
		int m_iPresentType;
		int m_iPresentState;
		int m_iPresentMent;
		int m_iPresentValue1;
		int m_iPresentValue2;
		int m_iPresentValue3;
		int m_iPresentValue4;
		int m_iPresentPeriod;
		bool m_bDirectReward;
		QuestReward()
		{
			m_iPresentType = m_iPresentState = m_iPresentMent = m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = m_iPresentPeriod = 0;
			m_bDirectReward = false;
		}
	};
	typedef std::map< DWORD, QuestReward > QuestRewardMap;
	QuestRewardMap m_QuestRewardMap;

protected:
	DWORD m_dwCurrentTime;

protected:
	DWORD m_dwOneDayQuestHour;
	DWORD m_dwNextOneDayQuestDate;

protected:
	void ClearQuestVariety();

protected:
	QuestParent *CreateQuest( const ioHashString &rClassName );
	void SetQuest(char* csClassName, DWORD dwMainIndex, DWORD dwSubIndex, ioINILoader &rkLoader, bool bCreate = false, DWORD dwChangeSubIndex = 0, DWORD dwChangeNextSubIndex = 0);

public:
	void LoadINIData();

public:
	QuestParent *GetQuest( DWORD dwMainIndex, DWORD dwSubIndex );

public:
	bool SendRewardPresent( User *pSendUser, DWORD dwPresentID );
	void GetRewardPresent(DWORD dwPresentID, int& iType, int& iCode, int& iCount, int& iParam);

public:
	bool IsSameChanneling( User *pSendUser, DWORD dwMainIndex, DWORD dwSubIndex );

public:
	DWORD GetPrevOneDayQuestDate();

public:
	void ProcessQuest();
	void SendAliveEventQuest( User *pSendUser );

protected:
	void ProcessOneDayQuest();

public:
	static ioQuestManager& GetSingleton();

public:   
	ioQuestManager();
	virtual ~ioQuestManager();
};
#define g_QuestMgr ioQuestManager::GetSingleton()
#endif