#include <QDir>
#include <QLineEdit>
#include <QMessageBox>

#include "../Extensions/ExceptionExt.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Windows/Registry.hpp"
#include "../Core/Logger.hpp"
#include "ImageManager.hpp"
#include "WindowManager.hpp"
#include "SetUpWindowsWrapper.hpp"

SetUpWindowsWrapper::SetUpWindowsWrapper(QWidget* parent) : QStackedWidget(parent)
{
    // Makes window unresizeable and equal to the size of the background
    setFixedSize(PROGRAM_CONSTANTS->SET_UP_WINDOW_SIZE);
    setWindowFlags(windowFlags() |  Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint &
                                   ~Qt::WindowMinimizeButtonHint);

    AddWidgets();
    setCurrentWidget(pSelectProfileWindow);
    AttachConnections();
}

void SetUpWindowsWrapper::AttachConnections()
{
    connect(pSelectProfileWindow,   &SelectProfileWindow::gProfileSelected, 
            this,                   &SetUpWindowsWrapper::SelectProfileWindow_GProfileSelected);
    connect(pSelectProfileWindow,   &SelectProfileWindow::gzhProfileSelected,
            this,                   &SetUpWindowsWrapper::SelectProfileWindow_GZHProfileSelected);
    connect(pSelectProfileWindow,   &SelectProfileWindow::customProfileSelected, 
            this,                   &SetUpWindowsWrapper::SelectProfileWindow_CustomProfileSelected);

    connect(pLoadWindow, &LoadWindow::btnBackClicked,
            this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
    connect(pLoadWindow, &LoadWindow::btnStartClicked,
            this,                   &SetUpWindowsWrapper::LoadWindow_LoadFromTheFile);
    connect(pLoadWindow, &LoadWindow::btnFromGameClicked,
            this,                   &SetUpWindowsWrapper::LoadWindow_LoadFromTheGame);

    connect(pSettingsWindow,        &SettingsWindow::languageChanged,
            this,                   &SetUpWindowsWrapper::SettingsWindow_LanguageChanged);
    connect(pSettingsWindow,        &SettingsWindow::btnBackClicked,
            this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
}

void SetUpWindowsWrapper::DetachConnections()
{
    disconnect(pSelectProfileWindow,   &SelectProfileWindow::gProfileSelected, 
               this,                   &SetUpWindowsWrapper::SelectProfileWindow_GProfileSelected);
    disconnect(pSelectProfileWindow,   &SelectProfileWindow::gzhProfileSelected,
               this,                   &SetUpWindowsWrapper::SelectProfileWindow_GZHProfileSelected);
    disconnect(pSelectProfileWindow,   &SelectProfileWindow::customProfileSelected, 
               this,                   &SetUpWindowsWrapper::SelectProfileWindow_CustomProfileSelected);

    disconnect(pLoadWindow, &LoadWindow::btnBackClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
    disconnect(pLoadWindow, &LoadWindow::btnStartClicked,
               this,                   &SetUpWindowsWrapper::LoadWindow_LoadFromTheFile);
    disconnect(pLoadWindow, &LoadWindow::btnFromGameClicked,
               this,                   &SetUpWindowsWrapper::LoadWindow_LoadFromTheGame);

    disconnect(pSettingsWindow,        &SettingsWindow::languageChanged,
               this,                   &SetUpWindowsWrapper::SettingsWindow_LanguageChanged);
    disconnect(pSettingsWindow,        &SettingsWindow::btnBackClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
}

void SetUpWindowsWrapper::AddWidgets()
{
    pSelectProfileWindow   = new SelectProfileWindow();
    pLoadWindow = new LoadWindow();
    pSettingsWindow        = new SettingsWindow();

    pSelectProfileWindow->setFixedSize(size());
    pLoadWindow->setFixedSize(size());
    pSettingsWindow->setFixedSize(size());

    addWidget(pSelectProfileWindow);
    addWidget(pLoadWindow);
    addWidget(pSettingsWindow);
}

void SetUpWindowsWrapper::DeleteWidgets()
{
    pSelectProfileWindow->deleteLater();
    pLoadWindow->deleteLater();
    pSettingsWindow->deleteLater();
}

void SetUpWindowsWrapper::SettingsWindow_LanguageChanged()
{
    WINDOW_MANAGER->SetTranslator();
    DetachConnections();
    DeleteWidgets();
    AddWidgets();
    AttachConnections();
    setCurrentWidget(pSettingsWindow);
}

void SetUpWindowsWrapper::BtnLoadFromFile_Clicked() { setCurrentWidget(pLoadWindow); }
void SetUpWindowsWrapper::BtnSettings_Clicked()     { setCurrentWidget(pSettingsWindow); }
void SetUpWindowsWrapper::BtnBack_Clicked()
{
    setCurrentWidget(pSelectProfileWindow);
    WINDOW_MANAGER->SetCSFFilePath("");
    WINDOW_MANAGER->ApplyDefaultProfileStyleSheet();
}

void SetUpWindowsWrapper::SelectProfileWindow_GProfileSelected()                           { WINDOW_MANAGER->StartUpWindow_GProfileSelected(); BtnLoadFromFile_Clicked(); }
void SetUpWindowsWrapper::SelectProfileWindow_GZHProfileSelected()                         { WINDOW_MANAGER->StartUpWindow_GZHProfileSelected(); BtnLoadFromFile_Clicked(); }
void SetUpWindowsWrapper::SelectProfileWindow_CustomProfileSelected(const QString& folder) { WINDOW_MANAGER->StartUpWindow_CustomProfileSelected(folder); BtnLoadFromFile_Clicked(); }

void SetUpWindowsWrapper::LoadWindow_LoadFromTheGame()
{
    const QString gamePath = WINDOW_MANAGER->ProfileFolder.split('/')[2] == PROGRAM_CONSTANTS->G_FOLDER_NAME
        ? QString::fromStdWString(Windows::Registry::GetPathToGame(Windows::Registry::Games::Generals))
        : QString::fromStdWString(Windows::Registry::GetPathToGame(Windows::Registry::Games::GeneralsZeroHour));
    
    const QString pathDataEngGenCsf = gamePath + "Data\\English\\generals.csf";

    if (QFile::exists(pathDataEngGenCsf))
    {
        WINDOW_MANAGER->SetCSFFilePath(pathDataEngGenCsf);
    }
    else
    {
        LOGMSG("Unable to find " + pathDataEngGenCsf);
        LOGMSG("Searching generals.csf in all archives");

        QDir gameDir(gamePath);
        QFileInfoList fileList = gameDir.entryInfoList(QDir::Filter::Files
                                                       | QDir::Filter::NoDotAndDotDot
                                                       | QDir::Filter::Readable,
                                                       QDir::SortFlag::Name);
        for(const QFileInfo& elem : fileList)
        {
            if (elem.suffix().toUpper() != "BIG")
                continue;

            LOGMSG("Processing " + elem.absoluteFilePath() + "...");
            try
            {
                CSFParser parser(elem.absoluteFilePath());
                WINDOW_MANAGER->SetCSFFilePath(elem.absoluteFilePath());
                break;
            }
            catch(...)
            {
                LOGMSG("String table doesn't exist in archive");
                continue;
            }
        }
    }
    
    WINDOW_MANAGER->StartUpWindow_AcceptConfiguration();
}

void SetUpWindowsWrapper::LoadWindow_LoadFromTheFile() 
{
    WINDOW_MANAGER->SetCSFFilePath(pLoadWindow->findChild<QLineEdit*>("lneFilePath", Qt::FindChildrenRecursively)->text());
    WINDOW_MANAGER->StartUpWindow_AcceptConfiguration();
}
