

#include "stdafx.h"

#include "ioSetItemInfo.h"
#include <strsafe.h>

ioSetItemInfo::ioSetItemInfo()
{
	m_dwSetCode = 0;
	m_dwRequireRightCode = -1;
	m_ePackageType = PT_NORMAL;

	m_vSetItemCodeList.reserve( MAX_EQUIP_SLOT );
	m_vNeedLevelInfoList.reserve( 10 );
}

ioSetItemInfo::~ioSetItemInfo()
{
	m_vSetItemCodeList.clear();
	m_vNeedLevelInfoList.clear();
}

void ioSetItemInfo::LoadInfo( ioINILoader &rkLoader )
{
	char szBuf[MAX_PATH];
	rkLoader.LoadString( "name", "", szBuf, MAX_PATH * 2 );
	m_SetName = szBuf;

	m_dwSetCode = rkLoader.LoadInt( "set_code", 0 );
	m_dwRequireRightCode = rkLoader.LoadInt( "need_right_code", -1 );
	m_ePackageType       = (PackageType)rkLoader.LoadInt( "package_type", PT_NORMAL );

	enum { MAX_NEED_LEVEL = 100, };
	for ( int i = 0; i < MAX_NEED_LEVEL; i++ )
	{
		NeedLevelInfo kInfo;
		char szKeyName[MAX_PATH]="";
		StringCbPrintf( szKeyName, sizeof(szKeyName), "need_level_type%d", i+1 );
		kInfo.m_eNeedLevelType = (NeedLevelType) rkLoader.LoadInt( szKeyName, -1 );
		if( kInfo.m_eNeedLevelType == NLT_NONE )
			break;
		ZeroMemory( szKeyName, sizeof( szKeyName ) );
		StringCbPrintf( szKeyName, sizeof(szKeyName), "need_level%d", i+1 );
		kInfo.m_iNeedLevel = rkLoader.LoadInt( szKeyName, -1);
		if( kInfo.m_iNeedLevel == -1 )
			break;
		m_vNeedLevelInfoList.push_back( kInfo );
	}
}

int ioSetItemInfo::GetSetItemCnt() const
{
	return m_vSetItemCodeList.size();
}

DWORD ioSetItemInfo::GetSetItemCode( int iIndex ) const
{
	if( COMPARE( iIndex, 0, GetSetItemCnt() ) )
		return m_vSetItemCodeList[iIndex];

	return 0;
}

int ioSetItemInfo::GetNeedLevelInfoListCnt() const
{
	return m_vNeedLevelInfoList.size();	
}

ioSetItemInfo::NeedLevelType ioSetItemInfo::GetNeedLevelType( int iIndex ) const
{
	if( COMPARE( iIndex, 0, GetNeedLevelInfoListCnt() ) )
		return m_vNeedLevelInfoList[iIndex].m_eNeedLevelType;

	return NLT_NONE;
}

int ioSetItemInfo::GetNeedLevel( int iIndex ) const
{
	if( COMPARE( iIndex, 0, GetNeedLevelInfoListCnt() ) )
		return m_vNeedLevelInfoList[iIndex].m_iNeedLevel;

	return -1;
}