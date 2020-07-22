#include <Model/ModelEditor/EntitySelection.h>

EntitySelection::EntitySelection(QObject* parent)
    : QObject(parent)
{
}

bool EntitySelection::isSelected(const RR::ApiHandle<RR::Entity>& object) const
{
    return m_selected.contains(object);
}


void EntitySelection::select(const RR::ApiHandle<RR::Entity>& object, SelectionType type)
{
    switch (type)
    {
        case SelectionType::Add:
        {
            m_selected.insert(object);
            selectionChanged({object}, {});
            break;
        }
        case SelectionType::Toggle:
        {
            if (isSelected(object))
            {
                deselect(object);
            }
            else
            {
                select(object, SelectionType::Add);
            }
            break;
        }
        case SelectionType::SelectExclusively:
        {
            QList<RR::ApiHandle<RR::Entity>> justDeselected;
            QList<RR::ApiHandle<RR::Entity>> justSelected;

            if (isSelected(object))
            {
                //it's selected already. Deselect all of the others objects
                m_selected.remove(object);
                justDeselected = m_selected.toList();
            }
            else
            {
                // if it was not selected it means we are just selecting it now
                // and deselecting everything
                justSelected.push_back(object);
                justDeselected = m_selected.toList();
            }

            // make the selected list only contain the object
            m_selected.clear();
            m_selected.insert(object);

            selectionChanged(justSelected, justDeselected);
            break;
        }
    }
}

void EntitySelection::deselect(const RR::ApiHandle<RR::Entity>& object)
{
    if (m_selected.remove(object))
    {
        selectionChanged({}, {object});
    }
}

void EntitySelection::deselectAll()
{
    QSet<RR::ApiHandle<RR::Entity>> deselected;
    m_selected.swap(deselected);
    selectionChanged({}, deselected.toList());
}

QSet<RR::ApiHandle<RR::Entity>>::const_iterator EntitySelection::begin() const
{
    return m_selected.begin();
}

QSet<RR::ApiHandle<RR::Entity>>::const_iterator EntitySelection::end() const
{
    return m_selected.end();
}


void EntitySelection::setSelection(const QList<RR::ApiHandle<RR::Entity>>& newSelection)
{
    QList<RR::ApiHandle<RR::Entity>> justDeselected;
    QList<RR::ApiHandle<RR::Entity>> justSelected;

    // not very efficient, we assume the selections won't be too big
    for (auto&& entity : newSelection)
    {
        if (!m_selected.remove(entity))
        {
            // the newSelection id it wasn't in m_selected: it is selecting it.
            justSelected.push_back(entity);
        }
        else
        {
            // the element was in the old selection and it's in the new selection too: do nothing,
            // so removing it from m_selected it means removing it from the justDeselected list
        }
    }
    justDeselected = m_selected.toList();
    m_selected.clear();
    for (auto&& entity : newSelection)
    {
        m_selected.insert(entity);
    }
    selectionChanged(justSelected, justDeselected);
}

void EntitySelection::focusEntity(const RR::ApiHandle<RR::Entity>& entity)
{
    Q_EMIT entityFocused(entity);
}
