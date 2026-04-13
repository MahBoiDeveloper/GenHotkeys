#include <QApplication>
#include <QMenuBar>
#include <QHeaderView>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QFileDialog>

#include "../Extensions/BoolExt.hpp"
#include "../Extensions/L10NExt.hpp"
#include "../Windows/Registry.hpp"
#include "../Windows/Locale.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Info.hpp"
#include "../Logger.hpp"
#include "../Convert.hpp"
#include "../FactionsManager.hpp"

#include "ImageManager.hpp"
#include "LoadFromTheFileWindow.hpp"
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

    QHBoxLayout* ltFactions = new QHBoxLayout();
    ltFactions->setObjectName(nameof(ltFactions));

    QVector<QVector<QString>> factionGroups = PROGRAM_CONSTANTS->GetActiveProfile().GetFactionGroups();
    if (factionGroups.isEmpty())
    {
        QVector<QString> fallbackGroup;
        for (int i = 0; i < FACTIONS_MANAGER->Count(); ++i)
            fallbackGroup.push_back(FACTIONS_MANAGER->FindByIndex(i).GetShortName());

        if (!fallbackGroup.isEmpty())
            factionGroups.push_back(fallbackGroup);
    }

    for (const auto& group : factionGroups)
    {
        QVBoxLayout* ltCurrentFaction    = new QVBoxLayout();
        QHBoxLayout* ltCurrentSubfaction = new QHBoxLayout();
        ltCurrentFaction->setObjectName(nameof(ltCurrentFaction));
        ltCurrentSubfaction->setObjectName(nameof(ltCurrentSubfaction));

        int addedButtons = 0;

        for (const auto& shortName : group)
        {
            if (!FACTIONS_MANAGER->ContainsShortName(shortName))
                continue;

            const Faction& currFaction = FACTIONS_MANAGER->FindByShortName(shortName);

            QPushButton* factionButton = new QPushButton{currFaction.GetDisplayName()};
            factionButton->setToolTip(currFaction.GetDisplayNameDescription());

            if (PROGRAM_CONSTANTS->USA_SHORT_NAMES.contains(shortName))
                factionButton->setProperty("faction", "USA");

            if (PROGRAM_CONSTANTS->PRC_SHORT_NAMES.contains(shortName))
                factionButton->setProperty("faction", "PRC");

            if (PROGRAM_CONSTANTS->GLA_SHORT_NAMES.contains(shortName))
                factionButton->setProperty("faction", "GLA");

            connect(factionButton, &QPushButton::pressed, this, [=, this]()
            {
                SetGameObjectList(shortName);
            });

            pFactionsButtonsGroup->addButton(factionButton);

            if (addedButtons == 0)
                ltCurrentFaction->addWidget(factionButton);
            else
                ltCurrentSubfaction->addWidget(factionButton);

            ++addedButtons;
        }

        if (addedButtons == 0)
            continue;

        if (addedButtons > 1)
            ltCurrentFaction->addLayout(ltCurrentSubfaction);

        ltFactions->addLayout(ltCurrentFaction);
    }

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
    
    QPushButton* btnEmptyButton= new QPushButton();
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

    QVBoxLayout* ltMain = new QVBoxLayout();
    ltMain->addLayout(ltFactions);
    ltMain->addLayout(ltContent);
    ltMain->addWidget(pStatusBar);

    // Main widget
    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(ltMain);
    setCentralWidget(centralWidget);

    // Set start faction
    const auto firstFactionButton = pFactionsButtonsGroup->button(BUTTONS_GROUP_START_ID);
    if (firstFactionButton != nullptr) firstFactionButton->click();
}

void EditorWindow::ConfigureMenu()
{
    QMenu* mnFileOptions = new QMenu(tr("File"));
    QAction* actOpen     = new QAction(tr("Open"));
    QAction* actSave     = new QAction(tr("Save"));
    QAction* actSaveAs   = new QAction(tr("Save As..."));
    QAction* actClose    = new QAction(tr("Close"));

    actOpen->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    actSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    actSaveAs->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    actClose->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    mnFileOptions->addAction(actOpen);
    mnFileOptions->addAction(actSave);
    mnFileOptions->addAction(actSaveAs);
    mnFileOptions->addAction(actClose);
    menuBar()->addMenu(mnFileOptions);

    connect(actOpen, &QAction::triggered, this, &EditorWindow::ActOpen_Triggered);
    connect(actSave, &QAction::triggered, this, &EditorWindow::ActSave_Triggered);
    connect(actSaveAs, &QAction::triggered, this, &EditorWindow::ActSaveAs_Triggered);
    connect(actClose, &QAction::triggered, this, &EditorWindow::ActClose_Triggered);

    QMenu* mnViewOptions = new QMenu(tr("View"));
    QMenu* mnStatusBarChecbox = new QMenu(tr("Status Bar"));
    QAction* actEnableStatusBar = new QAction(tr("Enable"));
    QAction* actDisableStatusBar = new QAction(tr("Disable"));
    actDisableStatusBar->setProperty("selected", !PROGRAM_CONSTANTS->pSettingsFile->IsStatusBarEnabled());
    mnStatusBarChecbox->addAction(actEnableStatusBar);
    mnStatusBarChecbox->addAction(actDisableStatusBar);
    mnViewOptions->addMenu(mnStatusBarChecbox);
    menuBar()->addMenu(mnViewOptions);

    connect(actEnableStatusBar,  &QAction::triggered, this, &EditorWindow::ActEnableStatusBar_Triggered);
    connect(actDisableStatusBar, &QAction::triggered, this, &EditorWindow::ActDisableStatusBar_Triggered);
   
    QAction* actSettings = new QAction(tr("Settings"));
    connect(actSettings, &QAction::triggered, this, &EditorWindow::ActSettings_Triggered);
    menuBar()->addAction(actSettings);

    QAction* actAbout = new QAction(tr("About"));
    connect(actAbout, &QAction::triggered, this, &EditorWindow::ActAbout_Triggered);
    menuBar()->addAction(actAbout);
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
            currentNewEntityItem->setIcon(0, QPixmap::fromImage(ImageManager::DecodeWebpIcon(go.iconName)));
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
        tr("<p>Authors: %1<br>Version: %2<br><br>Program licensed with %3<br><br>GitHub repository:<br>%4<br><br>Support development:<br>%5</p>")
            .arg(AUTHORS)
            .arg(VERSION)
            .arg("<a style=\"color: CLR;\" href=\"https://github.com/MahBoiDeveloper/GZHHotkeysEditor/blob/main/LICENSE\">GNU GPL v3</a>"q
                .replace("CLR", PROGRAM_CONSTANTS->LINK_COLOR))
            .arg("<a style=\"color: CLR;\" href=\"https://github.com/MahBoiDeveloper/GZHHotkeysEditor\">github.com/MahBoiDeveloper/GZHHotkeysEditor</a>"q
                .replace("CLR", PROGRAM_CONSTANTS->LINK_COLOR))
            .arg("<a style=\"color: CLR;\" href=\"https://boosty.to/mah_boi\">boosty.to/mah_boi</a></p>"q
                .replace("CLR", PROGRAM_CONSTANTS->LINK_COLOR))
    };
    lblAboutText->setObjectName("left");
    lblAboutText->setTextFormat(Qt::RichText);
    lblAboutText->setTextInteractionFlags(Qt::TextBrowserInteraction);
    lblAboutText->setOpenExternalLinks(true);
    lblContent->addWidget(lblAboutText, 0, 0);
    
    QLabel* lblEditorIcon = new QLabel();
    lblEditorIcon->setPixmap(QPixmap::fromImage(ImageManager::DecodeBigEditorWebpIcon()));
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
        const QString installedPath = PROGRAM_CONSTANTS->GetActiveProfile().GetInstalledCSFPath();

        if (installedPath.isEmpty())
        {
            ActSaveAs_Triggered();
            return;
        }

        CSF_PARSER->Path = installedPath.toStdWString();
    }

    CSF_PARSER->Save();
    LOGMSG("Changes has been saved");

    pStatusBar->showMessage(tr("Changes has been saved to the file:") + " " + CSF_PARSER->Path, STATUS_BAR_TIMEOUT);
}

void EditorWindow::ActSaveAs_Triggered()
{
    QFileDialog* fdSelectFileWindow = new QFileDialog();
    connect(fdSelectFileWindow, &QFileDialog::fileSelected, this, &EditorWindow::ActSaveAs_SaveToSelectedFile);
    fdSelectFileWindow->setFileMode(QFileDialog::FileMode::AnyFile);
    fdSelectFileWindow->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    fdSelectFileWindow->setDefaultSuffix("csf");
    fdSelectFileWindow->exec();
}

void EditorWindow::ActSaveAs_SaveToSelectedFile(const QString& filepath)
{ 
    CSF_PARSER->Save(filepath);
    pStatusBar->showMessage(tr("Saved as:") + " " + filepath, STATUS_BAR_TIMEOUT);
}

void EditorWindow::ActOpen_Triggered()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Open"));
    dialog.setFixedSize(PROGRAM_CONSTANTS->SET_UP_WINDOW_SIZE);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    dialog.setWindowModality(Qt::WindowModality::ApplicationModal);

    auto* openWidget = new LoadFromTheFileWindow(&dialog);
    auto* layout = new QVBoxLayout();
    layout->addWidget(openWidget);
    dialog.setLayout(layout);

    connect(openWidget, &LoadFromTheFileWindow::btnBackClicked, &dialog, &QDialog::reject);
    connect(openWidget, &LoadFromTheFileWindow::selectedProfileChanged, &dialog, [](const QString& profileId)
    {
        WINDOW_MANAGER->PreviewThemeForProfile(profileId);
    });
    connect(openWidget, &LoadFromTheFileWindow::btnStartClicked, this, [this, openWidget, &dialog]()
    {
        ActOpen_NewHotkeyFileSelected(openWidget->GetSelectedFilePath(), openWidget->GetSelectedProfileId());
        dialog.accept();
    });
    connect(&dialog, &QDialog::rejected, this, []()
    {
        WINDOW_MANAGER->ApplyTheme();
    });

    dialog.exec();
}

void EditorWindow::ActClose_Triggered() { emit closeEditor(); }

void EditorWindow::UpdateStatusBar(const Qt::CheckState& state)
{
    bool bState = ToBool(state);
    PROGRAM_CONSTANTS->pSettingsFile->SetStatusBarStatus(state);
    PROGRAM_CONSTANTS->pSettingsFile->Save();
    pStatusBar->setHidden(!bState);
}

void EditorWindow::ActEnableStatusBar_Triggered() { UpdateStatusBar(Qt::CheckState::Checked); }

void EditorWindow::ActDisableStatusBar_Triggered() { UpdateStatusBar(Qt::CheckState::Unchecked); }

void EditorWindow::ActOpen_NewHotkeyFileSelected(const QString& filepath, const QString& profileId)
{
    LOGMSG("Selected file: " + filepath);
    emit newHotkeyFileSelected(filepath, profileId);
    pStatusBar->showMessage(tr("Opening selected file"), STATUS_BAR_TIMEOUT);
}

void EditorWindow::SettingsWindow_LanguageChanged()
{
    this->pSettingsWindow->close();
    pStatusBar->showMessage(tr("Changing editor language"), STATUS_BAR_TIMEOUT);
    emit languageChanged();
}

#pragma endregion

