#include "stdafx.h"
#include "ioUserAccessory.h"
#include "User.h"
#include "AccessoryManager.h"
#include "../ioCriticalError.h"
#include "../MainProcess.h"
#include "../QueryData/QueryResultData.h"
#include "../DataBase/DBClient.h"
#include "../DataBase/LogDBClient.h"
#include "ioCharacter.h"
#include "../EtcHelpFunc.h"

ioUserAccessory::ioUserAccessory()
{
}

ioUserAccessory::~ioUserAccessory()
{
	Destroy();
}

void ioUserAccessory::Init(User* pUser)
{
	m_mUserAccessoryMap.clear();
	m_pUser = pUser;
}

void ioUserAccessory::Destroy()
{
	m_mUserAccessoryMap.clear();
	m_pUser = NULL;
}

void ioUserAccessory::DBtoData( CQueryResultData *query_data, int& iLastIndex )
{
	if( !m_pUser )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ioUserAccessory::DBtoData() User NULL!!"); 
		return;
	}

	LOOP_GUARD();
	while( query_data->IsExist() )
	{	
		Accessory cAccessory;

		DBTIMESTAMP dts;
		int iIndex			= 0;
		DWORD dwAccessoryCode = 0;
		BYTE byPeriodType	= 0;
		int iClassType		= 0;	
		int iAccessoryValue = 0;
		bool bChange		= false;

		PACKET_GUARD_BREAK( query_data->GetValue( iIndex, sizeof(int) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( dwAccessoryCode, sizeof(int) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( byPeriodType, sizeof(BYTE) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( (char*)&dts, sizeof(DBTIMESTAMP) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( iAccessoryValue, sizeof(int) ) );
		PACKET_GUARD_BREAK( query_data->GetValue( iClassType, sizeof(int) ) );				//장착중인 클래스.
		
		if( 0 != iClassType && !m_pUser->IsCharClassType( iClassType ) )
		{
			iClassType	= 0;
			bChange = true;
		}
		else
		{
			//해당 클래스에 장착
			SetEquipInfo(iClassType, dwAccessoryCode, iIndex, iAccessoryValue);
		}

		CTime kLimitTime(Help::GetSafeValueForCTimeConstructor(dts.year,dts.month,dts.day, dts.hour,dts.minute,dts.second));
		
		cAccessory.SetAccessoryCode(dwAccessoryCode);
		cAccessory.SetAccessoryIndex(iIndex);
		cAccessory.SetPeriodType(byPeriodType);
		cAccessory.SetWearingClass(iClassType);
		cAccessory.SetDate(kLimitTime.GetYear(), kLimitTime.GetMonth(), kLimitTime.GetDay(), kLimitTime.GetHour(), kLimitTime.GetMinute());
		cAccessory.SetAccessoryValue(iAccessoryValue);

		m_mUserAccessoryMap.insert(make_pair(iIndex, cAccessory));
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] accessory dbtodata : [user:%d index:%d code:%d value:%d]", m_pUser->GetUserIndex(), iIndex, dwAccessoryCode, iAccessoryValue);

		//들어온 장비에대한 마지막 index를 가지고 있어야되.
		iLastIndex = iIndex;

		if( bChange )
		{
			SYSTEMTIME sys = {0,0,0,0,0,0,0,0};
			g_AccessoryMgr.ConvertCTimeToSystemTime(sys, kLimitTime);
			g_DBClient.OnUpdateAccessoryData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), iIndex, dwAccessoryCode, byPeriodType, sys, iClassType, iAccessoryValue);
			LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] equip accessory update : [user:%d index:%d code:%d value:%d]", m_pUser->GetUserIndex(), iIndex, dwAccessoryCode, iAccessoryValue);
		}
	}
	LOOP_GUARD_CLEAR();
}

void ioUserAccessory::FillMoveData( SP2Packet &rkPacket )
{
	PACKET_GUARD_VOID(rkPacket.Write((int)m_mUserAccessoryMap.size()));
	UserAccessoryItem::iterator it = m_mUserAccessoryMap.begin();

	for( ; it != m_mUserAccessoryMap.end() ; it++ )
	{
		Accessory cAccessory = it->second;
		cAccessory.FillMoveData(rkPacket);
	}
}

void ioUserAccessory::ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode )
{
	int iSize = 0;
	PACKET_GUARD_VOID( rkPacket.Read(iSize) );

	for( int i = 0; i < iSize; i++ )
	{
		Accessory cAccessory;
		cAccessory.ApplyMoveData(rkPacket);

		m_mUserAccessoryMap.insert(make_pair(cAccessory.GetAccessoryIndex(), cAccessory));
	}
}

bool ioUserAccessory::IsEnableAdd()
{
	int iMaxCount = g_CriticalError.GetMaxAccessoryCount();

	if( iMaxCount <= (int)m_mUserAccessoryMap.size() )
		return false;

	return true;
}

void ioUserAccessory::AddAccessoryItem(Accessory& AccessoryInfo)
{
	m_mUserAccessoryMap.insert( make_pair(AccessoryInfo.GetAccessoryIndex(), AccessoryInfo) );
}

bool ioUserAccessory::DeleteAccessoryItem(DWORD& dwIndex)
{
	if( !m_pUser )
		return false;

	UserAccessoryItem::iterator it = m_mUserAccessoryMap.find(dwIndex);

	if( it == m_mUserAccessoryMap.end() )
		return false;	//없는 아이템

	m_mUserAccessoryMap.erase(it);
	
	//DB삭제
	g_DBClient.OnDeleteAccessoryData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), dwIndex);
	return true;	//성공
}

bool ioUserAccessory::IsEmpty()
{
	if( m_mUserAccessoryMap.size() != 0 )
		return false;

	return true;
}

Accessory* ioUserAccessory::GetAccessory(DWORD dwIndex)
{
	UserAccessoryItem::iterator it = m_mUserAccessoryMap.find(dwIndex);
	if( it == m_mUserAccessoryMap.end() )
		return NULL;

	return &(it->second);
}

//bool ioUserAccessory::ChangeAccessory(ioCharacter* pCharacter, DWORD dwClassIndex, int iEquipPos, DWORD dwTargetIndex, DWORD dwAccessoryCode, bool bEquip)
//{
//	if( bEquip )
//	{
//		//캐릭터에 장착
//		pCharacter->ChangeAccessoryItem(iEquipPos, dwTargetIndex, dwAccessoryCode);
//	}
//	else
//	{
//		//캐릭터에 장착 해제
//		pCharacter->ChangeAccessoryItem(iEquipPos, 0, 0);
//	}
//
//	return true;
//}

int ioUserAccessory::ChangeEquipInfo(int iClassArray, DWORD& dwTargetIndex, int iEquipPos, bool bEquip, Accessory* pAccessory)
{
	if( !m_pUser )
		return ACCESSORY_CHANGE_EXECPTION;

	if( iEquipPos < 0 || iEquipPos >= MAX_CHAR_ACCESSORY_SLOT )
		return ACCESSORY_CHANGE_DISABLE_POS;

	if( !pAccessory )
		return ACCESSORY_CHANGE_NO_TARGET;

	if( (pAccessory->GetAccessoryCode()-ACCESSORY_SUBTYPE_DELIMITER)/ACCESSORY_SUBTYPE_DELIMITER != iEquipPos )
		return ACCESSORY_CHANGE_DISABLE_POS;

	if( bEquip && g_AccessoryMgr.IsEquipHero( pAccessory->GetAccessoryCode(), iClassArray ) )
		return ACCESSORY_CHANGE_EXECPTION;

	if( pAccessory->GetPeriodType() == PCPT_TIME )
	{
		CTime kCurTime = CTime::GetCurrentTime();
		CTime kLimitTime( Help::GetSafeValueForCTimeConstructor( pAccessory->GetYear(),
																	 pAccessory->GetMonth(),
																	 pAccessory->GetDay(),
																	 pAccessory->GetHour(),
																	 pAccessory->GetMinute(),
																	 0) );
		CTimeSpan kRemainTime = kLimitTime - kCurTime;
		if( kRemainTime.GetTotalMinutes() <= 0 )
			return ACCESSORY_CHANGE_PERIOD;
	}

	ioCharacter* pCharacter = m_pUser->GetCharacter(iClassArray);
	if( !pCharacter )
		return ACCESSORY_CHANGE_NO_CHARACTER;

	if( pAccessory->GetWearingClass() != 0 )
	{
		int iArray = m_pUser->GetCharArray(pAccessory->GetWearingClass());
		ioCharacter* pChar =  m_pUser->GetCharacter(iArray);
		if( pChar )
		{
			int iArray = pChar->GetAccessorySlot(dwTargetIndex);
			if( iArray >= 0 )
				return ACCESSORY_CHANGE_EQUIP_ITEM;
			pAccessory->SetWearingClass(0);
		}
		else
			pAccessory->SetWearingClass(0);
	}

	int iPrevAccessoryIndex = pCharacter->GetAccessoryIndex(iEquipPos);

	if( bEquip )
	{
		Accessory* pPrevAccessory = GetAccessory(iPrevAccessoryIndex);

		if( pPrevAccessory )
		{
			ReleaseAccessory(pPrevAccessory);
		}

		pCharacter->ChangeAccessoryItem(iEquipPos, dwTargetIndex, pAccessory->GetAccessoryCode(), pAccessory->GetAccessoryValue());
		EquipAccessory(pCharacter->GetClassType(), pAccessory);
	}
	else
	{
		//캐릭터에 장착 해제
		pCharacter->ChangeAccessoryItem(iEquipPos, 0, 0, 0);
		ReleaseAccessory(pAccessory);
	}

	return ACCESSORY_CHANGE_SUCCESS;
}

void ioUserAccessory::ReleaseAccessoryWithAccessoryIndex(DWORD dwIndex)
{
	UserAccessoryItem::iterator it = m_mUserAccessoryMap.find(dwIndex);
	if( it == m_mUserAccessoryMap.end() )
		return;

	Accessory* pAccessory = &(it->second);
	if( !pAccessory )
		return;

	ReleaseAccessory(pAccessory);
}

void ioUserAccessory::ReleaseAccessoryWithClassType(int iClassType)
{
	UserAccessoryItem::iterator it = m_mUserAccessoryMap.begin();
	if( it == m_mUserAccessoryMap.end() )
		return;

	for( ; it != m_mUserAccessoryMap.end(); it++ )
	{
		Accessory* pAccessory = &(it->second);
		if( !pAccessory )
			continue;

		if( pAccessory->GetWearingClass() == iClassType )
		{
			ReleaseAccessory(pAccessory);
		}
	}
}
void ioUserAccessory::EquipAccessory(const int iClassType, Accessory* pAccessory)
{
	if( !m_pUser )
		return;

	if( pAccessory )
	{
		pAccessory->SetWearingClass(iClassType);
		//장착 정보 DBupdate
		SYSTEMTIME sysTime = {0,0,0,0,0,0,0,0};
		pAccessory->GetAccessoryLimitDate(sysTime);
		g_DBClient.OnUpdateAccessoryData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(), pAccessory->GetAccessoryIndex(), pAccessory->GetAccessoryCode(), pAccessory->GetPeriodType(),
			sysTime, pAccessory->GetWearingClass(), pAccessory->GetAccessoryValue());

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] equip accessory update : [user:%d index:%d code:%d value:%d]", m_pUser->GetUserIndex(), pAccessory->GetAccessoryIndex(), pAccessory->GetAccessoryCode(), pAccessory->GetAccessoryValue());
	}
}

void ioUserAccessory::ReleaseAccessory(Accessory* pAccessory)
{
	if( !m_pUser )
		return;

	if( pAccessory )
	{
		pAccessory->SetWearingClass(0);
		SYSTEMTIME sysTime = {0,0,0,0,0,0,0,0};
		pAccessory->GetAccessoryLimitDate(sysTime);
		g_DBClient.OnUpdateAccessoryData(m_pUser->GetUserDBAgentID(), m_pUser->GetAgentThreadID(), m_pUser->GetUserIndex(),  pAccessory->GetAccessoryIndex(), pAccessory->GetAccessoryCode(), pAccessory->GetPeriodType(),
			sysTime, 0, pAccessory->GetAccessoryValue());

		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "[test][accessory] release accessory update : [user:%d index:%d code:%d value:%d]", m_pUser->GetUserIndex(), pAccessory->GetAccessoryIndex(), pAccessory->GetAccessoryCode(), pAccessory->GetAccessoryValue());
	}
}

void ioUserAccessory::DeleteAccessoryPassedDate(IntVec &vDeleteIndex)
{
	CTime kCurTime = CTime::GetCurrentTime();

	UserAccessoryItem::iterator it = m_mUserAccessoryMap.begin();
	if( it == m_mUserAccessoryMap.end() )
		return;

	while( it != m_mUserAccessoryMap.end() )
	{
		Accessory *pAccessory = &(it->second);
		if( pAccessory )
		{
			if( pAccessory->GetPeriodType() == PCPT_MORTMAIN )
			{
				++it;
				continue;
			}

			CTime kLimitTime( Help::GetSafeValueForCTimeConstructor( pAccessory->GetYear(),
																	 pAccessory->GetMonth(),
																	 pAccessory->GetDay(),
																	 pAccessory->GetHour(),
																	 pAccessory->GetMinute(),
																	 0) );

			CTimeSpan kRemainTime = kLimitTime - kCurTime;

			if( kRemainTime.GetTotalMinutes() > 0 )
			{
				++it;
				continue;
			}

			vDeleteIndex.push_back(pAccessory->GetAccessoryIndex());

			m_mUserAccessoryMap.erase(it++);
		}
		else
			++it;
	}
}

int ioUserAccessory::GetAccessoryEquipPos(int iCode)
{
	if( iCode <= 0 )
		return -1;

	return (iCode-ACCESSORY_SUBTYPE_DELIMITER) / ACCESSORY_SUBTYPE_DELIMITER;
}

void ioUserAccessory::SetEquipInfo(int& iClassType, int iAccessoryCode, int iAccessoryIndex, int iAccessoryValue)
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

	int iSlot = GetAccessoryEquipPos(iAccessoryCode);
	if( iSlot < 0 )
	{
		iClassType	= 0;
		return;
	}

	int iCurEquipInfo = pChar->GetAccessoryIndex(iSlot);
	if( iCurEquipInfo != 0 )
	{
		iClassType	= 0;
		return;
	}

	pChar->ChangeAccessoryItem(iSlot, iAccessoryIndex, iAccessoryCode, iAccessoryValue);
}

void ioUserAccessory::SendAllAccessoryInfo()
{
	if( !m_pUser )
		return;
		
	int iSize = m_mUserAccessoryMap.size();

	SP2Packet kPacket( STPK_USER_ACCESSORY_DATA );
	PACKET_GUARD_VOID( kPacket.Write(iSize) );

	UserAccessoryItem::iterator it = m_mUserAccessoryMap.begin();

	for( ; it != m_mUserAccessoryMap.end(); it++ )
	{
		Accessory cAccessory = it->second;

		if( cAccessory.GetAccessoryCode() <= 0 )
			continue;

		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetAccessoryIndex()) );
		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetAccessoryCode()) );
		PACKET_GUARD_VOID( kPacket.Write((BYTE)cAccessory.GetPeriodType()) );
		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetYearMonthDayValue()) );
		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetHourMinute()) );
		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetWearingClass()) );
		PACKET_GUARD_VOID( kPacket.Write(cAccessory.GetAccessoryValue()) );
	}

	m_pUser->SendMessage(kPacket);
}