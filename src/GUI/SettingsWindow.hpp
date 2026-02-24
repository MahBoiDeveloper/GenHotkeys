#pragma once
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>

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
    QCheckBox*   chkEnableSteamIntegration       = nullptr;
    QLabel*      lblLanguage                     = nullptr;
    QComboBox*   cmbLanguage                     = nullptr;
    QVBoxLayout* ltMain                          = nullptr;
    QHBoxLayout* ltButtons                       = nullptr;
    QHBoxLayout* ltSettings                      = nullptr;
    QVBoxLayout* ltLeftColumn                    = nullptr;
    QVBoxLayout* ltRightColumn                   = nullptr;
    QHBoxLayout* ltLanguage                      = nullptr;

private: // Methods
    /// @brief Enables/disables debug console.
    void ConsoleWindowStateUpdate(const Qt::CheckState& state);
    /// @brief Enables/disables Discord RPC integration.
    /// @todo Implement later.
    void DiscordRPCStateUpdate(const Qt::CheckState& state);
    /// @brief Enables/disables Steam integration.
    void SteamAPIStateUpdate(const Qt::CheckState& state);
public:
    SettingsWindow(QWidget* parent = nullptr);
private slots:
    void BtnSave_Clicked();
    void BtnResetAll_Clicked();
signals:
    void btnBackClicked();
    void languageChanged();
};
