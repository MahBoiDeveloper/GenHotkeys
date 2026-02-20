#pragma once
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include "../Settings.hpp"

class SettingsWindow final : public QWidget
{
    Q_OBJECT

private: // Data
    QPushButton* btnBack                         = nullptr;
    QPushButton* btnSave                         = nullptr;
    QPushButton* btnResetAll                     = nullptr;
    QCheckBox*   chkEnableDebugConsole           = nullptr;
    QCheckBox*   chkEnableDiscordRPC             = nullptr;
    QCheckBox*   chkForceSystemLanguageOnStartUp = nullptr;
    QCheckBox*   chkEnableStatusBar              = nullptr;
    QLabel*      lblLanguage                     = nullptr;
    QComboBox*   cmbLanguage                     = nullptr;
    QVBoxLayout* ltMain                          = nullptr;
    QHBoxLayout* ltButtons                       = nullptr;
    QHBoxLayout* ltSettings                      = nullptr;
    QVBoxLayout* ltLeftColumn                    = nullptr;
    QVBoxLayout* ltRightColumn                   = nullptr;
    QHBoxLayout* ltLanguage                      = nullptr;

private: // Methods
    /// @brief Enable/disable debug console.
    void ConsoleWindowStateUpdate(const Qt::CheckState& state);
    /// @brief Enable/disable Discord RPC integration.
    /// @todo Implement later.
    void DiscordRPCStateUpdate(const Qt::CheckState& state);
    /// @brief Enable/disable status bar.
    void StatusBarStateUpdate(const Qt::CheckState& state);
private slots:
    void BtnSave_Clicked();
    void BtnResetAll_Clicked();
public:
    SettingsWindow(QWidget* parent = nullptr);
signals:
    void btnBackClicked();
    void languageChanged();
};
