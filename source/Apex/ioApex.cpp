#include "stdafx.h"
#include "../NodeInfo/UserNodeManager.h"
#include "../Local/ioLocalParent.h"
#include "../Local/ioLocalManager.h"
#include "ioApex.h"

#ifdef SRC_LATIN

int GetDataFromApex(void *pMyParaData, char* pData, char cDataType, unsigned int nSendID, int nRetLen)
{
	int nRet = 0;
	switch (cDataType) {
	case 'D':
		{
			SP2Packet kPacket( STPK_PROTECT_CHECK );
			ApexPacket rkPacket;
			rkPacket.nType = APEX_CMD_DATA;
			CopyMemory(rkPacket.byBuffer, pData, nRetLen);
			rkPacket.nLength = nRetLen;

			kPacket << rkPacket;

			User *pUser = g_UserNodeManager.GetUserNode((DWORD)nSendID);

			if(pUser)
				pUser->SendMessage( kPacket );
		}
		break;
	case 'K':
		{
			User *pUser = g_UserNodeManager.GetUserNode((DWORD)nSendID);

			if(pUser)
			{
				//hr 라틴추가
				pUser->SetLogoutType( 6 );
				pUser->ExceptionClose( 0 );
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s : Error : %s:%s:%d", __FUNCTION__, pUser->GetPublicID().c_str(), pUser->GetPrivateID().c_str(), pUser->GetUserIndex() );
			}
		}
		break;
	default:
		break;
	}
	return nRet;	
}

ioApex *ioApex::sg_Instance = NULL;


ioApex::ioApex(void)
{
	m_bUse = false;
	m_dwProcessTime = 0;
}

ioApex::~ioApex(void)
{

}

ioApex & ioApex::GetInstance()
{
	if( sg_Instance == NULL )
		sg_Instance = new ioApex;

	return (*sg_Instance);
}

void ioApex::ReleaseInstance()
{
	if( sg_Instance )
		delete sg_Instance;

	sg_Instance = NULL;
}

bool ioApex::Start( bool bUse )
{
	m_bUse = bUse;

	if( !m_bUse )
		return true;

	return s_start();
}

void ioApex::End()
{
	s_end();
}

void ioApex::GetDataFromAS()
{
	s_getData_FromApex_cb(0, GetDataFromApex, NULL);
}

void ioApex::NoticeApexProxy_UserLogIn(const DWORD dwUserIdx, const ioHashString &szPrivateID, const ioHashString &szPublicIP)
{
	s_sendData_ToApex('L', dwUserIdx, szPrivateID.c_str(), szPrivateID.Length());

	char szBuf[5] = {0};
	nUserIp_st *pSt = (nUserIp_st*)szBuf;
	pSt->Cmd_UserIpFlag = 0x01;
	pSt->nClientIp = inet_addr(szPublicIP.c_str());
	s_sendData_ToApex('S', dwUserIdx, (const char*)szBuf, 5);
}


int ioApex::NoticeApexProxy_UserLogout(const DWORD dwUserIdx, const ioHashString &szPrivateID)
{
	LOG.PrintTimeAndLog( 0, "ioApex::NoticeApexProxy_UserLogout UserIndex : %d, PrivateID :%s", dwUserIdx, szPrivateID.c_str() );
	s_sendData_ToApex('G', dwUserIdx, szPrivateID.c_str(), szPrivateID.Length());
	return 0;
}

int ioApex::NoticeApexProxy_UserData(const DWORD dwUserIdx, const char *pBuf, int nBufLen) 
{
	s_sendData_ToApex('T', dwUserIdx, pBuf, nBufLen);
	return 0;
}

#endif
