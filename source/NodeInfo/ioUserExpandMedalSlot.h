#ifndef __ioUserExpandMedalSlot_h__
#define __ioUserExpandMedalSlot_h__

#include "ioDBDataController.h"

class cSerialize;

// �޴޽��Ծ������� ����Ͽ� ���� �޴޽��Ը��� �����ϴ� �޴��� Ŭ����

class ioUserExpandMedalSlot :  public ioDBDataController
{
public:
	enum
	{
		MAX_SLOT     = 10,
	};

	enum
	{
		DEFAULT_YEAR    = 2010,			// 2010���� DB�� �������� �ʴ´�. �� DateData�� �⵵�� 0�̸� 2010�̶� ���̴�. 1�̸� 2011��
		DATE_YEAR_VALUE = 100000000,    // ����� ������.
		DATE_MONTH_VALUE= 1000000,      // ������ ������.
		DATE_DAY_VALUE =  10000,        // �ϱ��� ������.
		DATE_HOUR_VALUE = 100,          // �ñ��� ������.
	};

	struct ExpandMedalSlot
	{
		BYTE	m_iSlotNumber;	// ���Թ�ȣ.
		int 	m_iClassType;	// �ش�뺴.
		DWORD	m_dwLimitTime;	// ��ȿ�Ⱓ. ( ���� Ȱ��ȭ ���� �ð� )

		ExpandMedalSlot()
		{
			Init();
		}
		void Init()
		{
			m_iSlotNumber	= 0;
			m_iClassType	= 0;
			m_dwLimitTime	= 0;
		}

		// ��¥�� (�� 2013.04.20.02.40)
		SHORT GetYear()
		{
			int iYear = m_dwLimitTime/DATE_YEAR_VALUE;
			iYear += DEFAULT_YEAR;
			return iYear;			// [2013]04200240
		}
		SHORT GetMonth()
		{
			return ( m_dwLimitTime/DATE_MONTH_VALUE ) % 100;	// 2013[04]200240
		}
		SHORT GetDay()
		{
			return ( m_dwLimitTime/DATE_DAY_VALUE) % 100;	// 201304[20]0240
		}
		SHORT GetHour()
		{
			return ( m_dwLimitTime/DATE_HOUR_VALUE ) % 100;		// 20130420[02]40
		}
		SHORT GetMinute()
		{
			return m_dwLimitTime % 100;				// 2013042002[40]
		}
		bool IsMortmain()
		{
			if( m_dwLimitTime == 0 )
				return true;
			else
				return false;
		}
		void SetDate( int iYear , int iMonth, int iDay, int iHour, int iMinute )
		{
			m_dwLimitTime = ( (iYear - DEFAULT_YEAR) * 100000000 ) + ( iMonth * 1000000 ) + ( iDay * 10000 ) + ( iHour * 100 ) + iMinute;
		}
	};
	typedef std::vector<ExpandMedalSlot>	ExpandMedalSlotVec;

protected:
	struct EXPANDMEDALSLOTDB
	{
		bool     m_bChange;
		DWORD    m_dwIndex;
		ExpandMedalSlot m_kExpandMedalSlot[MAX_SLOT];		

		EXPANDMEDALSLOTDB()
		{
			m_bChange = false;
			m_dwIndex = NEW_INDEX;
			memset( m_kExpandMedalSlot, 0, sizeof( m_kExpandMedalSlot ) );
		}
	};
	typedef std::vector< EXPANDMEDALSLOTDB > vEXPANDMEDALSLOTDB;
	vEXPANDMEDALSLOTDB m_vExpandMedalSlotList;

protected:
	void Clear();

protected:
	void InsertDBExMedalSlot( EXPANDMEDALSLOTDB &kExMedalSlotDB, int iLogType );
	void GetQueryArgument( IN ExpandMedalSlot &rkExpandMedalSlot, cSerialize& v_FT );

public:
	virtual void Initialize( User *pUser );
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );	
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );
	
public:
	bool AddExpandMedalSlot( IN ExpandMedalSlot &rkNewExMedalSlot, IN int iLogType );
	
	int  GetExpandMedalSlotNum( int iClassType );
	bool GetRowExMedalSlot( IN DWORD dwIndex, OUT ExpandMedalSlot kExMedalSlot[MAX_SLOT] );

	void DeleteExMedalSlotPassedDate( OUT ExpandMedalSlotVec &rkNewExMedalSlot );
	void DeleteExMedalSlotGradeUp( int iClassType,  int iLevel );
	void FillUseClass( IN int iClassType, OUT SP2Packet &rkPacket );

public:
	static ioUserExpandMedalSlot& GetSingleton();

public:
	ioUserExpandMedalSlot(void);
	virtual ~ioUserExpandMedalSlot(void);
};

#endif // __ioUserExpandMedalSlot_h__