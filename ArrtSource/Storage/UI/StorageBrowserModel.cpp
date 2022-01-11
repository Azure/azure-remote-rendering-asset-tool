#include <QIcon>
#include <Storage/StorageAccount.h>
#include <Storage/UI/StorageBrowserModel.h>

void StorageBrowserModel::SetFilter(StorageEntry::Type showTypes, const QString& parentPathFilter)
{
    m_showTypes = showTypes;
    m_parentPathFilter = parentPathFilter;
}

bool StorageBrowserModel::SetAccountAndContainer(StorageAccount* account, const QString& containerName)
{
    if (m_storageAccount == account && m_containerName == containerName)
        return false;

    m_storageAccount = account;
    m_containerName = containerName;

    RefreshModel(true);

    return true;
}

void StorageBrowserModel::RefreshModel(bool fullReset)
{
    if (fullReset)
    {
        beginResetModel();

        m_rootEntry.m_parent = nullptr;
        m_rootEntry.m_children.clear();
        m_rootEntry.m_rowIndex = 0;
        m_rootEntry.m_fullPath = "";
        m_rootEntry.m_name = m_containerName;
        m_rootEntry.m_retrievedChildren = true;
        m_rootEntry.m_Type = StorageEntry::Type::Folder;

        FillChildEntries(&m_rootEntry, "", m_rootEntry.m_children);

        endResetModel();
    }
    else
    {
        RefreshEntry(&m_rootEntry);
    }
}

QModelIndex StorageBrowserModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    StorageEntry* entryPtr = nullptr;

    if (!parent.isValid())
    {
        if (row > 0)
            return {};

        entryPtr = &m_rootEntry;
    }
    else
    {
        StorageEntry* parentPtr = (StorageEntry*)parent.internalPointer();

        if (row >= parentPtr->m_children.size())
            return {};

        entryPtr = &parentPtr->m_children[row];
    }

    if (!entryPtr->m_retrievedChildren)
    {
        entryPtr->m_retrievedChildren = true;

        if (entryPtr->m_fullPath.endsWith("/"))
        {
            FillChildEntries(entryPtr, entryPtr->m_fullPath, entryPtr->m_children);
        }
    }

    return createIndex(row, column, entryPtr);
}

QModelIndex StorageBrowserModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return {};

    StorageEntry* entry = (StorageEntry*)child.internalPointer();

    if (entry->m_parent == nullptr)
        return {};

    return createIndex(entry->m_parent->m_rowIndex, child.column(), entry->m_parent);
}

int StorageBrowserModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return 1;
    }
    else
    {
        StorageEntry* entry = (StorageEntry*)parent.internalPointer();
        return (int)entry->m_children.size();
    }
}

int StorageBrowserModel::columnCount(const QModelIndex& /*= QModelIndex()*/) const
{
    return 1;
}

QVariant StorageBrowserModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return {};

    const StorageEntry* entry = (const StorageEntry*)(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        return entry->m_name;
    }

    if (role == Qt::UserRole)
    {
        return entry->m_fullPath;
    }

    if (role == Qt::DecorationRole)
    {
        switch (entry->m_Type)
        {
            case StorageEntry::Type::Folder:
                return QIcon(":/ArrtApplication/Icons/directory.svg");

            case StorageEntry::Type::ArrAsset:
                return QIcon(":/ArrtApplication/Icons/remoterendering.png");

            case StorageEntry::Type::SrcAsset:
                return QIcon(":/ArrtApplication/Icons/model.svg");

            default:
                break;
        }
    }

    return {};
}

void StorageBrowserModel::FillChildEntries(StorageEntry* entry, const QString& entryPath, std::vector<StorageEntry>& output) const
{
    if (m_storageAccount == nullptr)
        return;

    std::vector<StorageBlobInfo> directories, files;
    m_storageAccount->ListBlobDirectory(m_containerName, entryPath, directories, files);

    output.reserve(directories.size() + files.size());
    int rowIdx = 0;

    for (const StorageBlobInfo& dir : directories)
    {
        StorageEntry e;
        e.m_parent = entry;
        e.m_rowIndex = rowIdx++;
        e.m_name = dir.m_path.mid(entryPath.length()).chopped(1); // remove the prefix path and the trailing slash
        e.m_fullPath = dir.m_path;
        e.m_Type = StorageEntry::Type::Folder;

        if (!m_parentPathFilter.isEmpty() && (!m_parentPathFilter.startsWith(e.m_fullPath)))
        {
            continue;
        }

        output.push_back(e);
    }

    for (const StorageBlobInfo& file : files)
    {
        StorageEntry e;
        e.m_parent = entry;
        e.m_rowIndex = rowIdx++;
        e.m_name = file.m_path.mid(entryPath.length()); // remove the prefix path
        e.m_fullPath = file.m_path;

        if (IsSrcAsset(e.m_name))
        {
            e.m_Type = StorageEntry::Type::SrcAsset;
        }
        else if (IsArrAsset(e.m_name))
        {
            e.m_Type = StorageEntry::Type::ArrAsset;
        }

        if (m_showTypes != e.m_Type && m_showTypes != StorageEntry::Type::Other)
        {
            continue;
        }

        if (!m_parentPathFilter.isEmpty() && (!m_parentPathFilter.startsWith(e.m_fullPath)))
        {
            continue;
        }

        output.push_back(e);
    }
}

void StorageBrowserModel::RefreshEntry(StorageEntry* entry)
{
    if (!entry->m_retrievedChildren || (!entry->m_fullPath.isEmpty() && !entry->m_fullPath.endsWith("/")))
        return;

    std::vector<StorageEntry> children;
    FillChildEntries(entry, entry->m_fullPath, children);

    entry->m_hasChanged = entry->m_children.size() != children.size();

    if (!entry->m_hasChanged)
    {
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (entry->m_children[i].IsDifferent(children[i]))
            {
                entry->m_hasChanged = true;
                break;
            }
        }
    }

    if (entry->m_hasChanged)
    {
        const QModelIndex idx = createIndex(entry->m_rowIndex, 0, entry);

        if (!entry->m_children.empty())
        {
            beginRemoveRows(idx, 0, (int)entry->m_children.size() - 1);
            entry->m_children.clear();
            endRemoveRows();
        }

        if (!children.empty())
        {
            beginInsertRows(idx, 0, (int)children.size() - 1);
            entry->m_children = children;
            endInsertRows();
        }
    }
    else
    {
        for (size_t c = 0; c < entry->m_children.size(); ++c)
        {
            RefreshEntry(&entry->m_children[c]);
        }
    }
}

bool StorageBrowserModel::IsArrAsset(const QString& file)
{
    return file.endsWith(".arrAsset", Qt::CaseInsensitive);
}

bool StorageBrowserModel::IsSrcAsset(const QString& file)
{
    return file.endsWith(".gltf", Qt::CaseInsensitive) || file.endsWith(".glb", Qt::CaseInsensitive) || file.endsWith(".fbx", Qt::CaseInsensitive);
}

bool StorageEntry::IsDifferent(const StorageEntry& rhs) const
{
    if (m_name != rhs.m_name)
        return true;

    // more criteria?

    return false;
}
