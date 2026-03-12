#pragma once
#include <QLabel>
#include <QTimer>
#include <QPushButton>

class ActionHotkeyWidget : public QWidget
{
    Q_OBJECT

private: // Data
    const int   TIMER_TIMEOUT               = 1300;  // Checked for W, the widest letter
    const int   DEFAULT_HOTKEY_BUTTON_WIDTH = 15;

    QString     hotkey;
    QLabel      actionNameLabel;
    QPushButton btnHotkey;
    QLabel      image;
    QTimer      signalTimer;
    int         timerMseconds;

protected: // Methods
    void keyPressEvent(QKeyEvent*   event) override;
    void focusOutEvent(QFocusEvent* event) override;
public:
    explicit ActionHotkeyWidget(const QString& actionName, const QString& hotkeyStr, const QString& iconName, QWidget* parent = nullptr);
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
};
