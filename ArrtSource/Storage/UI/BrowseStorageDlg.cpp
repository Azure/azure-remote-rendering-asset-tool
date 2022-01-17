#include <QMessageBox>
#include <QPushButton>
#include <Storage/StorageAccount.h>
#include <Storage/UI/BrowseStorageDlg.h>

BrowseStorageDlg::BrowseStorageDlg(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, const QString& parentFilter, QWidget* parent /*= {}*/)
    : QDialog(parent)
    , m_showTypes(showTypes)
{
    setupUi(this);

    connect(StorageBrowser, &StorageBrowserWidget::ItemSelected, this, &BrowseStorageDlg::ItemSelected);

    StorageBrowser->SetStorageAccount(account, showTypes, startContainer, parentFilter);

    if (account->GetConnectionStatus() != StorageConnectionStatus::Authenticated)
    {
        QMessageBox::warning(this, "Storage account not set up", "You need to be connected to an Azure Storage account.", QMessageBox::Ok, QMessageBox::Ok);
    }
}

BrowseStorageDlg::~BrowseStorageDlg() = default;

void BrowseStorageDlg::on_Buttons_accepted()
{
    accept();
}

void BrowseStorageDlg::on_Buttons_rejected()
{
    reject();
}

void BrowseStorageDlg::ItemSelected(QString container, QString path, bool dblClick)
{
    m_currentContainer = container;
    m_currentItem = path;

    QPushButton* openButton = Buttons->button(QDialogButtonBox::StandardButton::Open);

    if (m_showTypes == StorageEntry::Type::Folder)
    {
        // if this is a browser for a folder, any selection is accepted
        openButton->setEnabled(true);
        return;
    }
    else
    {
        if (m_currentItem.isEmpty())
        {
            openButton->setEnabled(false);
            return;
        }

        // in all other cases, only file selections are allowed
        openButton->setEnabled(!m_currentItem.endsWith("/"));

        if (openButton->isEnabled() && dblClick)
        {
            // double clicks only open files, not folders
            accept();
            return;
        }
    }
}
