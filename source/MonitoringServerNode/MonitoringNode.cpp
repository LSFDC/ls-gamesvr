#include "stdafx.h"
//#include "../Window.h"
#include "../MainProcess.h"
#include "../Shutdown.h"
#include "../Network/GameServer.h"
#include "../Network/ioPacketQueue.h"
#include "../Util/RingBuffer.h"
#include "..\NodeInfo\UserNodeManager.h"
#include "MonitoringNodeManager.h"
#include ".\monitoringnode.h"

MonitoringNode::MonitoringNode( SOCKET s, DWORD dwSendBufSize, DWORD dwRecvBufSize ) : CConnectNode( s, dwSendBufSize, dwRecvBufSize ) 
{
	InitData();
}

MonitoringNode::~MonitoringNode(void)
{
}


void MonitoringNode::InitData()
{
	m_eSessionState = SS_DISCONNECT;
	m_iLogCnt = 0;
}

void MonitoringNode::OnCreate()
{
	CConnectNode::OnCreate();
	InitData();
	m_eSessionState = SS_CONNECT;
}

void MonitoringNode::OnDestroy()
{
	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonitoringNode::OnDestroy()" );
	CConnectNode::OnDestroy();
	m_eSessionState = SS_DISCONNECT;
}

void MonitoringNode::OnSessionDestroy()
{
}

void MonitoringNode::SessionClose(BOOL safely)
{
	if(!safely)
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CONNECT_TYPE_MONITORING Exception Close :%d", GetLastError() );
	}

	CPacket packet(MNSTPK_CLOSE);
	ReceivePacket( packet );
}

bool MonitoringNode::SendMessage( CPacket &rkPacket )
{
	g_ProcessChecker.MonitoringSendMessage( rkPacket.GetPacketID(), rkPacket.GetBufferSize() );
	if( m_socket == INVALID_SOCKET ) return false;

	CPacket SendPacket( rkPacket.GetBuffer(), rkPacket.GetBufferSize() );

	// (��Ŷ��� ����) �����̵� ����͸��� �°�
	return CConnectNode::SendMessage( SendPacket.GetBuffer() + SendPacket.GetCurPos(), SendPacket.GetBufferSize() - SendPacket.GetCurPos(), FALSE );
}

bool MonitoringNode::CheckNS( CPacket &rkPacket )
{
	return true;             //��Ʈ�� ���� �ʿ����.
}

int MonitoringNode::GetConnectType()
{
	return CONNECT_TYPE_MONITORING;
}

void MonitoringNode::ReceivePacket( CPacket &packet )
{
	g_RecvQueue.InsertQueue( (DWORD)this, packet, PK_QUEUE_SESSION );
}

void MonitoringNode::PacketParsing( CPacket &packet )
{
	SP2Packet &kPacket = (SP2Packet&)packet;

	FUNCTION_TIME_CHECKER( 500000.0f, kPacket.GetPacketID() );          // 0.5 �� �̻� �ɸ���α� ����

	switch( kPacket.GetPacketID() )
	{
	case MNSTPK_CLOSE:
		OnClose( kPacket );
		break;
	case MNSTPK_STATUS_REQUEST:
		OnStatus( kPacket );
		break;
	case MNSTPK_CHANGE_REQUEST:
		OnChange( kPacket );
		break;
 
	default:
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "MonitoringServer::PacketParsing �˼����� ��Ŷ : 0x%x", kPacket.GetPacketID() );
		break;
	}
}

void MonitoringNode::DispatchReceive( CPacket& packet, DWORD bytesTransferred )
{
	m_recvIO.AddBytesTransferred( bytesTransferred );

	while( m_recvIO.GetBytesTransferred() > 0 )
	{
		// ����� ���� ��Ŷ�� ����� ���Ѵ�. CConnectNode::DispatchReceive() �̺κи� �ٸ�.
		if( m_recvIO.GetBytesTransferred() < 4 ) // 4: m_usCommand + m_usSize
			break;

		unsigned short usCommand = 0;
		unsigned short m_usSize  = 0;
		m_recvIO.GetBuffer( &usCommand, 2, 0 );
		m_recvIO.GetBuffer( &m_usSize, 2, 2 );

		DWORD dwPacketSize = 0;
		DWORD dwPacketID   = 0;
		if( usCommand == MONITORING_STATUS_CMD )
		{
			dwPacketID   = MNSTPK_STATUS_REQUEST; // �츮 ��Ŷ ID�� ��ȯ
			dwPacketSize = m_usSize; 
			if( !COMPARE( dwPacketSize, 0, sizeof( MonitorStatusRequest ) + 1 ) )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s MNSTPK_STATUS_REQUEST Error Size : %d", __FUNCTION__, dwPacketSize );
				break;
			}
		}
		else if( usCommand == MONITORING_CHANGE_CMD )
		{
			dwPacketID = MNSTPK_CHANGE_REQUEST; // �츮 ��Ŷ ID�� ��ȯ
			dwPacketSize = m_usSize; 
			if( !COMPARE( dwPacketSize, 0, sizeof( tagMonitorChangeRequest ) + 1 ) )
			{
				LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s MNSTPK_CHANGE_REQUEST Error Size : %d", __FUNCTION__, dwPacketSize );
				break;
			}
		}
		else
		{
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s Error ID : %d", __FUNCTION__, usCommand );
			break;
		}

		SP2Packet RecvPacket( dwPacketID );
		RecvPacket.SetDataAdd( (char*)m_recvIO.GetBuffer(), min( dwPacketSize, m_recvIO.GetBytesTransferred() ), true );

		if( (RecvPacket.IsValidPacket() == true) && (m_recvIO.GetBytesTransferred() >= dwPacketSize) )
		{
			if( !CheckNS( RecvPacket ) ) return;

			ReceivePacket( RecvPacket );

			m_recvIO.AfterReceive( dwPacketSize );
		}
		else 
			break;
	}

	WaitForPacketReceive();
}

void MonitoringNode::OnClose( SP2Packet &rkPacket )
{
	if(!IsDisconnectState())
	{
		OnDestroy();
		OnSessionDestroy();
		
		g_MonitoringNodeManager.RemoveNode(this);
	}
}

void MonitoringNode::OnStatus( SP2Packet &rkPacket )
{
	MonitorStatusResult kStatus;
	// ���Ӽ��� ���� ó�� ��
	if( g_App.IsReserveLogOut() )
		kStatus.m_cStatus = MonitorStatusResult::STATUS_EXITING;
	else
		kStatus.m_cStatus = MonitorStatusResult::STATUS_RUN;

	kStatus.m_iCuCount = g_UserNodeManager.GetNodeSize();

	m_iLogCnt++;
	if( (m_iLogCnt%100) == 0 )
	{
		LOG.PrintTimeAndLog( LOG_TEST_LEVEL, "%s %d", __FUNCTION__, kStatus.m_iCuCount );
		m_iLogCnt = 0;
	}
	SP2Packet kResult( MNSTPK_STATUS_RESULT );
	kResult << kStatus;
	SendMessage( kResult );
}

void MonitoringNode::OnChange( SP2Packet &rkPacket )
{
	MonitorChangeRequest kChange;
	rkPacket >> kChange;

	MonitorChangeResult kRChagne;

	if( kChange.m_iReqStatus == MonitorChangeRequest::CHANGE_OPEN ) 
	{
		// ��� ����
		kRChagne.m_iResult = MonitorChangeResult::CHANGE_FAIL;
	}
	else if( kChange.m_iReqStatus == MonitorChangeRequest::CHANGE_BLOCK ) 
	{
		// ��� ����
		kRChagne.m_iResult = MonitorChangeResult::CHANGE_FAIL;
	}
	else if( kChange.m_iReqStatus == MonitorChangeRequest::CHANGE_EXIT ) 
	{
		g_App.Shutdown(SHUTDOWN_SAFE);
		kRChagne.m_iResult = MonitorChangeResult::CHANGE_SUCCESS;
	}

	LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "%s %d", __FUNCTION__, kChange.m_iReqStatus );
	SP2Packet kResult( MNSTPK_CHANGE_RESULT );
	kResult << kRChagne;
	SendMessage( kResult );
}
