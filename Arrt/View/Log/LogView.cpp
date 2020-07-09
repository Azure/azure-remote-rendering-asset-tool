#include <QFontMetrics>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>
#include <QSplitter>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <Utils/FontOverrideModel.h>
#include <View/ArrtStyle.h>
#include <View/Log/LogView.h>
#include <ViewModel/Log/LogModel.h>
#include <ViewUtils/DpiUtils.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FocusableContainer.h>

Q_DECLARE_METATYPE(QtMsgType);

class LogModelDelegate : public QItemDelegate
{
public:
    LogModelDelegate(QObject* parent)
        : QItemDelegate(parent)
    {
    }

    void setSelectedItem(const QModelIndex& item)
    {
        m_selectedItem = item;
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const override
    {
        QFontMetrics fm(ArrtStyle::s_logModelListFont, option.widget);
        QSize s(20, fm.height());
        return s + QSize(m_margins.left() + m_margins.right(), m_margins.top() + m_margins.bottom());
    }

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        int y = option.rect.bottom();
        const bool current = index.row() == m_selectedItem.row();

        if (current)
        {
            QColor highlightColor = option.palette.highlight().color();
            painter->fillRect(option.rect, highlightColor);

            painter->setPen(QPen(highlightColor.darker(), 1));
            painter->drawLine(QPoint(option.rect.left(), y), QPoint(option.rect.right(), y));

            painter->setPen(option.palette.highlightedText().color());
        }
        else
        {
            painter->setPen(QPen(ArrtStyle::s_listSeparatorColor, 1));
            painter->drawLine(QPoint(option.rect.left(), y), QPoint(option.rect.right(), y));

            painter->setPen(option.palette.text().color());
        }

        if (index.column() == LogModel::LogColumn::Severity)
        {
            //icon
            QVariant sev = index.data(LogModel::getSeverityRole());
            if (sev.isValid())
            {
                const QIcon iconToPaint = LogView::getIconFromMsgType(sev.value<QtMsgType>());
                QRect r(0, 0, DpiUtils::size(14), DpiUtils::size(14));
                const QPixmap pixmap = iconToPaint.pixmap(r.size());
                r.moveCenter(option.rect.center());
                painter->drawPixmap(r, pixmap);
            }
        }
        else
        {
            painter->setFont(QFont(ArrtStyle::s_logModelListFont, option.widget));
            QRect r = option.rect.marginsRemoved(m_margins);
            painter->drawText(r, index.data().toString());
        }
    }

private:
    const QMargins m_margins = {3, 4, 3, 4};
    QPersistentModelIndex m_selectedItem;
};


LogView::LogView(LogModel* model, QWidget* parent)
    : QFrame(parent)
    , m_model(model)
{
    auto* const l = new QVBoxLayout(this);

    {
        auto* const hl = new QHBoxLayout();

        QtMsgType severityLevels[] = {QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg};
        for (QtMsgType severityLevel : severityLevels)
        {
            auto levelString = LogModel::severityToString(severityLevel);
            auto* severityButton = new FlatButton(levelString);
            severityButton->setCheckable(true);
            severityButton->setChecked(m_model->isSeverityLevelEnabled(severityLevel));
            severityButton->setIcon(getIconFromMsgType(severityLevel));
            severityButton->setIconSize(QSize(DpiUtils::size(18), DpiUtils::size(18)));
            severityButton->setToolTip(levelString, tr("Toggle filter for log messages of type %1").arg(levelString));
            hl->addWidget(severityButton);
            connect(severityButton, &QToolButton::toggled, this, [this, severityLevel](bool isToggled) {
                m_model->enableSeverityLevel(severityLevel, isToggled);
            });
        }

        m_buttonSimple = new FlatButton(tr("Simple"));
        m_buttonSimple->setIconSize(QSize(DpiUtils::size(18), DpiUtils::size(18)));
        m_buttonSimple->setToolTip(tr("Compact log view"), tr("Visualize only the columns for category/type/message"));
        m_buttonSimple->setCheckable(true);
        m_buttonSimple->setAutoExclusive(true);
        m_buttonSimple->setIcon(ArrtStyle::s_simpleIcon);

        m_buttonDetailed = new FlatButton(tr("Detailed"));
        m_buttonDetailed->setIconSize(QSize(DpiUtils::size(18), DpiUtils::size(18)));
        m_buttonDetailed->setToolTip(tr("Detailed log view"), tr("Visualize all of the columns"));
        m_buttonDetailed->setCheckable(true);
        m_buttonDetailed->setAutoExclusive(true);
        m_buttonDetailed->setIcon(ArrtStyle::s_detailsIcon);

        connect(m_buttonDetailed, &FlatButton::toggled, this, [this](bool isToggled) {
            setDetailedView(isToggled);
        });

        hl->addStretch(1);
        hl->addWidget(m_buttonSimple, 0);
        hl->addWidget(m_buttonDetailed, 0);

        l->addLayout(hl);
    }

    auto* splitter = new QSplitter(Qt::Vertical);
    l->addWidget(splitter, 1);

    {
        m_logList = new QTreeView();
        auto* const logDelegate = new LogModelDelegate(m_logList);

        m_logList->setItemDelegate(logDelegate);
        splitter->addWidget(new FocusableContainer(m_logList));

        m_logList->setHeaderHidden(true);

        auto* const logListModel = new FontOverrideListModel(ArrtStyle::s_logModelListFont, m_model->getLogTableModel(), m_logList);
        m_logList->setModel(logListModel);

        auto scrollToBottom = [this]() {
            m_logList->scrollToBottom();
        };
        connect(logListModel, &QAbstractItemModel::rowsInserted, this, scrollToBottom);
        connect(logListModel, &QAbstractItemModel::modelReset, this, scrollToBottom);
        connect(logListModel, &QAbstractItemModel::layoutChanged, this, scrollToBottom);

        m_logList->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(m_logList->selectionModel(), &QItemSelectionModel::currentChanged, this, [this, logDelegate](const QModelIndex& current, const QModelIndex& /*previous*/) {
            logDelegate->setSelectedItem(current);
            m_textView->setText(current.sibling(current.row(), LogModel::LogColumn::Message).data().toString());
            m_logList->model()->layoutChanged();
        });

        const int iconColumnSize = DpiUtils::size(18);
        m_logList->header()->setMinimumSectionSize(iconColumnSize);
        m_logList->setColumnWidth(LogModel::LogColumn::Severity, iconColumnSize);
        m_logList->header()->setSectionResizeMode(LogModel::LogColumn::Severity, QHeaderView::Fixed);

        m_logList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }

    {
        m_textView = new QTextEdit(this);
        m_textView->setReadOnly(true);
        splitter->addWidget(new FocusableContainer(m_textView));
    }

    setFrameShape(QFrame::StyledPanel);

    setDetailedView(m_detailedView, true);
}

void LogView::setDetailedView(bool detailed, bool force)
{
    if (m_detailedView != detailed || force)
    {
        m_detailedView = detailed;
        m_logList->setHeaderHidden(!m_detailedView);
        m_logList->header()->setSectionHidden(LogModel::LogColumn::DateTime, !m_detailedView);
        m_logList->header()->setSectionHidden(LogModel::LogColumn::Location, !m_detailedView);

        m_buttonDetailed->setChecked(m_detailedView);
        m_buttonSimple->setChecked(!m_detailedView);
    }
}


QIcon LogView::getIconFromMsgType(QtMsgType type)
{
    switch (type)
    {
        case QtDebugMsg:
            return ArrtStyle::s_debugIcon;
        case QtWarningMsg:
            return ArrtStyle::s_warningIcon;
        case QtCriticalMsg:
            return ArrtStyle::s_criticalIcon;
        case QtFatalMsg:
            return ArrtStyle::s_criticalIcon;
        case QtInfoMsg:
            return ArrtStyle::s_infoIcon;
        default:
            return {}; //no icon for the lowest level
    }
}
