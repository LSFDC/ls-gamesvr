#pragma once

// ������ Ÿ�԰� ������
enum VariableType 
{ 
	vChar = 100,
	vWChar,
	vTimeStamp,
	vLONG,
	vINT64,
	vSHORT,
	vWrong
};

struct ValueType
{
	VariableType type;			// � Ÿ���� �������ΰ�
	int size;					// �������?
	ValueType()
	{
		type = vWrong;
		size = 0;
	}
};

typedef vector<ValueType> vVALUETYPE;

class cSerialize;

// CQuery�� ���
struct QueryHeader
{
	QueryHeader()
	{
		nMsgType			= -1;
		nQueryType			= -1;
		nFieldLength		= 0;
		nResultLength		= 0;
		nReturnLength		= -1;
		nValueTypeCnt		= -1;
		nSetValue		 = -1;
		nQueryBufferSize	= -1;
		nIndex				= 0;
		nResultType		= 0;
		nQueryId			= 0;
		nDatabaseId			= 0;
	}
	int nMsgType;				// �޼��� Ÿ��,	DB�������� ����ϱ⺸�� ���߿� ���Ӽ����� ����� �޾Ƽ� ó���Ҷ� �ʿ�
	int nQueryType;				// ������ Ÿ�� (insert = 0, delete = 1, select = 2, update = 3)
	int nFieldLength;			// ���� �κ� ������
	int nResultLength;			// ��� �κ� ������
	int nReturnLength;			// �ٽ� �ǵ��� �޾ƾ� �ϴ� �������� ������
	int nValueTypeCnt;			// ������� ����
	int nSetValue;				// MoveNext : �����̺� ��� ����?
	int nQueryBufferSize;       // ������ ������ (���� ������)
	unsigned int nIndex;        // ���� ������ ���� �ε���.
	int nResultType;           // ��� ������ �ൿ.
	int nQueryId;				// �����ϰ��� �ϴ� ������ȣ
	int nDatabaseId;				// ��񱸺�
};

class CQuery
{
protected:
	QueryHeader	m_queryHeader;	// �������
	char *m_pBuffer;			// ��������, ���������Ÿ�Ե�, �ٽ� �����޾ƾ��� ������
public:
	void Clear()
	{
		memset(&m_queryHeader,0,sizeof(QueryHeader));
		if(m_pBuffer != NULL)
		{
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}
	}
};

class CQueryData : public CQuery  
{
public://GET 
	//HEADER
	int GetMsgType()		{ return m_queryHeader.nMsgType; }
	int GetQueryType()		{ return m_queryHeader.nQueryType; }
	int GetFieldSize()		{ return m_queryHeader.nFieldLength; }
	int GetResultSize()		{ return m_queryHeader.nResultLength; }
	int GetValueCnt()		{ return m_queryHeader.nValueTypeCnt; }
	int GetReturnSize()		{ return m_queryHeader.nReturnLength; }
	int GetSetValue()		{ return m_queryHeader.nSetValue; }
	int GetBufferSize()		{ return m_queryHeader.nQueryBufferSize; }
	unsigned int GetIndex()	{ return m_queryHeader.nIndex; }
	int GetResultType()		{ return m_queryHeader.nResultType; }
	int GetQueryID()		{ return m_queryHeader.nQueryId; }
	int GetDatabaseID()		{ return m_queryHeader.nDatabaseId; }

	//DATA
	QueryHeader *GetHeader(){ return &m_queryHeader; }
	char *GetBuffer()		{ return m_pBuffer; }
	
	void GetFields(cSerialize& fieldTypes);
	void GetResults(vVALUETYPE& valueTypes);
	void GetReturns(char* buffer, int &size);

public:
	//SET
	void SetData(
		unsigned int nIndex,
		int nResultType,
		int nMsgType, 
		int nQueryType,
		int nQueryId, 
		cSerialize& fieldTypes,
		vVALUETYPE& valueTypes, 
		int nSetValue = 1);
	void SetBuffer(const char *buffer);

protected:
	//�����ޱ� ���� �����ϴ� ������
	char m_szReturnBuf[MAX_BUFFER];
	int  m_iReturnLength;

public:
	void SetReturnData( const void *pData, int iSize );
	void Copy( CQueryData &queryData );

public:
	CQueryData();
	virtual ~CQueryData();
};
