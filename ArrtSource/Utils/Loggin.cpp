#include <App/AppWindow.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <Utils/Logging.h>

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

    int prevIcon = m_maxLogType;

    switch (type)
    {
        case QtDebugMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/debug.svg"));
            break;
        case QtInfoMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/info.svg"));
            m_maxLogType = std::max(m_maxLogType, 1);
            break;
        case QtWarningMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/warning.svg"));
            m_maxLogType = std::max(m_maxLogType, 2);
            break;
        case QtCriticalMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/critical.svg"));
            m_maxLogType = std::max(m_maxLogType, 3);
            break;
        case QtFatalMsg:
            item->setIcon(QIcon(":/ArrtApplication/Icons/error.svg"));
            m_maxLogType = std::max(m_maxLogType, 4);
            break;
    }

    if (prevIcon != m_maxLogType)
    {
        switch (m_maxLogType)
        {
            case 1:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/info.svg"));
                break;
            case 2:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/warning.svg"));
                break;
            case 3:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/critical.svg"));
                break;
            case 4:
                Tabs->setTabIcon(3, QIcon(":/ArrtApplication/Icons/error.svg"));
                break;
        }
    }

    LogList->addItem(item);
}

QLoggingCategory LoggingCategory::ArrSdk(QT_TR_NOOP("ARR SDK"));
QLoggingCategory LoggingCategory::RenderingSession(QT_TR_NOOP("SESSION"));
QLoggingCategory LoggingCategory::AzureStorage(QT_TR_NOOP("STORAGE"));

void ForwardArrLogMsgToQt(RR::LogLevel logLevel, const void* msgPtr)
{
    const char* msg = static_cast<const char*>(msgPtr);

    switch (logLevel)
    {
        case RR::LogLevel::Debug:
            qDebug(LoggingCategory::ArrSdk, msg);
            break;
        case RR::LogLevel::Information:
            qInfo(LoggingCategory::ArrSdk, msg);
            break;
        case RR::LogLevel::Warning:
            qWarning(LoggingCategory::ArrSdk, msg);
            break;
        case RR::LogLevel::Error:
            qCritical(LoggingCategory::ArrSdk, msg);
            break;
    }
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
                return QString("ARR::Status=%1").arg((int)arrStatus);
        }
    }
}

QString toString(const RR::ConnectionStatus& connectionStatus)
{
    switch (connectionStatus)
    {
        case RR::ConnectionStatus::Disconnected:
            return "Disconnected";
        case RR::ConnectionStatus::Connecting:
            return "Connecting";
        case RR::ConnectionStatus::Connected:
            return "Connected";
    }
    return "ConnectionStatus:Unknown";
}

QString toString(const RR::RenderingSessionStatus& sessionStatus)
{
    switch (sessionStatus)
    {
        case RR::RenderingSessionStatus::Unknown:
            return "Unknown";
        case RR::RenderingSessionStatus::Starting:
            return "Starting";
        case RR::RenderingSessionStatus::Ready:
            return "Ready";
        case RR::RenderingSessionStatus::Stopped:
            return "Stopped";
        case RR::RenderingSessionStatus::Expired:
            return "Expired";
        case RR::RenderingSessionStatus::Error:
            return "Error";
    }
    return "SessionStatus:Unknown";
}

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
    return logger.noquote() << QJsonDocument(prettyJson.m_jsonObj).toJson(QJsonDocument::Indented);
}

QDebug& operator<<(QDebug& logger, const RR::Status& arrStatus)
{
    return logger.noquote() << toString(arrStatus);
}

QDebug& operator<<(QDebug& logger, const RR::Result& arrResult)
{
    return logger.noquote() << RR::ResultToString(arrResult);
}

QDebug& operator<<(QDebug& logger, const RR::SessionGeneralContext& context)
{
    QJsonObject contextInfo;
    contextInfo["result"] = RR::ResultToString(context.Result);
    contextInfo["http_response_code"] = (int)context.HttpResponseCode;
    contextInfo["error_message"] = QString::fromStdString(context.ErrorMessage);
    contextInfo["request_correlation_vector"] = QString::fromStdString(context.RequestCorrelationVector);
    contextInfo["response_correlation_vector"] = QString::fromStdString(context.ResponseCorrelationVector);
    return logger << "Session context: " << PrettyJson(contextInfo);
}

QDebug& operator<<(QDebug& logger, const RR::ConnectionStatus& connectionStatus)
{
    return logger.noquote() << toString(connectionStatus);
}
