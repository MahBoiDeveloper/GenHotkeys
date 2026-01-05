#include "Logger.hpp"
#include "FactionsManager.hpp"

FactionManager::FactionManager()
{
    vFactions.reserve(12);

    for(const auto& elem : TECH_TREE_SOURCE.Query("$.TechTree").toArray())
        vFactions.push_back(Faction{elem.toObject()});
}

int FactionManager::Count() { return vFactions.size(); }
const Faction& FactionManager::FindByIndex(int position) { return vFactions.at(position); }
const Faction& FactionManager::FindByShortName(const QString& name)
{
    int tmp = 0;

    for(int i = 0; i < vFactions.count(); i++)
    {
        const Faction& elem = vFactions[i];
        
        if(elem.GetShortName() != name)
            continue;

        tmp = i;
        break;
    }
    
    return vFactions.at(tmp);
}
