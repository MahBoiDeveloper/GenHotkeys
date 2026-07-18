#pragma once
#include <QStackedWidget>
#include "LoadFromTheFileWindow.hpp"
#include "SelectProfileWindow.hpp"
#include "SettingsWindow.hpp"

class SetUpWindowsWrapper final : public QStackedWidget
{
    Q_OBJECT
private: // Data
    SelectProfileWindow*   pSelectProfileWindow   = nullptr;
    LoadFromTheFileWindow* pLoadFromTheFileWindow = nullptr;
    SettingsWindow*        pSettingsWindow        = nullptr;

private: // Methods
    /// @brief Connects slots and singals.
    void AttachConnections();
    /// @brief Disconnects slots and signals.
    void DetachConnections();
    /// @brief Initialize `SelectProfileWindow`, `LoadFromTheGameWindow`, `LoadFromTheFileWindow` class objects.
    void AddWidgets();
    /// @brief Free heap from `SelectProfileWindow`, `LoadFromTheGameWindow`, `LoadFromTheFileWindow` data.
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
    void LoadFromTheFileWindow_AcceptConfiguration();
    /// @brief Returns checked configuration of user preferences.
    void LoadFromTheGameWindow_AcceptConfiguration();
    /// @brief Handles selection of the Generals profile.
    void SelectProfileWindow_GProfileSelected();
    /// @brief Handles selection of the Generals Zero Hour profile.
    void SelectProfileWindow_GZHProfileSelected();
    /// @brief Handles selection of a custom profile.
    void SelectProfileWindow_CustomProfileSelected(const QString& folder);
};
