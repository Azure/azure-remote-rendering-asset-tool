#pragma once
#include <Model/IncludesAzureStorage.h>
#include <Model/Settings/AccountConnectionStatus.h>
#include <Model/StorageUtils/Cancellable.h>
#include <QDir>
#include <QObject>

namespace azure
{
    namespace storage
    {
        class cloud_blob_client;
        class list_blob_item;
    } // namespace storage
} // namespace azure


// Utility class for querying the azure blob storage api for a certain storage account

class AzureStorageManager : public QObject
{
    Q_OBJECT
public:
    AzureStorageManager(QObject* parent);
    ~AzureStorageManager();

    bool isValid();

    // auth for the storage account
    void connectAccount(const wchar_t* url, const wchar_t* name, const wchar_t* key);
    void reconnectAccount();

    // list all of the blob containers, by calling the callback with the container names
    std::shared_ptr<Cancellable> getContainers(const QObject* context, std::function<void(utility::string_t)> callback, std::function<void()> endCallback);

    // return the uri of the container from the name
    azure::storage::storage_uri getContainerUriFromName(const QString& containerName) const;

    // return the container from the name, using the client credentials
    azure::storage::cloud_blob_container getContainerFromName(const QString& containerName) const;

    // return the container from the uri, using the client credentials
    azure::storage::cloud_blob_container getContainerFromUri(const azure::storage::storage_uri& containerUri) const;

    // create a new container. If the container is created (or is already there) returns true in the callback. The callback is executed in the main thread
    void createContainer(const QString& containerNamebool, std::function<void(bool)> endCallback);

    // fetch all of the blobs of a container in a flat list, in an asynchronous way, and segmented. Callback is executed on the main thread and it gets passed the segment,
    // which also contains a continuation token (empty when it's the last call)
    std::shared_ptr<Cancellable> getAllBlobsAsyncSegmented(const QObject* context, const QString& container, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback);

    // fetch just the elements in the current directory passed. It will return also directories
    std::shared_ptr<Cancellable> getBlobsInDirectoryAsyncSegmented(const QObject* context, const QString& container, utility::string_t directory, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback);

    // return all the subdirectories in a certain directory
    void getSubDirectories(const QString& containerName, const QString& directory, const std::function<void(const azure::storage::cloud_blob_directory& dir)>& callback);

    // delete a blob or a whole directory
    void deleteBlobOrDirectory(const QString& containerName, const utility::string_t& prefix);

    // generate a Sas token for a specified blob with a timeout. The accessTypeMask is composed of azure::storage::blob_shared_access_policy flags
    utility::string_t getSasToken(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 20) const;

    // generate a Sas for a specified blob with a timeout. The accessTypeMask is composed of azure::storage::blob_shared_access_policy flags
    utility::string_t getSasUrl(const azure::storage::storage_uri& uri, int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read, unsigned int minutes = 20) const;

    // write a text file to azure in a certain container, returns true if the upload succeeded
    bool writeTextFileSync(const utility::string_t& data, const azure::storage::storage_uri& containerUri, const utility::string_t& fileName);

    // read a text file from a container, return true if the download succeeded
    bool readTextFileSync(const azure::storage::storage_uri& containerUri, const utility::string_t& fileName, utility::string_t& outText);

    // return the underlying azure api for the blob client
    azure::storage::cloud_blob_client* getBlobClient() const;

    // return account name
    utility::string_t getAccountName();

    AccountConnectionStatus getStatus() const { return m_status; }

Q_SIGNALS:
    void onStatusChanged();

private:
    mutable std::unique_ptr<azure::storage::cloud_blob_client> m_blobClient;
    azure::storage::storage_credentials m_storageCredentials;
    utility::string_t m_storageUrl;
    utility::string_t m_accountName;
    utility::string_t m_key;
    AccountConnectionStatus m_status = AccountConnectionStatus::NotAuthenticated;

    void setStatus(AccountConnectionStatus status);
    void connect();
};
