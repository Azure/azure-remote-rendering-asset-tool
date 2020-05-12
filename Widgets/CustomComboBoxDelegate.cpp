#include <Widgets/CustomComboBox.h>
#include <Widgets/CustomComboBoxDelegate.h>

CustomComboBoxDelegate::CustomComboBoxDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QSize CustomComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (isSeparator(index))
    {
        return QSize(0, 7);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

void CustomComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (isSeparator(index))
    {
        QWidget* parentWidget = qobject_cast<QWidget*>(parent());
        assert(parentWidget != nullptr);

        QStyleOptionMenuItem itemOption;
        itemOption.init(parentWidget);
        itemOption.menuItemType = QStyleOptionMenuItem::Separator;
        itemOption.rect = option.rect;

        parentWidget->style()->drawControl(QStyle::CE_MenuItem, &itemOption, painter);
    }
    else if (index.data(Qt::AccessibleDescriptionRole).toString() == CustomComboBox::g_indentTag)
    {
        QStyleOptionViewItem childOption = option;
        const int indent = option.fontMetrics.horizontalAdvance(QString(4, QChar(' ')));
        childOption.rect.adjust(indent, 0, 0, 0);
        QStyledItemDelegate::paint(painter, childOption, index);
    }
    else
    {
        QStyleOptionViewItem parentOption = option;
        if (index.data(Qt::AccessibleDescriptionRole).toString() == CustomComboBox::g_groupTag)
        {
            parentOption.font.setBold(true);
        }

        QStyledItemDelegate::paint(painter, parentOption, index);
    }
}


bool CustomComboBoxDelegate::isSeparator(const QModelIndex& index)
{
    return index.data(Qt::AccessibleDescriptionRole).toString() == CustomComboBox::g_separatorTag;
}
