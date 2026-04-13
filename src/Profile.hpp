#pragma once
#include <optional>
#include <QVector>
#include <QString>

#include "Windows/Registry.hpp"

class Profile final
{
private:
    QString                                 folderPath;
    QString                                 id;
    QString                                 displayName;
    QString                                 windowTitle;
    QString                                 shortWindowTitle;
    QString                                 techTreeFileName;
    QString                                 csfRelativePath;
    std::optional<Windows::Registry::Games> registryGame;
    QVector<QVector<QString>>               factionGroups;
    size_t                                  steamAppId = 0;
    int                                     sortOrder = 0;

    static std::optional<Windows::Registry::Games> ParseRegistryGame(const QString& value);

public:
    Profile() = default;
    explicit Profile(const QString& profileFolderPath);

    const QString& GetId() const;
    const QString& GetDisplayName() const;
    const QString& GetWindowTitle() const;
    const QString& GetShortWindowTitle() const;
    const QString& GetCSFRelativePath() const;
    const QVector<QVector<QString>>& GetFactionGroups() const;
    size_t GetSteamAppID() const;
    int GetSortOrder() const;

    bool HasRegistryGame() const;
    Windows::Registry::Games GetRegistryGame() const;

    QString GetFolderPath() const;
    QString GetTechTreePath() const;
    QString GetThemeFolderPath() const;
    QString GetThemeOverridesPath() const;
    QString GetInstalledCSFPath() const;
};
