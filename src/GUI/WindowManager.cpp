#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>

#include "../Parsers/CSFParser.hpp"
#include "../Windows/Registry.hpp"
#include "../Windows/Locale.hpp"
#include "../Logger.hpp"
#include "../Convert.hpp"
#include "../Exception.hpp"

#include "ImageManager.hpp"
#include "WindowManager.hpp"

WindowManager::WindowManager()
{    
    SetTranslator();

    qApp->setWindowIcon(QIcon(QPixmap::fromImage(ImageManager::DecodeEditorWebpIcon())));
    
    LOGMSG("Loading \"" + PROGRAM_CONSTANTS->STYLES_SHEET_FILE + "\"...");
    QFile css{PROGRAM_CONSTANTS->STYLES_SHEET_FILE};
    if (css.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(css.readAll());
        css.close();
        LOGMSG("Styles sheet has been loaded");
    }
    else
    {
        LOGMSG("Unable to read the style file");
    }

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

    LOGMSG("Loading editor window...");
    pHotkeysEditor = new EditorWindowWrapper();
    pHotkeysEditor->setWindowTitle(strWindowName);
    pHotkeysEditor->show();
    pStartUpWindow->close();
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

WindowManager::~WindowManager()
{
    if (pHotkeysEditor != nullptr) pHotkeysEditor->deleteLater();
    if (pStartUpWindow != nullptr) pStartUpWindow->deleteLater();
    if (pAppTranslator != nullptr) pAppTranslator->deleteLater();
}
