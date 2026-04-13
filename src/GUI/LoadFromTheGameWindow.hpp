#pragma once
#include <QButtonGroup>
#include <QString>
#include <QWidget>

class LoadFromTheGameWindow : public QWidget
{
    Q_OBJECT
private:
    QButtonGroup* pProfilesGroup = nullptr;
public: // Methods
    LoadFromTheGameWindow(QWidget* parent = nullptr);
    QString GetSelectedProfileId() const;
signals:
    void btnBackClicked();
    void btnStartClicked();
    void selectedProfileChanged(const QString& profileId);
};
