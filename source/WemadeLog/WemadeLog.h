#pragma once

enum WemadeLogTypes
{
	WE_LOG_BEGIN = 0,

	WE_LOG_CONCURRENT = 101,

	WE_LOG_DATA_PCROOM = 201,
	WE_LOG_DATA_PIECE_OBTAIN = 202,
	WE_LOG_DATA_PIECE_MIX = 203,
	WE_LOG_DATA_PIECE_DIVIDE = 204,
	WE_LOG_BUY_ITEM_GOLD = 205,
	WE_LOG_DATA_MEDAL = 206,
	WE_LOG_DATA_MEDAL_EXTEND = 207,
	WE_LOG_DATA_TRADE = 208,
	WE_LOG_DATA_LOCALINFO = 209,
	WE_LOG_DATA_TUTORIAL = 210,

	WE_LOG_BUY_ITEM_CLASS = 301,
	WE_LOG_BUY_ITEM_DECORATION = 302,
	WE_LOG_BUY_ITEM_EQUIP = 303,
	WE_LOG_DATA_LEAGUE_PRESENT = 304,
	WE_LOG_USE_ITEM = 305,

	WE_LOG_BUY_ITEM_SPECIAL = 401,
	WE_LOG_DATA_CLOVER = 402,
	WE_LOG_DATA_PLAY = 403,
	WE_LOG_DATA_TIME = 404,

	WE_LOG_DATA_CHARACTER = 501,
	WE_LOG_DATA_PESO = 502,
	WE_LOG_DATA_QUEST = 503,
	WE_LOG_PRESENT = 504,
	WE_LOG_SUBSCRIPTION = 505,

	WE_LOG_DATA_PET	= 601,
	WE_LOG_DATA_UPGRADE = 602,
	WE_LOG_DATA_CHAR_AWAKE = 603,
	WE_LOG_DATA_COSTUME	= 604,

	WE_LOG_GAME_LOG	=  701,

	WE_LOG_OVSEAS	= 901,

	WE_LOG_END = 1000
};
