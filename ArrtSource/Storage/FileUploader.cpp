#include <QApplication>
#include <QDir>
#include <QStringList>
#include <Storage/FileUploader.h>
#include <Storage/IncludeAzureStorage.h>
#include <Utils/LogHelpers.h>

FileUploader::FileUploader(UpdateCallback callback)
    : m_remainingFilesCallback(std::move(callback))
{
}

// upload multiple files to a blob storage directory. SourceRootDirectory will map to destDirectory
void FileUploader::UploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const azure::storage::cloud_blob_container& container, const QString& destDirectory)
{
    if (sourceFilePaths.isEmpty())
        return;

    m_hadErrors = false;

    const int toUpload = sourceFilePaths.size();
    const int remainingFiles = m_remainingFiles.fetch_add(toUpload) + toUpload;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, this]()
                              { m_remainingFilesCallback(remainingFiles, false); });

    auto asyncCallback = [sourceRootDirectory, sourceFilePaths, container, destDirectory, this](int from, int to)
    {
        for (int i = from; i < to; ++i)
        {
            UploadFileInternalSync(sourceRootDirectory, sourceFilePaths[i], container, destDirectory);
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
void FileUploader::UploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFilePath, const azure::storage::cloud_blob_container& container, const QString& destDirectory)
{
    // open the file stream
    Concurrency::streams::fstream fileStream;
    Concurrency::streams::istream is = fileStream.open_istream(sourceFilePath.toStdWString()).get();

    // after it's opened, upload it to the blob path
    QString blobPath = (destDirectory + sourceRootDirectory.relativeFilePath(sourceFilePath));

    azure::storage::cloud_block_blob blob = container.get_block_blob_reference(blobPath.toStdWString());
    try
    {
        blob.upload_from_stream(is);

        qInfo(LoggingCategory::AzureStorage)
            << "File upload finished."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath;
    }
    catch (...)
    {
        m_hadErrors = true;

        qCritical(LoggingCategory::AzureStorage)
            << "File upload failed."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath;
    }

    const int remainingFiles = m_remainingFiles.fetch_sub(1) - 1;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, this]()
                              { m_remainingFilesCallback(remainingFiles, m_hadErrors); });
}