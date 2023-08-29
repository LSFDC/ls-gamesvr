
#ifndef _ioQuest_h_
#define _ioQuest_h_

#include "NodeHelpStructDefine.h"
#include "ioDBDataController.h"
#include "ioQuestManager.h"

class QuestParent;
class QuestDataParent
{
public:
	enum
	{
		DEFAULT_YEAR    = 2010,			// 2010���� DB�� �������� �ʴ´�. �� DateData�� �⵵�� 0�̸� 2010�̶� ���̴�. 1�̸� 2011��
		DATE_YEAR_VALUE = 100000000,    // ����� ������.
		DATE_MONTH_VALUE= 1000000,      // ������ ������.
		DATE_DAY_VALUE =  10000,        // �ϱ��� ������.
		DATE_HOUR_VALUE = 100,          // �ñ��� ������.

		INDEX_HALF_VALUE = 10000,		// IndexData���� �߶� 2���� ������ �ִ´�.
		MAGIC_VALUE      = 100,			// MagicData���� �߶� 3���� ������ �ִ´�.
		ALARM_VALUE      = 10,			// MagicData���� �߶� 3���� ������ �ִ´�.
	};

protected:
	DWORD m_dwIndexData;           // ���� �ε��� + ���� �ε���
	DWORD m_dwDateData;            // ����/�Ϸ� �ð��� : 00�� 01�� 01�� 00�� 00��  = 2010(Default) << �� << �� << �� << �� << ��

protected:
	QuestParent *m_pLinkQuest;

public:
	DWORD GetMainIndex();
	DWORD GetSubIndex();

public:
	DWORD GetYear();
	DWORD GetMonth();
	DWORD GetDay();
	DWORD GetHour();
	DWORD GetMinute();

public:
	virtual DWORD GetIndexData(){ return m_dwIndexData; }
	virtual DWORD GetDateData(){ return m_dwDateData; }
	virtual QuestParent *GetLinkQuest();

public:
	void SetIndexData( DWORD dwIndexData );
	void SetDateData( DWORD dwDateData );
	void SetDate( WORD wYear, WORD wMonth, WORD wDay, WORD wHour, WORD wMinute );
	void SetLinkQuest( QuestParent *pQuest );

public:
	bool CheckDelete();
	bool CheckOneDayDelete( DWORD dwCheckTime );
	bool CheckGuildQuestDelete( bool bGuild );

public:
	QuestDataParent();
	virtual ~QuestDataParent();
};

class QuestData : public QuestDataParent
{
protected:
	DWORD m_dwValueData;           // �޼���
	DWORD m_dwMagicData;           // �߰���(2Byte��밡��) + �˸��� ��� + ����(����,�޼�,�Ϸ�)

public:
	DWORD GetCurrentData();
	DWORD GetCurrentMagicData();
	DWORD GetState();

public:
	bool  IsAlarm();

public:
	inline DWORD GetValueData(){ return m_dwValueData; }
	inline DWORD GetMagicData(){ return m_dwMagicData; }

public:
	void PresetData( QuestParent *pQuest ); 
	void SetCurrentData( DWORD dwCurrentData );
	void SetCurrentMagicData( DWORD dwCurrentMagicData );
	void SetAlarm( bool bAlarm );
	void SetState( DWORD dwState );

public:
	void AttainIntegrity( User *pUser );

public:
	void ApplyData( CQueryResultData *query_data );	
	void ApplyData( SP2Packet &rkPacket );

public:
	void ClearData();

public:
	QuestData();
	virtual ~QuestData();
};

class QuestCompleteData : public QuestDataParent
{
public:
	void PresetData( QuestParent *pQuest ); 

public:
	void ApplyData( CQueryResultData *query_data );	
	void ApplyData( SP2Packet &rkPacket );

public:
	void ClearData();

public:
	QuestCompleteData();
	virtual ~QuestCompleteData();
};
//////////////////////////////////////////////////////////////////////////
class ioQuest : public ioDBDataController
{
public:
	enum
	{
		MAX_SLOT   = 20,	
	};

protected:
	// ���� & �޼� ���
	struct QuestInfo   
	{
		bool      m_bChange;
		DWORD     m_dwIndex;
		QuestData m_Data[MAX_SLOT];

		QuestInfo()
		{
			m_bChange = false;
			m_dwIndex = NEW_INDEX;
		}
	};
	typedef std::vector< QuestInfo > vQuestInfo;
	vQuestInfo m_vItemList;

	// �Ϸ� ���
	struct QuestCompleteInfo
	{
		bool      m_bChange;
		DWORD     m_dwIndex;
		QuestCompleteData m_Data[MAX_SLOT];

		QuestCompleteInfo()
		{
			m_bChange = false;
			m_dwIndex = NEW_INDEX;
		}
	};
	typedef std::vector< QuestCompleteInfo > vQuestCompleteInfo;
	vQuestCompleteInfo m_vCompleteList;

	// ������ ���
	struct QuestDeleteInfo
	{
		DWORD m_dwMainIndex;
		DWORD m_dwSubIndex;
		QuestDeleteInfo()
		{
			m_dwMainIndex = m_dwSubIndex = 0;
		}
	};
	typedef std::vector< QuestDeleteInfo > vQuestDeleteInfo;
	vQuestDeleteInfo m_vDeleteList;

	// ����� üũ
	struct QuestAbuseInfo
	{
		WORD m_dwMainIndex;
		WORD m_dwSubIndex;
		DWORD m_dwCurrentTime;
		bool m_bServerMoving;

		QuestAbuseInfo()
		{
			m_dwMainIndex = m_dwSubIndex = 0;
			m_dwCurrentTime = 0;
			m_bServerMoving = false;
		}
	};
	typedef std::vector< QuestAbuseInfo > vQuestAbuseInfo;
	vQuestAbuseInfo m_AbuseList;

public:
	virtual void Initialize( User *pUser );

	// ���� & �޼� ���
protected:
	void InsertDB( QuestInfo &kQuestDB );     

public:
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );	

public:
	QuestData GetQuestData( DWORD dwMainIndex, DWORD dwSubIndex );
	QuestData AddQuestData( DWORD dwMainIndex, DWORD dwSubIndex );
	QuestData SetQuestCurrentData( QuestData &kData );
	QuestData SetQuestAlarm( DWORD dwMainIndex, DWORD dwSubIndex, bool bAlarm );
	void SetQuestReward( DWORD dwMainIndex, DWORD dwSubIndex, bool isRewardSelectStyle, std::vector<BYTE>& vSelIndexes, SP2Packet &rkPacket );

protected:
	// return : ������ ���� �Ǿ����� true / ������ ���޵��� �ʾ����� false
	bool PackQuestReward( SP2Packet& rpacket, QuestParent* quest, BYTE* pindexes, int numindex );

	// �Ϸ� ���
protected:
	void InsertCompleteDB( QuestCompleteInfo &kQuestDB );

public:
	virtual bool DBtoNewCompleteIndex( DWORD dwIndex );
	virtual void DBtoCompleteData( CQueryResultData *query_data );	

public:
	QuestCompleteData AddQuestCompleteData( DWORD dwMainIndex, DWORD dwSubIndex );

	// ����
public:
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	bool IsQuestComplete( DWORD dwMainIndex, DWORD dwSubIndex );
	bool IsQuestIndexCheck( DWORD dwMainIndex, DWORD dwSubIndex );

public:
	void CheckQuestReConnectSeconds( int iTotalSeconds );

public:
	void ClearQuestInfo( DWORD dwMainIndex, DWORD dwSubIndex );
	void ClearOneDayQuestCompleteAll();
	void ClearGuildQuestProgressAll();

public: // Abuse
	void AddAbuseCheckQuest( DWORD dwMainIndex, DWORD dwSubIndex );				// ����� üũ�ϴ� ����Ʈ ���
	void DeleteAbuseCheckQuest( DWORD dwMainIndex, DWORD dwSubIndex );			// �Ϸ��� ����Ʈ�� ����� üũ���� ����
	bool IsQuestAbuse( QuestData &rkServerQuest, QuestData &rkClientData );		// ����Ʈ �����

public:  //�׽�Ʈ�� ��ũ�� 
	void InitQuestData();

public:
	ioQuest();
	virtual ~ioQuest();
};

#endif