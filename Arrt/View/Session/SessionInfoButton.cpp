#include <QEvent>
#include <QPalette>
#include <QStyleOption>
#include <QStylePainter>
#include <QVBoxLayout>
#include <Utils/DpiUtils.h>
#include <View/ArrtStyle.h>
#include <View/Session/SessionInfoButton.h>
#include <ViewModel/Session/SessionPanelModel.h>
#include <Widgets/TimeValidator.h>

namespace
{
    // all of these are in percentage compared to the font height
    const int s_marginsPcX = 25;
    const int s_marginsPcY = 0;
    const int s_iconPcSize = 65;
    const int s_expandAreaPcX = 120;
    const int s_expandIconPcSize = 60;
    const int s_timeUiMarginsPcX = 20;
} // namespace

SessionInfoButton::SessionInfoButton(SessionPanelModel* model, QWidget* parent)
    : FlatButton("", parent)
    , m_model(model)
{
    setContentsMargins(0, 0, 0, 0);

    QObject::connect(m_model, &SessionPanelModel::sessionChanged, this, [this]() {
        update();
        setMinimumSize(minimumSizeHint());
    });
    setMinimumSize(minimumSizeHint());
    setCheckable(true);
}

void SessionInfoButton::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::RenderHint::Antialiasing);
    p.translate(0.5, 0.5);

    {
        // draw background like a button
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        if (opt.state & (QStyle::State_Sunken | QStyle::State_MouseOver))
        {
            QRect r = rect();
            QColor rectColor = ArrtStyle::s_buttonUncheckedColor;

            if (opt.state.testFlag(QStyle::State_Sunken))
            {
                rectColor = rectColor.darker();
            }

            // on mouse over the button is a bit lighter
            if (opt.state.testFlag(QStyle::State_MouseOver))
            {
                rectColor = rectColor.lighter(120);
            }

            p.setPen(Qt::NoPen);
            p.setBrush(rectColor);
            p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 8.0, 8.0);
        }
    }

    QRect r = rect();

    const QFontMetrics fm(ArrtStyle::s_sessionStatusFont, this);
    const QFontMetricsF fmTime(ArrtStyle::s_sessionTimeFont, this);

    const int marginsX = fm.height() * s_marginsPcX / 100;
    const int marginsY = fm.height() * s_marginsPcY / 100;
    const int arrowSpace = fm.height() * s_expandAreaPcX / 100;
    const int separator = fm.horizontalAdvance(' ');
    const int iconSize = fm.height() * s_iconPcSize / 100.0;
    const int expandIconSize = fm.height() * s_expandIconPcSize / 100.0;
    const int timeMargin = fm.height() * s_timeUiMarginsPcX / 100.0;

    const int timeAreaWidth = timeMargin + iconSize + separator + fmTime.horizontalAdvance(getTimeString()) + timeMargin;

    r.adjust(marginsX, marginsY, 0, -marginsY);
    QRect outRect;

    p.setPen(palette().text().color());
    p.setFont(ArrtStyle::s_sessionStatusFont);
    p.drawText(r, 0, getString(), &outRect);


    QRectF timeBoundingRect = fmTime.boundingRect(getTimeString());

    QRect timeIconBox = rect().adjusted(0, 2, 0, -2);
    timeIconBox.setWidth(timeAreaWidth);
    timeIconBox.moveRight(r.right() - arrowSpace);

    p.setPen(Qt::NoPen);
    p.setBrush(ArrtStyle::s_buttonCheckedColor);
    p.drawRoundedRect(timeIconBox, 8.0, 8.0);

    QRect iconRect;
    iconRect.setSize(QSize(iconSize, iconSize));
    iconRect.moveCenter(timeIconBox.center());
    iconRect.moveLeft(timeIconBox.left() + timeMargin);
    p.drawPixmap(iconRect, getIconFromStatus(m_model->getStatus()).pixmap(iconRect.size()));

    QRect timeRect = r;
    timeRect.setLeft(iconRect.right() + 1 + timeMargin);
    timeRect.setRight(timeIconBox.right() - timeMargin);

    // this is to precisely center the text vertically. The text boundingRect, which is used to align the text,
    // includes the space for any character in the font, but we want to align only the drawn characters.
    // To work around that, we use tightBoundingRect (the bounding rect of the drawn pixels), and move the
    // main bounding rect so that the tight bounding rect is aligned with the center of the control.
    QRectF timeTightRect = fmTime.tightBoundingRect(getTimeString()).adjusted(0, 0, -1, -1);

    QPointF diff = timeRect.center() - timeTightRect.center() + QPoint(0, 1);
    timeBoundingRect.translate(diff);

    p.setPen(palette().text().color());
    p.setFont(ArrtStyle::s_sessionTimeFont);
    p.drawText(timeBoundingRect, Qt::AlignTop, getTimeString());

    QRect expandIconArea = r;
    expandIconArea.setLeft(timeIconBox.right() + 1);
    QRect expandIcon;
    expandIcon.setSize(QSize(expandIconSize, expandIconSize));
    expandIcon.moveCenter(expandIconArea.center());
    p.drawPixmap(expandIcon, (isChecked() ? ArrtStyle::s_collapseIcon : ArrtStyle::s_expandIcon).pixmap(expandIcon.width(), expandIcon.height()));
}


QSize SessionInfoButton::sizeHint() const
{
    QFontMetrics fmStatus(ArrtStyle::s_sessionStatusFont, this);
    QFontMetrics fmTime(ArrtStyle::s_sessionTimeFont, this);
    const int iconSize = fmStatus.height() * s_iconPcSize / 100;
    const int separator = fmStatus.horizontalAdvance(' ');
    const int marginsX = fmStatus.height() * s_marginsPcX / 100;
    const int marginsY = fmStatus.height() * s_marginsPcY / 100;
    const int arrowSpace = fmStatus.height() * s_expandAreaPcX / 100;
    const int timeMargin = fmStatus.height() * s_timeUiMarginsPcX / 100;
    const int timeAreaWidth = timeMargin + iconSize + separator + fmTime.horizontalAdvance(getTimeString()) + timeMargin;

    return QSize(marginsX + fmStatus.horizontalAdvance(getString()) + separator + timeAreaWidth + arrowSpace, marginsY + fmStatus.height() + marginsY);
}

QSize SessionInfoButton::minimumSizeHint() const
{
    return sizeHint();
}

QString SessionInfoButton::getString() const
{
    return tr("Session status: %1").arg(getStringFromStatus(m_model->getStatus()));
}

QString SessionInfoButton::getTimeString() const
{
    return m_model->getRemainingTime();
}

QString SessionInfoButton::getStringFromStatus(SessionPanelModel::Status status)
{
    switch (status)
    {
        case SessionPanelModel::Status::NotActive:
            return tr("Not Active");
        case SessionPanelModel::Status::Stopped:
            return tr("Stopped");
        case SessionPanelModel::Status::Expired:
            return tr("Expired");
        case SessionPanelModel::Status::Error:
            return tr("Error");
        case SessionPanelModel::Status::StartRequested:
            return tr("Start Requested");
        case SessionPanelModel::Status::Starting:
            return tr("Starting");
        case SessionPanelModel::Status::ReadyNotConnected:
            return tr("Session Ready");
        case SessionPanelModel::Status::ReadyConnecting:
            return tr("Connecting");
        case SessionPanelModel::Status::ReadyConnected:
            return tr("Connected");
        case SessionPanelModel::Status::StopRequested:
            return tr("Stop Requested");
        default:
            return tr("Not Recognized");
    }
}

QIcon SessionInfoButton::getIconFromStatus(SessionPanelModel::Status status)
{
    switch (status)
    {
        case SessionPanelModel::Status::NotActive:
            return ArrtStyle::s_session_not_activeIcon;
        case SessionPanelModel::Status::Stopped:
            return ArrtStyle::s_session_stoppedIcon;
        case SessionPanelModel::Status::Expired:
            return ArrtStyle::s_session_expiredIcon;
        case SessionPanelModel::Status::Error:
            return ArrtStyle::s_session_errorIcon;
        case SessionPanelModel::Status::StartRequested:
            return ArrtStyle::s_session_startingIcon;
        case SessionPanelModel::Status::Starting:
            return ArrtStyle::s_session_startingIcon;
        case SessionPanelModel::Status::ReadyNotConnected:
            return ArrtStyle::s_session_readyIcon;
        case SessionPanelModel::Status::ReadyConnecting:
            return ArrtStyle::s_session_connectingIcon;
        case SessionPanelModel::Status::ReadyConnected:
            return ArrtStyle::s_session_connectedIcon;
        case SessionPanelModel::Status::StopRequested:
            return ArrtStyle::s_session_stop_requestedIcon;
        default:
            return ArrtStyle::s_session_not_activeIcon;
    }
}
