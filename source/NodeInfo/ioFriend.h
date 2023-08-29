
#ifndef _ioFriend_h_
#define _ioFriend_h_

#define MAX_FRIEND_IDX_VALUE		2147483647
#define SEND_FRIEND_REQUEST_LIST    10
#define MAX_BEFORE_RECEIVE_CLOVERCOUNT	2

class User;
typedef struct tagFriend
{
	int          m_iIndex;			// (db) idx
	DWORD        m_dwUserIndex;
	ioHashString m_szName;              
	int          m_iGradeLevel;
	int          m_iCampPosition;
	DWORD        m_dwRegTime;

	bool		 m_bCloverChangeState;	// save state
	int			 m_iCloverSendCount;
	int			 m_iCloverSendDate;
	int			 m_iCloverReceiveCount;
	int			 m_iCloverBeforeReceiveDate;
	int			 m_iCloverBeforeReceiveCount;

	enum
	{
		DEFAULT_YEAR    = 2000,			// Friend RegTime�� 2012 - 2000 �� �� 12 ������ ����Ǿ��ִ�;
		DATE_YEAR_VALUE = 100000000,    // ����� ������.
		DATE_MONTH_VALUE= 1000000,      // ������ ������.
		DATE_DAY_VALUE =  10000,        // �ϱ��� ������.
		DATE_HOUR_VALUE = 100,          // �ñ��� ������.
	};

	// Clover
	int GetCloverSendDate(){ return m_iCloverSendDate; }

	// Date
	DWORD GetYearRegTime(){ return DEFAULT_YEAR + ( m_dwRegTime / DATE_YEAR_VALUE ); }
	DWORD GetMonthRegTime(){ return ( m_dwRegTime % DATE_YEAR_VALUE ) / DATE_MONTH_VALUE; }
	DWORD GetDayRegTime(){	return ( m_dwRegTime % DATE_MONTH_VALUE ) / DATE_DAY_VALUE; }
	DWORD GetHourRegTime(){ return ( m_dwRegTime % DATE_DAY_VALUE ) / DATE_HOUR_VALUE; }
	DWORD GetMinuteRegTime(){ return ( m_dwRegTime % DATE_HOUR_VALUE ); }

	tagFriend()
	{
		m_iIndex		= -1;
		m_dwUserIndex   = 0;
		m_iGradeLevel	= 0;		
		m_iCampPosition = 0;
		m_dwRegTime     = 0;

		m_bCloverChangeState		= false;
		m_iCloverSendCount			= 0;
		m_iCloverSendDate			= 0;
		m_iCloverReceiveCount		= 0;
		m_iCloverBeforeReceiveDate	= 0;
		m_iCloverBeforeReceiveCount	= 0;
	}
}FRIEND;
typedef std::vector< FRIEND > vFRIEND;
typedef vFRIEND::iterator vFRIEND_iter;

enum
{	// ���� ���� Ÿ��
	BFT_NONE		= 0,    // ���� �ƴ�
	BFT_SET 		= 1,	// ���� ����
	BFT_DISMISS     = 2,    // ���� ������.
};
typedef struct tagBestFriend
{
	enum
	{
		DEFAULT_YEAR    = 2010,			// 2010���� DB�� �������� �ʴ´�. �� DateData�� �⵵�� 0�̸� 2010�̶� ���̴�. 1�̸� 2011��
		DATE_YEAR_VALUE = 100000000,    // ����� ������.
		DATE_MONTH_VALUE= 1000000,      // ������ ������.
		DATE_DAY_VALUE =  10000,        // �ϱ��� ������.
		DATE_HOUR_VALUE = 100,          // �ñ��� ������.
	};

	DWORD m_dwIndex;          // ���� �ε���
	DWORD m_dwUserIndex;      // ���� ���� �ε���
	DWORD m_dwState;		  // ���� ���� - ������.. / �뺴 �뿩 
	DWORD m_dwMagicDate;      // ���� ���� ���� �ð� - ������ ���� ��� �ð� / �뿩�� ���� �뿩 ���� �ð�

	// Date
	DWORD GetYear(){ return DEFAULT_YEAR + ( m_dwMagicDate / DATE_YEAR_VALUE ); }
	DWORD GetMonth(){ return ( m_dwMagicDate % DATE_YEAR_VALUE ) / DATE_MONTH_VALUE; }
	DWORD GetDay(){	return ( m_dwMagicDate % DATE_MONTH_VALUE ) / DATE_DAY_VALUE; }
	DWORD GetHour(){ return ( m_dwMagicDate % DATE_DAY_VALUE ) / DATE_HOUR_VALUE; }
	DWORD GetMinute(){ return ( m_dwMagicDate % DATE_HOUR_VALUE ); }

	void SetDate( CTime &rkDate )
	{
		m_dwMagicDate = ( (rkDate.GetYear() - DEFAULT_YEAR) * DATE_YEAR_VALUE ) + (rkDate.GetMonth() * DATE_MONTH_VALUE) + 
						(rkDate.GetDay() * DATE_DAY_VALUE) + (rkDate.GetHour() * DATE_HOUR_VALUE) + rkDate.GetMinute();
	}

	bool  m_bChangeData;
	tagBestFriend()
	{
		m_dwIndex = m_dwUserIndex = 0;
		m_dwState       = BFT_NONE;
		m_dwMagicDate   = 0;       
		m_bChangeData   = false;
	}
}BestFriend;
typedef std::vector< BestFriend > vBestFriend;
typedef vBestFriend::iterator vBestFriend_iter;

class ioFriend
{
protected:
	vFRIEND m_vFriendList;
	vBestFriend m_vBestFriendList;

public:
	void Initialize();

public:
	int  GetLastFriendIndex();
	int  GetFriendSize(){ return m_vFriendList.size(); }

public:
	bool IsFriend( const ioHashString &szName );
	bool IsFriend( DWORD dwTableIndex );
	bool IsFriendRegHourCheck( const ioHashString &szName, DWORD dw24HourBefore );
	int  GetFriendOnlineUserCount( DWORD dw24HourBefore );
	
public:
	void InsertFriend( int iIndex, DWORD dwUserIndex, const ioHashString &szName, int iGradeLevel, int iCampPos, DWORD dwRegTime
					, int iSendCount, int iSendDate, int iReceiveCount, int iReceiveDate, int iBeforeReceiveCount, bool bSave );
	void SetFriendCloverSendData( const DWORD dwUserIndex, const int iCount, const int iDate );
	void SetFriendBeforeReceiveCloverData( const DWORD dwUserIndex, const int iCount, const int iDate );
	void SetFriendReceiveCloverData( const DWORD dwUserIndex, const int iCount );
	void SetFriendBeforeReceiveCloverDelete( const DWORD dwUserIndex );
	void InitFriendBeforeReceiveCloverCount();

	bool DeleteFriend( const ioHashString &szName );

public:
	int          FriendLastIterSize( int iLastIndex );
	vFRIEND_iter FriendIter( int iLastIndex );
	vFRIEND_iter FriendEnd(){ return m_vFriendList.end(); }
	FRIEND		 &GetFriend( DWORD dwTableIndex );
	FRIEND		 &GetFriendToUserIndex( DWORD dwUserIndex );
	FRIEND		 &GetFriendToUserName( const ioHashString &rkName );
	DWORD        GetFriendNameToUserIndex( const ioHashString &rkName );

public:
	void FillMoveDataToBestFriend( SP2Packet &rkPacket );
	void ApplyMoveDataToBestFriend( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	int          BestFriendLastIterSize( int iLastIndex );
	vBestFriend_iter BestFriendIter( int iLastIndex );
	vBestFriend_iter BestFriendEnd(){ return m_vBestFriendList.end(); }

public:
	void  InsertBestFriend( User *pOwner, DWORD dwIndex, DWORD dwUserIndex, DWORD dwState, DWORD dwMagicDate );
	void  UpdateBestFriend( User *pOwner, DWORD dwUserIndex, DWORD dwState, DWORD dwMagicDate );
	DWORD GetBestFriendState( DWORD dwUserIndex );
	DWORD GetBestFriendState( const ioHashString &rkFriendID );
	void  CheckBestFriendToDate( DWORD dwCheckState, DWORDVec &rUserIndex );
	void  DeleteBestFriendTable( User *pOwner, DWORD dwUserIndex );
	bool  ClearBestFriend( User *pOwner, DWORD dwUserIndex, DWORD &rMagicDate );
	void  ExceptionClearBestFriend( User *pOwner, DWORD dwUserIndex, CTime &rkTime );
	DWORD SetBestFriendTime( const ioHashString &rkFriendID, CTime &rkTime );
	DWORD GetBestFriendTime( DWORD dwUserIndex );

public:
	bool  IsRentalAlreadyCheck( DWORD dwUserIndex );
	int   GetRentalCharCount();

public:
	void SaveBestFriend( User *pOwner );
	void SaveFriendClover( User *pOwner );

public:
	ioFriend();
	virtual ~ioFriend();
};

#endif