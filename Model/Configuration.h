#pragma once
#include <QMap>
#include <QObject>

class VideoSettings;
class CameraSettings;
class ArrAccountSettings;
class AzureStorageAccountSettings;
class QTimer;
class QLatin1String;

// Class holding all of the application preferences, persisted as a json file

class Configuration : public QObject
{
    Q_OBJECT

public:
    // build the class by loading it from a json file
    Configuration(QString fileName, QObject* parent);
    ~Configuration();

    // Session ID of the session that was last running, to connect to it when the app is restarted
    const std::string& getRunningSession() const;
    void setRunningSession(std::string runningSession);

    VideoSettings* getVideoSettings() const { return m_videoSettings; }
    CameraSettings* getCameraSettings() const { return m_cameraSettings; }
    ArrAccountSettings* getArrAccountSettings() const { return m_arrAccountSettings; }
    AzureStorageAccountSettings* getAzureStorageAccountSettings() const { return m_azureStorageAccountSettings; }

    QVariant getUiState(const QString& parameter) const;
    void setUiState(const QString& parameter, QVariant variant);

    template <typename T>
    T getUiState(const QString& parameter, const T& defaultValue) const;
    template <typename T>
    void setUiState(const QString& parameter, T value);

private:
    QString m_fileName;
    VideoSettings* const m_videoSettings;
    CameraSettings* const m_cameraSettings;
    ArrAccountSettings* const m_arrAccountSettings;
    AzureStorageAccountSettings* const m_azureStorageAccountSettings;
    QMap<QString, QVariant> m_uiStateMap;
    QTimer* m_saveTimer = {};
    std::string m_runningSession;
    bool m_invalidated = false;

    void load();
    void invalidate();
    void save();
};

template <typename T>
T Configuration::getUiState(const QString& parameter, const T& defaultValue) const
{
    QVariant v = getUiState(parameter);
    if (v.isValid() && v.canConvert<T>())
    {
        return v.value<T>();
    }
    else
    {
        return defaultValue;
    }
}

template <typename T>
void Configuration::setUiState(const QString& parameter, T value)
{
    setUiState(parameter, QVariant::fromValue(std::move(value)));
}
