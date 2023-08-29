#ifndef __ioChannelingNodeParent_h__
#define __ioChannelingNodeParent_h__

class User;
class UserBonusCash;

// �ѱ� ä�θ� ���񽺴� ä�θ� ID ������ ����� ( private id�� ä�θ� ID�� ������� ���� )
class ioChannelingNodeParent
{
public: 
	enum 
	{
		MAX_ITEM_VALUE = 5,
	};
public:
	virtual bool AddRequestGetCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType ) = 0;
	virtual bool OnRecieveGetCash( User *pUser, SP2Packet &rkRecievePacket, DWORD dwErrorPacketID, DWORD dwErrorPacketType ) = 0;

	virtual bool AddRequestOutputCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType, IN const char *szRecvPrivateID, IN const char *szRecvPublicID, IN DWORD dwRecvUserIndex = 0) = 0;
	virtual bool CheckRequestOutputCash( User *pUser, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType ) = 0;
	virtual bool CheckRequestOutputCash( User *pUser, int& iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType, DWORD dwItemCode, IntOfTwoVec& vConsumeInfo) = 0;
	virtual bool CheckReciveOutputCash( User *pUser, int iReturnValue, int iPayAmt, bool bBillingError, const ioHashString &rsBillingError, DWORD dwErrorPacketID, DWORD dwErrorPacketType ) = 0;
	virtual bool UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual void SendCancelCash( IN SP2Packet &rkPacket ) {}
	virtual void SendAddCash( IN User *pUser, IN int iAddCash, IN DWORD dwEtcItemType ) {}
	virtual bool OnRecieveAddCash( User *pUser, SP2Packet &rkRecievePacket ) { return true; };

	static void GetItemValueList( IN SP2Packet &rkPacket, IN int iType, OUT int iItemValueList[MAX_ITEM_VALUE] );
	static void SetItemValueList( OUT SP2Packet &rkPacket, IN int iType, IN const int iItemValueList[MAX_ITEM_VALUE] );

	virtual bool AddRequestFillCashUrlPacket( IN User *pUser, OUT SP2Packet &rkSendPacket ){ return false; }

	virtual bool AddReuestSubscriptionRetractCashCheckPacket( IN User *pUser, OUT SP2Packet & rkSendPacket );
	virtual bool OnReceiveSubscriptionRetractCashCheck( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool AddReuestSubscriptionRetractCashPacket( IN User *pUser, OUT SP2Packet & rkSendPacket );
	virtual bool OnReceiveSubscriptionRetractCash( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool AddReuestSubscriptionRetractCashFailPacket( IN User *pUser, OUT SP2Packet & rkSendPacket );
	virtual bool OnReceiveSubscriptionRetractCashFail( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool IsSubscriptionRetractCheck() = 0;
	virtual bool IsSubscrptionRetract() = 0;
	virtual bool IsPossibleToUseBonusCash(DWORD dwPacketID)
	{
		if( (dwPacketID == STPK_PRESENT_BUY) || (dwPacketID == STPK_CHAR_EXTEND) || (dwPacketID == STPK_CHAR_CHANGE_PERIOD) )
			return false;

		return true;
	}
	// ������ ���ŷ� ��� �� ���ʽ� ĳ��
	BOOL GetBonusCashInfoForOutputCash(User* pUser, const int iCurCash, int& iGoodsPrice, IntOfTwoVec& vConsumeInfo)
	{
		if( !pUser )
			return FALSE;

		int iBonusCash = pUser->GetAmountOfBonusCash();
		
		//HRYOON BONUS CASH ��ĳ��+���ʽ�ĳ�� >= �����۰���
		if( iCurCash + iBonusCash >= iGoodsPrice )
		{
			//HRYOON BONUS CASH ���ʽ� ĳ�� ��뿩�� 
			if( iGoodsPrice > iCurCash )
			{
				//HRYOON BONUS CASH ����� ���ʽ� ĳ�� ���
				iBonusCash	= iGoodsPrice - iCurCash;
				iGoodsPrice	= iCurCash;			// �������� ������ ��ĳ��

				UserBonusCash* pInfo = pUser->GetUserBonusCash();		//?
				if( pInfo )
				{
					pInfo->GetMoneyForConsume(vConsumeInfo, iBonusCash);		

					int iSize	= vConsumeInfo.size();
					int iVal	= 0;

					for( int i = 0; i < iSize; i++ )
						iVal += vConsumeInfo[i].value2;		//���ݾ�
				
					if( iVal == iBonusCash )				//���ݾ� == ����ؾ��� �ݾ� ������ OK
						return TRUE;

				}
			}
			else
				return TRUE;		//���ʽ� ĳ�� �����ص� �Ǵ� ���
		}

		return FALSE;
	}

public:
	virtual ChannelingType GetType();

public:
	ioChannelingNodeParent(void);
	virtual ~ioChannelingNodeParent(void);
};

#endif // __ioChannelingNodeParent_h__