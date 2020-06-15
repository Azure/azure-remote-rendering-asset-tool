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
    BlobContainerSelectorModel(AzureStorageManager* storageManager, QString container, QObject* parent);
    ~BlobContainerSelectorModel();

    // the model is a flat list of all of the containers. It can be used in a combobox
    QAbstractItemModel* getAvailableContainersModel() const;

    // return the current container
    QString getCurrentContainer() const;

    // set the current container by name
    void setCurrentContainer(QString containerName);

signals:
    // when getCurrentContainer(index) returns a different value
    void currentContainerChanged();

private:
    QStandardItemModel* m_availableContainersModel;
    AzureStorageManager* const m_storageManager;
    QString m_currentContainerName;
    // this container is the container name that the user wants to select.
    // it is only selected if there is a match with an existing container
    QString m_desiredContainerName;
    std::shared_ptr<Cancellable> m_fetcher;
    std::vector<QString> m_fetchedModel;
    bool m_inhibitUpdates = false;

    QString itemString(int index) const;

    // recompute the list of containers
    void updateModel();
};
