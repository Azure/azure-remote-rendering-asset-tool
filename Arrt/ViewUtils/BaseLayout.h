#pragma once

#include <QLayout>

// base class for a custom layout, based on a QLayoutItem list.

class BaseLayout : public QLayout
{
public:
    BaseLayout(QWidget* parent = nullptr);

    virtual ~BaseLayout();

    virtual void addItem(QLayoutItem* item) override;
    virtual Qt::Orientations expandingDirections() const override;
    virtual int count() const override;
    virtual QLayoutItem* itemAt(int index) const override;
    virtual QLayoutItem* takeAt(int index) override;
    virtual void setGeometry(const QRect& rect) override;

protected:
    QList<QLayoutItem*> m_items;

    virtual int doLayout(const QRect& rect, bool testOnly) const = 0;
};
