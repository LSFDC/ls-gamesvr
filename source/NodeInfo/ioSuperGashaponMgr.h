#pragma once
#include "../Util/Singleton.h"
#include "../Util/IORandom.h"

class User;
class ioSuperGashaponMgr : public Singleton< ioSuperGashaponMgr >
{
public:
	struct SuperGashaponElement
	{
		short m_iPresentType;
		int   m_iPresentValue1;
		int   m_iPresentValue2;
		short m_iSuperGashaponMent;
		int	  m_iSuperGashaponPeriod;

		SuperGashaponElement()
		{
			m_iPresentType			= 0;
			m_iPresentValue1		= 0;
			m_iPresentValue2		= 0;
			m_iSuperGashaponMent	= 0;
			m_iSuperGashaponPeriod	= 0;
		}
	};
	typedef std::vector< SuperGashaponElement > vSuperGashaponElement;

protected:

	// ���۰�í�� Package
	struct SuperGashaponPackage
	{
		vSuperGashaponElement m_vPackageElement;
		DWORD m_dwRand;
		DWORD m_dwIndex;

		int m_iLimit;

		short m_iSuperGashaponMent;
		
		SuperGashaponPackage()
		{
			m_vPackageElement.clear();

			m_iLimit				= -1;

			m_dwRand				= 0;
			m_dwIndex				= 0;
			m_iSuperGashaponMent	= 0;
		}
	};

	class SuperGashaponPackageSort : public std::binary_function< const SuperGashaponPackageSort&, const SuperGashaponPackageSort&, bool >
	{
	public:
		bool operator()( const SuperGashaponPackage &lhs , const SuperGashaponPackage &rhs ) const
		{
			if( lhs.m_dwRand < rhs.m_dwRand )
				return true;
			return false;
		}
	};

	typedef std::vector< SuperGashaponPackage > vSuperGashaponPackage;

protected:
	struct SuperGashaponPackageInfo
	{
		vSuperGashaponPackage	m_vSuperGashaponPackageList;	 //�⺻ ��Ű��
		vSuperGashaponPackage	m_vSuperGashaponSubPackageList;  //��ü ��Ű��

		DWORD					m_dwSuperGashaponPackageSeed;
		DWORD					m_dwSuperGashaponSubPackageSeed;
		IORandom				m_SuperGashaponPackageRandom;
		IORandom				m_SuperGashaponSubPackageRandom;

		DWORD					m_dwEtcItemType;		
		ioHashString			m_szGashaponSendID;

		int						m_iSuperGashaponPeriod;

		DWORD					m_dwLimit;

		SuperGashaponPackageInfo()
		{
			m_vSuperGashaponPackageList.clear();
			m_vSuperGashaponSubPackageList.clear();

			m_dwSuperGashaponPackageSeed	= 0;
			m_dwSuperGashaponSubPackageSeed	= 0;
			m_dwEtcItemType					= 0;
			m_iSuperGashaponPeriod			= 0;
			m_dwLimit						= 0;
		}
	};

	typedef std::vector< SuperGashaponPackageInfo > vSuperGashaponPackageInfo;
	vSuperGashaponPackageInfo m_vSuperGashaponPackageInfoList;

protected:
	char m_szBuffer[MAX_PATH];
	char m_szKey[MAX_PATH];

	char m_szUsedCountININame[MAX_PATH];

public:
	void LoadINI();
	void CheckNeedReload();	

protected:
	void LoadSuperGashaponPackage( ioINILoader &rkLoader );

	void LoadPackage( ioINILoader &rkLoader, SuperGashaponPackageInfo& rkInfo, bool bSubPackage = false );
	void LoadPackageElement( ioINILoader &rkLoader, SuperGashaponPackage& rkPresentData, int iPackageIndex, bool bSubPackage = false );

protected:
	SuperGashaponPackageInfo* GetSuperGashaponPackageInfo( DWORD dwEtcItemType );

public:
	DWORD GetSuperGashaponPackageLimit( DWORD dwEtcItemType );

public:
	void SendSuperGashaponSelectPackage( User *pSendUser, DWORD dwEtcItemType, const SuperGashaponPackage &rkPackage, int iPeriod, const ioHashString& szGashaponSendID );

	bool SendSuperGashaponRandPackage( User *pSendUser, DWORD dwEtcItemType, DWORD& dwPackageIndex );
	bool SendSuperGashaponRandSubPackage( User *pSendUser, DWORD dwEtcItemType, DWORD& dwSubPackageIndex );

	bool SendSuperGashaponAllPackage( User *pSendUser, DWORD dwEtcItemType );
	bool SendSuperGashaponPackage( User *pSendUser, DWORD dwEtcItemType, DWORD dwPackageIndex );

public:
	bool FindSuperGashaponPackageRandom( User *pSendUser, DWORD dwEtcItemType, DWORD& rdwPackageIndex, DWORD& rdwPackageLimitMax );
	bool SendSuperGashponLimitCheck( User *pSendUser, DWORD dwEtcItemType, int iUseType );

public:
	bool IsLimitGashapon( DWORD dwEtcItemType );

public:
	static ioSuperGashaponMgr& GetSingleton();

public:
	ioSuperGashaponMgr();
	~ioSuperGashaponMgr();
};

#define g_SuperGashaponMgr ioSuperGashaponMgr::GetSingleton()