#pragma once
#include <QButtonGroup>
#include <QString>
#include <QWidget>

class LoadFromTheFileWindow : public QWidget
{
    Q_OBJECT
private:
    QButtonGroup* pProfilesGroup = nullptr;
public: // Methods
    LoadFromTheFileWindow(QWidget *parent = nullptr);
    QString GetSelectedFilePath() const;
    QString GetSelectedProfileId() const;
signals:
    void btnBackClicked();
    void btnStartClicked();
    void selectedProfileChanged(const QString& profileId);
};
