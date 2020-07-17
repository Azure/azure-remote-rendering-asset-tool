#pragma once
#include <QObject>

// model for the new version view

class NewVersionModel : public QObject
{
    Q_OBJECT
public:
    NewVersionModel(QString currentVersion, QString newVersion, QObject* parent = {});

    QString getTitle() const;
    QString getText() const;
    void goToLatestReleases() const;

private:
    const QString m_currentVersion;
    const QString m_newVersion;
};
