#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "../Core/ProgramConstants.hpp"

class SelectProfileWindow : public QWidget
{
    Q_OBJECT
private: // Data
    QVBoxLayout* ltMain           = nullptr;
    QHBoxLayout* ltProfiles       = nullptr;
    QVBoxLayout* ltGameProfiles   = nullptr;
    QVBoxLayout* ltCustomProfiles = nullptr;
    QPushButton* btnGenerals      = nullptr;
    QPushButton* btnGeneralsZH    = nullptr;
    QPushButton* btnSettings      = nullptr;
public: // Methods
    SelectProfileWindow(QWidget* parent = nullptr);
signals:
    void gProfileSelected();
    void gzhProfileSelected();
    void customProfileSelected(QString data);
    void settingsClicked();
private slots:
    void BtnGenerals_Clicked();
    void BtnGeneralsZH_Clicked();
    void BtnCustomProfile_Clicked();
    void BtnSettings_Clicked();
};
