#pragma once
#include <QDir>
#include <QObject>
#include <Utils/Value.h>

class BlobDirsTreeModel;
class BlobsListModel;
class AzureStorageManager;
class FileUploader;
class DirectoryProvider;

// model for the BlobExplorerView. It represents the navigation of a blob container, with a current directory and
// a list of blobs

class BlobExplorerModel : public QObject
{
    Q_OBJECT

public:
    // constructor for a blob explorer
    BlobExplorerModel(AzureStorageManager* storageManager, bool allowsUpload, const QString& allowedExtensions, QString uploadFileFilters, QString container, const QString& directory, QObject* parent);
    // constructor for the "directory explorer"
    BlobExplorerModel(AzureStorageManager* storageManager, QString container, const QString& directory, QObject* parent);

    void setContainer(const QString& containerName);

    DirectoryProvider* getDirectoryModel() const;

    BlobsListModel* getBlobsModel() const;

    void setDirectory(const QString& directory);
    QString getDirectory() const;

    void setSelectedBlob(QString path, QString url);
    QString getSelectedBlobUrl() const;
    QString getSelectedBlobPath() const;
    QString getSelectedRelativeBlobPath() const;

    bool acceptSelectedBlob() const;
    void submit();

    QString getUploadStatus() const;

    // upload a series of files and directory (local paths) to the current container and directory
    void uploadBlobs(const QDir& rootDir, const QStringList& localPaths);

    // it converts a set of files+directories to a list of files. Utility function used by the view to know how many files
    // are actually going to be uploaded
    QStringList getFullFileList(const QStringList& localPaths) const;

    // true if the explorer allows blobs to be uploaded
    bool allowsUpload() const;

    // return the string used in the file/directory/selector to filter the files
    QString getUploadFileFilters() const;

    // return the upload progress. -1 if no upload is in progress
    int getUploadProgress() const;

Q_SIGNALS:
    void directoryChanged();
    void submitted();
    void uploadStatusChanged();
    void selectionChanged();
    void uploadProgressChanged();
    void uploadFinished(bool success);

private:
    bool const m_allowsUpload;

    AzureStorageManager* const m_storageManager;

    DirectoryProvider* m_directoryProvider = {};
    BlobsListModel* m_blobsListModel = {};

    QString m_selectedBlobPath;
    QString m_selectedBlobUrl;
    int m_remainingFiles = 0;
    int m_totalFiles = 0;
    bool m_errorWhenUploadingFiles = false;
    QScopedPointer<FileUploader> m_fileUploader;
    Value<QString>* m_containerName;
    const bool m_isDirectoryExplorer = false;
    const QString m_uploadFileFilters;

    BlobExplorerModel(AzureStorageManager* storageManager, bool allowsUpload, bool directoryExplorer, const QString& allowedExtensions, QString uploadFileFilters, QString container, const QString& directory, QObject* parent);

    void fileCountChanged(int remainingFiles, bool errorWhenUploadingFiles);
};
