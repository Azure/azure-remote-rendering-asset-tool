#pragma once
#include <QObject>
#include <ViewModel/Parameters/ParameterModel.h>

class QAbstractItemModel;
class QStandardItemModel;
class Configuration;
class NotificationButtonModel;
class NotificationButtonModelImplementation;

// Model for collecting logs emmitted by QMessageLogger.

class LogModel : public QObject
{
    Q_OBJECT

public:
    LogModel(Configuration* configuration, QObject* parent = {});
    ~LogModel();

    enum MsgSeverity
    {
        Debug = 1 << QtDebugMsg,
        Info = 1 << QtInfoMsg,
        Warning = 1 << QtWarningMsg,
        Critical = 1 << QtCriticalMsg,
    };
    Q_DECLARE_FLAGS(SeverityMask, MsgSeverity);

    NotificationButtonModel* getNotificationButtonModel() const;

    QAbstractItemModel* getLogTableModel() const;

    // number of entries that are not read yet
    int getUnreadEntriesCount() const;

    // Called from UI to filter messages by severity level.
    void enableSeverityLevel(QtMsgType type, bool isEnabled);
    bool isSeverityLevelEnabled(QtMsgType type) const;

    // maximum error type among all of the unread entries
    QtMsgType getUnreadEntryType() const;

    // this is called in the UI so that getUnreadLogEntriesCount will be reset
    void markAllAsRead();

    enum LogColumn
    {
        DateTime = 0,
        Severity,
        Category,
        Message,
        Location,
        NumberOfColumns
    };

    // return the role number for the severity level
    static int getSeverityRole();

    // return textual representation of severity
    static QString severityToString(QtMsgType type);

private:
    void addMessage(const QDateTime& datetime, const QString& category, const QtMsgType type, const QString& msg, const QString& location);

private:
    class ProxyModel;

    ProxyModel* m_proxyModel;
    Configuration* const m_configuration;
    QStandardItemModel* m_logTableModel;
    int m_unreadEntries = 0;
    QtMsgType m_unreadEntryType = QtDebugMsg;
    NotificationButtonModelImplementation* const m_buttonModel = {};

    void updateNotifications();

    static int getSeverityOfMsgType(QtMsgType type);
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LogModel::SeverityMask);
