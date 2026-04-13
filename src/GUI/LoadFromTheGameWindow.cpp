#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPixmap>
#include <QCheckBox>
#include <QTranslator>

#include "../ProgramConstants.hpp"
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

    pProfilesGroup = new QButtonGroup(this);
    QVBoxLayout* ltChoiseGame = new QVBoxLayout();
    const QString activeProfileId = PROGRAM_CONSTANTS->HasActiveProfile()
        ? PROGRAM_CONSTANTS->GetActiveProfile().GetId()
        : QString();
    bool hasSelectedProfile = false;
    QRadioButton* firstButton = nullptr;

    for (const auto& profile : PROGRAM_CONSTANTS->Profiles)
    {
        if (!profile.HasRegistryGame())
            continue;

        QRadioButton* button = new QRadioButton(profile.GetDisplayName());
        button->setProperty("profileId", profile.GetId());
        button->setObjectName(profile.GetId());
        pProfilesGroup->addButton(button);
        connect(button, &QRadioButton::toggled, this, [this, button](const bool isChecked)
        {
            if (isChecked)
                emit selectedProfileChanged(button->property("profileId").toString());
        });

        if (firstButton == nullptr)
            firstButton = button;

        if (!hasSelectedProfile && profile.GetId() == activeProfileId)
        {
            button->setChecked(true);
            hasSelectedProfile = true;
        }

        ltChoiseGame->addWidget(button);
    }

    if (!hasSelectedProfile && firstButton != nullptr)
    {
        firstButton->setChecked(true);
    }

    btnOk->setEnabled(firstButton != nullptr);

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

QString LoadFromTheGameWindow::GetSelectedProfileId() const
{
    const auto* checkedButton = pProfilesGroup->checkedButton();

    if (checkedButton == nullptr)
        return QString();

    return checkedButton->property("profileId").toString();
}
