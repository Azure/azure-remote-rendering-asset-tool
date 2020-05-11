#pragma once
#include <QObject>

class BlobContainerSelectorModel;
class AzureStorageManager;
class BlobExplorerModel;
class Configuration;
class NotificationButtonModel;
class NotificationButtonModelImplementation;

// model for the blob view used to select the input container/dir/model for conversion. Used by InputSelectionView

class UploadModel : public QObject
{
    Q_OBJECT

public:
    UploadModel(AzureStorageManager* storageManager, Configuration* configuration, QObject* parent = nullptr);
    virtual ~UploadModel();

    NotificationButtonModel* getNotificationButtonModel() const;

    BlobContainerSelectorModel* getContainersModel() const;
    BlobExplorerModel* getExplorerModel() const;

private:
    AzureStorageManager* const m_storageManager;
    Configuration* const m_configuration;
    BlobContainerSelectorModel* const m_containersModel;
    BlobExplorerModel* const m_explorerModel;
    NotificationButtonModelImplementation* const m_buttonModel;

    void updateButton();
    int m_successCount = 0;
    int m_failureCount = 0;
};
