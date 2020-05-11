#include <Model/ArrSessionManager.h>
#include <QDebug>
#include <ViewModel/ModelEditor/SceneTreeModel.h>

SceneTreeModel::SceneTreeModel(ArrSessionManager* sessionManager, QObject* parent)
    : QAbstractItemModel(parent)
    , m_sessionManager(sessionManager)
{
    QObject::connect(sessionManager, &ArrSessionManager::sessionAboutToChange, this, [this]() {
        reset(true);
    });
    QObject::connect(sessionManager, &ArrSessionManager::sessionChanged, this, [this]() {
        reset();
    });
    QObject::connect(sessionManager, &ArrSessionManager::rootIdChanged, this, [this]() {
        reset();
    });
    reset();
}



void SceneTreeModel::reset(bool makeEmpty)
{
    QAbstractItemModel::beginResetModel();
    m_indices.clear();
    std::shared_ptr<RR::Entity> root = nullptr;
    if (m_sessionManager->loadedModel())
    {
        root = m_sessionManager->loadedModel()->Root();
    }
    m_rootItem.reset(new EntityCache(root, QModelIndex()));
    if (makeEmpty)
    {
        m_rootItem->m_childrenComputed = true;
    }
    m_indices.insert(root ? root->Handle() : 0, QModelIndex());
    QAbstractItemModel::endResetModel();
}

QModelIndex SceneTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    auto* cachedItem = toCachedItem(parent);
    cachedItem->ensureChildrenComputed(this, parent);
    const auto& children = cachedItem->m_children;
#ifdef QT_DEBUG
    // this check is only to avoid a crash in an assert in QAbstractItemView::setModel
    // which has the assumption that the model has at least one item under the root item
    if (row >= children.size())
    {
        return {};
    }
#endif
    return createIndex(row, column, children[row]);
}

SceneTreeModel::EntityCache* SceneTreeModel::toCachedItem(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return m_rootItem.get();
    }
    else
    {
        return reinterpret_cast<SceneTreeModel::EntityCache*>(index.internalPointer());
    }
}

std::shared_ptr<RR::Entity> SceneTreeModel::getEntityFromIndex(const QModelIndex& index) const
{
    if (auto* item = toCachedItem(index))
    {
        return item->m_entity;
    }
    else
    {
        return {};
    }
}

QModelIndex SceneTreeModel::getIndexFromEntity(const std::shared_ptr<RR::Entity>& entity) const
{
    auto it_first = m_indices.find(entity->Handle());
    if (it_first != m_indices.end())
    {
        return it_first.value();
    }
    else
    {
        // couldn't find the index. It could mean it's not expanded yet.
        // expand based on the parent chain
        QList<std::shared_ptr<RR::Entity>> parents;

        std::shared_ptr<RR::Entity> parent = nullptr;
        if (entity && entity->Valid())
        {
            parent = entity->Parent();
        }

        while (parent)
        {
            parents.push_front(parent);
            if (m_indices.contains(parent->Handle()))
            {
                break;
            }
            parent = parent->Parent();
        }
        if (!parent)
        {
            // couldn't find any matching index in any of the parent chain
            return {};
        }

        // traverse in reverse order and expand
        for (const std::shared_ptr<RR::Entity>& p : parents)
        {
            auto it = m_indices.find(p->Handle());
            if (it == m_indices.end())
            {
                qCritical() << tr("Error: can't find index in scene tree model");
                return {};
            }
            QModelIndex index = it.value();
            // trigger the child computation
            this->index(index.row(), index.column(), index);
        }

        // try again. It should return
        auto it = m_indices.find(entity->Handle());
        if (it != m_indices.end())
        {
            return it.value();
        }
        else
        {
            qCritical() << tr("Error: can't find index in scene tree model");
        }
    }
    return {};
}

QVariant SceneTreeModel::data(const QModelIndex& index, int role) const
{
    return toCachedItem(index)->data(m_sessionManager, role);
}

QModelIndex SceneTreeModel::parent(const QModelIndex& index) const
{
    return toCachedItem(index)->m_parent;
}

int SceneTreeModel::rowCount(const QModelIndex& parent) const
{
    auto* cachedItem = toCachedItem(parent);
    cachedItem->ensureChildrenComputed(this, parent);
    return cachedItem->m_children.size();
}

SceneTreeModel::EntityCache::EntityCache(std::shared_ptr<RR::Entity> entity, QModelIndex parent)
    : m_parent(parent)
    , m_entity(std::move(entity))
{
}

SceneTreeModel::EntityCache::~EntityCache()
{
    for (auto* child : m_children)
    {
        delete child;
    }
}


QVariant SceneTreeModel::EntityCache::data(ArrSessionManager* sessionManager, int role) const
{
    if (auto* clientApi = sessionManager->getClientApi())
    {
        switch (role)
        {
            case Qt::DisplayRole:
            {
                if (m_entity && m_entity->Valid())
                {
                    return QString::fromUtf8(m_entity->Name().c_str());
                }
                return QString();
            }
        }
    }
    return {};
}

void SceneTreeModel::EntityCache::ensureChildrenComputed(const SceneTreeModel* model, QModelIndex thisModelIndex)
{
    if (!m_childrenComputed)
    {
        m_childrenComputed = true;
        if (auto* clientApi = model->m_sessionManager->getClientApi())
        {
            if (m_entity && m_entity->Valid())
            {
                int childNumber = 0;
                for (const std::shared_ptr<RR::Entity>& child : m_entity->Children())
                {
                    m_children.push_back(new EntityCache(child, thisModelIndex));
                    model->m_indices.insert(child->Handle(), model->index(childNumber++, 0, thisModelIndex));
                }
            }
        }
    }
}
