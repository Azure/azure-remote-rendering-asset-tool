#pragma once

#include <QAbstractItemModel>
#include <Storage/IncludeAzureStorage.h>
#include <vector>
#include <Conversion/Conversion.h>

class StorageAccount;

struct StorageEntry
{
    enum class Type
    {
        Other,    ///< All other file types
        Folder,   ///< A folder
        ArrAsset, ///< A file with '.arrAsset' file extension
        SrcAsset, ///< A file with '.fbx', '.gltf', '.glb', '.e57', '.ply', '.xyz', '.las', or '.laz' file extension
    };

    QString m_fullPath;
    QString m_name;
    bool m_retrievedChildren = false;
    bool m_hasChanged = false;
    int m_rowIndex = -1;
    StorageEntry* m_parent = nullptr;
    std::vector<StorageEntry> m_children;
    Type m_Type = Type::Other;

    bool IsDifferent(const StorageEntry& rhs) const;
};

/// The QAbstractItemModel representing the file structure in the storage account
///
/// The model queries the Azure Storage only as much as needed to show the file structure.
/// Collapsed folders are not queried, only when the user expands them.
class StorageBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    void SetFilter(StorageEntry::Type showTypes, const QString& parentPathFilter);
    bool SetAccountAndContainer(StorageAccount* account, const QString& containerName);

    /// Checks whether there are any changes in the storage account (files added or removed) and updates the respective local data.
    ///
    /// If fullReset is true, all data is discarded and built new.
    void RefreshModel(bool fullReset);

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    static bool IsArrAsset(const QString& file);
    static bool IsSrcAsset(const QString& file);

private:
    void FillChildEntries(StorageEntry* entry, const QString& entryPath, std::vector<StorageEntry>& output) const;
    void RefreshEntry(StorageEntry* entry);

    StorageAccount* m_storageAccount = nullptr;
    QString m_containerName;
    StorageEntry::Type m_showTypes = StorageEntry::Type::Other;
    QString m_parentPathFilter;

    mutable StorageEntry m_rootEntry;
};