#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QToolTip>
#include <QWidget>
#include <View/ArrtStyle.h>

namespace
{
    static QWindow* qt_getWindow(const QWidget* widget)
    {
        return widget ? widget->window()->windowHandle() : 0;
    }
} // namespace

const QColor ArrtStyle::s_listSeparatorColor = QColor(60, 60, 60);
const QColor ArrtStyle::s_debugColor = QColor(180, 180, 0);
const QColor ArrtStyle::s_warningColor = QColor(170, 100, 0);
const QColor ArrtStyle::s_errorColor = QColor(200, 0, 0);
const QColor ArrtStyle::s_infoColor = QColor(0, 40, 200);
const QColor ArrtStyle::s_buttonCheckedColor = QColor(35, 35, 35);
const QColor ArrtStyle::s_buttonUncheckedColor = QColor(63, 63, 63);
const QColor ArrtStyle::s_underTextColor = QColor(200, 200, 200);

const QColor ArrtStyle::s_successColor = Qt::darkGreen;
const QColor ArrtStyle::s_runningColor = Qt::darkYellow;
const QColor ArrtStyle::s_failureColor = Qt::darkRed;
const QColor ArrtStyle::s_progressBackgroundColor = Qt::lightGray;
const QColor ArrtStyle::s_progressColor = Qt::blue;

const QColor ArrtStyle::s_connectedColor = Qt::green;
const QColor ArrtStyle::s_connectingColor = Qt::yellow;
const QColor ArrtStyle::s_disconnectedColor = Qt::red;

const QFont ArrtStyle::s_widgetFont = QFont("Segoe UI", 12);
const QFont ArrtStyle::s_directoryPopupFont = QFont("Segoe UI", 11);
const QFont ArrtStyle::s_blobNameFont = QFont("Segoe UI", 14);
const QFont ArrtStyle::s_blobPathFont = QFont("Segoe UI", 9);
const QFont ArrtStyle::s_blobStatusFont = QFont("Segoe UI", 10);
const QFont ArrtStyle::s_conversionListFont = QFont("Segoe UI", 13);
const QFont ArrtStyle::s_conversionTimeFont = QFont("Segoe UI", 11, -1, true);
const QFont ArrtStyle::s_configurationLabelFont = QFont("Segoe UI", 10);
const QFont ArrtStyle::s_logModelListFont = QFont("Segoe UI", 10);
const QFont ArrtStyle::s_sessionStatusFont = QFont("Segoe UI", 12);
const QFont ArrtStyle::s_sessionTimeFont = QFont("Segoe UI", 10);
const QFont ArrtStyle::s_formHeaderFont = QFont("Segoe UI", 10);
const QFont ArrtStyle::s_notificationFont = QFont("Segoe UI", 8);
const QFont ArrtStyle::s_mainButtonFont = QFont("Segoe UI", 20);

QIcon ArrtStyle::s_expandedIcon;
QIcon ArrtStyle::s_notexpandedIcon;
QIcon ArrtStyle::s_newfolderIcon;
QIcon ArrtStyle::s_showblobsIcon;
QIcon ArrtStyle::s_directorymodeIcon;
QIcon ArrtStyle::s_uploadIcon;
QIcon ArrtStyle::s_parentdirIcon;
QIcon ArrtStyle::s_directoryIcon;
QIcon ArrtStyle::s_modelIcon;
QIcon ArrtStyle::s_newIcon;
QIcon ArrtStyle::s_removeIcon;
QIcon ArrtStyle::s_collapseIcon;
QIcon ArrtStyle::s_expandIcon;
QIcon ArrtStyle::s_startIcon;
QIcon ArrtStyle::s_logIcon;
QIcon ArrtStyle::s_simpleIcon;
QIcon ArrtStyle::s_detailsIcon;
QIcon ArrtStyle::s_debugIcon;
QIcon ArrtStyle::s_warningIcon;
QIcon ArrtStyle::s_criticalIcon;
QIcon ArrtStyle::s_infoIcon;
QIcon ArrtStyle::s_extendtimeIcon;
QIcon ArrtStyle::s_inspectorIcon;
QIcon ArrtStyle::s_stopIcon;
QIcon ArrtStyle::s_autoextendtimeIcon;
QIcon ArrtStyle::s_session_not_activeIcon;
QIcon ArrtStyle::s_session_stoppedIcon;
QIcon ArrtStyle::s_session_expiredIcon;
QIcon ArrtStyle::s_session_errorIcon;
QIcon ArrtStyle::s_session_startingIcon;
QIcon ArrtStyle::s_session_readyIcon;
QIcon ArrtStyle::s_session_connectingIcon;
QIcon ArrtStyle::s_session_connectedIcon;
QIcon ArrtStyle::s_session_stop_requestedIcon;
QIcon ArrtStyle::s_conversionIcon;
QIcon ArrtStyle::s_renderingIcon;
QIcon ArrtStyle::s_settingsIcon;
QIcon ArrtStyle::s_conversion_runningIcon;
QIcon ArrtStyle::s_conversion_succeededIcon;
QIcon ArrtStyle::s_conversion_canceledIcon;
QIcon ArrtStyle::s_conversion_failedIcon;
QIcon ArrtStyle::s_refreshIcon;

ArrtStyle::ArrtStyle()
    : QProxyStyle("Fusion")
{
}

void ArrtStyle::polish(QPalette& pal)
{
    pal.setColor(QPalette::Window, QColor(53, 53, 53));
    pal.setColor(QPalette::WindowText, Qt::white);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    pal.setColor(QPalette::Base, QColor(42, 42, 42));
    pal.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    pal.setColor(QPalette::Text, Qt::white);
    pal.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    pal.setColor(QPalette::Dark, QColor(35, 35, 35));
    pal.setColor(QPalette::Shadow, QColor(20, 20, 20));
    pal.setColor(QPalette::Button, QColor(53, 53, 53));
    pal.setColor(QPalette::ButtonText, Qt::white);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    pal.setColor(QPalette::BrightText, Qt::red);
    pal.setColor(QPalette::Link, QColor(42, 130, 218));
    pal.setColor(QPalette::Highlight, QColor(240, 180, 50));
    pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    pal.setColor(QPalette::HighlightedText, Qt::black);
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    pal.setColor(QPalette::ToolTipBase, pal.base().color());
    pal.setColor(QPalette::ToolTipText, pal.mid().color());

    QToolTip::setPalette(pal);
}

void ArrtStyle::polish(QApplication* app)
{
    app->setFont(s_widgetFont, "QWidget");

    // icons have to be created only after the GuiApplication is created
    s_expandedIcon = QIcon(":/ArrtApplication/Icons/expanded.svg");
    s_notexpandedIcon = QIcon(":/ArrtApplication/Icons/notexpanded.svg");
    s_newfolderIcon = QIcon(":/ArrtApplication/Icons/newfolder.svg");
    s_showblobsIcon = QIcon(":/ArrtApplication/Icons/showblobs.svg");
    s_directorymodeIcon = QIcon(":/ArrtApplication/Icons/directorymode.svg");
    s_uploadIcon = QIcon(":/ArrtApplication/Icons/upload.svg");
    s_parentdirIcon = QIcon(":/ArrtApplication/Icons/parentdir.svg");
    s_directoryIcon = QIcon(":/ArrtApplication/Icons/directory.svg");
    s_modelIcon = QIcon(":/ArrtApplication/Icons/model.svg");
    s_newIcon = QIcon(":/ArrtApplication/Icons/new.svg");
    s_removeIcon = QIcon(":/ArrtApplication/Icons/remove.svg");
    s_collapseIcon = QIcon(":/ArrtApplication/Icons/collapse.svg");
    s_expandIcon = QIcon(":/ArrtApplication/Icons/expand.svg");
    s_logIcon = QIcon(":/ArrtApplication/Icons/log.svg");
    s_simpleIcon = QIcon(":/ArrtApplication/Icons/simple.svg");
    s_detailsIcon = QIcon(":/ArrtApplication/Icons/details.svg");
    s_debugIcon = QIcon(":/ArrtApplication/Icons/debug.svg");
    s_warningIcon = QIcon(":/ArrtApplication/Icons/warning.svg");
    s_criticalIcon = QIcon(":/ArrtApplication/Icons/critical.svg");
    s_infoIcon = QIcon(":/ArrtApplication/Icons/info.svg");
    s_extendtimeIcon = QIcon(":/ArrtApplication/Icons/extendtime.svg");
    s_inspectorIcon = QIcon(":/ArrtApplication/Icons/inspector.svg");
    s_stopIcon = QIcon(":/ArrtApplication/Icons/stop.svg");
    s_autoextendtimeIcon = QIcon(":/ArrtApplication/Icons/autoextendtime.svg");
    s_startIcon = QIcon(":/ArrtApplication/Icons/start.svg");
    s_session_not_activeIcon = QIcon(":/ArrtApplication/Icons/session_not_active.svg");
    s_session_stoppedIcon = QIcon(":/ArrtApplication/Icons/session_stopped.svg");
    s_session_expiredIcon = QIcon(":/ArrtApplication/Icons/session_expired.svg");
    s_session_errorIcon = QIcon(":/ArrtApplication/Icons/session_error.svg");
    s_session_startingIcon = QIcon(":/ArrtApplication/Icons/session_starting.svg");
    s_session_readyIcon = QIcon(":/ArrtApplication/Icons/session_ready.svg");
    s_session_connectingIcon = QIcon(":/ArrtApplication/Icons/session_connecting.svg");
    s_session_connectedIcon = QIcon(":/ArrtApplication/Icons/session_connected.svg");
    s_session_stop_requestedIcon = QIcon(":/ArrtApplication/Icons/session_stop_requested.svg");
    s_conversionIcon = QIcon(":/ArrtApplication/Icons/conversion.svg");
    s_renderingIcon = QIcon(":/ArrtApplication/Icons/rendering.svg");
    s_settingsIcon = QIcon(":/ArrtApplication/Icons/settings.svg");
    s_conversion_runningIcon = QIcon(":/ArrtApplication/Icons/conversion_running.svg");
    s_conversion_succeededIcon = QIcon(":/ArrtApplication/Icons/conversion_succeeded.svg");
    s_conversion_canceledIcon = QIcon(":/ArrtApplication/Icons/conversion_canceled.svg");
    s_conversion_failedIcon = QIcon(":/ArrtApplication/Icons/conversion_failed.svg");
    s_refreshIcon = QIcon(":/ArrtApplication/Icons/refresh.svg");
}

void ArrtStyle::drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* widget) const
{
    if (element == CE_ToolButtonLabel)
    {
        if (const QStyleOptionToolButton* toolbutton = qstyleoption_cast<const QStyleOptionToolButton*>(opt))
        {
            // only fixes the alignment when the button has an icon+text and no arrow.
            // This code is a modified version of qfusionstyle.cpp

            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (!hasArrow && !toolbutton->icon.isNull() && !toolbutton->text.isEmpty() &&
                toolbutton->toolButtonStyle == Qt::ToolButtonTextBesideIcon)
            {
                QRect rect = toolbutton->rect;
                int shiftX = 0;
                int shiftY = 0;
                if (toolbutton->state & (State_Sunken | State_On))
                {
                    shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                    shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
                }

                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;

                QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                QIcon::Mode mode;
                if (!(toolbutton->state & State_Enabled))
                    mode = QIcon::Disabled;
                else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                    mode = QIcon::Active;
                else
                    mode = QIcon::Normal;
                pm = toolbutton->icon.pixmap(qt_getWindow(widget), toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                             mode, state);
                pmSize = pm.size() / pm.devicePixelRatio();

                shiftX += pmSize.width() / 3;

                p->setFont(toolbutton->font);
                QRect pr = rect,
                      tr = rect;
                int alignment = Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;

                if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon)
                {
                    pr.setHeight(pmSize.height() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                    tr.adjust(0, pr.height() - 1, 0, -1);
                    pr.translate(shiftX, shiftY);
                    proxy()->drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                    alignment |= Qt::AlignCenter;
                }
                else
                {
                    pr.setWidth(pmSize.width() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                    tr.adjust(pr.width(), 0, 0, 0);
                    pr.translate(shiftX, shiftY);
                    proxy()->drawItemPixmap(p, QStyle::visualRect(opt->direction, rect, pr), Qt::AlignCenter, pm);
                    alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                }
                tr.translate(shiftX, shiftY);
                const QString text = toolbutton->text; //not eliding
                proxy()->drawItemText(p, QStyle::visualRect(opt->direction, rect, tr), alignment, toolbutton->palette,
                                      toolbutton->state & State_Enabled, text,
                                      QPalette::ButtonText);

                return;
            }
        }
    }

    return QProxyStyle::drawControl(element, opt, p, widget);
}

int ArrtStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint)
    {
        case QStyle::SH_ToolTip_WakeUpDelay:
            return 1000;
        case QStyle::QStyle::SH_ToolTip_FallAsleepDelay:
            return 1000;
        case QStyle::SH_ToolTipLabel_Opacity:
            return 240;
        default:
            return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

void ArrtStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_PanelButtonCommand)
    {
    }
    if (element == QStyle::PE_PanelButtonTool || element == QStyle::PE_PanelButtonCommand)
    {
        QRect r = option->rect.adjusted(0, 1, -1, 0);
        // on a toggle button the checked state is dark and unchecked one is slightly lighter than the background
        QColor rectColor = option->state.testFlag(State_Raised) ? s_buttonUncheckedColor : s_buttonCheckedColor;

        // while mouse is pressed, the button is a bit darker
        if (option->state.testFlag(State_Sunken))
        {
            rectColor = rectColor.darker();
        }

        // on mouse over the button is a bit lighter
        if (option->state.testFlag(State_MouseOver))
        {
            rectColor = rectColor.lighter(120);
        }

        if (option->state.testFlag(State_Raised))
        {
            painter->setPen(Qt::NoPen);
        }
        else
        {
            // checked toggle buttons have a lighter border
            painter->setPen(option->palette.mid().color());
        }
        painter->setBrush(rectColor);

        painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 5.0, 5.0);
        return;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QLabel* ArrtStyle::createHeaderLabel(const QString& title, const QString& text)
{
    QString t;
    if (!title.isEmpty())
    {
        t = QString("<h3>%1</h3>").arg(title);
    }
    if (!text.isEmpty())
    {
        t += "<small>" + text;
    }
    auto* label = new QLabel(t);
    label->setWordWrap(true);
    return label;
}

QString ArrtStyle::formatToolTip(const QString& title, const QString& details)
{
    return QString("<font color=\'white\'><b>%1</b></font><br>%2").arg(title).arg(details);
}
