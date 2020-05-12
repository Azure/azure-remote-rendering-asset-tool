#include <Model/Configuration.h>
#include <Model/Log/LogHelpers.h>
#include <Model/Settings/ArrAccountSettings.h>
#include <Model/Settings/AzureStorageAccountSettings.h>
#include <Model/Settings/CameraSettings.h>
#include <Model/Settings/VideoSettings.h>
#include <QFile>
#include <QMetaEnum>
#include <QTimer>
#include <Utils/JsonUtils.h>

using namespace std::chrono_literals;

namespace
{
    const std::chrono::milliseconds s_saveTimer = 10s;
}

Configuration::Configuration(QString fileName, QObject* parent)
    : QObject(parent)
    , m_fileName(std::move(fileName))
    , m_videoSettings(new VideoSettings(this))
    , m_cameraSettings(new CameraSettings(this))
    , m_arrAccountSettings(new ArrAccountSettings(this))
    , m_azureStorageAccountSettings(new AzureStorageAccountSettings(this))
{
    load();

    QObject::connect(m_videoSettings, &VideoSettings::changed, this, [this]() {
        invalidate();
    });
    QObject::connect(m_cameraSettings, &CameraSettings::changed, this, [this]() {
        invalidate();
    });
    QObject::connect(m_arrAccountSettings, &ArrAccountSettings::changed, this, [this]() {
        invalidate();
    });
    QObject::connect(m_azureStorageAccountSettings, &AzureStorageAccountSettings::changed, this, [this]() {
        invalidate();
    });

    m_saveTimer = new QTimer(this);
    m_saveTimer->setInterval(s_saveTimer);
    connect(m_saveTimer, &QTimer::timeout, this, [this]() {
        if (m_invalidated)
        {
            save();
        }
        else
        {
            m_saveTimer->stop();
        }
    });
}

Configuration::~Configuration()
{
    if (m_invalidated)
    {
        save();
    }
}

const std::string& Configuration::getRunningSession() const
{
    return m_runningSession;
}
void Configuration::setRunningSession(std::string runningSession)
{
    if (runningSession != m_runningSession)
    {
        m_runningSession = std::move(runningSession);
        invalidate();
    }
}

QVariant Configuration::getUiState(const QString& parameter) const
{
    auto it = m_uiStateMap.constFind(parameter);
    if (it == m_uiStateMap.constEnd())
    {
        return {};
    }
    else
    {
        return it.value();
    }
}

void Configuration::setUiState(const QString& parameter, QVariant variant)
{
    m_uiStateMap[parameter] = std::move(variant);
    invalidate();
}

void Configuration::load()
{
    QFile file(m_fileName);
    m_invalidated = false;

    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to load configuration file: %s", qPrintable(m_fileName));
    }

    QByteArray saveData = file.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    qInfo(LoggingCategory::configuration)
        << tr("Current configuration:\n")
        << PrettyJson(loadDoc.object());

    const QJsonObject& root = loadDoc.object();

    {
        QJsonObject storageAccount = root[QLatin1String("storageaccount")].toObject();
        if (!storageAccount.isEmpty())
        {
            m_azureStorageAccountSettings->loadFromJson(storageAccount);
        }
    }
    {
        QJsonObject arrAccount = root[QLatin1String("arraccount")].toObject();
        if (!arrAccount.isEmpty())
        {
            m_arrAccountSettings->loadFromJson(arrAccount);
        }
    }

    {
        m_runningSession = root[QLatin1String("runningsession")].toString().toStdString();
    }

    {
        QJsonObject videoConfig = root[QLatin1String("videoconfig")].toObject();
        if (!videoConfig.isEmpty())
        {
            m_videoSettings->loadFromJson(videoConfig);
        }
    }
    {
        QJsonObject cameraConfig = root[QLatin1String("cameraconfig")].toObject();
        if (!cameraConfig.isEmpty())
        {
            m_cameraSettings->loadFromJson(cameraConfig);
        }
    }

    {
        m_uiStateMap.clear();
        QJsonObject uiStateObject = root[QLatin1String("uistate")].toObject();
        for (auto it = uiStateObject.begin(), end = uiStateObject.end(); it != end; ++it)
        {
            m_uiStateMap.insert(it.key(), it.value().toVariant());
        }
    }
}

void Configuration::invalidate()
{
    m_invalidated = true;
    if (!m_saveTimer->isActive())
    {
        m_saveTimer->start();
    }
}

void Configuration::save()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qFatal("Couldn't open configuration file for write access: %s", qPrintable(m_fileName));
    }

    QJsonObject root;

    root[QLatin1String("arraccount")] = m_arrAccountSettings->saveToJson();
    root[QLatin1String("storageaccount")] = m_azureStorageAccountSettings->saveToJson();
    root[QLatin1String("runningsession")] = QString::fromStdString(m_runningSession);
    root[QLatin1String("videoconfig")] = m_videoSettings->saveToJson();
    root[QLatin1String("cameraconfig")] = m_cameraSettings->saveToJson();

    QJsonObject uiStateObject;
    for (auto it = m_uiStateMap.begin(), end = m_uiStateMap.end(); it != end; ++it)
    {
        uiStateObject[it.key()] = QJsonValue::fromVariant(it.value());
    }
    root[QLatin1String("uistate")] = uiStateObject;

    QJsonDocument saveDoc(root);
    const auto bytesWritten = file.write(saveDoc.toJson());
    if (bytesWritten == -1)
    {
        qFatal("Failed to save configuration.");
    }
    else
    {
        qDebug(LoggingCategory::configuration)
            << "Configuration file has been updated:" << m_fileName;
        m_invalidated = false;
    }
}
