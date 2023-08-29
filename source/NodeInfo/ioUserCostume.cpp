#include "stdafx.h"
#include "ioUserCostume.h"
#include "User.h"
#include "CostumeManager.h"
#include "../ioCriticalError.h"
#include "../MainProcess.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "ioCharacter.h"
#include "../EtcHelpFunc.h"

ioUserCostume::ioUserCostume()
{
}

ioUserCostume::~ioUserCostume()
{
	Destroy();
}

void ioUserCostume::Init(User* pUser)
{
	m_mUserCostumeMap.clear();
	m_pUser = pUser;
}

void ioUserCostume::Destroy()
{
	m_mUserCostumeMap.clear();
	m_pUser = NULL;
}

void ioUserCostume::DBtoData( CQueryResultData *query_data, int& iLastIndex )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserCostume::DBtoData() User NULL!!"); 
		return;
	}

	LOOP_GUARD();
	while( query_data->IsExist() )
	{	
		Costume cCostume;

		DBTIMESTAMP dts;
		int iIndex			= 0;
		DWORD dwCostumeCode = 0;
		DWORD dwMCostume	= 0;
		DWORD dwFCostume	= 0;
		BYTE byPeriodType	= 0;
		int iClassType		= 0;	
		bool bChange		= false;

		PACKET_GUARD_BREAK( query_data->GetValue( iIndex, sizeof(int) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( dwCostumeCode, sizeof(int) ) );
#ifdef CUSTOM_COSTUME
		PACKET_GUARD_BREAK( query_data->GetValue( dwMCostume, sizeof(int) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( dwFCostume, sizeof(int) ) );
#endif
		PACKET_GUARD_BREAK( query_data->GetValue( byPeriodType, sizeof(BYTE) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( (char*)&dts, sizeof(DBTIMESTAMP) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( iClassType, sizeof(int) ) );				//�������� Ŭ����.

		if( 0 != iClassType && !m_pUser->IsCharClassType( iClassType ) )//( !m_pUser->IsCharClassType( iClassType ) || !m_pUser->IsActiveChar(iClassType) ) )
		{
			iClassType	= 0;
			bChange = true;
		}
		else
		{
			//�ش� Ŭ������ ����
			SetEquipInfo(iClassType, dwCostumeCode, iIndex, dwMCostume, dwFCostume );
		}

		CTime kLimitTime(Help::GetSafeValueForCTimeConstructor(dts.year,dts.month,dts.day, dts.hour,dts.minute,dts.second));
		
		cCostume.SetCostumeCode(dwCostumeCode);
		cCostume.SetCostumeIndex(iIndex);
#ifdef CUSTOM_COSTUME
		cCostume.SetMaleCustom( dwMCostume );
		cCostume.SetFemaleCustom( dwFCostume );
#endif
		cCostume.SetPeriodType(byPeriodType);
		cCostume.SetWearingClass(iClassType);
		cCostume.SetDate(kLimitTime.GetYear(), kLimitTime.GetMonth(), kLimitTime.GetDay(), kLimitTime.GetHour(), kLimitTime.GetMinute());

		m_mUserCostumeMap.insert(make_pair(iIndex, cCostume));

		//���� ��񿡴��� ������ index�� ������ �־�ߵ�.
		iLastIndex = iIndex;

		if( bChange )
		{
			SYSTEMTIME sys = {0,0,0,0,0,0,0,0};
			g_CostumeMgr.ConvertCTimeToSystemTime(sys, kLimitTime);
#ifdef CUSTOM_COSTUME
			g_DBClient.OnUpdateCostumeData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), iIndex, dwCostumeCode, dwMCostume, dwFCostume,  byPeriodType, sys, iClassType);
#else
			g_DBClient.OnUpdateCostumeData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), iIndex, dwCostumeCode, 0, 0, byPeriodType, sys, iClassType);
#endif
		}
	}
	LOOP_GUARD_CLEAR();
}

void ioUserCostume::FillMoveData( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID(rkPacket.Write((int)m_mUserCostumeMap.size()));
	UserCostumeItem::iterator it = m_mUserCostumeMap.begin();

	for( ; it != m_mUserCostumeMap.end() ; it++ )
	{
		Costume cCostume = it->second;
		cCostume.FillMoveData(rkPacket);
	}
}

void ioUserCostume::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode )
{
	int iSize = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		Costume cCostume;
		cCostume.ApplyMoveData(rkPacket);

		m_mUserCostumeMap.insert(make_pair(cCostume.GetCostumeIndex(), cCostume));
	}
}

bool ioUserCostume::IsEnableAdd()
{
	int iMaxCount = g_CriticalError.GetMaxCostumeCount();

	if( iMaxCount <= (int)m_mUserCostumeMap.size() )
		return false;

	return true;
}

void ioUserCostume::AddCostumeItem(Costume& costumeInfo)
{
	m_mUserCostumeMap.insert( make_pair(costumeInfo.GetCostumeIndex(), costumeInfo) );
}

bool ioUserCostume::DeleteCostumeItem(DWORD& dwIndex)
{
	if( !m_pUser )
		return false;

	UserCostumeItem::iterator it = m_mUserCostumeMap.find(dwIndex);

	if( it == m_mUserCostumeMap.end() )
		return false;	//���� ������

	m_mUserCostumeMap.erase(it);
	
	//DB����
	g_DBClient.OnDeleteCostumeData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), dwIndex);
	return true;	//����
}


bool ioUserCostume::DeleteCostumeSkin( INT dwIndex, BYTE byDeleteType )
{
	if( byDeleteType < SDT_MALE || byDeleteType > SDT_FEMALE )
	{
		LOG.PrintTimeAndLog( 0 , " %s - unknowable gender", __FUNCTION__ );
		return FALSE;
	}

	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( 0 , " %s - unknow user", __FUNCTION__ );
		return FALSE;
	}

	g_DBClient.OnDeleteCostumeSkinData( m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), dwIndex, m_pUser->GetUserIndex(), byDeleteType);
	return TRUE;
}

bool ioUserCostume::IsEmpty()
{
	if( m_mUserCostumeMap.size() != 0 )
		return false;

	return true;
}

Costume* ioUserCostume::GetCostume(DWORD dwIndex)
{
	UserCostumeItem::iterator it = m_mUserCostumeMap.find(dwIndex);
	if( it == m_mUserCostumeMap.end() )
		return NULL;

	return &(it->second);
}

int ioUserCostume::ChangeEquipInfo(int iClassArray, DWORD& dwTargetIndex, int iEquipPos, bool bEquip, Costume* pCostume)
{
	if( !m_pUser )
		return COSTUME_CHANGE_EXECPTION;

	if( iEquipPos < 0 || iEquipPos >= MAX_CHAR_COSTUME_SLOT )
		return COSTUME_CHANGE_DISABLE_POS;

	if( !pCostume )
		return COSTUME_CHANGE_NO_TARGET;

	if( pCostume->GetCostumeCode()/COSTUME_SUBTYPE_DELIMITER != iEquipPos )
		return COSTUME_CHANGE_DISABLE_POS;
	
	ioCharacter* pCharacter = m_pUser->GetCharacter(iClassArray);
	if( !pCharacter )
		return COSTUME_CHANGE_NO_CHARACTER;

	if( pCostume->GetWearingClass() != 0 )
	{
		int iArray = m_pUser->GetCharArray(pCostume->GetWearingClass());
		ioCharacter* pChar =  m_pUser->GetCharacter(iArray);
		if( pChar )
		{
			int iArray = pChar->GetCostumeSlot(dwTargetIndex);
			if( iArray >= 0 )
				return COSTUME_CHANGE_EQUIP_ITEM;
			pCostume->SetWearingClass(0);
		}
		else
			pCostume->SetWearingClass(0);
	}

	int iPrevCostumeIndex = pCharacter->GetCostumeIndex(iEquipPos);

	if( bEquip )
	{
		Costume* pPrevCostume = GetCostume(iPrevCostumeIndex);

		if( pPrevCostume )
		{
			ReleaseCostume(pPrevCostume);
		}
#ifdef CUSTOM_COSTUME
		pCharacter->ChangeCostumeItem( iEquipPos, dwTargetIndex, pCostume->GetCostumeCode(), pCostume->GetMaleCustom(), pCostume->GetFemaleCustom() );
#else
		pCharacter->ChangeCostumeItem(iEquipPos, dwTargetIndex, pCostume->GetCostumeCode());
#endif
		EquipCostume(pCharacter->GetClassType(), pCostume);
	}
	else
	{
		//ĳ���Ϳ� ���� ����
		pCharacter->ChangeCostumeItem(iEquipPos, 0, 0);
		ReleaseCostume(pCostume);
	}

	return COSTUME_CHANGE_SUCCESS;
}

void ioUserCostume::ReleaseCostumeWithCostumeIndex(DWORD dwIndex)
{
	UserCostumeItem::iterator it = m_mUserCostumeMap.find(dwIndex);
	if( it == m_mUserCostumeMap.end() )
		return;

	Costume* pCostume = &(it->second);
	if( !pCostume )
		return;

	ReleaseCostume(pCostume);
}

void ioUserCostume::ReleaseCostumeWithClassType(int iClassType)
{
	UserCostumeItem::iterator it = m_mUserCostumeMap.begin();
	if( it == m_mUserCostumeMap.end() )
		return;

	for( ; it != m_mUserCostumeMap.end(); it++ )
	{
		Costume* pCostume = &(it->second);
		if( !pCostume )
			continue;

		if( pCostume->GetWearingClass() == iClassType )
		{
			ReleaseCostume(pCostume);
		}
	}
}
void ioUserCostume::EquipCostume(const int iClassType, Costume* pCostume)
{
	if( !m_pUser )
		return;

	if( pCostume )
	{
		pCostume->SetWearingClass(iClassType);
		//���� ���� DBupdate
		SYSTEMTIME sysTime = {0,0,0,0,0,0,0,0};
		pCostume->GetCostumeLimitDate(sysTime);
		g_DBClient.OnUpdateCostumeData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), pCostume->GetCostumeIndex(), 
			pCostume->GetCostumeCode(), pCostume->GetMaleCustom(), pCostume->GetFemaleCustom(), pCostume->GetPeriodType(), sysTime, pCostume->GetWearingClass());
	}
}

void ioUserCostume::ReleaseCostume(Costume* pCostume)
{
	if( !m_pUser )
		return;

	if( pCostume )
	{
		pCostume->SetWearingClass(0);
		SYSTEMTIME sysTime = {0,0,0,0,0,0,0,0};
		pCostume->GetCostumeLimitDate(sysTime);
		g_DBClient.OnUpdateCostumeData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(),  pCostume->GetCostumeIndex(), pCostume->GetCostumeCode(), pCostume->GetMaleCustom(), pCostume->GetFemaleCustom(), pCostume->GetPeriodType(),
			sysTime, 0);
	}
}

void ioUserCostume::DeleteCostumePassedDate(IntVec &vDeleteIndex)
{
	CTime kCurTime = CTime::GetCurrentTime();

	UserCostumeItem::iterator it = m_mUserCostumeMap.begin();
	if( it == m_mUserCostumeMap.end() )
		return;

	while( it != m_mUserCostumeMap.end() )
	{
		Costume *pCostume = &(it->second);
		if( pCostume )
		{
			if( pCostume->GetPeriodType() == PCPT_MORTMAIN )
			{
				++it;
				continue;
			}

			CTime kLimitTime( Help::GetSafeValueForCTimeConstructor( pCostume->GetYear(),
																	 pCostume->GetMonth(),
																	 pCostume->GetDay(),
																	 pCostume->GetHour(),
																	 pCostume->GetMinute(),
																	 0) );

			CTimeSpan kRemainTime = kLimitTime - kCurTime;

			if( kRemainTime.GetTotalMinutes() > 0 )
			{
				++it;
				continue;
			}

			vDeleteIndex.push_back(pCostume->GetCostumeIndex());

			m_mUserCostumeMap.erase(it++);
		}
		else
			++it;
	}
}

int ioUserCostume::GetCostumeEquipPos(int iCode)
{
	if( iCode <= 0 )
		return -1;

	return iCode / COSTUME_SUBTYPE_DELIMITER;
}

void ioUserCostume::SetEquipInfo(int& iClassType, int iCostumeCode, int iCostumeIndex, DWORD dwMaleCode, DWORD dwFemaleCode)
{
	if( !m_pUser )
		return;

	int iArray = m_pUser->GetCharArrayByClass(iClassType);
	ioCharacter* pChar = m_pUser->GetCharacter(iArray);

	if( !pChar )
	{
		iClassType = 0;
		return;
	}

	int iSlot = GetCostumeEquipPos(iCostumeCode);
	if( iSlot < 0 )
	{
		iClassType	= 0;
		return;
	}

	int iCurEquipInfo = pChar->GetCostumeIndex(iSlot);
	if( iCurEquipInfo != 0 )
	{
		iClassType	= 0;
		return;
	}

	pChar->ChangeCostumeItem(iSlot, iCostumeIndex, iCostumeCode, dwMaleCode, dwFemaleCode );
}

void ioUserCostume::SendAllCostumeInfo()
{
	if( !m_pUser )
		return;
		
	int iSize = m_mUserCostumeMap.size();

	SP2Packet kPacket( STPK_USER_COSTUME_DATA );
	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	UserCostumeItem::iterator it = m_mUserCostumeMap.begin();

	for( ; it != m_mUserCostumeMap.end(); it++ )
	{
		Costume cCostume = it->second;

		if( cCostume.GetCostumeCode() <= 0 )
			continue;

		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetCostumeIndex()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetCostumeCode()) );
		PACKET_GUARD_VOID( kPacket.Write((BYTE)cCostume.GetPeriodType()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetYearMonthDayValue()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetHourMinute()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetMaleCustom()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetFemaleCustom()) );
		PACKET_GUARD_VOID( kPacket.Write(cCostume.GetWearingClass()) );
	}

	m_pUser->SendMessage(kPacket);
}
