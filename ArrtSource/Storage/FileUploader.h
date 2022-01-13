#pragma once

#include <atomic>
#include <functional>

class QDir;
class QStringList;
class QString;
class StorageAccount;

/// Used to upload files to Azure Storage asynchronously.
class FileUploader
{
public:
    using UpdateCallback = std::function<void(int remainingFiles, float percentage)>;

    FileUploader(UpdateCallback callback, StorageAccount* storageAccount);

    /// Uploads multiple files to a blob storage directory.
    ///
    /// The relative path from sourceRootDirectory to sourceFilePaths is used to determine the relative sub-path in destDirectory.
    void UploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const QString& containerName, const QString& destDirectory);

    void NotifyBytesRead(int64_t bytes);

private:
    UpdateCallback m_remainingFilesCallback;
    std::atomic<int64_t> m_totalBytesToRead = 0;
    std::atomic<int64_t> m_bytesRead = 0;
    std::atomic<int> m_remainingFiles = 0;

    void UploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFile, const QString& containerName, const QString& destDirectory);

    StorageAccount* m_storageAccount = nullptr;
};
