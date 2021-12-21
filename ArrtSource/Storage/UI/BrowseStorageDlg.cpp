#include <QPushButton>
#include <Storage/UI/BrowseStorageDlg.h>

BrowseStorageDlg::BrowseStorageDlg(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, QWidget* parent /*= {}*/)
    : QDialog(parent)
    , m_showTypes(showTypes)
{
    setupUi(this);

    connect(StorageBrowser, &StorageBrowserWidget::itemSelected, this, &BrowseStorageDlg::ItemSelected);

    StorageBrowser->SetStorageAccount(account, showTypes, startContainer);
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

void BrowseStorageDlg::ItemSelected(QString container, QString path, azure::storage::storage_uri uri, bool dblClick)
{
    m_currentContainer = container;
    m_currentItem = path;
    m_currentUri = uri;

    QPushButton* openButton = Buttons->button(QDialogButtonBox::StandardButton::Open);

    if (m_currentItem.isEmpty())
    {
        openButton->setEnabled(false);
        return;
    }

    if (m_showTypes == StorageEntry::Type::Folder)
    {
        // if this is a browser for a folder, any selection is accepted
        openButton->setEnabled(true);
        return;
    }
    else
    {
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
