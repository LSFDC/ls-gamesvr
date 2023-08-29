
#ifndef _ioUserSubscription_h_
#define _ioUserSubscription_h_

class User;
class CQueryResultData;

class ioUserSubscription
{
public:
	enum
	{
		SUBSCRIPTION_STATE_NORMAL,
		SUBSCRIPTION_STATE_RETRACT_WAIT,
	};

	enum DeleteSubscriptionType
	{
		DST_TIMEOVER = 1,
		DST_RECV     = 2,
		DST_RETR     = 3,
	};

public:
	struct SubscriptionData
	{
		DWORD m_dwIndex;					// DB �ε���

		ioHashString m_szSubscriptionID;
		int m_iSubscriptionGold;

		short   m_iPresentType;            // ( 1.�뺴 - 2.ġ�� - 3.���� - 4.��� - 5.��� )
		int     m_iPresentValue1;          // �뺴:(�뺴Ÿ��), ġ��,����ġ��(ITEMSLOT�� m_item_type), ����(ETCITEMSLOT�� m_iType), ���(��ұݾ�), ���(����ڵ�), ������(�̱��ȣ)
		int     m_iPresentValue2;          // �뺴:(�뺴�Ⱓ), ġ��,����ġ��(ITEMSLOT�� m_item_code), ����(ETCITEMSLOT�� m_iValue1), ���(NONE), ���((Ȯ�� Ÿ�� * 100000000) + ( ��� ���尪 * 10000 ) + ���Ⱓ)
		                                   // ������(00[00]:1���,0��� | [00]00:���Array 1����99 -1 ���� ���� array ���� )
		short m_iSubscriptionState;
		DWORD   m_dwLimitDate;             // �Ⱓ ���� : 0909141200(2009��9��14��12�ñ��� ���� ���� öȸ�Ұ�), 0�̸� ������ ��ǰ
		int m_iRetractGold;
		int m_iUsedBonusCash;

		SubscriptionData()
		{
			m_dwIndex = 0;

			m_iSubscriptionGold = 0;
			m_iRetractGold = 0;
			m_iUsedBonusCash	= 0;

			m_iPresentType = 0;
			m_iPresentValue1 = m_iPresentValue2 = 0;
			m_dwLimitDate = 0;
			m_iSubscriptionState = SUBSCRIPTION_STATE_NORMAL;
		}
	};

protected:
	User *m_pUser;
	
	typedef std::vector< SubscriptionData > vSubscriptionData;
	vSubscriptionData m_vSubscriptionList;
	vSubscriptionData m_vTempMemoryData;
	
	DWORD m_dwLastDBIndex;

public:
	void Initialize( User *pUser );

	// DB Load
public:
	void DBtoSubscriptionData( CQueryResultData *query_data );	

public:
	DWORD GetLastSubscriptionDBIndex();

protected:
	ioUserSubscription::SubscriptionData& GetSubscriptionData( DWORD dwIndex, const ioHashString& szSuscriptionID );
	bool DeleteSubscriptionData( DWORD dwIndex, const ioHashString& szSuscriptionID, DeleteSubscriptionType eDeleteSubscriptionType );
	bool CompareSubscriptionData( SubscriptionData &kData );

public:
	bool CheckExistSubscriptionData( DWORD dwIndex, const ioHashString& szSuscriptionID );
	bool CheckSubscriptionState( DWORD dwIndex, const ioHashString& szSuscriptionID );

	void SubscriptionRecv( IN SP2Packet &rkRecvPacket, OUT SP2Packet &rkPacket );
	void SubscriptionRetr( DWORD dwIndex, const ioHashString& szSuscriptionID, int iGold, SP2Packet &rkPacket, bool bDefaultGold );

	bool CheckLimitData( DWORD dwIndex, const ioHashString& szSuscriptionID );

	void SetSubscriptionState( DWORD dwIndex, const ioHashString& szSuscriptionID, short iState );
	void SetRetractGold( DWORD dwIndex, const ioHashString& szSuscriptionID, int iGold );

	int GetSubscriptionGold( DWORD dwIndex, const ioHashString& szSuscriptionID );
	int GetRetractGold( DWORD dwIndex, const ioHashString& szSuscriptionID );

	int GetDBRetractGold(DWORD dwIndex, const ioHashString& szSuscriptionID );
	int GetUsedBonusCash(DWORD dwIndex, const ioHashString& szSuscriptionID );

public:
	void AddSubscriptionMemory( DWORD dwUserIndex, const ioHashString &szSubscriptionID, int iSubscriptionGold,
								short iPresentType, int iPresentValue1, int iPresentValue2,
								CTime &rkLimitTime );
	void SendSubscriptionMemory();
	void LogoutMemorySubscriptionInsert();     // ���� �α� �ƿ��ÿ� ����

public:
	void SubscriptionMileageSend(SubscriptionData &rkData);

public:
	int GetSubscriptionDataCnt();

	void FillMoveData( SP2Packet &rkPacket );
	void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	ioUserSubscription();
	virtual ~ioUserSubscription();
};

#endif