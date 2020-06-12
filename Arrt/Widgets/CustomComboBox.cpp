#include <Widgets/CustomComboBox.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QStandardItemModel>
#include <QStyleOption>
#include <QStylePainter>
#include <Widgets/CustomComboBoxDelegate.h>

const char* CustomComboBox::g_separatorTag = "separator";
const char* CustomComboBox::g_indentTag = "indent";
const char* CustomComboBox::g_groupTag = "group";
const int CustomComboBox::g_iconSize = 16;

CustomComboBox::CustomComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setIconSize(QSize(g_iconSize, g_iconSize));
    view()->setItemDelegate(new CustomComboBoxDelegate(view()));

    auto onModelChanged = [this] { m_cachedSizeHint = QSize(); };

    QObject::connect(model(), &QAbstractItemModel::rowsInserted, this, onModelChanged);
    QObject::connect(model(), &QAbstractItemModel::rowsRemoved, this, onModelChanged);
    QObject::connect(model(), &QAbstractItemModel::modelReset, this, onModelChanged);
}

void CustomComboBox::paintEvent(QPaintEvent*)
{
    //exactly like QComboBox::paintEvent. The only difference is that the options are initialized by a virtual function
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focus rect and selected etc.
    QStyleOptionComboBox opt;
    getOptions(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

bool CustomComboBox::event(QEvent* e)
{
    if (e->type() == QEvent::Wheel)
    {
        //if the combobox doesn't have the focus, it is transparent to the wheel event
        if (!hasFocus())
        {
            return false;
        }
    }
    return QComboBox::event(e);
}

void CustomComboBox::getOptions(QStyleOptionComboBox* option)
{
    initStyleOption(option);
    //leaves a small margin
    option->rect.adjust(0, 1, 0, -1);

    //override the text color
    option->palette.setColor(QPalette::ButtonText, palette().color(QPalette::Text));
    option->palette.setColor(QPalette::Disabled, QPalette::ButtonText, palette().color(QPalette::Disabled, QPalette::Text));
}

QSize CustomComboBox::sizeHint() const
{
    if (m_cachedSizeHint.isEmpty())
    {
        m_cachedSizeHint = QComboBox::sizeHint();

        // Check if there are any indented items.
        for (int i = 0; i < count(); i++)
        {
            if (itemData(i, Qt::AccessibleDescriptionRole) == g_indentTag)
            {
                const int indent = fontMetrics().horizontalAdvance(QString(4, QChar(' ')));
                m_cachedSizeHint = QSize(m_cachedSizeHint.width() + indent, m_cachedSizeHint.height());
                break;
            }
        }
    }

    return m_cachedSizeHint;
}

QSize CustomComboBox::minimumSizeHint() const
{
    return QSize(50, 20);
}

void CustomComboBox::addIndentedItem(const QString& text, const QVariant& userData)
{
    addIndentedItem(QIcon(), text, userData);
}

void CustomComboBox::addIndentedItem(const QIcon& icon, const QString& text, const QVariant& userData)
{
    const int index = count();
    addItem(icon, text, userData);
    setItemData(index, g_indentTag, Qt::AccessibleDescriptionRole);
}

void CustomComboBox::addGroupItem(const QString& text, const QVariant& userData)
{
    addGroupItem(QIcon(), text, userData);
}

void CustomComboBox::addGroupItem(const QIcon& icon, const QString& text, const QVariant& userData)
{
    const int index = count();
    addItem(icon, text, userData);
    setItemType(index, ItemType::Group);
}

CustomComboBox::ItemType CustomComboBox::getItemType(int index) const
{
    assert(index >= 0 && index < count());
    QString role = itemData(index, Qt::AccessibleDescriptionRole).toString();
    if (role == g_separatorTag)
    {
        return ItemType::Separator;
    }
    else if (role == g_indentTag)
    {
        return ItemType::Indented;
    }
    else if (role == g_groupTag)
    {
        return ItemType::Group;
    }
    return ItemType::Default;
}

void CustomComboBox::setItemType(int index, ItemType type)
{
    assert(index >= 0 && index < count());
    bool selectable = false;
    switch (type)
    {
        case CustomComboBox::ItemType::Default:
        {
            setItemData(index, QVariant(), Qt::AccessibleDescriptionRole);
            selectable = true;
        }
        break;
        case CustomComboBox::ItemType::Indented:
        {
            setItemData(index, g_indentTag, Qt::AccessibleDescriptionRole);
            selectable = true;
        }
        break;
        case CustomComboBox::ItemType::Separator:
        {
            setItemData(index, g_separatorTag, Qt::AccessibleDescriptionRole);
        }
        break;
        case CustomComboBox::ItemType::Group:
        {
            setItemData(index, g_groupTag, Qt::AccessibleDescriptionRole);
        }
        break;
    }

    if (auto* m = qobject_cast<QStandardItemModel*>(model()))
    {
        QModelIndex modelIndex = model()->index(index, modelColumn());
        QStandardItem* item = m->itemFromIndex(modelIndex);
        item->setSelectable(selectable);
        item->setEnabled(selectable);
    }
}
