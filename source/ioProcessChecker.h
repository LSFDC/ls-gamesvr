
#ifndef _ioProcessChecker_h_
#define _ioProcessChecker_h_

#include "ioPacketStatistics.h"
#include <unordered_map>


class FunctionTimeChecker
{
protected:
	LARGE_INTEGER m_freq;
	UINT64		  m_start;
	UINT64		  m_end;

	bool		  m_bFrequency;
	LONG		  m_iFileLine;	
	LPSTR         m_pFileName;
	double        m_fCheckMicSec;
	DWORD         m_dwPacketID;

public:
	FunctionTimeChecker( LPSTR pFileName, LONG iFileLine, double fCheckMicSec, DWORD dwPacketID );
	virtual ~FunctionTimeChecker();
};
#define FUNCTION_TIME_CHECKER( f, d )        FunctionTimeChecker ftc( __FILE__, __LINE__, f, d )
//////////////////////////////////////////////////////////////////////////
class ioProcessChecker
{
private:
	static ioProcessChecker *sg_Instance;

protected:
	DWORD m_dwCurTime;
	DWORD m_dwLogTime;										// ���ʿ� 1�� �α׸� ������ΰ�?

	// ������ ���� Ƚ��
	DWORD m_dwMainLoop;										// LogTime���� ���� ���� ȣ�� Ƚ��
	DWORD m_dwLogDBLoop;									// LogTime���� LogDB ���� ȣ�� Ƚ��
	DWORD m_dwUDPLoop;                                      // LogTime���� UDP ���� ȣ�� Ƚ��
	DWORD m_dwClientAcceptLoop;                             // LogTime���� ClientAccept ���� ȣ�� Ƚ��
	DWORD m_dwServerAcceptLoop;                             // LogTime���� ServerAccept ���� ȣ�� Ƚ��
	DWORD m_dwMonitoringAcceptLoop;                         // LogTime���� MonitoringAccept ���� ȣ�� Ƚ��

	// ��Ŷ ����
	__int64 m_iUserSend;                                     // ������ �������� ���� ��û�� ��Ŷ�� ������
	__int64 m_iUserSendComplete;                             // ������ �������� ���� ó���� ��Ŷ�� ������
	__int64 m_iUserRecv;                                     // �������� ���� ��Ŷ ������
	__int64 m_iServerSend;                                   // ������ �������� ���� ��û�� ��Ŷ�� ������
	__int64 m_iServerSendComplete;                           // ������ �������� ���� ó���� ��Ŷ�� ������
	__int64 m_iServerRecv;                                   // �������� ���� ��Ŷ ������
	__int64 m_iMServerSend;                                  // ������ ���μ������� ���� ��û�� ��Ŷ�� ������
	__int64 m_iMServerSendComplete;                          // ������ ���μ������� ���� ó���� ��Ŷ�� ������
	__int64 m_iMServerRecv;                                  // ���μ������� ���� ��Ŷ ������
	__int64 m_iDBServerSend;                                 // ������ DB�������� ���� ��û�� ��Ŷ�� ������
	__int64 m_iDBServerSendComplete;                         // ������ DB�������� ���� ó���� ��Ŷ�� ������
	__int64 m_iDBServerRecv;                                 // DB�������� ���� ��Ŷ ������
	__int64 m_iLogDBSend;									 // ������ LogDB�������� ���� ��û�� ��Ŷ�� ������
	__int64 m_iLogDBRecv;									 // LogDB�������� ���� ��Ŷ ������
	__int64 m_iUDPSend;										 // ������ UDP ���� ��û�� ��Ŷ�� ������
	__int64 m_iUDPRecv;										 // UDP ���� ��Ŷ ������
	__int64 m_iBillingRelayServerSend;                       // ������ BillingRelayServer���� ���� ��û�� ��Ŷ�� ������
	__int64 m_iBillingRelayServerSendComplete;               // ������ BillingRelayServer���� ���� ó���� ��Ŷ�� ������
	__int64 m_iBillingRelayServerRecv;                       // BillingRelayServer���� ���� ��Ŷ ������
	__int64 m_iMonitoringSend;                               // ������ Monitoring���� ���� ��û�� ��Ŷ�� ������
	__int64 m_iMonitoringSendComplete;                       // ������ Monitoring���� ���� ó���� ��Ŷ�� ������
	__int64 m_iMonitoringRecv;                               // Monitoring���� ���� ��Ŷ ������
	int     m_iMainProcessMaxPacket;                         // ���� ���μ������� �� �������� ó���ϴ� ��Ŷ�� �� ���� ���� ��Ŷ

	ioPacketStatistics m_ServerStatistics;

	// ������ ���� �ð�
protected:
	struct PerformanceTime
	{
		LARGE_INTEGER m_freq;
		UINT64		  m_start;
		UINT64		  m_end;
		bool		  m_bFrequency;		
	};
	PerformanceTime   m_MainThreadTime;
	double            m_fMainTreadMaxTime;

	PerformanceTime   m_BroadcastThreadTime;
	double            m_fBroadcastTreadMaxTime;

	PerformanceTime   m_WorkThreadTime;
	double            m_fWorkTreadMaxTime;

	PerformanceTime   m_ServerAThreadTime;
	double            m_fServerATreadMaxTime;

	PerformanceTime   m_ClientAThreadTime;
	double            m_fClientATreadMaxTime;

	PerformanceTime   m_MonitoringAThreadTime;
	double            m_fMonitoringATreadMaxTime;

public:
	static ioProcessChecker &GetInstance();
	static void ReleaseInstance();

public:
	void LoadINI();
	void Initialize();
	void Process();
	void WriteLOG();

public:
	void MainThreadCheckTimeStart();
	void MainThreadCheckTimeEnd();
	void BroadcastThreadCheckTimeStart();
	void BroadcastThreadCheckTimeEnd();
	void WorkThreadCheckTimeStart();
	void WorkThreadCheckTimeEnd();
	void ServerAThreadCheckTimeStart();
	void ServerAThreadCheckTimeEnd();
	void ClientAThreadCheckTimeStart();
	void ClientAThreadCheckTimeEnd();
	void MonitoringAThreadCheckTimeStart();
	void MonitoringAThreadCheckTimeEnd();

public:
	void CallMainThread(){ m_dwMainLoop++; }
	void CallLogDBThread(){ m_dwLogDBLoop++; }
	void CallUDPThread(){ m_dwUDPLoop++; }
	void CallClientAccept(){ m_dwClientAcceptLoop++; }
	void CallServerAccept(){ m_dwServerAcceptLoop++; }
	void CallMonitoringAccept(){ m_dwMonitoringAcceptLoop++; }

public:
	void ProcessIOCP( int iConnectType, DWORD dwFlag, DWORD dwByteTransfer );
	void UserSendMessage( DWORD dwID, int iSize );
	void UserSendComplete( int iSize );
	void UserRecvMessage( int iSize );
	void ServerPacketProcess( DWORD dwID );
	void ServerSendMessage( DWORD dwID, int iSize );
	void ServerSendComplete( int iSize );
	void ServerRecvMessage( int iSize );
	void MainServerSendMessage( DWORD dwID, int iSize );
	void MainServerSendComplete( int iSize );
	void MainServerRecvMessage( int iSize );
	void DBServerSendMessage( DWORD dwID, int iSize );
	void DBServerSendComplete( int iSize );
	void DBServerRecvMessage( int iSize );
	void LogDBSendMessage( DWORD dwID, int iSize );
	void LogDBRecvMessage( int iSize );
	void UDPSendMessage( DWORD dwID, int iSize );
	void UDPRecvMessage( int iSize );
	void BillingRelayServerSendMessage( DWORD dwID, int iSize );
	void BillingRelayServerSendComplete( int iSize );
	void BillingRelayServerRecvMessage( int iSize );
	void MonitoringSendMessage( DWORD dwID, int iSize );
	void MonitoringSendComplete( int iSize );
	void MonitoringRecvMessage( int iSize );
	void MainProcessMaxPacket( int iPacketCnt );

private:
	ioProcessChecker();
	virtual ~ioProcessChecker();
};
#define g_ProcessChecker ioProcessChecker::GetInstance()
#endif