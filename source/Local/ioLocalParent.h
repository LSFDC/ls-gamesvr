#ifndef __ioLocalParent_h__
#define __ioLocalParent_h__

#include "ioLocalManager.h"

class SP2Packet;
class User;

class ioLocalParent
{
protected:
	ioHashString m_sGuildMaster;
	ioHashString m_sGuildSecondMaster;
	ioHashString m_sGuildGeneral;
	ioHashString m_sDuplicationMent;
	ioHashString m_sExitingServerMent;
	ioHashString m_sGuildBuilder;
	typedef std::map< ioHashString, DWORD > EUCountryCCUMap;

	EUCountryCCUMap m_EUCountryCCUMap;		// ���� ������ ccu 

public:
	virtual ioLocalManager::LocalType GetType() = 0;
	virtual const char *GetTextListFileName() = 0;

	virtual bool ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize ) = 0;
	virtual bool DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize ) = 0;
	virtual bool IsRightTimeLoginKey( DWORD dwTotalMins ) = 0;
	virtual bool IsRightLoginKey( const char *szDBKey, const char *szDecryptKey ) = 0;
	
	virtual void ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket ) = 0;
	virtual bool ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket ) { return true; }
	virtual void ApplyRequestLogin( IN SP2Packet &rkPacket, OUT ioHashString &rsValue1, OUT ioHashString &rsValue2 ){}
	virtual bool SendLoginData( User *pUser ) = 0;
	virtual bool SendLogoutData( User *pUser ) { return true; }
	virtual bool CheckDuplication( User *pUser, const ioHashString &rsPrivateID ){ return true; }
	virtual bool SendTokenData( User *pUser ) { return true; }

	virtual void FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket ) = 0;
	virtual void FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket , const char *szRecvPrivateID ){}
	virtual void FillRequestLogin( OUT SP2Packet &rkPacket, IN ioHashString &rsValue1, IN ioHashString &rsValue2 ){}
	virtual void SendRefundCash( User *pUser, int iTransactionID, bool bRefund ) = 0;
	virtual void SendCancelCash( IN SP2Packet &rkPacket ) {}
	virtual bool UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType );

	virtual void FillSubscriptionRetractCheckCash( IN User *pUser, IN SP2Packet &rkPacket ) {};
	virtual void FillSubscriptionRetractCash( IN User *pUser, IN SP2Packet &rkPacket ) {};

	virtual void SendUserInfo( User *pUser ) = 0;

	virtual int  GetFirstIDMaxSize() = 0;

	virtual const char *GetGuildMasterPostion();
	virtual const char *GetGuildSecondMasterPosition();
	virtual const char *GetGuildBuilderPosition();
	virtual const char *GetGuildGeneralPosition();
	virtual void  GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName ) = 0;

	virtual bool IsCheckKorean() = 0;
	virtual bool IsFirstIDCheckPass() { return false; }
	virtual bool IsChannelingID() = 0;

	virtual bool IsBillingTestID( const ioHashString &rsPublicID ) = 0;

	virtual bool IsSamePCRoomUser() = 0;
	virtual bool IsBadPingKick( bool bLadder ) = 0;
	virtual bool IsPrivateLowerID() { return false; }

	virtual int GetLimitGradeLevel() = 0; // -1�̸� limit�� ����.
	virtual bool IsRightID( const char *szID ) { return true; }

	virtual const char *GetDuplicationMent();
	virtual const char *GetExitingServerMent();
	virtual const char *GetOtherComanyErrorMent(){ return ""; }
	virtual bool IsRightLicense();
	virtual int GetLicenseDate() { return 0; }
	virtual bool IsFirstSoldierSelect(){ return false; }
	virtual bool IsDecryptID() { return true; }
	virtual bool IsMileage() { return false; }
	virtual bool IsPCRoomCheck() { return false; }
	virtual bool IsIPBonus() { return false; }
	virtual bool IsGetCashAfterPublicIP() { return false; }
	virtual bool IsRunUserShutDown() { return false; }
	virtual bool IsFriendRecommend() { return false; }
	virtual bool IsRightNewID( const char *szID ) { return true; }
	virtual bool GetCCUCountSend() { return true; }

	virtual void LoadINI();


	virtual void IncreaseCountryCCU( const ioHashString &rsCountryCode ) {};
	virtual void DecreaseCountryCCU( const ioHashString &rsCountryCode ) {};
	virtual void FillCountryCCU( OUT SP2Packet &rkPacket ) {};

public:
	ioLocalParent(void);
	virtual ~ioLocalParent(void);
};

#endif // __ioLocalParent_h__
