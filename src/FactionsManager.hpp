#pragma once
#include <memory>
#include <QVector>
#include "Parsers/JSONFile.hpp"
#include "Faction.hpp"

#define FACTIONS_MANAGER FactionManager::Instance

class FactionManager final
{
private: // Data
    const JSONFile TECH_TREE_SOURCE{PROGRAM_CONSTANTS->TECH_TREE_FILE};
    QVector<Faction> vFactions;
public:
    inline static std::unique_ptr<FactionManager> Instance = nullptr;

private: // Methods
    void Parse();
public:
    /// @brief Default constructor.
    explicit FactionManager();
    /// @brief Return count of parsed factions.
    int Count();
    /// @brief Return faction by its index in container.
    const Faction& FindByIndex(int position);
    /// @brief Return faction reference.
    const Faction& FindByShortName(const QString& name);
};
