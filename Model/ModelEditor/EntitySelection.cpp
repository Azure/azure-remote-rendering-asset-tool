#include <Model/ModelEditor/EntitySelection.h>

namespace
{
    uint qHash(const std::shared_ptr<RR::Entity>& e)
    {
        return static_cast<uint>(std::hash<unsigned long long>()(e.get() && e->Valid() ? e->Handle() : 0));
    }
} // namespace
EntitySelection::EntitySelection(QObject* parent)
    : QObject(parent)
{
}

bool EntitySelection::isSelected(const std::shared_ptr<RR::Entity>& object) const
{
    return m_selected.contains(object->Handle());
}


void EntitySelection::select(const std::shared_ptr<RR::Entity>& object, SelectionType type)
{
    switch (type)
    {
        case SelectionType::Add:
        {
            m_selected.insert(object->Handle());
            m_selectedE.push_back(std::make_shared<RR::Entity>(object->Handle()));
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
            QList<unsigned long long> justDeselected;
            QList<std::shared_ptr<RR::Entity>> justSelectedE;

            if (isSelected(object))
            {
                //it's selected already. Deselect all of the others objects
                m_selected.remove(object->Handle());
                justDeselected = m_selected.toList();
            }
            else
            {
                // if it was not selected it means we are just selecting it now
                // and deselecting everything
                justSelectedE.push_back(object);
                justDeselected = m_selected.toList();
            }

            // make the selected list only contain the object
            m_selected.clear();
            m_selected.insert(object->Handle());
            m_selectedE.clear();
            m_selectedE.push_back(object);
            QList<std::shared_ptr<RR::Entity>> justDeselectedE;
            for (auto&& id : justDeselected)
            {
                justDeselectedE.push_back(std::make_shared<RR::Entity>(id));
            }

            selectionChanged(justSelectedE, justDeselectedE);
            break;
        }
    }
}

void EntitySelection::deselect(const std::shared_ptr<RR::Entity>& object)
{
    if (m_selected.remove(object->Handle()))
    {
        m_selectedE.clear();
        for (auto&& id : m_selected)
        {
            m_selectedE.push_back(std::make_shared<RR::Entity>(id));
        }
        selectionChanged({}, {object});
    }
}

void EntitySelection::deselectAll()
{
    QSet<unsigned long long> deselected;
    m_selected.swap(deselected);
    m_selectedE.clear();
    QList<std::shared_ptr<RR::Entity>> deselectedE;
    for (auto&& id : deselected)
    {
        deselectedE.push_back(std::make_shared<RR::Entity>(id));
    }
    selectionChanged({}, deselectedE);
}

QList<std::shared_ptr<RR::Entity>>::const_iterator EntitySelection::begin() const
{
    return m_selectedE.begin();
}

QList<std::shared_ptr<RR::Entity>>::const_iterator EntitySelection::end() const
{
    return m_selectedE.end();
}


void EntitySelection::setSelection(const QList<std::shared_ptr<RR::Entity>>& newSelection)
{
    QList<std::shared_ptr<RR::Entity>> justDeselected;
    QList<std::shared_ptr<RR::Entity>> justSelected;

    // not very efficient, we assume the selections won't be too big
    for (auto&& entity : newSelection)
    {
        if (!m_selected.remove(entity->Handle()))
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
    for (auto&& id : m_selected)
    {
        justDeselected.push_back(std::make_shared<RR::Entity>(id));
    }
    m_selected.clear();
    for (auto&& entity : newSelection)
    {
        m_selected.insert(entity->Handle());
    }
    m_selectedE = newSelection;
    selectionChanged(justSelected, justDeselected);
}
