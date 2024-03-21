#include <Utils/Logging.h>

#include <App/AppWindow.h>

#include <QAccessible>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <map>

void ScreenReaderAlert(const char* state, const char* announcement)
{
    if (!QAccessible::isActive())
        return;

    QWidget* focusedWidget = QApplication::focusWidget();
    if (focusedWidget == nullptr)
        return;

    if (announcement == nullptr)
        announcement = "";

    static std::map<std::string, std::string> lastAnnouncement;

    // don't announce anything twice
    if (lastAnnouncement[state] == announcement)
        return;

    lastAnnouncement[state] = announcement;

    if (strcmp(announcement, "") == 0)
        return;

    QAccessibleValueChangeEvent ev(focusedWidget, announcement);
    QAccessible::updateAccessibility(&ev);
}

void ArrtAppWindow::on_ClearLogButton_clicked()
{
    LogTab->LogList->clear();
    LogTab->LogList->addItem("The log has been cleared."); // for accessibility reasons always have one item in the log
    ScreenReaderAlert("Logging", "The log has been cleared.");
    m_logClearMsgAdded = true;

    Tabs->setTabIcon(3, QIcon());
    m_maxLogType = QtDebugMsg;
}

void ArrtAppWindow::on_ExportLogButton_clicked()
{
    bool exported = true;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log File"),
                                                    "",
                                                    tr("Text files (*.txt)"));

    if (fileName.isEmpty() || fileName.isNull())
    {
        // the user closed the dialog
        return;
    }

    QSaveFile fileOut(fileName);
    exported &= fileOut.open(QIODeviceBase::WriteOnly);
    QTextStream out(&fileOut);
    for (int i = 0; i < LogTab->LogList->count(); i++)
    {
        out << LogTab->LogList->item(i)->text() << Qt::endl;
    }
    exported &= fileOut.commit();

    if (exported)
    {
        ScreenReaderAlert("Logging", "The log file has been exported.");
        qInfo(LoggingCategory::Logging) << QString("Exported log to file %1").arg(fileName);
    }
    else
    {
        ScreenReaderAlert("Logging", "Failed to export log to file.");
        qCritical(LoggingCategory::Logging) << QString("Failed to export log to file %1").arg(fileName);
    }
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

void ArrtAppWindow::LogMessageHandler(QtMsgType type, const QString& /*category*/, const QString& msg)
{
    QString line = QDateTime::currentDateTime().toString("[dd.MM.yyyy hh:mm:ss.z tt] ");

    QListWidgetItem* item = new QListWidgetItem();

    int prevIcon = m_maxLogType;

    switch (type)
    {
        case QtDebugMsg:
            item->setIcon(QIcon::fromTheme("debug"));
            line += "Debug: ";
            break;
        case QtInfoMsg:
            item->setIcon(QIcon::fromTheme("info"));
            m_maxLogType = std::max(m_maxLogType, 1);
            line += "Info: ";
            break;
        case QtWarningMsg:
            item->setIcon(QIcon::fromTheme("warning"));
            m_maxLogType = std::max(m_maxLogType, 2);
            line += "Warning: ";
            break;
        case QtCriticalMsg:
            item->setIcon(QIcon::fromTheme("critical"));
            m_maxLogType = std::max(m_maxLogType, 3);
            line += "Critical: ";
            break;
        case QtFatalMsg:
            item->setIcon(QIcon::fromTheme("error"));
            m_maxLogType = std::max(m_maxLogType, 4);
            line += "Error: ";
            break;
    }

    //line += QString("%1: ").arg(category);
    line += msg;

    item->setText(line);

    if (prevIcon != m_maxLogType)
    {
        switch (m_maxLogType)
        {
            case 1:
                Tabs->setTabIcon(3, QIcon::fromTheme("info"));
                break;
            case 2:
                Tabs->setTabIcon(3, QIcon::fromTheme("warning"));
                break;
            case 3:
                Tabs->setTabIcon(3, QIcon::fromTheme("critical"));
                break;
            case 4:
                Tabs->setTabIcon(3, QIcon::fromTheme("error"));
                break;
        }
    }

    if (m_logClearMsgAdded)
    {
        m_logClearMsgAdded = false;
        LogTab->LogList->clear();
    }

    LogTab->LogList->addItem(item);
}

QLoggingCategory LoggingCategory::ArrSdk(QT_TR_NOOP("ARR SDK"));
QLoggingCategory LoggingCategory::RenderingSession(QT_TR_NOOP("SESSION"));
QLoggingCategory LoggingCategory::AzureStorage(QT_TR_NOOP("STORAGE"));
QLoggingCategory LoggingCategory::Logging(QT_TR_NOOP("LOGGING"));


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
    return RR::ResultToString(RR::StatusToResult(arrStatus));
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
