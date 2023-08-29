#pragma once

#include "ioChannelingNodeParent.h"

class ioChannelingNodeNexonBuy : public ioChannelingNodeParent
{
protected:
	virtual bool AddRequestGetCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType );
	virtual bool OnRecieveGetCash( User *pUser, SP2Packet &rkRecievePacket, DWORD dwErrorPacketID, DWORD dwErrorPacketType );
	virtual void SendCancelCash( IN SP2Packet &rkPacket );

	virtual bool AddRequestOutputCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType, IN const char *szRecvPrivateID, IN const char *szRecvPublicID, IN DWORD dwRecvUserIndex = 0);
	virtual bool CheckRequestOutputCash( User *pUser, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType );
	virtual bool CheckRequestOutputCash( User *pUser, int& iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType, DWORD dwItemCode, IntOfTwoVec& vConsumeInfo);
	virtual bool CheckReciveOutputCash( User *pUser, int iReturnValue, int iPayAmt, bool bBillingError, const ioHashString &rsBillingError, DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool AddReuestSubscriptionRetractCashPacket( IN User *pUser, OUT SP2Packet & rkSendPacket );
	virtual bool OnReceiveSubscriptionRetractCash( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual bool AddRequestFillCashUrlPacket( IN User *pUser, OUT SP2Packet &rkSendPacket );
public:
	virtual bool IsSubscriptionRetractCheck() { return false; } //kyg Wemade�� û��öȸ ��ȸ ���� 
	virtual bool IsSubscrptionRetract() { return true; } //kyg û��öȸ ���� 

public:
	virtual ChannelingType GetType();

private:
	void GetHexMD5( OUT char *szHexMD5, IN int iHexSize, IN const char *szSource );

public:
	ioChannelingNodeNexonBuy(void);
	virtual ~ioChannelingNodeNexonBuy(void);
};

 