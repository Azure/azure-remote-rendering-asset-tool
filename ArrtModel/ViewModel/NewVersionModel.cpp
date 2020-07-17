#include <QDesktopServices>
#include <QTextStream>
#include <QUrl>
#include <ViewModel/NewVersionModel.h>

NewVersionModel::NewVersionModel(QString currentVersion, QString newVersion, QObject* parent)
    : QObject(parent)
    , m_currentVersion(std::move(currentVersion))
    , m_newVersion(std::move(newVersion))
{
}

QString NewVersionModel::getTitle() const
{
    return tr("About Azure Remote Rendering Asset Tool");
}

QString NewVersionModel::getText() const
{
    return tr("Current version: ") + m_currentVersion + "\n" + tr("Latest Version: ") + m_newVersion;
}

void NewVersionModel::goToLatestReleases() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/releases/"));
}
