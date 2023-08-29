#ifndef __TradeSyncManger_h__
#define __TradeSyncManger_h__

#include "../Util/Singleton.h"

enum { MAX_SEND_ITEM = 200, };

// ������ ����Ʈ ���� Ÿ�� 
enum
{
	TRADE_ITEM_LIST_FIRST	= 0,	// ó�� ���۵Ǵ� ������ ����Ʈ
	TRADE_ITEM_LIST_SENDING	= 1,	// ���� ���� ������ ����Ʈ
	TRADE_ITEM_LIST_LAST	= 2,	// ������ ������ ����Ʈ
};

class TradeSyncManager : public Singleton< TradeSyncManager > 
{
protected:
	struct TradeSyncInfo
	{
		DWORD m_dwUserIndex;
		DWORD m_dwTradeIndex;
		DWORD m_dwItemType;
		DWORD m_dwItemMagicCode;
		DWORD m_dwItemValue;
		
		DWORD m_dwItemMaleCustom;
		DWORD m_dwItemFemaleCustom;
		__int64 m_iItemPrice;
		DWORD m_dwRegisterDate1;
		DWORD m_dwRegisterDate2;

		DWORD m_dwRegisterPeriod;

		TradeSyncInfo()
		{
			m_dwUserIndex = 0;
			m_dwTradeIndex = 0;
			m_dwItemType = 0;
			m_dwItemMagicCode = 0;
			m_dwItemValue = 0;

			m_dwItemMaleCustom = 0;
			m_dwItemFemaleCustom = 0;
			m_iItemPrice = 0;
			m_dwRegisterDate1 = 0;
			m_dwRegisterDate2 = 0;

			m_dwRegisterPeriod = 0;
		}
	};
	
 	typedef std::vector< TradeSyncInfo > vTradeSyncInfo;
	typedef std::map< DWORD, TradeSyncInfo > mapTradeSyncInfo;
	mapTradeSyncInfo m_mapTradeSyncInfo;	// ���� ���� �޸� �� ��� �ִ� ������ ����Ʈ

public:
	TradeSyncManager();
	virtual ~TradeSyncManager();

	void Init();
	void Destroy();

public:
	// Ŭ�� ��û�� ���� ����
	void SendTradeItemList( User *pUser );

	// ���μ����� ���� �޴� �ŷ��� ������ ��ü ����Ʈ
	void RecvAllTradeItem( SP2Packet& rkPacket );
	
	// ���� ������ ���� ���� �ŷ��� ������ �߰� ����
	void RecvAddTradeItem( SP2Packet& rkPacket );

	// ���� ������ ���� ���� �ŷ��� ������ ���� ����
	void RecvDelTradeItem( SP2Packet& rkPacket );

public:
	static TradeSyncManager& GetSingleton();
};

#define g_TradeSyncMgr TradeSyncManager::GetSingleton()

#endif