#include <Model/AzureStorageManager.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/BlobExplorer/DirectoryProvider.h>
#include <string>
#include <utility>


Q_DECLARE_METATYPE(azure::storage::storage_uri);
Q_DECLARE_METATYPE(utility::string_t);
Q_DECLARE_METATYPE(BlobsListModel::LoadingStatus);
Q_DECLARE_METATYPE(BlobsListModel::EntryType);

namespace
{
    const std::wstring s_directoryUpPath = L"../";
}

BlobsListModel::BlobsListModel(DirectoryProvider* directoryProvider, AzureStorageManager* storageManager, const QString& allowedExtensions, QPointer<Value<QString>> containerName, QObject* parent)
    : QAbstractItemModel(parent)
    , m_storageManager(storageManager)
    , m_containerName(std::move(containerName))
    , m_directoryProvider(directoryProvider)
{
    m_fileFilter.setAllowedExtensions(allowedExtensions);

    auto onStatusChanged = [this]() {
        const bool enabled = m_storageManager->getStatus() == AccountConnectionStatus::Connected;
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            refresh();
        }
    };
    onStatusChanged();
    connect(m_storageManager, &AzureStorageManager::onStatusChanged, this, onStatusChanged);

    connect(m_directoryProvider, &DirectoryProvider::directoryChanged, this,
            [this]() {
                if (m_filterType == FilterType::JustAllowedExtensions)
                {
                    filter();
                }
                else
                {
                    refresh();
                }
            });

    connect(m_containerName, &Value<QString>::valueChanged, this, [this] { refresh(); });
}

BlobsListModel::~BlobsListModel()
{
    if (m_fetcher)
    {
        // if the fetching is still in progress, cancel it, so that the assert won't be triggered
        m_fetcher->cancel();
    }
}


void BlobsListModel::refresh()
{
    if (m_fetcher)
    {
        m_fetcher->cancel();
        m_fetcher = nullptr;
    }
    beginResetModel();
    m_allItems.clear();
    m_filteredListIndices.clear();
    endResetModel();
    setCurrentItem({});

    if (!m_enabled)
    {
        return;
    }

    QString containerName;
    if (m_containerName)
    {
        containerName = m_containerName->get();
    }
    if (containerName.isEmpty())
    {
        return;
    }

    if ((m_filterType == FilterType::AllFilesAndDirectories || m_filterType == FilterType::OnlySubDirectories) && m_directoryProvider->canGoToParentDirectory())
    {
        m_allItems.push_back({s_directoryUpPath, {}});
        m_filteredListIndices.push_back(m_allItems.size() - 1);
    }

    auto processRows = [this](const azure::storage::list_blob_item_segment& segment) {
        const int oldRowCounts = rowCount();

        for (auto& bi : segment.results())
        {
            if (bi.is_blob())
            {
                if (m_filterType != FilterType::OnlySubDirectories)
                {
                    auto blob = bi.as_blob();
                    const utility::string_t& name = blob.name();
                    if (filterBlobName(name))
                    {
                        m_allItems.push_back({name, blob.uri()});
                        if (m_directoryProvider->isBlobInDirectory(name))
                        {
                            m_filteredListIndices.push_back(m_allItems.size() - 1);
                        }
                    }
                }
            }
            else
            {
                auto dir = bi.as_directory();
                const auto& name = dir.prefix();
                m_allItems.push_back({name, dir.uri()});
                m_filteredListIndices.push_back(m_allItems.size() - 1);
            }
        }
        if (rowCount() > oldRowCounts)
        {
            beginInsertRows({}, oldRowCounts, rowCount() - 1);
            endInsertRows();
        }

        if (segment.continuation_token().empty())
        {
            updateModelBlobIndex();
        }
    };

    switch (m_filterType)
    {
        case FilterType::AllFilesAndDirectories:
        case FilterType::OnlySubDirectories:
            m_fetcher = m_storageManager->getBlobsInDirectoryAsyncSegmented(this, containerName, m_directoryProvider->getPrefix(), processRows);
            break;
        case FilterType::JustAllowedExtensions:
            m_fetcher = m_storageManager->getAllBlobsAsyncSegmented(this, containerName, processRows);
            break;
    }
}

namespace
{
    // get a view of the file name or directory (without directory and extension)
    std::wstring_view getFilenameOrDirFromFullPath(const std::wstring_view& path)
    {
        std::wstring_view fileName;
        fileName = path;
        //remove the last character from the search, so that if it's a "/"
        //it will find the parent separator
        std::wstring_view fileNameWithNoLastCharacter = fileName.substr(0, fileName.length() - 1);
        auto sep = fileNameWithNoLastCharacter.find_last_of(L'/');
        if (sep != std::string::npos)
        {
            fileName = path.substr(sep + 1);
        }
        else
        {
            fileName = path;
        }
        return fileName;
    }

    QString stringViewToQString(const std::wstring_view& v)
    {
        return QString::fromWCharArray(v.data(), static_cast<int>(v.size()));
    }
} // namespace

BlobsListModel::Item::Item(std::wstring path, azure::storage::storage_uri uri)
    : m_path(std::move(path))
    , m_uri(std::move(uri))
{
}

BlobsListModel::FilterType BlobsListModel::getFilterType() const
{
    return m_filterType;
}

void BlobsListModel::setFilterType(FilterType type)
{
    if (m_filterType != type)
    {
        m_filterType = type;
        refresh();
        Q_EMIT filterTypeChanged();
    }
}

void BlobsListModel::doubleClickItem(const QModelIndex& index)
{
    switch (index.data(BlobsListModel::ENTRY_TYPE_ROLE).value<BlobsListModel::EntryType>())
    {
        case BlobsListModel::EntryType::Directory:
        {
            m_directoryProvider->setDirectory(index.data(BlobsListModel::PATH_ROLE).toString());
            break;
        }
        case BlobsListModel::EntryType::DirectoryUp:
        {
            m_directoryProvider->goToParentDirectory();
            break;
        }
        case BlobsListModel::EntryType::Model:
        {
            Q_EMIT blobSelected(
                index.data(BlobsListModel::PATH_ROLE).toString(),
                QString::fromStdWString(index.data(BlobsListModel::URL_ROLE).value<azure::storage::storage_uri>().primary_uri().to_string()));
            break;
        }
        case BlobsListModel::EntryType::ConfigFile:
        case BlobsListModel::EntryType::Texture:
        case BlobsListModel::EntryType::Other:
        case BlobsListModel::EntryType::NoType:
            // nothing
            break;
    }
}

void BlobsListModel::setCurrentItem(const QModelIndex& index)
{
    auto entryType = EntryType::NoType;
    QString path;
    QString url;
    if (index.isValid())
    {
        entryType = index.data(BlobsListModel::ENTRY_TYPE_ROLE).value<BlobsListModel::EntryType>();
        path = index.data(BlobsListModel::PATH_ROLE).toString();
        url = QString::fromStdWString(index.data(BlobsListModel::URL_ROLE).value<azure::storage::storage_uri>().primary_uri().to_string());
    }

    m_modelBlobIndex = index;

    if (entryType != m_currentBlobType || m_currentBlobPath != path || m_currentBlobUrl != url)
    {
        m_currentBlobType = entryType;
        m_currentBlobPath = path;
        m_currentBlobUrl = url;
        Q_EMIT currentBlobChanged();
    }
}


void BlobsListModel::deleteCurrentItem()
{
    QString containerName;
    if (m_containerName)
    {
        containerName = m_containerName->get();
    }
    if (!containerName.isEmpty() && !m_currentBlobPath.isEmpty() && m_currentBlobType != EntryType::DirectoryUp && m_currentBlobType != EntryType::NoType)
    {
        m_storageManager->deleteBlobOrDirectory(containerName, m_currentBlobPath.toStdWString());
        refresh();
    }
}

void BlobsListModel::getCurrentItem(EntryType& type, QString& path, QString& url) const
{
    type = m_currentBlobType;
    path = m_currentBlobPath;
    url = m_currentBlobUrl;
}


void BlobsListModel::filter()
{
    beginResetModel();
    std::vector<int> map;
    map.reserve(m_allItems.count());
    for (int i = 0; i < m_allItems.count(); ++i)
    {
        if (m_directoryProvider->isBlobInDirectory(m_allItems[i].m_path))
        {
            map.push_back(i);
        }
    }
    m_filteredListIndices.swap(map);
    endResetModel();
}

QModelIndex BlobsListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        //only the root has children
        return {};
    }
    if (row < 0 || row >= static_cast<int>(m_filteredListIndices.size()))
    {
        return {};
    }
    return createIndex(row, column, reinterpret_cast<void*>(static_cast<long long>(row)));
}

QVariant BlobsListModel::data(const QModelIndex& index, int role) const
{
    if (const auto* item = getCachedItem(index))
    {
        switch (role)
        {
            case Qt::AccessibleTextRole:
            case Qt::DisplayRole:
            {
                auto name = getFilenameOrDirFromFullPath(item->m_path);

                if (m_filterType == FilterType::JustAllowedExtensions && m_fileFilter.match(name))
                {
                    name = name.substr(0, name.find_last_of(L'.'));
                }

                return stringViewToQString(name);
            }
            case URL_ROLE:
            {
                return QVariant::fromValue(item->m_uri);
            }
            case PATH_ROLE:
            {
                return QVariant::fromValue(QString::fromStdWString(item->m_path));
            }
            case LOADING_STATUS_ROLE:
            {
                if (m_loadingModelBlobPath._Starts_with(item->m_path))
                {
                    return QVariant::fromValue(m_modelBlobLoadingStatus);
                }
                else
                {
                    return QVariant::fromValue(LoadingStatus::NOT_LOADED);
                }
            }
            case LOADING_PROGRESS:
            {
                if (index == m_loadingBlobIndex)
                {
                    return QVariant::fromValue(m_modelBlobLoadingProgress);
                }
                else
                {
                    return QVariant::fromValue(-1.0f);
                }
            }
            case ENTRY_TYPE_ROLE:
            {
                if (m_filterType == FilterType::JustAllowedExtensions)
                {
                    return QVariant::fromValue(EntryType::Model);
                }
                else
                {
                    if (item->m_path == s_directoryUpPath)
                    {
                        return QVariant::fromValue(EntryType::DirectoryUp);
                    }
                    else if (!item->m_path.empty() && item->m_path[item->m_path.length() - 1] == '/')
                    {
                        return QVariant::fromValue(EntryType::Directory);
                    }
                    else if (m_fileFilter.match(item->m_path))
                    {
                        return QVariant::fromValue(EntryType::Model);
                    }
                    else
                    {
                        return QVariant::fromValue(EntryType::Other);
                    }
                    //<TODO> other file types, if needed? We might want to extent m_fileFilter to return the type instead.
                }
            }
        }
    }
    return QVariant();
}

int BlobsListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(m_filteredListIndices.size());
}

const BlobsListModel::Item* BlobsListModel::getCachedItem(const QModelIndex& modelIndex) const
{
    if (!modelIndex.isValid())
    {
        return nullptr;
    }
    if (modelIndex.row() < 0 || modelIndex.row() >= static_cast<int>(m_filteredListIndices.size()))
    {
        return nullptr;
    }
    return &m_allItems[m_filteredListIndices[modelIndex.row()]];
}

void BlobsListModel::setLoadingModelBlob(const QString& path)
{
    m_loadingModelBlobPath = path.toStdWString();
    // the blob status, in the current implementation, is only relative to a single item (m_modelBlobIndex).
    // if we change that item, this implies the data getter for the old item will return NOT_LOADED.
    // This means we need to call dataChanged on the old item to notify the view that it needs to refresh it.
    // this is done here with setModelBlobStatus
    setLoadingModelBlobStatus(LoadingStatus::NOT_LOADED);
    m_loadingBlobIndex = QPersistentModelIndex{};

    //search for the item
    for (int i = 0; i < rowCount(); ++i)
    {
        auto item = index(i, 0);
        if (item.data(PATH_ROLE).toString() == path)
        {
            m_loadingBlobIndex = item;
            break;
        }
    }
}

QString BlobsListModel::statusToString(LoadingStatus status)
{
    switch (status)
    {
        case LoadingStatus::FAILED:
            return tr("Failed");
        case LoadingStatus::LOADED:
            return tr("Loaded");
        case LoadingStatus::LOADING:
            return tr("Loading");
        case LoadingStatus::NOT_LOADED:
            return tr("Not Loaded");
        default:
            return {};
    }
}


void BlobsListModel::setLoadingModelBlobStatus(LoadingStatus status)
{
    m_modelBlobLoadingStatus = status;
    if (status != LoadingStatus::NOT_LOADED)
    {
        m_loadingBlobIndex = m_modelBlobIndex;
    }
    if (m_loadingBlobIndex.isValid())
    {
        dataChanged(m_loadingBlobIndex, m_loadingBlobIndex, QVector<int>(1, LOADING_STATUS_ROLE));
    }
}

BlobsListModel::LoadingStatus BlobsListModel::getModelBlobStatus() const
{
    return m_modelBlobLoadingStatus;
}

void BlobsListModel::setLoadingModelProgress(float loadingProgress)
{
    m_modelBlobLoadingProgress = loadingProgress;
    if (m_loadingBlobIndex.isValid())
    {
        dataChanged(m_loadingBlobIndex, m_loadingBlobIndex, QVector<int>(1, LOADING_PROGRESS));
    }
}

void BlobsListModel::updateModelBlobIndex()
{
    // find the correct m_modelBlobIndex
    for (int row = 0; row < rowCount(); ++row)
    {
        QModelIndex item = index(row, 0);
        if (item.data(LOADING_STATUS_ROLE).value<LoadingStatus>() != LoadingStatus::NOT_LOADED)
        {
            m_loadingBlobIndex = item;
            return;
        }
    }
}

bool BlobsListModel::filterBlobName(const utility::string_t& name) const
{
    if (m_filterType == FilterType::JustAllowedExtensions)
    {
        return m_fileFilter.match(name);
    }
    else
    {
        return true;
    }
}
