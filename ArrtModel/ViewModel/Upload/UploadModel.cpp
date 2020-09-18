#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <Model/ConversionManager.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/NotificationButtonModelImplementation.h>
#include <ViewModel/Upload/UploadModel.h>

UploadModel::UploadModel(AzureStorageManager* storageManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_configuration(configuration)
    , m_containersModel(new BlobContainerSelectorModel(storageManager, m_configuration->getUiState(QLatin1Literal("upload:defaultContainer"), QString()), ConversionManager::s_default_input_container, true, this))
    , m_explorerModel(new BlobExplorerModel(storageManager, true, QString("fbx;gltf;glb"), tr("3d Model Files (*.gltf *.glb *.fbx);;All Files (*.*)"), m_containersModel->getCurrentContainer(), m_configuration->getUiState(QLatin1Literal("upload:defaultDirectory"), QString()), this))
    , m_buttonModel(new NotificationButtonModelImplementation(this))
{
    m_explorerModel->getBlobsModel()->setFilterType(BlobsListModel::FilterType::AllFilesAndDirectories);

    QObject::connect(m_containersModel, &BlobContainerSelectorModel::currentContainerChanged, this, [this]() {
        m_explorerModel->setContainer(m_containersModel->getCurrentContainer());
    });

    connect(m_buttonModel, &NotificationButtonModelImplementation::onVisualizedChanged, this, [this]() {
        updateButton();
    });

    connect(m_explorerModel, &BlobExplorerModel::uploadFinished, this, [this](bool succeeded) {
        if (succeeded)
        {
            m_successCount++;
        }
        else
        {
            m_failureCount++;
        }
        updateButton();
    });
    connect(m_explorerModel, &BlobExplorerModel::uploadProgressChanged, this, [this]() {
        updateButton();
    });

    updateButton();
}

UploadModel::~UploadModel()
{
    m_configuration->setUiState(QLatin1Literal("upload:defaultContainer"), m_containersModel->getCurrentContainer());
    m_configuration->setUiState(QLatin1Literal("upload:defaultDirectory"), m_explorerModel->getDirectory());
}

NotificationButtonModel* UploadModel::getNotificationButtonModel() const
{
    return m_buttonModel;
}

BlobContainerSelectorModel* UploadModel::getContainersModel() const
{
    return m_containersModel;
}

BlobExplorerModel* UploadModel::getExplorerModel() const
{
    return m_explorerModel;
}

void UploadModel::updateButton()
{
    if (m_buttonModel->isVisualized())
    {
        m_successCount = 0;
        m_failureCount = 0;
        m_buttonModel->setNotifications({});
    }
    else
    {
        std::vector<NotificationButtonModelImplementation::Notification> notifications;
        if (m_successCount > 0)
        {
            notifications.push_back({NotificationButtonModel::Notification::Type::Completed});
        }
        if (m_failureCount > 0)
        {
            notifications.push_back({NotificationButtonModel::Notification::Type::Failed});
        }
        m_buttonModel->setNotifications(std::move(notifications));
    }
    const int uploadProgress = m_explorerModel->getUploadProgress();
    m_buttonModel->setProgress(uploadProgress != -1, uploadProgress);
    m_buttonModel->setStatusString(m_explorerModel->getUploadStatus());
}

void UploadModel::refresh()
{
    m_containersModel->refresh();
    m_explorerModel->refresh();
}
