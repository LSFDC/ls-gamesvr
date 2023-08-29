#include "stdafx.h"
#include ".\ioLocalThailand.h"
#include <strsafe.h>
#include "..\NodeInfo\User.h"
#include "../BillingRelayServer/BillingRelayServer.h"
#include "../EtcHelpFunc.h"
#include "ioLocalThailandLanguage.h"
#include "../NodeInfo/UserNodeManager.h"

ioLocalThailand::ioLocalThailand(void)
{
	LoadINI();
}

ioLocalThailand::~ioLocalThailand(void)
{
}

ioLocalManager::LocalType ioLocalThailand::GetType()
{
	return ioLocalManager::LCT_THAILAND;
}

const char * ioLocalThailand::GetTextListFileName()
{
	return "text.txt";
}

bool ioLocalThailand::ParseLoginData( IN ioHashString &rsEncLoginKeyAndID, OUT char *szEncLoginKey, IN int iEncLoginKeySize, OUT char *szPrivateID, IN int iPrivaiteIDSize )
{
	return true;
}
// ���������� ���ؼ� �α��� ������ �ϹǷ� ������ true�� ���� 
bool ioLocalThailand::DecryptLoginKey( IN const char *szEncLoginKey, IN const char *szUserKey, OUT char *szDecLoginKey, IN int iDecLoginKeySize )
{
	return true;
}

bool ioLocalThailand::IsRightTimeLoginKey( DWORD dwTotalMins )
{
	return true;
}

bool ioLocalThailand::IsRightLoginKey( const char *szDBKey, const char *szDecryptKey )
{
	return true;
}

void ioLocalThailand::ApplyConnect( IN User *pUser, IN SP2Packet &rkPacket )
{
	

	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}
	ioHashString sEndPW;
	rkPacket >> sEndPW;

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioLocalThailand::ApplyConnect --token : %s", sEndPW.c_str() );

    // encode pw �� 
	pUser->SetBillingUserKey( sEndPW );
	

}

bool ioLocalThailand::ApplyLogin( IN User *pUser, IN SP2Packet &rkPacket )
{
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

	rkPacket >> szUID;			//�׽�Ʈ �� ���� �׳� ���̵� ���� �����ٲ��� ���� 
	rkPacket >> sGarenaName;	// ������ ���� �ִ°� ��� ������ ����
	rkPacket >> GcaType;
	rkPacket >> GoldMemberType;
	rkPacket >> ipBonus;
	rkPacket >> mobileFlag;

	if( GcaType > 0 )
	{
		pUser->SetPCRoomNumber( (DWORD)GcaType );
	}
	// real private id�� ��ü -> ���������� �ִ°�
#ifdef LOCAL_DBG && SRC_TH
	if( szUID.Length() <= 1 )
		szUID = pUser->GetPrivateID();
#endif
	pUser->SetPrivateID( szUID );
	pUser->SetDBAgentID( Help::GetUserDBAgentID( pUser->GetPrivateID() ) );
	return true;
}

bool ioLocalThailand::SendLoginData( User *pUser )
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
	pUser->SetBillingUserKey( szUserKey ); // �α��ο����� ����ϹǷ� �ٷ� ����

	return true;
}

//HRYOON 20150102 �±� ��ū ��ȣȭ ��� ����
bool ioLocalThailand::SendTokenData( User *pUser )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	Help::GetGUID( szTempGUID, sizeof(szTempGUID) );
	pUser->SetBillingGUID( szTempGUID );
	pUser->SetPrivateID( szTempGUID );				//�˻��, ������ ���䰪�� �������� � �������� �Ǵ��� �� �ִ°� ���� guid �ۿ� ����, 


	SP2Packet kBillingPacket( BSTPK_GA_ID_REQUEST );		
	kBillingPacket << pUser->GetBillingGUID();
	kBillingPacket << pUser->GetBillingUserKey();	//������ ��ū��	
	kBillingPacket << BSTPK_GA_ID_RESULT;			// return msg type

	if( !g_BillingRelayServer.SendMessage( kBillingPacket ) )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send Fail : %s", __FUNCTION__, pUser->GetBillingGUID().c_str() );
		SP2Packet kPacket( STPK_TH_GA_ID_REQ );
		kPacket << false << "";
		pUser->SendMessage( kPacket );
		pUser->SetBillingUserKey( "" ); // ��ū ��ȣȭ������ ����ϹǷ� �ٷ� ����
		pUser->ClearBillingGUID();
	}
	else
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Send : %s, %s", __FUNCTION__, pUser->GetBillingGUID().c_str(), pUser->GetPrivateID().c_str() );

	ioHashString szUserKey;
	pUser->SetBillingUserKey( szUserKey ); // ��ū ��ȣȭ������ ����ϹǷ� �ٷ� ����

	return true;
}

void ioLocalThailand::FillRequestGetCash( IN User *pUser, IN SP2Packet &rkPacket )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	rkPacket << pUser->GetPublicIP();
}

void ioLocalThailand::FillRequestOutputCash( IN User *pUser, IN SP2Packet &rkPacket, const char *szRecvPrivateID )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return;
	}

	if( !szRecvPrivateID )
		return;

	ioHashString sRecvPrivateID = szRecvPrivateID; // ��Ŷ�� const char*�� �ٷ� ���� �� ���
	rkPacket << sRecvPrivateID;
}

void ioLocalThailand::SendRefundCash( User *pUser, int iTransactionID, bool bRefund )
{

}

//�����ϰ� ���� �ݾ� ����
bool ioLocalThailand::UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	int iChannelingType = 0;
	int iCash			= 0;
	int iRealCash		= 0;

	rkRecievePacket >> iChannelingType;
	rkRecievePacket >> iCash;			//�̺�Ʈ ĳ�� + ��ĳ��
	rkRecievePacket >> iRealCash;		//���� ���� ĳ��
	
	pUser->SetCash( iCash );
	pUser->SetPurchasedCash( iRealCash );
	pUser->SetChannelingCash( 0 );


	return true;
}

bool ioLocalThailand::CheckDuplication( User *pUser, const ioHashString &rsPrivateID )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return true;
	}

	if( rsPrivateID.IsEmpty() )
		return true;

	if( g_UserNodeManager.IsConnectUser( rsPrivateID ) )  //������..
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
		pUser->SetPrivateID( szEmpty ); // �ʱ�ȭ
		pUser->ClearBillingGUID();
		return false;
	}

	return true;
}

void ioLocalThailand::SendUserInfo( User *pUser )
{

}

int ioLocalThailand::GetFirstIDMaxSize()
{
	return 64;
}

bool ioLocalThailand::IsCheckKorean()
{
	return false;
}

bool ioLocalThailand::IsChannelingID()
{
	return false;
}

bool ioLocalThailand::IsBillingTestID( const ioHashString &rsPublicID )
{
	return false;
}

void ioLocalThailand::GetGuildTitle( OUT char *szTitle, IN int iTitleSize, IN const char *szName )
{
	StringCbPrintf( szTitle, iTitleSize, "%s.", szName );
}

bool ioLocalThailand::IsSamePCRoomUser()
{
	return false;
}

int ioLocalThailand::GetLimitGradeLevel()
{
	return -1;
}

bool ioLocalThailand::IsBadPingKick( bool bLadder )
{
	return false;
}
//20141017- HR ������
bool ioLocalThailand::IsRightID( const char *szID )
{
	return true;
	enum { MIN_LENGTH = 6, MAX_LENGTH = 128, }; // MIN_LENGTH 1+5(domain) / 32(max id) + 8( domain + ������ )

	int iSize = strlen( szID );
	if ( iSize < MIN_LENGTH || iSize > MAX_LENGTH )
		return false;

	for (int i=0; i<iSize; i++)
	{
		if ((!COMPARE(szID[i], 'A', 'Z'+1)) &&
			(!COMPARE(szID[i], 'a', 'z'+1)) &&
			(!COMPARE(szID[i], '0', '9'+1)) &&
			(szID[i]!='!') &&
			(szID[i]!='@') &&
			(szID[i]!='#') &&
			(szID[i]!='$') &&
			(szID[i]!='%') &&
			(szID[i]!='^') &&
			(szID[i]!='&') &&
			(szID[i]!='*') &&
			(szID[i]!='(') &&
			(szID[i]!=')') &&
			(szID[i]!='_') &&
			(szID[i]!='-') &&
			(szID[i]!='[') &&
			(szID[i]!=']') &&
			(szID[i]!=' ') &&	
			(szID[i]!='.') &&   // THPP.tester001 �������� �����ϴ� �����ڷ� ���
			(szID[i]!='|') )    // �α��� ���̵�� private ���̵� �����ϱ� ���ؼ� �α��� ���̵� ���� ����
		{
			return false;
		}
	}
	return true;
}

const char * ioLocalThailand::GetOtherComanyErrorMent()
{
	return "Garena Error : ";
}

bool ioLocalThailand::IsPrivateLowerID()
{
	return false;
}

int ioLocalThailand::GetLicenseDate()
{
	return 30160806;

}

void ioLocalThailand::LoadINI()
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