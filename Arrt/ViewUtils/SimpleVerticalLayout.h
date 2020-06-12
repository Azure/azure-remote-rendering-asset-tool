#pragma once
#include <ViewUtils/BaseLayout.h>

// layout implementation, similar to QVBoxLayout, but enforcing that the items take
// maximum width and expand only vertically

class SimpleVerticalLayout : public BaseLayout
{
public:
    SimpleVerticalLayout(QWidget* parent = nullptr, bool resizeParent = false);

    void insertWidget(int idx, QWidget* item);

    Qt::Orientations expandingDirections() const override;
    virtual bool hasHeightForWidth() const override;
    virtual int heightForWidth(int width) const override;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSize() const override;
    virtual void setGeometry(const QRect& rect) override;

protected:
    virtual int doLayout(const QRect& rect, bool testOnly) const override;
    mutable int m_lastW = 100;
    int m_spacing = 0;
    bool m_resizeParent = false;
};
