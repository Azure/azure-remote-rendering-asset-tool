#include <Model\AzureStorageManager.h>
#include <QObject>
#include <QStandardItemModel>
#include <ViewModel\BlobExplorer\DirectoryProvider.h>

namespace
{
    bool isDirectoryValid(const QString& directory)
    {
        if (!directory.isEmpty())
        {
            if (*directory.rbegin() != '/')
            {
                return false;
            }
            if (directory.size() == 1)
            {
                //this is when the directory=="/" which is not a valid prefix
                return false;
            }
            if (directory.indexOf("//") != -1)
            {
                return false;
            }
        }
        return true;
    }
} // namespace

DirectoryProvider::DirectoryProvider(AzureStorageManager* storageManager, Value<QString>* container, const QString& directory, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_container(container)
{
    setDirectory(directory);
}

void DirectoryProvider::goToParentDirectory()
{
    if (!canGoToParentDirectory())
    {
        return;
    }

    //starts iterating from the one to the last, since we assume that non empty directories always end with '/'
    int parentSlash = m_directory.size() - 2;
    for (auto it = m_directory.rbegin() + 1; it != m_directory.rend(); ++it)
    {
        if (*it == '/')
        {
            break;
        }
        --parentSlash;
    }
    if (parentSlash == -1)
    {
        setDirectory("");
    }
    else
    {
        setDirectory(m_directory.left(parentSlash + 1)); // +1 to keep the found slash
    }
}

bool DirectoryProvider::canGoToParentDirectory()
{
    return !m_directory.isEmpty();
}

QString DirectoryProvider::getDirectory() const
{
    return m_directory;
}

bool DirectoryProvider::setDirectory(const QString& directory)
{
    if (m_directory != directory)
    {
        if (!isDirectoryValid(directory))
        {
            return false;
        }
        m_directory = directory;

        m_prefix = directory.toStdWString();
        Q_EMIT directoryChanged();
        return true;
    }
    return true;
}

void DirectoryProvider::navigateIntoDirectory(const QString& directory)
{
    QString dir = directory;
    if (dir.endsWith('/'))
    {
        dir.chop(1);
    }
    if (dir.startsWith('/'))
    {
        dir.remove(0, 1);
    }

    setDirectory(m_directory + dir + "/");
}

const utility::string_t& DirectoryProvider::getPrefix() const
{
    return m_prefix;
}

utility::string_t DirectoryProvider::getRelativePath(const utility::string_t& absolutePath) const
{
    if (absolutePath._Starts_with(m_prefix))
    {
        return absolutePath.substr(m_prefix.size());
    }
    return absolutePath;
}

bool DirectoryProvider::isBlobInDirectory(const utility::string_t& blobPath) const
{
    return blobPath._Starts_with(m_prefix);
}

QPointer<QAbstractItemModel> DirectoryProvider::createNextDirectoriesModel(const QString& fromDirectory)
{
    auto* model = new QStandardItemModel();
    QString container = m_container ? m_container->get() : QString();
    if (!container.isEmpty())
    {
        m_storageManager->getBlobsInDirectoryAsyncSegmented(this, container, fromDirectory.toStdWString(), [model = QPointer<QStandardItemModel>(model)](const azure::storage::list_blob_item_segment& segment) {
            if (model)
            {
                for (const azure::storage::list_blob_item& item : segment.results())
                {
                    if (!item.is_blob())
                    {
                        azure::storage::cloud_blob_directory dir = item.as_directory();
                        auto* newItem = new QStandardItem();
                        auto dirString = QString::fromStdWString(dir.prefix());
                        newItem->setData(dirString, Qt::DisplayRole);
                        newItem->setData(dirString, Qt::UserRole);
                        model->appendRow(newItem);
                    }
                }
            }
        });
    }
    return model;
}
