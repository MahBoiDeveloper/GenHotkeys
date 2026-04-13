#pragma once
#include <QStackedWidget>
#include <QString>
#include "GreetingWindow.hpp"
#include "LoadFromTheGameWindow.hpp"
#include "LoadFromTheFileWindow.hpp"
#include "SettingsWindow.hpp"

class SetUpWindowsWrapper final : public QStackedWidget
{
    Q_OBJECT
private: // Data
    GreetingWindow*        pGreetingWidget        = nullptr;
    LoadFromTheGameWindow* pLoadFromTheGameWindow = nullptr;
    LoadFromTheFileWindow* pLoadFromTheFileWindow = nullptr;
    SettingsWindow*        pSettingsWindow        = nullptr;

private: // Methods
    /// @brief Connects slots and singals.
    void AttachConnections();
    /// @brief Disconnects slots and signals.
    void DetachConnections();
    /// @brief Initialize `GreetingWindow`, `LoadFromTheGameWindow`, `LoadFromTheFileWindow`.
    void AddWidgets();
public:
    SetUpWindowsWrapper(QWidget* parent = nullptr);

private slots:
    /// @brief Show `GreetingWindow`, if button `Back` has been clicked.
    void BtnBack_Clicked();
    /// @brief Change app language.
    void SettingsWindow_LanguageChanged();
    /// @brief Show `SettingsWindow` if button `btnSettings` has been clicked.
    void BtnSettings_Clicked();
    /// @brief Show window to load hotkeys information from .csf file.
    void BtnLoadFromFile_Clicked();
    /// @brief Show window to load hotkeys information from the game.
    void BtnLoadFromGame_Clicked();
    /// @brief Preview theme for the currently selected profile.
    void LoadWindow_ProfileChanged(const QString& profileId);
    /// @brief Returns checked configuration of user preferences.
    void LoadFromTheFileWindow_AcceptConfiguration();
    /// @brief Returns checked configuration of user preferences.
    void LoadFromTheGameWindow_AcceptConfiguration();
};
