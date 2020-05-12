#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QAbstractItemModel>

class ArrSessionManager;

// QAbstractItemModel for the scene tree, showing all of the entities in the currently loaded model

class SceneTreeModel : public QAbstractItemModel
{
public:
    SceneTreeModel(ArrSessionManager* sessionManager, QObject* parent = nullptr);

    // QAbstractItemModel overrides
    //
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;

    virtual int columnCount(const QModelIndex& /*parent = {}*/) const override { return 1; }
    virtual int rowCount(const QModelIndex& /*parent = {}*/) const override;

    RR::ApiHandle<RR::Entity> getEntityFromIndex(const QModelIndex& index) const;
    QModelIndex getIndexFromEntity(const RR::ApiHandle<RR::Entity>& entity) const;

private:
    ArrSessionManager* const m_sessionManager;

    struct EntityCache
    {
        EntityCache(RR::ApiHandle<RR::Entity> entity, QModelIndex parent);
        ~EntityCache();
        QVariant data(ArrSessionManager* sessionManager, int role = Qt::DisplayRole) const;

        QVector<EntityCache*> m_children;
        QModelIndex m_parent;
        RR::ApiHandle<RR::Entity> m_entity;
        bool m_childrenComputed = false;

        void ensureChildrenComputed(const SceneTreeModel* model, QModelIndex thisModelIndex);
    };

    friend struct EntityCache;

    void reset(bool makeEmpty = false);
    EntityCache* toCachedItem(const QModelIndex& index) const;
    QScopedPointer<EntityCache> m_rootItem;

    mutable QMap<RR::ApiHandle<RR::Entity>, QModelIndex> m_indices;
};
