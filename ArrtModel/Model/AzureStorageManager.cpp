#include <Model/AzureStorageManager.h>
#include <Model/IncludesAzureStorage.h>
#include <Model/Log/LogHelpers.h>
#include <Model/StorageUtils/BlobFetcher.h>
#include <Model/StorageUtils/ContainerFetcher.h>
#include <QApplication>
#include <QCoreApplication>
#include <QPointer>
#include <QThread>

AzureStorageManager::AzureStorageManager(QObject* parent)
    : QObject(parent)
{
}

AzureStorageManager::~AzureStorageManager()
{
}

void AzureStorageManager::connectAccount(const wchar_t* url, const wchar_t* name, const wchar_t* key)
{
    if (getStatus() == AccountConnectionStatus::Connected &&
        m_accountName == name && m_key == key && m_storageUrl == url)
    {
        return;
    }

    m_accountName = name;
    m_key = key;
    m_storageUrl = url;

    try
    {
        m_storageCredentials = azure::storage::storage_credentials(m_accountName, m_key);
    }
    catch (std::exception& /*e*/)
    {
        m_blobClient.reset();
        setStatus(AccountConnectionStatus::FailedToConnect);
        return;
    }

    connect();
}

void AzureStorageManager::reconnectAccount()
{
    connect();
}

void AzureStorageManager::connect()
{
    using namespace azure::storage;

    if (m_blobClient)
    {
        setStatus(AccountConnectionStatus::Disconnected);
        m_blobClient = nullptr;
    }

    if (m_accountName.empty() || m_key.empty() || m_storageUrl.empty())
    {
        return;
    }

    setStatus(AccountConnectionStatus::Connecting);

    QPointer<AzureStorageManager> thisPtr = this;
    std::thread([this, thisPtr, storageUrl = m_storageUrl, storageCredentials = m_storageCredentials]() {
        bool storageIsValid = true;
        std::unique_ptr<azure::storage::cloud_blob_client> client;
        try
        {
            client = std::make_unique<cloud_blob_client>(storage_uri(storageUrl), storageCredentials);
            cloud_queue_client cc(storage_uri(storageUrl), storageCredentials);
            continuation_token ct;
            cc.list_queues_segmented(ct);
        }
        catch (std::exception& /*e*/)
        {
            storageIsValid = false;
            client.reset();
        }

        QMetaObject::invokeMethod(QApplication::instance(), [this, thisPtr, storageIsValid, client = std::move(client)]() mutable {
            if (thisPtr)
            {
                m_blobClient = std::move(client);
                setStatus(storageIsValid ? AccountConnectionStatus::Connected : AccountConnectionStatus::FailedToConnect);
            }
        });
    })
        .detach();
}

bool AzureStorageManager::isValid()
{
    return getBlobClient() != nullptr;
}

azure::storage::cloud_blob_client* AzureStorageManager::getBlobClient() const
{
    return m_blobClient.get();
}

// return account name

utility::string_t AzureStorageManager::getAccountName()
{
    return m_accountName;
}

std::shared_ptr<Cancellable> AzureStorageManager::getContainers(const QObject* context, std::function<void(utility::string_t)> callback, std::function<void()> endCallback)
{
    if (auto* bc = getBlobClient())
    {
        return ContainerFetcher::startFetching(
            context, bc,
            [callback = std::move(callback)](const azure::storage::container_result_segment& containerSegment) {
                for (const azure::storage::cloud_blob_container& c : containerSegment.results())
                {
                    callback(c.name());
                }
            },
            std::move(endCallback));
    }
    return {};
}

azure::storage::storage_uri AzureStorageManager::getContainerUriFromName(const QString& containerName) const
{
    using namespace azure::storage;
    if (auto* bc = getBlobClient())
    {
        cloud_blob_container container = bc->get_container_reference(containerName.toStdWString());
        return container.uri();
    }
    else
    {
        return {};
    }
}

azure::storage::cloud_blob_container AzureStorageManager::getContainerFromName(const QString& containerName) const
{
    return getContainerFromUri(getContainerUriFromName(containerName));
}

azure::storage::cloud_blob_container AzureStorageManager::getContainerFromUri(const azure::storage::storage_uri& containerUri) const
{
    if (auto* bc = getBlobClient())
    {
        return azure::storage::cloud_blob_container(containerUri, bc->credentials());
    }
    else
    {
        return {};
    }
}

void AzureStorageManager::createContainer(const QString& containerName, std::function<void(bool)> endCallback)
{
    try
    {
        QPointer<AzureStorageManager> thisPtr = this;
        getContainerFromName(containerName).create_if_not_exists_async().then([endCallback, containerName, thisPtr](const pplx::task<bool>& previousTask) {
            bool succeeded = false;
            try
            {
                // the task is already completed, so this won't block
                bool wasntThere = previousTask.get();
                succeeded = true;
                if (wasntThere)
                {
                    qInfo(LoggingCategory::azureStorage) << tr("New container created:") << containerName;
                }
            }
            catch (std::exception& e)
            {
                qWarning(LoggingCategory::azureStorage) << tr("Failed creating a container:") << containerName << endl
                                                        << e.what();
            }
            QMetaObject::invokeMethod(QApplication::instance(), [endCallback, succeeded, thisPtr]() {
                if (thisPtr)
                {
                    endCallback(succeeded);
                }
            });
        });
    }
    catch (std::exception& e)
    {
        qWarning(LoggingCategory::azureStorage) << tr("Error invoking the container creation for container:") << containerName << endl
                                                << e.what();
        endCallback(false);
    }
}


std::shared_ptr<Cancellable> AzureStorageManager::getAllBlobsAsyncSegmented(const QObject* context, const QString& containerName, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback)
{
    azure::storage::cloud_blob_container container = getContainerFromName(containerName);
    if (container.is_valid())
    {
        return BlobFetcher::startFetching(context, container, std::move(callback));
    }
    else
    {
        return {};
    }
}

std::shared_ptr<Cancellable> AzureStorageManager::getBlobsInDirectoryAsyncSegmented(const QObject* context, const QString& containerName, utility::string_t directory, std::function<void(const azure::storage::list_blob_item_segment& segment)> callback)
{
    azure::storage::cloud_blob_container container = getContainerFromName(containerName);
    if (container.is_valid())
    {
        return BlobFetcher::startFetching(context, container, std::move(directory), std::move(callback));
    }
    else
    {
        return {};
    }
}


void AzureStorageManager::getSubDirectories(const QString& containerName, const QString& directory, const std::function<void(const azure::storage::cloud_blob_directory& dir)>& callback)
{
    using namespace azure::storage;

    cloud_blob_container container = getContainerFromName(containerName);
    // Output URI of each item.
    if (container.is_valid())
    {
        azure::storage::list_blob_item_iterator end_of_results;
        for (auto it = container.list_blobs(directory.toStdWString(), false, blob_listing_details::none, 0, {}, {}); it != end_of_results; ++it)
        {
            if (!it->is_blob())
            {
                callback(it->as_directory());
            }
        }
    }
}

void AzureStorageManager::deleteBlobOrDirectory(const QString& containerName, const utility::string_t& prefix)
{
    if (prefix.empty())
    {
        return;
    }
    using namespace azure::storage;
    cloud_blob_container container = getContainerFromName(containerName);
    if (container.is_valid())
    {
        if (*prefix.rbegin() == '/')
        {
            cloud_blob_directory dir = container.get_directory_reference(prefix);
            for (auto& b : dir.list_blobs(true, {}, 10000, {}, {}))
            {
                if (b.is_blob())
                {
                    b.as_blob().delete_blob();
                }
            }
        }
        else
        {
            auto b = container.get_blob_reference(prefix);
            b.delete_blob();
        }
    }
}


utility::string_t AzureStorageManager::getSasToken(const azure::storage::storage_uri& uri, int accessTypeMask, unsigned int minutes) const
{
    using namespace azure::storage;
    if (auto* bc = getBlobClient())
    {
        azure::storage::cloud_blob_container container = getContainerFromUri(uri);
        if (container.is_valid())
        {
            blob_shared_access_policy policy(
                utility::datetime::utc_now() + utility::datetime::from_minutes(minutes),
                accessTypeMask);

            utility::string_t sas = container.get_shared_access_signature(policy);

            QJsonObject info;
            info[QLatin1String("token")] = QString::fromStdWString(sas);
            info[QLatin1String("uri")] = QString::fromStdWString(uri.path());
            QJsonObject policyInfo;
            policyInfo[QLatin1String("permissions")] = QString::fromStdWString(policy.permissions_to_string());
            policyInfo[QLatin1String("expiration date")] = QString::fromStdWString(policy.expiry().to_string(utility::datetime::ISO_8601));
            info[QLatin1String("policy")] = policyInfo;
            qInfo(LoggingCategory::azureStorage)
                << tr("Shared access signature generated:\n")
                << PrettyJson(info);

            return sas;
        }
    }
    return {};
}

utility::string_t AzureStorageManager::getSasUrl(const azure::storage::storage_uri& uri, int accessTypeMask, unsigned int minutes) const
{
    using namespace azure::storage;
    if (auto* bc = getBlobClient())
    {
        utility::string_t sas = getSasToken(uri, accessTypeMask, minutes);
        utility::string_t uriAndSas = uri.primary_uri().to_string();
        uriAndSas.append(L"?");
        uriAndSas.append(sas);
        return uriAndSas;
    }
    return {};
}

bool AzureStorageManager::writeTextFileSync(const utility::string_t& data, const azure::storage::storage_uri& containerUri, const utility::string_t& fileName)
{
    bool success = false;
    azure::storage::cloud_blob_container container = getContainerFromUri(containerUri);
    if (container.is_valid())
    {
        auto blob = container.get_block_blob_reference(fileName);

        try
        {
            blob.upload_text(data);
            success = true;
        }
        catch (...)
        {
            qWarning(LoggingCategory::azureStorage)
                << tr("Error while writing text file:") << QString::fromStdWString(fileName);
        }
    }
    return success;
}

bool AzureStorageManager::readTextFileSync(const azure::storage::storage_uri& containerUri, const utility::string_t& fileName, utility::string_t& outText)
{
    bool success = false;

    azure::storage::cloud_blob_container container = getContainerFromUri(containerUri);
    if (container.is_valid())
    {
        auto blob = container.get_block_blob_reference(fileName);
        if (blob.is_valid())
        {
            if (blob.exists())
            {
                try
                {
                    outText = blob.download_text();
                    success = true;
                }
                catch (...)
                {
                    qWarning(LoggingCategory::azureStorage)
                        << tr("Error while reading text file:") << QString::fromStdWString(fileName);
                }
            }
        }
    }
    return success;
}

void AzureStorageManager::setStatus(AccountConnectionStatus status)
{
    if (m_status != status)
    {
        m_status = status;
        Q_EMIT onStatusChanged();
    }
}
