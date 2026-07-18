#pragma once
#include <memory>
#include <QVector>
#include "../Parsers/JSONFile.hpp"
#include "../Core/Faction.hpp"

#define FACTIONS_MANAGER FactionManager::Instance

class FactionManager final
{
private: // Data
    JSONFile TechTreeSource;
    QVector<Faction> vFactions;
public:
    inline static std::unique_ptr<FactionManager> Instance = nullptr;

private: // Methods
    void Parse();
public:
    /// @brief Parse `TechTree.json` from the specific path as the game tech tree.
    explicit FactionManager(const QString techtreejson);
    /// @brief Return count of parsed factions.
    int Count();
    /// @brief Return faction by its index in container.
    const Faction& FindByIndex(int position);
    /// @brief Return faction reference.
    const Faction& FindByShortName(const QString& name);
    /// @brief Updates faction names to the current locale.
    void UpdateFactionNames();
};
