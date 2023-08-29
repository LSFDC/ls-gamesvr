#include "stdafx.h"
#include ".\iolocaltaiwan.h"
#include "../MainProcess.h"
#include "../Util/ioEncrypted.h"
#include <strsafe.h>
#include "..\NodeInfo\User.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../EtcHelpFunc.h"
#include "../NodeInfo/UserNodeManager.h"
#include "../Channeling/ioChannelingNodeParent.h"

ioLocalTaiwan::ioLocalTaiwan(void)
{
	LoadINI();
}

ioLocalTaiwan::~ioLocalTaiwan(void)
{
}

ioLocalManager::LocalType ioLocalTaiwan::GetType()
{
	return ioLocalManager::LCT_TAIWAN;
}

const char * ioLocalTaiwan::GetTextListFileName()
{
	return "text_tw.txt";
}

bool ioLocalTaiwan::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
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

	//test encode
	//encodeKey (encodeKey, encodeKey) eae0ca332fb475f9f9d8daf598471c
	char enkey[20] = { 0 };
	//ioEncrypted::Encode15("111111111111111", "111111111111111", enkey);
	//encodeID (UserID, encodeKey) 9fb48d6772ea34adbaa2
	char enID[20] = { 0 };
	//ioEncrypted::Encode15("180258575", "111111111111111", enID);
	//sendKey (enKey & enID) eae0ca332fb475f9f9d8daf598471c9fb48d6772ea34adbaa2

	// save Enc login key
	StringCbCopyN(szEncLoginKey, iEncLoginKeySize, rsEncLoginKeyAndID.c_str(), ENC_LOGIN_KEY_NUM );

	if( iPrivaiteIDSize != DATA_LEN )
		return false;

	if(!ioEncrypted::Decode15(szCipherID, (char*)g_App.GetSecondKey().c_str(), szPrivateID))
		return false;

	return true;
}

bool ioLocalTaiwan::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return ioEncrypted::Decode15((char*)szEncLoginKey, (char*)szUserKey, szDecLoginKey );
}
/*
bool ioLocalTaiwan::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
{	
	return true;
}
// 빌링서버를 통해서 로그인 인증을 하므로 무조건 true로 셋팅 

bool ioLocalTaiwan::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return true;
}
*/
bool ioLocalTaiwan::IsRightTimeLoginKey( DWORD dwTotalMins )
{
	return true;
}

bool ioLocalTaiwan::IsRightLoginKey( const char *szDBKey, const char *szDecryptKey )
{
	return true;
}

void ioLocalTaiwan::ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket )
{
	/*
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	rkPacket >> sEndPW;

    // encode pw 값 (임시저장)
	pUser->SetBillingUserKey( sEndPW );
	*/

	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	rkPacket >> sEndPW;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioLocalTaiwan::ApplyConnect --token : %s", sEndPW.c_str() );

	// encode pw 값 
	pUser->SetBillingUserKey( sEndPW );


}

bool ioLocalTaiwan::ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket )
{
	//if( !pUser )
	//{
	//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
	//	return true;
	//}

	//ioHashString sPrivateID;
	//rkPacket >> sPrivateID;
	//
	//// real private id로 교체
	//pUser->SetPrivateID( sPrivateID );
	//pUser->SetDBAgentID( Help::GetUserDBAgentID( pUser->GetPrivateID() ) );
	//return true;
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}
	unsigned short	GcaType;
	unsigned char	GoldMemberType;
	byte			ipBonus;
	byte			mobileFlag;

	ioHashString	szUID;
	ioHashString	sGarenaName;

	rkPacket >> szUID;			//테스트 인 경우는 그냥 아이디 값만 보내줄꺼임 실제 
	rkPacket >> sGarenaName;	// 가레나 에서 주는값 디비에 쓰이지 않음
	rkPacket >> GcaType;
	rkPacket >> GoldMemberType;
	rkPacket >> ipBonus;
	rkPacket >> mobileFlag;

	if( GcaType > 0 )
	{
		pUser->SetPCRoomNumber( (DWORD)GcaType );
	}
	// real private id로 교체 -> 가레나에서 주는값
	pUser->SetPrivateID( szUID );
	pUser->SetDBAgentID( Help::GetUserDBAgentID( pUser->GetPrivateID() ) );
	return true;
}

void ioLocalTaiwan::LoadINI()
{
	char	szBuf[MAX_PATH]		= {0,};

	ioINILoader kLoaderLocal( "config/sp2_local.ini" );
	kLoaderLocal.SetTitle( "text" );

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "guild_master", "", szBuf, MAX_PATH );
	m_sGuildMaster = szBuf;

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "guild_second_master", "", szBuf, MAX_PATH );
	m_sGuildSecondMaster = szBuf;

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "guild_general", "", szBuf, MAX_PATH );
	m_sGuildGeneral = szBuf;

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "guild_builder", "", szBuf, MAX_PATH );
	m_sGuildBuilder = szBuf;
}


void ioLocalTaiwan::ApplyRequestLogin( IN SP2Packet &rkPacket, OUT ioHashString &rsValue1, OUT ioHashString &rsValue2 )
{
	rkPacket >> rsValue1; // macaddress
}

bool ioLocalTaiwan::SendLoginData( User *pUser )
{
	//if( !pUser )
	//{
	//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
	//	return true;
	//}

	//char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	//Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
	//pUser->SetBillingGUID( szTempGUID );

	//SP2Packet kBillingPacket( BSTPK_LOGIN );
	//kBillingPacket << pUser->GetBillingGUID();
	//kBillingPacket << pUser->GetPrivateID();
	//kBillingPacket << pUser->GetBillingUserKey();
	//kBillingPacket << pUser->GetPublicIP();
	//kBillingPacket << BSTPK_LOGIN_RESULT; // return msg type

	//if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	//{
	//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %s:%s", __FUNCTION__, pUser->GetPrivateID().c_str(),  pUser->GetBillingGUID().c_str() );
	//	SP2Packet kPacket( STPK_CONNECT );
	//	kPacket << CONNECT_EXCEPT << "";
	//	pUser->SendMessage( kPacket );
	//	pUser->ClearBillingGUID();
	//}
	//else
	//	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send : %s:%s", __FUNCTION__, pUser->GetPrivateID().c_str(), pUser->GetBillingGUID().c_str() );

	//ioHashString szUserKey;
	//pUser->SetBillingUserKey( szUserKey ); // 로그인에서만 사용하므로 바로 삭제

	//return true;
	

	SP2Packet kPacket( BSTPK_FIRST_LOGIN );
	kPacket.Write( pUser->GetPrivateID() );
	kPacket.SetPosBegin();

	g_BillingRelayServer.SendMessage( kPacket );

	return false;
	/*
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
	kBillingPacket << pUser->GetBillingUserKey();	
	kBillingPacket << pUser->GetPublicIP();
	kBillingPacket << BSTPK_LOGIN_RESULT; // return msg type

	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %s:%s", __FUNCTION__, pUser->GetPrivateID().c_str(),  pUser->GetBillingGUID().c_str() );
		SP2Packet kPacket( STPK_CONNECT );
		kPacket << CONNECT_EXCEPT << "";
		pUser->SendMessage( kPacket );
		pUser->ClearBillingGUID();
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send : %s:%s", __FUNCTION__, pUser->GetPrivateID().c_str(), pUser->GetBillingGUID().c_str() );

	ioHashString szUserKey;
	pUser->SetBillingUserKey( szUserKey ); // 로그인에서만 사용하므로 바로 삭제

	return true;*/
}

void ioLocalTaiwan::FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << pUser->GetPublicIP();
}

void ioLocalTaiwan::FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket, const char *szRecvPrivateID )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	if( !szRecvPrivateID )
		return;

	ioHashString sRecvPrivateID = szRecvPrivateID; // 패킷에 const char*을 바로 넣을 수 없어서
	rkPacket << sRecvPrivateID;
}

//구매하고 남은 금액 세팅
bool ioLocalTaiwan::UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	int iChannelingType = 0;
	int iCash			= 0;
	int iRealCash		= 0;

	rkRecievePacket >> iChannelingType;
	rkRecievePacket >> iCash;			//이벤트 캐쉬 + 실캐쉬
	rkRecievePacket >> iRealCash;		//구매 가능 캐쉬

	pUser->SetCash( iCash );
	pUser->SetPurchasedCash( iRealCash );
	pUser->SetChannelingCash( 0 );
	
	return true;
}


void ioLocalTaiwan::FillRequestLogin( OUT SP2Packet &rkPacket, IN ioHashString &rsValue1, IN ioHashString &rsValue2 )
{
	rkPacket << rsValue1; // macaddress
}

void ioLocalTaiwan::SendRefundCash( User *pUser, int iTransactionID, bool bRefund )
{

}


bool ioLocalTaiwan::CheckDuplication( User *pUser, const ioHashString &rsPrivateID )
{
	return true;

	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	if( rsPrivateID.IsEmpty() )
		return true;

	if( g_UserNodeManager.IsConnectUser( rsPrivateID ) )  //접속중..
	{
		SP2Packet kReturn( ASTPK_OTHER_COMPANY_LOGIN_RESULT );
		kReturn << false;
		kReturn << true;
		char szMent[MAX_PATH]="";
		StringCbCopy( szMent , sizeof( szMent ), GetDuplicationMent() );
		kReturn << szMent;
		pUser->SendMessage( kReturn );
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Exist User: [%s]:%s", __FUNCTION__, rsPrivateID.c_str() , pUser->GetBillingGUID().c_str() );
		ioHashString szEmpty;
		pUser->SetPrivateID( szEmpty ); // 초기화
		pUser->ClearBillingGUID();
		return false;
	}

	return true;
}

void ioLocalTaiwan::SendUserInfo( User *pUser )
{

}

int ioLocalTaiwan::GetFirstIDMaxSize()
{
	return 12;
}

const char * ioLocalTaiwan::GetGuildMasterPostion()
{
	return m_sGuildMaster.c_str();
}

const char * ioLocalTaiwan::GetGuildSecondMasterPosition()
{
	return m_sGuildSecondMaster.c_str();
}

const char * ioLocalTaiwan::GetGuildGeneralPosition()
{
	return m_sGuildGeneral.c_str();
}

const char * ioLocalTaiwan::GetGuildBuilderPosition()
{
	return m_sGuildBuilder.c_str();
}

bool ioLocalTaiwan::IsCheckKorean()
{
	return false;
}

bool ioLocalTaiwan::IsChannelingID()
{
	return false;
}

bool ioLocalTaiwan::IsBillingTestID( const ioHashString &rsPublicID )
{
	return false;
}

void ioLocalTaiwan::GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName )
{
	StringCbPrintf( szTitle, iTitleSize, "%s.", szName );
}

bool ioLocalTaiwan::IsSamePCRoomUser()
{
	return false;
}

int ioLocalTaiwan::GetLimitGradeLevel()
{
	return -1;
}

bool ioLocalTaiwan::IsBadPingKick( bool bLadder )
{
	return true;
}

bool ioLocalTaiwan::IsRightID( const char *szID )
{
	return true;	//사용하지 않음
	enum { MIN_LENGTH = 1, MAX_LENGTH = 60, };

	int iSize = strlen( szID );
	if ( iSize < MIN_LENGTH || iSize > MAX_LENGTH )
		return false;

	return true;
}

const char * ioLocalTaiwan::GetDuplicationMent()
{
	return Help::GetDuplication().c_str();
}

const char * ioLocalTaiwan::GetExitingServerMent()
{
	return Help::GetExiting().c_str();
}

const char * ioLocalTaiwan::GetOtherComanyErrorMent()
{
	return "Garena Error : ";
}

bool ioLocalTaiwan::IsPrivateLowerID()
{
	return false;
}

int ioLocalTaiwan::GetLicenseDate()
{
	return 30111131;
}

void ioLocalTaiwan::SendCancelCash( IN SP2Packet &rkPacket )
{
	if( rkPacket.GetPacketID() != BSTPK_OUTPUT_CASH_RESULT )
		return;

	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex = 0;
	ioHashString szPrivateID;
	ioHashString szOrderNo;
	ioHashString szUserIP;
	int          iPayAmt  = 0;
	int          iType    = 0;
	int          iReturnValue = 0;
	int          iTransactionID = 0;
	int          iTotalCash     = 0;
	int          iPurchasedCash = 0;

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
	rkPacket >> iTotalCash;
	rkPacket >> iPurchasedCash;
	rkPacket >> szPrivateID;
	rkPacket >> szOrderNo;

	//--------------------------------SEND------------------------------------//
	char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
	SP2Packet kBillingPacket( BSTPK_CANCEL_CASH );
	// 공통
	kBillingPacket << iChannelingType;
	kBillingPacket << szTempGUID;
	kBillingPacket << dwUserIndex;

	// Cancel Step 3
	kBillingPacket << szPrivateID;
	kBillingPacket << szOrderNo;

	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send Fail : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
	else
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[GOLD_INFO]%s Send : %d:%s:%s:%s", __FUNCTION__, dwUserIndex, szPrivateID.c_str(), szBillingGUID.c_str(), szTempGUID );
	}
}