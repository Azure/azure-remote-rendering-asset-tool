#pragma once

#include <QComboBox>
#include <QStyledItemDelegate>

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    // Enum defining the type of item that was added to the combo box.
    enum class ItemType
    {
        Default,   // Item added with QComboBox::addItem
        Indented,  // Item added with QhkComboBox::addIndentedItem
        Separator, // Item added with QComboBox::insertSeparator
        Group      // Item added with QhkComboBox::addGroupItem
    };

    CustomComboBox(QWidget* parent = nullptr);

    virtual void paintEvent(QPaintEvent* e) override;
    virtual bool event(QEvent* e) override;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

    /// Adds an item that is indented.
    void addIndentedItem(const QString& text, const QVariant& userData = QVariant());
    void addIndentedItem(const QIcon& icon, const QString& text, const QVariant& userData = QVariant());

    /// Adds a non-selectable group item.
    void addGroupItem(const QString& text, const QVariant& userData = QVariant());
    void addGroupItem(const QIcon& icon, const QString& text, const QVariant& userData = QVariant());

    /// Returns the item type, i.e. what kind of item resides at the given index.
    ItemType getItemType(int index) const;
    /// Changes the itemType of the given index.
    void setItemType(int index, ItemType type);

    static const char* g_separatorTag;
    static const char* g_indentTag;
    static const char* g_groupTag;
    static const int g_iconSize;

protected:
    virtual void getOptions(QStyleOptionComboBox* option);

    mutable QSize m_cachedSizeHint;
};
