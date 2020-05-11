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

// Class used to trigger and hold the status of an asynchronous file upload.
// It will send the status update by calling a callback with the remaining files

class FileUploader
{
public:
    typedef std::function<void(int remainingFiles, bool thereWasAnError)> UpdateCallback;

    FileUploader(UpdateCallback callback);

    // upload multiple files to a blob storage directory. SourceRootDirectory will map to destDirectory
    void uploadFilesAsync(const QDir& sourceRootDirectory, const QStringList& sourceFilePaths, const azure::storage::cloud_blob_container& container, const QString& destDirectory);

private:
    UpdateCallback m_remainingFilesCallback;
    std::atomic<int> m_remainingFiles = 0;

    void uploadFileInternalSync(const QDir& sourceRootDirectory, const QString& sourceFile, const azure::storage::cloud_blob_container& container, const QString& destDirectory);
};
