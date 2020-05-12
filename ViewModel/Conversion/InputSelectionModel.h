#pragma once
#include <QObject>

class BlobContainerSelectorModel;
class AzureStorageManager;
class BlobExplorerModel;
class Configuration;

// model for the blob view used to select the input container/dir/model for conversion. Used by InputSelectionView

class InputSelectionModel : public QObject
{
    Q_OBJECT

public:
    InputSelectionModel(AzureStorageManager* storageManager, QString container, QString directory, Configuration* configuration, QObject* parent = nullptr);
    virtual ~InputSelectionModel();

    BlobContainerSelectorModel* getContainersModel() const;

    BlobExplorerModel* getExplorerModel() const;

    bool canSubmit() const;
    void submit();

Q_SIGNALS:
    void submitted();
    void canSubmitChanged();

private:
    AzureStorageManager* const m_storageManager;
    Configuration* const m_configuration;
    BlobContainerSelectorModel* const m_containersModel;
    BlobExplorerModel* const m_explorerModel;
    bool m_canSubmit = false;
    QString defaultContainer(QString container) const;
    QString defaultDirectory(QString directory) const;
};
