#pragma once
#include <QObject>

// model for the new version view

class NewVersionModel : public QObject
{
    Q_OBJECT
public:
    NewVersionModel(QString currentVersion, QString latestVersion, QObject* parent = {});

    QString getTitle() const;
    QString getCurrentVersion() const;
    QString getLatestVersion() const;

    void goToLatestReleases() const;
    void downloadLatestRelease() const;

private:
    const QString m_currentVersion;
    const QString m_latestVersion;
};
