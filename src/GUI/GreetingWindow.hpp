#pragma once
#include <QWidget>
#include "../Core/ProgramConstants.hpp"

class GreetingWindow : public QWidget
{
    Q_OBJECT
public: // Methods
    GreetingWindow(QWidget* parent = nullptr);

signals:
    void btnLoadFromFileClicked();
    void btnLoadFromGameClicked();
    void btnSettingsClicked();
};
