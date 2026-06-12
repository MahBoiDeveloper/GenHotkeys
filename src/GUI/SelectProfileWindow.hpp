#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "../Core/ProgramConstants.hpp"

class SelectProfileWindow : public QWidget
{
    Q_OBJECT
private: // Data
    QHBoxLayout* ltMain           = nullptr;
    QVBoxLayout* ltGameProfiles   = nullptr;
    QVBoxLayout* ltCustomProfiles = nullptr;
    QPushButton* btnGenerals      = nullptr;
    QPushButton* btnGeneralsZH    = nullptr;
public: // Methods
    SelectProfileWindow(QWidget* parent = nullptr);
};
