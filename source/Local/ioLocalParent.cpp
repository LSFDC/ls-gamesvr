#include "stdafx.h"
#include ".\iolocalparent.h"
#include "..\NodeInfo\User.h"
#include "../EtcHelpFunc.h"

ioLocalParent::ioLocalParent(void)
{
}

ioLocalParent::~ioLocalParent(void)
{
}

bool ioLocalParent::IsRightLicense()
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	int iDate = (st.wYear * 10000) + (st.wMonth * 100) + st.wDay;

	if( iDate >= GetLicenseDate() )
		return false;

	return true;
}

bool ioLocalParent::UpdateOutputCash( User *pUser, SP2Packet &rkRecievePacket, int iPayAmt, DWORD dwErrorPacketID, DWORD dwErrorPacketType )
{
	if( !pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s pUser == NULL.", __FUNCTION__ );
		return false;
	}
	
	int iChannelingType = 0;
	int iTotalCash     = 0;
	int iPurchasedCash = 0;
	rkRecievePacket >> iChannelingType;
	rkRecievePacket >> iTotalCash;
	rkRecievePacket >> iPurchasedCash;
	pUser->SetCash( iTotalCash );
	pUser->SetPurchasedCash( iPurchasedCash );

	return true;
}
void ioLocalParent::LoadINI()
{
	char szBuf[MAX_PATH]="";
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

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "duplication", "", szBuf, MAX_PATH );
	m_sDuplicationMent = szBuf;

	ZeroMemory( szBuf, sizeof( szBuf ) );
	kLoaderLocal.LoadString( "exiting", "", szBuf, MAX_PATH );
	m_sExitingServerMent = szBuf;
}
const char * ioLocalParent::GetGuildMasterPostion()
{
	return m_sGuildMaster.c_str();
}

const char * ioLocalParent::GetGuildSecondMasterPosition()
{
	return m_sGuildSecondMaster.c_str();
}

const char * ioLocalParent::GetGuildGeneralPosition()
{
	return m_sGuildGeneral.c_str();
}

const char * ioLocalParent::GetGuildBuilderPosition()
{
	return m_sGuildBuilder.c_str();
}

const char * ioLocalParent::GetDuplicationMent()
{
	return m_sDuplicationMent.c_str();
}

const char * ioLocalParent::GetExitingServerMent()
{
	return m_sExitingServerMent.c_str();
}