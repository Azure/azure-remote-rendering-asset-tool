#pragma once
#include <QStyledItemDelegate>

class CustomComboBoxDelegate : public QStyledItemDelegate
{
public:
    CustomComboBoxDelegate(QObject* parent);
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /// return true if the index in the combobox model is a separator
    static bool isSeparator(const QModelIndex& index);
};
