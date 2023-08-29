#pragma once

enum AccessoryType
{
	ACST_NONE		= 0,
	ACST_RING		= 1,
	ACST_NECKLACE	= 2,
	ACST_BRACELET	= 3,
};

enum AccessoryPeriodType
{
	ASPT_TIME		= 0,
	ASPT_MORTMAIN	= 1,
};

class Accessory
{
public:
	Accessory();
	~Accessory();

	void Init();
	void Destroy();

public:
	// ��¥��
	SHORT GetYear()
	{
		return m_iValue1/10000;           // [2009]0715
	}
	SHORT GetMonth()
	{
		return ( m_iValue1/100 ) % 100;   //  2009[07]15
	}
	SHORT GetDay()
	{
		return m_iValue1 % 100;           //  200907[15]
	}
	SHORT GetHour()
	{
		return m_iValue2 / 100;           //  [21]23   ( 21�� 23�� )
	}
	SHORT GetMinute()
	{
		return m_iValue2 % 100;           //  21[23]
	}
	void SetDate( int iYear , int iMonth, int iDay, int iHour, int iMinute )
	{
		m_iValue1 = ( iYear * 10000 ) + ( iMonth * 100 ) + iDay;
		m_iValue2 = ( iHour * 100 ) + iMinute;
	}
	void SetDate( int iYearMonthDay, int iHourMinute)
	{
		m_iValue1 = iYearMonthDay;
		m_iValue2 = iHourMinute;
	}
public:
	void FillMoveData( SP2Packet &rkPacket );
	void ApplyMoveData( SP2Packet &rkPacket );

public:
	void GetAccessoryLimitDate(SYSTEMTIME& sysTime);

	inline int GetAccessoryIndex() { return m_dwIndex; }
	inline void SetAccessoryIndex(DWORD dwIndex) { m_dwIndex = dwIndex; }

	inline int GetAccessoryCode() { return m_iAccessoryCode; }
	inline void SetAccessoryCode(int iCode) { m_iAccessoryCode = iCode; }

	inline int GetPeriodType()	{ return m_iPeriodType; }
	inline void SetPeriodType(int iType) { m_iPeriodType= iType; }

	inline int GetWearingClass() { return m_iWearingClassType; }
	inline void SetWearingClass( int iClassType ) { m_iWearingClassType = iClassType; }

	inline int GetYearMonthDayValue() { return m_iValue1; }
	inline int GetHourMinute() { return m_iValue2; }

	inline int GetAccessoryValue() { return m_iAccessoryValue; }
	inline void SetAccessoryValue( int iValue ) { m_iAccessoryValue = iValue; }

protected:
	DWORD m_dwIndex;
	int m_iAccessoryCode;
	int m_iPeriodType;
	int m_iValue1;     // ������� ��Ÿ�� 20090715(2009�� 7�� 15�� )
	int m_iValue2;     // �ð��� ��Ÿ�� 1232 (12��32��)

	int m_iWearingClassType;
	int m_iAccessoryValue;
};