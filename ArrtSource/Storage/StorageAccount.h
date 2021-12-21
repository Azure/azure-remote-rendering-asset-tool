#pragma once

#include <QObject>
#include <Storage/FileUploader.h>
#include <Storage/IncludeAzureStorage.h>
#include <Utils/ConnectionStatus.h>

class StorageAccount : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void ConnectionStatusChanged();

public:
    StorageAccount(FileUploader::UpdateCallback uploadCallback);
    ~StorageAccount();

    FileUploader* GetFileUploader() { return m_fileUploader.get(); }

    bool LoadSettings();
    void SaveSettings() const;
    void SetSettings(const QString& accountName, const QString& accountKey, const QString& endpointUrl);

    struct BlobInfo
    {
        QString m_path;
        azure::storage::storage_uri m_uri;
    };

    void ListContainers(std::vector<QString>& containers) const;
    void ListBlobDirectory(const QString& containerName, const QString& prefixPath, std::vector<BlobInfo>& directories, std::vector<BlobInfo>& files) const;

    void ConnectToStorageAccount();
    void DisconnectFromStorageAccount();

    QString GetAccountName() const { return m_accountName; }
    QString GetAccountKey() const { return m_accountKey; }
    QString GetEndpointUrl() const { return m_endpointUrl; }

    AccountConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }

    QString GetSasToken(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 60 * 24) const;
    QString GetSasUrl(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 60 * 24) const;

    bool CreateContainer(const QString& containerName, QString& errorMsg);
    bool DeleteContainer(const QString& containerName, QString& errorMsg);

    bool CreateFolder(const QString& containerName, const QString& path, QString& errorMsg);
    bool DeleteItem(const QString& containerName, const QString& path, QString& errorMsg);
    bool CreateTextItem(const QString& containerName, const QString& path, const QString& content, QString& errorMsg);

    azure::storage::storage_uri GetContainerUriFromName(const QString& containerName) const;
    azure::storage::cloud_blob_container GetContainerFromUri(const azure::storage::storage_uri& containerUri) const;
    azure::storage::cloud_blob_container GetContainerFromName(const QString& containerName) const;

private:
    void SetConnectionStatus(AccountConnectionStatus newStatus);
    void Reconnect();

    AccountConnectionStatus m_connectionStatus = AccountConnectionStatus::NotAuthenticated;

    QString m_accountName;
    QString m_accountKey;
    QString m_endpointUrl;

    std::unique_ptr<azure::storage::cloud_blob_client> m_blobClient;
    azure::storage::storage_credentials m_storageCredentials;
    std::unique_ptr<FileUploader> m_fileUploader;
};