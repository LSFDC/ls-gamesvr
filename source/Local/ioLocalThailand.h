#ifndef __ioLocalThailand_h__
#define __ioLocalThailand_h__

#include "ioLocalParent.h"

class ioLocalThailand  : public ioLocalParent
{
public:
	virtual ioLocalManager::LocalType GetType();
	virtual const char *GetTextListFileName();

	virtual bool ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize );
	virtual bool DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize );
	virtual bool IsRightTimeLoginKey( DWORD dwTotalMins );
	virtual bool IsRightLoginKey( const char *szDBKey, const char *szDecryptKey );

	virtual void ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket );
	virtual bool ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket );
	virtual bool SendLoginData( User *pUser );
	virtual bool CheckDuplication( User *pUser, const ioHashString &rsPrivateID );

	virtual void FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket );
	virtual void FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket, const char *szRecvPrivateID );
	virtual void SendRefundCash( User *pUser, int iTransactionID, bool bRefund );
	virtual bool UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType );
	virtual void SendUserInfo( User *pUser );

	virtual int  GetFirstIDMaxSize();

	virtual void GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName );

	virtual bool IsCheckKorean();
	virtual bool IsFirstIDCheckPass() { return true; }
	virtual bool IsChannelingID();

	virtual bool IsBillingTestID( const ioHashString &rsPublicID );

	virtual bool IsSamePCRoomUser();
	virtual bool IsBadPingKick( bool bLadder );
	virtual bool IsPrivateLowerID();

	virtual int GetLimitGradeLevel();
	virtual bool IsRightID( const char *szID );

	virtual const char *GetOtherComanyErrorMent();

	virtual int GetLicenseDate();

	virtual bool IsDecryptID() { return false; }
	virtual bool IsIPBonus() { return false; }
	virtual bool IsGetCashAfterPublicIP() { return false; }

	virtual bool SendTokenData( User *pUser );

	virtual void LoadINI();

public:
	ioLocalThailand(void);
	virtual ~ioLocalThailand(void);
};

#endif // __ioLocalThailand_h__