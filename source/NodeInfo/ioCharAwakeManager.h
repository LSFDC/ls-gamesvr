#ifndef __ioCharAwakeManger_h__
#define __ioCharAwakeManger_h__

#include "../Util/Singleton.h"

class ioCharAwakeManager : public Singleton< ioCharAwakeManager > 
{
protected:
	enum
	{
		AWAKE_NONE      = 0,
		AWAKE_NORMAL	= 1,
		AWAKE_RARE      = 2
	};
	
	struct AwakeMaterialInfo
	{
		int iMaterialType;
		int iMaterialCode;
		int iNeedCount;

		AwakeMaterialInfo()
		{
			iMaterialType = 0;
			iMaterialCode = 0;
			iNeedCount = 0;
		}
	};

	struct AwakeInfo
	{
		int iAwakeType;			// ���� ����
		int iMaterialInfoNum;	// AwakeMaterialInfo ����ü�� ����� ��ȣ
		int iActDay;			// ���� �ð�

		AwakeInfo()
		{
			iAwakeType = 0;
			iMaterialInfoNum = 0;
			iActDay = 0;
		}
	};

protected:
	void Clear();

protected:
	int m_iMaxAwakePeriod;
	int m_iMaxAwakeKind;
	int m_iMaxMaterial;

	std::vector< int > m_vAwakeAddDateInfo; //���� ������ ���� ���� ����
	std::vector< AwakeMaterialInfo > m_vAwakeMaterialInfo; // ������ ���� ��� ���� ���� 
	std::vector< AwakeInfo > m_vAwakeInfo; //���� ����

public:
	static ioCharAwakeManager& GetSingleton();

public:
	void LoadINI();

	bool IsLoadedAwakeDay( int iAwakeDay );	//���� ������ ���� ����.

	void GetMaterialInfo( const int iAwakeType, const int iAddDay, const int iSoldierCode, int &iMaterialCode, int &iMaterialCount );

	AwakeInfo* GetAwakeInfo( const int iAwakeType, const int iAddDay );

	bool CheckRightDate( const int& iAddDay );
	bool CheckMaxAwakePeriod( const int& iAddDay, const int& iCurEndDate );
	
	bool CheckRightAwakeChange( const int iPrevAwake, const int iNextAwake );

public:
	ioCharAwakeManager();
	virtual ~ioCharAwakeManager();
};

#define g_CharAwakeManager ioCharAwakeManager::GetSingleton()

#endif