#pragma once

#include <Model/IncludesAzureRemoteRendering.h>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class ArrSessionManager;
namespace
{
    uint qHash(const RR::ApiHandle<RR::Material>& m)
    {
        return static_cast<uint>(std::hash<unsigned long long>()(m && *m->Valid() ? m->Handle() : 0));
    }
} // namespace
// List model for the material list. It contains all of the materials in the scene

class MaterialListModel : public QStandardItemModel
{
public:
    MaterialListModel(ArrSessionManager* sessionManager, QObject* parent = nullptr);

    enum Roles
    {
        OBJECT_MATERIAL_ROLE = Qt::UserRole + 1 //RR::ObjectId for the material
    };

private:
    ArrSessionManager* const m_sessionManager;

    void reset();
};

// Filtered list model for the material list used in View/MaterialsList. It will contain only the materials
// which are in a selection of entities.

class MaterialFilteredListModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void filterBasedOnEntities(const QList<RR::ApiHandle<RR::Entity>>& entityIds);
    // true is the list is filtered
    bool isFiltered() const;
    // return the list of materials belonging to the entities in the filter. Empty set if the list is not filtered
    const QSet<RR::ApiHandle<RR::Material>>& getFilteredMaterialSet() const;

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

Q_SIGNALS:
    void filterChanged();

private:
    QList<RR::ApiHandle<RR::Entity>> m_entityIds;
    QSet<RR::ApiHandle<RR::Material>> m_materials;
};
