#include "StdAfx.h"
#include "MainProcess.h"
#include "Shutdown.h"
#include "Manager.h"
#include "NodeInfo\SchedulerNode.h"
#include "Local\ioLocalManager.h"

Manager::Manager(void)
{
	Init();
}

Manager::~Manager(void)
{
	Destroy();
}

void Manager::Init()
{
}

void Manager::Destroy()
{
}

BOOL Manager::Run(const char* scriptName)
{
	if(!Startup(scriptName))
		return FALSE;

	if(!Prepare())
		return FALSE;

	Timer();
	return TRUE;
}

BOOL Manager::Startup(const char* scriptName)
{
	timeBeginPeriod(1);

	SetHeapInformation();			// LFH

	if(!g_App.Startup(scriptName))
		return FALSE;
	return TRUE;
}

void Manager::SetHeapInformation()
{
	HANDLE heaps[1025];
	BOOL results[1025];

	ZeroMemory( heaps, sizeof( heaps ) );
	ZeroMemory( results, sizeof( results ) );

	ULONG HeapFragValue = 2;

	DWORD count = GetProcessHeaps( 1024, heaps );
	for(DWORD i = 0; i < count; i++)
	{
		results[i] = HeapSetInformation( heaps[i], HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue) ); 
		if( !results[i] )
		{
			DisplayHeapInfo( heaps[i] );
		}
	}
}

void Manager::DisplayHeapInfo(HANDLE heap) 
{ 
	ULONG heapInfo; 
	SIZE_T size; 

	if( HeapQueryInformation( heap, HeapCompatibilityInformation, &heapInfo, sizeof(heapInfo), &size ) == FALSE ) 
	{ 
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "SetHeapInformation Failed" );
	} 
} 

BOOL ConsoleHandler(DWORD fdwCtrlType) 
{ 
	switch (fdwCtrlType) 
	{ 
		// Handle the CTRL+C signal. 
		case CTRL_C_EVENT: 
		case CTRL_CLOSE_EVENT: // CTRL+CLOSE: confirm! that the user wants to exit. 
		case CTRL_BREAK_EVENT: 
		case CTRL_LOGOFF_EVENT: 
		case CTRL_SHUTDOWN_EVENT: 
		default: 
			g_App.Shutdown(SHUTDOWN_QUICK); 
			return FALSE;
	} 
	return TRUE;
}	

BOOL Manager::Prepare()
{
	/*����� �����ϴ��Լ� ȣ���ϵ��� ��������*/ 
    if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandler, TRUE) ) 
		return FALSE; 

	// scheduler
	m_scheduler	= g_InternalNode;
	if( !m_scheduler )
		return FALSE;

	if( ! g_App.Initialize( m_scheduler ) )
		return FALSE;
	
	// init
	InitScheduler();

	return TRUE;
}

void Manager::InitScheduler()
{
	ioINILoader iLoader("ls_config_game.ini");
	iLoader.SetTitle( "ANTIHACK" );
	int iUDPTimer = iLoader.LoadInt( "UdpCheckTimer", 3000 );

	m_scheduler->OnCreate();
	m_scheduler->AddSchedule(ITPK_ROOM_PROCESS, 700);
	m_scheduler->AddSchedule(ITPK_TIMER_PROCESS, 500);
	m_scheduler->AddSchedule(ITPK_PING_PROCESS, 5000);
	m_scheduler->AddSchedule(ITPK_USERGHOST_PROCESS, 5000);
	m_scheduler->AddSchedule(ITPK_USERUPDATE_PROCESS, 10000);
	m_scheduler->AddSchedule(ITPK_MONSTERCOIN_PROCESS, 1000);
	m_scheduler->AddSchedule(ITPK_SENDBUFFER_FLUSH_PROCESS, g_App.GetNagleTime() );	// default : 30
#ifdef ANTIHACK
	m_scheduler->AddSchedule(ITPK_UDP_ANTIHACK_CHECK, iUDPTimer );
	m_scheduler->AddSchedule(ITPK_ANTIHACK_RELOAD, 60000 );
#endif

	//Billing �� ���� �����ڼ� �����°� �ؾ���
	//��ƾ�϶���
	if( ioLocalManager::GetLocalType() == ioLocalManager::LCT_LATIN )
	{
		m_scheduler->AddSchedule(ITPK_BILLING_CCU_COUNT, 60000);	//1�и��� ������ �����ڼ� ����
	}

	//������ ��Ī
	m_scheduler->AddSchedule(ITPK_LADDER_TEAM_MATCHING, 1000);
}

void Manager::Timer()
{
	while( TRUE )
	{
		m_scheduler->Begin();
		while( TRUE )
		{
			Schedule* pSchedule = m_scheduler->GetSchedule();
			if( pSchedule )
			{
				m_scheduler->Call( pSchedule->GetCommand() );
			}
			else
			{
				break;
			}
		}
		m_scheduler->End();

		Sleep(1);
	}
}
