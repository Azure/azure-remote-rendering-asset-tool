#pragma once

#include <QDialog>

#include "ui_BrowseStorageDlg.h"
#include <Storage/UI/StorageBrowserModel.h>

class StorageAccount;

/// The modal dialog presented when picking a file or folder in Azure Storage
///
/// Most functionality is already provided by StorageBrowserWidget.
class BrowseStorageDlg : public QDialog, Ui_BrowseStorageDlg
{
    Q_OBJECT
public:
    BrowseStorageDlg(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, const QString& parentFilter, QWidget* parent = {});
    ~BrowseStorageDlg();

    /// Returns the name of the selected storage container.
    const QString& GetSelectedContainer() const { return m_currentContainer; }

    /// Returns the relative path of the selected item.
    const QString& GetSelectedItem() const { return m_currentItem; }

    /// Returns the storage_uri of the selected item.
    const azure::storage::storage_uri& GetSelectedUri() const { return m_currentUri; }

private Q_SLOTS:
    void on_Buttons_accepted();
    void on_Buttons_rejected();
    void ItemSelected(QString container, QString path, azure::storage::storage_uri uri, bool dblClick);

private:
    QString m_currentContainer;
    QString m_currentItem;
    azure::storage::storage_uri m_currentUri;
    StorageEntry::Type m_showTypes = StorageEntry::Type::Other;
};
