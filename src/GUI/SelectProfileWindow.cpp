#include <QDir>
#include "../Extensions/StringExt.hpp"
#include "../Core/Logger.hpp"
#include "../Core/ProgramConstants.hpp"
#include "SelectProfileWindow.hpp"

using namespace StringExt;

SelectProfileWindow::SelectProfileWindow(QWidget* parent) : QWidget(parent)
{
    ltMain           = new QVBoxLayout();
    ltProfiles       = new QHBoxLayout();
    ltGameProfiles   = new QVBoxLayout();
    ltCustomProfiles = new QVBoxLayout();
    btnGenerals      = new QPushButton(tr(PROGRAM_CONSTANTS->G_FOLDER_NAME.toStdString().c_str()));
    btnGeneralsZH    = new QPushButton(tr(PROGRAM_CONSTANTS->GZH_FOLDER_NAME.toStdString().c_str()));
    btnSettings      = new QPushButton();

    btnGenerals->setObjectName(nameof(btnGenerals));
    connect(btnGenerals, &QPushButton::clicked, this, &SelectProfileWindow::BtnGenerals_Clicked);
    btnGeneralsZH->setObjectName(nameof(btnGeneralsZH));
    connect(btnGeneralsZH, &QPushButton::clicked, this, &SelectProfileWindow::BtnGeneralsZH_Clicked);
    QPixmap pxmSettings = QPixmap(PROGRAM_CONSTANTS->GEARS_ICON_FILE);
    btnSettings->setObjectName(nameof(btnSettings));
    btnSettings->setIcon(pxmSettings);
    btnSettings->setIconSize(pxmSettings.size());
    btnSettings->setFixedSize(pxmSettings.size());
    connect(btnSettings, &QPushButton::clicked, this, &SelectProfileWindow::BtnSettings_Clicked);
    ltGameProfiles->addWidget(btnGenerals);
    ltGameProfiles->addWidget(btnGeneralsZH);

    QDir profilesDir(PROGRAM_CONSTANTS->PROFILES_FOLDER);
    auto list = profilesDir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::Name);
    list.removeOne(PROGRAM_CONSTANTS->G_FOLDER_NAME);
    list.removeOne(PROGRAM_CONSTANTS->GZH_FOLDER_NAME);
    for (const auto& elem : list)
    {
        LOGMSG("Add custom profile " + elem);
        QPushButton* btnCustomProfile = new QPushButton();
        btnCustomProfile->setObjectName(nameof(btnCustomProfile));
        btnCustomProfile->setProperty("profileName", "btn"q + elem);
        btnCustomProfile->setText(elem);
        connect(btnCustomProfile, &QPushButton::clicked, this, &SelectProfileWindow::BtnCustomProfile_Clicked);
        ltCustomProfiles->addWidget(btnCustomProfile);
    }

    ltProfiles->addLayout(ltGameProfiles);
    ltProfiles->addLayout(ltCustomProfiles);

    ltMain->setAlignment(Qt::AlignmentFlag::AlignHCenter);
    ltMain->addLayout(ltProfiles);
    ltMain->addWidget(btnSettings, 0, Qt::AlignmentFlag::AlignHCenter);

    setLayout(ltMain);
}

void SelectProfileWindow::BtnSettings_Clicked()   { emit settingsClicked(); }
void SelectProfileWindow::BtnGenerals_Clicked()   { emit gProfileSelected(); }
void SelectProfileWindow::BtnGeneralsZH_Clicked() { emit gzhProfileSelected(); }
void SelectProfileWindow::BtnCustomProfile_Clicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    if (btn == nullptr)
        return;

    emit customProfileSelected(btn->text());
}
