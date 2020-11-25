#pragma once

#include <QLayout>


class FlowLayout : public QLayout
{
    Q_OBJECT;

public:
    FlowLayout(QWidget* widget = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    //
    // QLayout Implementation
    //

    void addItem(QLayoutItem* item) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;

    //
    // Get spacing
    //

    int horizontalSpacing() const;
    int verticalSpacing() const;

    void insertWidget(int position, QWidget* widget);

private:
    int doLayout(const QRect& rect, bool testOnly) const;

    QList<QLayoutItem*> m_items;
    int m_hSpace;
    int m_vSpace;

    mutable int m_cachedWidth;
    mutable int m_cachedHeightForWidth;
};
