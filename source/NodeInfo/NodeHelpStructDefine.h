#include "..\Network\SP2Packet.h"


#ifndef _NodeHelpStructDefine_h_
#define _NodeHelpStructDefine_h_

extern CLog LOG;

enum EntryType 
{
	ET_TEMPORARY      =  1, 
	ET_FORMALITY      =  5, 
	ET_TERMINATION    = -1,  // ���Ӽ��������� �����ϴ� Ÿ��, DB���� �������� ����.
	ET_FORMALITY_CASH =  10, // ���İ��� �����μ� ĳ�� ������ ������ ����
};

enum
{
	USER_TYPE_NORMAL       = 0,    // �Ϲ� ����
	USER_TYPE_BROADCAST_AFRICA = 1,    // 2009.02.19 ������ī ��� �̺�Ʈ ���� ����
	USER_TYPE_LIMIT_IGNORE = 2,    // �뺴�� ġ���� �������� ���� �����ο� Ÿ��
	USER_TYPE_BROADCAST_MBC= 3,    // 2009.03.04 MBC ���� ��� �̺�Ʈ ���� ����
};

enum
{
	HERO_SEASON_RANK_1 = 0,    //������ ���� ��ŷ �ֱ� ù ����
	HERO_SEASON_RANK_2,		   //������ ���� ��ŷ �ֱ� �ι�° ����
	HERO_SEASON_RANK_3,		   //������ ���� ��ŷ �ֱ� ����° ����
	HERO_SEASON_RANK_4,		   //������ ���� ��ŷ �ֱ� �׹�° ����
	HERO_SEASON_RANK_5,		   //������ ���� ��ŷ �ֱ� �ټ���° ����
	HERO_SEASON_RANK_6,		   //������ ���� ��ŷ �ֱ� ������° ����
	HERO_SEASON_RANK_MAX,
};

struct RecordData
{
	int m_iWin;
	int m_iLose;
	int m_iKill;
	int m_iDeath;
	int m_iContinuousKill;
	RecordData()
	{
		Init();
	}
	void Init()
	{
		m_iWin = m_iLose = 0;
		m_iKill = m_iDeath = 0;
		m_iContinuousKill = 0;
	}
};

struct USERDATA          //2007.05.29 LJH
{
	DWORD	m_user_idx;						//���� ����Ű
	ioHashString	m_private_id;			//�α��� ���̵�
	ioHashString    m_public_id;            //���� ���̵�
	int		m_cash;							//���� ĳ��
	__int64	m_money;						//���� �Ӵ�  EX.������ �ڵ� ����.
	int     m_connect_count;                //���� Ƚ��.
	CTime   m_connect_time;                 //�ֱ� ���� �ð� - �������� ���¿��� �������� ����!!!!!!!!
	int     m_user_state;                   //�Ʒ��ϱ� ����.
	int     m_grade_level;                  //���
	int     m_grade_exp;                    //��� ���� ����ġ
	int		m_fishing_level;				//���� ����
	int		m_fishing_exp;					//���� ���� ����ġ
	int     m_user_event_type;              //Ư���� ���� �̺�Ʈ Ÿ��
	int     m_user_rank;                    //��ü ��ŷ    
	EntryType m_eEntryType;                 //�ӽð���, ���İ���(�Ǹ�Ȯ��), �ӽð��� ���� ( UserMemberDB.joinType , -1���� DB�� �������� ����-���Ӽ���������-)
	int     m_camp_position;                //�Ҽ� ���� : (0:���Ҽ�)(1:���ձ�)(2:������)
	int     m_ladder_point;                 //���� ����Ʈ
	int     m_accumulation_ladder_point;    //���� ���� ����Ʈ
	int     m_camp_rank;                    //���� ��ŷ
	ChannelingType m_eChannelingType;       //ä�θ� : �Ϲ�, mgame, ��
	ioHashString   m_szChannelingUserID;    //ä�θ����� ID
	ioHashString   m_szChannelingUserNo;    //ä�θ����� �����ĺ�no : naver���� ���
	int            m_iChannelingCash;       //ä�θ��翡 �����ϴ� Cash�� �Ӵ�
	BlockType      m_eBlockType;            //����Ÿ��
	CTime          m_kBlockTime;            //���ܳ�¥
	int            m_refill_data;           //���ʹޱ���� ���� ��
	bool           m_bSavePossible;         //ó�� �α��ν� ��� �뺴 �������� ������ true�� �� �ڷδ� ���� �̵��ص� �׻� true��.
	int            m_purchased_cash;        //������ ������ �ְ� ������ ĳ�� m_cash�� �Ϻ� �ݾ�
	int            m_iExcavationLevel;      //�߱�����
	int            m_iExcavationExp;        //�߱� ���� ����ġ
	int            m_iAccrueHeroExpert;     //������ ���� ����ġ
	int		       m_iHeroExpert;           //������ ����ġ
	CTime		   m_login_time;			//�α��� �� �ð�.
	
	//hr �����߰�
	
	int            m_iEUGender;
	int            m_iEUAge;
	__int64		   m_iNexonSN;
	DWORD		   m_dwEUContryType;
	//ioHashString   m_dwEUContryType;


	//hr ��ƾ�߰�
	ioHashString   m_Country;
	ioHashString   m_Gender;
	ioHashString   m_Cafe;
	ioHashString   m_Age;
	ioHashString   m_LatinPrivateIP;
	ioHashString   m_NexonEUID;
	ioHashString   m_LatinConnTime;


	int			   m_iFirstWinCount;
	int			   m_iFistLoseCount;	
	int			   m_iWinCount;
	int			   m_iLoseCount;

	__int64		   m_iFirstPeso;		//��ƾ : �α׾ƿ� �� ��� - ù �α�� ���
	int			   m_iGiveUp;			//�������� Ƚ��
	int			   m_iFirstExp;			//�α�� ����ġ
	int			   m_iClientLogoutType;	//���� ���� ���� �� ����
	//

	//hr �Ϲ��߰�
	DWORD		   m_dwUSMemberIndex;
	ioHashString   m_USMemberID;
	DWORD		   m_USMemberType;

	USERDATA()
	{
		Initialize();
	}
	
	void Initialize()
	{
		m_user_idx		  = 0;				
		m_private_id.Clear();
		m_public_id.Clear();
		m_cash			  = 0;
		m_money			  = 0;				
		m_connect_count	  = 0;           
		m_user_state      = 0;
		m_grade_level     = 0;
		m_grade_exp		  = 0;      
		m_fishing_level	  = 0;
		m_fishing_exp	  = 0;
		m_user_event_type = USER_TYPE_NORMAL;
		m_user_rank       = 0;
		m_eEntryType      = ET_TEMPORARY;
		m_camp_position   = 0;
		m_ladder_point    = 0;
		m_accumulation_ladder_point = 0;
		m_camp_rank       = 0;
		m_eChannelingType = CNT_NONE;
		m_szChannelingUserID.Clear();
		m_szChannelingUserNo.Clear();
		m_iChannelingCash = 0;
		m_eBlockType      = BKT_NONE;
		m_refill_data     = 0;
		m_bSavePossible   = false;
		m_purchased_cash  = 0;
		m_iExcavationLevel= 0;
		m_iExcavationExp  = 0;
		m_iAccrueHeroExpert = 0;
		m_iHeroExpert		= 0;
		m_iFirstWinCount	= 0;
		m_iFistLoseCount	= 0;
		m_iFirstPeso		= 0;

		//hr eu & latin
		m_iEUGender			= 0;
	    m_iEUAge			= 0;
	    m_iNexonSN			= 0;
		m_dwEUContryType		= 0;

		m_Country.Clear();
		m_Gender.Clear();
		m_Cafe.Clear();
		m_Age.Clear();
		m_LatinPrivateIP.Clear();
		m_NexonEUID.Clear();
		m_LatinConnTime.Clear();

		m_iFirstWinCount	= 0;
		m_iFistLoseCount	= 0;
		m_iWinCount			= 0;
		m_iLoseCount		= 0;
		m_iFirstPeso		= 0;	//��ƾ : �α׾ƿ� �� ��� - ù �α�� ���
		m_iGiveUp			= 0;	//�������� Ƚ��
		m_iFirstExp			= 0;	//�α�� ����ġ
		m_iClientLogoutType	= 0;	//���� ���� ���� �� ����

		m_dwUSMemberIndex	= 0;
		m_USMemberID.Clear();
		m_USMemberType = 0;

	}

	bool IsChange()
	{		
		if( m_user_idx == 0 )
			return false;
		return m_bSavePossible;
	}
};

struct UserRelativeGradeData
{
	int  m_init_code;     //����� ��� ����ġ ���� �ڵ�
	bool m_enable_reward; //����� ��� ������ �ִ°�?
	int  m_iBackupExp;

	UserRelativeGradeData()
	{
		Initialize();
	}

	void Initialize()
	{
		m_init_code = 0;
		m_enable_reward = 0;
		m_iBackupExp = 0;
	}

	void FillData( SP2Packet &rkPacket )
	{
		rkPacket << m_init_code;
		rkPacket << m_enable_reward;
		rkPacket << m_iBackupExp;
	}

	void ApplyData( SP2Packet &rkPacket )
	{
		rkPacket >> m_init_code;
		rkPacket >> m_enable_reward;
		rkPacket >> m_iBackupExp;
	}
};

struct UserHeroData
{
	int            m_iHeroTitle;            //������ Īȣ
	int            m_iHeroTodayRank;        //������ ���� ��ŷ
	int            m_iHeroSeasonRank[HERO_SEASON_RANK_MAX];       //������ ���� ��ŷ

	UserHeroData()
	{
		Initialize();
	}

	void Initialize()
	{
		m_iHeroTitle      = 0;
		m_iHeroTodayRank  = 0;
		for (int i = 0; i < HERO_SEASON_RANK_MAX ; i++)
			m_iHeroSeasonRank[i] = 0;
	}
};

enum
{
	MAX_DISPLAY_CNT = 10,
};
struct UserHeadquartersOption
{
	// ���� �ɼ�
	short m_sLock;

	// ���� �ɼ�
	DWORD m_dwCharacterIndex[MAX_DISPLAY_CNT];
	int   m_iCharacterXPos[MAX_DISPLAY_CNT];
	int   m_iCharacterZPos[MAX_DISPLAY_CNT];

	bool  m_bChangeOption;

	UserHeadquartersOption()
	{
		Initialize();		
	}

	void Initialize()
	{
		m_sLock = 0;
		m_bChangeOption = false;

		for(int i = 0;i < MAX_DISPLAY_CNT;i++)
		{
			m_dwCharacterIndex[i] = 0;
			m_iCharacterXPos[i] = m_iCharacterZPos[i] = 0;
		}
	}

	void FillData( SP2Packet &rkPacket )
	{
		rkPacket << m_sLock;
		rkPacket << m_bChangeOption;

		for (int i = 0; i < MAX_DISPLAY_CNT ; i++)
		{
			rkPacket << m_dwCharacterIndex[i] << m_iCharacterXPos[i] << m_iCharacterZPos[i]; 
		}
	}

	void ApplyData( SP2Packet &rkPacket )
	{
		rkPacket >> m_sLock;
		rkPacket >> m_bChangeOption;

		for (int i = 0; i < MAX_DISPLAY_CNT ; i++)
		{
			rkPacket >> m_dwCharacterIndex[i] >> m_iCharacterXPos[i] >> m_iCharacterZPos[i]; 
		}
	}
};

struct ITEM_DATA          //2007.05.29 LJH
{ 
	int		m_item_code;             //������ ����
	int  	m_item_reinforce;	     //��ȭ ��
	DWORD   m_item_male_custom;      //���� Ŀ����
	DWORD   m_item_female_custom;    //���� Ŀ����
	
	ITEM_DATA()
	{
		Initialize();
	}

	void Initialize()
	{
		m_item_code = 0;
		m_item_reinforce = 0;
		m_item_male_custom = m_item_female_custom = 0;
	}	

	ITEM_DATA& operator = ( const ITEM_DATA &rhs )
	{
		m_item_code			= rhs.m_item_code;
		m_item_reinforce	= rhs.m_item_reinforce;
		m_item_male_custom	= rhs.m_item_male_custom;
		m_item_female_custom= rhs.m_item_female_custom;
		return *this;
	}
};
typedef std::vector< ITEM_DATA > ITEM_DATAVec;

struct CharCostume
{
	int m_iCostumeIndex;
	int m_iCostumeCode;
	int m_iMaleCustomCode;
	int m_iFemalCustomCode;
	CharCostume()
	{
		m_iCostumeIndex	= 0;
		m_iCostumeCode	= 0;
		m_iMaleCustomCode = 0;
		m_iFemalCustomCode = 0;
	}
};

struct CharAccessory
{
	int m_iAccessoryIndex;
	int m_iAccessoryCode;
	int m_iAccessoryValue;

	CharAccessory()
	{
		m_iAccessoryIndex	= 0;
		m_iAccessoryCode	= 0;
		m_iAccessoryValue	= 0;
	}
};

struct ITEMSLOT          //2007.08.25 LJH 
{
	int		m_item_type;     //���� Ư�� 1111 22 333 ( ��Ʈ, ����(����), ġ��Ÿ�� ) (UserItemDB : itemX_type)
	int  	m_item_code;	 //������ ġ�� Ÿ��      ( ġ�� ���� ��ȣ 0 ~ 9999) (UserItemDB: itemX_code �� õ�ڸ�����)
	int     m_item_equip;    //������ ���� ����      ( 0:������, 1:���� ) (UserItemDB: itemX_code  �� ���ڸ�)           
                             //ex) UserItemDb item1_code 10005 ( 5�� ġ�� Ÿ���� ������ )
	ITEMSLOT()
	{
		Initialize();
	}

	void Initialize()
	{
		m_item_type		= 0;
		m_item_code     = 0;
		m_item_equip    = 0;
	}
};
typedef std::vector< ITEMSLOT > ITEMSLOTVec;

// �Ⱓ�� & ����
enum  CharPeriodType
{
	CPT_TIME     = 0,
	CPT_MORTMAIN = 1,
	CPT_DATE	= 2, // ��ƾ�� �Ⱓ�� �뺴�� ���� �߰�
};

// ��ǥ �뺴 ����
enum
{
	CLT_GENERAL= 0,
	CLT_LEADER = 1,
};

// �뿩 �뺴
enum
{
	CRT_GENERAL= 0,
	CRT_RENTAL = 1,
};

// ���� 
enum
{
	AWAKE_NONE      = 0,
	AWAKE_NORMAL	= 1,
	AWAKE_RARE      = 2
};

enum
{
	DEFAULT_YEAR    = 2010,			// 2010���� DB�� �������� �ʴ´�. �� DateData�� �⵵�� 0�̸� 2010�̶� ���̴�. 1�̸� 2011��
	DATE_YEAR_VALUE = 100000000,    // ����� ������.
	DATE_MONTH_VALUE= 1000000,      // ������ ������.
	DATE_DAY_VALUE =  10000,        // �ϱ��� ������.
	DATE_HOUR_VALUE = 100,          // �ñ��� ������.
};

struct CHARACTER                        //2007.05.29 LJH 
{
	// ���� �߰��ÿ� FillData(), ApplayData(), Init() �Լ��� �����ؾ���.
	int m_class_type;       //Ŭ���� Ÿ�� : ��Ʈ ������ Ÿ��
	int	m_kindred;          //���� 1(�޸�)2(����)3(�����)
	int	m_sex;              //���� 1(����)2(����)
	int	m_beard;            //���� 1(���������) 2,3,4(���� ����)
	int	m_face;             //�� 1(�����������⺻)
	int	m_hair;		        //�Ӹ� 1(�����������⺻)
	int	m_skin_color;       //�Ǻλ� 1,2,3,4
	int	m_hair_color;       //�Ӹ��� 1,2,3
	int	m_accessories;      //��ű� 1
	int m_underwear;        //�ӿ�   1,2,.....

	int m_extra_item[MAX_CHAR_DBITEM_SLOT]; // ������ ������ ����

	int m_iSlotIndex;       //�뺴 ���Կ����� ��ġ
	int m_iLimitSecond;     //���� �Ⱓ

    short m_sLeaderType;          // ��ǥ �뺴
	short m_sRentalType;          // �뿩 ����
	DWORD m_dwRentalMinute;       // �뿩�� �ð�
	CharPeriodType m_ePeriodType; // �����ѿ뺴, �Ⱓ���뺴

	bool m_bActive;         //��� ������ �뺴.
	byte m_chExerciseStyle; //ü�� ��Ÿ�� 

	BYTE m_chAwakeType;		//���� ����, ���� ���� ( 0:NONE, 1:NORMAL, 2:RARE )
    int m_iAwakeLimitTime;	//���� ���� ��

	BYTE m_byReinforceGrade;

	CharCostume m_costume_item[MAX_CHAR_COSTUME_SLOT];
	CharAccessory m_accessory_item[MAX_CHAR_ACCESSORY_SLOT];

	CHARACTER()
	{
		Init();
	}

	void Init()
	{
		m_class_type    = 0;
		m_kindred		= 1;
		m_sex			= 1;
		m_face			= 1;
		m_hair			= 1;
		m_skin_color	= 1;
		m_hair_color	= 1;
		m_underwear     = 1;

		m_beard			= 0;     //���� ����ϴ� �� ����
		m_accessories	= 0;     //���� ����ϴ� �� ����

		for (int i = 0; i < MAX_CHAR_DBITEM_SLOT ; i++)
			m_extra_item[i] = 0;

		m_iSlotIndex    = -1;
		m_iLimitSecond  = 0;

		m_sLeaderType  = CLT_GENERAL;
		m_sRentalType  = CRT_GENERAL;
		m_dwRentalMinute = 0;
#ifdef DEFAULT_CHAR_DATE
		m_ePeriodType  = CPT_DATE;
#else
		m_ePeriodType  = CPT_TIME;
#endif

		m_bActive       = true;
		m_chExerciseStyle = 0x00;
		m_chAwakeType	  = AWAKE_NONE;		
		m_iAwakeLimitTime = 0;
		m_byReinforceGrade = 0;

		for( int i = 0; i < MAX_CHAR_COSTUME_SLOT; i++ )
		{
			m_costume_item[i].m_iCostumeIndex		= 0;
			m_costume_item[i].m_iCostumeCode		= 0;
			m_costume_item[i].m_iMaleCustomCode		= 0;
			m_costume_item[i].m_iFemalCustomCode	= 0;
		}

		for( int i = 0; i < MAX_CHAR_ACCESSORY_SLOT; i++ )
		{
			m_accessory_item[i].m_iAccessoryIndex = 0;
			m_accessory_item[i].m_iAccessoryCode  = 0;
			m_accessory_item[i].m_iAccessoryValue  = 0;
		}
	}

// 	void FillData( SP2Packet &rkPacket )
// 	{
// 		rkPacket << m_class_type;
// 		rkPacket << m_kindred;
// 		rkPacket << m_sex;			
// 		rkPacket << m_beard;		
// 		rkPacket << m_face;			
// 		rkPacket << m_hair;			
// 		rkPacket << m_skin_color;	
// 		rkPacket << m_hair_color;	
// 		rkPacket << m_accessories;	
// 		rkPacket << m_underwear;    
// 
// 		for (int i = 0; i < MAX_CHAR_DBITEM_SLOT ; i++)
// 			rkPacket << m_extra_item[i]; 
// 
// 		rkPacket << m_iSlotIndex;    
// 		rkPacket << m_iLimitSecond;  
// 
// 		rkPacket << m_sLeaderType;
// 		rkPacket << m_sRentalType;
// 		rkPacket << m_dwRentalMinute;
// 		rkPacket << (int)m_ePeriodType;   
// 
// 		rkPacket << m_bActive;       
// 		rkPacket << m_chExerciseStyle;   
// 	}

	bool FillData( SP2Packet &rkPacket )
	{
		PACKET_GUARD_bool( rkPacket.Write(m_class_type) );
		PACKET_GUARD_bool( rkPacket.Write(m_kindred) );
		PACKET_GUARD_bool( rkPacket.Write(m_sex) );		
		PACKET_GUARD_bool( rkPacket.Write(m_beard) );	
		PACKET_GUARD_bool( rkPacket.Write(m_face) );		
		PACKET_GUARD_bool( rkPacket.Write(m_hair) );		
		PACKET_GUARD_bool( rkPacket.Write(m_skin_color) );
		PACKET_GUARD_bool( rkPacket.Write(m_hair_color) );
		PACKET_GUARD_bool( rkPacket.Write(m_accessories) );
		PACKET_GUARD_bool( rkPacket.Write(m_underwear) );

		for (int i = 0; i < MAX_CHAR_DBITEM_SLOT ; i++)
		{
			PACKET_GUARD_bool( rkPacket.Write(m_extra_item[i]) ); 
		}

		PACKET_GUARD_bool( rkPacket.Write(m_iSlotIndex) );
		PACKET_GUARD_bool( rkPacket.Write(m_iLimitSecond) );  

		PACKET_GUARD_bool( rkPacket.Write(m_sLeaderType) );
		PACKET_GUARD_bool( rkPacket.Write(m_sRentalType) );
		PACKET_GUARD_bool( rkPacket.Write(m_dwRentalMinute) );
		PACKET_GUARD_bool( rkPacket.Write((int)m_ePeriodType) );

		PACKET_GUARD_bool( rkPacket.Write(m_bActive) );
		PACKET_GUARD_bool( rkPacket.Write(m_chExerciseStyle) );

		PACKET_GUARD_bool( rkPacket.Write( m_chAwakeType ) );
		PACKET_GUARD_bool( rkPacket.Write( m_iAwakeLimitTime ) );

		PACKET_GUARD_bool( rkPacket.Write( m_byReinforceGrade ) );

		for( int i = 0; i < MAX_CHAR_COSTUME_SLOT; i++ )
		{
			PACKET_GUARD_bool( rkPacket.Write(m_costume_item[i].m_iCostumeIndex) );
			PACKET_GUARD_bool( rkPacket.Write(m_costume_item[i].m_iCostumeCode) );
#ifdef CUSTOM_COSTUME
			PACKET_GUARD_bool( rkPacket.Write(m_costume_item[i].m_iMaleCustomCode) );
			PACKET_GUARD_bool( rkPacket.Write(m_costume_item[i].m_iFemalCustomCode) );
#endif
		}

		for( int i = 0; i < MAX_CHAR_ACCESSORY_SLOT; i++ )
		{
			PACKET_GUARD_bool( rkPacket.Write(m_accessory_item[i].m_iAccessoryIndex) );
			PACKET_GUARD_bool( rkPacket.Write(m_accessory_item[i].m_iAccessoryCode) );
			PACKET_GUARD_bool( rkPacket.Write(m_accessory_item[i].m_iAccessoryValue) );
		}
		return true;
	}

	bool ApplyData( SP2Packet &rkPacket )
	{
		PACKET_GUARD_bool( rkPacket.Read(m_class_type) );
		PACKET_GUARD_bool( rkPacket.Read(m_kindred) );
		PACKET_GUARD_bool( rkPacket.Read(m_sex) );
		PACKET_GUARD_bool( rkPacket.Read(m_beard) );
		PACKET_GUARD_bool( rkPacket.Read(m_face) );
		PACKET_GUARD_bool( rkPacket.Read(m_hair) );
		PACKET_GUARD_bool( rkPacket.Read(m_skin_color) );
		PACKET_GUARD_bool( rkPacket.Read(m_hair_color) );
		PACKET_GUARD_bool( rkPacket.Read(m_accessories) );
		PACKET_GUARD_bool( rkPacket.Read(m_underwear) );

		for (int i = 0; i < MAX_CHAR_DBITEM_SLOT ; i++)
		{
			PACKET_GUARD_bool( rkPacket.Read(m_extra_item[i]) ); 	
			
		}
		PACKET_GUARD_bool( rkPacket.Read(m_iSlotIndex) );
		PACKET_GUARD_bool( rkPacket.Read(m_iLimitSecond) );  

		PACKET_GUARD_bool( rkPacket.Read(m_sLeaderType) );
		PACKET_GUARD_bool( rkPacket.Read(m_sRentalType) );
		PACKET_GUARD_bool( rkPacket.Read(m_dwRentalMinute) );

		int iPeriodType = 0;
		PACKET_GUARD_bool( rkPacket.Read(iPeriodType) );
		m_ePeriodType = (CharPeriodType) iPeriodType;   

		PACKET_GUARD_bool( rkPacket.Read(m_bActive) );
		PACKET_GUARD_bool( rkPacket.Read(m_chExerciseStyle) );

		PACKET_GUARD_bool( rkPacket.Read( m_chAwakeType ) );
		PACKET_GUARD_bool( rkPacket.Read( m_iAwakeLimitTime ) );

		PACKET_GUARD_bool( rkPacket.Read( m_byReinforceGrade ) );

		for( int i = 0; i < MAX_CHAR_COSTUME_SLOT; i++ )
		{
			PACKET_GUARD_bool( rkPacket.Read(m_costume_item[i].m_iCostumeIndex) );
			PACKET_GUARD_bool( rkPacket.Read(m_costume_item[i].m_iCostumeCode) );
#ifdef CUSTOM_COSTUME
			PACKET_GUARD_bool( rkPacket.Read(m_costume_item[i].m_iMaleCustomCode) );
			PACKET_GUARD_bool( rkPacket.Read(m_costume_item[i].m_iFemalCustomCode) );
#endif
		}

		for( int i = 0; i < MAX_CHAR_ACCESSORY_SLOT; i++ )
		{
			PACKET_GUARD_bool( rkPacket.Read(m_accessory_item[i].m_iAccessoryIndex) );
			PACKET_GUARD_bool( rkPacket.Read(m_accessory_item[i].m_iAccessoryCode) );
			PACKET_GUARD_bool( rkPacket.Read(m_accessory_item[i].m_iAccessoryValue) );
		}
		return true;
	}
};

enum StatType
{
	STAT_NONE,
	STAT_STR,
	STAT_DEX,
	STAT_INT,
	STAT_VIT,
};

struct Stat
{
	union
	{
		struct
		{
			float	m_fStrength;
			float	m_fDexterity;
			float	m_fIntelligence;
			float	m_fVitality;
		};

		float m_fStat[4];
	};

	Stat()
	{
		Initialize();
	}

	void Initialize()
	{
		m_fStrength = 0.0f;
		m_fDexterity = 0.0f;
		m_fIntelligence = 0.0f;
		m_fVitality = 0.0f;
	}

	void ZeroBound()
	{
		m_fStrength     = max( m_fStrength, 0.0f );
		m_fDexterity    = max( m_fDexterity, 0.0f );
		m_fIntelligence = max( m_fIntelligence, 0.0f );
		m_fVitality     = max( m_fVitality, 0.0f );
	}

	Stat& operator += ( const Stat &rhs )
	{
		m_fStrength += rhs.m_fStrength;
		m_fDexterity += rhs.m_fDexterity;
		m_fIntelligence += rhs.m_fIntelligence;
		m_fVitality += rhs.m_fVitality;

		return *this;
	}
	
	Stat& operator -= ( const Stat &rhs )
	{
		m_fStrength -= rhs.m_fStrength;
		m_fDexterity -= rhs.m_fDexterity;
		m_fIntelligence -= rhs.m_fIntelligence;
		m_fVitality -= rhs.m_fVitality;

		return *this;
	}
};

enum RaceDetailType
{
	RDT_HUMAN_MAN,
	RDT_HUMAN_WOMAN,
	RDT_ELF_MAN,
	RDT_ELF_WOMAN,
	RDT_DWARF_MAN,
	RDT_DWARF_WOMAN,
	MAX_RACE_DETAIL
};

enum ObejctCreatorType
{
	OCT_NONE,
	OCT_SOILDER,
	OCT_EQUIP_SKILL,
	OCT_EQUIP_BUFF1,
	OCT_EQUIP_BUFF2,
};


#endif