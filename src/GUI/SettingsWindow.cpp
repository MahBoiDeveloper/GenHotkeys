#include <windows.h> // Allows disable console
#include <QMessageBox>

#include "../Logger.hpp"
#include "../ProgramConstants.hpp"
#include "../Windows/Locale.hpp"
#include "WindowManager.hpp"
#include "SettingsWindow.hpp"

SettingsWindow::SettingsWindow(QWidget* parent) : QWidget(parent)
{
    ltMain                          = new QVBoxLayout();
    ltButtons                       = new QHBoxLayout();
    ltSettings                      = new QHBoxLayout();
    ltLeftColumn                    = new QVBoxLayout();
    ltRightColumn                   = new QVBoxLayout();
    ltLanguage                      = new QHBoxLayout();
    btnBack                         = new QPushButton();
    btnSave                         = new QPushButton();
    btnResetAll                     = new QPushButton();
    chkEnableDebugConsole           = new QCheckBox();
    chkEnableDiscordRPC             = new QCheckBox();
    chkForceSystemLanguageOnStartUp = new QCheckBox();
    lblLanguage                     = new QLabel();
    cmbLanguage                     = new QComboBox();

    chkEnableDebugConsole->setText(tr("Enable debug console"));
    chkEnableDebugConsole->setObjectName(nameof(chkEnableDebugConsole));
    if (PROGRAM_CONSTANTS->pSettingsFile->IsConsoleEnabled())
        chkEnableDebugConsole->setCheckState(Qt::CheckState::Checked);
    else
        chkEnableDebugConsole->setCheckState(Qt::CheckState::Unchecked);

    chkEnableDiscordRPC->setText(tr("Enable Discord RPC (WIP)"));
    chkEnableDiscordRPC->setObjectName(nameof(chkEnableDiscordRPC));
    if (PROGRAM_CONSTANTS->pSettingsFile->IsDiscordRPCEnabled())
        chkEnableDiscordRPC->setCheckState(Qt::CheckState::Checked);
    else
        chkEnableDiscordRPC->setCheckState(Qt::CheckState::Unchecked);

    chkForceSystemLanguageOnStartUp->setText(tr("Force editor to use system language on start up"));
    chkForceSystemLanguageOnStartUp->setObjectName(nameof(chkForceSystemLanguageOnStartUp));
    if (PROGRAM_CONSTANTS->pSettingsFile->IsForceSystemLanguageOnStartUpEnabled())
        chkForceSystemLanguageOnStartUp->setCheckState(Qt::CheckState::Checked);
    else
        chkForceSystemLanguageOnStartUp->setCheckState(Qt::CheckState::Unchecked);

    lblLanguage->setText(tr("Language:"));
    lblLanguage->setObjectName(nameof(lblLanguage));
    
    for (size_t i : PROGRAM_CONSTANTS->Languages.keys())
        cmbLanguage->addItem(PROGRAM_CONSTANTS->Languages.value(i).second);

    cmbLanguage->setCurrentIndex(static_cast<int>(PROGRAM_CONSTANTS->pSettingsFile->GetLanguage()));
    cmbLanguage->setCurrentText(PROGRAM_CONSTANTS->Languages.value(PROGRAM_CONSTANTS->pSettingsFile->GetLanguage()).second);

    ltLanguage->setAlignment(Qt::AlignLeading);
    ltLanguage->addWidget(lblLanguage);
    ltLanguage->addWidget(cmbLanguage);

    ltLeftColumn->addWidget(chkEnableDebugConsole);
    ltLeftColumn->addWidget(chkEnableDiscordRPC);
    ltLeftColumn->addWidget(chkForceSystemLanguageOnStartUp);
    ltLeftColumn->addLayout(ltLanguage);

    btnSave->setText(tr("SAVE"));
    btnSave->setObjectName(nameof(btnSave));
    connect(btnSave, &QPushButton::clicked, this, &SettingsWindow::BtnSave_Clicked);

    btnResetAll->setText(tr("RESET ALL"));
    btnResetAll->setObjectName(nameof(btnResetAll));
    connect(btnResetAll, &QPushButton::clicked, this, &SettingsWindow::BtnResetAll_Clicked);

    btnBack->setText(tr("BACK"));
    btnBack->setObjectName(nameof(btnBack));
    connect(btnBack, &QPushButton::clicked, this, &SettingsWindow::btnBackClicked);

    ltSettings->addLayout(ltLeftColumn);
    ltSettings->addLayout(ltRightColumn);

    ltButtons->addWidget(btnSave);
    ltButtons->addWidget(btnResetAll);
    ltButtons->addWidget(btnBack);

    ltMain->setAlignment(Qt::AlignCenter);
    ltMain->addLayout(ltSettings);
    ltMain->addLayout(ltButtons);
    ltMain->setSpacing(10);
    ltMain->setContentsMargins(160, 120, 160, 120);
    setLayout(ltMain);
}

void SettingsWindow::BtnSave_Clicked()
{
    bool isRestartRequired = false;
    
    PROGRAM_CONSTANTS->pSettingsFile->SetConsoleStatus(chkEnableDebugConsole->checkState());
    ConsoleWindowStateUpdate(chkEnableDebugConsole->checkState());

    PROGRAM_CONSTANTS->pSettingsFile->SetDiscordRPCStatus(chkEnableDiscordRPC->checkState());
    DiscordRPCStateUpdate(chkEnableDiscordRPC->checkState());

    PROGRAM_CONSTANTS->pSettingsFile->SetForceSystemLanguageOnStartUp(chkForceSystemLanguageOnStartUp->checkState());
    
    bool isNewLanguageAssigned = cmbLanguage->currentIndex() != PROGRAM_CONSTANTS->pSettingsFile->GetLanguage();
    
    if (isNewLanguageAssigned)
        PROGRAM_CONSTANTS->pSettingsFile->SetLanguage(cmbLanguage->currentIndex());

    PROGRAM_CONSTANTS->pSettingsFile->Save();

    // Might be useful in future, but right now -- dead code
    if (isRestartRequired)
    {
        QMessageBox msgBox(nullptr);
        msgBox.setModal(true);
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.setWindowTitle(tr("Setting Apply Confirmation"));
        msgBox.setInformativeText(tr("You have selected settings that require a complete restart of the editor. Do you want to apply new settings and restart editor?"));

        QPushButton* btnMsgYes    = msgBox.addButton(tr("YES"), QMessageBox::YesRole);
        QPushButton* btnMsgCancel = msgBox.addButton(tr("CANCEL"), QMessageBox::RejectRole);

        int ret = msgBox.exec();

        switch (ret)
        {
            case QMessageBox::RejectRole:
                PROGRAM_CONSTANTS->pSettingsFile->Parse();
                return;
                break;
            case QMessageBox::AcceptRole:
            default:
                break;
        }
    }

    // Moved to last command due to compatibility issues
    // to save settings into file before other code would be executed
    if (isNewLanguageAssigned)
        emit languageChanged();
}

void SettingsWindow::BtnResetAll_Clicked()
{ 
    PROGRAM_CONSTANTS->pSettingsFile->SetToDefault();

    if (PROGRAM_CONSTANTS->pSettingsFile->IsConsoleEnabled())
        chkEnableDebugConsole->setCheckState(Qt::CheckState::Checked);
    else
        chkEnableDebugConsole->setCheckState(Qt::CheckState::Unchecked);

    if (PROGRAM_CONSTANTS->pSettingsFile->IsDiscordRPCEnabled())
        chkEnableDiscordRPC->setCheckState(Qt::CheckState::Checked);
    else
        chkEnableDiscordRPC->setCheckState(Qt::CheckState::Unchecked);
    
    if (PROGRAM_CONSTANTS->pSettingsFile->IsForceSystemLanguageOnStartUpEnabled())
        chkForceSystemLanguageOnStartUp->setCheckState(Qt::CheckState::Checked);
    else
        chkForceSystemLanguageOnStartUp->setCheckState(Qt::CheckState::Unchecked);

    chkEnableDebugConsole->update();
    chkEnableDiscordRPC->update();
    chkForceSystemLanguageOnStartUp->update();
}

void SettingsWindow::ConsoleWindowStateUpdate(const Qt::CheckState& state)
{
    switch (state)
    {
        case Qt::CheckState::Checked:
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            break;
        case Qt::CheckState::Unchecked:
            ShowWindow(GetConsoleWindow(), SW_HIDE);
            break;
    }
}

void SettingsWindow::DiscordRPCStateUpdate(const Qt::CheckState& state)
{
}
