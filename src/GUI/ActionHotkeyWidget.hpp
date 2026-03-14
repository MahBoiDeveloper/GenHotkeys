#pragma once
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class ActionHotkeyWidget : public QWidget
{
    Q_OBJECT

private: // Data
    // Constants
    const int   TIMER_TIMEOUT               = 1300;
    const int   DEFAULT_HOTKEY_BUTTON_WIDTH = 15; // Checked for W, the widest letter

    // Internal data
    QString     hotkey;
    QString     actionName;
    QString     csfLabelName;
    QTimer      signalTimer;
    int         timerMseconds;

    // Layout
    QHBoxLayout ltMainBlock;
    QLabel      lblImage;
    QLabel      lblActionName;
    QLineEdit   txtActionName; // Questionable. Need experiments with enabling edits in QLabel in runtime.
    QPushButton btnHotkey;

private: // Methods
    /// @brief Hide/show action label to show/hide text editor.
    void HideActionLabel(bool value);
protected:
    void keyPressEvent(QKeyEvent*            event) override;
    void focusOutEvent(QFocusEvent*          event) override;
public:
    explicit ActionHotkeyWidget(const QString& csfString, const QString& iconName, QWidget* parent = nullptr);
    /// @brief Returns current action name without ampersand.
    QString GetActionName() const;
    /// @brief Returns current action hotkey after ampersand.
    QString GetHotkey() const;
    /// @brief Change hotkeys property "unique" and updates its style.
    void    HighlightKey(bool isKeysLessThanTwo);

signals:
    void RepeatNewHotkeySignal();
    /// @brief Emits new hotkey after change.
    void HotkeyChanged(QString hotkey);
private slots:
    void ChangeHotkeyClick();
    void ActEdit_Triggered();
};
