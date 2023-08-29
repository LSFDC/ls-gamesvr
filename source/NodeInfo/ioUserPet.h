#ifndef __ioUserPet_h__
#define __ioUserPet_h__

class cSerialize;

class ioUserPet : public ioDBDataController
{
public:
	enum
	{
		MAX_SLOT     = 20,
		NONE_EQUIP	 = -1
	};

	typedef struct tagPetSlot
	{
		int m_iIndex;
		int m_iPetCode;
		int m_iPetRank;
		int m_iCurLevel;
		int m_iCurExp;
		int m_iMaxExp; //DB���� ó�� �����ö� ü���ְ� �������� �ٽ� ü���ְ�
		bool m_bEquip;
		bool m_bChange;

		tagPetSlot()
		{
			Init();
		}

		void Init()
		{
			m_iIndex = 0;		//���� db�ε��� ( �� ��ġ == 0, index��ȣ�� �޾ƾ��ϴ� ���� == NEW_INDEX )
			m_iPetCode = 0;
			m_iPetRank = 0;
			m_iCurLevel = 0;
			m_iMaxExp = 0;
			m_iCurExp = 0;
			m_bEquip = false;
			m_bChange = false;
		}
	}PETSLOT;

protected:
	//�� ���� ����
	typedef std::vector< PETSLOT > vPetSlotList;
	vPetSlotList m_vPetSlotList;

	//m_vPetSlotList�� �ε���
	int m_iEquipPetListIdx;

public:
	virtual void Initialize( User *pUser );
	virtual bool DBtoNewIndex( DWORD dwIndex );
	virtual void DBtoData( CQueryResultData *query_data );	
	virtual void SaveData();
	virtual void FillMoveData( SP2Packet &rkPacket );
	virtual void ApplyMoveData( SP2Packet &rkPacket, bool bDummyNode = false  );

public:
	bool GetPetInfo( const int& iPetIndex, PETSLOT& rkPet, int& iListIndx );
	bool GetPetInfo( const int& iPetIndex, int& iListIndx );
	bool GetPetInfo( const int& iPetIndex, PETSLOT& rkPet );

	int  GetPetVecIndex( const int& iPetIndex );
	int  GetPetCode( const int iPetIndex );

	void SetPetInfo( const PETSLOT& rkPet );

	bool EquipPet( const int& iPetIndex, PETSLOT& rkPet );
	bool ClearPetEquip( const int& iPetIndex ); 

	int PetNurture( User* pUser, const int& iPetIndex, const int& iMaterialCode, PETSLOT& rkPetInfo, int &iNeedMaterialCount );

	bool AddData( PETSLOT &rkNewSlot, CQueryData &query_data );
	void AddPet( const int& iNewIndex, const int& iPetCode, const int& iPetRank, const int& iPetLevel );
	bool DeleteData( const int& iPetIndex );

	bool GetEquipPetInfo( PETSLOT &rkNewSlot );
	
	bool CheckPetHavePossible( );
	int CheckEmptyVecIndex();

	void SetPetData( const PETSLOT &rkPetSlot );

public:
	int CheckEquipPetVecIndex();
	void CheckEquipIndexError();

public:
	void TestInput();

public:
	ioUserPet();
	virtual ~ioUserPet();
};

#endif