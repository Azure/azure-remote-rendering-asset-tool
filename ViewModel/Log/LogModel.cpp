#include <Model/Configuration.h>
#include <QApplication>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <View/ArrtStyle.h>
#include <ViewModel/Log/LogModel.h>
#include <ViewModel/NotificationButtonModelImplementation.h>
#include <ViewModel/Parameters/ComboBoxModel.h>

Q_DECLARE_METATYPE(QtMsgType);

namespace
{
    // log item, not editable
    class LogItem : public QStandardItem
    {
    public:
        LogItem(const QString& txt)
            : QStandardItem(txt)
        {
            setEditable(false);
        }

        LogItem()
        {
            setEditable(false);
        }
    };
} // namespace

// Proxy model for logs model which provides filtering based on severity mask.
class LogModel::ProxyModel : public QSortFilterProxyModel
{
public:
    ProxyModel(QStandardItemModel* itemModel, QObject* parent)
        : QSortFilterProxyModel(parent)
    {
        setSourceModel(itemModel);
    }

    void setSeverityMask(SeverityMask mask)
    {
        m_severityMask = mask;
        invalidateFilter();
    }

    SeverityMask getSeverityMask()
    {
        return m_severityMask;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        const QModelIndex index = sourceModel()->index(sourceRow, 1, sourceParent);
        const auto severity = sourceModel()->data(index, LogModel::getSeverityRole()).value<QtMsgType>();
        return m_severityMask.testFlag(MsgSeverity(1 << severity));
    }

private:
    LogModel::SeverityMask m_severityMask = MsgSeverity::Info | MsgSeverity::Warning | MsgSeverity::Critical;
};

namespace
{
    LogModel* s_logModelInstance = nullptr;
    QtMessageHandler s_previousMessageHandler = nullptr;
} // namespace

void LogModel::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QDateTime time = QDateTime::currentDateTime();
    QString category = QString(context.category);
    QString location = QString(context.function) + tr(" at ") + QString("%1:%2").arg(context.file).arg(context.line);
    QMetaObject::invokeMethod(QApplication::instance(), [time, type, category, msg, location] {
        if (s_logModelInstance)
        {
            s_logModelInstance->addMessage(time, category, type, msg, location);
        }
    });

    if (s_previousMessageHandler)
    {
        s_previousMessageHandler(type, context, msg);
    }
}

LogModel::LogModel(Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_configuration(configuration)
    , m_buttonModel(new NotificationButtonModelImplementation(this))
{
    m_logTableModel = new QStandardItemModel(0, LogColumn::NumberOfColumns, this);
    m_logTableModel->setHorizontalHeaderLabels(
        {tr("Date"),
         {},
         tr("Category"),
         tr("Message"),
         tr("Location")});
    m_proxyModel = new ProxyModel(m_logTableModel, m_logTableModel);

    m_proxyModel->setSeverityMask(SeverityMask(m_configuration->getUiState("logModel:severityMask", (SeverityMask::Int)SeverityMask(MsgSeverity::Info | MsgSeverity::Warning | MsgSeverity::Critical))));

    connect(m_buttonModel, &NotificationButtonModelImplementation::onVisualizedChanged, this, [this]() {
        updateNotifications();
    });
    updateNotifications();

    if (s_logModelInstance)
    {
        qFatal("Only one instance of LogModel is allowed.");
    }
    s_logModelInstance = this;
    s_previousMessageHandler = qInstallMessageHandler(messageHandler);
}

LogModel::~LogModel()
{
    m_configuration->setUiState("logModel:severityMask", (SeverityMask::Int)m_proxyModel->getSeverityMask());
    qInstallMessageHandler(s_previousMessageHandler);
    s_logModelInstance = nullptr;
    s_previousMessageHandler = nullptr;
}

void LogModel::addMessage(const QDateTime& datetime, const QString& category, const QtMsgType type, const QString& msg, const QString& location)
{
    auto* severityItem = new LogItem();
    severityItem->setAccessibleText(severityToString(type));
    severityItem->setData(QVariant::fromValue(type), getSeverityRole());

    QList<QStandardItem*> items;
    items.append(new LogItem(datetime.toString()));
    items.append(severityItem);
    // the category strings are enumerated in LogHelpers.h. This conversion back from QString to const char* is because it's better to translate
    // the category right before printing it, which implies that the stored category (QString) has to be treated as a key (const char*) for a later translation.
    items.append(new LogItem(tr(qPrintable(category))));
    items.append(new LogItem(msg));
    items.append(new LogItem(location));
    m_logTableModel->appendRow(items);

    if (isSeverityLevelEnabled(type))
    {
        m_unreadEntries++;
        if (getSeverityOfMsgType(type) < getSeverityOfMsgType(m_unreadEntryType))
        {
            m_unreadEntryType = type;
        }
        updateNotifications();
    }
}

int LogModel::getSeverityRole()
{
    return Qt::UserRole + 1;
}

QString LogModel::severityToString(QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:
            return tr("Debug");
        case QtInfoMsg:
            return tr("Info");
        case QtWarningMsg:
            return tr("Warning");
        case QtCriticalMsg:
            return tr("Critical");
        case QtFatalMsg:
            return tr("Fatal");
    }
    return tr("unknown");
}

NotificationButtonModel* LogModel::getNotificationButtonModel() const
{
    return m_buttonModel;
}

QAbstractItemModel* LogModel::getLogTableModel() const
{
    return m_proxyModel;
}

int LogModel::getUnreadEntriesCount() const
{
    return m_unreadEntries;
}

void LogModel::enableSeverityLevel(QtMsgType type, bool isEnabled)
{
    auto mask = m_proxyModel->getSeverityMask();
    mask.setFlag(MsgSeverity(1 << type), isEnabled);
    m_proxyModel->setSeverityMask(mask);
}

bool LogModel::isSeverityLevelEnabled(QtMsgType type) const
{
    const auto mask = m_proxyModel->getSeverityMask();
    return mask.testFlag(MsgSeverity(1 << type));
}

QtMsgType LogModel::getUnreadEntryType() const
{
    return m_unreadEntryType;
}

void LogModel::markAllAsRead()
{
    if (m_unreadEntries > 0)
    {
        m_unreadEntries = 0;
        m_unreadEntryType = QtDebugMsg;
        updateNotifications();
    }
}

int LogModel::getSeverityOfMsgType(QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:
            return 4;
        case QtInfoMsg:
            return 3;
        case QtWarningMsg:
            return 2;
        case QtCriticalMsg:
            return 1;
        case QtFatalMsg:
            return 0;
        default:
            return 5;
    }
}

void LogModel::updateNotifications()
{
    if (m_buttonModel->isVisualized() || m_unreadEntries == 0)
    {
        m_unreadEntries = 0;
        m_unreadEntryType = QtDebugMsg;
        m_buttonModel->setNotifications({});
    }
    else
    {
        typedef NotificationButtonModel::Notification::Type NotificationType;
        NotificationType type = NotificationType::Undefined;
        switch (m_unreadEntryType)
        {
            case QtDebugMsg:
                type = NotificationType::Debug;
                break;
            case QtWarningMsg:
                type = NotificationType::Warning;
                break;
            case QtCriticalMsg:
                type = NotificationType::Error;
                break;
            case QtFatalMsg:
                type = NotificationType::Error;
                break;
            case QtInfoMsg:
                type = NotificationType::Info;
                break;
        }
        m_buttonModel->setNotifications({{type, m_unreadEntries}});
    }
    m_buttonModel->setStatusString(ArrtStyle::formatParameterList({tr("Number of new log entries"), tr("Highest level in new entries")}).arg(m_unreadEntries).arg(severityToString(m_unreadEntryType)));
}
