#include <QApplication>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QUuid>
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

    if (m_remainingFiles == 0)
    {
        // reset to zero, if there are currently no file uploads running
        m_bytesRead = 0;
        m_totalBytesToRead = 0;
    }

    // add up all the remaining bytes to upload
    for (const QString& file : sourceFilePaths)
    {
        QFileInfo fi(file);
        m_totalBytesToRead.fetch_add(fi.size());
    }

    const int64_t read = m_bytesRead;
    const int64_t total = m_totalBytesToRead;
    const float percentage = (total > 0) ? (double)read / (double)total : 1.0;

    const int toUpload = sourceFilePaths.size();
    const int remainingFiles = m_remainingFiles.fetch_add(toUpload) + toUpload;
    QMetaObject::invokeMethod(QApplication::instance(), [this, remainingFiles, percentage]()
                              { m_remainingFilesCallback(remainingFiles, percentage); });


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

void FileUploader::NotifyBytesRead(int64_t bytes)
{
    m_bytesRead.fetch_add(bytes);

    const int remainingFiles = m_remainingFiles;

    int64_t read = m_bytesRead;
    int64_t total = m_totalBytesToRead;
    const float percentage = (total > 0) ? (double)read / (double)total : 1.0;

    QMetaObject::invokeMethod(QApplication::instance(), [this, remainingFiles, percentage]()
                              { m_remainingFilesCallback(remainingFiles, percentage); });
}

#if NEW_STORAGE_SDK

class FileStream : public Azure::Core::IO::BodyStream
{
public:
    FileStream(const std::string& path)
        : m_file(path.c_str())
    {
        if (m_file.open(QIODevice::OpenModeFlag::ReadOnly))
        {
            m_size = m_file.size();
        }

        // we upload files in 4 MB chunks ('blocks')
        // blocks of up to 100 MB would be possible, but become more and more unreliable
        m_maxBytes = 1024 * 1024 * 4;
    }

    virtual int64_t Length() const override
    {
        return std::min(m_maxBytes, (m_size - m_offset));
    }

    virtual void Rewind() override
    {
        // in case of an error, the Storage SDK will rewind and try again
        // this only affects the current block, though, not the entire file
        m_read = 0;
        m_file.seek(m_offset);
    }

    bool FinishBlock()
    {
        // the next block starts at this offset
        m_offset += m_read;
        m_read = 0;
        return m_offset < m_size; // return true, if there is more work left to do
    }

    int64_t GetBytesRead() const
    {
        return m_read;
    }

private:
    virtual size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& /*context*/) override
    {
        count = std::min<size_t>(m_maxBytes - m_read, count);

        if (count == 0)
            return 0;

        const int64_t read = m_file.read((char*)buffer, count);
        m_read += read;
        return (size_t)read;
    }

    QFile m_file;
    int64_t m_read = 0;
    int64_t m_size = 0;
    int64_t m_offset = 0;
    int64_t m_maxBytes = 0;
};

#endif

// upload one file to a blob storage directory synchronously. SourceRootDirectory will map to destDirectory
void FileUploader::UploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFilePath, const QString& containerName, const QString& destDirectory)
{
    // after it's opened, upload it to the blob path
    QString blobPath = (destDirectory + sourceRootDirectory.relativeFilePath(sourceFilePath));

    const QFileInfo fi(sourceFilePath);
    const int64_t fileSize = fi.size();


    try
    {
#if NEW_STORAGE_SDK
        {
            auto container = m_storageAccount->GetStorageContainerFromName(containerName);

            {
                // first upload a dummy file without blob blocks, such that any potentially existing blocks on the server get discarded
                // see https://gauravmantri.com/2013/05/18/windows-azure-blob-storage-dealing-with-the-specified-blob-or-block-content-is-invalid-error/
                {
                    QByteArray data = QString("dummy").toUtf8();
                    Azure::Core::IO::MemoryBodyStream dummyStream(reinterpret_cast<const uint8_t*>(data.data()), data.length());
                    container.UploadBlob(blobPath.toStdString(), dummyStream);
                }

                // now delete that blob again, to purge everything associated with this blob
                // after this, we can safely upload the data in multiple blocks
                container.DeleteBlob(blobPath.toStdString());
            }

            auto blobClient = container.GetBlockBlobClient(blobPath.toStdString());
            FileStream stream(sourceFilePath.toStdString());

            std::vector<std::string> blocks;

            // we can't upload files larger than 100MB in one operation
            // for one, this is not even allowed
            // and two, the larger the block, the more likely it becomes that the upload fails
            do
            {
                // we need a unique name for each staged block, so we generate a GUID
                QUuid guid = QUuid::createUuid();
                // have to remove the dashes to make the block identifier valid
                std::string guidString = guid.toString(QUuid::WithoutBraces).replace("-", "").toStdString();
                blocks.push_back(guidString);
                blobClient.StageBlock(guidString, stream);

                // update the progress every time a block has finished uploading
                NotifyBytesRead(stream.GetBytesRead());

            } while (stream.FinishBlock());

            // tell Azure Storage that the file is finished and from which blocks it is made up
            blobClient.CommitBlockList(blocks);
        }
#else
        {
            // open the file stream
            Concurrency::streams::fstream fileStream;
            Concurrency::streams::istream is = fileStream.open_istream(sourceFilePath.toStdWString()).get();

            auto container = m_storageAccount->GetContainerFromName(containerName);

            azure::storage::cloud_block_blob blob = container.get_block_blob_reference(blobPath.toStdWString());
            blob.upload_from_stream(is);

            NotifyBytesRead(fileSize);
        }
#endif

        qInfo(LoggingCategory::AzureStorage)
            << "File upload finished."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath;
    }
    catch (const std::exception& e)
    {
        qCritical(LoggingCategory::AzureStorage)
            << "File upload failed."
            << "\n  Src: " << sourceFilePath
            << "\n  Dst: " << blobPath
            << "\n  Msg: " << e.what();
    }

    const int remainingFiles = m_remainingFiles.fetch_sub(1) - 1;

    int64_t read = m_bytesRead;
    int64_t total = m_totalBytesToRead;
    const float percentage = (total > 0) ? (double)read / (double)total : 1.0;

    QMetaObject::invokeMethod(QApplication::instance(), [this, remainingFiles, percentage]()
                              { m_remainingFilesCallback(remainingFiles, percentage); });
}
