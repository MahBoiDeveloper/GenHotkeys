#include <QDir>
#include <QJsonArray>

#include "Parsers/JSONFile.hpp"
#include "Profile.hpp"

using namespace Windows;

std::optional<Registry::Games> Profile::ParseRegistryGame(const QString& value)
{
    const QString normalized = value.trimmed();

    if (normalized.compare("Generals", Qt::CaseInsensitive) == 0)
        return Registry::Games::Generals;

    if (normalized.compare("GeneralsZeroHour", Qt::CaseInsensitive) == 0)
        return Registry::Games::GeneralsZeroHour;

    return std::nullopt;
}

Profile::Profile(const QString& profileFolderPath)
    : folderPath{QDir::fromNativeSeparators(profileFolderPath)}
{
    const JSONFile json{folderPath + "/Profile.json"};
    const QJsonObject root = json.GetMainObject();

    id               = root.value("Id").toString();
    displayName      = root.value("DisplayName").toString();
    windowTitle      = root.value("WindowTitle").toString(displayName);
    shortWindowTitle = root.value("ShortWindowTitle").toString(windowTitle);
    techTreeFileName = root.value("TechTreeFile").toString("TechTree.json");
    csfRelativePath  = root.value("CSFRelativePath").toString("Data\\English\\generals.csf");
    steamAppId       = static_cast<size_t>(root.value("SteamAppID").toInt());
    sortOrder        = root.value("SortOrder").toInt();
    registryGame     = ParseRegistryGame(root.value("RegistryGame").toString());

    for (const auto& groupValue : root.value("FactionGroups").toArray())
    {
        QVector<QString> group;

        for (const auto& factionValue : groupValue.toArray())
            group.push_back(factionValue.toString());

        if (!group.isEmpty())
            factionGroups.push_back(group);
    }
}

const QString& Profile::GetId() const { return id; }
const QString& Profile::GetDisplayName() const { return displayName; }
const QString& Profile::GetWindowTitle() const { return windowTitle; }
const QString& Profile::GetShortWindowTitle() const { return shortWindowTitle; }
const QString& Profile::GetCSFRelativePath() const { return csfRelativePath; }
const QVector<QVector<QString>>& Profile::GetFactionGroups() const { return factionGroups; }
size_t Profile::GetSteamAppID() const { return steamAppId; }
int Profile::GetSortOrder() const { return sortOrder; }

bool Profile::HasRegistryGame() const { return registryGame.has_value(); }
Registry::Games Profile::GetRegistryGame() const { return registryGame.value_or(Registry::Games::GeneralsZeroHour); }

QString Profile::GetFolderPath() const { return folderPath; }
QString Profile::GetTechTreePath() const { return folderPath + "/" + techTreeFileName; }
QString Profile::GetThemeFolderPath() const { return folderPath + "/Theme"; }
QString Profile::GetThemeStylesPath() const { return GetThemeFolderPath() + "/Styles.css"; }

QString Profile::GetInstalledCSFPath() const
{
    if (!HasRegistryGame())
        return QString();

    const QString gamePath = QString::fromStdWString(Registry::GetPathToGame(GetRegistryGame()));

    if (gamePath.isEmpty())
        return QString();

    return QDir::fromNativeSeparators(gamePath) + "/" + QDir::fromNativeSeparators(csfRelativePath);
}
