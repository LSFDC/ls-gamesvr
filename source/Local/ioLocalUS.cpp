#include "stdafx.h"
#include ".\iolocalus.h"
#include <strsafe.h>
#include "../NodeInfo/User.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../EtcHelpFunc.h"
#include "../MainProcess.h"
#include "../Channeling/ioChannelingNodeParent.h"
#include "../NodeInfo/UserNodeManager.h"

ioLocalUS::ioLocalUS(void)
{
}

ioLocalUS::~ioLocalUS(void)
{
}

ioLocalManager::LocalType ioLocalUS::GetType()
{
	return ioLocalManager::LCT_US;
}

const char * ioLocalUS::GetTextListFileName()
{
	return "text_us.txt";
}
bool ioLocalUS::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
{
	// extract ID
	char szCipherID[ENC_ID_NUM_PLUS_ONE]="";
	int iLength = rsEncLoginKeyAndID.Length();
	int iCipherIDCnt = 0;
	for (int i = ENC_LOGIN_KEY_NUM; i < iLength ; i++)
	{
		szCipherID[iCipherIDCnt] = rsEncLoginKeyAndID.At(i);
		iCipherIDCnt++;
		if(iCipherIDCnt >= ENC_ID_NUM_PLUS_ONE)
			break;
	}

	// save Enc login key
	StringCbCopyN(szEncLoginKey, iEncLoginKeySize, rsEncLoginKeyAndID.c_str(), ENC_LOGIN_KEY_NUM );

	if( iPrivaiteIDSize != DATA_LEN )
		return false;

	if(!ioEncrypted::Decode15(szCipherID, (char*)g_App.GetSecondKey().c_str(), szPrivateID))
		return false;

	return true;
}

bool ioLocalUS::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return ioEncrypted::Decode15((char*)szEncLoginKey, (char*)szUserKey, szDecLoginKey );
}

/*
bool ioLocalUS::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
{
	return true;
}
// �̱������� ���������� ���ؼ� �α��� ������ �ϹǷ� ������ true�� ���� 
bool ioLocalUS::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return true;
}
*/
bool ioLocalUS::IsRightLoginKey( const char *szDBKey, const char *szDecryptKey )
{
	return true;
}

bool ioLocalUS::IsRightTimeLoginKey( DWORD dwTotalMins )
{
	return true;
}

//������ū
void ioLocalUS::ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	DWORD		 memberType = 0;
	rkPacket >> sEndPW;		//������ū
	rkPacket >> memberType;	//�������� ����Ÿ��

    // encode pw �� 
	pUser->SetBillingUserKey( sEndPW );
	pUser->SetUSMemberType( memberType );

}

bool ioLocalUS::ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}
	
	ioHashString	szUserId;
	ioHashString    szUserIndex;	
	
	rkPacket >> szUserIndex;		//int ���� �Ϲ̾��̵� ����userID
	rkPacket >> szUserId;			// �Ϲ̿��� ���� �� �ִ� id

	// real private id�� ��ü
	//pUser->SetUSMemberIndex( dwMemberID );
	pUser->SetPrivateID(szUserIndex);		//GAuthClient ���� ������������� ���� userID�� �����Ѵ�. 
	pUser->SetUSMemberID( szUserId );
	
	pUser->SetDBAgentID( Help::GetUserDBAgentID( pUser->GetPrivateID() ) );
	
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Success %d:%s:%s, USMemberType:%d, USMemberIndex:%s, USMemberID:%s", 
		__FUNCTION__, szUserIndex.c_str(), szUserId.c_str(), pUser->GetPrivateID().c_str(), pUser->GetUSMemberType(), pUser->GetUSMemberIndex(), pUser->GetUSMemberID().c_str());
	return true;
	/*
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	enum 
	{ 
		MAX_TOKEN   = 3, 
		ID_ARRAY    = 0,
		TOKEN_ARRAY = 1,
		NAME_ARRAY  = 2,
	};

	ioHashString sReturnValue;
	rkPacket >> sReturnValue;
	
	char szTemp[MAX_PATH*2]="";
	StringCbCopy( szTemp, sizeof( szTemp ), sReturnValue.c_str() );

	ioHashString sPrivateID;
	ioHashString sLoginToken;
	ioHashString sUserName;

	char* next_token = NULL;
	for (int i = 0; i < MAX_TOKEN ; i++)
	{
		char *pPos = NULL;
		if( i == 0 )
			pPos = strtok_s( szTemp, US_TOKEN, &next_token );
		else
			pPos = strtok_s( NULL, US_TOKEN, &next_token );

		if( pPos == NULL )
			break;

		if( i == ID_ARRAY )
			sPrivateID  = pPos;
		else if( i == TOKEN_ARRAY )
			sLoginToken = pPos;
		else if( i == NAME_ARRAY )
			sUserName   = pPos;
	}

	LOG.PrintTimeAndLog( 0, "%s %s %s %s", __FUNCTION__, sPrivateID.c_str(), sLoginToken.c_str(), sUserName.c_str() );

	if( g_UserNodeManager.IsConnectUser( sPrivateID ) )   //������..
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Duplication: %s:%s", __FUNCTION__, sPrivateID.c_str(), pUser->GetBillingGUID().c_str() );
		SP2Packet kPacket( STPK_CONNECT );
		kPacket << CONNECT_ID_ALREADY << sPrivateID;
		pUser->SendMessage( kPacket );
		return false;
	}

	if( !sLoginToken.IsEmpty() )
	{
		SP2Packet kPacket( STPK_LOGIN_TOKEN );
		kPacket << sLoginToken;
		pUser->SendMessage( kPacket );
	}

	if( !sUserName.IsEmpty() )
	{
		pUser->SetBillingUserKey( sUserName ); // ���ο� Wemade USA�� UserName�� ���� �Ѵ�.
	}

	// real private id�� ��ü
	pUser->SetPrivateID( sPrivateID );
	pUser->SetDBAgentID( Help::GetUserDBAgentID( pUser->GetPrivateID() ) );
	return true;
	*/
}
void ioLocalUS::ApplyRequestLogin( IN SP2Packet &rkPacket, OUT ioHashString &rsValue1, OUT ioHashString &rsValue2 )
{
	rkPacket >> rsValue1; // usertype
}

//HRYOON 140925
bool ioLocalUS::SendLoginData( User *pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
	pUser->SetBillingGUID( szTempGUID );

	SP2Packet kBillingPacket( BSTPK_LOGIN );
	kBillingPacket << pUser->GetBillingGUID();
	kBillingPacket << pUser->GetPrivateID();
	kBillingPacket << pUser->GetBillingUserKey();	//��������Ű
	kBillingPacket << pUser->GetPublicIP();
	kBillingPacket << BSTPK_LOGIN_RESULT; // return msg type
	//�߰��� �������� �Ϲ� ä�θ� �������� �����ϴ� ���� �Բ� ��������
	kBillingPacket << "WMU";
	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %s:%s(%s)", __FUNCTION__, pUser->GetPrivateID().c_str(),  pUser->GetBillingGUID().c_str(), pUser->GetBillingUserKey().c_str() );
		SP2Packet kPacket( STPK_CONNECT );
		kPacket << CONNECT_EXCEPT << "";
		pUser->SendMessage( kPacket );
		pUser->ClearBillingGUID();
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send : %s:%s(%s)", __FUNCTION__, pUser->GetPrivateID().c_str(), pUser->GetBillingGUID().c_str(), pUser->GetBillingUserKey().c_str() );

	ioHashString szUserKey;
	pUser->SetBillingUserKey( szUserKey ); // Encode PW�� ���⼭ ���� �Ѵ�.

	return true;
}

void ioLocalUS::FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << pUser->GetUSMemberID() << pUser->GetUSMemberType();
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioLocalUS::FillRequestGetCash USMemberID : %s, MemberType : %d", 
												pUser->GetUSMemberID().c_str(), pUser->GetUSMemberType());
}

void ioLocalUS::FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket, const char *szRecvPrivateID )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << g_App.GetCSPort();
	rkPacket << pUser->GetUSMemberType();	//���� ������ Ÿ�� 1: ����
	rkPacket << pUser->GetUSMemberID();		//US ������̵�

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s MemberType : %d,%s ", __FUNCTION__, pUser->GetUSMemberType(), pUser->GetUSMemberID().c_str() );
}

//HRYOON ������׷��̵忡�� �����ִ� �� ���� ������� �˾ƾ��� 140926
void ioLocalUS::FillRequestLogin( OUT SP2Packet &rkPacket, IN ioHashString &rsValue1, IN ioHashString &rsValue2 )
{
	rkPacket << rsValue1; // usertype ( USER_TYPE_NORMAL / USER_TYPE_FB )
}

void ioLocalUS::SendRefundCash( User *pUser, int iTransactionID, bool bRefund )
{
}

void ioLocalUS::SendUserInfo( User *pUser )
{
}

void ioLocalUS::SendCancelCash( IN SP2Packet &rkPacket )
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
	ioHashString szBillingUserKey;
	ioHashString szChargeNo;

	rkPacket >> dwUserIndex;
	rkPacket >> szBillingGUID;
	rkPacket >> iReturnValue;

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
	rkPacket >> szBillingUserKey;
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
	kBillingPacket << szBillingUserKey;
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

int ioLocalUS::GetFirstIDMaxSize()
{
	return 12;
}

const char * ioLocalUS::GetGuildMasterPostion()
{
	return "Leader";
}

const char * ioLocalUS::GetGuildSecondMasterPosition()
{
	return "Officer";
}

const char * ioLocalUS::GetGuildGeneralPosition()
{
	return "Member";
}

const char * ioLocalUS::GetGuildBuilderPosition()
{
	return "Admin";
}

bool ioLocalUS::IsCheckKorean()
{
	return false;
}

bool ioLocalUS::IsChannelingID()
{
	return false;
}

bool ioLocalUS::IsBillingTestID( const ioHashString &rsPublicID )
{
	return false;
}

void ioLocalUS::GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName )
{
	StringCbPrintf( szTitle, iTitleSize, "%s.", szName );
}

bool ioLocalUS::IsSamePCRoomUser()
{
	return false;
}

int ioLocalUS::GetLimitGradeLevel()
{
	return -1;
}

bool ioLocalUS::IsBadPingKick( bool bLadder )
{
	return false;
}

int ioLocalUS::GetLicenseDate()
{
	return 30160806;

}

#if _TESTMODE
bool ioLocalUS::SendLogoutData( User *pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	SP2Packet kBillingPacket( BSTPK_LOGOUT );
	kBillingPacket.Write( pUser->GetPrivateID() );
	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )			
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %s:%s(%s)", __FUNCTION__, pUser->GetPrivateID().c_str(),  pUser->GetBillingGUID().c_str(), pUser->GetBillingUserKey().c_str() );
	}
	
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GAMELOG] ioLocalLatin::SendLogoutData ID:%s", pUser->GetPrivateID().c_str() );

	return true;
}
#endif