#include <windows.h>

#include <QApplication>
#include <QAbstractButton>
#include <QMenuBar>
#include <QHeaderView>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>

#include "../Extensions/ExceptionExt.hpp"
#include "../Extensions/BoolExt.hpp"
#include "../Extensions/L10NExt.hpp"
#include "../Core/Logger.hpp"
#include "../Core/Convert.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Managers/FactionsManager.hpp"
#include "../Managers/SteamManager.hpp"
#include "../Windows/Registry.hpp"
#include "../Windows/Locale.hpp"

#include "Info.hpp"
#include "ImageManager.hpp"
#include "WindowManager.hpp"
#include "SettingsWindow.hpp"
#include "EditorWindow.hpp"

using namespace StringExt;
using namespace BoolExt;

// TODO: Move definition to the something like reflection header
inline int operator+(Qt::Modifier mod, Qt::Key key) { return (static_cast<int>(mod) + static_cast<int>(key)); }

#pragma region CTORs and Setters

EditorWindow::EditorWindow(QWidget* parent)
    : QMainWindow(parent)
    , pFactionsButtonsGroup{new QButtonGroup{this}}
    , pEntitiesTreeWidget{new QTreeWidget}
    , pHotkeysArea{new QScrollArea}
    , pKeyboardWindow{new QScrollArea}
    , pStatusBar{new QStatusBar}
    , pHotkeysPanelsWidget{nullptr}
    , pAboutDialog{nullptr}
{
    LOGMSG("Total faction count that has been read from json file: " + FACTIONS_MANAGER->Count());

    resize(PROGRAM_CONSTANTS->EDITOR_INITIAL_WINDOW_SIZE);
    ConfigureMenu();

    pEntitiesTreeWidget->header()->hide();
    // Enable smooth scrolling
    pEntitiesTreeWidget->setVerticalScrollMode(QTreeWidget::ScrollMode::ScrollPerPixel);
    // Set icon size
    pEntitiesTreeWidget->setIconSize(QSize{PROGRAM_CONSTANTS->ICON_MIN_HEIGHT, PROGRAM_CONSTANTS->ICON_MIN_HEIGHT});

    connect(pEntitiesTreeWidget, &QTreeWidget::itemSelectionChanged, this, &EditorWindow::SetHotkeysPanels);

    QBoxLayout* ltFactions = nullptr;
    int factonsCount = FACTIONS_MANAGER->Count();
    size_t language = PROGRAM_CONSTANTS->pSettingsFile->GetLanguage();

    ltFactions = new QHBoxLayout();
    ltFactions->setObjectName(nameof(ltFactions));

    if (factonsCount > Faction::MAXIMUM_FACTION_COUNT)
    {
        throw ExceptionExt("Unable to parse more than " + ToQString(Faction::MAXIMUM_FACTION_COUNT) + " factions. Found factions: " + factonsCount);
    }

    if (factonsCount < Faction::MINIMAL_FACTION_COUNT)
    {
        throw ExceptionExt("Unable to parse less than " + ToQString(Faction::MINIMAL_FACTION_COUNT) + " factions. Found factions: " + factonsCount);
    }

    QVBoxLayout* ltFactionsUSA    = new QVBoxLayout();
    QHBoxLayout* ltSubfactionsUSA = new QHBoxLayout();
    ltFactionsUSA->setObjectName(nameof(ltFactionsUSA));
    ltSubfactionsUSA->setObjectName(nameof(ltSubfactionsUSA));

    QVBoxLayout* ltFactionsPRC    = new QVBoxLayout();
    QHBoxLayout* ltSubfactionsPRC = new QHBoxLayout();
    ltFactionsPRC->setObjectName(nameof(ltFactionsPRC));
    ltSubfactionsPRC->setObjectName(nameof(ltSubfactionsPRC));

    QVBoxLayout* ltFactionsGLA    = new QVBoxLayout();
    QHBoxLayout* ltSubfactionsGLA = new QHBoxLayout();
    ltFactionsGLA->setObjectName(nameof(ltFactionsGLA));
    ltSubfactionsGLA->setObjectName(nameof(ltSubfactionsGLA));

    #define ADD_TO_LAYOUT(x) \
    { \
        if (PROGRAM_CONSTANTS->x##_SHORT_NAMES.contains(shortName)) \
        { \
            factionButton->setProperty("faction", QString(#x)); \
            if (shortName == QString(#x)) \
                ltFactions##x->addWidget(factionButton); \
            else \
                ltSubfactions##x->addWidget(factionButton); \
        } \
    }

    for (int i = 0; i < FACTIONS_MANAGER->Count(); ++i)
    {
        const Faction& currFaction = FACTIONS_MANAGER->FindByIndex(i);

        QPushButton* factionButton = new QPushButton(currFaction.GetDisplayName());
        factionButton->setToolTip(currFaction.GetDisplayNameDescription());

        QString shortName = currFaction.GetShortName();

        ADD_TO_LAYOUT(USA);
        ADD_TO_LAYOUT(PRC);
        ADD_TO_LAYOUT(GLA);

        connect(factionButton, &QPushButton::pressed, this, [=, this]()
        {
            SetGameObjectList(shortName);
        });

        pFactionsButtonsGroup->addButton(factionButton);
    }
    
    #undef ADD_TO_LAYOUT

    #define RM_IF_EMPTY(x) \
    { \
        if (!ltSubfactions##x->count()) \
            ltSubfactions##x->deleteLater(); \
        else \
            ltFactions##x->addLayout(ltSubfactions##x); \
    }

    RM_IF_EMPTY(USA);
    RM_IF_EMPTY(PRC);
    RM_IF_EMPTY(GLA);

    #undef RM_IF_EMPTY

    ltFactions->addLayout(ltFactionsUSA);
    ltFactions->addLayout(ltFactionsPRC);
    ltFactions->addLayout(ltFactionsGLA);

    connect(pFactionsButtonsGroup, &QButtonGroup::idClicked, this, [=, this](int id)
    {
        // Take the focus from the buttons group
        pEntitiesTreeWidget->setFocus();

        // Select pressed faction button
        for (auto* button : pFactionsButtonsGroup->buttons())
        {
            if (button == pFactionsButtonsGroup->button(id))
                button->setDown(true);
            else
                button->setDown(false);
        }
    });

    // Fill all available space
    pHotkeysArea->setWidgetResizable(true);

    // Draw keyboard with letters
    pKeyboardWindow->setObjectName("Keyboard");
    QHBoxLayout* pKeyboardFirstLine;
    QHBoxLayout* pKeyboardSecondLine;
    QHBoxLayout* pKeyboardThirdLine;
    QVBoxLayout* pKeyboardLines = new QVBoxLayout();
    
    QPushButton* btnEmptyButton = new QPushButton();
    btnEmptyButton->setProperty("key", "null");
    btnEmptyButton->setFixedWidth(PROGRAM_CONSTANTS->EMPTY_KEY_WIDTH);

    pKeyboardFirstLine  = CreateKeysOnKeyboard("QWERTYUIOP");
    pKeyboardSecondLine = CreateKeysOnKeyboard("ASDFGHJKL");
    pKeyboardThirdLine  = CreateKeysOnKeyboard("ZXCVBNM");
    
    pKeyboardThirdLine->addWidget(btnEmptyButton);

    pKeyboardFirstLine->setAlignment(Qt::AlignHCenter);
    pKeyboardSecondLine->setAlignment(Qt::AlignHCenter);
    pKeyboardThirdLine->setAlignment(Qt::AlignHCenter);

    pKeyboardLines->addLayout(pKeyboardFirstLine);
    pKeyboardLines->addLayout(pKeyboardSecondLine);
    pKeyboardLines->addLayout(pKeyboardThirdLine);

    pKeyboardWindow->setLayout(pKeyboardLines);

    QVBoxLayout* ltGameObject = new QVBoxLayout();
    ltGameObject->addWidget(pHotkeysArea, HOTKEYS_AREA_STRETCH);
    ltGameObject->addWidget(pKeyboardWindow, KEYBOARD_WINDOW_STRETCH);

    QHBoxLayout* ltContent = new QHBoxLayout();
    ltContent->addWidget(pEntitiesTreeWidget, ENTITIES_TREE_WIDGET_STRETCH);
    ltContent->addLayout(ltGameObject, GAME_OBJECT_LAYOUT_STRETCH);

    pStatusBar->setSizeGripEnabled(false);
    pStatusBar->setHidden(!PROGRAM_CONSTANTS->pSettingsFile->IsStatusBarEnabled());
    pStatusBar->installEventFilter(this);

    pStatusBarHistory = new QPlainTextEdit();
    pStatusBarHistory->setObjectName(nameof(pStatusBarHistory));
    pStatusBarHistory->setReadOnly(true);
    pStatusBarHistory->installEventFilter(this);
    pStatusBarHistory->viewport()->installEventFilter(this);

    QVBoxLayout* ltMain = new QVBoxLayout();
    ltMain->addLayout(ltFactions);
    ltMain->addLayout(ltContent);
    ltMain->addWidget(pStatusBar);

    // Main widget
    QWidget* editorContent = new QWidget();
    editorContent->setLayout(ltMain);
    pEditorPages = new QStackedWidget();
    pEditorPages->setContentsMargins(ltMain->contentsMargins());
    ltMain->setContentsMargins(0, 0, 0, 0);
    pEditorPages->addWidget(editorContent);
    pEditorPages->addWidget(pStatusBarHistory);
    setCentralWidget(pEditorPages);

    // Set start faction
    const auto firstFactionButton = pFactionsButtonsGroup->button(BUTTONS_GROUP_START_ID);
    if (firstFactionButton != nullptr)
        firstFactionButton->click();

    connect(this, &QMainWindow::close, this, &EditorWindow::ClosingWindow);
}

void EditorWindow::ConfigureMenu()
{
    // File
    QMenu*   mnFileOptions = new QMenu(tr("File"));
    QAction* actOpen       = new QAction(tr("Open"));
    QAction* actSave       = new QAction(tr("Save"));
    QAction* actSaveAs     = new QAction(tr("Save As..."));
    QAction* actClose      = new QAction(tr("Close"));

    actOpen->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    actSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    actSaveAs->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    actClose->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    mnFileOptions->addAction(actOpen);
    mnFileOptions->addAction(actSave);
    mnFileOptions->addAction(actSaveAs);
    mnFileOptions->addAction(actClose);
    menuBar()->addMenu(mnFileOptions);

    connect(actOpen,   &QAction::triggered, this, &EditorWindow::ActOpen_Triggered);
    connect(actSave,   &QAction::triggered, this, &EditorWindow::ActSave_Triggered);
    connect(actSaveAs, &QAction::triggered, this, &EditorWindow::ActSaveAs_Triggered);
    connect(actClose,  &QAction::triggered, this, &EditorWindow::ActClose_Triggered);

    // View
    QMenu*   mnViewOptions       = new QMenu(tr("View"));
    QMenu*   mnStatusBarChecbox  = new QMenu(tr("Status Bar"));
    QAction* actEnableStatusBar  = new QAction(tr("Enable"));
    QAction* actDisableStatusBar = new QAction(tr("Disable"));
    actDisableStatusBar->setProperty("selected", !PROGRAM_CONSTANTS->pSettingsFile->IsStatusBarEnabled());
    mnStatusBarChecbox->addAction(actEnableStatusBar);
    mnStatusBarChecbox->addAction(actDisableStatusBar);
    mnViewOptions->addMenu(mnStatusBarChecbox);
    menuBar()->addMenu(mnViewOptions);

    connect(actEnableStatusBar,  &QAction::triggered, this, &EditorWindow::ActEnableStatusBar_Triggered);
    connect(actDisableStatusBar, &QAction::triggered, this, &EditorWindow::ActDisableStatusBar_Triggered);
   
    // Actions
    QMenu*   mnActions        = new QMenu(tr("Actions"));
    QAction* actRunGame       = new QAction(tr("Run Game"));
    QAction* actExtractHotkey = new QAction(tr("Exctract Hotkey"));
    actRunGame->setToolTip(tr("Run game in Steam"));
    actExtractHotkey->setToolTip(tr("Move hotkey to its \"island\" before object/action name"));
    mnActions->setToolTipsVisible(true);
    mnActions->addAction(actRunGame);
    mnActions->addAction(actExtractHotkey);
    menuBar()->addMenu(mnActions);

    connect(actRunGame,       &QAction::triggered, this, &EditorWindow::ActRunGame_Triggered);
    connect(actExtractHotkey, &QAction::triggered, this, &EditorWindow::ActExtractHotkey_Triggered);

    // Settings
    QAction* actSettings = new QAction(tr("Settings"));
    menuBar()->addAction(actSettings);
    connect(actSettings, &QAction::triggered, this, &EditorWindow::ActSettings_Triggered);

    // About    
    QAction* actAbout = new QAction(tr("About"));
    menuBar()->addAction(actAbout);
    connect(actAbout, &QAction::triggered, this, &EditorWindow::ActAbout_Triggered);
}

void EditorWindow::SetGameObjectList(const QString& factionShortName)
{
    pEntitiesTreeWidget->clear();

    QMap<Faction::GameObject, GameObjectTypes> goMap = FACTIONS_MANAGER->FindByShortName(factionShortName).GetTechTree();

    // Skip if there are no entities of that type
    if(goMap.isEmpty()) return;

    // Create sections for all faction entities types
    for(const auto& objectType : PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS.keys())
    {
        // Create new section of tree list
        QTreeWidgetItem* newTopEntityItem = new QTreeWidgetItem();
        newTopEntityItem->setText(0, L10N(PROGRAM_CONSTANTS->INGAME_ENTITIES_STRINGS.value(objectType)));

        // Decorate
        newTopEntityItem->setIcon(0, ImageManager::GetGameObjectTypePixmap(objectType)
                                                   .scaledToHeight(PROGRAM_CONSTANTS->ICON_SCALING_HEIGHT, Qt::SmoothTransformation));

        // If there no objects by type - then skip
        if (goMap.keys(objectType).isEmpty())
        {
            continue;
        }
        
        // Append entities to the section
        for (const auto& go : goMap.keys(objectType))
        {
            QTreeWidgetItem* currentNewEntityItem = new QTreeWidgetItem();
            currentNewEntityItem->setText(0, CSF_PARSER->GetStringValue(go.ingameName));
            currentNewEntityItem->setIcon(0, QPixmap::fromImage(ImageManager::LoadActionIcon(go.iconName)));
            currentNewEntityItem->setData(0, Qt::UserRole, QVariant::fromValue(QPair{factionShortName, go.iconName}));
            newTopEntityItem->addChild(currentNewEntityItem);
        }

        // Add section to the list
        pEntitiesTreeWidget->addTopLevelItem(newTopEntityItem);
    }

    // Configure view
    pEntitiesTreeWidget->expandAll();
    pEntitiesTreeWidget->scrollToTop();

    // Skip if missing top item
    const auto firstTopLevelItem = pEntitiesTreeWidget->topLevelItem(0);
    if (firstTopLevelItem == nullptr) return;

    // Skip if missing first entity
    const auto firstEntity = firstTopLevelItem->child(0);
    if (firstEntity == nullptr) return;

    // Set start entity
    pEntitiesTreeWidget->setCurrentItem(firstEntity);
}

void EditorWindow::SetHotkeysPanels()
{
    // Skip if there are no selected items
    if (pEntitiesTreeWidget->selectedItems().isEmpty()) return;

    // Current single selected item
    QTreeWidgetItem* pItem = pEntitiesTreeWidget->selectedItems().first();

    // Skip if it's the top level section item
    for (int i = 0; i < pEntitiesTreeWidget->topLevelItemCount(); ++i)
    {
        if (pItem == pEntitiesTreeWidget->topLevelItem(i))
        {
            return;
        }
    }

    const QPair<QString, QString> specialItemInfo = pItem->data(0, Qt::UserRole).value<QPair<QString, QString>>();

    const QString& factionShortName = specialItemInfo.first;
    const QString& gameObjectName   = specialItemInfo.second;

    const auto gameObjectKeyboardLayouts = FACTIONS_MANAGER->FindByShortName(factionShortName).GetKeyboardLayoutsByObjectName(gameObjectName);

    // Recreate panels widget
    if (pHotkeysPanelsWidget != nullptr) pHotkeysPanelsWidget->deleteLater();
    pHotkeysPanelsWidget = new QTabWidget();

    // Forget old hotkey widgets
    vHotkeyWidgets.clear();

    // Panel index
    int i = 0;
    
    for (const auto& currLayout : gameObjectKeyboardLayouts)
    {
        QSet<ActionHotkeyWidget*> currentPanelWidgets;
        QVBoxLayout* ltHotkeys = new QVBoxLayout();

        for (const auto& currAction : currLayout)
        {
            ActionHotkeyWidget* actionHotkey = new ActionHotkeyWidget(currAction.hotkeyString, currAction.iconName);

            connect(actionHotkey, &ActionHotkeyWidget::HotkeyChanged, this, [=, this](const QString& newHotkey)
            {
                // Set new hotkey
                SetActionHotkey(factionShortName, gameObjectName, currAction.iconName, newHotkey);

                // Highlight keys for entity
                HighlightCurrentKeys();

                // Set flag into wrapper
                WINDOW_MANAGER->pHotkeysEditor->hasDataChanged = true;

                emit pHotkeysPanelsWidget->currentChanged(i);
            });

            ltHotkeys->addWidget(actionHotkey);

            // Remember widget
            currentPanelWidgets.insert(actionHotkey);
        }

        // Remember hotkeys panel
        vHotkeyWidgets.append(currentPanelWidgets);

        // Highlight keys for entity
        HighlightCurrentKeys();

        // Condense the actions at the top with minumum spacing
        ltHotkeys->setAlignment(Qt::AlignTop);
        ltHotkeys->setSpacing(0);

        QWidget* panelScrollWidget = new QWidget();
        panelScrollWidget->setLayout(ltHotkeys);
        panelScrollWidget->setObjectName(QString("Layout ") + QString::number(i + 1));
        pHotkeysPanelsWidget->addTab(panelScrollWidget, QString(tr("Layout %1")).arg(++i));
    }

    // If only one panel -> hide header
    if (pHotkeysPanelsWidget->count() < 2) pHotkeysPanelsWidget->tabBar()->hide();

    pHotkeysPanelsWidget->setMinimumSize(pHotkeysPanelsWidget->sizeHint());
    pHotkeysArea->setWidget(pHotkeysPanelsWidget);

    connect(pHotkeysPanelsWidget, &QTabWidget::currentChanged, this, &EditorWindow::KeyboardWindow_Update);

    emit pHotkeysPanelsWidget->currentChanged(0);
}

void EditorWindow::HighlightCurrentKeys()
{
    // Skip if no widgets
    if (vHotkeyWidgets.isEmpty()) return;

    // Else code does check for collisions
    for (auto& panel : vHotkeyWidgets)
    {
        // Fill list with only letters of keys
        QList<QString> keysCollisions;
        for (const auto& hotkeyWidget: panel)
            keysCollisions.push_back(hotkeyWidget->GetHotkey());

        for (auto& hotkeyWidget : panel)
        {
            const QString& thisHotkey = hotkeyWidget->GetHotkey();

            if (keysCollisions.count(thisHotkey) < 2) hotkeyWidget->HighlightKey(false);
            else                                      hotkeyWidget->HighlightKey(true);

            if (!PROGRAM_CONSTANTS->pSettingsFile->GetAllowedKeys().contains(Convert::ToQtKey(thisHotkey[0])))
                hotkeyWidget->HighlightKey(true);
        }
    }
}

QHBoxLayout* EditorWindow::CreateKeysOnKeyboard(const QString& str)
{
    QHBoxLayout* pKeys = new QHBoxLayout();
    for (const auto& ch : str)
    {
        auto tmp = new QPushButton(ch);
        tmp->setProperty("key", ch);
        tmp->setObjectName(ch);
        tmp->setFixedWidth(PROGRAM_CONSTANTS->KEYBOARD_KEY_WIDTH);
        pKeys->addWidget(tmp);
    }
    return pKeys;
}

void EditorWindow::SetActionHotkey(const QString& fctShortName, const QString& goName, const QString& actName, const QString& hk)
{
    Faction& fct = const_cast<Faction&>(FACTIONS_MANAGER->FindByShortName(fctShortName));
    fct.SetHotkey(goName, actName, hk);
}

#pragma endregion

#pragma region Handlers

void EditorWindow::KeyboardWindow_Nullify()
{
    for (const QChar& qc : QString("QWERTYUIOPASDFGHJKLZXCVBNM")) 
    {
        auto key = pKeyboardWindow->findChild<QPushButton*>(qc, Qt::FindChildrenRecursively);
        key->setProperty("status", "null");
        key->style()->unpolish(key);
        key->style()->polish(key);
        key->update();
    }
}

void EditorWindow::KeyboardWindow_Update(int id)
{
    KeyboardWindow_Nullify();
    auto currTab = pHotkeysPanelsWidget->findChild<QWidget*>(QString("Layout ") + QString::number(id + 1), Qt::FindChildrenRecursively);
        
    QString accum;
    for (const auto& elem : currTab->findChildren<ActionHotkeyWidget*>(EmptyString, Qt::FindChildrenRecursively))
        accum += elem->GetHotkey().toUpper();

    for (const QChar& ch : accum)
    {
        auto key = pKeyboardWindow->findChild<QPushButton*>(ch, Qt::FindChildrenRecursively);

        if (key == nullptr)
            continue;

        if (accum.count(ch) < 2) key->setProperty("status", "good");
        else                     key->setProperty("status", "bad");

        if (!PROGRAM_CONSTANTS->pSettingsFile->GetAllowedKeys().contains(Convert::ToQtKey(ch)))
            key->setProperty("status", "bad");
        
        key->style()->unpolish(key);
        key->style()->polish(key);
        key->update();
    }
}

void EditorWindow::ActAbout_Triggered()
{
    // If dialog already exists, we reuse created window
    if (pAboutDialog != nullptr)
    {
        pAboutDialog->activateWindow();
        return;
    }

    QGridLayout* lblContent = new QGridLayout();
    lblContent->setSizeConstraint(QLayout::SetFixedSize);
        
    QLabel* lblAboutText = new QLabel
    {
        tr("<p>Authors: %1<br>Version: %2<br>Commit: %3<br><br>Program licensed with %4<br><br>GitHub repository:<br>%5<br><br>Support development:<br>%6</p>")
            .arg(AUTHORS)
            .arg(VERSION)
            .arg("<a style=\"color: CLR;\" href=\"https://github.com/MahBoiDeveloper/GenHotkeys/commit/COMMIT\">COMMIT</a>"q)
            .arg("<a style=\"color: CLR;\" href=\"https://github.com/MahBoiDeveloper/GenHotkeys/blob/main/LICENSE\">GNU GPL v3</a>"q)
            .arg("<a style=\"color: CLR;\" href=\"https://github.com/MahBoiDeveloper/GenHotkeys\">github.com/MahBoiDeveloper/GenHotkeys</a>"q)
            .arg("<a style=\"color: CLR;\" href=\"https://boosty.to/mah_boi\">boosty.to/mah_boi</a></p>"q)
            .replace("COMMIT", GIT_COMMIT_HASH)
            .replace("CLR", PROGRAM_CONSTANTS->LINK_COLOR)
    };
    lblAboutText->setObjectName("left");
    lblAboutText->setTextFormat(Qt::RichText);
    lblAboutText->setTextInteractionFlags(Qt::TextBrowserInteraction);
    lblAboutText->setOpenExternalLinks(true);
    lblContent->addWidget(lblAboutText, 0, 0);
    
    QLabel* lblEditorIcon = new QLabel();
    lblEditorIcon->setPixmap(QPixmap::fromImage(ImageManager::LoadBigEditorIcon()));
    lblEditorIcon->setObjectName("right");
    lblContent->addWidget(lblEditorIcon, 0, 1);

    pAboutDialog = new QDialog{this};
    pAboutDialog->setWindowTitle(tr("About"));
    pAboutDialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pAboutDialog->setWindowFlags(pAboutDialog->windowFlags() &
                               ~Qt::WindowContextHelpButtonHint |
                                Qt::MSWindowsFixedSizeDialogHint);

    QVBoxLayout* ltMainBlock = new QVBoxLayout();
    ltMainBlock->addLayout(lblContent);

    pAboutDialog->setLayout(ltMainBlock);
    pAboutDialog->setWindowModality(Qt::WindowModality::ApplicationModal);
    pAboutDialog->show();
    pAboutDialog->activateWindow();
    
    connect(pAboutDialog, &QDialog::finished, this, [this]()
    {
        pAboutDialog->deleteLater();
        pAboutDialog = nullptr;
    });
}

void EditorWindow::ActSettings_Triggered()
{
    if (pSettingsWindow != nullptr)
    {
        pSettingsWindow->show();
        return;
    }

    // Read this thread: 
    //    https://forum.qt.io/topic/146107/can-t-show-the-border-of-the-class-inheriting-qwidget-class
    // to understand why QWidget's inherited class has been wrapped into another native QWidget

    auto sw = new SettingsWindow();
    auto lt = new QVBoxLayout();
    lt->addWidget(sw);

    pSettingsWindow = new QWidget();
    pSettingsWindow->setObjectName(nameof(pSettingsWindow));
    pSettingsWindow->setWindowTitle(tr("Settings"));
    pSettingsWindow->setFixedSize(PROGRAM_CONSTANTS->SET_UP_WINDOW_SIZE);
    pSettingsWindow->setWindowFlags(Qt::WindowType::MSWindowsFixedSizeDialogHint);
    pSettingsWindow->setWindowFlags(pSettingsWindow->windowFlags() 
                                    & ~Qt::WindowType::WindowMaximizeButtonHint
                                    & ~Qt::WindowType::WindowMinimizeButtonHint);
    pSettingsWindow->setWindowModality(Qt::WindowModality::ApplicationModal);

    pSettingsWindow->setLayout(lt);

    connect(sw, &SettingsWindow::languageChanged, this,            &EditorWindow::SettingsWindow_LanguageChanged);
    connect(sw, &SettingsWindow::btnBackClicked,  pSettingsWindow, &QWidget::close);

    pSettingsWindow->show();
}

void EditorWindow::ActSave_Triggered()
{
    LOGMSG("Saving changes to .csf file...");
    
    if (CSF_PARSER->Path.ends_with(L".big") || CSF_PARSER->Path.ends_with(L".BIG"))
    {
        CSF_PARSER->Path =
        (
            (
                WINDOW_MANAGER->ProfileFolder.split('/')[2] == PROGRAM_CONSTANTS->G_FOLDER_NAME
                    ? QString::fromStdWString(Windows::Registry::GetPathToGame(Windows::Registry::Games::Generals))
                    : QString::fromStdWString(Windows::Registry::GetPathToGame(Windows::Registry::Games::GeneralsZeroHour))
            )
            + "Data\\English\\generals.csf"q
        ).toStdWString();
    }

    CSF_PARSER->Save();
    LOGMSG("Changes has been saved");

    ShowStatusMessage(tr("Changes has been saved to the file:") + " " + CSF_PARSER->Path);

    WINDOW_MANAGER->pHotkeysEditor->hasDataChanged = false;
}

void EditorWindow::ActSaveAs_Triggered()
{
    QFileDialog* fdSelectFileWindow = new QFileDialog();
    connect(fdSelectFileWindow, &QFileDialog::fileSelected, this, &EditorWindow::ActSaveAs_SaveToSelectedFile);
    fdSelectFileWindow->setFileMode(QFileDialog::FileMode::AnyFile);
    fdSelectFileWindow->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    fdSelectFileWindow->setDefaultSuffix("csf");
    fdSelectFileWindow->exec();

    WINDOW_MANAGER->pHotkeysEditor->hasDataChanged = false;
}

void EditorWindow::ActSaveAs_SaveToSelectedFile(const QString& filepath)
{ 
    CSF_PARSER->Save(filepath);
    ShowStatusMessage(tr("Saved as:") + " " + filepath);
}

void EditorWindow::ActOpen_Triggered()
{
    QFileDialog* fdSelectFileWindow = new QFileDialog();
    connect(fdSelectFileWindow, &QFileDialog::fileSelected, this, &EditorWindow::ActOpen_NewHotkeyFileSelected);
    fdSelectFileWindow->setFileMode(QFileDialog::FileMode::ExistingFile);
    fdSelectFileWindow->setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fdSelectFileWindow->setNameFilters({tr("Binary files") + " (*.csf *.big)",
                                        tr("Any files")  + " (*)"});
    fdSelectFileWindow->exec();
}

void EditorWindow::ActClose_Triggered() { emit closeEditor(); }

void EditorWindow::UpdateStatusBar(const Qt::CheckState& state)
{
    bool bState = ToBool(state);
    PROGRAM_CONSTANTS->pSettingsFile->SetStatusBarStatus(state);
    PROGRAM_CONSTANTS->pSettingsFile->Save();
    if (!bState)
        pEditorPages->setCurrentIndex(0);
    pStatusBar->setHidden(!bState);
}

void EditorWindow::ShowStatusMessage(const QString& message)
{
    const QString historyEntry = QString::number(statusMessages.size() + 1) + ". " + message;
    statusMessages.append(historyEntry);
    if (pEditorPages->currentWidget() == pStatusBarHistory)
        pStatusBarHistory->appendPlainText(historyEntry);
    pStatusBar->showMessage(message, STATUS_BAR_TIMEOUT);
}

bool EditorWindow::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == pStatusBar || watched == pStatusBarHistory || watched == pStatusBarHistory->viewport())
        && event->type() == QEvent::MouseButtonDblClick
        && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton
        && pStatusBar->isEnabled() && !pStatusBar->isHidden())
    {
        const bool expand = pEditorPages->currentIndex() == 0;
        pEditorPages->setCurrentIndex(expand ? 1 : 0);
        if (expand)
        {
            pStatusBarHistory->setPlainText(statusMessages.join('\n'));
            auto* scrollBar = pStatusBarHistory->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
        }
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void EditorWindow::ActEnableStatusBar_Triggered() { UpdateStatusBar(Qt::CheckState::Checked); }

void EditorWindow::ActDisableStatusBar_Triggered() { UpdateStatusBar(Qt::CheckState::Unchecked); }

void EditorWindow::ActOpen_NewHotkeyFileSelected(const QString& filepath)
{
    LOGMSG("Selected file: " + filepath);
    emit newHotkeyFileSelected(filepath);
    ShowStatusMessage(tr("Opening selected file"));
}

void EditorWindow::ActRunGame_Triggered() 
{
    QString cmd = "steam://rungameid/"q + STEAM_MANAGER->GetAppID();
    LOGMSG("Opening game via link " + cmd);
    ShowStatusMessage(tr("Opening game via link") + " " + cmd);
    auto status = ShellExecuteW(nullptr, L"open", cmd.toStdWString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    LOGMSG("Returned status: " + reinterpret_cast<size_t>(status));
}

void EditorWindow::ActExtractHotkey_Triggered()
{
    QMessageBox confirmationDialog(
        QMessageBox::Warning,
        tr("Warning"),
        tr("Extract hotkeys may take a long time.\n\nAre you sure to apply this action?"),
        QMessageBox::Ok | QMessageBox::Cancel,
        this);
    confirmationDialog.setDefaultButton(QMessageBox::Cancel);
    confirmationDialog.button(QMessageBox::Ok)->setText(tr("OK"));
    confirmationDialog.button(QMessageBox::Cancel)->setText(tr("Cancel"));
    confirmationDialog.findChild<QDialogButtonBox*>()->setCenterButtons(true);

    const auto confirmation = confirmationDialog.exec();

    if (confirmation != QMessageBox::Ok)
        return;

    ShowStatusMessage(tr("Applying hotkey extraction..."));

    CSF_PARSER->ExtractHotkeys(PROGRAM_CONSTANTS->HOTKEY_CSF_CATEGORY);
    ActSave_Triggered();

    const int currentLayout = pHotkeysPanelsWidget != nullptr ? pHotkeysPanelsWidget->currentIndex() : 0;
    SetHotkeysPanels();
    if (pHotkeysPanelsWidget != nullptr)
        pHotkeysPanelsWidget->setCurrentIndex(currentLayout);
}

void EditorWindow::SettingsWindow_LanguageChanged()
{
    this->pSettingsWindow->close();
    ShowStatusMessage(tr("Changing editor language"));
    emit languageChanged();
}

void EditorWindow::ClosingWindow()
{
    if (WINDOW_MANAGER->pHotkeysEditor->hasDataChanged)
    {
        QMessageBox confirmationDialog(
            QMessageBox::Warning,
            tr("Warning"),
            tr("Are you want to save changes before quit?"),
            QMessageBox::Ok | QMessageBox::Cancel,
            this);
        confirmationDialog.setDefaultButton(QMessageBox::Cancel);
        confirmationDialog.button(QMessageBox::Default)->setText(tr("Save"));
        confirmationDialog.button(QMessageBox::Ok)->setText(tr("Don't Save"));
        confirmationDialog.button(QMessageBox::Cancel)->setText(tr("Cancel"));
        confirmationDialog.findChild<QDialogButtonBox*>()->setCenterButtons(true);

        const auto confirmation = confirmationDialog.exec();

        if (confirmation == QMessageBox::Cancel)
            return;

        if (confirmation == QMessageBox::Default)
        {
            ActSave_Triggered();   
        }
    }

    close();
}

#pragma endregion
