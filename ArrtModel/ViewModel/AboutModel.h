#pragma once
#include <QObject>

// model for the about view, showing information on ARRT

class AboutModel : public QObject
{
    Q_OBJECT
public:
    AboutModel(QObject* parent = {});

    QIcon getIcon() const;
    QString getTitle() const;
    QString getAppName() const;
    QString getVersion() const;
    QString getCopyrightNotice() const;

    void checkLatestReleases() const;
};
