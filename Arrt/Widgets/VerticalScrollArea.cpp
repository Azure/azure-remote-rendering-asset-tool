#include <QScrollBar>
#include <ViewUtils/SimpleVerticalLayout.h>
#include <Widgets/VerticalScrollArea.h>

VerticalScrollArea::VerticalScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);

    QWidget* scrollAreaWidget = new QWidget(this);
    scrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    {
        m_innerContentLayout = new SimpleVerticalLayout(scrollAreaWidget, true);
        m_innerContentLayout->setContentsMargins(0, 0, 0, 0);
    }
    setWidgetResizable(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setFixedWidth(10);

    QScrollArea::setWidget(scrollAreaWidget);
}

QLayout* VerticalScrollArea::getContentLayout()
{
    return m_innerContentLayout;
}

void VerticalScrollArea::resizeEvent(QResizeEvent* event)
{
    QSize contentSize = contentWidgetSizeHint();
    widget()->resize(contentSize);
    QScrollArea::resizeEvent(event);
}

QSize VerticalScrollArea::contentWidgetSizeHint() const
{
    int w = size().width();
    if (verticalScrollBar()->isVisibleTo(this))
    {
        w -= verticalScrollBar()->width() + 1;
    }
    int h = widget()->hasHeightForWidth() ? widget()->heightForWidth(w) : widget()->sizeHint().height();

    return QSize(w, h);
}
