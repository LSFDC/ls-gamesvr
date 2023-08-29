
#ifndef _TrainingModeHelp_h_
#define _TrainingModeHelp_h_

#include "User.h"

struct TrainingRecord : public ModeRecord
{
	int  iMonsterSyncCount;                               // �ڽ��� ����ȭ�� �ð� �ִ� ���� ����
	DWORD dwRunningManDeco;
	ioHashString szRunningManName;

	TrainingRecord()
	{
		dwRunningManDeco = 0;
		iMonsterSyncCount = 0;
	}
};

typedef std::vector< TrainingRecord > TrainingRecordList;

#endif