#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>

#include "../Extensions/L10NExt.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Windows/Registry.hpp"
#include "../Windows/Locale.hpp"
#include "../FactionsManager.hpp"
#include "../DiscordManager.hpp"
#include "../Logger.hpp"
#include "../Convert.hpp"
#include "../Exception.hpp"
#include "../SteamManager.hpp"

#include "ImageManager.hpp"
#include "WindowManager.hpp"

WindowManager::WindowManager()
{    
    SetTranslator();
    UpdateWindowTitle();

    qApp->setWindowIcon(QIcon(QPixmap::fromImage(ImageManager::DecodeEditorWebpIcon())));
    ApplyTheme();

    LOGMSG("Loading launch window...");
    pStartUpWindow = new SetUpWindowsWrapper();
    pStartUpWindow->setWindowTitle(strWindowName);
    LOGMSG("Launch window has been loaded");
}

bool WindowManager::ApplyTheme(const Profile* profile)
{
    LOGMSG("Loading \"" + PROGRAM_CONSTANTS->STYLES_SHEET_FILE + "\"...");

    QFile baseCss{PROGRAM_CONSTANTS->STYLES_SHEET_FILE};
    if (!baseCss.open(QIODevice::ReadOnly))
    {
        LOGMSG("Unable to read the style file");
        return false;
    }

    QString styleSheet = QString::fromUtf8(baseCss.readAll());
    baseCss.close();

    if (profile != nullptr)
    {
        const QString overridePath = profile->GetThemeStylesPath();

        if (QFile::exists(overridePath))
        {
            LOGMSG("Loading profile theme overrides \"" + overridePath + "\"...");

            QFile overridesCss{overridePath};
            if (overridesCss.open(QIODevice::ReadOnly))
            {
                QString overrides = QString::fromUtf8(overridesCss.readAll());
                overridesCss.close();

                overrides.replace("@PROFILE_THEME_ROOT@", QDir::fromNativeSeparators(profile->GetThemeFolderPath()));
                styleSheet += "\n\n" + overrides;
            }
            else
            {
                LOGMSG("Unable to read profile theme override file");
            }
        }
    }

    qApp->setStyleSheet(styleSheet);
    LOGMSG("Styles sheet has been applied");
    return true;
}

bool WindowManager::ApplyTheme()
{
    if (PROGRAM_CONSTANTS->HasActiveProfile())
        return ApplyTheme(&PROGRAM_CONSTANTS->GetActiveProfile());

    return ApplyTheme(nullptr);
}

bool WindowManager::PreviewThemeForProfile(const QString& profileId)
{
    for (const auto& profile : PROGRAM_CONSTANTS->Profiles)
    {
        if (profile.GetId().compare(profileId, Qt::CaseInsensitive) != 0)
            continue;

        return ApplyTheme(&profile);
    }

    return ApplyTheme();
}

bool WindowManager::InitializeCSFParser()
{
    if (strCSFFilePath == "")
    {
        QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER),
                                       L10N(PROGRAM_CONSTANTS->CSF_EMPTY_STRING_ERROR));
        return false;
    }

    if (!QFile::exists(strCSFFilePath))
    {
        QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER),
                                       L10N(PROGRAM_CONSTANTS->CSF_DOESNT_EXIST_ERROR));
        return false;
    }

    if (CSF_PARSER != nullptr)
    {
        LOGMSG("CSF parser has been initialized. Re-initializing existing parser.");
        CSF_PARSER = nullptr; // It will cause a bug, if it will be used twice.
    }
    
    try
    {
        CSF_PARSER = std::make_unique<CSFParser>(strCSFFilePath);
    }
    catch(const Exception& e)
    {
        QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER),
                                       e.what());
        return false;
    }

    if (!CSF_PARSER->ExistCategory(PROGRAM_CONSTANTS->HOTKEY_CSF_CATEGORY))
    {
        QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER), 
                                       L10N(PROGRAM_CONSTANTS->CSF_NO_CTLBAR_ERROR));
        return false;
    }

    if (!CSF_PARSER->ExistCategory(PROGRAM_CONSTANTS->OBJECT_CSF_CATEGORY))
    {
        QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER), 
                                       L10N(PROGRAM_CONSTANTS->CSF_NO_OBJECT_ERROR));
        return false;
    }

    return true;
}

void WindowManager::StartUpWindow_AcceptConfiguration()
{
    // 2nd init protection 
    if (pHotkeysEditor != nullptr)
        return;

    if (!InitializeCSFParser())
        return;

    if (PROGRAM_CONSTANTS->HasActiveProfile())
    {
        FACTIONS_MANAGER = std::make_unique<FactionManager>();
        SetTranslator();
        UpdateWindowTitle();
        ApplyTheme();
    }

    if (PROGRAM_CONSTANTS->pSettingsFile->IsSteamIntegrationEnabled())
        STEAM_MANAGER->Initialize();

    if (PROGRAM_CONSTANTS->pSettingsFile->IsDiscordRPCEnabled())
        DISCORD_MANAGER->Initialize();

    pStartUpWindow->hide();

    LOGMSG("Loading editor window...");
    pHotkeysEditor = new EditorWindowWrapper();
    pHotkeysEditor->setWindowTitle(strWindowName);
    pHotkeysEditor->show();
    LOGMSG("Editor window has been loaded");
}

void WindowManager::SetTranslator()
{
    size_t language;

    if (pAppTranslator != nullptr) 
        qApp->removeTranslator(pAppTranslator);

    language = PROGRAM_CONSTANTS->pSettingsFile->GetLanguage();

    QString lngShortName = PROGRAM_CONSTANTS->Languages.value(language).first;
    LOGMSG("Set editor language to " + lngShortName.toUpper());

    pAppTranslator = new QTranslator();
    pAppTranslator->load(lngShortName, PROGRAM_CONSTANTS->TRANSLATIONS_FOLDER);
    qApp->installTranslator(pAppTranslator);

    if (FACTIONS_MANAGER != nullptr)
        FACTIONS_MANAGER->UpdateFactionNames();
}

void WindowManager::UpdateWindowTitle()
{
    if (PROGRAM_CONSTANTS->HasActiveProfile())
        strWindowName = PROGRAM_CONSTANTS->GetActiveProfile().GetWindowTitle();
    else
        strWindowName = PROGRAM_CONSTANTS->COMMON_TITLE;

    if (pStartUpWindow != nullptr)
        pStartUpWindow->setWindowTitle(strWindowName);

    if (pHotkeysEditor != nullptr)
        pHotkeysEditor->setWindowTitle(strWindowName);
}

void WindowManager::Show()                               { pStartUpWindow->show(); }
void WindowManager::SetCSFFilePath(const QString& input) { strCSFFilePath = input; }

void WindowManager::EditorWindow_NewHotkeyFileSelected()
{
    CSF_PARSER = nullptr;
    pHotkeysEditor->close();
    pHotkeysEditor = nullptr;
    StartUpWindow_AcceptConfiguration();
}

WindowManager::~WindowManager()
{
    if (pHotkeysEditor != nullptr) pHotkeysEditor->deleteLater();
    if (pStartUpWindow != nullptr) pStartUpWindow->deleteLater();
    if (pAppTranslator != nullptr) pAppTranslator->deleteLater();
}
