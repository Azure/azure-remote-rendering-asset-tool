#pragma once

#include <Widgets/CustomComboBoxDelegate.h>

#include <QCheckBox>

class CheckComboBox;

// subclass of QCheckBox to override the click behavior.
class CheckBoxItem : public QCheckBox
{
public:
    CheckBoxItem(QWidget* parent);

protected:
    // Allow the check box to be clickable from any position in the widget.
    virtual bool hitButton(const QPoint& /*pos*/) const override { return true; }

    virtual void nextCheckState() override;
};

class CheckComboBoxDelegate : public CustomComboBoxDelegate
{
public:
    CheckComboBoxDelegate(QObject* parent, CheckComboBox* comboBox);

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    virtual bool eventFilter(QObject* receiver, QEvent* event) override;
};
