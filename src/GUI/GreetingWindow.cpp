#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "../Windows/Locale.hpp"
#include "../Logger.hpp"
#include "../ProgramConstants.hpp"
#include "WindowManager.hpp"
#include "ImageManager.hpp"
#include "GreetingWindow.hpp"

GreetingWindow::GreetingWindow(QWidget* parent) : QWidget(parent)
{
    QPushButton* btnLoadFromGame = nullptr;
    QPushButton* btnLoadFromFile = nullptr;
    QHBoxLayout* ltButtons       = nullptr;
    QVBoxLayout* ltMain          = nullptr;
    QHBoxLayout* ltSettings      = nullptr;
    QVBoxLayout* ltLanguages     = nullptr;

    // Add "New Project" and "Load Project" buttons to the window
    btnLoadFromGame = new QPushButton(tr("LOAD FROM") + '\n' + tr("THE GAME"));
    btnLoadFromGame->setFixedSize(PROGRAM_CONSTANTS->START_BUTTON_SIZE);
    btnLoadFromGame->setObjectName(nameof(btnLoadFromGame));
    connect(btnLoadFromGame, &QPushButton::clicked, this, &GreetingWindow::btnLoadFromGameClicked);

    btnLoadFromFile = new QPushButton(tr("LOAD FROM") + '\n' + tr("THE FILE"));
    btnLoadFromFile->setFixedSize(PROGRAM_CONSTANTS->START_BUTTON_SIZE);
    btnLoadFromFile->setObjectName(nameof(btnLoadFromFile));
    connect(btnLoadFromFile, &QPushButton::clicked, this, &GreetingWindow::btnLoadFromFileClicked);
    
    QPushButton* btnSettings = new QPushButton();
    QPixmap      pxmSettings = QPixmap{PROGRAM_CONSTANTS->GEARS_ICON_FILE};
    btnSettings->setObjectName(nameof(btnSettings));
    btnSettings->setIcon(pxmSettings);
    btnSettings->setIconSize(pxmSettings.size());
    btnSettings->setFixedSize(pxmSettings.size());
    connect(btnSettings, &QPushButton::clicked, this, &GreetingWindow::btnSettingsClicked);

    ltLanguages = new QVBoxLayout();
    ltLanguages->addStretch(1);

    ltSettings = new QHBoxLayout();
    ltSettings->addLayout(ltLanguages);
    ltSettings->addWidget(btnSettings);

    ltButtons = new QHBoxLayout();
    ltButtons->setSpacing(50);
    ltButtons->setAlignment(Qt::AlignTop);
    ltButtons->addWidget(btnLoadFromGame);
    ltButtons->addWidget(btnLoadFromFile);
    ltButtons->setSpacing(30);

    ltMain = new QVBoxLayout();
    ltMain->setAlignment(Qt::AlignCenter);
    ltMain->addLayout(ltButtons);
    ltMain->addLayout(ltSettings);
    ltMain->setSpacing(20);
    ltMain->setContentsMargins(160, 120, 160, 120);
    
    setLayout(ltMain);
}

