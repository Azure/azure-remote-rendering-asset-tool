#pragma once

#include <Model/IncludesAzureStorage.h>
#include <QAbstractItemModel>
#include <QPointer>
#include <Utils/ExtensionFilter.h>
#include <Utils/Value.h>

class AzureStorageManager;
class Cancellable;
class DirectoryProvider;


// Flat list model for all of the blobs in a container, under a directory and with certain extensions
class BlobsListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    BlobsListModel(DirectoryProvider* directoryProvider, AzureStorageManager* storageManager, const QString& allowedExtensions, QPointer<Value<QString>> containerName, QObject* parent = nullptr);
    ~BlobsListModel();

    enum class EntryType
    {
        Model,
        Directory,
        DirectoryUp,
        ConfigFile,
        Texture,
        Other,
        NoType
    };

    enum class FilterType
    {
        AllFilesAndDirectories, // show all of the files in the current directory, including directories and [..]
        OnlySubDirectories,     // show just the subdirectories from the current directory
        JustAllowedExtensions   // show all of the files that are allowed, in the subtree in the directory
    };
    Q_ENUM(FilterType);

    FilterType getFilterType() const;
    void setFilterType(FilterType type);

    // this action is re-routed from the view, when an item is double-clicked.
    // It would cause the item to be selected if it's a model, or the directory to be changed if it's a directory
    // When a model is double clicked, a "blobSelected" signal is emitted
    void doubleClickItem(const QModelIndex& index);

    // this action is re-routed from the view, when an item is selected.
    // It will change the current blob and trigger a notification
    void setCurrentItem(const QModelIndex& index);

    // delete the current blob (or directory)
    void deleteCurrentItem();

    // return the current item data
    void getCurrentItem(EntryType& type, QString& path, QString& url) const;

    void refresh();

    // QAbstractItemModel overrides
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex&) const override { return {}; }

    virtual int columnCount(const QModelIndex& /*parent = QModelIndex()*/) const override { return 1; }
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override { return !parent.isValid(); }


    enum Roles
    {
        URL_ROLE = Qt::UserRole + 1, // (azure::storage::storage_uri) data role for the blob url
        PATH_ROLE,                   // (QString) data role for the blob path (basically the full blob name)
        ENTRY_TYPE_ROLE,             // (EntryType) type of entry
        LOADING_STATUS_ROLE,         // (LoadingStatus) data role for the loading status
        LOADING_PROGRESS             // (float) data role for the loading progress (useful only when LoadingStatus==LOADING)
    };

    // set the index of the current model that can have a status != NOT_LOADED
    void setLoadingModelBlob(const QString& path);

    enum class LoadingStatus : char
    {
        NOT_LOADED,
        LOADING,
        LOADED,
        FAILED
    };

    static QString statusToString(LoadingStatus status);

    // set the status of the current model
    void setLoadingModelBlobStatus(LoadingStatus status);

    // return the status of the current model
    LoadingStatus getModelBlobStatus() const;

    // set the loading progress of the current model (when getModelBlobStatus() == LOADING)
    void setLoadingModelProgress(float loadingProgress);

Q_SIGNALS:
    void filterTypeChanged();
    void blobSelected(const QString& path, const QString& url);
    void currentBlobChanged();

private:
    // internal representation of a node in the blob hierarchy (dir/blob). Items are referenced by their index in m_allItems and
    struct Item
    {
        Item(std::wstring path, azure::storage::storage_uri uri);
        std::wstring m_path;
        azure::storage::storage_uri m_uri;
    };
    QVector<Item> m_allItems;

    AzureStorageManager* const m_storageManager;

    utility::string_t m_loadingModelBlobPath;
    mutable QPersistentModelIndex m_modelBlobIndex;
    mutable QPersistentModelIndex m_loadingBlobIndex;

    LoadingStatus m_modelBlobLoadingStatus = LoadingStatus::NOT_LOADED;
    float m_modelBlobLoadingProgress = -1.0f;

    ExtensionFilter m_fileFilter;

    QPointer<Value<QString>> m_containerName;

    DirectoryProvider* const m_directoryProvider;

    // index list of all of the visible items taken from m_allItems
    std::vector<int> m_filteredListIndices;

    std::shared_ptr<Cancellable> m_fetcher;

    FilterType m_filterType = FilterType::JustAllowedExtensions;

    EntryType m_currentBlobType = EntryType::NoType;
    QString m_currentBlobPath;
    QString m_currentBlobUrl;

    bool m_enabled = false;

    const Item* getCachedItem(const QModelIndex& modelIndex) const;
    void updateModelBlobIndex();
    void filter();

    bool filterBlobName(const utility::string_t& name) const;
};
