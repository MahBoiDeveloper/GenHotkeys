#pragma once
#include <QStackedWidget>
#include "LoadWindow.hpp"
#include "SelectProfileWindow.hpp"
#include "SettingsWindow.hpp"

class SetUpWindowsWrapper final : public QStackedWidget
{
    Q_OBJECT
private: // Data
    SelectProfileWindow*   pSelectProfileWindow   = nullptr;
    LoadWindow* pLoadWindow = nullptr;
    SettingsWindow*        pSettingsWindow        = nullptr;

private: // Methods
    /// @brief Connects slots and singals.
    void AttachConnections();
    /// @brief Disconnects slots and signals.
    void DetachConnections();
    /// @brief Initialize `SelectProfileWindow`, `LoadFromTheGameWindow`, `LoadWindow` class objects.
    void AddWidgets();
    /// @brief Free heap from `SelectProfileWindow`, `LoadFromTheGameWindow`, `LoadWindow` data.
    void DeleteWidgets();
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
    /// @brief Returns checked configuration of user preferences.
    void LoadWindow_LoadFromTheFile();
    /// @brief Returns checked configuration of user preferences.
    void LoadWindow_LoadFromTheGame();
    /// @brief Handles selection of the Generals profile.
    void SelectProfileWindow_GProfileSelected();
    /// @brief Handles selection of the Generals Zero Hour profile.
    void SelectProfileWindow_GZHProfileSelected();
    /// @brief Handles selection of a custom profile.
    void SelectProfileWindow_CustomProfileSelected(const QString& folder);
};
