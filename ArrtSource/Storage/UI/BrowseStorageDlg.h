#pragma once

#include <QDialog>

#include "ui_BrowseStorageDlg.h"
#include <Storage/UI/StorageBrowserModel.h>

class StorageAccount;

class BrowseStorageDlg : public QDialog, Ui_BrowseStorageDlg
{
    Q_OBJECT
public:
    BrowseStorageDlg(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, QWidget* parent = {});
    ~BrowseStorageDlg();

    const QString& GetSelectedContainer() const { return m_currentContainer; }
    const QString& GetSelectedItem() const { return m_currentItem; }
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
