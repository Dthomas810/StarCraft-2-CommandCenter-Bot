#include "StrategyManager.h"
#include "CCBot.h"
#include "JSONTools.h"
#include "Util.h"
#include "MetaType.h"

Strategy::Strategy()
{

}

Strategy::Strategy(const std::string & name, const CCRace & race, const BuildOrder & buildOrder, const Condition & scoutCondition, const Condition & attackCondition)
    : m_name            (name)
    , m_race            (race)
    , m_buildOrder      (buildOrder)
    , m_wins            (0)
    , m_losses          (0)
    , m_scoutCondition  (scoutCondition)
    , m_attackCondition (attackCondition)
{

}

// constructor
StrategyManager::StrategyManager(CCBot & bot)
    : m_bot(bot)
	, marinesToProduce (3)
	, needMoreProdcution (true)
{

}

void StrategyManager::onStart()
{
    readStrategyFile(m_bot.Config().ConfigFileLocation);
}

void StrategyManager::onFrame()
{

}

const Strategy & StrategyManager::getCurrentStrategy() const
{
    auto strategy = m_strategies.find(m_bot.Config().StrategyName);

    BOT_ASSERT(strategy != m_strategies.end(), "Couldn't find Strategy corresponding to strategy name: %s", m_bot.Config().StrategyName.c_str());

    return (*strategy).second;
}

const BuildOrder & StrategyManager::getOpeningBookBuildOrder() const
{
    return getCurrentStrategy().m_buildOrder;
}

bool StrategyManager::scoutConditionIsMet() const
{
    return getCurrentStrategy().m_scoutCondition.eval();
}

bool StrategyManager::attackConditionIsMet() const
{
    return getCurrentStrategy().m_attackCondition.eval();
}


bool StrategyManager::needMoreRax() const {
	if (m_bot.GetMinerals() > 600) return true;

	return false;

}

bool StrategyManager::shouldExpandNow() const
{
	

	if (m_bot.GetMinerals() > 400) {
		return true;

	}

	return false;
}

void StrategyManager::addStrategy(const std::string & name, const Strategy & strategy)
{
    m_strategies[name] = strategy;
}

const UnitPairVector StrategyManager::getBuildOrderGoal() const
{
	CCRace race = m_bot.GetPlayerRace(Players::Self);

	if (Util::IsTerran(race)) return StrategyManager::getTerranBuildOrderGoal();

	else if(Util::IsProtoss(race)) return StrategyManager::getProtossBuildOrderGoal();

	else if (Util::IsZerg(race)) return StrategyManager::getZergBuildOrderGoal();


    return std::vector<UnitPair>();
}

const UnitPairVector StrategyManager::getTerranBuildOrderGoal() const
{
	std::vector<UnitPair> goal;
	//int numCC = m_bot.UnitInfo().getUnitTypeCount(Players::Self, UnitType(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND) );
	int numCC = m_bot.UnitInfo().getUnitTypeCount(Players::Self, UnitType(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER, m_bot), true);
	std::cout << "CC's = " + std::to_string(numCC) << std::endl;
	int numWorkers = m_bot.UnitInfo().getUnitTypeCount(Players::Self, UnitType(sc2::UNIT_TYPEID::TERRAN_SCV, m_bot), true);
	std::cout << "Workers's = " + std::to_string(numWorkers) << std::endl;
	int numMarines = m_bot.UnitInfo().getUnitTypeCount(Players::Self, UnitType(sc2::UNIT_TYPEID::TERRAN_MARINE, m_bot), true);
	std::cout << "Marines = " + std::to_string(numMarines)  << std::endl;
	/*int numHellions = m_bot.UnitInfo().getUnitTypeCount(Players::Self, UnitType(sc2::UNIT_TYPEID::TERRAN_HELLION, m_bot), true);
	std::cout << "Hellions = " + std::to_string(numHellions) << std::endl;*/

	// If we're under saturation, want to keep making workers until we meet it (16 per mineral line)
	//we'll only work towards saturating 3 bases for now, optimal economy is ~3-4 mineral lines
	//At that point we move workers to new mineral lines instead of creating new workers, that'll be done in a different manager (saturationManager if it doesn't already exist)
	if (numWorkers < numCC * 17 && numCC<4) {
		goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_SCV, m_bot), numCC));
	}


	//Assuming we're doing a marine rush, we want to make sure we meet the attack condition even if botconfig won't get us there
	//TODO : Grab the unit and amount from attack condition(s) and check that we are working towards them
	int Mneeded = 40 - numMarines;
	if (Mneeded  > 0) {

		//TODO : Need to know how many of the production buildings  we have so we know how many units we can produce without over queueing
		// Hardcode for now
		goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_MARINE, m_bot), marinesToProduce));
	}

	if (needMoreRax() && needMoreProdcution) {
		goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_BARRACKS, m_bot), 3));
		
	}

	//int Hneeded = 3 - numHellions;
	//if (Mneeded  > 0) {
	//	goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_HELLION, m_bot), 1));
	//}
	

	boolean flag = true; 
	//TODO :  saturation to be managed
	if (shouldExpandNow() && flag) {
		std::cout << "Trying to expand now from bo goal" << std::endl;
		goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER, m_bot), 1));
		//goal.push_back(std::pair<UnitType, int>(UnitType(sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND, m_bot), 1));
		
	}



	//TODO : if currently doing certain strategy, aim for certain composition

	return goal;
}

const UnitPairVector StrategyManager::getProtossBuildOrderGoal() const
{
    return std::vector<UnitPair>();
}


const UnitPairVector StrategyManager::getZergBuildOrderGoal() const
{
    return std::vector<UnitPair>();
}


void StrategyManager::onEnd(const bool isWinner)
{
	
}

void StrategyManager::readStrategyFile(const std::string & filename)
{
    CCRace race = m_bot.GetPlayerRace(Players::Self);
    std::string ourRace = Util::GetStringFromRace(race);

    std::ifstream file(filename);
    json j;
    file >> j;

#ifdef SC2API
    const char * strategyObject = "SC2API Strategy";
#else
    const char * strategyObject = "BWAPI Strategy";
#endif

    // Parse the Strategy Options
    if (j.count(strategyObject) && j[strategyObject].is_object())
    {
        const json & strategy = j[strategyObject];

        // read in the various strategic elements
        JSONTools::ReadBool("ScoutHarassEnemy", strategy, m_bot.Config().ScoutHarassEnemy);
        JSONTools::ReadString("ReadDirectory", strategy, m_bot.Config().ReadDir);
        JSONTools::ReadString("WriteDirectory", strategy, m_bot.Config().WriteDir);

        // if we have set a strategy for the current race, use it
        if (strategy.count(ourRace.c_str()) && strategy[ourRace.c_str()].is_string())
        {
            m_bot.Config().StrategyName = strategy[ourRace.c_str()].get<std::string>();
        }

        // check if we are using an enemy specific strategy
        JSONTools::ReadBool("UseEnemySpecificStrategy", strategy, m_bot.Config().UseEnemySpecificStrategy);
        if (m_bot.Config().UseEnemySpecificStrategy && strategy.count("EnemySpecificStrategy") && strategy["EnemySpecificStrategy"].is_object())
        {
            // TODO: Figure out enemy name
            const std::string enemyName = "ENEMY NAME";
            const json & specific = strategy["EnemySpecificStrategy"];

            // check to see if our current enemy name is listed anywhere in the specific strategies
            if (specific.count(enemyName.c_str()) && specific[enemyName.c_str()].is_object())
            {
                const json & enemyStrategies = specific[enemyName.c_str()];

                // if that enemy has a strategy listed for our current race, use it
                if (enemyStrategies.count(ourRace.c_str()) && enemyStrategies[ourRace.c_str()].is_string())
                {
                    m_bot.Config().StrategyName = enemyStrategies[ourRace.c_str()].get<std::string>();
                    m_bot.Config().FoundEnemySpecificStrategy = true;
                }
            }
        }

        // Parse all the Strategies
        if (strategy.count("Strategies") && strategy["Strategies"].is_object())
        {
            const json & strategies = strategy["Strategies"];
            for (auto it = strategies.begin(); it != strategies.end(); ++it) 
            {
                const std::string & name = it.key();
                const json & val = it.value();              
                
                BOT_ASSERT(val.count("Race") && val["Race"].is_string(), "Strategy is missing a Race string");
                CCRace strategyRace = Util::GetRaceFromString(val["Race"].get<std::string>());
                
                BOT_ASSERT(val.count("OpeningBuildOrder") && val["OpeningBuildOrder"].is_array(), "Strategy is missing an OpeningBuildOrder arrau");
                BuildOrder buildOrder;
                const json & build = val["OpeningBuildOrder"];
                for (size_t b(0); b < build.size(); b++)
                {
                    if (build[b].is_string())
                    {
                        MetaType MetaType(build[b].get<std::string>(), m_bot);
                        buildOrder.add(MetaType);
                    }
                    else
                    {
                        BOT_ASSERT(false, "Build order item must be a string %s", name.c_str());
                        continue;
                    }
                }

                BOT_ASSERT(val.count("ScoutCondition") && val["ScoutCondition"].is_array(), "Strategy is missing a ScoutCondition array");
                Condition scoutCondition(val["ScoutCondition"], m_bot);
                
                BOT_ASSERT(val.count("AttackCondition") && val["AttackCondition"].is_array(), "Strategy is missing an AttackCondition array");
                Condition attackCondition(val["AttackCondition"], m_bot);

                addStrategy(name, Strategy(name, strategyRace, buildOrder, scoutCondition, attackCondition));
            }
        }
    }
}
