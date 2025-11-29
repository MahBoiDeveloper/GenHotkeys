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
    QDir transDir(PROGRAM_CONSTANTS->TRANSLATIONS_FOLDER);
    
    if (transDir.isEmpty())
    {
        LOGMSG(PROGRAM_CONSTANTS->TRANSLATIONS_FOLDER + " " + "is empty!");
        return;
    }

    auto files = transDir.entryInfoList(QDir::Filter::Files);
    _Languages.insert(0, {"en", "English"});
    int i = 0;
    for (const auto& file : files)
    {
        ++i;
        LOGMSG("Filtered file: " + file.fileName());
        if (file.completeSuffix() == "qm")
        {
            LOGMSG("Inserting: {" + file.completeBaseName() + ", " + Windows::Locale::LanguageName(file.completeBaseName()) + "}");
            _Languages.insert(i, {file.completeBaseName(), Windows::Locale::LanguageName(file.completeBaseName())});
        }
    }
}
