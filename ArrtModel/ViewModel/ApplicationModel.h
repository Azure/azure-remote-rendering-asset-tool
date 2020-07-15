#pragma once
#include <QObject>
#include <Utils/BoolWithReason.h>
#include <memory>

class AzureStorageManager;
class Configuration;
class ConversionManager;
class ConversionPageModel;
class RenderPageModel;
class LogModel;
class ArrSessionManager;
class ArrFrontend;
class SettingsModel;
class UploadModel;
class AboutModel;

// main model for the whole application, used by ApplicationView

class ApplicationModel : public QObject
{
    Q_OBJECT
public:
    ApplicationModel();
    ~ApplicationModel();

    ApplicationModel(const ApplicationModel&) = delete;
    ApplicationModel& operator=(const ApplicationModel&) = delete;

    UploadModel* getUploadModel() const;
    ConversionPageModel* getConversionPageModel() const;
    RenderPageModel* getRenderPageModel() const;

    LogModel* getLogModel() const;

    SettingsModel* getSettingsModel() const { return m_settingsModel; }

    void openFeedback() const;
    void openDocumentation() const;
    AboutModel* getAboutModel() const;
    void closeApplication();

Q_SIGNALS:
    void closeRequested();

private:
    Configuration* m_configuration = {};
    LogModel* m_logModel = {};
    ArrFrontend* m_frontend = {};
    AzureStorageManager* m_storageManager = {};
    ConversionManager* m_conversionManager = {};
    ArrSessionManager* m_sessionManager = {};
    RenderPageModel* m_renderPageModel = {};

    UploadModel* m_uploadModel = {};
    ConversionPageModel* m_conversionPageModel = {};
    SettingsModel* m_settingsModel = {};

    AboutModel* m_aboutModel;
};
