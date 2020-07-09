#pragma once
#include <QObject>

class BlobContainerSelectorModel;
class AzureStorageManager;
class BlobExplorerModel;
class Configuration;

// model for the blob view used to select the output container/dir for conversion. Used by OutputSelectionView

class OutputSelectionModel : public QObject
{
    Q_OBJECT

public:
    OutputSelectionModel(AzureStorageManager* storageManager, QString container, QString directory, Configuration* configuration, QObject* parent = nullptr);
    virtual ~OutputSelectionModel();

    BlobContainerSelectorModel* getContainersModel() const;

    BlobExplorerModel* getExplorerModel() const;

    // refresh container list and blob list
    void refresh();

    void submit();

Q_SIGNALS:
    void submitted();

private:
    AzureStorageManager* const m_storageManager;
    Configuration* const m_configuration;
    BlobContainerSelectorModel* const m_containersModel;
    BlobExplorerModel* const m_explorerModel;

    QString defaultContainer(QString container) const;
    QString defaultDirectory(QString directory) const;
};
