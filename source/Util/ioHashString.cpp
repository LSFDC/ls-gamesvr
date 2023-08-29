#include "stdafx.h"
#include "ioHashString.h"

ioHashString::ioHashString()
{
	m_iCapacity = DEFAULT_CAPACITY;
	m_pString = new char[ m_iCapacity ];

	Clear();

	m_dwHashCode = CalcHashCode( m_pString );
}

ioHashString::ioHashString( const char* str ) : m_pString(NULL)
{
	m_iLength = strlen( str );

	int iCapacity = max( m_iLength + 1, DEFAULT_CAPACITY );
	ReAllocCapacity( iCapacity );

	strcpy_s( m_pString, iCapacity, str );
	m_dwHashCode = CalcHashCode( str );
}

ioHashString::ioHashString( const ioHashString &rhs ) : m_pString(NULL)
{
	m_iLength = rhs.Length();

	ReAllocCapacity( rhs.m_iCapacity );

	strcpy_s( m_pString, rhs.m_iCapacity, rhs.m_pString );
	m_dwHashCode = rhs.m_dwHashCode;
}

ioHashString::~ioHashString()
{
	if( m_pString )
	{
		delete[] m_pString;
		m_pString = NULL;
	}
}

bool ioHashString::IsEmpty() const
{
	return ( m_iLength == 0 );
}

int ioHashString::Length() const
{
	return m_iLength;
}

void ioHashString::Clear()
{
	memset( m_pString, 0, m_iCapacity );
	m_iLength = 0;
}

const char* ioHashString::c_str() const
{
	return m_pString;
}

void ioHashString::ReAllocCapacity( int iNewCapacity )
{
	if( m_pString )
	{
		delete[] m_pString;
		m_pString = NULL;
	}

	m_iCapacity = iNewCapacity;
	m_pString = new char[iNewCapacity];
}

DWORD ioHashString::CalcHashCode( const char *str )
{
	DWORD dwLen, dwCh, dwResult;

	dwLen    = strlen( str );
	dwResult = HASH_CONST;

	for( DWORD i=0; i<dwLen ; i++ )
	{
		dwCh     = (DWORD)str[i];
		dwResult = ((dwResult<< 5) + dwResult) + dwCh; // hash * 33 + ch
	}

	return dwResult;
}

DWORD ioHashString::GetHashCode() const
{
	return m_dwHashCode;
}

void ioHashString::MakeLower()
{
	if( m_pString )
	{
		_strlwr_s( m_pString, m_iCapacity );
		m_dwHashCode = CalcHashCode( m_pString );
	}
}

void ioHashString::MakeUpper()
{
	if( m_pString )
	{
		_strupr_s( m_pString, m_iCapacity );
		m_dwHashCode = CalcHashCode( m_pString );
	}
}

bool ioHashString::operator<( const ioHashString &rhs ) const
{
	if( m_dwHashCode != rhs.m_dwHashCode )
		return ( m_dwHashCode < rhs.m_dwHashCode );

	return ( strcmp( m_pString, rhs.m_pString ) < 0 );
}

char ioHashString::At( int i ) const
{
	if( m_iLength < i || !m_pString )
	{
		assert( false );
		return 0;
	}

	return m_pString[i];
}

ioHashString& ioHashString::operator=( const ioHashString &rhs )
{
	m_iLength = rhs.Length();
	if( m_iCapacity < m_iLength + 1 )
	{
		ReAllocCapacity( m_iLength + 1 );
	}

	strcpy_s(m_pString, m_iCapacity, rhs.m_pString );
	m_dwHashCode = rhs.m_dwHashCode;

	return *this;
}

ioHashString& ioHashString::operator=( const char *szText )
{
	m_iLength = strlen( szText );
	if( m_iCapacity < m_iLength + 1 )
	{
		ReAllocCapacity( m_iLength + 1 );
	}

	strcpy_s( m_pString, m_iCapacity, szText );
	m_dwHashCode = CalcHashCode( szText );

	return *this;
}

bool ioHashString::operator==( const ioHashString &rhs ) const
{
	if( m_dwHashCode != rhs.m_dwHashCode )
		return false;

	if( strcmp( m_pString, rhs.m_pString ) != 0 )
		return false;

	return true;
}

bool ioHashString::operator==( const char *szText ) const
{
	if( strcmp( m_pString, szText ) != 0 )
		return false;

	return true;
}

bool ioHashString::operator!=( const ioHashString &rhs ) const
{
	if( *this == rhs )
		return false;

	return true;
}

bool ioHashString::operator!=( const char *szText ) const
{
	if( *this == szText )
		return false;

	return true;
}
