#include <QApplication>
#include <QJsonObject>
#include <QPointer>
#include <QSettings>
#include <Storage/StorageAccount.h>
#include <Utils/Logging.h>

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

    SanitizeSettings(m_accountName, m_accountKey, m_endpointUrl);

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

void StorageAccount::SanitizeSettings(QString& accountName, QString& accountKey, QString& endpointUrl)
{
    accountName = accountName.trimmed();
    accountKey = accountKey.trimmed();
    endpointUrl = endpointUrl.trimmed();

    // connecting to a storage account using HTTP (or no protocol) does work
    // however, converting models will fail, unless HTTPS is used
    if (endpointUrl.startsWith("http://", Qt::CaseInsensitive))
    {
        endpointUrl = endpointUrl.mid(7);
    }

    if (endpointUrl.startsWith("https://", Qt::CaseInsensitive))
    {
        endpointUrl = endpointUrl.mid(8);
    }

    if (!endpointUrl.isEmpty() && !endpointUrl.startsWith("https://", Qt::CaseInsensitive))
    {
        endpointUrl = "https://" + endpointUrl;
    }

    while (endpointUrl.endsWith("/"))
        endpointUrl.chop(1);
}

void StorageAccount::SetSettings(QString accountName, QString accountKey, QString endpointUrl)
{
    SanitizeSettings(accountName, accountKey, endpointUrl);

    if (m_accountName == accountName && m_accountKey == accountKey && m_endpointUrl == endpointUrl)
        return;

    m_accountName = accountName;
    m_accountKey = accountKey;
    m_endpointUrl = endpointUrl;

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
        m_azStorageCredentials = std::make_shared<StorageSharedKeyCredential>(m_accountName.toStdString(), m_accountKey.toStdString());
    }
    catch (std::exception& /*e*/)
    {
        DisconnectFromStorageAccount();
        SetConnectionStatus(StorageConnectionStatus::InvalidCredentials);
        return;
    }

    SetConnectionStatus(StorageConnectionStatus::CheckingCredentials);

    // create a helper thread that connects to Azure Storage in the background
    std::thread([this, endpointUrl = m_endpointUrl, credentials = m_azStorageCredentials]()
                { ConnectToAzureStorageThread(endpointUrl, credentials); })
        .detach();
}

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
                                  SetConnectionStatus(storageIsValid ? StorageConnectionStatus::Authenticated : StorageConnectionStatus::InvalidCredentials); });
}

void StorageAccount::DisconnectFromStorageAccount()
{
    SetConnectionStatus(StorageConnectionStatus::NotAuthenticated);

    m_azStorageServiceClient = nullptr;
    m_azStorageCredentials = nullptr;

    ClearCache();
}

bool StorageAccount::CreateContainer(const QString& containerName, QString& errorMsg)
{
    errorMsg.clear();

    try
    {
        auto res = m_azStorageServiceClient->CreateBlobContainer(containerName.toStdString());

        if (res.RawResponse->GetStatusCode() == Http::HttpStatusCode::Created)
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
        auto res = m_azStorageServiceClient->DeleteBlobContainer(containerName.toStdString());

        if (res.RawResponse->GetStatusCode() == Http::HttpStatusCode::Accepted)
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

    try
    {
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

        auto container = GetStorageContainerFromName(containerName);

        Azure::Core::IO::MemoryBodyStream stream(reinterpret_cast<const uint8_t*>(data.data()), data.length());

        container.UploadBlob(path.toStdString(), stream);

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
    if (m_azStorageServiceClient == nullptr)
        return;

    auto allContainers = m_azStorageServiceClient->ListBlobContainers();
    for (const auto& cont : allContainers.BlobContainers)
    {
        containers.push_back(cont.Name.c_str());
    }
}

void StorageAccount::ListBlobDirectory(const QString& containerName, const QString& prefixPath, std::vector<StorageBlobInfo>& directories, std::vector<StorageBlobInfo>& files) const
{
    if (m_azStorageServiceClient == nullptr)
        return;

    const QString cacheKey = containerName + "##" + prefixPath;

    auto cacheIt = m_cachedBlobs.find(cacheKey);
    if (cacheIt != m_cachedBlobs.end())
    {
        directories = cacheIt->second.m_directories;
        files = cacheIt->second.m_files;
        return;
    }

    auto container = GetStorageContainerFromName(containerName);

    ListBlobsOptions opt;
    opt.Prefix = prefixPath.toStdString();
    auto res = container.ListBlobsByHierarchy("/", opt);

    for (const auto& blob : res.Blobs)
    {
        StorageBlobInfo info;
        info.m_path = blob.Name.c_str();
        // info.m_uri = needed ??

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
        // info.m_uri = needed ??

        directories.push_back(info);
    }

    auto& cached = m_cachedBlobs[cacheKey];
    cached.m_directories = directories;
    cached.m_files = files;
}

void StorageAccount::ClearCache()
{
    m_cachedBlobs.clear();
}

Azure::Storage::Blobs::BlobContainerClient StorageAccount::GetStorageContainerFromName(const QString& containerName) const
{
    return m_azStorageServiceClient->GetBlobContainerClient(containerName.toStdString());
}

QString StorageAccount::CreateSasToken(const QString& containerName, unsigned int minutes /*= 60 * 24*/) const
{
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

    return {};
}

QString StorageAccount::CreateSasURL(const QString& containerName, const QString& itemPath, unsigned int minutes /*= 60 * 24*/) const
{
    QUrl url;
    url.setPath(itemPath);
    QString encodedPath = url.path(QUrl::ComponentFormattingOption::FullyEncoded);

    QString sas = CreateSasToken(containerName, minutes);
    QString uriAndSas = m_endpointUrl;
    uriAndSas.append("/");
    uriAndSas.append(containerName);
    uriAndSas.append("/");
    uriAndSas.append(encodedPath);
    uriAndSas.append("?");
    uriAndSas.append(sas);

    return uriAndSas;
}

void StorageAccountMock::ConnectToStorageAccount()
{
    m_connectionStatus = StorageConnectionStatus::Authenticated;
}

bool StorageAccountMock::CreateContainer(const QString&, QString&)
{
    return true;
}

bool StorageAccountMock::DeleteContainer(const QString&, QString&)
{
    return true;
}

bool StorageAccountMock::CreateTextItem(const QString&, const QString&, const QString&, QString&)
{
    return true;
}