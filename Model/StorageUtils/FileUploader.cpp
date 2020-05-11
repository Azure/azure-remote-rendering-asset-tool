#include <Model/IncludesAzureStorage.h>
#include <Model/Log/LogHelpers.h>
#include <Model/StorageUtils/FileUploader.h>
#include <QApplication>
#include <QDir>
#include <QStringList>

FileUploader::FileUploader(UpdateCallback callback)
    : m_remainingFilesCallback(std::move(callback))
{
}

// upload multiple files to a blob storage directory. SourceRootDirectory will map to destDirectory
void FileUploader::uploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const azure::storage::cloud_blob_container& container, const QString& destDirectory)
{
    const int toUpload = sourceFilePaths.size();
    const int remainingFiles = m_remainingFiles.fetch_add(toUpload) + toUpload;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, this]() {
        m_remainingFilesCallback(remainingFiles, false);
    });

    auto asyncCallback = [sourceRootDirectory, sourceFilePaths, container, destDirectory, this](int from, int to) {
        for (int i = from; i < to; ++i)
        {
            uploadFileInternalSync(sourceRootDirectory, sourceFilePaths[i], container, destDirectory);
        }
    };

    //divide the task in 16 parallel batches (if it can)
    static const int max_threads = 16;
    int batchStart = 0;
    int batchSize = sourceFilePaths.count() / max_threads;

    if (batchSize > 0)
    {
        for (int i = 0; i < max_threads - 1; ++i)
        {
            std::thread(std::bind(asyncCallback, batchStart, batchStart + batchSize)).detach();
            batchStart += batchSize;
        }
    }
    std::thread(std::bind(asyncCallback, batchStart, sourceFilePaths.count())).detach();
}

// upload one file to a blob storage directory synchronously. SourceRootDirectory will map to destDirectory
void FileUploader::uploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFilePath, const azure::storage::cloud_blob_container& container, const QString& destDirectory)
{
    // open the file stream
    Concurrency::streams::fstream fileStream;
    Concurrency::streams::istream is = fileStream.open_istream(sourceFilePath.toStdWString()).get();

    // after it's opened, upload it to the blob path
    QString blobPath = (destDirectory + sourceRootDirectory.relativeFilePath(sourceFilePath));

    bool thereWasAnError = false;
    azure::storage::cloud_block_blob blob = container.get_block_blob_reference(blobPath.toStdWString());
    try
    {
        blob.upload_from_stream(is);
    }
    catch (...)
    {
        qWarning(LoggingCategory::azureStorage)
            << QCoreApplication::tr("Error while uploading file:") << blobPath;
        thereWasAnError = true;
    }

    const int remainingFiles = m_remainingFiles.fetch_sub(1) - 1;
    QMetaObject::invokeMethod(QApplication::instance(), [remainingFiles, thereWasAnError, this]() {
        m_remainingFilesCallback(remainingFiles, thereWasAnError);
    });
}
