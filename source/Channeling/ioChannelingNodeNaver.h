#ifndef __ioChannelingNodeNaver_h__
#define __ioChannelingNodeNaver_h__

#include "ioChannelingNodeParent.h"

class ioChannelingNodeNaver : public ioChannelingNodeParent
{
protected:
	virtual bool AddRequestGetCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType );
	virtual bool OnRecieveGetCash( User *pUser, SP2Packet &rkRecievePacket, DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool AddRequestOutputCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType, IN const char *szRecvPrivateID, IN const char *szRecvPublicID, IN DWORD dwRecvUserIndex = 0);
	virtual bool CheckRequestOutputCash( User *pUser, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType );
	virtual bool CheckRequestOutputCash( User *pUser, int& iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType, DWORD dwItemCode, IntOfTwoVec& vConsumeInfo );
	virtual bool CheckReciveOutputCash( User *pUser, int iReturnValue, int iPayAmt, bool bBillingError, const ioHashString &rsBillingError, DWORD dwErrorPacketID, DWORD dwErrorPacketType );

public:
	virtual bool IsSubscriptionRetractCheck() { return false; } //kyg naver�� ���� ���� ���� 
	virtual bool IsSubscrptionRetract() { return false; } //kyg û��öȸ ������ 

public:
	virtual ChannelingType GetType();

public:
	ioChannelingNodeNaver(void);
	virtual ~ioChannelingNodeNaver(void);
};

#endif // __ioChannelingNodeNaver_h__