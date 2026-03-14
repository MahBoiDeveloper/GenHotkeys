#include <QMenu>
#include <QStyle>
#include <QAction>
#include <QKeyEvent>
#include <QMessageBox>

#include "../Extensions/L10NExt.hpp"
#include "../Parsers/CSFParser.hpp"
#include "../Parsers/TextParser.hpp"
#include "../Logger.hpp"

#include "ImageManager.hpp"
#include "ActionHotkeyWidget.hpp"

using namespace StringExt;

ActionHotkeyWidget::ActionHotkeyWidget(const QString& csfString,
                                       const QString& iconName,
                                       QWidget* parent)
    : QWidget{parent}
    , hotkey{TextParser::GetHotkey(CSF_PARSER->GetStringValue(csfString))}
    , actionName{TextParser::GetClearActionName(CSF_PARSER->GetStringValue(csfString))}
    , csfLabelName{csfString}
    , signalTimer{}
    , timerMseconds{TIMER_TIMEOUT}
{
    // Object name for css
    btnHotkey.setText(hotkey);
    btnHotkey.setObjectName(nameof(btnHotkey));
    btnHotkey.setProperty("unique", false);
    btnHotkey.setMinimumSize(ImageManager::DecodeMissingWebpIcon().size().width() + DEFAULT_HOTKEY_BUTTON_WIDTH,
                             ImageManager::DecodeMissingWebpIcon().size().height());

    connect(&btnHotkey, &QPushButton::pressed, this, &ActionHotkeyWidget::ChangeHotkeyClick);

    // Signal timer settings
    signalTimer.setSingleShot(true);
    connect(&signalTimer, &QTimer::timeout, this, &ActionHotkeyWidget::RepeatNewHotkeySignal);

    lblImage.setPixmap(QPixmap::fromImage(ImageManager::DecodeWebpIcon(iconName)));

    QAction *actEdit = new QAction(tr("Edit source text"), this);
    actEdit->setShortcut(Qt::Key_F2);
    actEdit->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
    connect(actEdit, &QAction::triggered, this, &ActionHotkeyWidget::ActEdit_Triggered);

    lblActionName.setText(actionName);
    lblActionName.setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    lblActionName.setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
    lblActionName.addAction(actEdit);

    txtActionName.setObjectName(nameof(txtActionName));
    txtActionName.setText(CSF_PARSER->GetStringValue(csfLabelName));
    txtActionName.hide();

    ltMainBlock.setAlignment(Qt::AlignTop);
    ltMainBlock.addWidget(&lblImage);
    ltMainBlock.addWidget(&lblActionName);
    ltMainBlock.addWidget(&txtActionName);
    ltMainBlock.setStretch(1, 1); // Move action name label to left
    ltMainBlock.addWidget(&btnHotkey);
    setLayout(&ltMainBlock);
    setToolTip(csfLabelName + "\n\n" + txtActionName.text());
}

#pragma region Methods

QString ActionHotkeyWidget::GetActionName() const { return lblActionName.text(); }
QString ActionHotkeyWidget::GetHotkey()     const { return btnHotkey.text(); }

void ActionHotkeyWidget::HighlightKey(bool isKeysMoreThanTwo)
{
    // If more than 2 == not unique
    btnHotkey.setProperty("unique", !isKeysMoreThanTwo);
    btnHotkey.style()->unpolish(&btnHotkey);
    btnHotkey.style()->polish(&btnHotkey);
    btnHotkey.update();
}

void ActionHotkeyWidget::HideActionLabel(bool value)
{
    if (value)
    {
        lblActionName.hide();
        txtActionName.show();
    }
    else
    {
        txtActionName.hide();
        lblActionName.show();
    }
}

#pragma endregion

#pragma region Overrides

void ActionHotkeyWidget::keyPressEvent(QKeyEvent* event)
{
    auto key_native = event->nativeVirtualKey();
    auto key_layout = event->key();

    // Skip if there are modifiers
    if (event->modifiers() != Qt::NoModifier)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    // Switch label to line edit
    if (event->key() == Qt::Key_F2)
    {
        ActEdit_Triggered();
        return;
    }

    if (lblActionName.isHidden())
    {
        QChar ch = TextParser::GetHotkey(txtActionName.text());

        switch (key_layout)
        {
        case Qt::Key_Return: // Use new text that user has suggested
            // Sanitize input
            if (!StringExt::IsACII(ch))
            {
                QMessageBox::critical(nullptr, L10N(PROGRAM_CONSTANTS->NON_ASCII_HOTKEY_HEADER),
                                               L10N(PROGRAM_CONSTANTS->NON_ASCII_HOTKEY_DESCRIPTION.arg(ch)));
                return;
            }

            // Update CSF and widget's fields
            hotkey = ch;
            actionName = TextParser::GetClearActionName(txtActionName.text());
            CSF_PARSER->SetStringValue(csfLabelName.toStdString(), txtActionName.text().toStdWString());
            
            // Update widget's UI
            HideActionLabel(false);
            lblActionName.setText(actionName);
            btnHotkey.setText(hotkey);
            setToolTip(csfLabelName + "\n\n" + txtActionName.text());

            emit HotkeyChanged(hotkey);
            return;

        case Qt::Key_Escape: // Restore previous settings if ESC has pressed
            HideActionLabel(false);
            txtActionName.setText(CSF_PARSER->GetStringValue(csfLabelName));
            parentWidget()->setFocus();
            return;

        default: // Ingore key parsing if text field is active
            return;
        }
    }

    auto key = static_cast<Qt::Key>(key_native);
    if (PROGRAM_CONSTANTS->pSettingsFile->GetAllowedKeys().contains(key))
    {
        // Set new text
        hotkey = QKeySequence(key).toString();
        QString txt = txtActionName.text().replace("&"q + TextParser::GetHotkey(txtActionName.text()), "&"q + hotkey);
        lblActionName.setText(TextParser::GetClearActionName(txt));
        txtActionName.setText(txt);
        setToolTip(csfLabelName + "\n\n" + txtActionName.text());
        CSF_PARSER->SetStringValue(csfLabelName.toStdString(), TextParser::SetHotkey(CSF_PARSER->GetStringValue(csfLabelName), hotkey.at(0)).toStdWString());
        
        // If the key is correct -> disconnect the input error reset signal
        disconnect(this, &ActionHotkeyWidget::RepeatNewHotkeySignal, this, &ActionHotkeyWidget::ChangeHotkeyClick);

        // Return focus to parent
        parentWidget()->setFocus();
    }
    else
    {
        btnHotkey.setText(!PROGRAM_CONSTANTS->KEYBOARD_KEYS.contains(key)
                          ? tr("It isn't character key!")
                          : tr("This key isn't allowed!"));
        
        // Start the signal timer with a delay of n milliseconds
        if (signalTimer.isActive())
            signalTimer.stop();

        signalTimer.start(timerMseconds);
    }

    btnHotkey.setProperty("unique", true);

    // Execute code from the parent
    QWidget::keyPressEvent(event);
}

void ActionHotkeyWidget::focusOutEvent(QFocusEvent* event)
{
    // Unset decoration
    QFont fnt = btnHotkey.font();
    fnt.setItalic(false);
    btnHotkey.setFont(fnt);
    btnHotkey.setText(hotkey);

    emit HotkeyChanged(hotkey);

    if (btnHotkey.property("unique").Bool)
    {
        btnHotkey.setProperty("unique", false);
    }

    // Stop timer
    signalTimer.stop();

    // Execute code from the parent
    QWidget::focusOutEvent(event);
}

#pragma endregion

#pragma region Handlers

void ActionHotkeyWidget::ChangeHotkeyClick()
{
    // Reconnect the input error reset signal
    disconnect(this, &ActionHotkeyWidget::RepeatNewHotkeySignal, this, &ActionHotkeyWidget::ChangeHotkeyClick);
    connect(this, &ActionHotkeyWidget::RepeatNewHotkeySignal, this, &ActionHotkeyWidget::ChangeHotkeyClick);

    // Decorate
    btnHotkey.setText(tr("Press the key..."));
    btnHotkey.setProperty("unique", true);
    QFont f(btnHotkey.font());
    f.setItalic(true);
    btnHotkey.setFont(f);

    // Set focus on hotkey element
    setFocus();
}

void ActionHotkeyWidget::ActEdit_Triggered() { HideActionLabel(true); }

#pragma endregion
