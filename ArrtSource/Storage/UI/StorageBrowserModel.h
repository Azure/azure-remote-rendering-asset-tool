#pragma once

#include <Storage/IncludeAzureStorage.h>
#include <QAbstractItemModel>
#include <vector>

class StorageAccount;

struct StorageEntry
{
    enum class Type
    {
        Other,
        Folder,
        ArrAsset,
        SrcAsset,
    };

    QString m_fullPath;
    QString m_name;
    azure::storage::storage_uri m_uri;
    bool m_retrievedChildren = false;
    bool m_hasChanged = false;
    int m_rowIndex = -1;
    StorageEntry* m_parent = nullptr;
    std::vector<StorageEntry> m_children;
    Type m_Type = Type::Other;

    bool IsDifferent(const StorageEntry& rhs) const;
};

class StorageBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    void SetShowTypes(StorageEntry::Type types);
    bool SetAccountAndContainer(StorageAccount* account, const QString& containerName);

    void RefreshModel(bool fullReset);

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    virtual QModelIndex parent(const QModelIndex& child) const override;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    void FillChildEntries(StorageEntry* entry, const QString& entryPath, std::vector<StorageEntry>& output) const;
    void RefreshEntry(StorageEntry* entry);

    StorageAccount* m_storageAccount = nullptr;
    QString m_containerName;
    StorageEntry::Type m_showTypes = StorageEntry::Type::Other;

    mutable StorageEntry m_rootEntry;
};