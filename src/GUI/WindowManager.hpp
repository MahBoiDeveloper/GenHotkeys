#pragma once
#include <QTranslator>

#include "../ProgramConstants.hpp"

#include "SetUpWindowsWrapper.hpp"
#include "EditorWindowWrapper.hpp"

#define WINDOW_MANAGER WindowManager::Instance

class WindowManager final
{
private: // Data
    SetUpWindowsWrapper* pStartUpWindow = nullptr;
    QTranslator*         pAppTranslator = nullptr;
    QString              strWindowName  = PROGRAM_CONSTANTS->COMMON_TITLE;
    QString              strCSFFilePath = "";
public:
    inline static std::unique_ptr<WindowManager> Instance = nullptr;
    EditorWindowWrapper* pHotkeysEditor = nullptr;

private: // Methods
    /// @brief Checks csf file to be compitable with GZH format.
    bool InitializeCSFParser();
    /// @brief Apply current style sheet and optional profile override.
    bool ApplyTheme(const Profile* profile);
public:
    WindowManager();
    ~WindowManager();
    /// @brief Enable start widget rendering.
    void Show();
    /// @brief Apply theme for the active profile or fallback theme.
    bool ApplyTheme();
    /// @brief Temporarily preview theme for a selected profile.
    bool PreviewThemeForProfile(const QString& profileId);
    /// @brief Set common L10N translator for the whole project app.
    void SetTranslator();
    /// @brief Update titles to match active profile.
    void UpdateWindowTitle();
    /// @brief Set CSF file path. Uses if in `LoadFromTheFileWindow` file has been set.
    void SetCSFFilePath(const QString& filepath);
    /// @brief Reinitialize CSF parser and editor window when new CSF/BIG file has been selected.
    void EditorWindow_NewHotkeyFileSelected();
    /// @brief Return CSF file path. Uses if in `LoadFromTheFileWindow` file has been set.
    void StartUpWindow_AcceptConfiguration();
};
