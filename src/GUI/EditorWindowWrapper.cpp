#include <QCloseEvent>

#include "EditorWindowWrapper.hpp"
#include "WindowManager.hpp"

EditorWindowWrapper::EditorWindowWrapper(QWidget* parent) : QStackedWidget(parent), editHistory{this}
{
    connect(&editHistory, &QUndoStack::cleanChanged, this, [this](bool clean)
    {
        hasDataChanged = !clean;
        setWindowModified(hasDataChanged);
    });
    resize(PROGRAM_CONSTANTS->EDITOR_INITIAL_WINDOW_SIZE);
    AddWidgets();
    setCurrentWidget(pEditorWindow);
    AttachConnections();
}

void EditorWindowWrapper::AddWidgets()
{
    pEditorWindow = new EditorWindow(&editHistory);
    addWidget(pEditorWindow);
}

void EditorWindowWrapper::AttachConnections()
{
    connect(pEditorWindow, &EditorWindow::languageChanged, 
            this,          &EditorWindowWrapper::EditorWindow_LanguageChanged);

    connect(pEditorWindow, &EditorWindow::newHotkeyFileSelected, 
            this,          &EditorWindowWrapper::EditorWindow_NewHotkeyFileSelected);

    connect(pEditorWindow, &EditorWindow::closeEditor, 
            this,          &EditorWindowWrapper::EditorWindow_CloseEditor);
}

void EditorWindowWrapper::DetachConnections()
{
    disconnect(pEditorWindow, &EditorWindow::languageChanged, 
               this,          &EditorWindowWrapper::EditorWindow_LanguageChanged);

    disconnect(pEditorWindow, &EditorWindow::newHotkeyFileSelected, 
               this,          &EditorWindowWrapper::EditorWindow_NewHotkeyFileSelected);

    disconnect(pEditorWindow, &EditorWindow::closeEditor, 
               this,          &EditorWindowWrapper::EditorWindow_CloseEditor);
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
    WINDOW_MANAGER->EditorWindow_NewHotkeyFileSelected(filepath);
}

void EditorWindowWrapper::closeEvent(QCloseEvent* event)
{
    if (pEditorWindow->ConfirmClose())
        event->accept();
    else
        event->ignore();
}

void EditorWindowWrapper::EditorWindow_CloseEditor() { close(); }
