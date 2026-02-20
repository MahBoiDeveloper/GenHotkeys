#pragma once
#include <QMainWindow>
#include <QScrollArea>
#include <QTreeWidgetItem>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStatusBar>

#include "../Parsers/JSONFile.hpp"
#include "ActionHotkeyWidget.hpp"

class EditorWindow final : public QMainWindow
{
    Q_OBJECT

private: // Data
    // Select faction buttons
    QButtonGroup* pFactionsButtonsGroup = nullptr;

    // Units list and theirs hotkeys
    QTreeWidget*  pEntitiesTreeWidget   = nullptr;
    QScrollArea*  pHotkeysArea          = nullptr;
    QScrollArea*  pKeyboardWindow       = nullptr;

    // Specific windows
    QDialog*      pAboutDialog          = nullptr;
    QWidget*      pSettingsWindow       = nullptr;

    // Current possible hotkeys layout
    QTabWidget*   pHotkeysPanelsWidget  = nullptr;

    // Container of the selected unit hotkeys
    QVector<QSet<ActionHotkeyWidget*>> vHotkeyWidgets;
public:
    QStatusBar*   pStatusBar            = nullptr;

public: // Methods
    EditorWindow(QWidget* parent = nullptr);

private:
    /// @brief Set context menu bar functions and logics.
    void ConfigureMenu();
    /// @brief Set game object and their actions for window by short faction name.
    void SetGameObjectList(const QString& factionShortName);
    /// @brief Set game object to display.
    void SetHotkeysPanels();
    /// @brief Set hotkeys colors. Default color is black. Changes color to red for keys, that is conflict to each other in one unit/building.
    void HighlightCurrentKeys();
    /// @brief Set key's styles on keyboard block to property `status=null`.
    void KeyboardWindow_Nullify();
    /// @brief Replace current action assigned hotkey with new one.
    void SetActionHotkey(const QString& fctShortName, const QString& goName, const QString& actName, const QString& hk);
    /// @brief Creates line of keys for keyboard layout.
    QHBoxLayout* CreateKeysOnKeyboard(const QString& str);
    /// @brief Saves changes into file.
    void Save();

private slots:
    void KeyboardWindow_Update(int id = 0);
    void ActAbout_Triggered();
    void ActSettings_Triggered();
    void ActSave_Triggered();
    void ActSaveAs_Triggered();
    void ActOpen_Triggered();
    void ActOpen_NewHotkeyFileSelected(const QString& filepath);
    void ActSaveAs_SaveToSelectedFile(const QString& filepath);

signals:
    void languageChanged();
    void newHotkeyFileSelected(const QString& filepath);
};
