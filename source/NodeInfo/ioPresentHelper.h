#ifndef __ioPresentHelper_h__
#define __ioPresentHelper_h__

#include "../Util/Singleton.h"
#include "../Util/IORandom.h"

#include "ioEtcItemHelp.h"

class User;

class ioPresentHelper : public Singleton< ioPresentHelper >
{
protected:
	DWORD m_dwProcessTime;

	// ���ϸ��� ��� ������~�� �̺�Ʈ
	struct OneDayGoldItemData
	{
		DWORD m_dwRand;
		bool  m_bAlarm;
		ioHashString m_szSendID;
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		OneDayGoldItemData()
		{
			m_dwRand = 0;
			m_bAlarm = false;
			m_iPresentType = 0;
			m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
		}
	};
	typedef std::vector< OneDayGoldItemData > vOneDayGoldItemData;
	vOneDayGoldItemData m_vOneDayGoldItemDataList;
	DWORD               m_dwOneDayGoldItemSeed;
	IORandom            m_OneDayGoldItemRandom;

protected:
	typedef std::map< DWORD, int > vRandTestMap;
	vRandTestMap m_RandTestMap;

protected:
	// �޸����� ComeBack ���ʽ� �̺�Ʈ
	struct DormancyUserData
	{
		ioHashString m_szSendID;
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		DormancyUserData()
		{
			m_iPresentType = 0;
			m_iPresentValue1 = m_iPresentValue2 = 0;
			m_iPresentValue3 = m_iPresentValue4 = 0;
		}
	};
	typedef std::vector< DormancyUserData > vDormancyUserData;
	vDormancyUserData m_vDormancyUserDataList;

protected:
	// ����� �û�� : ���� ���ʽ�
	struct SpacialAward
	{
		DWORD m_dwRand;
		bool  m_bAlarm;

		ioHashString m_szSendID;
		short m_iPresentType;
		short m_iPresentState;
		short m_iPresentMent;
		int   m_iPresentPeriod;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		SpacialAward()
		{
			m_dwRand = 0;
			m_bAlarm = false;
			m_iPresentType = m_iPresentState = m_iPresentMent = 0;
			m_iPresentPeriod = m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
		}
	};
	typedef std::vector< SpacialAward > vSpacialAward;
	vSpacialAward m_vSpacialAwardList;
	DWORD         m_dwSpacialAwardSeed;
	IORandom      m_SpacialAwardRandom;


	// ���� ��� Ư����
	struct MonsterAwardList
	{
		vSpacialAward m_AwardList;
		DWORD m_dwRandomSeed;
		
		MonsterAwardList()
		{
			m_dwRandomSeed = 0;
		}
	};

	typedef std::map< DWORD, MonsterAwardList > MonsterAwardMap;
	MonsterAwardMap m_MonsterAwardMap;
	IORandom m_MonsterAwardRandom;

	struct AwardTimeInfo
	{
		float m_fMinRate;
		float m_fMaxRate;
		DWORD m_dwAwardCode;

		AwardTimeInfo()
		{
			m_fMinRate = 0.0f;
			m_fMaxRate = 1.0f;
			m_dwAwardCode = 0;
		}
	};
	typedef std::vector< AwardTimeInfo > vAwardTimeInfoList;
	vAwardTimeInfoList m_vAwardTimeInfoList;


protected:
	// PvE ��� ���� ���� ���½� ���� ������
	struct MonsterPresent
	{
		DWORD m_dwRand;

		ioHashString m_szSendID;
		short m_iPresentType;
		short m_iPresentState;
		short m_iPresentMent;
		int   m_iPresentPeriod;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		int   m_iPresentSendCnt;

		MonsterPresent()
		{
			m_dwRand = 0;
			m_iPresentType = m_iPresentState = m_iPresentMent = 0;
			m_iPresentPeriod = m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
			m_iPresentSendCnt = 0;
		}
	};
	typedef std::vector< MonsterPresent > vMonsterPresent;
	
	struct MonsterPresentList
	{
		vMonsterPresent m_PresentList;
		DWORD           m_dwRandomSeed;
		MonsterPresentList()
		{
			m_dwRandomSeed = 0;
		}
	};

	typedef std::map< DWORD, MonsterPresentList > MonsterPresentMap;
	MonsterPresentMap m_MonsterPresentMap;
	IORandom          m_MonsterPresentRandom;
	bool              m_bAbuseMonsterPresent;


protected:
	// ��í�� ���Ѿ����� ��ǰ ����
	struct GashaponPresent
	{
		DWORD m_dwRand;
		bool  m_bWholeAlarm;
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		int   m_iPresentPeso;
		GashaponPresent()
		{
			m_dwRand       = 0;
			m_bWholeAlarm  = false;
			m_iPresentType = 0;
			m_iPresentValue1 = 0;
			m_iPresentValue2 = 0;
			m_iPresentValue3 = 0;
			m_iPresentValue4 = 0;
			m_iPresentPeso   = 0;
		}
	};

	class GashaponPresentSort : public std::binary_function< const GashaponPresent&, const GashaponPresent&, bool >
	{
	public:
		bool operator()( const GashaponPresent &lhs , const GashaponPresent &rhs ) const
		{
			if( lhs.m_dwRand < rhs.m_dwRand )
				return true;
			return false;
		}
	};

	typedef std::vector< GashaponPresent > vGashaponPresent;

	struct GashaponPresentInfo
	{
		vGashaponPresent    m_vGashaponPresentList;
		DWORD               m_dwGashaponPresentSeed;
		IORandom            m_GashaponPresentRandom;

		DWORD               m_dwEtcItemType;  
		ioHashString		m_szGashaponSendID;
		short				m_iGashaponAlarm;
		short				m_iGashaponMent;
		int					m_iGashaponPeriod; 

		GashaponPresentInfo()
		{
			m_vGashaponPresentList.clear();
			m_dwGashaponPresentSeed = 0;
			m_dwEtcItemType         = 0;
			m_iGashaponAlarm        = 0;
			m_iGashaponMent         = 0;
			m_iGashaponPeriod       = 0;

		}
	};
	
	typedef std::vector< GashaponPresentInfo > vGashaponPresentInfo;
	vGashaponPresentInfo m_vGashaponPresentInfoList;

protected:
	// ���� ������ ��Ű���� ����
	struct EtcItemPackagePresent
	{
		DWORD m_dwEtcItemType;
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		short m_iPresentMent;

		EtcItemPackagePresent()
		{
			m_dwEtcItemType = 0;
			m_iPresentType  = 0;
			m_iPresentValue1= 0;
			m_iPresentValue2= 0;
			m_iPresentValue3= 0;
			m_iPresentValue4= 0;
			m_iPresentMent  = 0;
		}
	};
	typedef std::vector< EtcItemPackagePresent > vEtcItemPackagePresent;
	vEtcItemPackagePresent m_vEtcItemPackagePresent;
	ioHashString           m_szEtcItemPackageSendID;
	short                  m_iEtcItemPackageAlarm;
	int                    m_iEtcItemPackagePeriod;

protected:
	// �̺�Ʈ�� ����
	struct EventPresent
	{
		DWORD m_dwEventType;
		DWORD m_dwRand;
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		short m_iPresentMent;
		bool  m_bAlwaysPresent;
		int   m_iPresentReceiveType; // �Էµ� Ÿ�԰� ������ Ÿ���� ��� ������ �ش�. 0�̸� �������� ����.
		int   m_iPresentSendCnt;     // ������ ������ ���� ( ��񺸱� 2�� ���޽õ� )

		EventPresent()
		{
			m_dwEventType   = 0;
			m_dwRand        = 0;
			m_iPresentType  = 0;
			m_iPresentValue1= 0;
			m_iPresentValue2= 0;
			m_iPresentValue3= 0;
			m_iPresentValue4= 0;
			m_iPresentMent  = 0;
			m_bAlwaysPresent= false;
			m_iPresentReceiveType = 0;
			m_iPresentSendCnt     = 0;
		}
	};

	struct EventTypeRand
	{
		DWORD         m_dwEventType;
		DWORD         m_dwEventPresentSeed;
		IORandom      m_EventPresentRandom;
		
		EventTypeRand()
		{
			m_dwEventType        = 0;
			m_dwEventPresentSeed = 0;
		}
	};

	typedef std::vector< EventTypeRand* > vEventTypeRand;
	typedef std::vector< EventPresent > vEventPresent;
	vEventPresent  m_vEventPresent;
	vEventTypeRand m_vEventTypeRand;
	
	ioHashString  m_szEventSendID;
	short         m_iEventAlarm;
	int           m_iEventPeriod;

protected:
	// ����
	struct SpecialFishing
	{
		DWORD m_dwRand;
		bool  m_bAlarm;

		ioHashString m_szSendID;
		short m_iPresentType;
		short m_iPresentState;
		short m_iPresentMent;
		int   m_iPresentPeriod;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;

		SpecialFishing()
		{
			m_dwRand = 0;
			m_bAlarm = false;
			m_iPresentType = m_iPresentState = m_iPresentMent = 0;
			m_iPresentPeriod = m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
		}
	};
	typedef std::vector< SpecialFishing > vSpecialFishing;
	vSpecialFishing m_vSpecialFishingList;
	DWORD         m_dwSpecialFishingSeed;
	IORandom      m_SpecialFishingRandom;

	vSpecialFishing m_vSpecialFishingPCRoomList;
	DWORD         m_dwSpecialFishingPCRoomSeed;
	IORandom      m_SpecialFishingPCRoomRandom;

	vSpecialFishing m_vEventSpecialFishingList;
	DWORD         m_dwEventSpecialFishingSeed;
	IORandom      m_EventSpecialFishingRandom;

	vSpecialFishing m_vGuildSpecialFishingList;
	DWORD         m_dwGuildSpecialFishingSeed;
	IORandom      m_GuildSpecialFishingRandom;

protected:
	// ���ñ�
	struct EventFishing
	{
		ioHashString m_szSendID;

		int m_iPresentNum;

		short m_iPresentType;
		short m_iPresentState;
		short m_iPresentMent;

		int   m_iPresentPeriod;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;

		EventFishing()
		{
			m_iPresentNum = 0;
			m_iPresentType = m_iPresentState = m_iPresentMent = 0;
			m_iPresentPeriod = m_iPresentValue1 = m_iPresentValue2 = m_iPresentValue3 = m_iPresentValue4 = 0;
		}
	};
	typedef std::vector< EventFishing > vEventFishing;
	vEventFishing m_vEventFishingPresent;

	IORandom m_EventFishingRandom;


protected:
	// RandomDeco
	struct RandomDecoPresent
	{
		DWORD m_dwRand;
		bool  m_bAlarm;

		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;
		int   m_iPresentPeso;
		
		RandomDecoPresent()
		{
			m_dwRand       = 0;
			m_bAlarm  = false;
			m_iPresentType = 0;
			m_iPresentValue1 = 0;
			m_iPresentValue2 = 0;
			m_iPresentValue3 = 0;
			m_iPresentValue4 = 0;
			m_iPresentPeso   = 0;
		}
	};
	typedef std::vector< RandomDecoPresent > vRandomDecoPresent;
	vRandomDecoPresent  m_vRandomDecoPresentListM;
	DWORD               m_dwRandomDecoPresentSeedM;
	IORandom            m_RandomDecoPresentRandomM;

	ioHashString m_szRandomDecoPresentSendIDM;
	short        m_iRandomDecoPresentAlarmM;
	short        m_iRandomDecoPresentMentM;
	int          m_iRandomDecoPresentPeriodM;

	vRandomDecoPresent  m_vRandomDecoPresentListW;
	DWORD               m_dwRandomDecoPresentSeedW;
	IORandom            m_RandomDecoPresentRandomW;

	ioHashString m_szRandomDecoPresentSendIDW;
	short        m_iRandomDecoPresentAlarmW;
	short        m_iRandomDecoPresentMentW;
	int          m_iRandomDecoPresentPeriodW;


// ��ǰ ���Ž� ����/////////////////////////////////////
public:
	enum  // m_iBuyType
	{
		BT_NONE        = 0,
		BT_SOLDIER     = 1,
		BT_DECO        = 2,
		BT_ETC         = 3,
		BT_PESO        = 4,
		BT_EXTRAITEM   = 5,
		BT_EXTRA_BOX   = 6,
		BT_RANDOM_DECO = 7,
		BT_GRADE_EXP   = 8,
		BT_MEDALITEM   = 9,
	};
protected:
	struct BuyPresent
	{
		DWORD m_dwEventType;
		short m_iBuyType;     // 1.�뺴,  2.ġ��, 3.����,  4.���, 5.���, 6,��񺸱�
		int   m_iBuyValue1;   // �뺴:(�뺴Ÿ��), ġ��(ITEMSLOT�� m_item_type), ����(ETCITEMSLOT�� m_iType), ���(��ұݾ�), ���(����ڵ�)
		int   m_iBuyValue2;   // �뺴:(�뺴�Ⱓ), ġ��(ITEMSLOT�� m_item_code), ����(ETCITEMSLOT�� m_iValue1), ���(NONE), ���(( ��� ���尪 * 10000 ) + ���Ⱓ)
		bool  m_bPassBuyValue1;
		bool  m_bPassBuyValue2;
		short m_iPresentType;
		short m_iPresentMent;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		int   m_iPresentValue3;
		int   m_iPresentValue4;

		BuyPresent()
		{
			m_dwEventType   = 0;
			m_iBuyType      = 0;
			m_iBuyValue1    = 0;
			m_iBuyValue2    = 0;
			m_bPassBuyValue1= false;
			m_bPassBuyValue2= false;
			m_iPresentType  = 0;
			m_iPresentMent  = 0;
			m_iPresentValue1= 0;
			m_iPresentValue2= 0;
			m_iPresentValue3= 0;
			m_iPresentValue4= 0;
		}
	};

	typedef std::vector< BuyPresent > vBuyPresent;
	vBuyPresent            m_vBuyPresent;
	ioHashString           m_szBuyPresentSendID;
	short                  m_iBuyPresentAlarm;
	int                    m_iBuyPresentPeriod;
////////////////////////////////////////////////////

// for user -> user present ////////////////////////
protected:
		int    m_iCanUserPresentCnt;
		short  m_iUserPresentAlarm;
		int    m_iUserPresentPeriod;
		int    m_iUserPresentMent;
		int    m_iUserPresentBonusPesoMent;
		int    m_iUserPresentBonusItemMent;
		int	   m_iUserPresentPopupNoMent;
////////////////////////////////////////////////////
// û��öȸ
protected:
	int m_iCanUserSubscriptionCnt;
	int m_iUserSubscriptionPeriod;

// For Trade
protected:
	short m_iTradePresentAlarm;
	short m_iTradeSellPresentAlarm;

	int m_iTradePresentPeriod;
	int m_iTradePresentBuyMent;
	int m_iTradePresentSellMent;
	int m_iTradePresentCancelMent;
	int m_iTradePresentTimeOutMent;

// For Alchemic
protected:
	short m_iAlchemicPresentAlarm;

	int m_iAlchemicPresentPeriod;
	int m_iAlchemicPresentSoldierMent;
	int m_iAlchemicPresentItemMent;
	int m_iAlchemicPresentPieceMent;

// for extraitem package //////////////////////////
protected:
		struct ExtraItemPackagePresent
		{
			int   m_iMachineCode;
			short m_iPresentType;
			int   m_iPresentValue1;
			int   m_iPresentValue2;
			int   m_iPresentValue3;
			int   m_iPresentValue4;
			short m_iPresentMent;

			ExtraItemPackagePresent()
			{
				m_iMachineCode  = 0;
				m_iPresentType  = 0;
				m_iPresentValue1= 0;
				m_iPresentValue2= 0;
				m_iPresentValue3= 0;
				m_iPresentValue4= 0;
				m_iPresentMent  = 0;
			}
		};
		typedef std::vector< ExtraItemPackagePresent > vExtraItemPackagePresent;
		vExtraItemPackagePresent m_vExtraItemPackagePresent;
		ioHashString             m_szExtraItemPackageSendID;
		short                    m_iExtraItemPackageAlarm;
		int                      m_iExtraItemPackagePeriod;
///////////////////////////////////////////////////

//  [4/29/2011 ��ȣ]
//  ���� �̻� ��� ���� 1�����ϸ��� ���� ����
protected:
    struct HighGradePresent
	{
		DWORD			m_dwRand;
		ioHashString	m_szSendID;
		short			m_iPresentType;
		int				m_iPresentValue1;
		int				m_iPresentValue2;
		int				m_iPresentValue3;
		int				m_iPresentValue4;
		short			m_iPresentMent;
		short			m_iPresentState;
		int             m_iPresentPeriod;
		HighGradePresent()
		{
			m_dwRand		= 0;
			m_iPresentType	= 0;
			m_iPresentValue1= 0;
			m_iPresentValue2= 0;
			m_iPresentValue3= 0;
			m_iPresentValue4= 0;
			m_iPresentMent  = 0;
			m_iPresentState = 0;
			m_iPresentPeriod= 0;
		}
	};
	typedef std::vector< HighGradePresent > HighGradePresentVec;
	struct HighGradePresentList
	{
		DWORD m_dwRandSeed;
		HighGradePresentVec m_PresentList;
		HighGradePresentList()
		{
			m_dwRandSeed = 0;
		}
	};
	typedef std::map< DWORD, HighGradePresentList > HighGradePresentMap;
	HighGradePresentMap m_HighGradePresentMap;
	IORandom            m_HighGradePresentRandom;

protected:
	void LoadOneDayEvent( ioINILoader &rkLoader );
	void LoadDormancyUserEvent( ioINILoader &rkLoader );
	void LoadSpacialAward( ioINILoader &rkLoader );
	void LoadMonsterPresent( ioINILoader &rkLoader );
	void LoadGashaponPresent( ioINILoader &rkLoader );

	void LoadEtcItemPackagePresent( ioINILoader &rkLoader );
	void LoadEventPresent( ioINILoader &rkLoader );
	void LoadMonsterSpecialAward( ioINILoader &rkLoader );
	void LoadBuyPresent( ioINILoader &rkLoader );
	void LoadUserPresent( ioINILoader &rkLoader );
	void LoadRandomDecoPresentM( ioINILoader &rkLoader );
	void LoadRandomDecoPresentW( ioINILoader &rkLoader );
	void LoadExtraItemPackagePresent( ioINILoader &rkLoader );
	void LoadTradePresent( ioINILoader &rkLoader );
	void LoadAlchemicPresent( ioINILoader &rkLoader );
	void LoadUserSubscription( ioINILoader &rkLoader );

	void LoadSpecialFishing( ioINILoader &rkLoader );			// �Ϲݻ��¿��� Ư���� ����
	void LoadEventSpecialFishing( ioINILoader &rkLoader );		// �̺�Ʈ���¿��� Ư���� ����
	void LoadSpecialFishingPCRoom( ioINILoader &rkLoader );		// �Ǿ�����¿��� Ư���� ����
	void LoadEventFishingPresent( ioINILoader &rkLoader );		// ���ñ� �̺�Ʈ
	void LoadGuildSpecialFishing( ioINILoader &rkLoader );

	void LoadHighGradePresent( ioINILoader &rkLoader );

protected:
	void RandTestMapInsert( DWORD dwCheckID );
	void PrintRandTestMap(); 

protected:
	EventTypeRand *GetEventTypeRand( DWORD dwEventType );
	GashaponPresentInfo *GetGashaponPresentInfo( DWORD dwEtcItemType );

public: 
	void LoadINI();
	void CheckNeedReload();

public:
	int GetOneDayEventPresent( ioHashString &rSendID, short &rPresentType, int &rPresentValue1, int &rPresentValue2, int &rPresentValue3, int &rPresentValue4, bool &rAlarm );

public:
	int GetMaxDormancyUserPresent();
	int GetDormancyUserPresent( int iArray, ioHashString &rSendID, short &rPresentType, int &rPresentValue1, int &rPresentValue2, int &rPresentValue3, int &rPresentValue4 );

public:
	bool SendSpacialAwardPresent( User *pSendUser, SP2Packet &rkPacket );
	bool SendMonsterSpacialAwardPresent( User *pSendUser, float fCurRate, SP2Packet &rkPacket );
	bool SendMonsterPresent( User *pSendUser, DWORD dwPresentCode, bool bAbuseTime );
	bool SendGashaponPresent( User *pSendUser, DWORD dwEtcItemType, short &riPresentType, int &riPresentValue1, int &riPresentValue2, int &riPresentValue3, int &riPresentValue4, bool &rbWholeAlarm, int &riPresentPeso );

#if defined( SRC_OVERSEAS )
	// �Ϲ� ��í ��Ű�� ���� ������ (��ũ��)		JCLEE 140718
	void SendAllGashaponPresent( User *pSendUser, DWORD dwEtcItemType );
#endif

	void SendGashponPresentList( User *pSendUser, DWORD dwEtcItemType );
	bool SendEtcItemPackagePresent( User *pSendUser, DWORD dwEtcItemType );
	bool SendEventPresent( User *pSendUser, DWORD dwEventType, int iPresentReceiveType = 0 );
	bool SendBuyPresent( User *pSendUser, DWORD dwEventType, short iBuyType, int iBuyValue1, int iBuyValue2, int iBonusMileage = 0 );
	bool SendRandomDecoPresent( User *pSendUser, short &riPresentType, int &riPresentValue1, int &riPresentValue2, int &riPresentValue3, int &riPresentValue4, bool &rbWholeAlarm, int &riPresentPeso, bool bMan );
	bool SendExtraItemPackagePresent( User *pSendUser, int iMachineCode );

	bool SendSpecialFishingPresent( User *pSendUser, SP2Packet &rkPacket );	// Ư���� ����
	bool SendEventFishingPresent( User *pSendUser, int iPresentNum, SP2Packet &rkPacket );	// ���ñ� �̺�Ʈ

	// Trade
	bool SendPresentByTradeItemBuy( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, DWORD dwRecvUserIndex,
									DWORD dwItemType, DWORD dwItemMagicCode, DWORD dwItemValue, DWORD dwItemMaleCustom, DWORD dwItemFemaleCustom, ioHashString &szRegisterUserNick );
	bool SendPresentByTradeItemSell( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, DWORD dwRecvUserIndex,
									 int iItemPrice, ioHashString &szRegisterUserNick );
	bool SendPresentByTradeCancel( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, DWORD dwRecvUserIndex,
								   DWORD dwItemType, DWORD dwItemMagicCode, DWORD dwItemValue, DWORD dwItemMaleCustom, DWORD dwItemFemaleCustom, ioHashString &szRegisterUserNick );

	bool SendPresentByTradeTimeOut( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, DWORD dwRecvUserIndex,
									DWORD dwItemType, DWORD dwItemMagicCode, DWORD dwItemValue, DWORD dwItemMaleCustom, DWORD dwItemFemaleCustom, ioHashString &szRegisterUserNick );

	// Alchemic
	bool SendPresentByAlchemicSoldier( User *pSendUser, DWORD dwClassType, DWORD dwPeriodValue );
	bool SendPresentByAlchemicItem( User *pSendUser, DWORD dwItemCode, int iLimitTime, int iReinforce );
	bool SendPresentByAlchemicItem( User *pSendUser, int iCode, int iCnt );

	int CheckSpecialFishingPresent( User *pUser );	// Test��

	bool GetHighGradePresent( DWORD dwGrade, ioHashString &rkSendID, short &rPresentType, int &rPresentValue1, int &rPresentValue2,
							  int &rPresentValue3, int &rPresentValue4, short &rPresentMent, short &rPresentState, int &rPresentPeriod );

	void SendAwardEtcItemBonus( User *pSendUser, DWORD dwEtcItemType, int iEtcItemCount );

public:
	// for user -> user present
	const int GetCanPresentCnt() const { return m_iCanUserPresentCnt; }
	
	int GetSubscriptionType( short iPresentType, int iBuyValue1, int iBuyValue2 );
	int GetCash( short iPresentType, int iBuyValue1, int iBuyValue2 );
	int GetBonusPeso( short iPresentType, int iBuyValue1, int iBuyValue2 );

	bool InsertUserPresent( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, const ioHashString &rszPublicID, const char *szPublicIP, DWORD dwRecvUserIndex, short iPresentType, int iBuyValue1, int iBuyValue2, bool bBonusMent, bool bBuyToPresent, bool bPopup = false, bool bPresentEvent = false );
	bool InsertUserPresentByBuyPresent( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwEventType, DWORD dwSendUserIndex, const ioHashString &rszPublicID, const char *szPublicIP, DWORD dwRecvUserIndex, short iPresentType, int iBuyValue1, int iBuyValue2, bool bPresentEvent );
	//

	const int GetCanSubscriptionCnt() const { return m_iCanUserSubscriptionCnt; }

	bool InsertUserSubscription( User *pUser, const ioHashString& szSubscriptionID, int iSubscriptionGold, int iBonusCash, short iPresentType,
								 int iBuyValue1, int iBuyValue2 );

	bool InsertUserPopupItem( const DWORD dwAgentID, const DWORD dwThreadID, DWORD dwSendUserIndex, const ioHashString &rszPublicID, const char *szPublicIP, int iPresentType, int iBuyValue1, int iBuyValue2, bool bBonus );

protected:
	DWORD GetAwardCode( float fCurRate );

public:
	bool SendRandPresentBySelectGashapon( User* pUser, DWORD dwEtcItemType, IN const SelectGashaponValueVec& vSelect, DWORD dwErrorPacketID, DWORD dwErrorType, short& m_iType, int& m_iValue1, int& m_iValue2 );

	// rising gashapon
	bool SendRandPresentByRisingGashapon( User* pUser, DWORD dwEtcItemType, DWORD dwErrorPacketID, DWORD dwErrorType, short& m_iType, int& m_iValue1, int& m_iValue2, int& m_iGashaponIndex );
	void SendRisingPresentProcess(User* pUser, GashaponPresentInfo * pInfo, GashaponPresent & rkPresent);
public:
	static ioPresentHelper& GetSingleton();

public:
	ioPresentHelper();
	virtual ~ioPresentHelper();
};

#define g_PresentHelper ioPresentHelper::GetSingleton()

#endif // __ioPresentHelper_h__

