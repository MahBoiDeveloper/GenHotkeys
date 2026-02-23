#pragma once
#include <QStackedWidget>
#include "EditorWindow.hpp"

class EditorWindowWrapper final : public QStackedWidget
{
    Q_OBJECT
private: // Data
    EditorWindow* pEditorWindow  = nullptr;
public:
    bool          hasDataChanged = false;

private: // Methods
    /// @brief Connects slots and singals.
    void AttachConnections();
    /// @brief Disconnects slots and signals.
    void DetachConnections();
    /// @brief Add editor window widget to this wrapper.
    void AddWidgets();
public:
    EditorWindowWrapper(QWidget* parent = nullptr);
public slots:
    void EditorWindow_CloseEditor();
    void EditorWindow_LanguageChanged();
    void EditorWindow_NewHotkeyFileSelected(const QString& filepath);
};
