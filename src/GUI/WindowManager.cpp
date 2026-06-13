#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>

#include "../Extensions/L10NExt.hpp"
#include "../Extensions/ExceptionExt.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Windows/Registry.hpp"
#include "../Windows/Locale.hpp"
#include "../Managers/FactionsManager.hpp"
#include "../Core/Logger.hpp"
#include "../Core/Convert.hpp"

#include "ImageManager.hpp"
#include "WindowManager.hpp"

WindowManager::WindowManager()
{
    QString style = StringExt::EmptyString;
    
    SetTranslator();

    qApp->setWindowIcon(QIcon(QPixmap::fromImage(ImageManager::DecodeEditorWebpIcon())));
    
    //  Apply general style
    LOGMSG("Loading \"" + PROGRAM_CONSTANTS->MAIN_STYLES_FILE + "\"...");
    QFile cssGLobal(PROGRAM_CONSTANTS->MAIN_STYLES_FILE);
    if (cssGLobal.open(QIODevice::ReadOnly))
    {
        style += cssGLobal.readAll();
        LOGMSG("Main styles sheet has been loaded");
    }
    else
    {
        LOGMSG("Unable to read the main style file");
    }

    style += "\n";

    // Apply ZH style as default
    LOGMSG("Loading \"" + PROGRAM_CONSTANTS->GZH_PROFILE_FOLDER + "/Theme/" + PROGRAM_CONSTANTS->STYLES_FILENAME + "\"...");
    QFile cssGZH(PROGRAM_CONSTANTS->GZH_PROFILE_FOLDER + "/Theme/" + PROGRAM_CONSTANTS->STYLES_FILENAME);
    if (cssGZH.open(QIODevice::ReadOnly))
    {
        style += cssGZH.readAll();
        LOGMSG("GeneralsZH styles sheet has been loaded");
    }
    else
    {
        LOGMSG("Unable to read the GeneralsZH style file");
    }

    qApp->setStyleSheet(style);

    LOGMSG("Loading launch window...");
    pStartUpWindow = new SetUpWindowsWrapper();
    pStartUpWindow->setWindowTitle(strWindowName);
    LOGMSG("Launch window has been loaded");
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
    catch(const ExceptionExt& e)
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
    {
        FACTIONS_MANAGER->UpdateFactionNames();
    }
}

void WindowManager::Show()                               { pStartUpWindow->show(); }
void WindowManager::SetCSFFilePath(const QString& input) { strCSFFilePath = input; }

void WindowManager::EditorWindow_NewHotkeyFileSelected()
{
    CSF_PARSER = nullptr;
    pHotkeysEditor->close();
    pHotkeysEditor = nullptr;
    InitializeCSFParser();
    StartUpWindow_AcceptConfiguration();
}

void WindowManager::StartUpWindow_GProfileSelected()
{
    if (FACTIONS_MANAGER != nullptr)
    {
        FACTIONS_MANAGER.release();
    }

    FACTIONS_MANAGER = std::make_unique<FactionManager>(PROGRAM_CONSTANTS->G_PROFILE_FOLDER
                                                        + "/" + PROGRAM_CONSTANTS->TECH_TREE_FILENAME);
}

void WindowManager::StartUpWindow_GZHProfileSelected()
{
    if (FACTIONS_MANAGER != nullptr)
    {
        FACTIONS_MANAGER.release();
    }

    FACTIONS_MANAGER = std::make_unique<FactionManager>(PROGRAM_CONSTANTS->GZH_PROFILE_FOLDER
                                                        + "/" + PROGRAM_CONSTANTS->TECH_TREE_FILENAME);
}

void WindowManager::StartUpWindow_CustomProfileSelected(const QString& folder)
{
    if (FACTIONS_MANAGER != nullptr)
    {
        FACTIONS_MANAGER.release();
    }
    
    FACTIONS_MANAGER = std::make_unique<FactionManager>(PROGRAM_CONSTANTS->PROFILES_FOLDER 
                                                        + "/" + folder + "/" + PROGRAM_CONSTANTS->TECH_TREE_FILENAME);
}


WindowManager::~WindowManager()
{
    if (pHotkeysEditor != nullptr) pHotkeysEditor->deleteLater();
    if (pStartUpWindow != nullptr) pStartUpWindow->deleteLater();
    if (pAppTranslator != nullptr) pAppTranslator->deleteLater();
}
