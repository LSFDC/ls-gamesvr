

#include "stdafx.h"

#include "ModeCreator.h"

#include "Mode.h"
#include "SymbolMode.h"
#include "HiddenkingMode.h"
#include "TrainingMode.h"
#include "SurvivalMode.h"
#include "HeroMatchMode.h"
#include "TeamSurvivalMode.h"
#include "BossMode.h"
#include "FootballMode.h"
#include "MonsterSurvivalMode.h"
#include "GangsiMode.h"
#include "DungeonAMode.h"
#include "HeadquartersMode.h"
#include "CatchRunningManMode.h"
#include "FightClubMode.h"
#include "TowerDefMode.h"
#include "DoubleCrownMode.h"
#include "ShuffleBonusMode.h"
#include "TeamSurvivalAIMode.h"
#include "HouseMode.h"
#include "UnderwearMode.h"
#include "CBTMode.h"
#include "RaidMode.h"

Mode* ModeCreator::CreateMode( Room *pCreator, ModeType eMode )
{
	Mode *pMode = NULL;
	switch( eMode )
	{
	case MT_SYMBOL:
		pMode = new SymbolMode( pCreator );
		break;
	case MT_UNDERWEAR:
		pMode = new UnderwearMode( pCreator );
		break;
	case MT_CBT:
		pMode = new CBTMode( pCreator );
		break;
	case MT_CATCH:
		pMode = new CatchMode( pCreator );
		break;
	case MT_KING:
		pMode = new HiddenkingMode( pCreator );
		break;
	case MT_TRAINING:
		pMode = new TrainingMode( pCreator );
		break;
	case MT_SURVIVAL:
		pMode = new SurvivalMode( pCreator );
		break;
	case MT_TEAM_SURVIVAL:
		pMode = new TeamSurvivalMode( pCreator );
		break;
	case MT_TEAM_SURVIVAL_AI:
		pMode = new TeamSurvivalAIMode( pCreator );
		break;
	case MT_BOSS: 
		pMode = new BossMode( pCreator );
		break;
	case MT_MONSTER_SURVIVAL: 
		pMode = new MonsterSurvivalMode( pCreator );
		break;
	case MT_FOOTBALL:
		pMode = new FootballMode( pCreator );
		break;
	case MT_HEROMATCH:
		pMode = new HeroMatchMode( pCreator );
		break;
	case MT_GANGSI:
		pMode = new GangsiMode( pCreator );
		break;
	case MT_DUNGEON_A:
		pMode = new DungeonAMode( pCreator );
		break;
	case MT_HEADQUARTERS:
		pMode = new HeadquartersMode( pCreator );
		break;
	case MT_CATCH_RUNNINGMAN:
		pMode = new CatchRunningManMode( pCreator );
		break;
	case MT_FIGHT_CLUB:
		pMode = new FightClubMode( pCreator );
		break;
	case MT_TOWER_DEFENSE:
	case MT_DARK_XMAS:
	case MT_FIRE_TEMPLE:
	case MT_FACTORY:
		pMode = new CTowerDefMode( pCreator, eMode );
		break;
	case MT_DOBULE_CROWN:
		pMode = new DoubleCrownMode( pCreator );
		break;
	case MT_SHUFFLE_BONUS:
		pMode = new ShuffleBonusMode( pCreator );
		break;
	case MT_HOUSE:
		pMode = new HouseMode( pCreator );
		break;
	case MT_RAID: 
		pMode = new RaidMode( pCreator, eMode );
		break;
	}	

	if( !pMode )
	{
		LOG.PrintTimeAndLog( LOG_DEBUG_LEVEL, "ModeCreator::CreateMode - %d Unknown ModeType", eMode );
		pMode = new SymbolMode( pCreator );	// юс╫ц
	}

	return pMode;
}
