#pragma once
#include <QSplitter>

class CustomHandle;

// custom splitter showing a collapsible handle on first or last widget, and drawing a label+icon when collapsed
class CustomSplitter : public QSplitter
{
public:
    enum WidgetPosition
    {
        FIRST,
        LAST
    };
    CustomSplitter(Qt::Orientation orientation, QWidget* parent);

    // set the panel name/icon for a widget and makes the widget collapsible
    bool makeWidgetCollapsible(WidgetPosition widget, QString panelName, QIcon icon);

    // programmatically collapse a widget
    void setCollapsed(WidgetPosition widgetPosition, bool collapsed);

protected:
    virtual QSplitterHandle* createHandle() override;
    virtual void showEvent(QShowEvent* event) override;

private:
    // true when showEvent has been called
    bool m_showing = false;

    void updateCollapsedWidgets();
    CustomHandle* getHandle(int index);
};
