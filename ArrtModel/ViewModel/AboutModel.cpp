#include <QDesktopServices>
#include <QIcon>
#include <QUrl>
#include <ViewModel/AboutModel.h>

AboutModel::AboutModel(QObject* parent)
    : QObject(parent)
{
}

QIcon AboutModel::getIcon() const
{
    return QIcon(":/ArrtApplication/Icons/remoterendering.png");
}

QString AboutModel::getTitle() const
{
    return tr("About Azure Remote Rendering Asset Tool");
}

QString AboutModel::getAppName() const
{
    return tr("<h3>Azure Remote Rendering Asset Tool</h3>");
}

QString AboutModel::getVersion() const
{
    return tr("Version") + QString(": %1").arg(QString(ARRT_VERSION));
}

QString AboutModel::getCopyrightNotice() const
{
    return QString::fromUtf8("(C) 2020 Microsoft Corporation");
}

void AboutModel::checkLatestReleases() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/releases/"));
}
