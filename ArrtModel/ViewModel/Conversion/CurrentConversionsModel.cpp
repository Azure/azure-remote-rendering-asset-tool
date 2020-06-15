#include <QItemSelectionModel>
#include <ViewModel/Conversion/CurrentConversionsModel.h>

Q_DECLARE_METATYPE(Conversion::Status);
Q_DECLARE_METATYPE(QElapsedTimer);

CurrentConversionsModel::CurrentConversionsModel(QItemSelectionModel* selectionModel, ConversionManager* conversionManager, QObject* parent)
    : QStandardItemModel(parent)
    , m_conversionManager(conversionManager)
    , m_itemSelectionModel(selectionModel)
{
    m_itemSelectionModel->setModel(this);
    for (int i = 0; i < m_conversionManager->getConversionsCount(); ++i)
    {
        ConversionManager::ConversionId id = m_conversionManager->getConversionId(i);
        addConversion(id);
    }

    QObject::connect(m_conversionManager, &ConversionManager::conversionAdded, this, [this](ConversionManager::ConversionId id) {
        addConversion(id);
    });
    QObject::connect(m_conversionManager, &ConversionManager::conversionRemoved, this, [this](ConversionManager::ConversionId id) {
        int row = getConversionRowFromId(id);
        if (row >= 0)
        {
            removeRow(row);
        }
    });
    QObject::connect(m_conversionManager, &ConversionManager::conversionUpdated, this, [this](ConversionManager::ConversionId id) {
        int row = getConversionRowFromId(id);
        if (row >= 0)
        {
            syncConversion(item(row));
        }
    });
}

void CurrentConversionsModel::addConversion(ConversionManager::ConversionId id)
{
    auto* standardItem = new QStandardItem();
    standardItem->setData(id, ID);
    standardItem->setEditable(false);
    syncConversion(standardItem);

    // the conversions are in inverse order
    insertRow(0, standardItem);
    m_itemSelectionModel->select(this->index(0, 0), QItemSelectionModel::ClearAndSelect);
}

void CurrentConversionsModel::syncConversion(QStandardItem* item)
{
    const int id = item->data(ID).toInt();
    if (const Conversion* conversion = m_conversionManager->getConversion(id))
    {
        QString name = conversion->m_name;
        if (name.isEmpty())
        {
            name = conversion->getDefaultName(id);
        }

        const int elapsedSeconds = conversion->m_startConversionTime.secsTo(conversion->m_endConversionTime);

        item->setText(tr("%1 (%2 seconds) [%3]").arg(name).arg(elapsedSeconds).arg(getStringFromStatus(conversion->m_status)));
        item->setData(name, NAME);
        item->setData(QVariant::fromValue(conversion->m_status), STATUS);
        item->setData(QVariant::fromValue(elapsedSeconds), INGESTION_TIME);
        item->setToolTip(item->text());
    }
}

QString CurrentConversionsModel::getStringFromStatus(Conversion::Status status)
{
    switch (status)
    {
        case Conversion::UNKNOWN:
            return tr("Unknown");
        case Conversion::NOT_STARTED:
            return tr("Not started");
        case Conversion::FAILED_TO_START:
            return tr("Failed to start");
        case Conversion::START_REQUESTED:
            return tr("Start requested");
        case Conversion::STARTING:
            return tr("Starting");
        case Conversion::SYNCHRONIZING:
            return tr("Synchronizing");
        case Conversion::CONVERTING:
            return tr("Converting");
        case Conversion::COMPLETED:
            return tr("Completed");
        case Conversion::CANCELED:
            return tr("Canceled");
        case Conversion::SYNCHRONIZATION_FAILED:
            return tr("Synchronization failed");
        case Conversion::CONVERSION_FAILED:
            return tr("Conversion failed");
    }
    return {};
}

ConversionManager::ConversionId CurrentConversionsModel::getConversionIdFromRow(int row) const
{
    return data(index(row, 0), ID).toUInt();
}

int CurrentConversionsModel::getConversionRowFromId(ConversionManager::ConversionId id) const
{
    for (int row = 0; row < rowCount(); ++row)
    {
        if (getConversionIdFromRow(row) == id)
        {
            return row;
        }
    }
    return -1;
}
