#include "EditorWindowWrapper.hpp"
#include "WindowManager.hpp"

EditorWindowWrapper::EditorWindowWrapper(QWidget* parent) : QStackedWidget(parent)
{
    resize(PROGRAM_CONSTANTS->EDITOR_WINDOW_SIZE);
    AddWidgets();
    setCurrentWidget(pEditorWindow);
    AttachConnections();
}

void EditorWindowWrapper::AddWidgets()
{
    pEditorWindow = new EditorWindow();
    addWidget(pEditorWindow);
}

void EditorWindowWrapper::AttachConnections()
{
    connect(pEditorWindow, &EditorWindow::languageChanged, 
            this,          &EditorWindowWrapper::EditorWindow_LanguageChanged);

    connect(pEditorWindow, &EditorWindow::newHotkeyFileSelected, 
            this,          &EditorWindowWrapper::EditorWindow_NewHotkeyFileSelected);
}

void EditorWindowWrapper::DetachConnections()
{
    disconnect(pEditorWindow, &EditorWindow::languageChanged, 
               this,          &EditorWindowWrapper::EditorWindow_LanguageChanged);

    disconnect(pEditorWindow, &EditorWindow::newHotkeyFileSelected, 
               this,          &EditorWindowWrapper::EditorWindow_NewHotkeyFileSelected);
}

void EditorWindowWrapper::EditorWindow_LanguageChanged()
{
    WINDOW_MANAGER->SetTranslator();

    DetachConnections();
    pEditorWindow->deleteLater();
    hide();

    AddWidgets();
    AttachConnections();
    setCurrentWidget(pEditorWindow);
    show();
}

void EditorWindowWrapper::EditorWindow_NewHotkeyFileSelected(const QString& filepath)
{
    WINDOW_MANAGER->SetCSFFilePath(filepath);
    WINDOW_MANAGER->EditorWindow_NewHotkeyFileSelected();
}
