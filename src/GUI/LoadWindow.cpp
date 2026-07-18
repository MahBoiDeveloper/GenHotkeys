#include <QRadioButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QLineEdit>
#include <QFontMetrics>
#include <QLabel>

#include "../Extensions/StringExt.hpp"
#include "LoadWindow.hpp"

LoadWindow::LoadWindow(QWidget* parent) : QWidget(parent)
{
    const int    SPACING         = 10;
    const int    BUTTON_WIDTH    = 80;
    QVBoxLayout* ltBottomButtons = new QVBoxLayout();
    QHBoxLayout* ltOkAndCancel   = new QHBoxLayout();
    QPushButton* btnOk           = new QPushButton(tr("START"));
    QPushButton* btnCancel       = new QPushButton(tr("BACK"));
    QPushButton* btnFromGame     = new QPushButton(tr("FROM THE GAME"));
    QLabel*      lblSelectFile   = new QLabel(tr("Select .csf or .big file:"));

    btnOk->setObjectName(nameof(btnOk));
    btnOk->setFixedWidth(BUTTON_WIDTH);
    btnCancel->setObjectName(nameof(btnCancel));
    btnCancel->setFixedWidth(BUTTON_WIDTH);
    btnFromGame->setObjectName(nameof(btnFromGame));
    btnFromGame->setFixedWidth(btnCancel->width() * 2 + SPACING);
    ltOkAndCancel->addWidget(btnOk);
    ltOkAndCancel->addWidget(btnCancel);
    ltBottomButtons->setAlignment(Qt::AlignmentFlag::AlignCenter);
    ltBottomButtons->setSpacing(SPACING);
    ltBottomButtons->addWidget(btnFromGame);
    ltBottomButtons->addLayout(ltOkAndCancel);
    connect(btnOk,       &QPushButton::clicked, this, [=, this] { emit btnStartClicked(); });
    connect(btnCancel,   &QPushButton::clicked, this, [=, this] { emit btnBackClicked(); });
    connect(btnFromGame, &QPushButton::clicked, this, [=, this] { emit btnFromGameClicked(); });

    lblSelectFile->setObjectName(nameof(lblSelectFile));
    lblSelectFile->setAlignment(Qt::AlignLeft);

    QLineEdit* lneFilePath = new QLineEdit();
    lneFilePath->setObjectName(nameof(lneFilePath));
    lneFilePath->setMaximumWidth(700);
    connect(lneFilePath, &QLineEdit::textChanged, this, &LoadWindow::lneFilePath_textChanged);

    QFont font(lneFilePath->font());
    font.setPointSize(font.pointSize()-2); // reduce standart font size
    lneFilePath->setFont(font);

    QFileDialog* fileDialog = new QFileDialog(); // dialog for selecting the path to the file
    fileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fileDialog->setNameFilters({tr("Binary files") + " (*.csf *.big)",
                                tr("Any files")  + " (*)"});
    connect(fileDialog, &QFileDialog::fileSelected, lneFilePath, &QLineEdit::setText);
    
    QPushButton* btnReview = new QPushButton(tr("REVIEW"));
    btnReview->setObjectName(nameof(btnReview));
    btnReview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(btnReview, &QPushButton::clicked, fileDialog, &QFileDialog::exec);

    QHBoxLayout* ltReview = new QHBoxLayout();
    ltReview->addWidget(lneFilePath);
    ltReview->addSpacing(5);
    ltReview->addWidget(btnReview);

    QVBoxLayout* ltMain = new QVBoxLayout();
    ltMain->setContentsMargins(80, 0, 80, 0);
    ltMain->setAlignment(Qt::AlignmentFlag::AlignCenter);
    ltMain->addStretch(3);
    ltMain->addWidget(lblSelectFile);
    ltMain->addSpacing(5);
    ltMain->addLayout(ltReview);
    ltMain->addStretch(2);
    ltMain->addLayout(ltBottomButtons);
    ltMain->addStretch(1);

    setLayout(ltMain);
}

void LoadWindow::lneFilePath_textChanged()
{
    QLineEdit* le = qobject_cast<QLineEdit*>(sender());
    QString tmp = le->text();
    
    if (tmp.startsWith("file:///"))
        tmp.remove("file:///");

    while (tmp.startsWith("\"") || tmp.endsWith("\""))
    {
        if (tmp.startsWith("\""))
            tmp.remove(0, 1);

        if (tmp.endsWith("\""))
            tmp.remove(tmp.length() - 1, 1);
    }

    le->setText(tmp);
}
