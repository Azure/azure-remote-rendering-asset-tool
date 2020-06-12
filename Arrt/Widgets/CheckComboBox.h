#pragma once

#include <QBitArray>
#include <QCheckBox>
#include <QComboBox>
#include <QStandardItemModel>
#include <Widgets/CustomComboBox.h>

// ComboBox with checkable items.
// To access the checked state of its items, use the role Qt::CheckStateRole in the QComboBox data model
class CheckComboBox : public CustomComboBox
{
    Q_OBJECT

public:
    CheckComboBox(QWidget* widget = nullptr);

    virtual bool eventFilter(QObject* object, QEvent* event) override;

    virtual void showPopup() override;

    Qt::CheckState getCheckState(int index) const;
    void setCheckState(int index, Qt::CheckState state);

    /// Adds an item that can be used to select / deselect all items at once.
    void addSelectAllItem(const QString& text = "All");
    int getSelectAllIndex() const;

Q_SIGNALS:
    /// Signal triggered when the user starts editing data.
    void beforeEdit();
    /// Signal triggered when the user stops editing data (the menu was closed).
    void afterEdit();
    /// Signal triggered when the data of any item has been edited by the user.
    void dataChanged();

protected:
    void startEdit();
    void endEdit();

    virtual void getOptions(QStyleOptionComboBox* opt) override;
    virtual QString getText();

    void saveState();
    void restoreState();

    bool isValidIndex(int index);

private Q_SLOTS:

    void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void onModelRowsInserted(const QModelIndex& parent, int first, int last);

private:
    void updateSelectAllItemState();

    QVector<Qt::CheckState> m_stateBeforeEdit;
    QPersistentModelIndex m_selectAllIndex;

    bool m_viewOpen;
    bool m_edited;
    bool m_inModelDataChanged;
};
