
#ifndef _ioUserPresent_h_
#define _ioUserPresent_h_

#define PRESENT_SOLDIER             1
#define PRESENT_DECORATION          2
#define PRESENT_ETC_ITEM            3
#define PRESENT_PESO                4
#define PRESENT_EXTRAITEM           5
#define PRESENT_EXTRAITEM_BOX       6
#define PRESENT_RANDOM_DECO			7
#define PRESENT_GRADE_EXP           8
#define PRESENT_MEDALITEM           9
#define PRESENT_ALCHEMIC_ITEM		10
#define PRESENT_PET_ITEM			11
#define PRESENT_COSTUME				12
#define PRESENT_BONUS_CASH			13
#define PRESENT_ACCESSORY			14
#define MAX_PRESENT_TYPE            15

// Ư���� üũ�ؾ��� ��Ʈ Ÿ�Ը� ����
#define PRESENT_TRADE_BUY_MENT		37
#define PRESENT_TRADE_SELL_MENT		38
#define PRESENT_TRADE_CANCEL_MENT	39
#define PRESENT_TRADE_TIMEOUT_MENT	40

// PresetType2 ���� ����
#define PRESENT_EXTRAITEM_DIVISION_1      100000000
#define PRESENT_EXTRAITEM_DIVISION_2      10000

#define PRESENT_INDEX_MEMORY            0
class User;
class CQueryResultData;

class ioUserPresent
{
public:
	enum
	{
		PRESENT_STATE_NORMAL = 0,      //
		PRESENT_STATE_NEW    = 1,      //
		PRESENT_STATE_DELETE = 2,      //
	};

protected:
	User *m_pUser;

protected:
	struct PresentData
	{
		DWORD m_dwIndex;                   // ���� DB �ε���.         // 
		DWORD m_dwSlotIndex;               // ���� SLOT �ε���.       //
		DWORD m_dwSendUserIndex;           // ���� ���� �ε���.
		ioHashString m_szSendID;           // ���� ����
		short   m_iPresentType;            // ( 1.�뺴 - 2.ġ�� - 3.���� - 4.��� - 5.��� )
		short   m_iPresentMent;            // ( ��ƮŸ��(Ŭ���̾�Ʈ���� ����) )
		short   m_iPresentState;           // ���ο� ���� 1, �̹� ���� ���� 0, ������ ���� 2,
		int     m_iPresentValue1;          // �뺴:(�뺴Ÿ��), ġ��,����ġ��(ITEMSLOT�� m_item_type), ����(ETCITEMSLOT�� m_iType), ���(��ұݾ�), ���(����ڵ�), ������(�̱��ȣ)
		int     m_iPresentValue2;          // �뺴:(�뺴�Ⱓ), ġ��,����ġ��(ITEMSLOT�� m_item_code), ����(ETCITEMSLOT�� m_iValue1), ���(NONE), ���((Ȯ�� Ÿ�� * 100000000) + ( ��� ���尪 * 10000 ) + ���Ⱓ)
		                                   // ������(00[00]:1���,0��� | [00]00:���Array 1����99 -1 ���� ���� array ���� )
		int     m_iPresentValue3;          // ���:(������Ų)
		int     m_iPresentValue4;          // ���:(������Ų)
		DWORD   m_dwLimitDate;             // ���� �Ⱓ ���� : 0909141200(2009��9��14��12�ñ��� ���� ���� ����), 0�̸� ������ ����
		PresentData()
		{
			m_dwIndex = m_dwSlotIndex = 0;
			m_dwSendUserIndex = 0;
			m_iPresentType = m_iPresentMent = m_iPresentState = 0;
			m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
			m_dwLimitDate  = 0;
		}
	};
	typedef std::vector< PresentData > vPresentData;
	vPresentData m_vPresentList;
	vPresentData m_vTempMemoryData;
	DWORD m_dwLastDBIndex;
	DWORD m_dwLastSlotIndex;
public:
	enum DeletePresentType
	{
		DPT_TIMEOVER = 1,
		DPT_RECV     = 2,
		DPT_SELL     = 3,
	};

public:
	void Initialize( User *pUser );

	// DB Load
public:
	void DBtoPresentData( CQueryResultData *query_data );	

public:
	DWORD GetLastPresentDBIndex();
	DWORD GetLastPresentSlotIndex();

protected:
	ioUserPresent::PresentData &GetPresentData( DWORD dwIndex, DWORD dwSlotIndex );
	bool ComparePresentData( ioUserPresent::PresentData &kData );

public:
	bool DeletePresentData( DWORD dwIndex, DWORD dwSlotIndex, DeletePresentType eDeletePresentType );
	bool PresentRecv( IN SP2Packet &rkRecvPacket, OUT SP2Packet &rkPacket );
	void PresentSell( DWORD dwIndex, DWORD dwSlotIndex, SP2Packet &rkPacket );

	BOOL HaveAThisItem(DWORD dwType, DWORD dwItemCode);

public:
	void AddPresentMemory( const ioHashString &szSendName, short iPresentType, int iPresentValue1, int iPresentValue2, int iPresentValue3, 
						   int iPresentValue4, short iPresentMent, CTime &rkLimitTime, short iPresentState );
	void SendPresentMemory();
	void LogoutMemoryPresentInsert();     // ���� �α� �ƿ��ÿ� ����

public:
	void CheckPresentLimitDate();

public:
	void AllDeleteData();

public:
	void FillMoveData( SP2Packet &rkPacket );
	void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	ioUserPresent();
	virtual ~ioUserPresent();
};

#endif