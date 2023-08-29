
#pragma once

struct stRewardData;

class ioBingo : public ioDBDataController
{
public:
	ioBingo();
	virtual ~ioBingo();
	
	void Init();
	void Destroy();

public:
	virtual void Initialize( User *pUser );
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

	// set db table
	void DBtoData_Number( CQueryResultData *query_data );
	void DBtoData_Present( CQueryResultData *query_data );

	enum
	{
		MAX				= 5,
		ALL_BINGO		= 12,	// ���� ����.
		PRESENT_COUNT	= 13,	// ��ü �������� = ���� 12 + �ú��� 1
		FLAG_VALUE		= 100,
	};

	enum
	{
		PRESENT_POS_0	= 0,	// across ��
		PRESENT_POS_1	= 1,	// 1��
		PRESENT_POS_2	= 2,	// 2��
		PRESENT_POS_3	= 3,	// 3��
		PRESENT_POS_4	= 4,	// 4��
		PRESENT_POS_5	= 5,	// 5��
		PRESENT_POS_6	= 6,	// cross ��
		PRESENT_POS_7	= 7,	// 1��
		PRESENT_POS_8	= 8,	// 2��
		PRESENT_POS_9	= 9,	// 3��
		PRESENT_POS_10	= 10,	// 4��
		PRESENT_POS_11	= 11,	// 5��

		PRESENT_POS_12	= 12,	// �ú���..
	};

	enum
	{
		LOG_CHOICE		= 0,
		LOG_FREE_CHOICE	= 1,
	};

	enum
	{
		LOG_NUMBER_MISS	= 0,
		LOG_NUMBER_HIT	= 1,
	};

	enum
	{
		RESTART_ROLLING     = 0,	// �������, ���� �ʱ�ȭ.
		RESTART_NO_ROILLING = 1,	// �α���
	};

	struct BingoNumber
	{
		BYTE m_Number;
		int m_BingoDuumyCode;

		BingoNumber()
		{
			Clear();
		}

		void Clear()
		{
			m_Number = 0;
			m_BingoDuumyCode = 0;
		}
	};

private:
	BOOL m_bChangeNumber;
	BOOL m_bChangePresent;
	BOOL m_bSendPresentState;
	BingoNumber m_BingoBoard[ MAX ][ MAX ];	// üũ�Ǿ������ +100
	BYTE m_present[ PRESENT_COUNT ];	    // üũ�Ǿ������ +100
	int  m_BingoDuumy[ MAX ][ MAX ];
	

public:
	User* GetUser(){ return m_pUser; }
	void SetUser( User* pUser ){ m_pUser = pUser; }
	BOOL GetChangeNumber(){ return m_bChangeNumber; }
	void SetChangeNumber( BOOL state ){ m_bChangeNumber = state; }
	BOOL GetChangePresent(){ return m_bChangePresent; }
	void SetChangePresent( BOOL state ){ m_bChangePresent = state; }
	
	void InitAll();
	void InitBingoNumber();
	void InitBingoPresent();
	void ResetBingoPresent();
	void GetBingoNumberData( BYTE (*pArray)[ MAX ] );
	void GetBingoPresentData( BYTE* pArray );

	// ��ȣ 1�� ����
	void ChoiceNumber( int selectNumber, int iType = 0 );

	// ���� ���� ����
	int FreeChoiceNumber();

	// ���� �ڼ���
	bool RandomShuffleNumber();

	// ������ ����
	void RandomShufflePresent();

private:
	void CheckBingo();
	void CheckPresent( const int index );
	void SendPresent( stRewardData& presentInfo );
	void SetSendPresentState( const BOOL bState ){ m_bSendPresentState = bState; }
	BOOL GetSendPresentState(){ return m_bSendPresentState; }

public:
	void OnBingoStart( User* pUser );
	void OnBingoNumberInitialize( User* pUser );
	void OnBingoALLInitialize( User* pUser );

	void FillBingoData( SP2Packet& rkPacket, const int iRollingType );

public:
	void SendBingoType( User* pUser );
};
