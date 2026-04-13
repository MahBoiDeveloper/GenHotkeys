#include <QDir>
#include <QLineEdit>
#include <QMessageBox>

#include "../Extensions/L10NExt.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Windows/Registry.hpp"
#include "../Logger.hpp"
#include "../Exception.hpp"
#include "WindowManager.hpp"
#include "ImageManager.hpp"
#include "LoadFromTheGameWindow.hpp"
#include "SetUpWindowsWrapper.hpp"

SetUpWindowsWrapper::SetUpWindowsWrapper(QWidget* parent) : QStackedWidget(parent)
{
    // Makes window unresizeable and equal to the size of the background
    setFixedSize(PROGRAM_CONSTANTS->SET_UP_WINDOW_SIZE);
    setWindowFlags(windowFlags() |  Qt::MSWindowsFixedSizeDialogHint);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint &
                                   ~Qt::WindowMinimizeButtonHint);

    AddWidgets();
    setCurrentWidget(pGreetingWidget);
    AttachConnections();
}

void SetUpWindowsWrapper::AttachConnections()
{
    connect(pSettingsWindow,        &SettingsWindow::languageChanged,
            this,                   &SetUpWindowsWrapper::SettingsWindow_LanguageChanged);

    connect(pGreetingWidget,        &GreetingWindow::btnLoadFromFileClicked,
            this,                   &SetUpWindowsWrapper::BtnLoadFromFile_Clicked);
    
    connect(pGreetingWidget,        &GreetingWindow::btnLoadFromGameClicked,
            this,                   &SetUpWindowsWrapper::BtnLoadFromGame_Clicked);

    connect(pGreetingWidget,        &GreetingWindow::btnSettingsClicked,
            this,                   &SetUpWindowsWrapper::BtnSettings_Clicked);

    connect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::btnBackClicked,
            this,                   &SetUpWindowsWrapper::BtnBack_Clicked);

    connect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::selectedProfileChanged,
            this,                   &SetUpWindowsWrapper::LoadWindow_ProfileChanged);
    
    connect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::btnStartClicked,
            this,                   &SetUpWindowsWrapper::LoadFromTheFileWindow_AcceptConfiguration);

    connect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::btnBackClicked,
            this,                   &SetUpWindowsWrapper::BtnBack_Clicked);

    connect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::selectedProfileChanged,
            this,                   &SetUpWindowsWrapper::LoadWindow_ProfileChanged);
    
    connect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::btnStartClicked,
            this,                   &SetUpWindowsWrapper::LoadFromTheGameWindow_AcceptConfiguration);

    connect(pSettingsWindow,        &SettingsWindow::btnBackClicked,
            this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
}

void SetUpWindowsWrapper::DetachConnections()
{
    disconnect(pSettingsWindow,        &SettingsWindow::languageChanged,
               this,                   &SetUpWindowsWrapper::SettingsWindow_LanguageChanged);

    disconnect(pGreetingWidget,        &GreetingWindow::btnLoadFromFileClicked,
               this,                   &SetUpWindowsWrapper::BtnLoadFromFile_Clicked);
        
    disconnect(pGreetingWidget,        &GreetingWindow::btnLoadFromGameClicked,
               this,                   &SetUpWindowsWrapper::BtnLoadFromGame_Clicked);

    disconnect(pGreetingWidget,        &GreetingWindow::btnSettingsClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
    
    disconnect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::btnBackClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);

    disconnect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::selectedProfileChanged,
               this,                   &SetUpWindowsWrapper::LoadWindow_ProfileChanged);

    disconnect(pLoadFromTheFileWindow, &LoadFromTheFileWindow::btnStartClicked,
               this,                   &SetUpWindowsWrapper::LoadFromTheFileWindow_AcceptConfiguration);

    disconnect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::btnBackClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);

    disconnect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::selectedProfileChanged,
               this,                   &SetUpWindowsWrapper::LoadWindow_ProfileChanged);

    disconnect(pLoadFromTheGameWindow, &LoadFromTheGameWindow::btnStartClicked,
               this,                   &SetUpWindowsWrapper::LoadFromTheGameWindow_AcceptConfiguration);

    disconnect(pSettingsWindow,        &SettingsWindow::btnBackClicked,
               this,                   &SetUpWindowsWrapper::BtnBack_Clicked);
}

void SetUpWindowsWrapper::AddWidgets()
{
    pGreetingWidget        = new GreetingWindow();
    pLoadFromTheGameWindow = new LoadFromTheGameWindow();
    pLoadFromTheFileWindow = new LoadFromTheFileWindow();
    pSettingsWindow        = new SettingsWindow();

    pGreetingWidget->setFixedSize(size());
    pLoadFromTheGameWindow->setFixedSize(size());
    pLoadFromTheFileWindow->setFixedSize(size());
    pSettingsWindow->setFixedSize(size());

    addWidget(pGreetingWidget);
    addWidget(pLoadFromTheGameWindow);
    addWidget(pLoadFromTheFileWindow);
    addWidget(pSettingsWindow);
}

void SetUpWindowsWrapper::SettingsWindow_LanguageChanged()
{
    WINDOW_MANAGER->SetTranslator();

    DetachConnections();
    pGreetingWidget->deleteLater();
    pLoadFromTheGameWindow->deleteLater();
    pLoadFromTheFileWindow->deleteLater();
    pSettingsWindow->deleteLater();

    AddWidgets();
    AttachConnections();
    setCurrentWidget(pSettingsWindow);
}

void SetUpWindowsWrapper::BtnLoadFromFile_Clicked() { setCurrentWidget(pLoadFromTheFileWindow); }
void SetUpWindowsWrapper::BtnLoadFromGame_Clicked() { setCurrentWidget(pLoadFromTheGameWindow); }
void SetUpWindowsWrapper::BtnSettings_Clicked()     { setCurrentWidget(pSettingsWindow); }
void SetUpWindowsWrapper::BtnBack_Clicked()
{
    WINDOW_MANAGER->SetCSFFilePath("");
    WINDOW_MANAGER->ApplyTheme();
    setCurrentWidget(pGreetingWidget);
}

void SetUpWindowsWrapper::LoadWindow_ProfileChanged(const QString& profileId)
{
    WINDOW_MANAGER->PreviewThemeForProfile(profileId);
}

void SetUpWindowsWrapper::LoadFromTheGameWindow_AcceptConfiguration()
{
    const QString profileId = pLoadFromTheGameWindow->GetSelectedProfileId();

    if (!PROGRAM_CONSTANTS->SetActiveProfile(profileId))
        return;

    const Profile& activeProfile = PROGRAM_CONSTANTS->GetActiveProfile();
    const QString gamePath = QString::fromStdWString(Windows::Registry::GetPathToGame(activeProfile.GetRegistryGame()));
    const QString pathDataEngGenCsf = activeProfile.GetInstalledCSFPath();
    bool hasFoundStringTable = false;

    WINDOW_MANAGER->SetCSFFilePath("");

    if (gamePath.isEmpty())
    {
        QMessageBox::critical(nullptr,
                              L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER),
                              L10N(PROGRAM_CONSTANTS->GAME_PATH_NOT_FOUND).arg(activeProfile.GetDisplayName()));
        return;
    }

    if (QFile::exists(pathDataEngGenCsf))
    {
        WINDOW_MANAGER->SetCSFFilePath(pathDataEngGenCsf);
        hasFoundStringTable = true;
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
                hasFoundStringTable = true;
                break;
            }
            catch(...)
            {
                LOGMSG("String table doesn't exist in archive");
                continue;
            }
        }
    }

    if (!hasFoundStringTable)
    {
        QMessageBox::critical(nullptr,
                              L10N(PROGRAM_CONSTANTS->CSF_ERROR_HEADER),
                              L10N(PROGRAM_CONSTANTS->CSF_EMPTY_DATA_ENGLISH).arg(QDir::toNativeSeparators(gamePath)));
        return;
    }
    
    WINDOW_MANAGER->StartUpWindow_AcceptConfiguration();
}

void SetUpWindowsWrapper::LoadFromTheFileWindow_AcceptConfiguration() 
{
    if (!PROGRAM_CONSTANTS->SetActiveProfile(pLoadFromTheFileWindow->GetSelectedProfileId()))
        return;

    WINDOW_MANAGER->SetCSFFilePath(pLoadFromTheFileWindow->findChild<QLineEdit*>("lneFilePath", Qt::FindChildrenRecursively)->text());
    WINDOW_MANAGER->StartUpWindow_AcceptConfiguration();
}
