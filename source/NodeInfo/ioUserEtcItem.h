#ifndef __ioUserEtcItem_h__
#define __ioUserEtcItem_h__

#include "ioDBDataController.h"

class Room; 

// ���� , ��, ��Ÿ ������
class ioUserEtcItem : public ioDBDataController
{
public:
	enum
	{
		MAX_SLOT     = 20,
		ITEM_USE_ON  = 1,
		ITEM_USE_OFF = 0,
		USE_TYPE_CUT_VALUE= 1000000,
	};

	struct ETCITEMSLOT
	{
		int m_iType;       // AABBBBBB // AA: 1:Ƚ����, 2:�ð���, 3:��û�� , 4:������� , 5:��¥���� // BBBBBB: ���� 1~99999 : ���� , 100000 ~ 199999 : Ŭ������������(XYYYZZ -> X:Ÿ��,YYY:Ŭ����Ÿ��,ZZ:����������) , 200000 ~ 299999 : ��(��Ȯ��) - �ʸ������� ������ 
		int m_iValue1;     // Type�� ���� ���ӻ��� �ٸ� ���� , ����ȿ�����̸� ���� Ƚ��, ��Ÿ �ð��Ҹ����̸� �����ð�, ��¥�������� ������� ��Ÿ�� 20090715(2009�� 7�� 15�� )
		int m_iValue2;     // Type�� ���� ���ӻ��� �ٸ� ���� : ������ ��뿩�� // 0 : �̻��, 1:���  | ��¥�������� �ð��� ��Ÿ�� 1232 (12��32��)

		ETCITEMSLOT()
		{
			m_iType   = 0;
			m_iValue1 = 0;
			m_iValue2 = 0;
		}
		
		int GetUseType()
		{
			return m_iType / USE_TYPE_CUT_VALUE;
		}
		void AddUse( int iUse )
		{
			m_iValue1 += iUse;
		}
		int GetUse()
		{
			return m_iValue1;
		}
		bool IsUse()
		{
			if( m_iValue2 == ITEM_USE_ON )
				return true;

			return false;
		}
		void SetUse( bool bUse )
		{
			if( bUse )
				m_iValue2 = ITEM_USE_ON;
			else
				m_iValue2 = ITEM_USE_OFF;
		}

		// UT_DATE: ��¥��
		SHORT GetYear()
		{
			return m_iValue1/10000;           // [2009]0715
		}
		SHORT GetMonth()
		{
			return ( m_iValue1/100 ) % 100;   //  2009[07]15
		}
		SHORT GetDay()
		{
			return m_iValue1 % 100;           //  200907[15]
		}
		// Value2���� ��¥�� ���� 4�ڸ��� ����Ѵ�. ���� �ڸ��� Ư�� �����ۿ��� ��� -
		SHORT GetHour()
		{
			return (m_iValue2 % 10000) / 100;           //  [21]23   ( 21�� 23�� )
		}
		SHORT GetMinute()
		{
			return (m_iValue2 % 10000) % 100;           //  21[23]
		}
		void SetDate( int iYear , int iMonth, int iDay, int iHour, int iMinute )
		{
			m_iValue1 = ( iYear * 10000 ) + ( iMonth * 100 ) + iDay;
			m_iValue2 = ((m_iValue2/10000)*10000) + ( ( iHour * 100 ) + iMinute );
		}

		int GetDateExcludeValue2()
		{
			return (m_iValue2 / 10000);
		}

		void SetDateExcludeValue2( int iValue2 )
		{
			int iDate = (m_iValue2 % 10000);
			m_iValue2 = (iValue2 * 10000) + iDate;
		}

		int GetDateExcludeValue3Time()
		{
			int iValue = GetDateExcludeValue2();
			return iValue % 1000;
		}

		int GetDateExcludeValue3State()
		{
			int iValue = GetDateExcludeValue2();
			return (iValue / 10000)-1;
		}

		void SetDateExcludeValue3( int iValue, int iState )
		{
			int iDate = ( m_iValue2 % 10000 );

			//999�� �̻� ���� �Ұ�
			iValue = min( 999, iValue );

			//State���
			iState = min( 9, iState );
			iState = (iState+1) * 10000;
			m_iValue2 = ( (iState + iValue) * 10000) + iDate;
		}
	};

protected:
	struct ETCITEMDB
	{
		bool     m_bChange;
		DWORD    m_dwIndex;
		ETCITEMSLOT m_EtcItem[MAX_SLOT];		
		ETCITEMDB()
		{
			m_bChange = false;
			m_dwIndex = NEW_INDEX;
			memset( m_EtcItem, 0, sizeof( m_EtcItem ) );
		}
	};
	typedef std::vector< ETCITEMDB > vETCITEMDB;
	vETCITEMDB m_vEtcItemList;

	typedef std::map< int, DWORD > StartTimeMap; // int : iType, DWORD : StartTime
	StartTimeMap m_StartTimeMap;

protected:
	void InsertDBEtcItem( ETCITEMDB &kEtcItemDB, bool bBuyCash, int iBuyPrice );
	void InsertStartTimeMap( int iType );
	void DeleteStartTimeMap( int iType );

public:
	virtual void Initialize( User *pUser );
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );	
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	void GainSpendTypeEtcItem( int iGainCode, int iGainCount );
	bool AddEtcItem( IN const ETCITEMSLOT &rkNewSlot, IN bool bBuyCash, IN int iBuyPrice, OUT DWORD &rdwIndex, OUT int &riArray );
	bool GetEtcItem( IN int iType, OUT ETCITEMSLOT &rkEtcItem );
	bool GetEtcItemByArray( IN int iArray, OUT ETCITEMSLOT &rkEtcItem );
	bool GetRowEtcItem( IN DWORD dwIndex,  OUT ETCITEMSLOT kEtcItem[MAX_SLOT] );
	int  GetEtcItemCurrentSlot();
	void SetEtcItem( const ETCITEMSLOT &rkEtcItem, int iLogType = 0 );
	void ioUserEtcItem::SetEtcItem( const ETCITEMSLOT &rkEtcItem, DWORD& dwIndex, int& iArray, int iLogType = 0 );
	bool DeleteEtcItem( int iType, int iDelLogType );
	void DeleteEtcItemZeroTime();
	void DeleteEtcItemPassedDate( OUT IntVec &rvTypeVec );

	bool SetStartTimeMap( Room *pRoom, const char *szFunction, int iType = 0/*ioEtcItem::EIT_NONE*/ ); // ioEtcItem.h �߰����� �ʱ� ���ؼ�
	void FillTimeData( SP2Packet &rkPacket, int iType = 0/*ioEtcItem::EIT_NONE*/ );
	bool UpdateTimeData( OUT IntVec &rvType, IN Room *pRoom, IN const char *szFunction, IN int iType = 0/*ioEtcItem::EIT_NONE*/ );
	void LeaveRoomTimeItem( OUT IntVec &rvType ); 
	
	bool GetEtcItemIndex( IN int iType, OUT DWORD &rdwIndex, OUT int &iFieldCnt );

	// �ð����� �����ϴ� ��í��
	void SendGashaponRealTime( int iMinute );
	void UpdateGashaponTime( int iMinute );

	BOOL HaveAThisItem(DWORD dwType);

public:
	ioUserEtcItem(void);
	virtual ~ioUserEtcItem(void);
};

#endif // __ioUserEtcItem_h__


