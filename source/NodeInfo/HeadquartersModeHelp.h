
#ifndef _HeadquartersModeHelp_h_
#define _HeadquartersModeHelp_h_

#include "User.h"

struct HeadquartersRecord : public ModeRecord
{
	int  iMonsterSyncCount;                               // �ڽ��� ����ȭ�� �ð� �ִ� ���� ����
	HeadquartersRecord()
	{
		iMonsterSyncCount = 0;
	}
};
typedef std::vector< HeadquartersRecord > HeadquartersRecordList;


struct HeadquartersCharRecord  
{
	// �Ϲ� ������
	DWORD        dwCode;
	DWORD        dwNPCIndex;
	ioHashString szName;
	float        fStartXPos;
	float        fStartZPos;
	DWORD        dwStartTime;
	RecordState  eState;
	ioHashString szSyncUser;
	TeamType     eTeam;
	DWORD        dwCurDieTime;
	bool         bAI;

	// ĳ���� ġ�� - ���� - ��� - �޴�
	int         dwCharIndex;
	int         iClassLevel;
	CHARACTER   kCharInfo;
	ITEM_DATA   kEquipItem[MAX_CHAR_DBITEM_SLOT];
	IntVec      vEquipMedal;
	BYTE        kCharGrowth[MAX_CHAR_GROWTH];
	BYTE        kItemGrowth[MAX_ITEM_GROWTH];

	// ���� ĳ����
	bool        bDisplayCharacter;
	DWORD       dwDisplayEtcMotion;

	HeadquartersCharRecord& operator = ( const HeadquartersCharRecord &rhs )
	{
		// �Ϲ� ������
		dwCode		= rhs.dwCode;
		dwNPCIndex  = rhs.dwNPCIndex;
		
		szName		= rhs.szName;
		fStartXPos	= rhs.fStartXPos;
		fStartZPos	= rhs.fStartZPos;
		dwStartTime = rhs.dwStartTime;
		eState		= rhs.eState;
		szSyncUser	= rhs.szSyncUser;
		eTeam		= rhs.eTeam;
		dwCurDieTime= rhs.dwCurDieTime;
		bAI         = rhs.bAI;

		// ĳ���� ġ�� - ���� - ��� - �޴�
		dwCharIndex = rhs.dwCharIndex;
		iClassLevel = rhs.iClassLevel;
		kCharInfo   = rhs.kCharInfo;
	
		int i = 0;
		for(i = 0;i < MAX_CHAR_DBITEM_SLOT;i++)
			kEquipItem[i] = rhs.kEquipItem[i];
		
		vEquipMedal.clear();
		for(i = 0;i < (int)rhs.vEquipMedal.size();i++)
			vEquipMedal.push_back( rhs.vEquipMedal[i] );
		
		for(i = 0;i < MAX_CHAR_GROWTH;i++)
			kCharGrowth[i] = rhs.kCharGrowth[i];

		for(i = 0;i < MAX_ITEM_GROWTH;i++)
			kItemGrowth[i] = rhs.kItemGrowth[i];

		bDisplayCharacter = rhs.bDisplayCharacter;
		dwDisplayEtcMotion= rhs.dwDisplayEtcMotion;

		return *this;
	}

	HeadquartersCharRecord()
	{
		// �Ϲ� ������
		dwCode		= 0;
		dwNPCIndex  = 0;
		fStartXPos  = 0.0f;
		fStartZPos  = 0.0f;
		dwStartTime = 0;
		eState      = RS_LOADING;
		eTeam       = TEAM_RED;
		dwCurDieTime= 0;
		bAI         = false;

		// ĳ���� ġ�� - ���� - ��� - �޴�
		dwCharIndex = 0;
		iClassLevel = 0;
		
		int i = 0;
		for(i = 0;i < MAX_CHAR_GROWTH;i++)
			kCharGrowth[i] = 0;

		for(i = 0;i < MAX_ITEM_GROWTH;i++)
			kItemGrowth[i] = 0;

		bDisplayCharacter = false;
		dwDisplayEtcMotion= 0;
	}	
};
typedef std::vector< HeadquartersCharRecord > HeadquartersCharRecordList;

#endif