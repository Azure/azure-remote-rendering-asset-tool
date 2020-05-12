#pragma once

#include <QScrollArea>

// Scroll area that only scrolls vertically and takes the whole width

class VerticalScrollArea : public QScrollArea
{
public:
    VerticalScrollArea(QWidget* parent = nullptr);

    QLayout* getContentLayout();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

    QSize contentWidgetSizeHint() const;

private:
    QLayout* m_innerContentLayout;
};
