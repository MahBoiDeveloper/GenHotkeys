#pragma once
#include <QWidget>

class LoadWindow : public QWidget
{
    Q_OBJECT
public: // Methods
    LoadWindow(QWidget *parent = nullptr);
signals:
    void btnBackClicked();
    void btnStartClicked();
    void btnFromGameClicked();
private slots:
    void lneFilePath_textChanged();
};
