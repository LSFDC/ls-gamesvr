// QueryData.cpp: implementation of the CQueryData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../util/cSerialize.h"
#include "QueryData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQueryData::CQueryData()
{
	memset( m_szReturnBuf, 0, sizeof( m_szReturnBuf ) );
	m_iReturnLength = 0;

	m_pBuffer = NULL;
	CQuery::Clear();
}

CQueryData::~CQueryData()
{
	CQuery::Clear();
}

void CQueryData::GetFields(cSerialize& fieldTypes)
{
	if(GetFieldSize() == 0) 
		return;

	if(m_pBuffer)
	{
		uint32 index = sizeof(int);

		fieldTypes.Reset();
		fieldTypes.SetBuffer( (uint8*)(m_pBuffer + index), GetFieldSize() );
	}
}

void CQueryData::GetResults(vVALUETYPE& valueTypes)
{
	valueTypes.clear();

	if(GetResultSize() == 0) 
		return;

	if(m_pBuffer)
	{
		uint32 index = sizeof(int) + GetFieldSize();
		for(int32 n = 0 ; n < GetResultSize() ; n += sizeof(ValueType) )
		{
			ValueType* type = (ValueType*)(m_pBuffer + index + n);
			valueTypes.push_back( *type );
		}
	}
}

void CQueryData::GetReturns(char* buffer, int &size)
{
	if(m_pBuffer)
	{
		uint32 index = sizeof(int) + GetFieldSize() + GetResultSize();
		size = GetReturnSize();
		memcpy( buffer, m_pBuffer + index, size );
	}
}

void CQueryData::SetReturnData( const void *pData, int iSize )
{
	memcpy( &m_szReturnBuf[m_iReturnLength], pData, iSize );
	m_iReturnLength += iSize;

	if( m_iReturnLength >= MAX_BUFFER )
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "CQueryData::SetReturnData Return Size : %d", m_iReturnLength );
}

void CQueryData::SetBuffer(const char *buffer)
{
	memcpy(&m_queryHeader,buffer,sizeof(QueryHeader));
	buffer+= sizeof(QueryHeader);
	m_pBuffer = new char[m_queryHeader.nQueryBufferSize];
	memcpy(m_pBuffer,buffer,m_queryHeader.nQueryBufferSize);		
}

void CQueryData::SetData(
						 unsigned int nIndex,
						 int nResultType,
						 int nMsgType, 
						 int nQueryType,
						 int nQueryId,
						 cSerialize& fieldTypes,
						 vVALUETYPE& valueTypes, 
						 int nSetValue)
{
	CQuery::Clear();
	//���
	m_queryHeader.nIndex            = nIndex;
	m_queryHeader.nResultType		= nResultType;
	m_queryHeader.nMsgType			= nMsgType;
	m_queryHeader.nQueryType		= nQueryType;
	m_queryHeader.nFieldLength		= fieldTypes.GetLength();
	m_queryHeader.nResultLength		= sizeof(ValueType) * valueTypes.size();
	m_queryHeader.nValueTypeCnt		= 0;
	m_queryHeader.nReturnLength		= m_iReturnLength;
	m_queryHeader.nSetValue         = nSetValue;
	m_queryHeader.nQueryId			= nQueryId;
	m_queryHeader.nDatabaseId		= 1;
	m_queryHeader.nQueryBufferSize  = sizeof(nQueryId) + m_queryHeader.nFieldLength + m_queryHeader.nResultLength + m_queryHeader.nReturnLength;	

	//���� �޸� �Ҵ�.
	if(m_queryHeader.nQueryBufferSize != 0)
	{
		m_pBuffer = new char[m_queryHeader.nQueryBufferSize];
		//������.
		int nSize = 0;                     //������ ��ġ
		int nlen = 0;
		if(0 != nQueryId)
		{
			nlen = sizeof(nQueryId);        //ī���� ������ ������
			memcpy(&m_pBuffer[nSize], &nQueryId, nlen);					 //���� ����
		}
		if(fieldTypes.GetLength() > 0)
		{
			nSize += nlen;                      
			nlen = m_queryHeader.nFieldLength;
			memcpy(&m_pBuffer[nSize], fieldTypes.GetBuffer(), nlen);      // �ʵ� Ÿ��
		}
		if(valueTypes.size() > 0)
		{
			nSize += nlen;                      
			nlen = m_queryHeader.nResultLength;
			memcpy(&m_pBuffer[nSize], &valueTypes[0], nlen);      // ��� Ÿ��
		}
		if(m_iReturnLength > 0)
		{
			nSize += nlen;
			memcpy(&m_pBuffer[nSize], m_szReturnBuf, m_iReturnLength);   //�������� ������.
		}
	}
}

void CQueryData::Copy( CQueryData &queryData )
{
	if(m_pBuffer)
	{
		delete []m_pBuffer;
	}

	m_pBuffer = new char[queryData.GetBufferSize()];
	if(m_pBuffer!=NULL)
	{
		m_queryHeader = queryData.m_queryHeader;
		memcpy( m_pBuffer, queryData.m_pBuffer, m_queryHeader.nQueryBufferSize );
	}
}
