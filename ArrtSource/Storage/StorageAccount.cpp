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
    m_fileUploader = std::make_unique<FileUploader>(uploadCallback);
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

    try
    {
        m_storageCredentials = azure::storage::storage_credentials(m_accountName.toStdWString(), m_accountKey.toStdWString());
    }
    catch (std::exception& /*e*/)
    {
        m_blobClient.reset();
        SetConnectionStatus(StorageConnectionStatus::InvalidCredentials);
        return;
    }

    DisconnectFromStorageAccount();

    if (m_accountName.isEmpty() || m_accountKey.isEmpty() || m_endpointUrl.isEmpty())
    {
        return;
    }

    SetConnectionStatus(StorageConnectionStatus::CheckingCredentials);

    // create a helper thread that connects to Azure Storage in the background
    std::thread([this, storageUrl = m_endpointUrl, storageCredentials = m_storageCredentials]()
                { ConnectToStorageAccountThread(storageUrl, storageCredentials); })
        .detach();
}

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

void StorageAccount::DisconnectFromStorageAccount()
{
    SetConnectionStatus(StorageConnectionStatus::NotAuthenticated);
    m_blobClient = nullptr;
}

bool StorageAccount::CreateContainer(const QString& containerName, QString& errorMsg)
{
    errorMsg.clear();

    try
    {
        if (GetContainerFromName(containerName).create_if_not_exists())
        {
            return true;
        }
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
        if (GetContainerFromName(containerName).delete_container_if_exists())
        {
            return true;
        }
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

    cloud_blob_container container = GetContainerFromName(containerName);
    if (!container.is_valid())
    {
        errorMsg = "Container is invalid";
        return false;
    }

    try
    {
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
        auto container = GetContainerFromName(containerName);

        QByteArray data = content.toUtf8();

        Concurrency::streams::rawptr_buffer<uint8_t> mem((uint8_t*)data.data(), data.size(), std::ios_base::in);

        Concurrency::streams::istream is = mem.create_istream();

        azure::storage::cloud_block_blob blob = container.get_block_blob_reference(path.toStdWString());
        blob.upload_from_stream(is);

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
    if (m_blobClient == nullptr)
        return;

    for (auto containersIt = m_blobClient->list_containers(); containersIt != azure::storage::container_result_iterator(); ++containersIt)
    {
        containers.push_back(QString::fromStdWString(containersIt->name()));
    }
}

void StorageAccount::ListBlobDirectory(const QString& containerName, const QString& prefixPath, std::vector<StorageBlobInfo>& directories, std::vector<StorageBlobInfo>& files) const
{
    if (m_blobClient == nullptr)
        return;

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
            info.m_uri = blobIt->as_blob().uri();

            // skip our own empty folder dummy files
            if (!info.m_path.endsWith(".EmptyFolderDummy"))
            {
                files.push_back(info);
            }
        }
        else
        {
            info.m_path = QString::fromStdWString(blobIt->as_directory().prefix());
            info.m_uri = blobIt->as_directory().uri();
            directories.push_back(info);
        }
    }
}

azure::storage::storage_uri StorageAccount::GetContainerUriFromName(const QString& containerName) const
{
    if (m_blobClient == nullptr)
        return {};

    cloud_blob_container container = m_blobClient->get_container_reference(containerName.toStdWString());
    return container.uri();
}

azure::storage::cloud_blob_container StorageAccount::GetContainerFromUri(const azure::storage::storage_uri& containerUri) const
{
    if (m_blobClient == nullptr)
        return {};

    return azure::storage::cloud_blob_container(containerUri, m_blobClient->credentials());
}

azure::storage::cloud_blob_container StorageAccount::GetContainerFromName(const QString& containerName) const
{
    return GetContainerFromUri(GetContainerUriFromName(containerName));
}

QString StorageAccount::CreateSasToken(const azure::storage::storage_uri& uri, int accessTypeMask /*= azure::storage::blob_shared_access_policy::permissions::read*/, unsigned int minutes /*= 60 * 24*/) const
{
    if (m_blobClient == nullptr)
        return {};

    azure::storage::cloud_blob_container container = GetContainerFromUri(uri);

    if (!container.is_valid())
        return {};

    blob_shared_access_policy policy(
        utility::datetime::utc_now() + utility::datetime::from_minutes(minutes),
        accessTypeMask);

    utility::string_t sas = container.get_shared_access_signature(policy);

    QJsonObject info;
    info["token"] = QString::fromStdWString(sas);
    info["uri"] = QString::fromStdWString(uri.path());
    QJsonObject policyInfo;
    policyInfo["permissions"] = QString::fromStdWString(policy.permissions_to_string());
    policyInfo["expiration date"] = QString::fromStdWString(policy.expiry().to_string(utility::datetime::ISO_8601));
    info["policy"] = policyInfo;

    return QString::fromStdWString(sas);
}

QString StorageAccount::CreateSasURL(const azure::storage::storage_uri& uri, int accessTypeMask /*= azure::storage::blob_shared_access_policy::permissions::read*/, unsigned int minutes /*= 60 * 24*/) const
{
    if (m_blobClient == nullptr)
        return {};

    QString sas = CreateSasToken(uri, accessTypeMask, minutes);
    QString uriAndSas = QString::fromStdWString(uri.primary_uri().to_string());
    uriAndSas.append("?");
    uriAndSas.append(sas);
    return uriAndSas;
}
