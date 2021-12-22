#pragma once

#include <atomic>
#include <functional>

class QDir;
class QStringList;
class QString;

namespace azure::storage
{
    class cloud_blob_container;
}

/// Used to upload files to Azure Storage asynchronously.
class FileUploader
{
public:
    using UpdateCallback = std::function<void(int remainingFiles)>;

    FileUploader(UpdateCallback callback);

    /// Uploads multiple files to a blob storage directory.
    ///
    /// The relative path from sourceRootDirectory to sourceFilePaths is used to determine the relative sub-path in destDirectory.
    void UploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const azure::storage::cloud_blob_container& container, const QString& destDirectory);

private:
    UpdateCallback m_remainingFilesCallback;
    std::atomic<int> m_remainingFiles = 0;

    void UploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFile, const azure::storage::cloud_blob_container& container, const QString& destDirectory);
};
