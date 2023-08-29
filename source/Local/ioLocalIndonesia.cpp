#include "stdafx.h"
#include ".\ioLocalIndonesia.h"
#include "../MainProcess.h"
#include "../Util/ioEncrypted.h"
#include <strsafe.h>
#include "..\NodeInfo\User.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../EtcHelpFunc.h"
#include "../NodeInfo/UserNodeManager.h"
#include "../Channeling/ioChannelingNodeParent.h"

ioLocalIndonesia::ioLocalIndonesia(void)
{
	LoadINI();
}

ioLocalIndonesia::~ioLocalIndonesia(void)
{
}

ioLocalManager::LocalType ioLocalIndonesia::GetType()
{
	return ioLocalManager::LCT_INDONESIA;
}

const char * ioLocalIndonesia::GetTextListFileName()
{
	return "text_id.txt";
}

bool ioLocalIndonesia::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
{	
	return true;
}
// ���������� ���ؼ� �α��� ������ �ϹǷ� ������ true�� ���� 
bool ioLocalIndonesia::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return true;
}

bool ioLocalIndonesia::IsRightTimeLoginKey( DWORD dwTotalMins )
{
	return true;
}

bool ioLocalIndonesia::IsRightLoginKey( const char *szDBKey, const char *szDecryptKey )
{
	return true;
}

void ioLocalIndonesia::ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	rkPacket >> sEndPW >> pUser->m_szMacAddress >> pUser->m_szPremiumKey 
		>> pUser->m_szAuthInfo >> pUser->m_dwSessionID;

    // �ε��׽þƴ� encode pw �� ( �ӽ� ���� ) 
	pUser->SetBillingUserKey( sEndPW );

}

bool ioLocalIndonesia::ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}

	// indonesia�� billingKey�� �޴´�.
	ioHashString sBillingKey;
	DWORD dwPCRoomNum = 0;
	rkPacket >> sBillingKey;
	rkPacket >> dwPCRoomNum;

	pUser->SetBillingUserKey( sBillingKey );
	pUser->SetPCRoomNumber( dwPCRoomNum );


	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pcroom Num %d", __FUNCTION__, dwPCRoomNum );

	return true;
}

void ioLocalIndonesia::LoadINI()
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

void ioLocalIndonesia::ApplyRequestLogin( IN SP2Packet &rkPacket, OUT ioHashString &rsValue1, OUT ioHashString &rsValue2 )
{
	rkPacket >> rsValue1; // macaddress
	rkPacket >> rsValue2; // reg key
}

bool ioLocalIndonesia::SendLoginData( User *pUser )
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
	kBillingPacket << pUser->GetBillingUserKey();
	kBillingPacket << pUser->GetPublicIP();
	kBillingPacket << BSTPK_LOGIN_RESULT; // return msg type

	kBillingPacket << pUser->m_szMacAddress;
	kBillingPacket << pUser->m_szPremiumKey;
	kBillingPacket << pUser->m_szAuthInfo;
	kBillingPacket << pUser->m_dwSessionID;

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
	pUser->SetBillingUserKey( szUserKey ); // �α��ο����� ����ϹǷ� �ٷ� ����

	return true;
}

void ioLocalIndonesia::FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << pUser->GetBillingUserKey();
}

void ioLocalIndonesia::FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket, const char *szRecvPrivateID )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << pUser->GetBillingUserKey();
	rkPacket << pUser->GetGradeLevel();
}

void ioLocalIndonesia::FillRequestLogin( OUT SP2Packet &rkPacket, IN ioHashString &rsValue1, IN ioHashString &rsValue2 )
{
	rkPacket << rsValue1; // macaddress
	rkPacket << rsValue2; // reg key
}

void ioLocalIndonesia::SendRefundCash( User *pUser, int iTransactionID, bool bRefund )
{

}

void ioLocalIndonesia::SendUserInfo( User *pUser )
{

}

int ioLocalIndonesia::GetFirstIDMaxSize()
{
	return 12;
}

const char * ioLocalIndonesia::GetGuildMasterPostion()
{
	return m_sGuildMaster.c_str();
}

const char * ioLocalIndonesia::GetGuildSecondMasterPosition()
{
	return m_sGuildSecondMaster.c_str();
}

const char * ioLocalIndonesia::GetGuildGeneralPosition()
{
	return m_sGuildGeneral.c_str();
}
const char * ioLocalIndonesia::GetGuildBuilderPosition()
{
	return m_sGuildBuilder.c_str();
}
bool ioLocalIndonesia::IsCheckKorean()
{
	return false;
}

bool ioLocalIndonesia::IsChannelingID()
{
	return false;
}

bool ioLocalIndonesia::IsBillingTestID( const ioHashString &rsPublicID )
{
	return false;
}

void ioLocalIndonesia::GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName )
{
	StringCbPrintf( szTitle, iTitleSize, "%s.", szName );
}

bool ioLocalIndonesia::IsSamePCRoomUser()
{
	return false;
}

int ioLocalIndonesia::GetLimitGradeLevel()
{
	return -1;
}

bool ioLocalIndonesia::IsBadPingKick( bool bLadder )
{
	if( bLadder )
		return true;
	else
		return false;
}

bool ioLocalIndonesia::IsRightID( const char *szID )
{
	enum { MIN_LENGTH = 6, MAX_LENGTH = 12, };

	int iSize = strlen( szID );
#if !defined (_LSWC)
	if ( iSize < MIN_LENGTH || iSize > MAX_LENGTH )
		return false;
#endif

	for (int i=0; i<iSize; i++)
	{
		if ((!COMPARE(szID[i], 'A', 'Z'+1)) &&
			(!COMPARE(szID[i], 'a', 'z'+1)) &&
			(!COMPARE(szID[i], '0', '9'+1)) &&
			(szID[i]!='-') &&
			(szID[i]!='_') )
		{
			return false;
		}
	}
	return true;
}

const char * ioLocalIndonesia::GetDuplicationMent()
{
	return "Telah mencoba login berulang kali.\r\n\r\n Silahkan coba login kembali 5 menit lagi.\r\n\r\n Jika masih mengalami masalah \r\n\r\n hubungi Tim GM.";
}

const char * ioLocalIndonesia::GetExitingServerMent()
{
	return "Server ditutup.\r\n\r\n Cek Pengumuman\r\n\r\n dan pelayanan akan kembali beberapa saat lagi.";
}

const char * ioLocalIndonesia::GetOtherComanyErrorMent()
{
	return "KREON Error : ";
}

bool ioLocalIndonesia::IsPrivateLowerID()
{
	return true;
}

int ioLocalIndonesia::GetLicenseDate()
{
	return 20160806;
}

bool ioLocalIndonesia::IsRightNewID( const char *szID )
{
	int size = strlen(szID);
	for (int i=0; i<size; i++)
	{
		if ((!COMPARE(szID[i], 'A', 'Z'+1)) &&
			(!COMPARE(szID[i], 'a', 'z'+1)) &&
			(!COMPARE(szID[i], '0', '9'+1)) )
		{
			return false;
		}
	}
	return true;
}