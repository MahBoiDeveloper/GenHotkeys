#pragma once
#include <QTranslator>

#include "../Core/ProgramConstants.hpp"

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
    /// @brief Applies common styles and selected profile theme.
    void ApplyProfileStyleSheet(const QString& profileFolder);
    /// @brief Read stylesheet file contents and write loading result to log.
    QString ReadStyleSheet(const QString& filepath, const QString& name) const;
public:
    WindowManager();
    ~WindowManager();
    /// @brief Enable start widget rendering.
    void Show();
    /// @brief Set common L10N translator for the whole project app.
    void SetTranslator();
    /// @brief Applies default startup theme.
    void ApplyDefaultProfileStyleSheet();
    /// @brief Set CSF file path. Uses if in `LoadFromTheFileWindow` file has been set.
    void SetCSFFilePath(const QString& filepath);
    /// @brief Reinitialize CSF parser and editor window when new CSF/BIG file has been selected.
    void EditorWindow_NewHotkeyFileSelected();
    /// @brief Return CSF file path. Uses if in `LoadFromTheFileWindow` file has been set.
    void StartUpWindow_AcceptConfiguration();
    /// @brief Initialize TechTree.json parsing by Generals profile.
    void StartUpWindow_GProfileSelected();
    /// @brief Initialize TechTree.json parsing by GeneralsZH profile.
    void StartUpWindow_GZHProfileSelected();
    /// @brief Initialize TechTree.json parsing by custom profile.
    void StartUpWindow_CustomProfileSelected(const QString& data);
};
