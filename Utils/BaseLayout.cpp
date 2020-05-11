#include <Utils/BaseLayout.h>

BaseLayout::BaseLayout(QWidget* parent)
    : QLayout(parent)
{
}

BaseLayout::~BaseLayout()
{
    for (QLayoutItem* item : m_items)
    {
        delete item;
    }
}

void BaseLayout::addItem(QLayoutItem* item)
{
    m_items.append(item);
    if (item && item->layout())
    {
        addChildLayout(item->layout());
    }
}

Qt::Orientations BaseLayout::expandingDirections() const
{
    return nullptr;
}

int BaseLayout::count() const
{
    return m_items.size();
}

QLayoutItem* BaseLayout::itemAt(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items[index] : nullptr;
}

QLayoutItem* BaseLayout::takeAt(int index)
{
    return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr;
}

void BaseLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}
