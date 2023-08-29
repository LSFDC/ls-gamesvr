#include "stdafx.h"
#include "TestNodeManager.h"
#include "ServerNodeManager.h"
#include "TournamentManager.h"
#include "UserNodeManager.h"


#include "../MainProcess.h"

#include "../EtcHelpFunc.h"
#include "../DataBase/DBClient.h"
#include "../QueryData/QueryResultData.h"
#include "../MainServerNode/MainServerNode.h"

template<> ioTestNodeManager* Singleton< ioTestNodeManager >::ms_Singleton = 0;

ioTestNodeManager::ioTestNodeManager( bool bTestServer )
{
	m_bLogPrint   = false;
	m_bTestServer = false;
}

ioTestNodeManager::~ioTestNodeManager()
{
	DestroyNode();
}

ioTestNodeManager& ioTestNodeManager::GetSingleton()
{
	return Singleton< ioTestNodeManager >::GetSingleton();
}

int ioTestNodeManager::IsLogPrint()
{
	if( m_bLogPrint )
		return  LOG_DEBUG_LEVEL;

	return LOG_TEST_LEVEL;
}

void ioTestNodeManager::LoadINI()
{
	DestroyNode();

	ioINILoader kLoader( "config/sp2_test_node.ini" );
	kLoader.SetTitle( "common" );
	
	m_bLogPrint   = kLoader.LoadBool( "log_print", false );	
	if( !m_bTestServer )
	{
		return;
	}	
	
	int iMaxNode  = kLoader.LoadInt( "max_node", 0 );

	char szKey[MAX_PATH] = "";
	for( int i = 0; i < iMaxNode; ++i )
	{		
		sprintf_s( szKey, "Node%d", i+1 );
		kLoader.SetTitle( szKey );
		
		TestNode* pNode = NULL;
		BOTTYPE Type = (BOTTYPE)kLoader.LoadInt( "type", 0 );
		int iSubType = kLoader.LoadInt( "sub_type", 0 );

		switch ( Type )
		{
		case BOT_USER:
			pNode = CreateUserBotTemplete( kLoader, iSubType );
			break;
		default:
			{
				LOG.PrintTimeAndLog( IsLogPrint(), "%s Load Fail : UnKown Type %d", __FUNCTION__, (int)Type );
				continue;
			}
		}

		if( pNode && pNode->LoadINI( kLoader ) )
		{
			pNode->SetType( Type );
			pNode->SetSubType( iSubType );
			pNode->OnCreate();
			m_TestNodeMap.insert( TestNodeMap::value_type( pNode->GetGUID(), pNode ) );
			LOG.PrintTimeAndLog( IsLogPrint() ,"%s - insert Ok : %s(%d, %d, %d)", __FUNCTION__, pNode->GetGUID().c_str(), (int)Type, iSubType, i );
		}
		else
		{
			LOG.PrintTimeAndLog(0, "%s Load Fail : INI FAIL( %d, %d, %d)", __FUNCTION__, (int)Type, iSubType, i );
		}
	}	
}

UserBot* ioTestNodeManager::CreateUserBotTemplete( ioINILoader &rkLoader, int iSubType )
{
	UserBot* pNode = NULL;
	switch( iSubType )
	{	
	case BOT_TURNAMENT:
		pNode = new TournamentBot;
		break;
	}

	return pNode;
}

void ioTestNodeManager::ServerDownAllSave()
{
}

void ioTestNodeManager::DestroyNode()
{
	if( m_TestNodeMap.empty() )
		return;

	TestNodeMap::iterator iter = m_TestNodeMap.begin();
	for( ; iter != m_TestNodeMap.end(); ++iter )
	{
		TestNode* pNode = iter->second;
		if( pNode )
			pNode->OnDestroy();

		SAFEDELETE(pNode);
	}
}

TestNode* ioTestNodeManager::GetNode( const ioHashString& szGUID )
{
	TestNodeMap::iterator iter = m_TestNodeMap.find( szGUID );
	if( iter == m_TestNodeMap.end() )
		return NULL;

	return iter->second;
}

void ioTestNodeManager::GetNodeList( TestNodeVec& DestTourVec )
{
	TestNodeMap::iterator iter = m_TestNodeMap.begin();
	for( ; iter != m_TestNodeMap.end(); ++iter )
	{
		TestNode* pNode = iter->second;
		if( pNode )			
			DestTourVec.push_back( pNode );		
	}
}

void ioTestNodeManager::OnUpdate()
{
	TestNodeMap::iterator iter = m_TestNodeMap.begin();
	for( ; iter != m_TestNodeMap.end(); ++iter )
	{
		TestNode* pNode = iter->second;
		if( pNode )
		{
			pNode->OnUpdate();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

TestNode::TestNode()
{
	char szTempGUID[USER_GUID_NUM_PLUS_ONE]="";
	Help::GetGUID(szTempGUID, sizeof(szTempGUID) );
	m_szGUID = szTempGUID;
}

TestNode::~TestNode()
{
}

bool TestNode::LoadINI( ioINILoader &rkLoader )
{
	return true;
}

bool TestNode::OnUpdate()
{
	return true;
}

void TestNode::OnCreate()
{

}

void TestNode::OnDestroy()
{

}

////////////////////////////////////////////////////////////////////////////////////////////
//
// �׽�Ʈ�� �������� ���, ������ �������� �ʾ����� ���� ������ ���
// ���� ������ �������� �׽�Ʈ�� �ϰ��� �Ҷ� ���
//
UserBot::UserBot()
{
	m_dwUserID		= 0;
	m_dwDBAgentID	= 0;
	m_State			= ULS_NONE;
	m_iDBCampPos	= CAMP_NONE;

	m_bUserConnect = false;
}

UserBot::~UserBot()
{

}

bool UserBot::LoadINI( ioINILoader &rkLoader )
{
	TestNode::LoadINI( rkLoader );

	char szBuf[MAX_PATH];
	rkLoader.LoadString( "private_id", "", szBuf, MAX_PATH );
	m_szPrivateID = szBuf;

	if( m_szPrivateID.IsEmpty() )
		return false;

	m_dwUserID	  = 0;
	m_dwDBAgentID = Help::GetUserDBAgentID( m_szPrivateID );

	return true;
}

UserBot* UserBot::ToUserBot( TestNode *pNode )
{
	if( !pNode || pNode->GetType() != BOT_USER )
		return NULL;

	return static_cast< UserBot* > ( pNode );
}

bool UserBot::IsActive()
{
	switch( m_State )
	{
	case ULS_LOADDB_DONE:
		return true;
	}

	return false;
}

void UserBot::OnCreate()
{

}

void UserBot::OnDestroy()
{

}

bool UserBot::OnUpdate()
{
	if( !g_DBClient.IsDBAgentActive( GetDBAgentID() ) )
		return false;
	
	switch( m_State )
	{
	case ULS_NONE:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s, %s", __FUNCTION__, GetGUID().c_str() );
			g_DBClient.OnSelectUserData( GetDBAgentID(), GetAgentThreadID(), GetGUID(), GetPrivateID() );
			SetState( ULS_LOADDB );			
		}
		return false;

	case ULS_LOADDB:
		return false;

	case ULS_LOADDB_FAIL:
		return false;
	}

	//������ ������ �����ϸ� ���μ����� ���� ���� ����
	if( g_UserNodeManager.GetUserNode( GetUserIndex() ) != NULL )
	{
		m_bUserConnect = true;
		return false;
	}
	m_bUserConnect = false;
	return true;
}

void UserBot::ApplyDBUserData( CQueryResultData *query_data )
{
	//SELECT
	int  user_idx = 0;
	char szDBID[ID_NUM_PLUS_ONE] = "";
	char nick_name[ID_NUM_PLUS_ONE] = "";
	query_data->GetValue(user_idx,sizeof(int));
	query_data->GetValue(szDBID,ID_NUM_PLUS_ONE);
	query_data->GetValue(nick_name,ID_NUM_PLUS_ONE);

	// �빮�ڰ� ������ ��츦 ���� ���� �ӽ� ���� �ڵ�	JCLEE 140507
	CString strDbID;
	strDbID = szDBID;
	strDbID.MakeLower();

	CString strPrivateID;
	strPrivateID = GetPrivateID().c_str();
	strPrivateID.MakeLower();
		
	if( strPrivateID != strDbID )
	{
		LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "private_id error : %s", GetGUID().c_str() );
		SetState( ULS_LOADDB_FAIL );
		return;
	}
	else if( user_idx <= 0 )
	{
		LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "user_idx error : %s", GetGUID().c_str() );
		SetState( ULS_LOADDB_FAIL );
		return;
	}

	m_dwUserID = user_idx;

	__int64 iMoney;
	int iUserState, iConnectCnt, iGreade, iGradeExp, iEventType, iRank;
	short iEntryType;

	//���� ���� �ð�
	DBTIMESTAMP dts;

	query_data->GetValue( iUserState,sizeof(int) );
	query_data->GetValue( iMoney,sizeof(__int64) );
	query_data->GetValue( iConnectCnt,sizeof(int) );
	query_data->GetValue( (char*)&dts, sizeof(DBTIMESTAMP) );
	query_data->GetValue( iGreade,sizeof(int) );	
	query_data->GetValue( iGradeExp,sizeof(int) );	
	query_data->GetValue( iEventType ,sizeof(int) );
	query_data->GetValue( iRank,sizeof(int) );
	query_data->GetValue( iEntryType,sizeof(short) ); // db smallint	
	query_data->GetValue( (char*)&dts, sizeof(DBTIMESTAMP) );	
	query_data->GetValue( m_iDBCampPos,sizeof(int) );

	SetState( ULS_LOADDB_DONE );

	LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s OK : %s", __FUNCTION__, GetGUID().c_str() );
	
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// ��ȸ�׽�Ʈ�� ���, ������ �������� �ʾ����� ������ ����Ͽ� ���� �� ��ȸ �� ����, �����ϱ� ����
//

TournamentBot::TournamentBot()
{
	m_dwTeamIndex  = 0;

	m_iLadderPoint = 0;
	m_CampPos	   = 0;

	m_TourBotType  = TBT_NONE;
	m_State		   = TS_NONE;
	m_CheerState   = TCS_NONE;
	
	m_dwJoinCheckTime  = 0;
	m_dwCheerCheckTime = 0;
	m_dwCurrCheckTime  = 0;

	m_dwTryCount = 0;
	m_dwPrevTournamentStartDate = 0;

	m_dwCheerTeam = 0;
}

TournamentBot::~TournamentBot()
{

}

bool TournamentBot::LoadINI( ioINILoader &rkLoader )
{
	if( !UserBot::LoadINI( rkLoader ) )
		return false;
		
	m_iLadderPoint	= rkLoader.LoadInt( "ladder_point", 0 );
	m_CampPos		= rkLoader.LoadInt( "camp", 0 );
	
	char szBuf[MAX_PATH];
	rkLoader.LoadString( "team_name", "", szBuf, MAX_PATH );
	m_szTeamName = szBuf;

	m_dwJoinCheckTime  = rkLoader.LoadInt( "join_check_time", 300000 );
	m_dwCheerCheckTime = rkLoader.LoadInt( "cheer_check_time", 300000 );
	m_dwCheerTeam	   = rkLoader.LoadInt( "cheer_check_time", 300000 );

	m_TourBotType	  = (TOURNAMENTBOTTYPE)rkLoader.LoadInt( "tournament_bot_type", 0 );
	if( m_TourBotType == TBT_NONE )
		return false;

	if( m_szTeamName.IsEmpty() )
		return false;
	
	return true;
}

TournamentBot* TournamentBot::ToTournamentBot( TestNode *pNode )
{
	UserBot* pUser = ToUserBot( pNode );
	if( !pUser || pUser->GetSubType() != BOT_TURNAMENT )
		return NULL;

	return static_cast< TournamentBot* > ( pNode );
}

TournamentBot* TournamentBot::GetTournamentBot( DWORD dwUserIdx )
{
	TestNodeVec BotVec;
	g_TestNodeMgr.GetNodeList( BotVec );
	TestNodeVec::iterator iter = BotVec.begin();
	for( ; iter != BotVec.end(); ++iter )
	{
		TournamentBot* pNode = ToTournamentBot( *iter );
		if( pNode && pNode->GetUserIndex() == dwUserIdx && !pNode->IsUserConnect() )
		{
			return pNode;
		}
	}

	return NULL;
}


void TournamentBot::OnCreate()
{

}

void TournamentBot::OnDestroy()
{

}

bool TournamentBot::OnUpdate()
{
	//���� ������ �ε�ɶ����� ���
	if( !UserBot::OnUpdate() )
		return false;

	DWORD dwTourIndex = g_TournamentManager.GetRegularTournamentIndex();

	//���Դ�ȸ������ �����ɶ����� ���
	if( dwTourIndex <= 0 )
		return false;
	
	switch( m_TourBotType )
	{
	case TBT_TOURNAMENT:
		ProcessTournamentJoinState( dwTourIndex );
		break;
	case TBT_CHEER:
		ProcessCheerTryState( dwTourIndex );
		break;
	}
	
	return true;
}

void TournamentBot::ProcessTournamentJoinState( DWORD dwTourIdx )
{
	switch( m_State )
	{
	case TS_NONE:
		{
			CampJoinTry( dwTourIdx );
		}
		break;
	case TS_JOIN:
		{
			TournamentJoinTry( dwTourIdx );
		}
		break;
	case TS_TEAM_GET:
		{
			//�������� ����� �ش� ������Ʈ���� ������Ʈ ���� ������ ����
			//���� ��� DB���� ������� ������� �ʾ� ���� ��ȯ�Ǵ� ��Ȳ
			TournamentJoinCheck( dwTourIdx );
		}
		break;
	case TS_TEAM_GET_FAIL:
		{
			//���н� �ٽ� �õ�(���ԱⰣ����..)
			TournamentJoinCheck( dwTourIdx );
		}
		break;
	case TS_TEAM_GET_OK:
		{
			//����ϱ�
			TournamentReStartCheck( dwTourIdx );
		}
		break;
	case TS_NOT_EXIST_TEAM:
		{
			//�����Ⱓ�� ��� ����δ� ���� ����
			//������ȸ�� �ö����� ���
			TournamentReStartCheck( dwTourIdx );
		}
	}
}

void TournamentBot::CampJoinTry( DWORD dwTourIdx )
{
	//�����Ⱓ�� �ƴϸ� �������� X
	if( g_TournamentManager.GetTournamentState( dwTourIdx ) == TournamentManager::STATE_TEAM_APP )
	{
		//���� ���� ������Ʈ
		if( GetDBCampPos() == CAMP_NONE )
		{
			g_DBClient.OnUpdateUserCampPosition( GetDBAgentID(), GetAgentThreadID(), GetUserIndex(), m_CampPos );
			SetDBCampPos( m_CampPos );
		}
	}
	m_State = TS_JOIN;
}

void TournamentBot::TournamentJoinTry( DWORD dwTourIdx )
{
	//�����Ⱓ�̸�
	if( g_TournamentManager.GetTournamentState( dwTourIdx ) == TournamentManager::STATE_TEAM_APP )
	{
		//��ȸ�� �����
		g_DBClient.OnInsertTournamentTeamCreate( GetDBAgentID(), GetAgentThreadID(), GetUserIndex(), dwTourIdx, m_szTeamName, 1, m_iLadderPoint, m_CampPos );
		m_State = TS_TEAM_GET;
		m_dwCurrCheckTime = TIMEGETTIME();

		LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s Tournament Join Try : %d", __FUNCTION__, m_dwTryCount );
	}
	else
	{
		//������ ���� ���ִٸ� ��ȸ�� ���� ���ϼ��� �������� ��ȸ ���� ���θ� �Ǵ�
		if( GetDBCampPos() != CAMP_NONE && m_dwTeamIndex == 0 )
		{
			g_DBClient.OnSelectTournamentTeamList( GetDBAgentID(),GetAgentThreadID(), GetGUID(), GetUserIndex(), MAX_INT_VALUE, TOURNAMENT_TEAM_MAX_LOAD );
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s my tournament team index select count : %d", __FUNCTION__, m_dwTryCount );
		}
		m_State = TS_TEAM_GET;
	}
}

void TournamentBot::TournamentJoinCheck( DWORD dwTourIdx )
{
	if( m_dwJoinCheckTime < TIMEGETTIME() - m_dwCurrCheckTime )
	{	
		if( g_TournamentManager.GetTournamentState( dwTourIdx ) != TournamentManager::STATE_TEAM_APP )
		{
			//���ԱⰣ�� �ƴϸ�
			m_State			  = TS_NOT_EXIST_TEAM;
			m_dwCurrCheckTime = 0;
			m_dwTryCount	  = 0;
		}
		else
		{
			//���ԱⰣ�̸� �ٽ� �õ�
			m_State			  = TS_NONE;
			m_dwCurrCheckTime = TIMEGETTIME();
			m_dwTryCount++;
		}
	}
}

void TournamentBot::TournamentReStartCheck( DWORD dwTourIdx )
{	
	if( m_dwPrevTournamentStartDate == 0 )
		m_dwPrevTournamentStartDate = g_TournamentManager.GetTournamentStartDate( dwTourIdx );

	if( 0 < m_dwPrevTournamentStartDate && m_dwPrevTournamentStartDate != g_TournamentManager.GetTournamentStartDate( dwTourIdx ) )
	{
		m_State						= TS_NONE;
		m_CheerState				= TCS_NONE;
		m_dwCurrCheckTime			= 0;
		m_dwTryCount				= 0;
		m_dwPrevTournamentStartDate = 0;
	}
}

void TournamentBot::ApplyTeamCreateOk( DWORD dwTeamIndex )
{
	m_dwTeamIndex = dwTeamIndex;
	m_State		  = TS_TEAM_GET_OK;
	m_dwTryCount  = 0;

	LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s Team Crreate OK : %d, %s", __FUNCTION__, m_dwTeamIndex, m_szTeamName.c_str() );
}

void TournamentBot::ApplyTeamCreateFail()
{
	m_State		  = TS_TEAM_GET_FAIL;
	m_dwTeamIndex = 0;

	LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s Team Crreate FAIL : %d, %s", __FUNCTION__, m_dwTeamIndex, m_szTeamName.c_str() );
}

void TournamentBot::ApplyUserTeamIndex( CQueryResultData *query_data )
{
	LOOP_GUARD();

	int iCount = 0;
	DWORD dwTeamIndex = 0;
	while( query_data->IsExist() )
	{
		iCount++;
		DWORD dwTourIndex, dwOwnerIndex;
		char szTeamName[TOURNAMENT_TEAM_NAME_NUM_PLUS_ONE] = "";

		query_data->GetValue( dwTeamIndex, sizeof(DWORD) );
		query_data->GetValue( dwTourIndex, sizeof(DWORD) );
		query_data->GetValue( szTeamName, TOURNAMENT_TEAM_NAME_NUM_PLUS_ONE );
		query_data->GetValue( dwOwnerIndex, sizeof(DWORD) );

		if( dwTourIndex == 0 )		
			continue;		

		if( dwTourIndex != g_TournamentManager.GetRegularTournamentIndex() )
			continue;

		if( GetUserIndex() == dwOwnerIndex && strcmp( m_szTeamName.c_str(), szTeamName ) == 0 )
		{
			LOOP_GUARD_CLEAR();
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - my team index find ok", __FUNCTION__ );
			m_dwTeamIndex = dwTeamIndex;
			m_State	= TS_TEAM_GET_OK;
			return;
		}
	}
	LOOP_GUARD_CLEAR();

	if( iCount == 0 )
	{
		m_State	= TS_TEAM_GET_FAIL;
		LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - my team index find fail", __FUNCTION__  );
		return;
	}

	if( iCount >= TOURNAMENT_TEAM_MAX_LOAD )
	{
		g_DBClient.OnSelectTournamentTeamList( GetDBAgentID(),GetAgentThreadID(), GetGUID(), GetUserIndex(), dwTeamIndex, TOURNAMENT_TEAM_MAX_LOAD );
	}

}

void TournamentBot::ProcessCheerTryState( DWORD dwTourIdx )
{
	if( g_TournamentManager.GetTournamentState( dwTourIdx ) != TournamentManager::STATE_TEAM_DELAY )
		return;
	
	switch( m_CheerState )
	{
	case TCS_NONE:
		{
			TournamentCheerNone( dwTourIdx );
		}
		break;
	case TCS_CHEER_TRY:
		{
			TournamentCheerTry( dwTourIdx );
		}
		break;
	case TCS_RESULT_GET:
		{
			//����� ���
		}
		break;
	case TCS_CHEER_DECISION_OK:
		{
			TournamentReStartCheck( dwTourIdx );
		}
		break;
	case TCS_CHEER_DECISION_FAIL:
		{
			TournamentReStartCheck( dwTourIdx );
		}
		break;
	}
}

void TournamentBot::TournamentCheerNone( DWORD dwTourIdx )
{
	if( m_dwCurrCheckTime == 0 )
		m_dwCurrCheckTime = TIMEGETTIME();

	if( m_dwCheerCheckTime < TIMEGETTIME() - m_dwCurrCheckTime )
	{				
		m_CheerState = TCS_CHEER_TRY;
	}
}

void TournamentBot::TournamentCheerTry( DWORD dwTourIdx )
{
	//g_DBClient.OnInsertTournamentCheerDecision( GetDBAgentID(), GetAgentThreadID(), GetUserIndex(), m_dwTeamIndex, dwTeamIdx );
	m_CheerState = TCS_RESULT_GET;
}

void TournamentBot::ApplyCheerDecisionOK( SP2Packet &rkPacket )
{
	DWORD dwTourIdx, dwTeamIdx;

	rkPacket >> dwTourIdx;
	rkPacket >> dwTeamIdx;

	m_CheerState = TCS_CHEER_DECISION_OK;
	LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s", __FUNCTION__  );
}

void TournamentBot::ApplyCheerDecisionFail( int iResult )
{
	m_CheerState = TCS_CHEER_DECISION_FAIL;

	switch( iResult )
	{
	case TOURNAMENT_CHEER_DECISION_ALREADY:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - alredy cheer team", __FUNCTION__ );
		}
		break;
	case TOURNAMENT_CHEER_DECISION_CLOSE:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - tournament close", __FUNCTION__ );
		}
		break;
	case TOURNAMENT_CHEER_DECISION_NOT_REGULAR:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - not regular tournament", __FUNCTION__ );
		}
		break;
	case TOURNAMENT_CHEER_DECISION_NONE_TEAM:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s - not find tournament team", __FUNCTION__ );
		}
		break;
	default:
		{
			LOG.PrintTimeAndLog( g_TestNodeMgr.IsLogPrint(), "%s", __FUNCTION__ );
		}
		break;		
	}	
}