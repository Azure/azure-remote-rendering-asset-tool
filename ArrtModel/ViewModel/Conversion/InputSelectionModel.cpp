#include <Model/AzureStorageManager.h>
#include <Model/Configuration.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/Conversion/InputSelectionModel.h>

InputSelectionModel::InputSelectionModel(AzureStorageManager* storageManager, QString container, QString directory, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_configuration(configuration)
    , m_containersModel(new BlobContainerSelectorModel(storageManager, defaultContainer(std::move(container)), this))
    , m_explorerModel(new BlobExplorerModel(storageManager, true, QString("fbx;gltf;glb"), tr("3d Model Files (*.gltf, *.fbx);;All Files (*.*)"), m_containersModel->getCurrentContainer(), defaultDirectory(std::move(directory)), this))
{
    auto filterType = m_configuration->getUiState(QLatin1Literal("inputSelection:filterType"), BlobsListModel::FilterType::JustAllowedExtensions);
    m_explorerModel->getBlobsModel()->setFilterType(filterType);

    QObject::connect(m_containersModel, &BlobContainerSelectorModel::currentContainerChanged, this, [this]() {
        m_explorerModel->setContainer(m_containersModel->getCurrentContainer());
    });

    QObject::connect(m_explorerModel, &BlobExplorerModel::submitted, this, [this]() { if (canSubmit()) submit(); });

    auto updateCanSubmit = [this]() {
        const bool canSubmit = !m_explorerModel->getSelectedBlobPath().isEmpty();
        if (canSubmit != m_canSubmit)
        {
            m_canSubmit = canSubmit;
            Q_EMIT canSubmitChanged();
        }
    };
    QObject::connect(m_explorerModel, &BlobExplorerModel::selectionChanged, this, updateCanSubmit);
    updateCanSubmit();
}

QString InputSelectionModel::defaultContainer(QString container) const
{
    if (container.isEmpty())
    {
        return m_configuration->getUiState(QLatin1Literal("inputSelection:defaultContainer"), QString());
    }
    else
    {
        return container;
    }
}

QString InputSelectionModel::defaultDirectory(QString directory) const
{
    if (directory.isEmpty())
    {
        return m_configuration->getUiState(QLatin1Literal("inputSelection:defaultDirectory"), QString());
    }
    else
    {
        return directory;
    }
}


InputSelectionModel::~InputSelectionModel()
{
    m_configuration->setUiState(QLatin1Literal("inputSelection:filterType"), m_explorerModel->getBlobsModel()->getFilterType());
    m_configuration->setUiState(QLatin1Literal("inputSelection:defaultContainer"), m_containersModel->getCurrentContainer());
    m_configuration->setUiState(QLatin1Literal("inputSelection:defaultDirectory"), m_explorerModel->getDirectory());
}

BlobContainerSelectorModel* InputSelectionModel::getContainersModel() const
{
    return m_containersModel;
}

BlobExplorerModel* InputSelectionModel::getExplorerModel() const
{
    return m_explorerModel;
}

bool InputSelectionModel::canSubmit() const
{
    return m_canSubmit;
}

void InputSelectionModel::submit()
{
    Q_EMIT submitted();
}
