#include <QApplication>
#include <QJsonObject>
#include <QPointer>
#include <QSettings>
#include <Storage/StorageAccount.h>
#include <Utils/Logging.h>
#include <cpprest/rawptrstream.h>

using namespace azure::storage;

StorageAccount::StorageAccount(FileUploader::UpdateCallback uploadCallback)
{
    m_fileUploader = std::make_unique<FileUploader>(uploadCallback, this);
}

StorageAccount::~StorageAccount() = default;

bool StorageAccount::LoadSettings()
{
    QSettings s;
    s.beginGroup("StorageAccount");
    m_accountName = s.value("AccountName").toString();
    m_accountKey = s.value("AccountKey").toString();
    m_endpointUrl = s.value("EndpointUrl").toString();
    s.endGroup();

    return !m_accountName.isEmpty() && !m_accountKey.isEmpty() && !m_endpointUrl.isEmpty();
}

void StorageAccount::SaveSettings() const
{
    QSettings s;
    s.beginGroup("StorageAccount");
    s.setValue("AccountName", m_accountName);
    s.setValue("AccountKey", m_accountKey);
    s.setValue("EndpointUrl", m_endpointUrl);
    s.endGroup();
}

void StorageAccount::SetSettings(const QString& accountName, const QString& accountKey, const QString& endpointUrl)
{
    if (m_accountName == accountName && m_accountKey == accountKey && m_endpointUrl == endpointUrl)
        return;

    m_accountName = accountName.trimmed();
    m_accountKey = accountKey.trimmed();
    m_endpointUrl = endpointUrl.trimmed();

    SaveSettings();

    DisconnectFromStorageAccount();
}

void StorageAccount::SetConnectionStatus(StorageConnectionStatus newStatus)
{
    if (m_connectionStatus != newStatus)
    {
        m_connectionStatus = newStatus;
        Q_EMIT ConnectionStatusChanged();
    }
}

void StorageAccount::ConnectToStorageAccount()
{
    if (m_connectionStatus == StorageConnectionStatus::Authenticated)
        return;

    DisconnectFromStorageAccount();

    if (m_accountName.isEmpty() || m_accountKey.isEmpty() || m_endpointUrl.isEmpty())
    {
        return;
    }

    try
    {
#if NEW_STORAGE_SDK
        m_azStorageCredentials = std::make_shared<StorageSharedKeyCredential>(m_accountName.toStdString(), m_accountKey.toStdString());
#else
        m_storageCredentials = azure::storage::storage_credentials(m_accountName.toStdWString(), m_accountKey.toStdWString());
#endif
    }
    catch (std::exception& /*e*/)
    {
        DisconnectFromStorageAccount();
        SetConnectionStatus(StorageConnectionStatus::InvalidCredentials);
        return;
    }

    SetConnectionStatus(StorageConnectionStatus::CheckingCredentials);

#if NEW_STORAGE_SDK
    // create a helper thread that connects to Azure Storage in the background
    std::thread([this, endpointUrl = m_endpointUrl, credentials = m_azStorageCredentials]()
                { ConnectToAzureStorageThread(endpointUrl, credentials); })
        .detach();
#else
    // create a helper thread that connects to Azure Storage in the background
    std::thread([this, storageUrl = m_endpointUrl, storageCredentials = m_storageCredentials]()
                { ConnectToStorageAccountThread(storageUrl, storageCredentials); })
        .detach();
#endif
}

#if NEW_STORAGE_SDK
void StorageAccount::ConnectToAzureStorageThread(const QString& endpointUrl, const std::shared_ptr<StorageSharedKeyCredential>& credentials)
{
    bool storageIsValid = true;
    std::unique_ptr<BlobServiceClient> client;

    try
    {
        client = std::make_unique<BlobServiceClient>(endpointUrl.toStdString(), credentials);
        client->ListBlobContainers();
    }
    catch (...)
    {
        storageIsValid = false;
        client.reset();
    }

    // we need to update our state on the main thread, so queue this call in Qt
    QMetaObject::invokeMethod(QApplication::instance(), [this, storageIsValid, client = std::move(client)]() mutable
                              {
                                  m_azStorageServiceClient = std::move(client);
                                  SetConnectionStatus(storageIsValid ? StorageConnectionStatus::Authenticated : StorageConnectionStatus::InvalidCredentials);
                              });
}
#else
void StorageAccount::ConnectToStorageAccountThread(const QString& storageUrl, const azure::storage::storage_credentials& credentials)
{
    bool storageIsValid = true;
    std::unique_ptr<azure::storage::cloud_blob_client> client;

    try
    {
        client = std::make_unique<cloud_blob_client>(storage_uri(storageUrl.toStdWString()), credentials);
        cloud_queue_client cc(storage_uri(storageUrl.toStdWString()), credentials);
        continuation_token ct;
        cc.list_queues_segmented(ct);
    }
    catch (...)
    {
        storageIsValid = false;
        client.reset();
    }

    // we need to update our state on the main thread, so queue this call in Qt
    QMetaObject::invokeMethod(QApplication::instance(), [this, storageIsValid, client = std::move(client)]() mutable
                              {
                                  m_blobClient = std::move(client);
                                  SetConnectionStatus(storageIsValid ? StorageConnectionStatus::Authenticated : StorageConnectionStatus::InvalidCredentials);
                              });
}
#endif

void StorageAccount::DisconnectFromStorageAccount()
{
    SetConnectionStatus(StorageConnectionStatus::NotAuthenticated);

#if NEW_STORAGE_SDK
    m_azStorageServiceClient = nullptr;
    m_azStorageCredentials = nullptr;
#else
    m_blobClient = nullptr;
#endif

    ClearCache();
}

bool StorageAccount::CreateContainer(const QString& containerName, QString& errorMsg)
{
    errorMsg.clear();

    try
    {
#if NEW_STORAGE_SDK
        auto res = m_azStorageServiceClient->CreateBlobContainer(containerName.toStdString());

        if (res.RawResponse->GetStatusCode() == Http::HttpStatusCode::Created)
        {
            return true;
        }
#else
        if (GetContainerFromName(containerName).create_if_not_exists())
        {
            return true;
        }
#endif
    }
    catch (std::exception& e)
    {
        errorMsg = e.what();
    }

    return false;
}

bool StorageAccount::DeleteContainer(const QString& containerName, QString& errorMsg)
{
    errorMsg.clear();

    try
    {
#if NEW_STORAGE_SDK
        auto res = m_azStorageServiceClient->DeleteBlobContainer(containerName.toStdString());

        if (res.RawResponse->GetStatusCode() == Http::HttpStatusCode::Accepted)
        {
            return true;
        }
#else
        if (GetContainerFromName(containerName).delete_container_if_exists())
        {
            return true;
        }
#endif
    }
    catch (std::exception& e)
    {
        errorMsg = e.what();
    }

    return false;
}

bool StorageAccount::CreateFolder(const QString& containerName, const QString& path, QString& errorMsg)
{
    const QString content = "Dummy file to create an empty folder. Created by Azure Remote Rendering Toolkit (ARRT). This file can safely be deleted.";

    return CreateTextItem(containerName, path + "/.EmptyFolderDummy", content, errorMsg);
}

bool StorageAccount::DeleteItem(const QString& containerName, const QString& path, QString& errorMsg)
{
    errorMsg.clear();

    if (path.isEmpty())
    {
        errorMsg = "Path is empty";
        return false;
    }

    try
    {

#if NEW_STORAGE_SDK

        auto container = GetStorageContainerFromName(containerName);

        if (path.endsWith("/"))
        {
            ListBlobsOptions opt;
            opt.Prefix = path.toStdString();
            auto subs = container.ListBlobs(opt);

            for (const auto& sub : subs.Blobs)
            {
                container.DeleteBlob(sub.Name);
            }
        }
        else
        {
            auto res = container.DeleteBlob(path.toStdString());
            if (res.Value.Deleted == false)
            {
                errorMsg = QString("Deletion failed: ") + res.RawResponse->GetReasonPhrase().c_str();
                return false;
            }
        }
#else
        cloud_blob_container container = GetContainerFromName(containerName);
        if (!container.is_valid())
        {
            errorMsg = "Container is invalid";
            return false;
        }

        if (path.endsWith("/"))
        {
            cloud_blob_directory dir = container.get_directory_reference(path.toStdWString());

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
            auto b = container.get_blob_reference(path.toStdWString());
            b.delete_blob();
        }
#endif

        // TODO: wouldn't need to clear the entire cache
        ClearCache();
        return true;
    }
    catch (std::exception& e)
    {
        errorMsg = e.what();
    }

    return false;
}

bool StorageAccount::CreateTextItem(const QString& containerName, const QString& path, const QString& content, QString& errorMsg)
{
    errorMsg.clear();

    try
    {
        QByteArray data = content.toUtf8();

#if NEW_STORAGE_SDK
        auto container = GetStorageContainerFromName(containerName);

        Azure::Core::IO::MemoryBodyStream stream(reinterpret_cast<const uint8_t*>(data.data()), data.length());

        container.UploadBlob(path.toStdString(), stream);
#else
        auto container = GetContainerFromName(containerName);

        Concurrency::streams::rawptr_buffer<uint8_t> mem((uint8_t*)data.data(), data.size(), std::ios_base::in);
        Concurrency::streams::istream is = mem.create_istream();

        azure::storage::cloud_block_blob blob = container.get_block_blob_reference(path.toStdWString());
        blob.upload_from_stream(is);
#endif

        // TODO: wouldn't need to clear the entire cache
        ClearCache();

        return true;
    }
    catch (std::exception& e)
    {
        errorMsg = e.what();
    }

    return false;
}

void StorageAccount::ListContainers(std::vector<QString>& containers) const
{
#if NEW_STORAGE_SDK
    if (m_azStorageServiceClient == nullptr)
        return;

    auto allContainers = m_azStorageServiceClient->ListBlobContainers();
    for (const auto& cont : allContainers.BlobContainers)
    {
        containers.push_back(cont.Name.c_str());
    }
#else
    if (m_blobClient == nullptr)
        return;

    for (auto containersIt = m_blobClient->list_containers(); containersIt != azure::storage::container_result_iterator(); ++containersIt)
    {
        containers.push_back(QString::fromStdWString(containersIt->name()));
    }
#endif
}

void StorageAccount::ListBlobDirectory(const QString& containerName, const QString& prefixPath, std::vector<StorageBlobInfo>& directories, std::vector<StorageBlobInfo>& files) const
{
    const QString cacheKey = containerName + "##" + prefixPath;

    auto cacheIt = m_cachedBlobs.find(cacheKey);
    if (cacheIt != m_cachedBlobs.end())
    {
        directories = cacheIt->second.m_directories;
        files = cacheIt->second.m_files;
        return;
    }

#if NEW_STORAGE_SDK

    auto container = GetStorageContainerFromName(containerName);

    ListBlobsOptions opt;
    opt.Prefix = prefixPath.toStdString();
    auto res = container.ListBlobsByHierarchy("/", opt);

    for (const auto& blob : res.Blobs)
    {
        StorageBlobInfo info;
        info.m_path = blob.Name.c_str();
        //info.m_uri = needed ??

        // skip our own empty folder dummy files
        if (!info.m_path.endsWith(".EmptyFolderDummy"))
        {
            files.push_back(info);
        }
    }

    for (const auto& blob : res.BlobPrefixes)
    {
        StorageBlobInfo info;
        info.m_path = blob.c_str();
        //info.m_uri = needed ??

        directories.push_back(info);
    }

#else

    auto container = GetContainerFromName(containerName);
    if (!container.is_valid() || container.name() == L"$root")
        return;

    azure::storage::list_blob_item_iterator endIt;
    for (auto blobIt = container.list_blobs(prefixPath.toStdWString(), false, blob_listing_details::none, 1000, {}, {}); blobIt != endIt; ++blobIt)
    {
        StorageBlobInfo info;

        if (blobIt->is_blob())
        {
            info.m_path = QString::fromStdWString(blobIt->as_blob().name());

            // skip our own empty folder dummy files
            if (!info.m_path.endsWith(".EmptyFolderDummy"))
            {
                files.push_back(info);
            }
        }
        else
        {
            info.m_path = QString::fromStdWString(blobIt->as_directory().prefix());
            directories.push_back(info);
        }
    }

#endif

    auto& cached = m_cachedBlobs[cacheKey];
    cached.m_directories = directories;
    cached.m_files = files;
}

void StorageAccount::ClearCache()
{
    m_cachedBlobs.clear();
}

#if NEW_STORAGE_SDK
Azure::Storage::Blobs::BlobContainerClient StorageAccount::GetStorageContainerFromName(const QString& containerName) const
{
    return m_azStorageServiceClient->GetBlobContainerClient(containerName.toStdString());
}
#else
azure::storage::cloud_blob_container StorageAccount::GetContainerFromName(const QString& containerName) const
{
    if (m_blobClient == nullptr)
        return {};

    return m_blobClient->get_container_reference(containerName.toStdWString());
}
#endif

QString StorageAccount::CreateSasToken(const QString& containerName, unsigned int minutes /*= 60 * 24*/) const
{
#if NEW_STORAGE_SDK
    if (m_azStorageServiceClient != nullptr)
    {
        BlobSasBuilder sb;
        sb.Protocol = SasProtocol::HttpsAndHttp;
        sb.BlobContainerName = containerName.toStdString();
        sb.ExpiresOn = Azure::DateTime(std::chrono::system_clock::now() + std::chrono::minutes(minutes));
        sb.Resource = BlobSasResource::BlobContainer;
        sb.SetPermissions(BlobContainerSasPermissions::Read | BlobContainerSasPermissions::Write | BlobContainerSasPermissions::List | BlobContainerSasPermissions::Create);

        QString sas = sb.GenerateSasToken(*m_azStorageCredentials).c_str();

        if (sas.startsWith("?"))
            sas = sas.mid(1);

        return sas;
    }
#else
    if (m_blobClient != nullptr)
    {

        azure::storage::cloud_blob_container container = GetContainerFromName(containerName);

        const int accessTypeMask = azure::storage::blob_shared_access_policy::permissions::read | azure::storage::blob_shared_access_policy::write | azure::storage::blob_shared_access_policy::list | azure::storage::blob_shared_access_policy::create;

        blob_shared_access_policy policy(
            utility::datetime::utc_now() + utility::datetime::from_minutes(minutes),
            accessTypeMask);

        return QString::fromStdWString(container.get_shared_access_signature(policy));
    }
#endif

    return {};
}

QString StorageAccount::CreateSasURL(const QString& containerName, const QString& itemPath, unsigned int minutes /*= 60 * 24*/) const
{
    QString sas = CreateSasToken(containerName, minutes);
    QString uriAndSas = m_endpointUrl;
    uriAndSas.append("/");
    uriAndSas.append(containerName);
    uriAndSas.append("/");
    uriAndSas.append(itemPath);
    uriAndSas.append("?");
    uriAndSas.append(sas);
    return uriAndSas;
}
