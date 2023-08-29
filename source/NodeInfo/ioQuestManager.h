#ifndef _ioQuestManager_h_
#define _ioQuestManager_h_

#include "../Util/Singleton.h"
#include "QuestVariety.h"

/**************************************************************************************/
/* ����Ʈ�� �߻��� Ŭ���̾�Ʈ���� üũ ��											  */
/* ����Ʈ�� �Ϸ�� �ִ��� Ŭ���̾�Ʈ�� ���� ��� üũ�ؾ� ���� ���� �� ������	  */
/* ��ɿ� ���� ���� üũ�� �δ㽺���� �� �ִ�. �� ��� �Ǵ��� 1ȸ���ϸ� �Ϸ�Ǵ�	  */
/* (�����ϱ�) ����Ʈ�� Ŭ���̾�Ʈ���� üũ�ص� �� ���ϴ�.							  */
/**************************************************************************************/

enum QuestPerform       // ����Ʈ ���� ���
{
	QP_NORMAL = 1,      // �⺻ ����Ʈ
	QP_EVENT  = 2,      // �̺�Ʈ ����Ʈ
};

enum QuestState
{
	QS_PROGRESS = 0,    // ������ - ���� ����
	QS_ATTAIN,			// �޼� - ����Ʈ �޼������� ������ ���� �ʾ���
	QS_COMPLETE,        // �Ϸ� - ���� �޾���.
};

class User;
class ioQuestManager : public Singleton< ioQuestManager >
{
protected:
	typedef std::vector< QuestParent * > vQuestVariety;
	vQuestVariety m_QuestVariety;

	// ���� ����Ʈ - ������ ������.
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