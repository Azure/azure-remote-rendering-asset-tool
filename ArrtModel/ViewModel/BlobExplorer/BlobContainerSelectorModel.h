#pragma once

#include <QObject>
#include <QStandardItemModel>

class QAbstractItemModel;
class QStrandardItemModel;
class AzureStorageManager;
class Cancellable;

// model used to represent an azure container selection. It will handle the qt itemModel to show the available containers
// (it can be used in a listView or in a comboBox) and it will handle the "current container"

class BlobContainerSelectorModel : public QObject
{
    Q_OBJECT

public:
    BlobContainerSelectorModel(AzureStorageManager* storageManager, QString container, QString defaultContainerName, bool canNavigateToNewContainers, QObject* parent);
    ~BlobContainerSelectorModel();

    // the model is a flat list of all of the containers. It can be used in a combobox
    QAbstractItemModel* getAvailableContainersModel() const;

    // return the current container
    QString getCurrentContainer() const;

    // set the current container by name
    void setCurrentContainer(QString containerName);

    // true when the user can call navigateToNewContainer
    bool canNavigateToNewContainers() const;

    // navigate to a new container, or to an existing one, if it's found
    void navigateToNewContainer(QString containerName);

    // refresh the list of containers
    void refresh();

signals:
    // when getCurrentContainer(index) returns a different value
    void currentContainerChanged();

private:
    QStandardItemModel* m_availableContainersModel;
    AzureStorageManager* const m_storageManager;
    QString m_currentContainerName;

    // default container displayed in the list at the beginning, also when it's not actually there.
    QString m_defaultContainerName;

    std::shared_ptr<Cancellable> m_fetcher;
    std::vector<QString> m_fetchedModel;
    bool m_inhibitUpdates = false;

    // true when the user can call navigateToNewContainer
    const bool m_canNavigateToNewContainers;

    QString itemString(int index) const;
};
