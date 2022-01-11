#include <QApplication>
#include <QDir>
#include <QStringList>
#include <Storage/FileUploader.h>
#include <Storage/IncludeAzureStorage.h>
#include <Storage/StorageAccount.h>
#include <Utils/Logging.h>

FileUploader::FileUploader(UpdateCallback callback, StorageAccount* storageAccount)
    : m_remainingFilesCallback(std::move(callback))
    , m_storageAccount(storageAccount)
{
}

// upload multiple files to a blob storage directory. SourceRootDirectory will map to destDirectory
void FileUploader::UploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const QString& containerName, const QString& destDirectory)
{
    if (sourceFilePaths.isEmpty())
        return;

    const int toUpload = sourceFilePaths.size();
    const int remainingFiles = m_remainingFiles.fetch_add(toUpload) + toUpload;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, this]()
                              { m_remainingFilesCallback(remainingFiles); });

    auto asyncCallback = [sourceRootDirectory, sourceFilePaths, containerName, destDirectory, this](int from, int to)
    {
        for (int i = from; i < to; ++i)
        {
            UploadFileInternalSync(sourceRootDirectory, sourceFilePaths[i], containerName, destDirectory);
        }
    };

    // if there are multiple files, upload them in parallel
    static const int max_threads = 4;
    int numThreads = 0;
    int batchSize = 0;

    if (sourceFilePaths.count() <= max_threads)
    {
        numThreads = sourceFilePaths.count();
        batchSize = 1;
    }
    else
    {
        numThreads = max_threads;
        batchSize = sourceFilePaths.count() / numThreads;
    }

    int batchStart = 0;

    for (int i = 0; i < numThreads - 1; ++i)
    {
        std::thread(std::bind(asyncCallback, batchStart, batchStart + batchSize)).detach();
        batchStart += batchSize;
    }

    std::thread(std::bind(asyncCallback, batchStart, sourceFilePaths.count())).detach();
}

// upload one file to a blob storage directory synchronously. SourceRootDirectory will map to destDirectory
void FileUploader::UploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFilePath, const QString& containerName, const QString& destDirectory)
{
    // after it's opened, upload it to the blob path
    QString blobPath = (destDirectory + sourceRootDirectory.relativeFilePath(sourceFilePath));

    try
    {
#if NEW_STORAGE_SDK
        {
            auto container = m_storageAccount->GetStorageContainerFromName(containerName);

            Azure::Core::IO::FileBodyStream stream(sourceFilePath.toStdString());

            container.UploadBlob(blobPath.toStdString(), stream);
        }
#else
        {
            // open the file stream
            Concurrency::streams::fstream fileStream;
            Concurrency::streams::istream is = fileStream.open_istream(sourceFilePath.toStdWString()).get();

            auto container = m_storageAccount->GetContainerFromName(containerName);

            azure::storage::cloud_block_blob blob = container.get_block_blob_reference(blobPath.toStdWString());
            blob.upload_from_stream(is);
        }
#endif

        qInfo(LoggingCategory::AzureStorage)
            << "File upload finished."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath;
    }
    catch (...)
    {
        qCritical(LoggingCategory::AzureStorage)
            << "File upload failed."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath;
    }

    const int remainingFiles = m_remainingFiles.fetch_sub(1) - 1;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, this]()
                              { m_remainingFilesCallback(remainingFiles); });
}
