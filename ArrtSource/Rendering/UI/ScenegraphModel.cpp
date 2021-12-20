#include <Rendering/ArrSession.h>
#include <Rendering/UI/ScenegraphModel.h>

ScenegraphModel::ScenegraphModel(ArrSession* session)
{
    m_arrSession = session;
}

ScenegraphModel::~ScenegraphModel() = default;

void ScenegraphModel::RefreshModel()
{
    beginResetModel();

    m_ArrHandleToQt.clear();
    m_modelRoots.clear();

    if (m_arrSession->GetSessionStatus().IsRunning())
    {
        auto& models = m_arrSession->GetLoadedModels();

        m_modelRoots.resize(models.size());

        // get all top-level entities of each model that was loaded
        // then traverse its children recursively
        for (int i = 0; i < (int)models.size(); ++i)
        {
            m_modelRoots[i].m_name = models[i].m_ModelName;
            m_modelRoots[i].m_rowIndex = i;
            m_modelRoots[i].m_arrEntity = models[i].m_LoadResult->GetRoot();

            m_ArrHandleToQt[m_modelRoots[i].m_arrEntity->GetHandle()] = createIndex(i, 0, &m_modelRoots[i]);

            FillChildEntries(&m_modelRoots[i]);
        }
    }

    endResetModel();
}

QModelIndex ScenegraphModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    SceneEntry* entryPtr = nullptr;

    if (!parent.isValid())
    {
        if (row >= m_modelRoots.size())
            return {};

        entryPtr = &m_modelRoots[row];
    }
    else
    {
        SceneEntry* parentPtr = (SceneEntry*)parent.internalPointer();

        if (row >= parentPtr->m_children.size())
            return {};

        entryPtr = &parentPtr->m_children[row];
    }

    return createIndex(row, column, entryPtr);
}

QModelIndex ScenegraphModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return {};

    SceneEntry* entry = (SceneEntry*)child.internalPointer();

    if (entry->m_parent == nullptr)
        return {};

    return createIndex(entry->m_parent->m_rowIndex, child.column(), entry->m_parent);
}

int ScenegraphModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return (int)m_modelRoots.size();
    }
    else
    {
        SceneEntry* entry = (SceneEntry*)parent.internalPointer();
        return (int)entry->m_children.size();
    }
}

int ScenegraphModel::columnCount(const QModelIndex& /*parent*/ /*= QModelIndex()*/) const
{
    return 1;
}

QVariant ScenegraphModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return {};

    const SceneEntry* entry = (const SceneEntry*)(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        return entry->m_name;
    }

    return {};
}

RR::ApiHandle<RR::Entity> ScenegraphModel::GetEntityHandle(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    const SceneEntry* entry = (const SceneEntry*)(index.internalPointer());
    return entry->m_arrEntity;
}

QModelIndex ScenegraphModel::MapArrEntityHandleToModelIndex(const RR::ApiHandle<RR::Entity>& handle) const
{
    auto it = m_ArrHandleToQt.find(handle->GetHandle());

    if (it != m_ArrHandleToQt.end())
    {
        return it->second;
    }

    return {};
}

void ScenegraphModel::FillChildEntries(SceneEntry* entry)
{
    std::vector<RR::ApiHandle<RR::Entity>> children;
    entry->m_arrEntity->GetChildren(children);

    entry->m_children.resize(children.size());

    for (int c = 0; c < (int)children.size(); ++c)
    {
        std::string name;
        children[c]->GetName(name);

        entry->m_children[c].m_arrEntity = children[c];
        entry->m_children[c].m_name = name.c_str();
        entry->m_children[c].m_parent = entry;
        entry->m_children[c].m_rowIndex = c;

        m_ArrHandleToQt[entry->m_children[c].m_arrEntity->GetHandle()] = createIndex(c, 0, &entry->m_children[c]);

        FillChildEntries(&entry->m_children[c]);
    }
}
