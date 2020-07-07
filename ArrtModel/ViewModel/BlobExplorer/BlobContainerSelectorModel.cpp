#include <Model/AzureStorageManager.h>
#include <QPointer>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>

BlobContainerSelectorModel::BlobContainerSelectorModel(AzureStorageManager* storageManager, QString container, QString defaultContainerName, bool canNavigateToNewContainers, QObject* parent)
    : QObject(parent)
    , m_storageManager(storageManager)
    , m_currentContainerName(std::move(container))
    , m_defaultContainerName(defaultContainerName)
    , m_canNavigateToNewContainers(canNavigateToNewContainers)
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
            QString currentContainer(m_currentContainerName);
            // reset the current container, so a call to setCurrentContainer will force the row selection in the UI.
            m_currentContainerName = "";

            model->clear();
            m_inhibitUpdates = true;
            int currentContainerRow = -1;

            // the QT model m_availableContainersModel will notify the view (the QComboBox)
            // when appending the first row, after clearing, and the view will select immediately
            // that first item, which will end up starting the fetching of the blobs in the first container,
            // until we select the desired container with setCurrentContainer below.
            // To avoid this, we add a first empty row, so that the selected container will become temporarily
            // empty, which clears the blob lists and doesn't start fetching, until the setCurrentContainer
            model->appendRow(new QStandardItem(""));

            int row = 0;
            auto addRow = [&model, &currentContainer, &row, &currentContainerRow](const QString& name) {
                model->appendRow(new QStandardItem(name));
                row++;
                if (name == currentContainer)
                {
                    currentContainerRow = row;
                }
            };

            if (!m_defaultContainerName.isEmpty())
            {
                addRow(m_defaultContainerName);
            }
            for (size_t i = 0; i < fetchedModel.size(); ++i)
            {
                if (fetchedModel[i] != m_defaultContainerName)
                {
                    addRow(fetchedModel[i]);
                }
            }
            m_inhibitUpdates = false;

            if (currentContainerRow == -1)
            {
                // if the container was not set, just default to the first row (which is the default container)
                if (currentContainer.isEmpty())
                {
                    currentContainerRow = 0;
                }
                else
                {
                    // default container was set but it's not there: treat it as a new container
                    addRow(currentContainer);
                }
            }
            setCurrentContainer(model->data(model->index(currentContainerRow, 0)).toString());
            model->removeRow(0);

            fetchedModel.clear();
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
    if (!m_inhibitUpdates && containerName != m_currentContainerName)
    {
        m_currentContainerName = containerName;
        Q_EMIT currentContainerChanged();
    }
}

bool BlobContainerSelectorModel::canNavigateToNewContainers() const
{
    return m_canNavigateToNewContainers;
}

void BlobContainerSelectorModel::navigateToNewContainer(QString containerName)
{
    if (m_canNavigateToNewContainers)
    {
        setCurrentContainer(containerName);
        updateModel();
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
