#include <Model/ArrSessionManager.h>
#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/Log/LogHelpers.h>
#include <QDebug>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/ModelsPage/ModelsPageModel.h>
#include <utility>

namespace
{
    static const int s_sasReadDurationInMinutes = 30;
}

ModelsPageModel::ModelsPageModel(AzureStorageManager* storageManager, ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_configuration(configuration)
    , m_containersModel(new BlobContainerSelectorModel(storageManager, m_configuration->getUiState("modelsPage:defaultContainer", QString()), "ARRT_Models", this))
    , m_explorerModel(new BlobExplorerModel(storageManager, true, QString("arrAsset"), tr("Converted 3d Model Files (*.arrAsset);;All Files (*.*)"), m_containersModel->getCurrentContainer(), m_configuration->getUiState("modelsPage:defaultDirectory", QString()), this))
    , m_sessionManager(sessionManager)
{
    auto filterType = m_configuration->getUiState(QLatin1Literal("modelsPage:filterType"), BlobsListModel::FilterType::JustAllowedExtensions);
    m_explorerModel->getBlobsModel()->setFilterType(filterType);

    QObject::connect(m_containersModel, &BlobContainerSelectorModel::currentContainerChanged, this, [this]() {
        m_explorerModel->setContainer(m_containersModel->getCurrentContainer());
    });

    QObject::connect(m_explorerModel, &BlobExplorerModel::submitted, this, [this]() {
        load(FromExplorer);
    });

    QObject::connect(m_explorerModel, &BlobExplorerModel::selectionChanged, this, [this]() {
        Q_EMIT canLoadChanged(FromExplorer);
    });

    QObject::connect(m_sessionManager, &ArrSessionManager::rootIdChanged, this, [this]() {
        setCurrentLoadingModel({}, true);
        Q_EMIT modelLoaded(m_sessionManager->loadedModel() != nullptr);
    });

    QObject::connect(m_storageManager, &AzureStorageManager::onStatusChanged, this, [this]() {
        const bool blobStorageAvailable = m_storageManager->getStatus() == AccountConnectionStatus::Connected;
        if (m_blobStorageAvailable != blobStorageAvailable)
        {
            m_blobStorageAvailable = blobStorageAvailable;
            Q_EMIT onBlobStorageAvailabilityChanged();
        }
    });

    auto checkEnabledChanged = [this]() {
        const bool enabled = isEnabled();
        if (enabled != m_enabled)
        {
            m_enabled = enabled;
            Q_EMIT onEnabledChanged();
        }
    };

    QObject::connect(m_sessionManager, &ArrSessionManager::changed, this, checkEnabledChanged);
    QObject::connect(m_sessionManager, &ArrSessionManager::onEnabledChanged, this, checkEnabledChanged);
}

ModelsPageModel::~ModelsPageModel()
{
    m_configuration->setUiState(QLatin1Literal("modelsPage:filterType"), m_explorerModel->getBlobsModel()->getFilterType());
    m_configuration->setUiState(QLatin1Literal("modelsPage:defaultContainer"), m_containersModel->getCurrentContainer());
    m_configuration->setUiState(QLatin1Literal("modelsPage:defaultDirectory"), m_explorerModel->getDirectory());
}

BlobContainerSelectorModel* ModelsPageModel::getContainersModel() const
{
    return m_containersModel;
}

BlobExplorerModel* ModelsPageModel::getExplorerModel() const
{
    return m_explorerModel;
}

void ModelsPageModel::setDirectory(const QString& directory)
{
    m_explorerModel->setDirectory(directory);
}

QString ModelsPageModel::getDirectory() const
{
    return m_explorerModel->getDirectory();
}

void ModelsPageModel::setCurrentLoadingModel(const QString& model, bool fromExplorer)
{
    if (m_loadingFromExplorer)
    {
        if (auto* bm = m_explorerModel->getBlobsModel())
        {
            // in case of disconnection or model being explicitly unloaded, reset the currently loaded model in m_blobsListModel
            bm->setLoadingModelBlobStatus(BlobsListModel::LoadingStatus::NOT_LOADED);
        }
    }

    m_loadingStatus = BlobsListModel::LoadingStatus::NOT_LOADED;
    m_loadingProgress = 0;

    m_currentModel = model;
    if (!m_currentModel.isEmpty())
    {
        m_loadingFromExplorer = fromExplorer;

        if (m_loadingFromExplorer)
        {
            if (auto* bm = m_explorerModel->getBlobsModel())
            {
                bm->setLoadingModelBlob(model);
                bm->setLoadingModelBlobStatus(BlobsListModel::LoadingStatus::LOADING);
            }
        }
        m_loadingStatus = BlobsListModel::LoadingStatus::LOADING;
    }

    Q_EMIT loadingStatusChanged();
}

void ModelsPageModel::setCurrentLoadingStatus(BlobsListModel::LoadingStatus status)
{
    m_loadingStatus = status;
    if (m_loadingFromExplorer)
    {
        if (auto* bm = m_explorerModel->getBlobsModel())
        {
            bm->setLoadingModelBlobStatus(m_loadingStatus);
        }
    }
    Q_EMIT loadingStatusChanged();
}

void ModelsPageModel::setCurrentLoadingProgress(float progress)
{
    m_loadingProgress = progress;
    if (m_loadingFromExplorer)
    {
        if (auto* bm = m_explorerModel->getBlobsModel())
        {
            bm->setLoadingModelProgress(progress);
        }
    }
    Q_EMIT loadingStatusChanged();
}

bool ModelsPageModel::loadModelImpl(const QString& path, const QString& sasUri, bool fromExplorer)
{
    using namespace azure::storage;

    if (!sasUri.isEmpty())
    {
        setCurrentLoadingModel(path, fromExplorer);

        RR::LoadResult loadResult = [this](RR::Result result, const RR::ApiHandle<RR::Entity>& /*root*/) {
            qDebug() << tr("LOADED RESULT ") << result;
            const bool loaded = result == RR::Result::Success;
            setCurrentLoadingStatus(loaded ? BlobsListModel::LoadingStatus::LOADED : BlobsListModel::LoadingStatus::FAILED);
        };

        RR::LoadProgress loadProgress = [this](float progress) {
            setCurrentLoadingProgress(progress);
        };

        if (m_sessionManager->loadModelAsync(path, sasUri.toUtf8().data(), loadResult, loadProgress) == RR::Result::Success)
        {
            qDebug() << tr("Success");
        }
        else
        {
            return false;
        }
    }

    return true;
}


QString ModelsPageModel::getCurrentLoadingModel() const
{
    return m_currentModel;
}

BlobsListModel::LoadingStatus ModelsPageModel::getCurrentLoadingStatus() const
{
    return m_loadingStatus;
}

float ModelsPageModel::getCurrentLoadingProgress() const
{
    return m_loadingProgress;
}

QString ModelsPageModel::getModelSasUri() const
{
    return m_modelSasUri;
}

void ModelsPageModel::setModelSasUri(QString sasUri)
{
    const bool wasValid = canLoad(FromSasUri);
    m_modelSasUri = std::move(sasUri);
    const bool isValid = canLoad(FromSasUri);
    if (wasValid != isValid)
    {
        Q_EMIT canLoadChanged(FromSasUri);
    }
}

bool ModelsPageModel::isEnabled() const
{
    return m_sessionManager->isEnabled() && m_sessionManager->getSessionStatus().m_status == SessionStatus::Status::ReadyConnected;
}

bool ModelsPageModel::isBlobStorageAvailable() const
{
    return m_blobStorageAvailable;
}

bool ModelsPageModel::canLoad(LoadingMode mode) const
{
    switch (mode)
    {
        case FromSasUri:
        {
            return !m_modelSasUri.isEmpty();
        }
        case FromExplorer:
        {
            return !m_explorerModel->getSelectedBlobPath().isEmpty();
        }
        default:
        {
            return false;
        }
    }
}

void ModelsPageModel::load(LoadingMode mode)
{
    switch (mode)
    {
        case FromSasUri:
        {
            QString path = m_modelSasUri.left(m_modelSasUri.lastIndexOf(QChar('?'))).mid(m_modelSasUri.lastIndexOf(QChar('/')) + 1);
            loadModelImpl(path, m_modelSasUri, false);
            break;
        }
        case FromExplorer:
        {
            azure::storage::storage_uri uri(m_explorerModel->getSelectedBlobUrl().toStdWString());
            QString path = m_explorerModel->getSelectedBlobPath();

            if (!uri.primary_uri().is_empty())
            {
                utility::string_t blobUri = m_storageManager->getSasUrl(uri, azure::storage::blob_shared_access_policy::permissions::read, s_sasReadDurationInMinutes);
                loadModelImpl(path, QString::fromStdWString(blobUri), true);
            }
            break;
        }
    }
}
