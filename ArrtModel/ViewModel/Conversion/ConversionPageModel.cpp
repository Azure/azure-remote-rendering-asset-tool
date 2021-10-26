#include <Model/AzureStorageManager.h>
#include <QItemSelectionModel>
#include <ViewModel/Conversion/ConversionModel.h>
#include <ViewModel/Conversion/ConversionPageModel.h>
#include <ViewModel/Conversion/CurrentConversionsModel.h>
#include <ViewModel/NotificationButtonModelImplementation.h>

ConversionPageModel::ConversionPageModel(ConversionManager* conversionManager, AzureStorageManager* storageManager, Configuration* configuration, QObject* parent)
    : QObject(parent)
    , m_conversionManager(conversionManager)
    , m_selectionModel(new QItemSelectionModel(nullptr, this))
    , m_currentConversionModels(new CurrentConversionsModel(m_selectionModel, conversionManager, this))
    , m_conversionModel(new ConversionModel(conversionManager, storageManager, configuration, this))
    , m_buttonModel(new NotificationButtonModelImplementation(this))
{
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
                m_conversionModel->setConversion(getCurrentConversionId());
            });

    connect(m_conversionModel, &ConversionModel::changed, this,
            [this]() {
                triggerChanged();
            });

    connect(conversionManager, &ConversionManager::onEnabledChanged, this, &ConversionPageModel::onEnabledChanged);

    connect(m_buttonModel, &NotificationButtonModelImplementation::onVisualizedChanged, this, [this]() {
        updateButton();
    });
    connect(conversionManager, &ConversionManager::runningConversionCountChanged, this, [this]() {
        updateButton();
    });
    connect(conversionManager, &ConversionManager::conversionCompleted, this, [this](ConversionManager::ConversionId, bool succeeded) {
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

    updateButton();
}

NotificationButtonModel* ConversionPageModel::getNotificationButtonModel() const
{
    return m_buttonModel;
}

QItemSelectionModel* ConversionPageModel::getSelectionModel() const
{
    return m_selectionModel;
}

void ConversionPageModel::addNewConversion()
{
    m_conversionManager->addNewConversion();
}

bool ConversionPageModel::canRemoveCurrentConversion() const
{
    if (const Conversion* conversion = m_conversionManager->getConversion(getCurrentConversionId()))
    {
        return !conversion->isActive();
    }
    else
    {
        return false;
    }
}

void ConversionPageModel::removeCurrentConversion()
{
    QModelIndexList indexList = m_selectionModel->selection().indexes();
    if (!indexList.isEmpty())
    {
        int selectedRow = indexList[0].row();
        m_conversionManager->removeConversion(getCurrentConversionId());
        if (m_currentConversionModels->rowCount() >= selectedRow)
        {
            selectedRow = m_currentConversionModels->rowCount() - 1;
        }
        if (selectedRow >= 0)
        {
            m_selectionModel->select(m_currentConversionModels->index(selectedRow, 0), QItemSelectionModel::ClearAndSelect);
        }
    }
}


CurrentConversionsModel* ConversionPageModel::getCurrentConversionsModel() const
{
    return m_currentConversionModels;
}

ConversionModel* ConversionPageModel::getSelectedConversionModel() const
{
    return m_conversionModel;
}

int ConversionPageModel::getCurrentConversionId() const
{
    QModelIndexList indexList = m_selectionModel->selection().indexes();
    if (indexList.isEmpty())
    {
        return ConversionModel::s_conversionNotSelected;
    }
    else
    {
        return indexList[0].data(CurrentConversionsModel::ID).toInt();
    }
}

bool ConversionPageModel::isEnabled() const
{
    return m_conversionManager->isEnabled();
}

void ConversionPageModel::triggerChanged()
{
    const bool canRemove = canRemoveCurrentConversion();
    if (canRemove != m_couldRemoveCurrentConversion)
    {
        m_couldRemoveCurrentConversion = canRemove;
        Q_EMIT changed();
    }
}


void ConversionPageModel::updateButton()
{
    const int runningCount = m_conversionManager->runningConversionCount();
    m_buttonModel->setProgress(runningCount > 0);
    std::vector<NotificationButtonModelImplementation::Notification> notifications;
    if (runningCount > 0)
    {
        notifications.push_back({NotificationButtonModel::Notification::Type::Running, runningCount});
    }

    if (m_buttonModel->isVisualized())
    {
        m_successCount = 0;
        m_failureCount = 0;
    }
    else
    {
        if (m_successCount > 0)
        {
            notifications.push_back({NotificationButtonModel::Notification::Type::Completed, m_successCount});
        }
        if (m_failureCount > 0)
        {
            notifications.push_back({NotificationButtonModel::Notification::Type::Failed, m_failureCount});
        }
    }
    m_buttonModel->setNotifications(std::move(notifications));
}
