#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include "Parsers/JSONFile.hpp"
#include "Windows/Locale.hpp"
#include "Logger.hpp"

#include "ProgramConstants.hpp"

ProgramConstants::ProgramConstants()
{
}

void ProgramConstants::InitializeFileSettings() { pSettingsFile = std::make_unique<Settings>(); }

void ProgramConstants::InitializeProfiles()
{
    QDir profilesDir(PROFILES_FOLDER);
    Profiles.clear();
    pActiveProfile = nullptr;

    if (!profilesDir.exists())
        return;

    for (const auto& dirInfo : profilesDir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot))
    {
        const QString profileFile = dirInfo.absoluteFilePath() + "/Profile.json";

        if (!QFileInfo::exists(profileFile))
            continue;

        Profiles.push_back(Profile{dirInfo.absoluteFilePath()});
    }

    std::sort(Profiles.begin(), Profiles.end(), [](const Profile& left, const Profile& right)
    {
        if (left.GetSortOrder() != right.GetSortOrder())
            return left.GetSortOrder() < right.GetSortOrder();

        return QString::compare(left.GetDisplayName(), right.GetDisplayName(), Qt::CaseInsensitive) < 0;
    });

    if (!Profiles.isEmpty())
        SetActiveProfile(Profiles.first().GetId());
}

void ProgramConstants::InitializeTranslations()
{
    QDir transDir(TRANSLATIONS_FOLDER);
    int i = 0;
    Languages.insert(DEFAULT_LANGUAGE_CODE, {"en", "English"});

    if (transDir.isEmpty())
    {
        LOGMSG(TRANSLATIONS_FOLDER + " folder is empty. Only English translation available!");
        return;
    }

    for (const auto& file : transDir.entryInfoList(QStringList( "*.qm" ), QDir::Filter::Files))
    {
        ++i;
        LOGMSG("Listed translation: {" + file.completeBaseName().toLower() + ", " + Windows::Locale::LanguageName(file.completeBaseName().toLower()) + "}");
        Languages.insert(i, {file.completeBaseName().toLower(), Windows::Locale::LanguageName(file.completeBaseName().toLower())});
    }
}

bool ProgramConstants::SetActiveProfile(const QString& profileId)
{
    for (const auto& profile : Profiles)
    {
        if (profile.GetId().compare(profileId, Qt::CaseInsensitive) != 0)
            continue;

        pActiveProfile = std::make_unique<Profile>(profile);
        return true;
    }

    return false;
}

bool ProgramConstants::HasActiveProfile() const
{
    return pActiveProfile != nullptr;
}

const Profile& ProgramConstants::GetActiveProfile() const
{
    return *pActiveProfile;
}
