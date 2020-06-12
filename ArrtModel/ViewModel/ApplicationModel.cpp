#include <Model/ArrFrontend.h>
#include <Model/ArrSessionManager.h>
#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/ConversionManager.h>
#include <QStandardPaths>
#include <ViewModel/ApplicationModel.h>
#include <ViewModel/Conversion/ConversionPageModel.h>
#include <ViewModel/Log/LogModel.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelsPage/ModelsPageModel.h>
#include <ViewModel/Render/RenderPageModel.h>
#include <ViewModel/Session/SessionPanelModel.h>
#include <ViewModel/Settings/SettingsModel.h>
#include <ViewModel/Upload/UploadModel.h>

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
    m_conversionManager = new ConversionManager(m_frontend, m_storageManager, this);
    m_sessionManager = new ArrSessionManager(m_frontend, m_configuration, this);
    m_renderPageModel = new RenderPageModel(m_storageManager, m_sessionManager, m_configuration, this);
    m_uploadModel = new UploadModel(m_storageManager, m_configuration, this);
    m_conversionPageModel = new ConversionPageModel(m_conversionManager, m_storageManager, m_configuration, this);
    m_settingsModel = new SettingsModel(m_configuration, m_frontend, m_storageManager, m_sessionManager, this);
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
