
#ifndef _ioHashString_h_
#define _ioHashString_h_

#define HASH_CONST	5381

#ifdef THAILAND_LONG_ID
#define DEFAULT_CAPACITY	44 // SetReturnData() size�� ����ϹǷ� ID_NUM_PLUS_ONE ���� ���� ������
#else
#define DEFAULT_CAPACITY	32 // SetReturnData() size�� ����ϹǷ� ID_NUM_PLUS_ONE ���� ���� ������
#endif

// Original version is www.gamza.net

class ioHashString
{
private:
	char	*m_pString;
	DWORD	m_dwHashCode;
	int		m_iLength;
	int		m_iCapacity;

private:
	void ReAllocCapacity( int iNewCapacity );

public:
	void Clear();

	bool IsEmpty() const;
	int  Length() const;
	char At( int i ) const;

	DWORD GetHashCode() const;
	const char* c_str() const;
	
	void MakeLower();
	void MakeUpper();

public:
	static DWORD CalcHashCode( const char *str );

public:
	bool operator < ( const ioHashString &rhs )	const;

	ioHashString& operator=( const ioHashString &rhs );
	ioHashString& operator=( const char *szText );

	bool operator==( const ioHashString &rhs ) const;
	bool operator==( const char *szText ) const;

	bool operator!=( const ioHashString &rhs ) const;
	bool operator!=( const char *szText ) const;


	struct KeyHasher
	{
		std::size_t operator()(const ioHashString& k) const
		{
			return k.GetHashCode();

		}
	};

public:
	ioHashString();
	ioHashString( const char *str );
	ioHashString( const ioHashString &rhs );
	~ioHashString();
};

typedef std::vector< ioHashString > ioHashStringVec;

#endif
