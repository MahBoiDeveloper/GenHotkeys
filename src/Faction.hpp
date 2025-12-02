#pragma once
#include <QMap>
#include <QVector>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include "ProgramConstants.hpp"

class Faction
{
public: // Types
    struct Action
    {
        QString iconName;
        QString hotkeyString;
    };

    struct GameObject
    {
        QString iconName;
        QString ingameName;
        QVector<QVector<Action>> keyboardLayouts;
    };

private: // Data
    QString shortName;
    QString displayName;
    QString displayNameDescription;
    QMap<GameObject, GameObjectTypes> techTree;
    QMap<size_t, QPair<QString, QString>> localizedDisplay;
public:
    inline static const int BASIC_FACTION_COUNT = 12;

private: // Methods
    QMap<GameObject, GameObjectTypes> ParseJsonObject(const QJsonObject& obj);
public:
    explicit Faction();
    explicit Faction(const QString& _shortName, const QString& _displayName, const QString& _displayNameDescription);
    explicit Faction(const QJsonObject& factionAsObject);

    /// @brief Returns short faction name from field `ShortName` of TechTree.json.
    const QString GetShortName() const;
    /// @brief Returns another one short faction name from field `DisplayName` of TechTree.json.
    const QString GetDisplayName() const;
    /// @brief Returns another one short faction name from field `DisplayName` of TechTree.json for the specific locale.
    const QString GetDisplayName(size_t lng) const;
    /// @brief Returns long faction name from field `DisplayNameDescription` of TechTree.json.
    const QString GetDisplayNameDescription() const;
    /// @brief Returns long faction name from field `DisplayNameDescription` of TechTree.json for the specific locale.
    const QString GetDisplayNameDescription(size_t lng) const;
    /// @brief Returns link to the techTree field.
    const QMap<GameObject, GameObjectTypes>& GetTechTree() const;
    /// @brief Returns link to the keyboard layout vector searching by object name.
    const QVector<QVector<Action>> GetKeyboardLayoutsByObjectName(const QString& objName) const;
    /// @brief Replace hotkey with new one.
    void SetHotkey(const QString& goName, const QString& actName, const QString& hk);
};

bool operator < (Faction::GameObject a, Faction::GameObject b);
