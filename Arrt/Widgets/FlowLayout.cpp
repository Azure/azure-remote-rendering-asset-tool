#include <QStyle>
#include <QWidget>
#include <Widgets/FlowLayout.h>

namespace
{
    static inline int smartSpacing(const FlowLayout* layout, QStyle::PixelMetric pm)
    {
        QObject* parent = layout->parent();
        if (!parent)
        {
            return -1;
        }
        else if (parent->isWidgetType())
        {
            QWidget* pw = static_cast<QWidget*>(parent);
            return pw->style()->pixelMetric(pm, nullptr, pw);
        }
        else
        {
            return static_cast<QLayout*>(parent)->spacing();
        }
    }
} // namespace

FlowLayout::FlowLayout(QWidget* widget, int margin, int hSpacing, int vSpacing)
    : QLayout(widget)
    , m_hSpace(hSpacing)
    , m_vSpace(vSpacing)
    , m_cachedWidth(-1)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem* item;
    while (item = takeAt(0))
    {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem* item)
{
    m_items.append(item);
}

void FlowLayout::insertWidget(int position, QWidget* widget)
{
    if (position >= m_items.size())
    {
        addWidget(widget);
    }
    else
    {
        if (m_items[position]->widget() != widget)
        {
            QLayoutItem* li = new QWidgetItemV2(widget);
            m_items.insert(position, li);
            //addChildWidget is done after the insertion, because it might remove the old item, therefore invalidating "position"
            addChildWidget(widget);
        }
    }
}


int FlowLayout::count() const
{
    return m_items.size();
}

QLayoutItem* FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

QLayoutItem* FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size())
    {
        return m_items.takeAt(index);
    }
    else
    {
        return nullptr;
    }
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    if (m_cachedWidth != width)
    {
        doLayout(QRect(0, 0, width, 0), true);
    }
    return m_cachedHeightForWidth;
}

void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    QSize size;
    int h = 0;
    for (auto iter = m_items.begin(); iter != m_items.end(); ++iter)
    {
        QLayoutItem* item = *iter;
        if (item->widget() && !item->widget()->isVisibleTo(parentWidget()))
        {
            continue;
        }
        h = qMax(item->sizeHint().height(), h);
    }
    h += 2 * margin();
    return QSize(0, h);
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (auto iter = m_items.begin(); iter != m_items.end(); ++iter)
    {
        QLayoutItem* item = *iter;
        if (item->widget() && !item->widget()->isVisibleTo(parentWidget()))
        {
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }

    size += QSize(2 * margin(), 2 * margin());
    return size;
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QList<QLayoutItem*> visibleItems;

    // for laying out we may only take the visible items into consideration
    // it becomes much easier to do this with a copy of the array, containing only the visible items
    for (auto iter = m_items.begin(); iter != m_items.end(); ++iter)
    {
        QLayoutItem* item = *iter;
        if (item->widget() && item->widget()->isVisibleTo(parentWidget()))
        {
            visibleItems.push_back(item);
        }
    }

    auto firstOfRow = visibleItems.begin();
    int lastOffset = -1;

    for (auto iter = visibleItems.begin(); iter != visibleItems.end(); ++iter)
    {
        QLayoutItem* item = *iter;

        QSize itemSize = item->sizeHint();
        if (itemSize.width() > effectiveRect.width())
        {
            //if the single item is bigger than the row, scale it
            if (itemSize.width() > 0)
            {
                itemSize.setWidth(effectiveRect.width());
                if (item->hasHeightForWidth())
                {
                    //adjust the height if this item has an "height for width" layout
                    itemSize.setHeight(item->heightForWidth(itemSize.width()));
                }
            }
            itemSize.boundedTo(item->minimumSize());
        }
        QWidget* wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
        {
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }
        int spaceY = verticalSpacing();
        if (spaceY == -1)
        {
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        int nextX = x + itemSize.width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            if (!testOnly && (alignment() & Qt::AlignHCenter))
            {
                const int w = (x - spaceX) - (*firstOfRow)->geometry().left();
                lastOffset = (effectiveRect.width() - w) / 2;

                for (auto i = firstOfRow; i != iter; ++i)
                {
                    (*i)->setGeometry((*i)->geometry().adjusted(lastOffset, 0, lastOffset, 0));
                }
            }
            firstOfRow = iter;

            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + itemSize.width() + spaceX;

            lineHeight = 0;
        }


        if (!testOnly)
        {
            item->setGeometry(QRect(QPoint(x, y), itemSize));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, itemSize.height());
    }

    if (!testOnly && (alignment() & Qt::AlignHCenter) && !visibleItems.isEmpty())
    {
        if (lastOffset == -1)
        {
            const int w = visibleItems.last()->geometry().right() - visibleItems.first()->geometry().left();
            lastOffset = (effectiveRect.width() - w) / 2;
        }

        for (auto i = firstOfRow; i != visibleItems.end(); ++i)
        {
            (*i)->setGeometry((*i)->geometry().adjusted(lastOffset, 0, lastOffset, 0));
        }
    }

    const int h = y + lineHeight - rect.y() + bottom;

    m_cachedWidth = rect.width();
    m_cachedHeightForWidth = h;
    return h;
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0)
    {
        return m_hSpace;
    }
    else
    {
        return smartSpacing(this, QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0)
    {
        return m_vSpace;
    }
    else
    {
        return smartSpacing(this, QStyle::PM_LayoutVerticalSpacing);
    }
}
