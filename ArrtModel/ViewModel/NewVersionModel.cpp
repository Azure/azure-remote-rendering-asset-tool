#include <QDesktopServices>
#include <QTextStream>
#include <QUrl>
#include <ViewModel/NewVersionModel.h>

NewVersionModel::NewVersionModel(QString currentVersion, QString latestVersion, QObject* parent)
    : QObject(parent)
    , m_currentVersion(std::move(currentVersion))
    , m_latestVersion(std::move(latestVersion))
{
}

QString NewVersionModel::getTitle() const
{
    return tr("New version");
}

QString NewVersionModel::getCurrentVersion() const
{
    return m_currentVersion;
}

QString NewVersionModel::getLatestVersion() const
{
    return m_latestVersion;
}

void NewVersionModel::goToLatestReleases() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/releases/"));
}

void NewVersionModel::downloadLatestRelease() const
{
    QString url = QString("https://github.com/Azure/azure-remote-rendering-asset-tool/releases/download/%1/ARRT.zip").arg(m_latestVersion);
    QDesktopServices::openUrl(QUrl(url));
}
