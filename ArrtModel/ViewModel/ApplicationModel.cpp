#include <ViewModel/ApplicationModel.h>

#include <Model/ArrFrontend.h>
#include <Model/ArrSessionManager.h>
#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/ConversionManager.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <Model/IncludesAzureStorage.h>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QPointer>
#include <QStandardPaths>
#include <QUrl>
#include <ViewModel/AboutModel.h>
#include <ViewModel/Conversion/ConversionPageModel.h>
#include <ViewModel/Log/LogModel.h>
#include <ViewModel/NewVersionModel.h>
#include <ViewModel/Render/RenderPageModel.h>
#include <ViewModel/Session/SessionPanelModel.h>
#include <ViewModel/Settings/SettingsModel.h>
#include <ViewModel/Upload/UploadModel.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma warning(push)
#pragma warning(disable : 4100 4189)
#include <AzureRemoteRendering.inl>
#pragma warning(disable : 4996)
#include <cpprest/http_client.h>
#pragma warning(pop)
#pragma clang diagnostic pop


ApplicationModel::ApplicationModel()
{
    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!appDataRootDir.mkpath(QString(".")))
    {
        return;
    }

    m_configuration = new Configuration(appDataRootDir.absolutePath() + QDir::separator() + QString("config.json"), this);
    m_logModel = new LogModel(m_configuration, this);
    m_frontend = new ArrFrontend(this);
    m_storageManager = new AzureStorageManager(this);
    m_conversionManager = new ConversionManager(m_frontend, m_storageManager, m_configuration, this);
    m_sessionManager = new ArrSessionManager(m_frontend, m_configuration, this);
    m_renderPageModel = new RenderPageModel(m_storageManager, m_sessionManager, m_configuration, this);
    m_uploadModel = new UploadModel(m_storageManager, m_configuration, this);
    m_conversionPageModel = new ConversionPageModel(m_conversionManager, m_storageManager, m_configuration, this);
    m_settingsModel = new SettingsModel(m_configuration, m_frontend, m_storageManager, m_sessionManager, this);

    m_aboutModel = new AboutModel(this);
}

void ApplicationModel::openFileNewIssue() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/issues/new"));
}

void ApplicationModel::openFeedback() const
{
    QDesktopServices::openUrl(QUrl("https://feedback.azure.com/forums/928696-azure-remote-rendering"));
}

void ApplicationModel::openDocumentation() const
{
    QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/blob/master/Documentation/index.md"));
}

void ApplicationModel::openPrivacyStatement() const
{
    QDesktopServices::openUrl(QUrl("https://privacy.microsoft.com/privacystatement"));
}

AboutModel* ApplicationModel::getAboutModel() const
{
    return m_aboutModel;
}

void ApplicationModel::closeApplication()
{
    Q_EMIT closeRequested();
}

ApplicationModel::~ApplicationModel()
{
    // delete in reverse order.
    delete m_settingsModel;
    delete m_conversionPageModel;
    delete m_uploadModel;
    delete m_renderPageModel;
    delete m_sessionManager;
    delete m_conversionManager;
    delete m_storageManager;

    // Frontend has to be after all of the service managers
    delete m_frontend;

    delete m_logModel;
    // m_configuration has to be the last one, because the other classes might try and save the configuration on deletion
    delete m_configuration;
}

UploadModel* ApplicationModel::getUploadModel() const
{
    return m_uploadModel;
}

ConversionPageModel* ApplicationModel::getConversionPageModel() const
{
    return m_conversionPageModel;
}

RenderPageModel* ApplicationModel::getRenderPageModel() const
{
    return m_renderPageModel;
}

LogModel* ApplicationModel::getLogModel() const
{
    return m_logModel;
}

void ApplicationModel::checkNewVersion()
{
    QPointer<ApplicationModel> thisPtr = this;

    using namespace web::http;
    client::http_client client(L"https://api.github.com/repos/Azure/azure-remote-rendering-asset-tool/releases/latest");
    client.request(methods::GET).then([thisPtr](const pplx::task<http_response>& previousTask) {
        // in case of any exception, don't show the dialog
        try
        {
            auto response = previousTask.get();
            if (response.status_code() == status_codes::OK)
            {
                auto json = response.extract_json().get();
                QString newVersion = QString::fromStdWString(json.at(L"tag_name").as_string());
                QString currentVersion(ARRT_VERSION);
                if (currentVersion != newVersion)
                {
                    QMetaObject::invokeMethod(QApplication::instance(), [thisPtr, currentVersion, newVersion]() {
                        if (thisPtr)
                        {
                            NewVersionModel* model = new NewVersionModel(currentVersion, newVersion);
                            Q_EMIT thisPtr->openNewVersionDialogRequested(model);
                        }
                    });
                }
            }
        }
        catch (std::exception& /*e*/)
        {
        }
    });
}
