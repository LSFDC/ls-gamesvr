// ls_gamesvr.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "ServiceLS.h"
#include "MainProcess.h"
#include "Shutdown.h"

#include <crtdbg.h>
#include "./CrashFind/BugslayerUtil.h"

CLog LOG;
CLog HackLOG;
CLog ProcessLOG;
CLog EventLOG;
CLog P2PRelayLOG;
CLog RateCheckLOG;
CLog TradeLOG;
CLog CriticalLOG;
#ifdef ANTIHACK
CLog CheatLOG;
CLog CheatUser;
#endif

LONG __stdcall ExceptCallBack ( EXCEPTION_POINTERS * pExPtrs );

int _tmain(int argc, _TCHAR* argv[])
{
	ServiceLS *service = new ServiceLS( argc, argv );

	service->ServiceMainProc();
	delete service;

	return 0;
}

LONG __stdcall ExceptCallBack ( EXCEPTION_POINTERS * pExPtrs )
{
	static bool bHappenCrash = false;

	// ���⼭ ���ܾ� ��.
	//UnHandledExceptionFilter( pExPtrs );

	if(bHappenCrash)
		return EXCEPTION_EXECUTE_HANDLER;

	char szLog[2048]="";
	strcpy_s(szLog, g_App.GetPublicIP().c_str());

	char szTemp[2048]="";
	CriticalLOG.PrintLog(0, "---- Crash Help Data ----");
	wsprintf(szTemp, "%s", GetFaultReason(pExPtrs));
	CriticalLOG.PrintLog(0, "%s", szTemp);
	strcat_s(szLog, "\n");
	strcat_s(szLog, szTemp);
	memset(szTemp, 0, sizeof(szTemp));

	wsprintf(szTemp, "%s", GetRegisterString(pExPtrs));
	CriticalLOG.PrintLog(0, "%s", szTemp);
	strcat_s(szLog, "\n");
	strcat_s(szLog, szTemp);
	
	const char * szBuff = GetFirstStackTraceString( GSTSO_SYMBOL | GSTSO_SRCLINE,pExPtrs  );
	do
	{
		CriticalLOG.PrintLog(0,"%s" , szBuff );	
		if(strlen(szLog)+strlen(szBuff) < 1500)
		{
			strcat_s(szLog, "\n");
			strcat_s(szLog, szBuff);
		}
		szBuff = GetNextStackTraceString( GSTSO_SYMBOL | GSTSO_SRCLINE , pExPtrs );
	}
	while ( NULL != szBuff );
	
	SP2Packet kPacket( LUPK_LOG );
	kPacket << "ServerError";
	kPacket << szLog;
	kPacket << 1000; // ������ȣ
	kPacket << true; // write db
	g_UDPNode.SendLog( kPacket );
	g_CriticalError.CheckCrashLog( szLog );
	CriticalLOG.PrintLog(0, "---- Crash Help End ----");

	bHappenCrash = false;

	//g_App.Shutdown(SHUTDOWN_CRASH);	
	return EXCEPTION_EXECUTE_HANDLER;
}
