#include <Model/AzureStorageManager.h>
#include <QPointer>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>

BlobContainerSelectorModel::BlobContainerSelectorModel(AzureStorageManager* storageManager, QString container, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_desiredContainerName(std::move(container))
{

    m_availableContainersModel = new QStandardItemModel(this);
    connect(m_storageManager, &AzureStorageManager::onStatusChanged, this, [this]() {
        if (m_storageManager->getStatus() == AccountConnectionStatus::Connected)
        {
            updateModel();
        }
    });
    updateModel();
}

BlobContainerSelectorModel::~BlobContainerSelectorModel()
{
    if (m_fetcher)
    {
        m_fetcher->cancel();
    }
}

void BlobContainerSelectorModel::updateModel()
{
    if (m_fetcher)
    {
        m_fetcher->cancel();
    }

    m_fetchedModel.clear();
    m_fetcher = m_storageManager->getContainers(
        this, [this](const utility::string_t& name) {
            QString qName = QString::fromStdWString(name);
            m_fetchedModel.push_back(qName); },
        [this]() {
            auto* model = m_availableContainersModel;
            std::vector<QString>& fetchedModel = m_fetchedModel;

            model->clear();
            if (!fetchedModel.empty())
            {
                m_inhibitUpdates = true;
                QString oldContainer(m_desiredContainerName);
                size_t rowInNewModel = 0; //default to the first element

                // the QT model m_availableContainersModel will notify the view (the QComboBox)
                // when appending the first row, after clearing, and the view will select immediately
                // that first item, which will end up starting the fetching of the blobs in the first container,
                // until we select the desired container with setCurrentContainer below.
                // To avoid this, we add a first empty row, so that the selected container will become temporarily
                // empty, which clears the blob lists and doesn't start fetching, until the setCurrentContainer
                model->appendRow(new QStandardItem(""));

                for (size_t i = 0; i < fetchedModel.size(); ++i)
                {
                    const QString& name = fetchedModel[i];
                    model->appendRow(new QStandardItem(name));
                    if (name == oldContainer)
                    {
                        rowInNewModel = i;
                    }
                }
                m_inhibitUpdates = false;

                setCurrentContainer(fetchedModel[rowInNewModel]);
                model->removeRow(0);

                fetchedModel.clear();
            }
        });
}

QAbstractItemModel* BlobContainerSelectorModel::getAvailableContainersModel() const
{
    return m_availableContainersModel;
}

QString BlobContainerSelectorModel::getCurrentContainer() const
{
    return m_currentContainerName;
}

void BlobContainerSelectorModel::setCurrentContainer(QString containerName)
{
    if (!m_inhibitUpdates)
    {
        m_desiredContainerName = containerName;
        if (!m_availableContainersModel->findItems(containerName).isEmpty())
        {
            m_currentContainerName = std::move(containerName);
            Q_EMIT currentContainerChanged();
        }
    }
}

QString BlobContainerSelectorModel::itemString(int index) const
{
    QStandardItem* item = m_availableContainersModel->item(index, 0);
    if (item)
    {
        return item->data(Qt::DisplayRole).toString();
    }
    return {};
}
