#include <Model/AzureStorageManager.h>
#include <Model/Log/LogHelpers.h>
#include <Model/StorageUtils/FileUploader.h>
#include <QDirIterator>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/BlobExplorer/DirectoryProvider.h>

BlobExplorerModel::BlobExplorerModel(AzureStorageManager* storageManager, bool allowsUpload, bool directoryExplorer, const QString& allowedExtensions, QString uploadFileFilters, QString container, const QString& directory, QObject* parent)
    : QObject(parent)
    , m_allowsUpload(allowsUpload)
    , m_storageManager(storageManager)
    , m_fileUploader(new FileUploader([this](int remaining, bool thereWasAnError) { fileCountChanged(remaining, thereWasAnError); }))
    , m_containerName(new Value<QString>(std::move(container), this))
    , m_isDirectoryExplorer(directoryExplorer)
    , m_uploadFileFilters(std::move(uploadFileFilters))
{
    m_directoryProvider = new DirectoryProvider(storageManager, m_containerName, directory, this);
    m_blobsListModel = new BlobsListModel(m_directoryProvider, storageManager, allowedExtensions, m_containerName, this);

    connect(m_blobsListModel, &BlobsListModel::blobSelected, this,
            [this](const QString& path, const QString& url) {
                setSelectedBlob(path, url);
                if (!m_isDirectoryExplorer)
                {
                    submit();
                }
            });

    connect(m_blobsListModel, &BlobsListModel::currentBlobChanged, this,
            [this]() {
                BlobsListModel::EntryType type;
                QString path;
                QString url;
                m_blobsListModel->getCurrentItem(type, path, url);
                if (m_isDirectoryExplorer && type == BlobsListModel::EntryType::Directory)
                {
                    setSelectedBlob(path, url);
                }
                else if (!m_isDirectoryExplorer && type == BlobsListModel::EntryType::Model)
                {
                    setSelectedBlob(path, url);
                }
                else
                {
                    setSelectedBlob({}, {});
                }
            });

    connect(m_directoryProvider, &DirectoryProvider::directoryChanged, this,
            [this]() {
                Q_EMIT directoryChanged();
            });
}

BlobExplorerModel::BlobExplorerModel(AzureStorageManager* storageManager, bool allowsUpload, const QString& allowedExtensions, QString uploadFileFilters, QString container, const QString& directory, QObject* parent)
    : BlobExplorerModel(storageManager, allowsUpload, false, allowedExtensions, std::move(uploadFileFilters), std::move(container), directory, parent)
{
}

BlobExplorerModel::BlobExplorerModel(AzureStorageManager* storageManager, QString container, const QString& directory, QObject* parent)
    : BlobExplorerModel(storageManager, false, true, {}, {}, std::move(container), directory, parent)
{
}

void BlobExplorerModel::setContainer(const QString& containerName)
{
    if (m_containerName->get() != containerName)
    {
        m_containerName->set(containerName);
        m_directoryProvider->setDirectory({});
    }
}


DirectoryProvider* BlobExplorerModel::getDirectoryModel() const
{
    return m_directoryProvider;
}

BlobsListModel* BlobExplorerModel::getBlobsModel() const
{
    return m_blobsListModel;
}

void BlobExplorerModel::setDirectory(const QString& directory)
{
    m_directoryProvider->setDirectory(directory);
}

QString BlobExplorerModel::getDirectory() const
{
    return m_directoryProvider->getDirectory();
}

void BlobExplorerModel::setSelectedBlob(QString path, QString url)
{
    m_selectedBlobPath = std::move(path);
    m_selectedBlobUrl = std::move(url);
    Q_EMIT selectionChanged();
}
QString BlobExplorerModel::getSelectedBlobUrl() const
{
    return m_selectedBlobUrl;
}
QString BlobExplorerModel::getSelectedBlobPath() const
{
    return m_selectedBlobPath;
}
QString BlobExplorerModel::getSelectedRelativeBlobPath() const
{
    return QString::fromStdWString(m_directoryProvider->getRelativePath(m_selectedBlobPath.toStdWString()));
}

bool BlobExplorerModel::acceptSelectedBlob() const
{
    return !m_selectedBlobPath.isEmpty();
}

void BlobExplorerModel::submit()
{
    Q_EMIT submitted();
}

QString BlobExplorerModel::getUploadStatus() const
{
    if (m_totalFiles == 0)
    {
        return {};
    }
    else if (m_remainingFiles > 0)
    {
        return tr("Uploading %1/%2 files").arg(m_totalFiles - m_remainingFiles).arg(m_totalFiles);
    }
    else if (m_errorWhenUploadingFiles)
    {
        return tr("Upload failed (%1 files)").arg(m_totalFiles);
    }
    else
    {
        return tr("Upload completed (%1 files)").arg(m_totalFiles);
    }
}

// upload a series of files and directory (local paths) to the current container and directory
void BlobExplorerModel::uploadBlobs(const QDir& rootDir, const QStringList& localPaths)
{
    if (!m_allowsUpload)
    {
        return;
    }
    QStringList toUpload = getFullFileList(localPaths);
    if (m_remainingFiles == 0)
    {
        // reset
        m_totalFiles = 0;
        m_errorWhenUploadingFiles = false;
    }
    m_totalFiles += localPaths.size();

    // first try to create the container if it doesn't exist
    QString containerName = m_containerName->get();
    QString directory = getDirectory();
    QPointer<BlobExplorerModel> thisPtr = this;
    m_storageManager->createContainer(m_containerName->get(), [thisPtr, containerName, rootDir, localPaths, directory](bool succeeded) {
        if (thisPtr)
        {
            if (succeeded)
            {
                auto container = thisPtr->m_storageManager->getContainerFromName(containerName);
                thisPtr->m_fileUploader->uploadFilesAsync(rootDir, localPaths, container, directory);
            }
            else
            {
                qWarning(LoggingCategory::azureStorage) << tr("Could not create container: ") << containerName;
            }
        }
    });
}

// it converts a set of files+directories to a list of files. Utility function used by the view to know how many files
// are actually going to be uploaded
QStringList BlobExplorerModel::getFullFileList(const QStringList& localPaths) const
{
    QStringList fullFileList;
    for (const auto& path : localPaths)
    {
        QDir dir(path);
        if (dir.exists())
        {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                it.next();
                fullFileList.append(it.filePath());
            }
        }
        else
        {
            if (QFileInfo::exists(path))
            {
                fullFileList.append(path);
            }
        }
    }
    return fullFileList;
}

// true if the exlorer allows blobs to be uploaded
bool BlobExplorerModel::allowsUpload() const
{
    return m_allowsUpload;
}

QString BlobExplorerModel::getUploadFileFilters() const
{
    return m_uploadFileFilters;
}

int BlobExplorerModel::getUploadProgress() const
{
    if (m_remainingFiles == 0 || m_totalFiles == 0)
    {
        return -1;
    }
    else
    {
        return 100 - (100 * m_remainingFiles / m_totalFiles);
    }
}

void BlobExplorerModel::fileCountChanged(int remainingFiles, bool errorWhenUploadingFiles)
{
    m_remainingFiles = remainingFiles;
    m_errorWhenUploadingFiles = (m_errorWhenUploadingFiles || errorWhenUploadingFiles);
    if (m_remainingFiles == 0)
    {
        getBlobsModel()->refresh();
    }
    Q_EMIT uploadStatusChanged();
    Q_EMIT uploadProgressChanged();
    if (remainingFiles == 0)
    {
        Q_EMIT uploadFinished(!errorWhenUploadingFiles);
    }
}
