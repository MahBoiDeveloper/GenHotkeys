#include <QDir>
#include "../Extensions/StringExt.hpp"
#include "../Core/Logger.hpp"
#include "../Core/ProgramConstants.hpp"
#include "SelectProfileWindow.hpp"

using namespace StringExt;

SelectProfileWindow::SelectProfileWindow(QWidget* parent) : QWidget(parent)
{
    ltMain           = new QHBoxLayout();
    ltGameProfiles   = new QVBoxLayout();
    ltCustomProfiles = new QVBoxLayout();
    btnGenerals      = new QPushButton(tr(PROGRAM_CONSTANTS->G_FOLDER_NAME.toStdString().c_str()));
    btnGeneralsZH    = new QPushButton(tr(PROGRAM_CONSTANTS->GZH_FOLDER_NAME.toStdString().c_str()));

    btnGenerals->setObjectName(nameof(btnGenerals));
    btnGeneralsZH->setObjectName(nameof(btnGeneralsZH));
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
        btnCustomProfile->setObjectName("btn"q + elem);
        btnCustomProfile->setText(elem);
        ltCustomProfiles->addWidget(btnCustomProfile);
    }

    ltMain->addLayout(ltGameProfiles);
    ltMain->addLayout(ltCustomProfiles);

    setLayout(ltMain);
}
