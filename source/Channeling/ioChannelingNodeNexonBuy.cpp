#include "stdafx.h"
#include "../NodeInfo/User.h"
#include "../Util/md5.h"
#include ".\ioChannelingNodeNexonBuy.h"
#include "..\Local\ioLocalParent.h"
#include "..\EtcHelpFunc.h"
#include "..\BillingRelayServer\BillingRelayServer.h"
#include "..\MainProcess.h"


ioChannelingNodeNexonBuy::ioChannelingNodeNexonBuy(void)
{
	
}

ioChannelingNodeNexonBuy::~ioChannelingNodeNexonBuy(void)
{
}

ChannelingType ioChannelingNodeNexonBuy::GetType()
{
	return CNT_NEXON;
}

bool ioChannelingNodeNexonBuy::AddRequestGetCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType )
{
	// pUser�� NULL�̴� ���� �޽����� ���� �� ����.
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserID()) );
	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserNo()) );

	return true;
}

bool ioChannelingNodeNexonBuy::OnRecieveGetCash( User *pUser, SP2Packet &rkRecievePacket, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}


	DWORD        dwUserIndex     = 0;
	ioHashString szBillingGUID;
	bool         bSetUserMouse   = false;
	int          iReturnValue    = 0;
	int          iReturnCash     = 0; 
	int			 iReturnError = 0;
	 
	bool         bBillingError   = false;
	ioHashString sBillingError;
 

	rkRecievePacket >> dwUserIndex;
	rkRecievePacket >> szBillingGUID;
	rkRecievePacket >> bSetUserMouse;
	rkRecievePacket >> iReturnValue;

	if( iReturnValue != CASH_RESULT_SUCCESS )
	{
		 rkRecievePacket >> iReturnError;
		 bBillingError = true;

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s ReturnValue is Error : %d:%s:%s[%d:%d:%s]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), pUser->GetBillingGUID().c_str(), iReturnValue, bBillingError, sBillingError.c_str() );
		pUser->ClearBillingGUID();
		SP2Packet kReturn( dwErrorPacketID );
		PACKET_GUARD_bool( kReturn.Write(dwErrorPacketType) );
		PACKET_GUARD_bool( kReturn.Write(bSetUserMouse) );
		PACKET_GUARD_bool( kReturn.Write(bBillingError) );
		if( bBillingError )
			PACKET_GUARD_bool( kReturn.Write(sBillingError) );
			
		 pUser->SendMessage( kReturn );
		 pUser->SetCash( 0 ); // �ʱ�ȭ
		 pUser->SetPurchasedCash( 0 ); // �ʱ�ȭ
		 pUser->SetChannelingCash( 0 ); // �ʱ�ȭ
		 return false;
	}

	int64 balance = 0;
	rkRecievePacket >> balance;
	iReturnCash = balance; // ������ 64byte
	
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][cash]Channeling nexon get cash success : [%d] [%s] [%d] [%d] [%d] [%d]", pUser->GetUserIndex(), pUser->GetBillingGUID().c_str(), iReturnCash, iReturnCash, pUser->GetCash(), pUser->GetPurchasedCash() );
	pUser->SetCash( iReturnCash );
	pUser->SetPurchasedCash( iReturnCash ); // ĳ���� ���� ������ ĳ���� �����ϴ�.
	pUser->SetChannelingCash( 0 );

	SP2Packet kPacket( STPK_GET_CASH );
	PACKET_GUARD_bool( kPacket.Write(GET_CASH_SUCCESS) );
	PACKET_GUARD_bool( kPacket.Write(bSetUserMouse) );
	PACKET_GUARD_bool( kPacket.Write(iReturnCash) );
	PACKET_GUARD_bool( kPacket.Write(iReturnCash) );
	pUser->SendMessage( kPacket );
	pUser->ClearBillingGUID();

	//Title check
	UserTitleInven* pInven = pUser->GetUserTitleInfo();
	if( pInven )
	{
		pInven->CheckTitleValue(TITLE_HAVE_GOLD, iReturnCash);
	}

	return true;
}

bool ioChannelingNodeNexonBuy::AddRequestOutputCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType, IN const char *szRecvPrivateID, IN const char *szRecvPublicID, IN DWORD dwRecvUserIndex )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserID()) );
	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserNo()) );

	return true;
}

bool ioChannelingNodeNexonBuy::CheckRequestOutputCash( User *pUser, int& iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType, DWORD dwItemCode, IntOfTwoVec& vConsumeInfo )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	int iCheckCash = pUser->GetCash();

	switch( dwErrorPacketID )
	{
	case STPK_PRESENT_BUY:
	case STPK_SUBSCRIPTION_BUY:
		iCheckCash = pUser->GetPurchasedCash();
		break;
	}

	if( iCheckCash < iPayAmt )
	{
		if( IsPossibleToUseBonusCash(dwErrorPacketID) )
		{
			if( GetBonusCashInfoForOutputCash(pUser, iCheckCash, iPayAmt, vConsumeInfo) )
				return true;
		}

		ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
		if( pLocal && pLocal->IsBillingTestID( pUser->GetPublicID() ) )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s IsBillingTestID is true : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
			return true;
		}

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Want of Cash : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
		SP2Packet kReturn( dwErrorPacketID );
		PACKET_GUARD_bool( kReturn.Write(dwErrorPacketType) );
		pUser->SendMessage( kReturn );
		return false;
	}

	return true;
}

bool ioChannelingNodeNexonBuy::CheckRequestOutputCash( User *pUser, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	int iCheckCash = pUser->GetCash();

	switch( dwErrorPacketID )
	{
	case STPK_PRESENT_BUY:
	case STPK_SUBSCRIPTION_BUY:
		iCheckCash = pUser->GetPurchasedCash();
		break;
	}

	if( iCheckCash < iPayAmt )
	{
		ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
		if( pLocal && pLocal->IsBillingTestID( pUser->GetPublicID() ) )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s IsBillingTestID is true : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
			return true;
		}

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Want of Cash : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
		SP2Packet kReturn( dwErrorPacketID );
		PACKET_GUARD_bool( kReturn.Write(dwErrorPacketType) );
		pUser->SendMessage( kReturn );
		return false;
	}

	return true;
}

bool ioChannelingNodeNexonBuy::CheckReciveOutputCash( User *pUser, int iReturnValue, int iPayAmt, bool bBillingError, const ioHashString &rsBillingError, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	if( iReturnValue != CASH_RESULT_SUCCESS )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s ReturnValue is Error : %d:%s:%s:%d", __FUNCTION__, iReturnValue, pUser->GetPublicID().c_str(), rsBillingError.c_str(), dwErrorPacketID );
		pUser->ClearBillingGUID();
		SP2Packet kReturn( dwErrorPacketID );

		PACKET_GUARD_bool( kReturn.Write(dwErrorPacketType) );
		PACKET_GUARD_bool( kReturn.Write(bBillingError) );

		if( bBillingError )
			PACKET_GUARD_bool( kReturn.Write(rsBillingError) );
	
		pUser->SendMessage( kReturn );
		return false;
	}
	//LOG.PrintTimeAndLog(0,"%s :: %s,",__FUNCTION__,pUser->GetPublicID().c_str());
	return true;
}


void ioChannelingNodeNexonBuy::SendCancelCash( IN SP2Packet &rkPacket )
{
	if( rkPacket.GetPacketID() != BSTPK_OUTPUT_CASH_RESULT )
		return;

	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	int          iReturnItemPrice = 0;
	DWORD        dwUserIndex = 0;
	ioHashString szPrivateID;
	int          iPayAmt  = 0;
	int          iType    = 0;
	int          iReturnValue = 0;
	int          iTransactionID = 0;
	int          iCash = 0;
	int          iPurchasedCash = 0;
	ioHashString szUserIP;
	ioHashString szChargeNo;

	rkPacket >> dwUserIndex;
	rkPacket >> szBillingGUID;
	rkPacket >> iReturnValue;
	rkPacket >> iReturnItemPrice;

	if( iReturnValue != CASH_RESULT_SUCCESS )
		return;

	rkPacket >> iType;
	rkPacket >> iPayAmt;
	rkPacket >> iTransactionID;
	int iItemValueList[ioChannelingNodeParent::MAX_ITEM_VALUE];
	for (int i = 0; i < ioChannelingNodeParent::MAX_ITEM_VALUE ; i++)
		iItemValueList[i] = 0;
	ioChannelingNodeParent::GetItemValueList( rkPacket, iType, iItemValueList );

	// Cancel Step 2
	rkPacket >> iChannelingType;
	rkPacket >> iCash;
	rkPacket >> iPurchasedCash; 
	rkPacket >> szPrivateID;
	rkPacket >> szUserIP;
	rkPacket >> szChargeNo;

	//--------------------------------SEND------------------------------------//
	char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
	SP2Packet kBillingPacket( BSTPK_CANCEL_CASH );
	// ����
	PACKET_GUARD_VOID( kBillingPacket.Write(iChannelingType) );
	PACKET_GUARD_VOID( kBillingPacket.Write(szTempGUID) );
	PACKET_GUARD_VOID( kBillingPacket.Write(dwUserIndex) );

	// Cancel Step 3
	PACKET_GUARD_VOID( kBillingPacket.Write(szPrivateID) );
	PACKET_GUARD_VOID( kBillingPacket.Write(szUserIP) );
	PACKET_GUARD_VOID( kBillingPacket.Write(szChargeNo) );

	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send Fail : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
}

bool ioChannelingNodeNexonBuy::AddRequestFillCashUrlPacket( IN User *pUser, OUT SP2Packet &rkSendPacket )
{
	// �ؽ��� ��� 
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

 	char szFillCashUrl[MAX_PATH+MAX_PATH]="";

	char szHash[MAX_PATH] = "";
	char szHKey[MAX_PATH] = "";

	char szNxid[MAX_PATH] = "";

	StringCbPrintf( szNxid, sizeof(szNxid), "%s@nx", pUser->GetChannelingUserNo().c_str() );

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szCurTime[MAX_PATH] = "";
	StringCbPrintf( szCurTime, sizeof(szCurTime), "%04d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour%12 );
	StringCbPrintf( szHash, sizeof(szHash), "LOSA%s@nx%s", pUser->GetChannelingUserNo().c_str(), szCurTime );

	GetHexMD5( szHKey, sizeof(szHKey), szHash );

	StringCbPrintf( szFillCashUrl, sizeof(szFillCashUrl), 
		"https://nxpay.nexon.com/cash/main.aspx?chid=LOSA&nxid=%s@nx&ctype=1&hkey=%s", 
		pUser->GetChannelingUserNo().c_str(), szHKey);
	
	SP2Packet kReturn( STPK_FILL_CASH_URL );
	PACKET_GUARD_bool( kReturn.Write(FILL_CASH_URL_OK) );
	PACKET_GUARD_bool( kReturn.Write(szFillCashUrl) );
	pUser->SendMessage(kReturn);

	return false;
}

bool ioChannelingNodeNexonBuy::AddReuestSubscriptionRetractCashPacket( IN User *pUser, OUT SP2Packet & rkSendPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserID()) );
	PACKET_GUARD_bool( rkSendPacket.Write(pUser->GetChannelingUserNo()) );
	
	return true;
}

bool ioChannelingNodeNexonBuy::OnReceiveSubscriptionRetractCash( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	LOG.PrintTimeAndLog(0,"%s :: %s,",__FUNCTION__,pUser->GetPublicID().c_str());
	DWORD        dwUserIndex     = 0;
	ioHashString szBillingGUID;
	DWORD		 dwIndex;
	ioHashString szSubscriptionID;
	int			 iReturnCode = 0;
	int			 iCanceledCashAmt = 0;
	int			 iRealCash = 0;
	int 		 iBonusCash = 0;//�̸� �ٲܰ� 

	PACKET_GUARD_bool( rReceivePacket.Read(dwUserIndex) );
	PACKET_GUARD_bool( rReceivePacket.Read(szBillingGUID) );
	PACKET_GUARD_bool( rReceivePacket.Read(dwIndex) );
	PACKET_GUARD_bool( rReceivePacket.Read(szSubscriptionID) );
	PACKET_GUARD_bool( rReceivePacket.Read(iReturnCode) );


	if( iReturnCode == BILLING_SUBSCRIPTION_RETRACT_RESULT_SUCCESS )
	{
		//û�� öȸ ��Ŷ (����)
		PACKET_GUARD_bool( rReceivePacket.Read(iCanceledCashAmt) );
		PACKET_GUARD_bool( rReceivePacket.Read(iRealCash) );
		PACKET_GUARD_bool( rReceivePacket.Read(iBonusCash) );

		int iResultCancelCash = pUser->GetSubscriptionGold( dwIndex, szSubscriptionID );

		SP2Packet kReturnPacket( STPK_SUBSCRIPTION_RETR );
		pUser->SetSubscriptionRetract( dwIndex, szSubscriptionID, iResultCancelCash, kReturnPacket );

		//kyg ���⿡ ��Ŷ �߰��Ұ��� �������� ���� �ʿ� 130704
		pUser->ClearBillingGUID();

		return pUser->SendMessage(kReturnPacket);
	}
	else if ( iReturnCode == BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL)
	{
		ioHashString errCode;
		
		PACKET_GUARD_bool( rReceivePacket.Read(errCode) );

		SP2Packet kPacket( STPK_SUBSCRIPTION_RETR );

		PACKET_GUARD_bool( kPacket.Write(SUBSCRIPTION_RETR_EXCEPTION) );
		PACKET_GUARD_bool( kPacket.Write(9) );

		pUser->SendMessage( kPacket );

		pUser->ClearBillingGUID();

		LOG.PrintTimeAndLog(0,"%s Error nexon Rtn ErrorCode(%d:%s)",__FUNCTION__,iReturnCode,errCode.c_str());
	}
	else
	{
		//error ��Ȳ�� 

		SP2Packet kPacket( STPK_SUBSCRIPTION_RETR );

		PACKET_GUARD_bool( kPacket.Write(SUBSCRIPTION_RETR_EXCEPTION) );
		PACKET_GUARD_bool( kPacket.Write(9) );

		pUser->SendMessage( kPacket );

		LOG.PrintTimeAndLog(0,"%s Error BillingRelayServer  Rtn WrongCode(%d)",__FUNCTION__,iReturnCode);
	}
	return true;
}

void ioChannelingNodeNexonBuy::GetHexMD5( OUT char *szHexMD5, IN int iHexSize, IN const char *szSource )
{
	enum { MAX_DIGEST = 16, };
	MD5Context md5_ctx;
	BYTE byDigest[MAX_DIGEST];

	MD5Init( &md5_ctx );
	MD5Update( &md5_ctx, (unsigned char const *)szSource, strlen( szSource ) );
	MD5Final( byDigest, &md5_ctx );

	for (int i = 0; i < MAX_DIGEST ; i++)
	{
		char szTempHex[MAX_PATH]="";
		StringCbPrintf(szTempHex, sizeof( szTempHex ), "%02x", byDigest[i]); // BYTE ĳ�����ؼ� FFFF���� �ʴ´�.
		StringCbCat( szHexMD5, iHexSize, szTempHex );	
	}
}