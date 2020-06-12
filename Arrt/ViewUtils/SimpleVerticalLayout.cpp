#include <QWidget>
#include <Utils/SimpleVerticalLayout.h>

SimpleVerticalLayout::SimpleVerticalLayout(QWidget* parent, bool resizeParent)
    : BaseLayout(parent)
    , m_resizeParent(resizeParent)
{
    setContentsMargins(0, 0, 0, 0);
    setSpacing(2);
}

Qt::Orientations SimpleVerticalLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

bool SimpleVerticalLayout::hasHeightForWidth() const
{
    return true;
}

int SimpleVerticalLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize SimpleVerticalLayout::sizeHint() const
{
    return QSize(m_lastW, doLayout(QRect(0, 0, m_lastW, 0), true));
}

QSize SimpleVerticalLayout::minimumSize() const
{
    return sizeHint();
}

void SimpleVerticalLayout::setGeometry(const QRect& rect)
{
    QRect r = rect;
    doLayout(rect, false);
    r.setHeight(heightForWidth(rect.width()));

    //forces the height of the parent widget to be sizeHint
    if (m_resizeParent)
    {
        parentWidget()->resize(r.size());
    }
    QLayout::setGeometry(r);
}

int SimpleVerticalLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();

    int w = effectiveRect.width();

    int _spacing = 0;
    for (auto iter = m_items.begin(); iter != m_items.end(); ++iter)
    {
        QLayoutItem* item = *iter;
        if (QWidget* wid = item->widget())
        {
            if (wid->isVisibleTo(parentWidget()))
            {
                y += _spacing;
                _spacing = spacing();

                int h = wid->hasHeightForWidth() ? wid->heightForWidth(w) : wid->sizeHint().height();

                if (!testOnly)
                {
                    item->setGeometry(QRect(x, y, w, h));
                }
                y += h;
            }
        }
        else if (QLayout* l = item->layout())
        {
            y += _spacing;
            _spacing = spacing();

            int h = l->hasHeightForWidth() ? l->heightForWidth(w) : l->sizeHint().height();

            if (!testOnly)
            {
                item->setGeometry(QRect(x, y, w, h));
            }
            y += h;
        }
    }

    y += contentsMargins().bottom();

    if (!testOnly)
    {
        m_lastW = rect.width();
    }

    return y;
}

void SimpleVerticalLayout::insertWidget(int idx, QWidget* item)
{
    if (m_items.size() <= idx)
    {
        addWidget(item);
    }
    else
    {
        if (m_items[idx]->widget() != item)
        {
            m_items.insert(idx, new QWidgetItemV2(item));
            addChildWidget(item);
        }
    }
}
