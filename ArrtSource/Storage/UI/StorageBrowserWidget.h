#pragma once

#pragma once

#include "ui_StorageBrowserWidget.h"
#include <Storage/UI/StorageBrowserModel.h>

class StorageAccount;

/// A QWidget for interacting with the contents of the storage account
///
/// Lists files inside storage containers, allows to add and delete files and folders
/// and keeps track of the selected items.
///
/// Used both in the main UI and in modal file pickers (see BrowseStorageDlg).
class StorageBrowserWidget : public QWidget, Ui_StorageBrowserWidget
{
    Q_OBJECT
public:
    StorageBrowserWidget(QWidget* parent = {});
    ~StorageBrowserWidget();

    /// Refreshes the underlying model
    void RefreshModel();

    void SetStorageAccount(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, const QString& parentFilter);

    /// Returns the currently selected storage container.
    const QString& GetSelectedContainer() const { return m_selectedContainer; }

Q_SIGNALS:
    void ItemSelected(QString container, QString path, azure::storage::storage_uri uri, bool dblClick);

private Q_SLOTS:
    void on_StorageContainer_currentIndexChanged(int index);
    void ItemDoubleClicked(const QModelIndex& index);
    void ItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void on_AddContainerButton_clicked();
    void on_DeleteContainerButton_clicked();
    void on_DeleteItemButton_clicked();
    void on_AddFolderButton_clicked();
    void on_UploadFileButton_clicked();
    void on_UploadFolderButton_clicked();
    void on_RefreshButton_clicked();

private:
    void UpdateUI();
    void EmitItemSelected(bool dblClick);
    void UploadItems(const QStringList& files);

    QString m_selectedContainer;
    QString m_selectedItem;
    StorageAccount* m_storageAccount = nullptr;
    StorageBrowserModel m_storageModel;
    std::vector<QString> m_StorageContainers;
    StorageEntry::Type m_showTypes = StorageEntry::Type::Other;
};
