#include <QStylePainter>
#include <QTimer>
#include <Utils/DpiUtils.h>
#include <View/ArrtStyle.h>
#include <Widgets/MainToolbarButton.h>

namespace
{
    const int s_progressInactive = -1;
    const int s_iconSizePc = 75;
    const int s_separationPc = 20;
    const int s_marginsPc = 10;
} // namespace

MainToolbarButton::MainToolbarButton(const QString& name, QWidget* parent)
    : FlatButton(name, parent)
    , m_loadingAnimationTimer(new QTimer(this))
    , m_progress(s_progressInactive)
{
    using namespace std::literals;

    setFont(ArrtStyle::s_mainButtonFont);
    setIconSize(QSize(DpiUtils::size(25), DpiUtils::size(25)));
    setMinimumHeight(DpiUtils::size(40));
    setCheckable(true);
    m_loadingAnimationTimer->setInterval(1000ms / 30);
    auto updateAnimation = [this]() {
        m_loadingAnimationTime++;
        if (m_loadingAnimationTime >= 100)
        {
            m_loadingAnimationTime = 0;
        }
        update();
    };
    connect(m_loadingAnimationTimer, &QTimer::timeout, this, updateAnimation);
}

void MainToolbarButton::setNotifications(std::vector<Notification> notifications)
{
    m_notifications = std::move(notifications);
    update();
}

void MainToolbarButton::startProgressAnimation()
{
    const bool isAnimating = m_loadingAnimationTimer->isActive();
    if (!isAnimating)
    {
        m_loadingAnimationTime = 0;
        m_loadingAnimationTimer->start();
        m_progress = s_progressInactive;
    }
}

void MainToolbarButton::setProgress(int progressPercentage)
{
    stopProgress();
    m_progress = progressPercentage;
    update();
}

void MainToolbarButton::stopProgress()
{
    if (m_loadingAnimationTimer->isActive())
    {
        m_loadingAnimationTimer->stop();
    }
    m_progress = s_progressInactive;
    update();
}

void MainToolbarButton::paintEvent(QPaintEvent* e)
{
    FlatButton::paintEvent(e);

    QStylePainter p(this);
    if (m_loadingAnimationTimer->isActive() || m_progress != s_progressInactive)
    {
        QRect r = contentsRect();
        r.adjust(4, 4, -4, -4);
        QPoint p1 = r.topLeft();
        QPoint p2 = r.topRight();

        QPen pen1(ArrtStyle::s_progressColor, 3);
        QPen pen2(ArrtStyle::s_progressBackgroundColor, 3);
        // inv indicates that the progress bar has inverted colors
        bool inv;
        // f is the progress value from 0 to 1
        float f;

        if (m_loadingAnimationTimer->isActive())
        {
            // if its animating, first divides the 100 frames in two animations from 0-49
            inv = m_loadingAnimationTime >= 50;
            // the two animations have opposite colors (first blue line moving left to right then gray line)
            f = (m_loadingAnimationTime % 50) / 50.0F;
            // make movement of the progress bar accelerate in its 50 frames
            f = f * f * f;
        }
        else
        {
            inv = false;
            f = float(m_progress) / 100;
        }
        QPoint middlePoint(p1.x() + float(p2.x() - p1.x()) * f, p1.y());
        p.setPen(inv ? pen2 : pen1);
        p.drawLine(p1, middlePoint);
        p.setPen(inv ? pen1 : pen2);
        p.drawLine(middlePoint, p2);
    }

    if (m_notifications.size() > 0)
    {
        QPoint topRight = contentsRect().topRight();
        QFontMetrics fm(ArrtStyle::s_notificationFont, this);

        for (auto&& n : m_notifications)
        {
            QSize textSize;
            if (!n.m_string.isEmpty())
            {
                textSize = fm.size(0, n.m_string);
            }
            else
            {
                textSize = QSize(0, fm.height());
            }

            QSize indicatorSize = textSize;
            QPixmap pixmap;
            if (!n.m_icon.isNull())
            {
                // all of the sizes are percentage of the font height
                // create the pixmap and find its size, including the space between the pixmap and the text (if there is text)
                pixmap = n.m_icon.pixmap(fm.height() * s_iconSizePc / 100, fm.height() * s_iconSizePc / 100);

                const int separation = n.m_string.isEmpty() ? 0 : (fm.height() * s_separationPc / 100);
                indicatorSize += QSize(pixmap.width() + separation, 0);
            }

            const int margin = fm.height() * s_marginsPc / 100;
            QMargins margins(margin, margin, margin, margin);

            QRect indicatorRect = QRect({0, 0}, indicatorSize).marginsAdded(margins);

            indicatorRect.moveTopRight(topRight + QPoint(-3, 3));

            p.setBrush(n.m_color);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(indicatorRect, DpiUtils::size(8), DpiUtils::size(8));

            // draw the content
            QRect indicatorRectContent = indicatorRect.marginsRemoved(margins);

            QRect textRect({0, 0}, textSize);
            textRect.moveTopRight(indicatorRectContent.topRight());

            p.setFont(ArrtStyle::s_notificationFont);
            p.setPen(Qt::white);
            p.drawText(textRect, 0, n.m_string);
            if (!pixmap.isNull())
            {
                QRect iconRect;
                iconRect.setSize(pixmap.size());
                iconRect.moveCenter(indicatorRectContent.center());
                iconRect.moveLeft(indicatorRectContent.left());
                p.drawPixmap(iconRect, pixmap);
            }
            topRight = indicatorRect.topLeft();
        }
    }
}
