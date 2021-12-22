#pragma once

#include <QObject>
#include <Storage/FileUploader.h>
#include <Storage/IncludeAzureStorage.h>

enum class StorageConnectionStatus
{
    Authenticated,
    NotAuthenticated,
    CheckingCredentials,
    InvalidCredentials
};

/// Holds information about a file or folder in a storage account
struct StorageBlobInfo
{
    QString m_path;
    azure::storage::storage_uri m_uri;
};


/// Manages all interactions with the Azure Storage account.
class StorageAccount : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void ConnectionStatusChanged();

public:
    StorageAccount(FileUploader::UpdateCallback uploadCallback);
    ~StorageAccount();

    FileUploader* GetFileUploader() { return m_fileUploader.get(); }


    /// Retrieves the last used connection settings.
    bool LoadSettings();

    /// Stores the current connection settings.
    void SaveSettings() const;

    /// Sets (and saves) new connection settings.
    ///
    /// If already connected to a storage account, the connection is closed.
    void SetSettings(const QString& accountName, const QString& accountKey, const QString& endpointUrl);

    /// Returns the currently set storage account name.
    QString GetAccountName() const { return m_accountName; }

    /// Returns the currently set storage account key.
    QString GetAccountKey() const { return m_accountKey; }

    /// Returns the currently set storage endpoint URL.
    QString GetEndpointUrl() const { return m_endpointUrl; }


    /// Attempts to connect to the storage account with the previously configured account credentials.
    ///
    /// When the connection status changes, the signal ConnectionStatusChanged() is emitted.
    /// The current status can be retrieved with GetConnectionStatus().
    void ConnectToStorageAccount();

    /// If connected to a storage account, this disconnects from it.
    void DisconnectFromStorageAccount();

    /// Returns the current connection status.
    ///
    /// When the connection status changes, the signal ConnectionStatusChanged() is emitted.
    StorageConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }


    /// Attempts to create a new storage container in the connected storage account.
    ///
    /// There are strict rules for what a valid container name is. In case of failure, 'errorMsg' provides some details.
    bool CreateContainer(const QString& containerName, QString& errorMsg);

    /// Attempts to delete the storage container with the given name.
    ///
    /// In case of failure, 'errorMsg' provides some details.
    bool DeleteContainer(const QString& containerName, QString& errorMsg);

    /// Attempts to create a new, "empty" folder in the given storage container and with the given path.
    ///
    /// In case of failure, 'errorMsg' provides some details.
    /// Note: In Azure Storage the concept of a folder doesn't really exist, and thus empty folders don't exist either.
    /// To work around this, many tools create dummy files. ARRT does this too, the file is named '.EmptyFolderDummy'.
    /// ListBlobDirectory() ignores these files, such that in the ARRT UI "empty" folders indeed appear to be empty.
    bool CreateFolder(const QString& containerName, const QString& path, QString& errorMsg);

    /// Attempts to delete a file or folder.
    ///
    /// In case of failure, 'errorMsg' provides some details.
    bool DeleteItem(const QString& containerName, const QString& path, QString& errorMsg);

    /// Attempts to create a text file in the given container and with the given path and content.
    ///
    /// In case of failure, 'errorMsg' provides some details.
    bool CreateTextItem(const QString& containerName, const QString& path, const QString& content, QString& errorMsg);

    /// Retrieves the names of all storage containers that exist in the connected storage account.
    void ListContainers(std::vector<QString>& containers) const;

    /// Lists all files and folders that exist inside the given storage container and sub-path.
    ///
    /// This is not recursive, only the next level of items is returned.
    void ListBlobDirectory(const QString& containerName, const QString& prefixPath, std::vector<StorageBlobInfo>& directories, std::vector<StorageBlobInfo>& files) const;


    /// Returns the storage_uri for the storage container with the given path.
    azure::storage::storage_uri GetContainerUriFromName(const QString& containerName) const;

    /// Returns the cloud_blob_container that belongs to the given storage_uri.
    azure::storage::cloud_blob_container GetContainerFromUri(const azure::storage::storage_uri& containerUri) const;

    /// Returns the cloud_blob_container for the storage container with the given name.
    azure::storage::cloud_blob_container GetContainerFromName(const QString& containerName) const;

    /// Creates a Storage Access Signature (SAS) token that is needed to read or write to files or folders in Azure Storage.
    ///
    /// The token is only valid for a limited amount of time (usually a day).
    /// It is necessary to pass such tokens to the conversion service for it to be able to read and write its inputs and output.
    /// The token is also needed to create a SAS URL to any file. This is needed when loading a model into the ARR runtime.
    QString CreateSasToken(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 60 * 24) const;

    /// Create a URL to a file in Azure Storage and appends the necessary SAS token.
    ///
    /// Such a URL is necessary when loading a model into ARR.
    QString CreateSasURL(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 60 * 24) const;

private:
    void SetConnectionStatus(StorageConnectionStatus newStatus);
    void ConnectToStorageAccountThread(const QString& storageUrl, const azure::storage::storage_credentials& credentials);

    StorageConnectionStatus m_connectionStatus = StorageConnectionStatus::NotAuthenticated;

    QString m_accountName;
    QString m_accountKey;
    QString m_endpointUrl;

    std::unique_ptr<azure::storage::cloud_blob_client> m_blobClient;
    azure::storage::storage_credentials m_storageCredentials;
    std::unique_ptr<FileUploader> m_fileUploader;
};