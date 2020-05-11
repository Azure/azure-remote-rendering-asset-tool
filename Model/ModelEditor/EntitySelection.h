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

    bool isSelected(const std::shared_ptr<RR::Entity>& object) const;
    void select(const std::shared_ptr<RR::Entity>& object, SelectionType type = SelectionType::SelectExclusively);
    void deselect(const std::shared_ptr<RR::Entity>& object);
    void deselectAll();

    void setSelection(const QList<std::shared_ptr<RR::Entity>>& newSelection);

    QList<std::shared_ptr<RR::Entity>>::const_iterator begin() const;
    QList<std::shared_ptr<RR::Entity>>::const_iterator end() const;

Q_SIGNALS:
    // notification that the selection has changed
    void selectionChanged(QList<std::shared_ptr<RR::Entity>> m_added, QList<std::shared_ptr<RR::Entity>> m_removed);

private:
    QSet<unsigned long long> m_selected;
    // To enable RR::Entity in public methods
    QList<std::shared_ptr<RR::Entity>> m_selectedE;
};
