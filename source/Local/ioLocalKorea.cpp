#include "stdafx.h"
#include ".\iolocalkorea.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "../Util/ioEncrypted.h"
#include <strsafe.h>
#include "../Channeling/ioChannelingNodeParent.h"
#include "../NodeInfo/User.h"
#include "../Channeling/ioChannelingNodeManager.h"
#include "../BillingRelayServer/BillingRelayServer.h"

ioLocalKorea::ioLocalKorea(void)
{
}

ioLocalKorea::~ioLocalKorea(void)
{
}

ioLocalManager::LocalType ioLocalKorea::GetType()
{
	return ioLocalManager::LCT_KOREA;
}

const char * ioLocalKorea::GetTextListFileName()
{
	return "text.txt";
}

bool ioLocalKorea::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
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
	//char enkey[256] = "";
	//ioEncrypted::Encode15("111111111111111", "111111111111111", enkey);
	//encodeID (UserID, encodeKey) 9fb48d6772ea34adbaa2
	//char enID[256] = "";
	//ioEncrypted::Encode15("tester01", "111111111111111", enID);
	//sendKey (enKey & enID) eae0ca332fb475f9f9d8daf598471c9fb48d6772ea34adbaa2

	// save Enc login key
	StringCbCopyN(szEncLoginKey, iEncLoginKeySize, rsEncLoginKeyAndID.c_str(), ENC_LOGIN_KEY_NUM );

	if( iPrivaiteIDSize != DATA_LEN )
		return false;

	if(!ioEncrypted::Decode15(szCipherID, (char*)g_App.GetSecondKey().c_str(), szPrivateID))
		return false;

	return true;
}

bool ioLocalKorea::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return ioEncrypted::Decode15((char*)szEncLoginKey, (char*)szUserKey, szDecLoginKey );
}

bool ioLocalKorea::IsRightTimeLoginKey( DWORD dwTotalMins )
{
#ifdef _DEBUG
	return true;
#endif

	enum { MAX_LIVE_MINUTES = 30 };

	if( dwTotalMins >= MAX_LIVE_MINUTES )
		return false;

	return true;
}

void ioLocalKorea::ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket )
{
	
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	rkPacket >> sEndPW;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioLocalKorea::ApplyConnect --token : %s", sEndPW.c_str() );

	// encode pw �� 
	pUser->SetBillingUserKey( sEndPW );
}

bool ioLocalKorea::SendLoginData( User *pUser )
{

	SP2Packet kPacket( BSTPK_FIRST_LOGIN );
	kPacket.Write( pUser->GetPrivateID() );
	kPacket.SetPosBegin();

	g_BillingRelayServer.SendMessage( kPacket );

	return false;
}

bool ioLocalKorea::IsRightLoginKey( const char *szDBKey, const char *szDecryptKey )
{
	if( strcmp( szDBKey, szDecryptKey ) == 0 )
		return true;

	return false;
}

void ioLocalKorea::FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket )
{

}

void ioLocalKorea::SendRefundCash( User *pUser, int iTransactionID, bool bRefund )
{

}

void ioLocalKorea::SendUserInfo( User *pUser )
{

}

int ioLocalKorea::GetFirstIDMaxSize()
{
	return 12;
}
/*
const char * ioLocalKorea::GetGuildMasterPostion()
{
	return "�����";
}

const char * ioLocalKorea::GetGuildSecondMasterPosition()
{
	return "�α����";
}

const char * ioLocalKorea::GetGuildBuilderPosition()
{
	return "Admin";
}

const char * ioLocalKorea::GetGuildGeneralPosition()
{
	return "����";
}
*/
const char * ioLocalKorea::GetGuildMasterPostion()
{
	return "Leader";
}

const char * ioLocalKorea::GetGuildSecondMasterPosition()
{
	return "Officer";
}

const char * ioLocalKorea::GetGuildGeneralPosition()
{
	return "Member";
}

const char * ioLocalKorea::GetGuildBuilderPosition()
{
	return "Admin";
}
bool ioLocalKorea::IsCheckKorean()
{
	return true;
}

bool ioLocalKorea::IsChannelingID()
{
	return true;
}

bool ioLocalKorea::IsBillingTestID( const ioHashString &rsPublicID )
{
	// �Ʒ� "�ݹ�", "�ݹ���", "�ݹ���" ���̵� ������ �ݵ�� ���������� �������ּ���.
	// �ݹ� : �Ϲ�, �ݹ��� : ������, galura17 : ���̹� ( byosa190 / �λ��׽�Ʈ1@ ), �ݹ��� : �������, �ݹ��� : ���Ϸ��� ( cash6 /cash6 )
	if( rsPublicID == "�ݹ�" || rsPublicID == "�ݹ���" || rsPublicID == "galura17" || rsPublicID == "�ݹ���" || rsPublicID == "�ݹ���" )
		return true;

	return false;
}

void ioLocalKorea::GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName )
{
	StringCbPrintf( szTitle, iTitleSize, "%s����Դϴ�.", szName );
}

bool ioLocalKorea::IsSamePCRoomUser()
{
	return true;
}

int ioLocalKorea::GetLimitGradeLevel()
{
	return -1;
}

bool ioLocalKorea::IsBadPingKick( bool bLadder )
{
	return true;
}

int ioLocalKorea::GetLicenseDate()
{
	return 30111131;
}


void ioLocalKorea::SendCancelCash( IN SP2Packet &rkPacket )
{
	if( rkPacket.GetPacketID() != BSTPK_OUTPUT_CASH_RESULT )
		return;

	int          iChannelingType = 0;
	ioHashString szBillingGUID;
	DWORD        dwUserIndex = 0;
	int          iPayAmt  = 0;
	int          iType    = 0;
	int          iReturnValue = 0;
	int          TransactionID = 0;

	rkPacket >> dwUserIndex;
	rkPacket >> szBillingGUID;
	rkPacket >> iReturnValue;
	
	if( iReturnValue != CASH_RESULT_SUCCESS )
		return;

	rkPacket >> iType;
	rkPacket >> iPayAmt;
	rkPacket >> TransactionID;
	int iItemValueList[ioChannelingNodeParent::MAX_ITEM_VALUE];
	for (int i = 0; i < ioChannelingNodeParent::MAX_ITEM_VALUE ; i++)
		iItemValueList[i] = 0;
	ioChannelingNodeParent::GetItemValueList( rkPacket, iType, iItemValueList );
	rkPacket >> iChannelingType;

	ioChannelingNodeParent *pNode = g_ChannelingMgr.GetNode( (ChannelingType)iChannelingType );
	if( pNode )
	{
		rkPacket.SetPosBegin();
		pNode->SendCancelCash( rkPacket );
	}
}

bool ioLocalKorea::UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	ioChannelingNodeParent *pNode = g_ChannelingMgr.GetNode( pUser->GetChannelingType() );
	if( !pNode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pNode == NULL.", __FUNCTION__ );
		return false;
	}
	
	if( !pNode->UpdateOutputCash( pUser, rkRecievePacket, iPayAmt, dwErrorPacketID, dwErrorPacketType ) )
		return ioLocalParent::UpdateOutputCash( pUser, rkRecievePacket, iPayAmt, dwErrorPacketID, dwErrorPacketType ); // Default

	return true;
}