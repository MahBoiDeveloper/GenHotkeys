#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPixmap>
#include <QCheckBox>
#include <QTranslator>

#include "../Windows/Registry.hpp"
#include "../Logger.hpp"
#include "LoadFromTheGameWindow.hpp"

LoadFromTheGameWindow::LoadFromTheGameWindow(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout*  ltOkAndCancel   = new QHBoxLayout();
    QPushButton*  btnOk           = new QPushButton(tr("START"));
    QPushButton*  btnCancel       = new QPushButton(tr("BACK"));
    QHBoxLayout*  ltBtnOk         = new QHBoxLayout();
    QHBoxLayout*  ltBtnCancel     = new QHBoxLayout();

    btnOk->setObjectName("btnOk");
    btnOk->setFixedWidth(80);
    btnOk->setObjectName(nameof(btnOk));
    ltBtnOk->setAlignment(Qt::Alignment::enum_type::AlignRight);
    ltBtnOk->addWidget(btnOk);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setFixedWidth(80);
    btnCancel->setObjectName(nameof(btnCancel));
    ltBtnCancel->setAlignment(Qt::Alignment::enum_type::AlignLeft);
    ltBtnCancel->addWidget(btnCancel);
    ltOkAndCancel->addLayout(ltBtnOk);
    ltOkAndCancel->addLayout(ltBtnCancel);

    connect(btnOk,     &QPushButton::clicked, this, [=, this] { emit btnStartClicked(); });
    connect(btnCancel, &QPushButton::clicked, this, [=, this] { emit btnBackClicked(); });

    // configure game buttons
    QRadioButton* rdxGenerals = new QRadioButton(Registry::ToQString(Registry::Games::Generals));
    rdxGenerals->setDisabled(true);
    rdxGenerals->setObjectName(nameof(rdxGenerals));
    QFont rbxGeneralsFont = rdxGenerals->font();
    rbxGeneralsFont.setStrikeOut(true);
    rdxGenerals->setFont(rbxGeneralsFont);

    QRadioButton* rdxZeroHour = new QRadioButton(Registry::ToQString(Registry::Games::GeneralsZeroHour));
    rdxZeroHour->setChecked(true);
    rdxGenerals->setObjectName(nameof(rdxGenerals));

    QVBoxLayout* ltChoiseGame = new QVBoxLayout();
    ltChoiseGame->addWidget(rdxGenerals);
    ltChoiseGame->addWidget(rdxZeroHour);

    // configure dialog view
    QVBoxLayout* ltMainBlock = new QVBoxLayout();
    ltMainBlock->setAlignment(Qt::Alignment::enum_type::AlignCenter);
    ltMainBlock->addStretch(5);
    ltMainBlock->addLayout(ltChoiseGame);
    ltMainBlock->addStretch(5);
    ltMainBlock->addLayout(ltOkAndCancel);
    ltMainBlock->addStretch(1);
    setLayout(ltMainBlock);
}
