
#pragma once

class ioClover : public ioDBDataController
{
	enum
	{
		DEFAULT_YEAR	= 2010,			// 2010
		DATE_YEAR_VALUE	= 100000000,    // ����� ������.
		DATE_MONTH_VALUE= 1000000,      // ������ ������.
		DATE_DAY_VALUE	= 10000,        // �ϱ��� ������.
		DATE_HOUR_VALUE = 100,          // �ñ��� ������.
	};

public:
	enum
	{
		CLOVER_TYPE_DEFAULT	= 0,
		CLOVER_TYPE_CHARGE,
		CLOVER_TYPE_REFILL,
		CLOVER_TYPE_SEND,
		CLOVER_TYPE_RECEIVE,
		CLOVER_TYPE_REMOVE,
	};

	enum GIFTCLOVER
	{
		GIFT_CLOVER_DEFAULT					= 0,
		GIFT_CLOVER_LOGIN,					// �α���
		GIFT_CLOVER_CHARGE_SUCCESS,			// ���� ����
		GIFT_CLOVER_CHARGE_FAIL,			// ���� ����
		GIFT_CLOVER_REFILL,					// ��ü����
	};

	enum FRIENDCLOVER
	{
		FRIEND_CLOVER_DEFAULT						= 10,

		FRIEND_CLOVER_SEND_SUCCESS,					// Ŭ�ι� ������ ����
		FRIEND_CLOVER_SEND_FAIL_ALREADY_SEND,		// Ŭ�ι� ������ ���� - ���� �� �ִ� �ð��� ���� �ȵ���.
		FRIEND_CLOVER_SEND_FAIL_NOT_FRIEND,			// Ŭ�ι� ������ ���� - ģ���� �ƴ�.
		FRIEND_CLOVER_SEND_FAIL_FRIEND_REG_TIME,	// Ŭ�ι� ������ ���� - ģ���� ���� �� �� �� ����.
		FRIEND_CLOVER_SEND_FAIL_NOT_ENOUGH_CLOVER,	// Ŭ�ι� ������ ���� - Ŭ�ι� ����

		FRIEND_CLOVER_RECV_SUCCESS,					// Ŭ�ι� �ޱ� ����
		FRIEND_CLOVER_RECV_FAIL_NOT_CLOVER,			// Ŭ�ι� �ޱ� ���� - ���� Ŭ�ι��� ����.
		FRIEND_CLOVER_RECV_FAIL_NOT_FRIEND,			// Ŭ�ι� �ޱ� ���� - ģ���� �ƴ�.

		FRIEND_CLOVER_COME_TO_FRIEND,				// ģ���� ���� Ŭ�ι��� �����ߴ�.
	};

	enum
	{
		MAGIC_TIME = 2,
	};

private:
	bool m_bChangeData;			// ������� ������ DB ����.
	int	m_iCloverCount;			// ������ �ִ� Ŭ�ι� ����.
	int m_iLastChargeTime;		// ������ ���� �ð�.
	short m_sRemainTime;		// �����ð�(��)
	DWORD m_dwCloverCheckTime;	// Check Time

public:
	void Init();
	void Destroy();

private:
	User* GetUser(){ return m_pUser; }
	void SetUser( User* pUser ){ m_pUser = pUser; }
	const bool GetChangeData(){ return m_bChangeData; }
	void SetChangeData( const bool bState ){ m_bChangeData = bState; }
	void SetCloverCount( const int iCount );
	void SetLastChargeTime( const int iChargeTime );
	void SetMaxRemainTime();

	void IncreaseCloverCount( const int iCount );	// ����
	void DecreaseCloverCount( const int iCount );	// �Ҹ�
	void UpdateCloverCharge( const int iCount );
	void UpdateCloverRefill( const int iCount, CTime& CurrentTime );
	void SetConvertDate( CTime& CurrentTime );
	void CheckCloverCount();
	void CheckCloverRemainTime();
	const int ConvertYYMMDDToDate( const WORD wYear, const WORD wMonth, const WORD wDay );

public:
	const int GetCloverCount();
	const int GetLastChargeTime(){ return m_iLastChargeTime; }
	const short GetCloverRemainTime();

	CTime ConvertNumberToCTime( const int iDateTime );
	BOOL IsValidCharge( CTime& CurrentTime );
	void IsSaveRemainTime();

	// Ŭ�ι� ���� ������ UpdateCloverInfo() ��..
	void UpdateCloverInfo( const int iType, const int iCount, CTime& CurrentTime );

public:
	virtual void Initialize( User *pUser );
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	ioClover();
	virtual ~ioClover();
};
