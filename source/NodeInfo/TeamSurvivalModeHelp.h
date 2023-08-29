
#ifndef _TeamSurvivalModeHelp_h_
#define _TeamSurvivalModeHelp_h_

#include "User.h"
#include "ModeHelp.h"

struct TeamSurvivalRecord : public ModeRecord
{
	TeamSurvivalRecord()
	{
	}
};

typedef std::vector< TeamSurvivalRecord > TeamSurvivalRecordList;

struct TeamSurvivalAIRecord : public ModeRecord
{
	int			iMonsterSyncCount;						// �ڽ��� ����ȭ�� �ð� �ִ� ���� ����
	bool		bPrisoner;
	bool		bClientViewState;
	bool		m_bReady;

	TeamSurvivalAIRecord()
	{
		iMonsterSyncCount = 0;
		bPrisoner = false;
		bClientViewState = false;
		m_bReady = false;
	}
};
typedef std::vector< TeamSurvivalAIRecord > TeamSurvivalAIRecordList;

#endif