#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QObject>
#include <QSet>

// class holding a selection of remote rendering identities, identified with a RR::ObjectId
class EntitySelection : public QObject
{
    Q_OBJECT
public:
    EntitySelection(QObject* parent);

    enum class SelectionType
    {
        Toggle,           // select if not selected and deselect if selected
        Add,              // always select (do nothing if it's selected)
        SelectExclusively // change the selection to contain just the passed element
    };

    bool isSelected(const RR::ApiHandle<RR::Entity>& object) const;
    void select(const RR::ApiHandle<RR::Entity>& object, SelectionType type = SelectionType::SelectExclusively);
    void deselect(const RR::ApiHandle<RR::Entity>& object);
    void deselectAll();
    void setSelection(const QList<RR::ApiHandle<RR::Entity>>& newSelection);

    QSet<RR::ApiHandle<RR::Entity>>::const_iterator begin() const;
    QSet<RR::ApiHandle<RR::Entity>>::const_iterator end() const;

    // called when an element is double clicked
    void focusEntity(const RR::ApiHandle<RR::Entity>& entity);

Q_SIGNALS:
    // notification that the selection has changed
    void selectionChanged(QList<RR::ApiHandle<RR::Entity>> m_added, QList<RR::ApiHandle<RR::Entity>> m_removed);
    // when an entity get focused
    void entityFocused(RR::ApiHandle<RR::Entity> focused);

private:
    QSet<RR::ApiHandle<RR::Entity>> m_selected;
};
