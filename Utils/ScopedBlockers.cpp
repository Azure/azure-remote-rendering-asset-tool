#include <QWidget>
#include <Utils/ScopedBlockers.h>

ScopedUpdatesDisabled::ScopedUpdatesDisabled(QWidget* widget1, QWidget* widget2, QWidget* widget3, QWidget* widget4, QWidget* widget5, QWidget* widget6)
{
    QWidget* widgets[] = {widget1, widget2, widget3, widget4, widget5, widget6};

    for (size_t i = 0; i < sizeof(widgets) / sizeof(widgets[0]); ++i)
    {
        if (widgets[i] != nullptr && widgets[i]->updatesEnabled())
        {
            m_widgets[i] = widgets[i];
            m_widgets[i]->setUpdatesEnabled(false);
        }
        else
        {
            m_widgets[i] = nullptr;
        }
    }
}

ScopedUpdatesDisabled::~ScopedUpdatesDisabled()
{
    for (QWidget* w : m_widgets)
    {
        if (w != nullptr)
        {
            w->setUpdatesEnabled(true);
        }
    }
}

ScopedBlockSignals::ScopedBlockSignals(QObject* object1, QObject* object2, QObject* object3, QObject* object4, QObject* object5, QObject* object6, QObject* object7, QObject* object8)
{
    QObject* objects[] = {object1, object2, object3, object4, object5, object6, object7, object8};

    for (size_t i = 0; i < sizeof(objects) / sizeof(objects[0]); ++i)
    {
        if (objects[i] != nullptr && !objects[i]->signalsBlocked())
        {
            m_objects[i] = objects[i];
            m_objects[i]->blockSignals(true);
        }
        else
        {
            m_objects[i] = nullptr;
        }
    }
}

ScopedBlockSignals::~ScopedBlockSignals()
{
    for (QObject* o : m_objects)
    {
        if (o != nullptr)
        {
            o->blockSignals(false);
        }
    }
}
