#include <QJsonArray>
#include <QDir>
#include "Parsers/JSONFile.hpp"
#include "Windows/Locale.hpp"
#include "Logger.hpp"

#include "ProgramConstants.hpp"

ProgramConstants::ProgramConstants()
{
}

void ProgramConstants::InitializeFileSettings() { pSettingsFile = std::make_unique<Settings>(); }

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
