#ifndef ___DEFINE_H__
#define ___DEFINE_H__
#include "../include/cSingleton.h"
#include "Network/GameSvrUDPModule.h"
#include "Network/GameSvrUDPNode.h"
#include "../../src/iocpSocketDLL/SocketModules/PacketQueue.h"
#include <iterator>
#include "IPBlocker/IPBlockerManager.h"
#include "Filter/WordFilterManager.h"
#include "Languages/ioLanguages.h"


class ioBroadCastRelayModule;
#define STR_IP_MAX 64
#define ENC_LOGIN_KEY_NUM       30
#define LOGIN_KEY_PLUS_ONE      16
#define ENC_ID_NUM_PLUS_ONE     25
#define STR_COUNTRY_TYPE         2

#ifdef THAILAND_LONG_ID
#define ID_NUMBER       128
#define ID_NUM_PLUS_ONE 129

#else
#define ID_NUMBER       20
#define ID_NUM_PLUS_ONE 21
#endif

#ifdef US_SHORT_ID
#define MIN_ID_NUMBER   1
#else
#define MIN_ID_NUMBER   4
#endif

#define PW_NUMBER       12
#define PW_NUM_PLUS_ONE 13
#define PW_ENCRYPT_NUMBER	24
#define PW_ENCRYPT_PLUS_ONE 25

#define MAX_RIGHT_SLOT_SIZE 20	//������ ���� �� �ִ� �ε��� ������

#define MAX_BATTLE_OBSERVER 4
#define MAX_PLAYER			16
#define MAX_PLAZA_PLAYER	32
#define SET_ITEM_CODE   700000  // ��Ʈ ������ �ν� �ڵ�. 

#define MAX_JOIN_CHANNEL 2

#define KINDRED_HUMAN	1
#define KINDRED_ELF		2
#define KINDRED_DWARF	3

#define	EQUIP_UNKNOWN	1000
#define EQUIP_WEAPON    0
#define EQUIP_ARMOR     1
#define EQUIP_HELM      2
#define EQUIP_CLOAK     3
#define EQUIP_OBJECT	4
#define EQUIP_WEAR      5
#define EQUIP_RING      6
#define EQUIP_NECKLACE  7
#define EQUIP_BRACELET  8
#define MAX_EQUIP_SLOT	9		//Character EquipSlot�� �ִ������

#define GUILD_NAME_NUMBER			20
#define GUILD_NAME_NUM_PLUS_ONE		21
#define GUILD_POS_NUMBER			20
#define GUILD_POS_NUM_PLUS_ONE		21
#define GUILD_TITLE_NUMBER			110
#define GUILD_TITLE_NUMBER_PLUS_ONE 111
#define GUILD_CREATE_ENTRY_USER     8
#define GUILD_MAX_ENTRY_DELAY_USER  16
#define GUILD_MAX_ENTRY_USER		32

#define TOURNAMENT_TITLE_NUM_PLUS_ONE              21
#define TOURNAMENT_CAMP_NAME_NUM_PLUS_ONE          21
#define TOURNAMENT_TEAM_NAME_NUM_PLUS_ONE          21
#define TOURNAMENT_TEAM_MAX_LOAD                   10
#define TOURNAMENT_CHEER_MAX_LOAD                  10

#define CREATE_RESERVE_DELAY_TIME          10000       //���� ��� ��Ƽ ���� ���� ��� �ð�

#define US_TUTORIAL_CLEAR      -1

#define MAX_CHAR_DBITEM_SLOT   4	//InventorySlot�� �ִ������
#define MAX_CHAR_COSTUME_SLOT	4	//costume max ������
#define MAX_CHAR_ACCESSORY_SLOT	3	//costume max ������

#define COSTUME_SUBTYPE_DELIMITER	100000
#define ACCESSORY_SUBTYPE_DELIMITER	1000000
#define IP_NUM_PLUS_ONE 16

#define USER_GUID_NUM_PLUS_ONE 32

#define CHANNELING_USER_ID_NUM_PLUS_ONE 33
#define CHANNELING_USER_NO_NUM_PLUS_ONE 21

#define MAX_CONTROL_KEYS_PLUS_ONE       201
#define USER_BIRTH_DATE_PLUS_ONE        7

#define CHANNELING_KEY_VALUE_PLUS_ONE	101

#define MAX_INT_VALUE					2147483647

// �α� ���� - 
#define LOG_TEST_LEVEL                  -1

#ifdef _DEBUG
#define LOG_SHUFFLE						0
#else
#define LOG_SHUFFLE						-1
#endif

#define LOG_DEBUG_LEVEL                 0
#define LOG_RELEASE_LEVEL               0

#define COMPARE(x,min,max) (((x)>=(min))&&((x)<(max)))
#define SAFEDELETE(x)		if(x != NULL) { delete x; x = NULL; }
#define SAFEDELETEARRAY(x)	if(x != NULL) { delete [] x; x = NULL; }

const int G_MAXDELAY_REQUESTUSERDATA		=	300;
const int G_MAXDELAY_CHECK_TITLE_PREMIUM    =	3000;

const int DB_COSTUME_SELECT_COUNT			=	500;
const int DB_EXTRAITEM_SELECT_COUNT			=	50;
const int DB_DECO_SELECT_COUNT				=	100;
const int DB_ACCESSORY_SELECT_COUNT			=	500;

const int MAX_SELECT_EXTRA_ITEM_CODE		=	10;

const int R_SOLDIER_START_NUM				=	501;
const int R_SOLDIER_END_NUM					=	600;

const int GUILD_MAP_ARRAY					=	64;
const int GUILD_MAP_XZ_ARRAY				=	GUILD_MAP_ARRAY * GUILD_MAP_ARRAY;
const int GUILD_MAP_Y_ARRAY					=	20;

const int DB_BUILT_BLOCK_ITEM_SELECT_COUNT	= 500;
const int DB_GUILD_IN_SELCET_COUNT			= 7000;

const int GUILD_ROOM_ITEM_DELIMITER			= 1000000;

const int HOME_MAP_ARRAY					=	32;
const int HOME_MAP_XZ_ARRAY					=	HOME_MAP_ARRAY * HOME_MAP_ARRAY;
const int HOME_MAP_Y_ARRAY					=	8;

const int HOME_MODE_ITEM_DELIMITER			= 1000000;

const int GFRIEND_SOLDIER_START_NUM			= 807;
const int GFRIEND_SOLDIER_END_NUM			= 938;

#define FREEDAY_EVENT_CODE			(0x02000000)  // jal : �������� �Ǿ���ó��

enum GuildRoomItemType
{
	GRT_NONE	= 0,
	GRT_BLOCK	= 1,
	GRT_TILE	= 2,
	GRT_FISHERY	= 3,
	GRT_END		= 4,
};

enum HomeModeItemType
{
	HMT_NONE	= 0,
	HMT_BLOCK	= 1,
	HMT_TILE	= 2,
	HMT_END		= 3,
};

enum BlockPropertyType
{
	BPT_NONE	= 0,
	BPT_GUILD	= 1,
	BPT_HOME	= 2,
};

//extern CLog CheatLOG;


//rudp ����ؼ� �õ��������� + 10���� ����
#define MAX_PASS_SEED_VALUE_USER 330 * 2
#define MAX_PASS_SEED_VALUE_NPC 220 * 2
#define MAX_SEED_CHECK_COUNT 100

struct Vector3
{
	float x,y,z;

	Vector3(){}
	Vector3( float _x, float _y, float _z )
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

struct Quaternion
{
	float x, y, z, w;
	
	Quaternion()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 1;
	}

	Quaternion( float _x, float _y, float _z, float _w )
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};

typedef std::vector< Vector3 > Vector3Vec;
typedef std::deque< Vector3 > Vector3Deq;

enum TeamType
{
	TEAM_NONE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_PRIVATE_1,
	TEAM_PRIVATE_2,
	TEAM_PRIVATE_3,
	TEAM_PRIVATE_4,
	TEAM_PRIVATE_5,
	TEAM_PRIVATE_6,
	TEAM_PRIVATE_7,
	TEAM_PRIVATE_8,
	TEAM_PRIVATE_9,
	TEAM_PRIVATE_10,
	TEAM_PRIVATE_11,
	TEAM_PRIVATE_12,
	TEAM_PRIVATE_13,
	TEAM_PRIVATE_14,
	TEAM_PRIVATE_15,
	TEAM_PRIVATE_16,
	TEAM_PRIVATE_17,
	TEAM_PRIVATE_18,
	TEAM_PRIVATE_19,
	TEAM_PRIVATE_20,
	TEAM_PRIVATE_21,
	TEAM_PRIVATE_22,
	TEAM_PRIVATE_23,
	TEAM_PRIVATE_24,
	TEAM_PRIVATE_25,
	TEAM_PRIVATE_26,
	TEAM_PRIVATE_27,
	TEAM_PRIVATE_28,
	TEAM_PRIVATE_29,
	TEAM_PRIVATE_30,
	TEAM_PRIVATE_31,
	TEAM_PRIVATE_32,
	TEAM_PRIVATE_33,
	TEAM_PRIVATE_34,
	TEAM_PRIVATE_35,
	TEAM_PRIVATE_36,
	TEAM_PRIVATE_37,
	TEAM_PRIVATE_38,
	TEAM_PRIVATE_39,
	TEAM_PRIVATE_40,
	TEAM_PRIVATE_41,
	TEAM_PRIVATE_42,
	TEAM_PRIVATE_43,
	TEAM_PRIVATE_44,
	TEAM_PRIVATE_45,
	TEAM_PRIVATE_46,
	TEAM_PRIVATE_47,
	TEAM_PRIVATE_48,
	TEAM_PRIVATE_49,
	TEAM_PRIVATE_50,
	TEAM_PRIVATE_51,
	TEAM_PRIVATE_52,
	TEAM_PRIVATE_53,
	TEAM_PRIVATE_54,
	TEAM_PRIVATE_55,
	TEAM_PRIVATE_56,
	TEAM_PRIVATE_57,
	TEAM_PRIVATE_58,
	TEAM_PRIVATE_59,
	TEAM_PRIVATE_60,
	TEAM_PRIVATE_61,
	TEAM_PRIVATE_62,
	TEAM_PRIVATE_63,
	TEAM_PRIVATE_64,
	TEAM_PRIVATE_65,
	TEAM_PRIVATE_66,
	TEAM_PRIVATE_67,
	TEAM_PRIVATE_68,
	TEAM_PRIVATE_69,
	TEAM_PRIVATE_70,
	TEAM_PRIVATE_71,
	TEAM_PRIVATE_72,
	TEAM_PRIVATE_73,
	TEAM_PRIVATE_74,
	TEAM_PRIVATE_75,
	TEAM_PRIVATE_76,
	TEAM_PRIVATE_77,
	TEAM_PRIVATE_78,
	TEAM_PRIVATE_79,
	TEAM_PRIVATE_80,
	TEAM_PRIVATE_81,
	TEAM_PRIVATE_82,
	TEAM_PRIVATE_83,
	TEAM_PRIVATE_84,
	TEAM_PRIVATE_85,
	TEAM_PRIVATE_86,
	TEAM_PRIVATE_87,
	TEAM_PRIVATE_88,
	TEAM_PRIVATE_89,
	TEAM_PRIVATE_90,
	TEAM_PRIVATE_91,
	TEAM_PRIVATE_92,
	TEAM_PRIVATE_93,
	TEAM_PRIVATE_94,
	TEAM_PRIVATE_95,
	TEAM_PRIVATE_96,
	TEAM_PRIVATE_97,
	TEAM_PRIVATE_98,
	TEAM_PRIVATE_99,
	TEAM_PRIVATE_100,
	TEAM_PRIVATE_101,
	TEAM_PRIVATE_102,
	TEAM_PRIVATE_103,
	TEAM_PRIVATE_104,
	TEAM_PRIVATE_105,
	TEAM_PRIVATE_106,
	TEAM_PRIVATE_107,
	TEAM_PRIVATE_108,
	TEAM_PRIVATE_109,
	TEAM_PRIVATE_110,
	TEAM_PRIVATE_111,
	TEAM_PRIVATE_112,
	TEAM_PRIVATE_113,
	TEAM_PRIVATE_114,
	TEAM_PRIVATE_115,
	TEAM_PRIVATE_116,
	TEAM_PRIVATE_117,
	TEAM_PRIVATE_118,
	TEAM_PRIVATE_119,
	TEAM_PRIVATE_120,
	TEAM_PRIVATE_121,
	TEAM_PRIVATE_122,
	TEAM_PRIVATE_123,
	TEAM_PRIVATE_124,
	TEAM_PRIVATE_125,
	TEAM_PRIVATE_126,
	TEAM_PRIVATE_127,
	TEAM_PRIVATE_128,
	TEAM_PRIVATE_129,
	TEAM_PRIVATE_130,
	TEAM_PRIVATE_131,
	TEAM_PRIVATE_132,
	TEAM_PRIVATE_133,
	TEAM_PRIVATE_134,
	TEAM_PRIVATE_135,
	TEAM_PRIVATE_136,
	TEAM_PRIVATE_137,
	TEAM_PRIVATE_138,
	TEAM_PRIVATE_139,
	TEAM_PRIVATE_140,
	TEAM_PRIVATE_141,
	TEAM_PRIVATE_142,
	TEAM_PRIVATE_143,
	TEAM_PRIVATE_144,
	TEAM_PRIVATE_145,
	TEAM_PRIVATE_146,
	TEAM_PRIVATE_147,
	TEAM_PRIVATE_148,
	TEAM_PRIVATE_149,
	TEAM_PRIVATE_150,
	TEAM_PRIVATE_151,
	TEAM_PRIVATE_152,
	TEAM_PRIVATE_153,
	TEAM_PRIVATE_154,
	TEAM_PRIVATE_155,
	TEAM_PRIVATE_156,
	TEAM_PRIVATE_157,
	TEAM_PRIVATE_158,
	TEAM_PRIVATE_159,
	TEAM_PRIVATE_160,
	TEAM_PRIVATE_161,
	TEAM_PRIVATE_162,
	TEAM_PRIVATE_163,
	TEAM_PRIVATE_164,
	TEAM_PRIVATE_165,
	TEAM_PRIVATE_166,
	TEAM_PRIVATE_167,
	TEAM_PRIVATE_168,
	TEAM_PRIVATE_169,
	TEAM_PRIVATE_170,
	TEAM_PRIVATE_171,
	TEAM_PRIVATE_172,
	TEAM_PRIVATE_173,
	TEAM_PRIVATE_174,
	TEAM_PRIVATE_175,
	TEAM_PRIVATE_176,
	TEAM_PRIVATE_177,
	TEAM_PRIVATE_178,
	TEAM_PRIVATE_179,
	TEAM_PRIVATE_180,
	TEAM_PRIVATE_181,
	TEAM_PRIVATE_182,
	TEAM_PRIVATE_183,
	TEAM_PRIVATE_184,
	TEAM_PRIVATE_185,
	TEAM_PRIVATE_186,
	TEAM_PRIVATE_187,
	TEAM_PRIVATE_188,
	TEAM_PRIVATE_189,
	TEAM_PRIVATE_190,
};

enum CampType
{
	CAMP_NONE,
	CAMP_BLUE,
	CAMP_RED,
};

enum WinTeamType
{
	WTT_NONE,
	WTT_RED_TEAM,
	WTT_BLUE_TEAM,
	WTT_DRAW,
	WTT_VICTORY_RED_TEAM,
	WTT_VICTORY_BLUE_TEAM
};

enum ModeType
{
	MT_NONE				= 0,
	MT_SYMBOL			= 1,
	MT_CATCH			= 2,
	MT_KING				= 3,
	MT_TRAINING			= 4,
	MT_SURVIVAL			= 5,
	MT_TEAM_SURVIVAL	= 6,
	MT_BOSS				= 7,
	MT_MONSTER_SURVIVAL = 8,
	MT_FOOTBALL			= 9,
	MT_HEROMATCH		= 10,
	MT_GANGSI			= 11,
	MT_DUNGEON_A    	= 12,
	MT_HEADQUARTERS     = 13,
	MT_CATCH_RUNNINGMAN = 14,
	MT_FIGHT_CLUB		= 15,
	MT_TOWER_DEFENSE	= 16,
	MT_DARK_XMAS		= 17,
	MT_FIRE_TEMPLE		= 18,
	MT_DOBULE_CROWN		= 19,
	MT_SHUFFLE_BONUS	= 20,
	MT_FACTORY			= 21,
	MT_TEAM_SURVIVAL_AI = 22,
	MT_HOUSE			= 23,
	MT_MYROOM			= 24,		
	MT_UNDERWEAR		= 25,
	MT_CBT				= 26,
	MT_RAID				= 27,
	MAX_MODE_TYPE
};

enum RoomStyle
{
	RSTYLE_NONE	= 0,
	RSTYLE_BATTLEROOM,	//������
	RSTYLE_PLAZA,		//����
	RSTYLE_LADDERBATTLE,//�����
	RSTYLE_HEADQUARTERS,//����
	RSTYLE_SHUFFLEROOM,	//����������
};

enum PlazaType
{
	PT_NONE      = 0,
	PT_BATTLE	 = 1,
	PT_COMMUNITY = 2,
	PT_GUILD     = 3,
};

// User::OnTrail / DBClient::OnInsertTrail �Լ����� ���
// Client ioMannerTrialChatManager::TrialType ����
enum TrialType
{
	TT_NONE         = 0,
	TT_NORMAL_CHAT  = 1, // ��
	TT_BATTLE_CHAT  = 2, // ������ 
	TT_CHANNEL_CHAT = 3,
	TT_MEMO         = 4,
	TT_GUILD_CHAT   = 5,
};

// DB�� ����Ǵ� ������ ���������� �������� �ʴ´�.
enum ChannelingType
{
	CNT_NONE        =   -1,
	CNT_WEMADEBUY   =    0,
	CNT_STEAM       =    1,
	CNT_MGAME       =   300,
	CNT_DAUM        =   400,
	CNT_NAVER       =   600,
	CNT_TOONILAND   =   700,
	CNT_NEXON		=	800,
	CNT_HANGAME		=	900,
};

//HR EU ���� Ÿ�� �߰���
enum EUCountryType
{
	EUCT_ENGLISH = 0,
	EUCT_DEUTSCH,
	EUCT_FRANCH,
	EUCT_ITALIANO,
	EUCT_POLISKY,
	EUCT_TURKEY,
	EUCT_NONE,
};

/*
#define EUCT_ENGLISH	"EU"
#define	EUCT_DEUTSCH	"DM"
#define EUCT_FRANCH		"FR"
#define EUCT_ITALIANO	"IT"
#define EUCT_POLISKY	"PO"
#define EUCT_TURKEY		"TK"
#define EUCT_NONE		"NO"
*/


// DB�� ����Ǵ� ������ ���������� �������� �ʴ´�.
enum BlockType
{
	BKT_NONE        = -1,
	BKT_NORMAL      =  0,   // ���ܾȵ� (����)
	BKT_WARNNING    = 10,   // ��� ( �� )
	BKT_LIMITATION  = 20,   // ���� ( �� )
	BKT_BLOCK       = 100,  // ���� ( �� )
};

// û��öȸ
enum
{
	SUBSCRIPTION_NONE	= 0,	// û��öȸ �Ұ�
	SUBSCRIPTION_TYPE1	= 1,	// û��öȸ ����
};

//���� ������ ���� ��ο� ���� ó��
enum RoomEntryType
{
	RET_PLAZA_TO_ROOM						= 0,
	RET_MOVE_PLAZA_TO_ROOM					= 1,
	RET_ROOM_TO_ROOM						= 2,
	RET_MOVE_ROOM_TO_ROOM					= 3,
};

//�뺴 , ������ �Ŀ���
enum PowerUpTargetType
{
	PUTT_NONE = 0,
	PUTT_CHAR = 1,
	PUTT_ITEM = 2,
};

enum PowerUpGradeType
{
	PUGT_NONE = 0,

	PUGT_CHAR_GRADE1 = 1,
	PUGT_CHAR_GRADE2 = 2,
	PUGT_CHAR_GRADE3 = 3,
	PUGT_CHAR_GRADE4 = 4,
	PUGT_CHAR_GRADE5 = 5,

	PUGT_ITEM_GRADE_BASE = 10000,

	PUGT_ITEM_GRADE1 = 50000,
	PUGT_ITEM_GRADE2 = 60000,
	PUGT_ITEM_GRADE3 = 70000,
	PUGT_ITEM_GRADE4 = 80000,
	PUGT_ITEM_GRADE5 = 90000,
};

enum PowerUpRareItemGradeType
{
	PURIGT_NONE = 0,

	PURIGT_RARE_ITEM_GRADE_BASE = 1000,

	PURIGT_RARE_ITEM_GRADE1 = 5000,
	PURIGT_RARE_ITEM_GRADE2 = 6000,
	PURIGT_RARE_ITEM_GRADE3 = 7000,
	PURIGT_RARE_ITEM_GRADE4 = 8000,
	PURIGT_RARE_ITEM_GRADE5 = 9000,
	PURIGT_RARE_ITEM_GRADE_MAX = 10000,
};

//�����ۺ� ���� Ÿ��
enum ExtraItemExtendType
{
	EIET_DEFAULT, // �⺻ ����Ʈ ������
	EIET_EXTRA, // ��� 
	EIET_RARE, // ����
	EIET_RARE_POWERUP,	// ��ȭ ���� ������
	EIET_SPECIAL_EXTRA, // Ư�� ���
	EIET_DEFAULT_POWERUP, // ��ȭ ����Ʈ ������
	EIET_EXTREA_POWERUP,  // ��ȭ ��� ������
};

//��� ������ Ÿ��
enum AllItemType
{
	AIT_SOLDIER			= 1,
	AIT_DECORATION,
	AIT_ETC_ITEM,
	AIT_PESO,
	AIT_EXTRAITEM,
	AIT_EXTRAITEM_BOX,
	AIT_RANDOM_DECO,
	AIT_EXP,
	AIT_MEDALITEM,
	AIT_ALCHEMIC_ITEM,
	AIT_PET,
	AIT_COSTUME,
};

//��Ų ���� ���� Ÿ��
enum SkinDeleteType
{
	SDT_ALL		= 0,
	SDT_MALE	= 1,
	SDT_FEMALE	= 2,
	SDT_POWERUP	= 3,
};

enum MaterialCode
{
	ADDITIVE_CODE_NUM = 100001,
};

enum AdditiveMissionTypes
{
	AMT_AWAKE		= 1,
	AMT_PET			= 2,
	AMT_COMPOUND	= 3,
	AMT_REINFORCE	= 4,
};

//��� ��ũ
enum GuildRank
{
	GUILD_RANK_F	= 0,
	GUILD_RANK_E	= 1,
	GUILD_RANK_D	= 2,
	GUILD_RANK_C	= 3,
	GUILD_RANK_B	= 4,
	GUILD_RANK_A	= 5,
	GUILD_RANK_S	= 6,
};

enum SpecialShopBuyType
{
	SBT_GOODS_BUY		= 0,
	SBT_GOODS_PRESENT	= 1,
};

enum EtcBuyResultType
{
    EBRT_BUY,
    EBRT_PRESENT,
};

enum ItemSearchingType
{
	IST_ALL		= 0,
	IST_EQUIP	= 1,
	IST_RELEASE	= 2,
};

enum GuildBlockRetrieveORDeleteType
{
	GBT_RETRIEVE	= 0,
	GBT_DELETE		= 1,
	GBT_EXCEPTION	= 2,
};

enum GuildBlockConstructORMoveType
{
	GBT_CONSTRUCT			= 1,
	GBT_MOVE				= 2,
	GBT_DEFAULT_CONSTRUCT	= 3,
};

enum GuildBlockAddType
{
	GBA_ITEM_BUY		= 1,
	GBA_ITEM_PRESENT	= 2,
};

enum GuilInvenRequestType
{
	INVEN_OPEN		= 0,
	CONSTRUCT_MODE	= 1,
};

enum BlockModeType
{
	BMT_GUILD		= 0,
	BMT_PERSONAL		= 1,
};

enum CompensationType
{
	CT_MAINTENANCE	= 0,	//���� ����.
};

enum SkillType
{
	ST_NONE,
	ST_ATTACK,
	ST_BUFF,
	ST_NORMAL,
	ST_PASSIVE,
	ST_RANGE,
	ST_MULTI,
	ST_COUNT,
};

enum AttackSubType
{
	AST_METEOR = 9,
	AST_RANDOM_GENERATE = 10,
};

enum BuffType
{
	BT_RESTORATION_GAUGE = 17,
};

enum GrowthType
{
	GT_SKILL_COOL_TIME = 7,
};


enum MoniterInertItemTypes
{
	MONITER_INSERT_ITEM_PUBLIC_ID = 1,
	MONITER_INSERT_ITEM_PRIVATE_ID = 2,
};


enum UpdateTimeCashType
{
	UTCT_SUCCESS	= 0,
	UTCT_EXPIRE		= 1,
	UTCT_FAIL		= 2,
};

enum TitleUpdateType
{
	TUT_INSERT		= 0,
	TUT_UPDATE		= 1,
	TUT_PREMIUM		= 2,
	TUT_LEVELUP		= 3,
	TUT_INSERT_ETC	= 4,
	TUT_EQUIP		= 5,
	TUT_ALL_RELEASE	= 6,
	TUT_RELEASE		= 7,
};

enum SpecialSoldierType
{
	SST_RSOLDIER	= 0,
	SST_GFRIEND		= 1,
	SST_END			= 2,
};

enum BonusCashUpdateType
{
	BUT_CHANGE	= 1,
	BUT_END		= 0,
};

enum TradeType
{
	TRADE_NONE	= 0,
	TRADE_ALL	= 1,	// ���μ����� ó�� ������ �ŷ��� ������ ��ü ����Ʈ ��Ŷ
	TRADE_ADD	= 2,	// ���μ����� ������ �ŷ��� ������ �߰� ��Ŷ
	TRADE_DEL	= 3,	// ���μ����� ������ �ŷ��� ������ ���� ��Ŷ
};

struct UserRankInfo
{
	ioHashString szName;
	int iRank;

	UserRankInfo()
	{
		iRank = 0;
	}
};

struct DamageTable
{
	ioHashString szName;
	int iDamage;

	DamageTable()
	{
		iDamage = 0;
	}
};

struct ControlKeys
{
	char m_szControlKeys[MAX_CONTROL_KEYS_PLUS_ONE];

	bool IsRight()
	{
		if( strcmp( m_szControlKeys, "" ) == 0 )
			return false;

		for (int i = 0; i < MAX_CONTROL_KEYS_PLUS_ONE ; i++)
		{
			if( m_szControlKeys[i] == NULL )
				break;

			if ((!COMPARE(m_szControlKeys[i], 'A', 'Z'+1)) &&
				(!COMPARE(m_szControlKeys[i], 'a', 'z'+1)) &&
				(!COMPARE(m_szControlKeys[i], '0', '9'+1)) )
			{
				return false;
			}

		}

		return true;
	}

	void Clear()
	{
		ZeroMemory( m_szControlKeys, sizeof( m_szControlKeys ) );
	}

	ControlKeys()
	{
		Clear();
	}

};

struct IntOfTwo
{
	int value1;
	int value2;

	IntOfTwo()
	{
		value1	= 0;
		value2	= 0;
	}
};

typedef std::vector< DamageTable > DamageTableList;

#define MAX_MODE ( MAX_MODE_TYPE - 1 )

typedef std::vector<int> IntVec;
typedef std::vector<DWORD> DWORDVec;
typedef std::vector<float> FloatVec;
typedef std::vector<bool> BoolVec;
typedef std::vector<UserRankInfo> UserRankInfoList;
typedef std::vector<IntOfTwo> IntOfTwoVec;

extern LONG __stdcall ExceptCallBack ( EXCEPTION_POINTERS * pExPtrs );
extern char *_i64toStr(__int64 i64Num);
/************************************************************************/
/* packetid(UDP ����)                                                                     */
/************************************************************************/
enum UDPPACKETID
{
	UDP_INSERTDATAPACKET = 0x8001,
	UDP_REMOVEPACKET,
	UDP_SENDPACKET,
	UDP_UPDATEWINCNT,
	UDP_UPDATESCORE,
	UDP_TIMER,
	UDP_ANTIHACK_RELOAD,
	UDP_UPDATE_SP_POTION,
	UDP_USER_DIESTATE,
};

/************************************************************************/
/* Struct                                                                     */
/************************************************************************/
struct RelayHeader : public NodeData
{
	int m_packetId;
};

struct UDPPacket :RelayHeader
{
	sockaddr_in m_client_addr;
	int m_size;
	char* m_buffer;

};

//////////////////////////////////////////////////////////////////////////
#ifndef ANTIHACK
struct InsertData : RelayHeader
{
	DWORD		 m_dwRoomIndex;
	DWORD		 m_dwUserIndex;
	char         m_szPublicIP[STR_IP_MAX];
	int          m_iClientPort;
};

struct UserData
{
	DWORD		 m_dwUserIndex;
	char         m_szPublicIP[STR_IP_MAX];
	int          m_iClientPort;

};

struct RelayGroup			
{
	typedef std::vector<UserData> RelayGroups;
	DWORD   m_dwRoomIndex;
	RelayGroups m_RelayUserList;

public:
	void GetUserLists(RelayGroups& relayGroup) 
	{
		std::copy(m_RelayUserList.begin(),m_RelayUserList.end(),std::back_inserter(relayGroup));
	}
	RelayGroups* GetUsers() { return &m_RelayUserList; }

	void AddUser(const DWORD dwUserIndex, const int iPort, const char* szIP)
	{
		for(unsigned int i=0; i< m_RelayUserList.size() ; i++)
		{
			UserData& rkUser = m_RelayUserList.at(i);
			if( rkUser.m_dwUserIndex == dwUserIndex )
			{
				// ���� ���� ����
				rkUser.m_iClientPort= iPort;
				strcpy_s(rkUser.m_szPublicIP, szIP);
				return;
			}
		}

		UserData kUserData;
		kUserData.m_dwUserIndex = dwUserIndex;
		kUserData.m_iClientPort = iPort;
		strcpy_s(kUserData.m_szPublicIP, szIP);

		m_RelayUserList.push_back(kUserData);
	}
	void RemoveUser(const DWORD dwUserIndex)
	{
		for(unsigned int i=0; i< m_RelayUserList.size() ; i++)
		{
			UserData& rkUser = m_RelayUserList.at(i);
			if( rkUser.m_dwUserIndex == dwUserIndex )
			{
				m_RelayUserList.erase(m_RelayUserList.begin() + i);
				break;
			}
		}
	}
};
#else
//////////////////////////////////////////////////////////////////////////
struct InsertData : RelayHeader
{
	DWORD		 m_dwRoomIndex;
	DWORD		 m_dwUserIndex;
	char         m_szPublicIP[STR_IP_MAX];
	int          m_iClientPort;
	DWORD		 m_dwUserSeed;
	DWORD		 m_dwNPCSeed;
	int			 m_iCoolType;
	int			 m_iModeType;
	int			 m_iRoomStyle;
	int			 m_iTeamType;
};

struct UpdateRelayGroupWinCntData : RelayHeader
{
	DWORD	m_dwRoomIndex;
	int		m_iRedTeamWinCnt;
	int		m_iBueTeamWinCnt;
};

struct UpdateRelayGroupScoreData : RelayHeader
{
	DWORD	m_dwRoomIndex;
	int		m_iTeamType;
};

struct UpdateAntihackInfo : RelayHeader
{
	float fAntiErrorRate;
	int iAntiWaitTime;

	int iPenguinCount;
	int iKickCount;
	int iTimeDecrease;	

	int iSkillHackCount;
	int iSkillKickCount;
	int iSkillTimeDecrease;
	int iExceptSkillID[10];
};

struct UpdateRelayGroupSpData : RelayHeader
{
	DWORD	m_dwRoomIndex;
	DWORD	m_dwUserIndex;
};

struct UpdateDieStateData : RelayHeader
{
	DWORD m_dwRoomIndex;
	DWORD m_dwUserIndex;
	BOOL  m_bDieState;
};

struct UserData
{
	DWORD		 m_dwUserIndex;
	char         m_szPublicIP[STR_IP_MAX];
	int          m_iClientPort;
	DWORD		 m_dwUserSeed;
	DWORD		 m_dwNPCSeed;

	DWORD		 m_dwUserSeedOri;
	DWORD		 m_dwNPCSeedOri;

	int			 m_iTeamType;		// ���� - 1 , ��� - 2 

	std::vector<DWORD> m_vecRUDP;
	std::vector<DWORD> m_vecRUDPErase;
	std::vector<DWORD> m_vecRUDPTest;

	std::vector<DWORD> m_vecRUDPNPC;
	std::vector<DWORD> m_vecRUDPEraseNPC;
	std::vector<DWORD> m_vecRUDPTestNPC;

	//ī���ÿ�
	DWORD		m_dwReqCount;			//��û����
	DWORD		m_dwAckCount;			//�����������
	DWORD		m_dwAckCountDupl;		//ack �ι��̻� �ð�� 
	DWORD		m_dwErrCount;			//��û ���� �ʾҴµ� �� ���
	DWORD		m_dwLossCount;			//���� ī��Ʈ
	DWORD		m_dwEraseCount;			//�õ� ���� �Ŀ� ���� ���

	//�������� ���� ��ų ���� ����
	DWORD		m_dwSkillUseTime[4];	//���4��
	DWORD		m_dwSkillUseTime2[4];	//���4��
	std::vector<DWORD> m_dwVecMultiCountSkill[4];	// ��ų�߿� ���� ��Ƽ� ���ݾ� ���� ��ų��.....

	//��ų�ʿ���
	bool		m_bRecoverSkill;		// ȸ����ų�� ��� �������� ����.
	DWORD		m_dwRecoverTime;
	//����Ʈ��ų����
	bool		m_bRecoverExtraSkill;
	DWORD		m_dwRecoverExtraTime;
	
	int			m_iRecoverSkillSize;
	int			m_iSlot[4];
	DWORD		m_dwLatedTime[4];

	void OnRecvoerSkill( DWORD dwRecoverTime )
	{
		//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - 1 OnRecvoerSkill , Time(%u)", dwRecoverTime );

		m_bRecoverSkill = true;
		m_dwRecoverTime = dwRecoverTime;
		if( m_bRecoverExtraSkill )
		{
			UpdateRecoverSkill();
		}
	}
	void OnRecoverSkillExtraInfo( DWORD dwRecoverExtraTime, int nSize, int* iSlots, DWORD* dwLatedTime )
	{
		m_bRecoverExtraSkill = true;
		m_dwRecoverExtraTime = dwRecoverExtraTime;

		//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - 2 OnRecoverSkillExtraInfo, Time(%u)", dwRecoverExtraTime );

		m_iRecoverSkillSize = nSize;
		if( m_iRecoverSkillSize > 4 )
		{
			//CheatLOG.PrintTimeAndLog( 0, "[skill] Skill Extra Info Size(%d) Error", m_iRecoverSkillSize );
			m_iRecoverSkillSize = 0;
		}

		// ��ųȸ���� ���õ� ��ų�� �������� ����� ���.. a,b ����߿� a �� �սǵɰ�쿡 ���� üũ�� �߰� �ؾ� �ұ�?
		for( int i = 0; i < m_iRecoverSkillSize; ++i )
		{
			m_iSlot[i] = iSlots[i];
			m_dwLatedTime[i] = dwLatedTime[i];
		}
		
		if( m_bRecoverSkill )
		{
			UpdateRecoverSkill();
		}
	}
	void UpdateRecoverSkill()
	{
		// ���� �ð��� ȣ������ �ʰ� �� ��Ŷ�� ���� �ð������� ����
		DWORD dwDiffTime = m_dwRecoverTime > m_dwRecoverExtraTime ? m_dwRecoverTime - m_dwRecoverExtraTime : m_dwRecoverExtraTime - m_dwRecoverTime;
		//���̰� �ʹ� ���� ���� ���� �����ε�..		
		// �׽�Ʈ �غ��� 5�� �ڿ��� ������ �ֵ��� ����
		if( dwDiffTime < 10000 ) 
		{
			//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - 3 UpdateRecoverSkill OK, Time(%u)", dwDiffTime );
			for( int i = 0; i < m_iRecoverSkillSize; ++i )
			{
				int iSlot = m_iSlot[i];
				if( m_dwSkillUseTime[i] > m_dwLatedTime[i] )
					m_dwSkillUseTime[i] -= m_dwLatedTime[i];
				else
					m_dwSkillUseTime[i] = 0;

				if( m_dwSkillUseTime2[i] > m_dwLatedTime[i] )
					m_dwSkillUseTime2[i] -= m_dwLatedTime[i];
				else
					m_dwSkillUseTime2[i] = 0;

				//CheatLOG.PrintTimeAndLog( 0, "[skill] Test - 4 UpdateRecoverSkill OK, slot(%d), time(%u)", iSlot, m_dwLatedTime[i] );
			}
			m_bRecoverSkill = m_bRecoverExtraSkill = false;
		}
		else
		{
			// �� ��� ���� �ϳ��� ���� �����̴�. ���ֵ� �ɵ�..
		}
	}

	DWORD		m_dwPenaltySkill;
	DWORD		m_dwPenaltyCount;
	DWORD		m_dwLastPenaltyTime;
	DWORD		m_dwLastSkillPenaltyTime;

	// ���Ƽ �α� ���� �߰�
	DWORD		m_dwAntiHitCount;
	DWORD		m_dwAntiHitCount2;
	DWORD		m_dwAntiSkillCount;
	DWORD		m_dwAntiSkillCount2;

	struct sAntiHackHitData
	{
		DWORD dwAttackerIndex;
		DWORD dwWeaponIndex;

		DWORD dwHitCount;
		DWORD dwWDCount;	// wounded, defense

		DWORD dwLastTime;
		sAntiHackHitData() : dwAttackerIndex(0), dwWeaponIndex(0), dwHitCount(0), dwWDCount(0){}
	};

	std::vector<sAntiHackHitData> m_vecAntiData;

	sAntiHackHitData& GetHitData( DWORD dwAttackerIndex, DWORD dwWeaponIndex )
	{
		int nSize = m_vecAntiData.size();
		for( int i = 0; i < nSize; ++i )
		{
			if( m_vecAntiData[i].dwAttackerIndex == dwAttackerIndex &&
				m_vecAntiData[i].dwWeaponIndex == dwWeaponIndex	)
				return m_vecAntiData[i];
		}
		//������

		sAntiHackHitData antiData;
		antiData.dwAttackerIndex = dwAttackerIndex;
		antiData.dwWeaponIndex = dwWeaponIndex;

		m_vecAntiData.push_back( antiData );
		nSize = m_vecAntiData.size();
		return m_vecAntiData[nSize-1];
	}

	void AddHitData( DWORD dwAttackerIndex, DWORD dwWeaponIndex/*, bool bLog */)
	{
		sAntiHackHitData& antiData = GetHitData( dwAttackerIndex, dwWeaponIndex );
		antiData.dwHitCount++;
		antiData.dwLastTime = TIMEGETTIME();
		
		/*if( bLog )
		{
			CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] AddHitData Attacker hit me(%u >> %u), Weapon(%u), HitCount(%u), Time(%u)", dwAttackerIndex, m_dwUserIndex ,dwWeaponIndex, antiData.dwHitCount, antiData.dwLastTime );
		}*/
		}

	void AddWoundedData( DWORD dwAttackerIndex, DWORD dwWeaponIndex/*, bool bLog*/ )
	{
		sAntiHackHitData& antiData = GetHitData( dwAttackerIndex, dwWeaponIndex );
		antiData.dwWDCount++;
		antiData.dwLastTime = TIMEGETTIME();

		/*if( bLog )
		{
			CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] AddWoundedData  me wounded Attacker(%u << %u), Weapon(%u), HitCount(%u), Time(%u)", m_dwUserIndex, dwAttackerIndex, dwWeaponIndex, antiData.dwHitCount, antiData.dwLastTime );
		}*/
	}

	//bullet�۾�
	struct sBulletData
	{
		DWORD dwItemCode;
		DWORD dwCurCount;
		DWORD dwMaxCount;
		DWORD dwReloadTime;
		std::vector<DWORD> vecWasteBullet;
		std::vector<DWORD> vecWasteTime;
		std::vector<DWORD> vecRecvTime;
		sBulletData() : dwItemCode(0), dwCurCount(0), dwMaxCount(0), dwReloadTime(0){ vecWasteBullet.reserve(7); vecWasteTime.reserve(7); vecRecvTime.reserve(7); };
		void WasteBullet( DWORD dwCount, DWORD dwWasteTime, DWORD dwRecvTime )
		{
			dwCurCount += dwCount;
			vecWasteBullet.push_back( dwCount );
			vecWasteTime.push_back( dwWasteTime );
			vecRecvTime.push_back( dwRecvTime );
		};
		void Clear()
		{
			dwCurCount = 0;
			vecWasteBullet.clear();
			vecWasteTime.clear();
			vecRecvTime.clear();
		};
	};

	std::map<DWORD,sBulletData> m_mapBulletDatas;
	sBulletData* GetBulletData( DWORD dwItemCode )
	{
		auto it = m_mapBulletDatas.find( dwItemCode );
		if( it != m_mapBulletDatas.end() )
		{
			sBulletData* pData = &(it->second);
			return pData;
		}
		return NULL;
	}

	sBulletData* InsertBulletData( DWORD dwItemCode, DWORD dwCount )
	{
		sBulletData sData;
		sData.dwItemCode = dwItemCode;
		sData.dwCurCount = 0;
		sData.dwMaxCount = dwCount;
		m_mapBulletDatas.insert( std::map<DWORD,sBulletData>::value_type(dwItemCode,sData) );
		return GetBulletData(dwItemCode);
	}

	void ClearBulletData() { m_mapBulletDatas.clear(); }

	//////////////////////////////////////////////////////////////////////////
	

	void		 Init()
	{
		m_vecRUDP.clear();
		m_vecRUDPErase.clear();
		m_vecRUDPTest.clear();
		m_vecRUDPNPC.clear();
		m_vecRUDPEraseNPC.clear();
		m_vecRUDPTestNPC.clear();

		m_vecAntiData.clear();

		m_dwReqCount = m_dwAckCount = m_dwAckCountDupl = m_dwErrCount = m_dwLossCount = m_dwEraseCount = 0;
		for( int i = 0; i < 4; ++i )
		{
			m_dwSkillUseTime[i] = 0;
			m_dwSkillUseTime2[i] = 0;
			m_dwLatedTime[i] = 0;
		}

		m_iRecoverSkillSize = 0;
		m_iTeamType = 0;
		m_dwPenaltySkill = m_dwPenaltyCount = m_dwLastPenaltyTime = m_dwLastSkillPenaltyTime = 0;
		m_bRecoverSkill = m_bRecoverExtraSkill = false;
		m_dwRecoverTime = m_dwRecoverExtraTime = 0;
		m_dwAntiHitCount = m_dwAntiSkillCount = m_dwAntiHitCount2 = m_dwAntiSkillCount2 = 0;

		m_mapBulletDatas.clear();
	}
	UserData()
	{
		m_vecAntiData.reserve( 100 );
	};

};
struct RelayGroup			
{
	typedef std::vector<UserData> RelayGroups;
	int		m_iCoolType;	//��ų ��Ÿ�ӿ�( �ɼǿ� ���� ������ )
	int		m_iModeType;
	int		m_iRoomStyle;
	DWORD   m_dwRoomIndex;
	RelayGroups m_RelayUserList;

	int		GetModeType() { return m_iModeType; }
	int		GetModeStyle() { return m_iRoomStyle; }

	int		m_iWinCntRed;
	int		m_iWinCntBlue;

	int		m_iScoreRed;
	int		m_iScoreBlue;
	void	IncTeamScoreRed(){ m_iScoreRed++; }
	void	IncTeamScoreBlue(){ m_iScoreBlue++; }

	float	m_fTeamBalanceRed;		//���ο�
	float	m_fTeamBalanceBlue;

	float	m_fWinBalanceRed;		//�����ھ�
	float	m_fWinBalanceBlue;

	float	m_fModeBalanceRed;		//��庰��..
	float	m_fModeBalanceBlue;

	float GetTeamBalanceRed(){ return m_fTeamBalanceRed; }
	float GetTeamBalanceBlue(){ return m_fTeamBalanceBlue; }

	float GetWinBalanceRed(){ return m_fWinBalanceRed; }
	float GetWinBalanceBlue(){ return m_fWinBalanceBlue; }

	float GetModeBalanceRed(){ return m_fModeBalanceRed; }
	float GetModeBalanceBlue(){ return m_fModeBalanceBlue; }


	bool m_bWriteLog;
	void SetWriteLog( bool bWrite ) { m_bWriteLog = bWrite; }
	bool IsWriteLog(){ return m_bWriteLog; }

	//anti hack ���� �ð��κ��� �갡..



	struct sAntiHackCheckTimeData
	{
		DWORD dwUser1;
		DWORD dwUser2;
		DWORD dwCheckTime;
		sAntiHackCheckTimeData() : dwUser1(0), dwUser2(0), dwCheckTime(0) {}
	};

	std::vector<sAntiHackCheckTimeData> m_vecAntiHackTime;

	bool IsAntiHackCheckTime( DWORD dwUser1, DWORD dwUser2, DWORD dwCurTime )
	{
		DWORD dwFinduser1 = dwUser1 < dwUser2 ? dwUser1 : dwUser2;
		DWORD dwFinduser2 = dwUser1 > dwUser2 ? dwUser1 : dwUser2;

		int nSize = m_vecAntiHackTime.size();
		for( int i = 0; i < nSize; ++i )
		{
			if( m_vecAntiHackTime[i].dwUser1 == dwFinduser1 &&
				m_vecAntiHackTime[i].dwUser2 == dwFinduser2 )
			{

				if( (dwCurTime-m_vecAntiHackTime[i].dwCheckTime) > 1000 )
				{
					m_vecAntiHackTime[i].dwCheckTime = dwCurTime;
					return true;
				}

			}
		}

		//������
		sAntiHackCheckTimeData antiHackCheckTimeData;
		antiHackCheckTimeData.dwUser1 = dwFinduser1;
		antiHackCheckTimeData.dwUser2 = dwFinduser2;
		antiHackCheckTimeData.dwCheckTime = dwCurTime;

		m_vecAntiHackTime.push_back( antiHackCheckTimeData );
		
		return false;
	}


public:
	void InitData()
	{
		m_iCoolType = 4;
		m_iModeType = m_iRoomStyle = 0;
		m_dwRoomIndex = 0;
		m_iWinCntRed = m_iWinCntBlue = m_iScoreRed = m_iScoreBlue = 0;
		m_fTeamBalanceRed = m_fTeamBalanceBlue = m_fWinBalanceRed = m_fWinBalanceBlue = 1.f;
		m_fModeBalanceRed = m_fModeBalanceBlue = 1.f;
		m_RelayUserList.clear();

		m_vecAntiHackTime.clear();
	}

	void CalculateTeamBalance()
	{
		int nSize = m_RelayUserList.size();
		int BlueCnt = 0, RedCnt = 0;
		for( int i = 0; i < nSize; ++i )
		{
			if( m_RelayUserList[i].m_iTeamType == TEAM_RED )
				RedCnt++;
			else if( m_RelayUserList[i].m_iTeamType == TEAM_BLUE )
				BlueCnt++;
		}
		if( RedCnt !=0 && BlueCnt != 0 )
		{
			m_fTeamBalanceRed = TeamBalanceCalcForGauge( RedCnt, BlueCnt, m_iModeType, m_iModeType );
			m_fTeamBalanceBlue = TeamBalanceCalcForGauge( BlueCnt, RedCnt, m_iModeType, m_iModeType );
		}
	}

	void CalculateTeamWinCnt()
	{
		auto func = []( int iOwnerTeamCnt, int iEnemyTeamCnt, int iModeType ) -> float
		{
			if( iModeType == MT_SYMBOL || iModeType == MT_KING )
			{
				if( iEnemyTeamCnt-iOwnerTeamCnt <= 0 )
					return 1.f;
				return 1.2f;
			}
			int iGap = iEnemyTeamCnt-iOwnerTeamCnt;
			if( iGap <= 0 )
				return 1.f;
			else if ( iGap == 1 )
				return 1.1f;
			return 1.2f;
		};

		m_fWinBalanceRed = func( m_iWinCntRed, m_iWinCntBlue, m_iModeType );
		m_fWinBalanceBlue = func( m_iWinCntBlue, m_iWinCntRed, m_iModeType );
	}

	void CalculateTeamScore()
	{
		//�����ٶ� ����ũ���!
		if( m_iModeType == MT_TEAM_SURVIVAL || m_iModeType == MT_DOBULE_CROWN )
		{
			int iTeamCntRed = 0, iTeamCntBlue = 0;
			for( DWORD i = 0; i < m_RelayUserList.size(); ++i )
			{
				if( m_RelayUserList[i].m_iTeamType == TEAM_RED )
					iTeamCntRed++;
				else if( m_RelayUserList[i].m_iTeamType == TEAM_BLUE )
					iTeamCntBlue++;
			}

			// �ϵ��ڵ�!! 2.f �� win_score_constant �� ������̴�. Ư�� ��� ini�� ���� �Ǿ� ����.(��� �� �ٲ��..)
			float fScoreKillPointRed = iTeamCntRed * 2.5f * 100.f;
			float fScoreKillPointBlue = iTeamCntBlue * 2.5f * 100.f;

			auto funcKillRate = []( float fKillPoint, float fScoreKillPoint ) -> float
			{
				if( fScoreKillPoint <= 0 )
					return 0.f;
				if( fKillPoint > 0 )
				{
					float fRate = fKillPoint / fScoreKillPoint;
					return min( fRate , 1.f );
				}

				return 0.f;
			};

			float fKillRateRed = funcKillRate( float(m_iScoreRed*100), fScoreKillPointRed );
			float fKillRateBlue = funcKillRate( float(m_iScoreBlue*100), fScoreKillPointBlue );

			auto funcScoreRate = []( float fOwnerTeamRate, float fEnemyTeamRate ) -> float
			{
				float fScoreGap = fEnemyTeamRate - fOwnerTeamRate;
				fScoreGap *= 100.f;

				float fResult;
				if( fScoreGap <= 0 )
					fResult = 1.f;
				else if( fScoreGap < 10.f )
					fResult = 1.1f;
				else
					fResult = 1.2f;

				return fResult;
			};

			m_fModeBalanceRed = funcScoreRate( fKillRateRed, fKillRateBlue );
			m_fModeBalanceBlue = funcScoreRate( fKillRateBlue, fKillRateRed );
		}
		// ����Ʈ Ŭ��
		else if( m_iModeType == MT_FIGHT_CLUB )
		{
			// �̰� ���߿�...

		}
	}


	// 	void GetUserLists(RelayGroups& relayGroup) 
	// 	{
	// 		std::copy(m_RelayUserList.begin(),m_RelayUserList.end(),std::back_inserter(relayGroup));
	// 	}
	RelayGroups* GetUsers() { return &m_RelayUserList; }

	void AddUser(const DWORD dwUserIndex, const int iPort, const char* szIP, DWORD dwUserSeed, DWORD dwNPCSeed, int iTeamType )
	{
		for(unsigned int i=0; i< m_RelayUserList.size() ; i++)
		{
			UserData& rkUser = m_RelayUserList.at(i);
			if( rkUser.m_dwUserIndex == dwUserIndex )
			{
					// ���� ���� ���� ( update �� ��� rudp ���� ���
					DWORD dwTotalCnt = rkUser.m_dwUserSeed - rkUser.m_dwUserSeedOri;
					dwTotalCnt += rkUser.m_dwNPCSeed - rkUser.m_dwNPCSeedOri;
					if( dwTotalCnt != 0 )
					{
// 					CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] RudpInfo uid:%u, RoomIndex:%u, modeType:%d, total:%u, req:%u, ack:%u, dup:%u, err:%u, loss:%u, del:%u",
// 						dwUserIndex, m_dwRoomIndex, m_iModeType, dwTotalCnt,
// 						rkUser.m_dwReqCount, rkUser.m_dwAckCount, rkUser.m_dwAckCountDupl, rkUser.m_dwErrCount, rkUser.m_dwLossCount, rkUser.m_dwEraseCount );
					}

				/*if( IsWriteLog() )
					CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] AddUser Update - Uidx(%u), Seed(%u/%u)", dwUserIndex, dwUserSeed, dwNPCSeed );*/

				rkUser.Init();
				rkUser.m_iClientPort = iPort;
				rkUser.m_dwUserSeed = dwUserSeed;
				rkUser.m_dwNPCSeed = dwNPCSeed;
				rkUser.m_dwUserSeedOri = dwUserSeed;
				rkUser.m_dwNPCSeedOri = dwNPCSeed;
				rkUser.m_iTeamType = iTeamType;

				strcpy_s(rkUser.m_szPublicIP, szIP);

				CalculateTeamBalance();
				return;
			}
		}

		UserData kUserData;
		kUserData.Init();

		kUserData.m_dwUserIndex = dwUserIndex;
		kUserData.m_iClientPort = iPort;
		kUserData.m_dwUserSeed = dwUserSeed;
		kUserData.m_dwNPCSeed = dwNPCSeed;
		kUserData.m_dwUserSeedOri = dwUserSeed;
		kUserData.m_dwNPCSeedOri = dwNPCSeed;
		kUserData.m_iTeamType = iTeamType;
		strcpy_s(kUserData.m_szPublicIP, szIP);		

		m_RelayUserList.push_back(kUserData);
		CalculateTeamBalance();

		/*if( IsWriteLog() )
		{
			CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] AddUser Insert - Uidx(%u), Seed(%u/%u)", dwUserIndex, dwUserSeed, dwNPCSeed );
		}*/
		}
	void RemoveUser(const DWORD dwUserIndex)
	{
		/*if( IsWriteLog() )
		{
			CheatLOG.PrintTimeAndLog( 0, "[ANTILOG] RemoveUser - Uidx(%u)", dwUserIndex );
		}*/

		for(unsigned int i=0; i< m_RelayUserList.size() ; i++)
		{
			UserData& rkUser = m_RelayUserList.at(i);
			if( rkUser.m_dwUserIndex == dwUserIndex )
			{
				m_RelayUserList.erase(m_RelayUserList.begin() + i);
				CalculateTeamBalance();
				break;
			}
		}
	}
	int GetUserCount()
	{
		return m_RelayUserList.size();
	}

	UserData* GetUserData( DWORD dwUserIndex )
	{
		for( DWORD i = 0; i < m_RelayUserList.size(); i++ )
		{
			if( m_RelayUserList[i].m_dwUserIndex == dwUserIndex )
				return &m_RelayUserList[i];
		}
		return NULL;
	}
	float TeamBalanceCalcForGauge( int iTeamCnt, int iEnemyTeamCnt, int iModeType, int iModeStyle )
	{
		float fResult = iEnemyTeamCnt / iTeamCnt;

		if( fResult < 1.f )
			fResult = 1.f;

		if ( fResult == 1.f )
		{
			if( iModeStyle == 0 )
				fResult *= 10.f;
			else if( iModeStyle == 4 )
				fResult *= 2.f;

			return fResult;
		}

		if( iModeType == 1 )
			fResult *= 2.f;
		else if( iModeType == 2 )
			fResult *= 1.1f;
		else if( iModeType == 3 )
			fResult *= 2.f;
		else if( iModeType == 6 )
			fResult *= 1.1f;
		else if( iModeType == 9 )
			fResult *= 2.f;

		return fResult;
	}

	void InitMode()
	{
		//���� �� ����
		if( m_iModeType == 4 || m_iModeType == 13)
		{
			m_fModeBalanceRed = m_fModeBalanceBlue = 2.0f;
		}

		//����Ʈ Ŭ���� ����
		else if( m_iModeType == 15 )
		{
			// �ִ� 1.3 * 1.3 ��  ������. �׷��� �ϴ� �ƽøذ����� �ӽ��۾�
			m_fModeBalanceRed = m_fModeBalanceBlue = 1.69f;
		}
		else
		{
			m_fTeamBalanceRed = m_fTeamBalanceBlue = 1.f;
			m_fWinBalanceRed = m_fWinBalanceBlue = 1.f;
			m_fModeBalanceRed = m_fModeBalanceBlue = 1.f;
		}
	}

	RelayGroup() : m_bWriteLog(false){}
};
#endif
//////////////////////////////////////////////////////////////////////////


struct SendRelayInsertData : RelayHeader // ������ �������� ������ ���� ����  
{
	int          m_modeType;
	DWORD		 m_dwRoomIndex;
	DWORD		 m_dwUserIndex;
	char         m_szPublicIP[STR_IP_MAX];
	int          m_iClientPort;
	char         m_szPublicID[ID_NUM_PLUS_ONE];
};
struct RemoveData :RelayHeader
{
	DWORD m_dwRoomIndex;
	DWORD m_dwUserIndex;
};

#pragma pack(push,1)
struct SendRelayInfo_
{
	TCHAR m_ipAddr[STR_IP_MAX];
	int m_port;
	int  m_userCount;
	int  m_roomCount;
	int  m_serverCount;
	int m_64DropCount;
	int m_256DropCount;
	int m_1024DropCount;
	int m_64UsingCount;
	int m_256UsingCount;
	int m_1024UsingCount;
	SendRelayInfo_()
	{
		ZeroMemory(m_ipAddr,STR_IP_MAX);
		m_port = 0; 
		m_userCount = 0;
		m_roomCount= 0;
		m_64DropCount= 0;
		m_256DropCount= 0;
		m_1024DropCount= 0;
		m_64UsingCount= 0;
		m_256UsingCount= 0;
		m_1024UsingCount= 0;
	}
};
#pragma pack(pop)

/************************************************************************/
/* Singleton                                                                     */
/************************************************************************/
typedef cSingleton<GameSvrUDPModule> S_UDPModule;
#define g_UDPModule (*S_UDPModule::GetInstance())

typedef cSingleton<GameSvrUDPNode> S_UDPNode;
#define g_UDPNode (*S_UDPNode::GetInstance())

typedef cSingleton<ioBroadCastRelayModule> S_RELAY;
#define g_Relay (*S_RELAY::GetInstance())

typedef cSingleton<IPBlockerManager> S_BLOCK;
#define g_IPBlock (*S_BLOCK::GetInstance())

typedef cSingleton<WordFilterManager> S_WordFilter;
#define g_WordFilter (*S_WordFilter::GetInstance())

typedef cSingleton<ioLanguages> S_Languages;
#define g_Languages (*S_Languages::GetInstance())
#endif

#define PACKET_GUARD_VOID(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return; } }
#define PACKET_GUARD_INT(x)		{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return -1; } }
#define PACKET_GUARD_BOOL(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return FALSE; } }
#define PACKET_GUARD_bool(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return false; } }
#define PACKET_GUARD_BREAK(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); break; } }
#define PACKET_GUARD_BOOL(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return FALSE; } }
#define PACKET_GUARD_NULL(x)	{ BOOL rtval = x; if( (rtval)==FALSE ) { LOG.PrintTimeAndLog(0,"[PACKET_GUARRD_ERROR] %s::%s",__FUNCTION__,#x); return NULL; } }

//x ���ڰ� mV maxValue �� 
#define MAX_GUARD(x, mV) if(x > mV || x < 0) { LOG.PrintTimeAndLog(0,"[error][limit][CHECK_MAX_VALUE_ERROR] %s::value:%d max:%d",__FUNCTION__,x, mV); x = 0; }

// template <typename T>
// typedef std::function<T(bool)> PACKET_FUNTION;

 