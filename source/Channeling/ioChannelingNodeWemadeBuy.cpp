#include "stdafx.h"
#include "../NodeInfo/User.h"
#include ".\ioChannelingNodeWemadeBuy.h"
#include "..\Local\ioLocalParent.h"
#include "..\EtcHelpFunc.h"
#include "..\BillingRelayServer\BillingRelayServer.h"
#include "..\MainProcess.h"
#include "../NodeInfo/ioEtcItem.h"

ioChannelingNodeWemadeBuy::ioChannelingNodeWemadeBuy(void)
{
	
}

ioChannelingNodeWemadeBuy::~ioChannelingNodeWemadeBuy(void)
{
}

ChannelingType ioChannelingNodeWemadeBuy::GetType()
{
	return CNT_WEMADEBUY;
}

bool ioChannelingNodeWemadeBuy::AddRequestGetCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType )
{
	// pUser�� NULL�̴� ���� �޽����� ���� �� ����.

	return true;
}

bool ioChannelingNodeWemadeBuy::OnRecieveGetCash( User *pUser, SP2Packet &rkRecievePacket, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
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
	int          iPurchasedCash  = 0;
	bool         bBillingError   = false;
	ioHashString sBillingError;

	rkRecievePacket >> dwUserIndex;
	rkRecievePacket >> szBillingGUID;
	rkRecievePacket >> bSetUserMouse;
	rkRecievePacket >> iReturnValue;

	if( iReturnValue != CASH_RESULT_SUCCESS )
	{
		rkRecievePacket >> bBillingError;
		if( bBillingError )
			rkRecievePacket >> sBillingError;

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s ReturnValue is Error : %d:%s:%s[%d:%d:%s]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), pUser->GetBillingGUID().c_str(), iReturnValue, bBillingError, sBillingError.c_str() );
		pUser->ClearBillingGUID();
		SP2Packet kReturn( dwErrorPacketID );
		kReturn << dwErrorPacketType;
		kReturn << bSetUserMouse;
		kReturn << bBillingError;
		if( bBillingError )
			kReturn << sBillingError;
		pUser->SendMessage( kReturn );
		pUser->SetCash( 0 ); // �ʱ�ȭ
		pUser->SetPurchasedCash( 0 ); // �ʱ�ȭ
		pUser->SetChannelingCash( 0 ); // �ʱ�ȭ
		return false;
	}

	rkRecievePacket >> iReturnCash;		//��ĳ�� + ���ʽ� ĳ��
	rkRecievePacket >> iPurchasedCash;	//��ĳ��

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[info][cash]Channeling wemade get cash success : [%d] [%s] [%d] [%d] [%d] [%d]", pUser->GetUserIndex(), pUser->GetBillingGUID().c_str(), iReturnCash, iPurchasedCash, pUser->GetCash(), pUser->GetPurchasedCash() );
	pUser->SetCash( iReturnCash );

	pUser->SetPurchasedCash( iPurchasedCash );
	pUser->SetChannelingCash( 0 );

	SP2Packet kPacket( STPK_GET_CASH );
	kPacket << GET_CASH_SUCCESS;
	kPacket << bSetUserMouse;
	kPacket << iReturnCash;
	kPacket << iPurchasedCash;
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

bool ioChannelingNodeWemadeBuy::AddRequestOutputCashPacket( IN User *pUser, OUT SP2Packet &rkSendPacket, IN DWORD dwErrorPacketID, IN DWORD dwErrorPacketType, IN const char *szRecvPrivateID, IN const char *szRecvPublicID, IN DWORD dwRecvUserIndex )
{
	return true;

	rkSendPacket << g_App.GetCSPort();

	// �����ΰ�� ���� ���� ������ �ִ�
	if( szRecvPrivateID != NULL && 
		szRecvPublicID != NULL )
	{
		ioHashString sRecvPrivateID = szRecvPrivateID; // ��Ŷ�� const char*�� �ٷ� ���� �� ���
		ioHashString sRecvPublicID  = szRecvPublicID;  // ��Ŷ�� const char*�� �ٷ� ���� �� ���
		rkSendPacket << sRecvPrivateID;
		rkSendPacket << sRecvPublicID;
		rkSendPacket << dwRecvUserIndex;
		
	}

	return true;
}

bool ioChannelingNodeWemadeBuy::CheckRequestOutputCash( User *pUser, int& iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType, DWORD dwItemCode, IntOfTwoVec& vConsumeInfo )
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
		iCheckCash = pUser->GetPurchasedCash();
	case STPK_ETCITEM_BUY:
		{
			if( ioEtcItem::EIT_ETC_TIME_CASH == dwItemCode )
				iCheckCash = pUser->GetPurchasedCash();
		}
		break;
	}

	//HRYOON BONUS CASH �������� ��ĳ�� ���� -> ���ʽ� ĳ�� ���
	if( iCheckCash < iPayAmt )
	{
#ifdef LSKR
		//HRYOON BONUS CASH ���ʽ� ĳ�� ���Ű��ɿ���( ����X, �뺴�Ⱓ����X )
		if( IsPossibleToUseBonusCash(dwErrorPacketID) )
		{
			//HRYOON BONUS CASH ����ε������� ������ / ����ε������� ������
			if( GetBonusCashInfoForOutputCash(pUser, iCheckCash, iPayAmt, vConsumeInfo) )
				return true;
			//iRCheckCash = iCheckCash;
		}
#endif

		ioLocalParent *pLocal = g_LocalMgr.GetLocal( ioLocalManager::GetLocalType() );
		if( pLocal && pLocal->IsBillingTestID( pUser->GetPublicID() ) )
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s IsBillingTestID is true : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
			return true;
		}

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Want of Cash : %d:%s:%d [%d:%d]", __FUNCTION__, pUser->GetUserIndex(), pUser->GetPublicID().c_str(), iPayAmt, dwErrorPacketID, iCheckCash );
		SP2Packet kReturn( dwErrorPacketID );
		kReturn << dwErrorPacketType;
		pUser->SendMessage( kReturn );
		return false;
	}

	return true;
}

bool ioChannelingNodeWemadeBuy::CheckRequestOutputCash( User *pUser, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
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
		kReturn << dwErrorPacketType;
		pUser->SendMessage( kReturn );
		return false;
	}

	return true;
}

bool ioChannelingNodeWemadeBuy::CheckReciveOutputCash( User *pUser, int iReturnValue, int iPayAmt, bool bBillingError, const ioHashString &rsBillingError, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}
	return true;
	if( iReturnValue != CASH_RESULT_SUCCESS )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s ReturnValue is Error : %d:%s:%s:%d", __FUNCTION__, iReturnValue, pUser->GetPublicID().c_str(), pUser->GetBillingGUID().c_str(), dwErrorPacketID );
		pUser->ClearBillingGUID();
		SP2Packet kReturn( dwErrorPacketID );
		if( iReturnValue == CASH_RESULT_ERROR_EXCESS_BUY )
		{
			if( dwErrorPacketID == STPK_CHAR_CREATE )
				dwErrorPacketType = CREATE_CHAR_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_CHAR_DECORATION_BUY )
				dwErrorPacketType = CHAR_DECORATION_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_CHAR_EXTEND )
				dwErrorPacketType = CHAR_EXTEND_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_ETCITEM_BUY )
				dwErrorPacketType = ETCITEM_BUY_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_CHAR_CHANGE_PERIOD )
				dwErrorPacketType = CHAR_CHANGE_PERIOD_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_EXTRAITEM_BUY )
				dwErrorPacketType = EXTRAITEM_BUY_BILLING_EXCESS_BUY;
			else if( dwErrorPacketID == STPK_PRESENT_BUY )
				dwErrorPacketType = PRESENT_BUY_BILLING_EXCESS_BUY;
#ifdef POPUPSTORE
			else if( dwErrorPacketID == STPK_POPUP_ITEM_BUY_RESULT )
				dwErrorPacketType = POPUP_ITEM_BILLING_EXCESS_BUY;
#endif	
		}
		kReturn << dwErrorPacketType;
		kReturn << bBillingError;
		if( bBillingError )
			kReturn << rsBillingError;
		pUser->SendMessage( kReturn );
		return false;
	}
	
	if( pUser->GetCash() < iPayAmt )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Want of Cash : %s:%s:%d:%d:%d", __FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetBillingGUID().c_str(), pUser->GetCash(), iPayAmt, dwErrorPacketID );
		pUser->ClearBillingGUID();
		SP2Packet kReturn( dwErrorPacketID );
		kReturn << dwErrorPacketType;
		pUser->SendMessage( kReturn );
		return false;
	}

	return true;
}


void ioChannelingNodeWemadeBuy::SendCancelCash( IN SP2Packet &rkPacket )
{
	if( rkPacket.GetPacketID() != BSTPK_OUTPUT_CASH_RESULT )
		return;

	int          iChannelingType = 0;
	ioHashString szBillingGUID;
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
	int			 iReturnItemPrice = 0;

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
	kBillingPacket << iChannelingType;
	kBillingPacket << szTempGUID;
	kBillingPacket << dwUserIndex;

	// Cancel Step 3
	kBillingPacket << szPrivateID;
	kBillingPacket << szUserIP;
	kBillingPacket << szChargeNo;

	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send Fail : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
}

bool ioChannelingNodeWemadeBuy::AddRequestFillCashUrlPacket( IN User *pUser, OUT SP2Packet &rkSendPacket )
{

	return true;
}

bool ioChannelingNodeWemadeBuy::AddReuestSubscriptionRetractCashCheckPacket( IN User *pUser, OUT SP2Packet & rkSendPacket )
{ //kyg �־���ϴ��� ��� 
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	rkSendPacket << pUser->GetPublicIP();

	return true;
}

bool ioChannelingNodeWemadeBuy::OnReceiveSubscriptionRetractCashCheck( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	DWORD		 dwUserIndex = 0;
	ioHashString szBillingGUID;
	DWORD		 dwIndex = 0;
	ioHashString szSubscriptionID;
	int			 iReturnCode;
	DWORD		 dwChargedAmt = 0; // ��ü ��� �ݾ� 
	DWORD		 dwRealChargedAmt = 0; 
	DWORD		 dwBonusChargedAmt = 0;
	DWORD		 dwCanceledAmt = 0; // �߿� û�� öȸ�� ���� ���� ��ü�ݾ�
	DWORD		 dwRealCash = 0; // dwCanceledAmt ���� ��¥ ĳ�� 
	DWORD		 dwBonusCash = 0; // dwCAncncledAmt ���� ���� ���� ĳ�� 

	PACKET_GUARD_bool( rReceivePacket.Read(dwUserIndex) );
	PACKET_GUARD_bool( rReceivePacket.Read(szBillingGUID) );
	PACKET_GUARD_bool( rReceivePacket.Read(dwIndex) );
	PACKET_GUARD_bool( rReceivePacket.Read(szSubscriptionID) );
	PACKET_GUARD_bool( rReceivePacket.Read(iReturnCode) );

	// û�� öȸ ��ȸ �Ϸ� //Index, SubscriptionID, SubscriptionGold, RetractGold
	if( iReturnCode == BILLING_SUBSCRIPTION_RETRACT_RESULT_SUCCESS )
	{
		PACKET_GUARD_bool( rReceivePacket.Read(dwChargedAmt) );
		PACKET_GUARD_bool( rReceivePacket.Read(dwRealChargedAmt) );
		PACKET_GUARD_bool( rReceivePacket.Read(dwBonusChargedAmt) );
		PACKET_GUARD_bool( rReceivePacket.Read(dwCanceledAmt) );
		PACKET_GUARD_bool( rReceivePacket.Read(dwRealCash) );
		PACKET_GUARD_bool( rReceivePacket.Read(dwBonusCash) );

		int iRetractGold = dwRealCash+dwBonusCash;
		pUser->SetRetractGold( dwIndex, szSubscriptionID, iRetractGold );

		SP2Packet kPacket( STPK_SUBSCRIPTION_RETR_CHECK );

		PACKET_GUARD_bool( kPacket.Write(SUBSCRIPTION_RETR_CHECK_OK) );
		PACKET_GUARD_bool( kPacket.Write(dwIndex) );
		PACKET_GUARD_bool( kPacket.Write(szSubscriptionID) );
		PACKET_GUARD_bool( kPacket.Write(dwRealChargedAmt) );
		PACKET_GUARD_bool( kPacket.Write(dwRealCash) );

		pUser->ClearBillingGUID();

		return pUser->SendMessage( kPacket );
	}
	else if ( iReturnCode == BILLING_SUBSCRIPTION_RETRACT_RESULT_FAIL )
	{
		SP2Packet kPacket( STPK_SUBSCRIPTION_RETR_CHECK );

		PACKET_GUARD_bool( kPacket.Write(SUBSCRIPTION_RETR_CHECK_EXCEPTION) );
		PACKET_GUARD_bool( kPacket.Write(9) );

		pUser->ClearBillingGUID();

		pUser->SendMessage( kPacket );

		ioHashString errorMsg;

		PACKET_GUARD_bool( rReceivePacket.Read(errorMsg) );

		LOG.PrintTimeAndLog(0, "%s Fail WeMadeyBuyServer Rtn ErrCode (%s)",__FUNCTION__,errorMsg.c_str());	
	}
	else
	{
		SP2Packet kPacket( STPK_SUBSCRIPTION_RETR_CHECK );

		PACKET_GUARD_bool( kPacket.Write(SUBSCRIPTION_RETR_CHECK_EXCEPTION) );
		PACKET_GUARD_bool( kPacket.Write(9) );

		pUser->ClearBillingGUID();

		pUser->SendMessage( kPacket );

		LOG.PrintTimeAndLog(0,"%s Fail BillingRelayServer Rtn WrongCode(%d)",__FUNCTION__,iReturnCode);
	}
		return true;
}

bool ioChannelingNodeWemadeBuy::AddReuestSubscriptionRetractCashPacket( IN User *pUser, OUT SP2Packet & rkSendPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	rkSendPacket << pUser->GetPublicIP();

	return true;
}

bool ioChannelingNodeWemadeBuy::OnReceiveSubscriptionRetractCash( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
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

		int iResultCancelCash = pUser->GetRetractGold( dwIndex, szSubscriptionID );

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

		LOG.PrintTimeAndLog(0,"%s Error WemadeBuyServer Rtn ErrorCode(%d:%s)",__FUNCTION__,iReturnCode,errCode.c_str());
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

bool ioChannelingNodeWemadeBuy::AddReuestSubscriptionRetractCashFailPacket( IN User *pUser, OUT SP2Packet & rkSendPacket )
{
	return true;
}

bool ioChannelingNodeWemadeBuy::OnReceiveSubscriptionRetractCashFail( IN User *pUser, OUT SP2Packet& rReceivePacket, IN DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	//
	return true;
}
