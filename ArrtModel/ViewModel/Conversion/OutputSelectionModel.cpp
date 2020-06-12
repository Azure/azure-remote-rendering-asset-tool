#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/Conversion/OutputSelectionModel.h>

OutputSelectionModel::OutputSelectionModel(AzureStorageManager* storageManager, QString container, QString directory, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_configuration(configuration)
    , m_containersModel(new BlobContainerSelectorModel(storageManager, defaultContainer(std::move(container)), this))
    , m_explorerModel(new BlobExplorerModel(storageManager, m_containersModel->getCurrentContainer(), defaultDirectory(std::move(directory)), this))
{
    auto filterType = m_configuration->getUiState(QLatin1Literal("outputSelection:filterType"), BlobsListModel::FilterType::OnlySubDirectories);
    m_explorerModel->getBlobsModel()->setFilterType(filterType);

    QObject::connect(m_containersModel, &BlobContainerSelectorModel::currentContainerChanged, this, [this]() {
        m_explorerModel->setContainer(m_containersModel->getCurrentContainer());
    });
}

QString OutputSelectionModel::defaultContainer(QString container) const
{
    if (container.isEmpty())
    {
        return m_configuration->getUiState(QLatin1Literal("outputSelection:defaultContainer"), QString());
    }
    else
    {
        return container;
    }
}

QString OutputSelectionModel::defaultDirectory(QString directory) const
{
    if (directory.isEmpty())
    {
        return m_configuration->getUiState(QLatin1Literal("outputSelection:defaultDirectory"), QString());
    }
    else
    {
        return directory;
    }
}


OutputSelectionModel::~OutputSelectionModel()
{
    m_configuration->setUiState(QLatin1Literal("outputSelection:filterType"), m_explorerModel->getBlobsModel()->getFilterType());
    m_configuration->setUiState(QLatin1Literal("outputSelection:defaultContainer"), m_containersModel->getCurrentContainer());
    m_configuration->setUiState(QLatin1Literal("outputSelection:defaultDirectory"), m_explorerModel->getDirectory());
}

BlobContainerSelectorModel* OutputSelectionModel::getContainersModel() const
{
    return m_containersModel;
}

BlobExplorerModel* OutputSelectionModel::getExplorerModel() const
{
    return m_explorerModel;
}

void OutputSelectionModel::submit()
{
    Q_EMIT submitted();
}
