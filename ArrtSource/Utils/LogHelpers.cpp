#include <App/AppWindow.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <Utils/LogHelpers.h>

void ArrtAppWindow::on_ClearLogButton_clicked()
{
    LogList->clear();

    Tabs->setTabIcon(3, QIcon());
    m_maxLogType = QtDebugMsg;
}

void ArrtAppWindow::LogMessageHandlerStatic(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // we don't want to show Qt messages
    if (strcmp(context.category, "default") == 0)
        return;

    QString category = QString(context.category);

    QMetaObject::invokeMethod(ArrtAppWindow::s_instance, [type, category, msg]
                              { ArrtAppWindow::s_instance->LogMessageHandler(type, category, msg); });
}

void ArrtAppWindow::LogMessageHandler(QtMsgType type, const QString& category, const QString& msg)
{
    QString line;

    line += QString("%1: ").arg(category);
    line += msg;

    QListWidgetItem* item = new QListWidgetItem();
    item->setText(line);

    QtMsgType prevIcon = m_maxLogType;

    switch (type)
    {
        case QtDebugMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/debug.svg"));
            break;
        case QtWarningMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/warning.svg"));
            m_maxLogType = std::max(m_maxLogType, QtWarningMsg);
            break;
        case QtCriticalMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/critical.svg"));
            m_maxLogType = std::max(m_maxLogType, QtCriticalMsg);
            break;
        case QtFatalMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/error.svg"));
            m_maxLogType = std::max(m_maxLogType, QtFatalMsg);
            break;
        case QtInfoMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/info.svg"));
            m_maxLogType = std::max(m_maxLogType, QtInfoMsg);
            break;
    }

    if (prevIcon != m_maxLogType)
    {

        switch (type)
        {
            case QtWarningMsg:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/warning.svg"));
                break;
            case QtCriticalMsg:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/critical.svg"));
                break;
            case QtFatalMsg:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/error.svg"));
                break;
            case QtInfoMsg:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/info.svg"));
                break;
        }
    }

    LogList->addItem(item);
}

QLoggingCategory LoggingCategory::ArrSdk(QT_TR_NOOP("ARR SDK"));
QLoggingCategory LoggingCategory::RenderingSession(QT_TR_NOOP("SESSION"));
QLoggingCategory LoggingCategory::AzureStorage(QT_TR_NOOP("STORAGE"));

// Wrapper around QJsonObject to prettify output to QDebug.
// QJsonObject already has implementation of stream to QDebug which serialize object
// in non-indented format and with special characters escaping.
struct PrettyJson
{
    PrettyJson(const QJsonObject& jsonObj)
        : m_jsonObj(jsonObj)
    {
    }

    const QJsonObject& m_jsonObj;
};

QDebug& operator<<(QDebug& logger, const PrettyJson& prettyJson)
{
    QDebugStateSaver ss(logger);
    logger.noquote() << QJsonDocument(prettyJson.m_jsonObj).toJson(QJsonDocument::Indented);
    return logger;
}

QString toString(const RR::RenderingSessionVmSize& vmSize)
{
    switch (vmSize)
    {
        case RR::RenderingSessionVmSize::None:
            return QCoreApplication::tr("None");
        case RR::RenderingSessionVmSize::Premium:
            return QCoreApplication::tr("Premium");
        case RR::RenderingSessionVmSize::Standard:
            return QCoreApplication::tr("Standard");
    }
    return QCoreApplication::tr("Unknown");
}

QString toString(const RR::ConnectionStatus& connectionStatus)
{
    switch (connectionStatus)
    {
        case RR::ConnectionStatus::Disconnected:
            return QCoreApplication::tr("Disconnected");
        case RR::ConnectionStatus::Connecting:
            return QCoreApplication::tr("Connecting");
        case RR::ConnectionStatus::Connected:
            return QCoreApplication::tr("Connected");
    }
    return QCoreApplication::tr("ConnectionStatus:Unknown");
}

QString toString(const RR::RenderingSessionStatus& sessionStatus)
{
    switch (sessionStatus)
    {
        case RR::RenderingSessionStatus::Unknown:
            return QCoreApplication::tr("Unknown");
        case RR::RenderingSessionStatus::Starting:
            return QCoreApplication::tr("Starting");
        case RR::RenderingSessionStatus::Ready:
            return QCoreApplication::tr("Ready");
        case RR::RenderingSessionStatus::Stopped:
            return QCoreApplication::tr("Stopped");
        case RR::RenderingSessionStatus::Expired:
            return QCoreApplication::tr("Expired");
        case RR::RenderingSessionStatus::Error:
            return QCoreApplication::tr("Error");
    }
    return QCoreApplication::tr("SessionStatus:Unknown");
}


QString toString(const RR::Status& arrStatus)
{
    if (arrStatus >= RR::Status::CoreReturnValueStart)
    {
        return RR::ResultToString((RR::Result)((uint32_t)arrStatus - 1 - (uint32_t)RR::Status::CoreReturnValueStart));
    }
    else
    {
        switch (arrStatus)
        {
            case RR::Status::OK:
                return "OK";
            case RR::Status::Failed:
                return "Failed";
            case RR::Status::ObjectDisposed:
                return "ObjectDisposed";
            case RR::Status::OutOfMemory:
                return "OutOfMemory";
            case RR::Status::InvalidArgument:
                return "InvalidArgument";
            case RR::Status::OutOfRange:
                return "OutOfRange";
            case RR::Status::NotImplemented:
                return "NotImplemented";
            case RR::Status::KeyNotFound:
                return "KeyNotFound";
            default:
                return (int)arrStatus;
        }
    }
}

QDebug& operator<<(QDebug& logger, const RR::Status& arrStatus)
{
    logger << toString(arrStatus).toUtf8().data();
    return logger;
}

QDebug& operator<<(QDebug& logger, const RR::Result& arrResult)
{
    logger << RR::ResultToString(arrResult);
    return logger;
}

QDebug& operator<<(QDebug& logger, const std::string& str)
{
    logger << QString::fromStdString(str);
    return logger;
}

QDebug& operator<<(QDebug& logger, const std::wstring& str)
{
    logger << QString::fromStdWString(str);
    return logger;
}

QDebug& operator<<(QDebug& logger, const RR::SessionGeneralContext& context)
{
    QJsonObject contextInfo;
    contextInfo[QLatin1String("result")] = RR::ResultToString(context.Result);
    contextInfo[QLatin1String("http_response_code")] = (int)context.HttpResponseCode;
    contextInfo[QLatin1String("error_message")] = QString::fromStdString(context.ErrorMessage);
    contextInfo[QLatin1String("request_correlation_vector")] = QString::fromStdString(context.RequestCorrelationVector);
    contextInfo[QLatin1String("response_correlation_vector")] = QString::fromStdString(context.ResponseCorrelationVector);
    return logger << QCoreApplication::tr("Session context: ") << PrettyJson(contextInfo);
}

QDebug& operator<<(QDebug& logger, const RR::RenderingSessionProperties& properties)
{
    QJsonObject sessionProperties;
    sessionProperties[QLatin1String("status")] = toString(properties.Status);
    sessionProperties[QLatin1String("size")] = toString(properties.Size);
    sessionProperties[QLatin1String("hostname")] = QString::fromStdString(properties.Hostname);
    sessionProperties[QLatin1String("message")] = QString::fromStdString(properties.Message);
    sessionProperties[QLatin1String("size_string")] = QString::fromStdString(properties.SizeString);
    sessionProperties[QLatin1String("id")] = QString::fromStdString(properties.Id);
    sessionProperties[QLatin1String("elapsed_time")] = QString("%1:%2").arg(properties.ElapsedTimeInMinutes / 60).arg(properties.ElapsedTimeInMinutes % 60);
    sessionProperties[QLatin1String("max_lease")] = QString("%1:%2").arg(properties.MaxLeaseInMinutes / 60).arg(properties.MaxLeaseInMinutes % 60);
    return logger << QCoreApplication::tr("Session properties:") << PrettyJson(sessionProperties);
}

QDebug& operator<<(QDebug& logger, const RR::ConnectionStatus& connectionStatus)
{
    return logger << toString(connectionStatus);
}

QDebug& operator<<(QDebug& logger, const RR::RenderingSessionCreationOptions& info)
{
    QJsonObject sessionInfo;
    sessionInfo[QLatin1String("max_lease")] = QString("%1:%2").arg(info.MaxLeaseInMinutes / 60).arg(info.MaxLeaseInMinutes % 60);
    sessionInfo[QLatin1String("vm_size")] = toString(info.Size);
    return logger << QCoreApplication::tr("Create rendering session info:") << PrettyJson(sessionInfo);
}

void qArrSdkMessage(RR::LogLevel logLevel, const void* msgVoid)
{
    const char* msg = static_cast<const char*>(msgVoid);
    const auto& category(LoggingCategory::ArrSdk);

    switch (logLevel)
    {
        case RR::LogLevel::Debug:
            qDebug(category, msg);
            break;
        case RR::LogLevel::Information:
            qInfo(category, msg);
            break;
        case RR::LogLevel::Warning:
            qWarning(category, msg);
            break;
        case RR::LogLevel::Error:
            qCritical(category, msg);
            break;
        case RR::LogLevel::None:
        case RR::LogLevel::Count:
            // nothing
            break;
    }
}
