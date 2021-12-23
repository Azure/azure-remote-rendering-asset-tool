#include <QDirIterator>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <Storage/StorageAccount.h>
#include <Storage/UI/StorageBrowserWidget.h>

StorageBrowserWidget::StorageBrowserWidget(QWidget* parent /*= {}*/)
    : QWidget(parent)
{
    setupUi(this);

    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), FileTree);
    connect(shortcut, SIGNAL(activated()), this, SLOT(on_DeleteItemButton_clicked()));
}

StorageBrowserWidget::~StorageBrowserWidget()
{
}

void StorageBrowserWidget::RefreshModel()
{
    m_storageModel.RefreshModel(false);
}

void StorageBrowserWidget::SetStorageAccount(StorageAccount* account, StorageEntry::Type showTypes, const QString& startContainer, const QString& parentFilter)
{
    if (m_storageAccount == account)
        return;

    const bool parentOnly = !parentFilter.isEmpty();
    const bool allowEdits = (showTypes == StorageEntry::Type::Other) && !parentOnly;

    // disallow editing containers when selecting a file or folder
    AddContainerButton->setVisible(allowEdits);
    DeleteContainerButton->setVisible(allowEdits);
    DeleteItemButton->setVisible(allowEdits);
    UploadFileButton->setVisible(allowEdits);
    UploadFolderButton->setVisible(allowEdits);
    
    StorageContainer->setEnabled(!parentOnly);
    AddFolderButton->setEnabled(!parentOnly);

    m_selectedContainer = startContainer;
    m_storageAccount = account;
    m_storageModel.SetFilter(showTypes, parentFilter);

    UpdateUI();

    connect(m_storageAccount, &StorageAccount::ConnectionStatusChanged, this, &StorageBrowserWidget::UpdateUI);

    FileTree->setModel(&m_storageModel);
    FileTree->expandToDepth(0);

    // these connections have to be set AFTER the tree model has been set for the first time
    connect(FileTree, &QTreeView::doubleClicked, this, &StorageBrowserWidget::ItemDoubleClicked);
    connect(FileTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &StorageBrowserWidget::ItemSelectionChanged);
}

void StorageBrowserWidget::on_StorageContainer_currentIndexChanged(int index)
{
    DeleteContainerButton->setEnabled(index >= 0);

    m_selectedContainer = StorageContainer->currentText();
    if (m_storageModel.SetAccountAndContainer(m_storageAccount, m_selectedContainer))
    {
        EmitItemSelected(false);

        FileTree->expandToDepth(0);
    }
}

void StorageBrowserWidget::ItemDoubleClicked(const QModelIndex&)
{
    EmitItemSelected(true);
}

void StorageBrowserWidget::ItemSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    EmitItemSelected(false);
}

void StorageBrowserWidget::EmitItemSelected(bool dblClick)
{
    m_selectedItem = QString();
    azure::storage::storage_uri uri;

    if (FileTree->selectionModel() && FileTree->selectionModel()->hasSelection())
    {
        m_selectedItem = m_storageModel.data(FileTree->selectionModel()->selectedIndexes()[0], Qt::UserRole).toString();
        uri = m_storageModel.data(FileTree->selectionModel()->selectedIndexes()[0], Qt::UserRole + 1).value<azure::storage::storage_uri>();

        DeleteItemButton->setEnabled(true);
        AddFolderButton->setEnabled(true);
    }
    else
    {
        DeleteItemButton->setEnabled(false);
        AddFolderButton->setEnabled(false);
    }

    Q_EMIT ItemSelected(StorageContainer->currentText(), m_selectedItem, uri, dblClick);
}

void StorageBrowserWidget::on_AddContainerButton_clicked()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "Add Storage Container", "Choose a name for the new Azure Storage container.", QLineEdit::Normal, {}, &ok, {}, Qt::InputMethodHint::ImhUrlCharactersOnly);

    if (!ok || name.isEmpty())
    {
        return;
    }

    QString errorMsg;
    if (m_storageAccount->CreateContainer(name, errorMsg))
    {
        if (StorageContainer->findText(name) == -1)
        {
            StorageContainer->addItem(name);
            StorageContainer->setCurrentIndex(StorageContainer->count() - 1);
        }
    }
    else
    {
        QMessageBox::warning(this, "Storage Container Creation Failed", QString("The storage container '%1' could not be created.\n\nReason: %2").arg(name).arg(errorMsg), QMessageBox::StandardButton::Ok);
    }
}

void StorageBrowserWidget::on_DeleteContainerButton_clicked()
{
    QString name = StorageContainer->currentText();

    if (QMessageBox::question(this, "Delete Storage Container?", QString("Do you want to delete the entire storage container '%1' and all its contents?\nThis operation cannot be undone.").arg(name), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No) != QMessageBox::StandardButton::Yes)
    {
        return;
    }

    QString errorMsg;

    if (m_storageAccount->DeleteContainer(name, errorMsg))
    {
        StorageContainer->removeItem(StorageContainer->findText(name));
    }
    else
    {
        QMessageBox::warning(this, "Storage Container Deletion Failed", QString("The storage container '%1' could not be deleted.\n\nReason: %2").arg(name).arg(errorMsg), QMessageBox::StandardButton::Ok);
    }
}

void StorageBrowserWidget::UpdateUI()
{
    if (m_storageAccount == nullptr || m_storageAccount->GetConnectionStatus() != StorageConnectionStatus::Authenticated)
    {
        setEnabled(false);
        return;
    }

    std::vector<QString> containers;
    m_storageAccount->ListContainers(containers);

    if (m_StorageContainers != containers)
    {
        m_StorageContainers = containers;
        StorageContainer->blockSignals(true);
        StorageContainer->clear();

        int containerIdx = 0;

        for (int i = 0; i < m_StorageContainers.size(); i++)
        {
            if (m_StorageContainers[i] == m_selectedContainer)
            {
                containerIdx = i;
            }

            StorageContainer->addItem(m_StorageContainers[i]);
        }

        StorageContainer->blockSignals(false);

        StorageContainer->setCurrentIndex(containerIdx);
    }

    m_selectedContainer = StorageContainer->currentText();
    if (m_storageModel.SetAccountAndContainer(m_storageAccount, m_selectedContainer))
    {
        EmitItemSelected(false);
        FileTree->expandToDepth(0);
    }

    setEnabled(true);
}

void StorageBrowserWidget::on_DeleteItemButton_clicked()
{
    if (m_selectedItem.isEmpty())
        return;

    if (m_selectedItem.endsWith("/"))
    {
        if (QMessageBox::question(this, "Confirm folder deletion", QString("Do you want to delete this folder and all files in it?\n\n%1/%2\n\nThis operation cannot be undone.").arg(m_selectedContainer).arg(m_selectedItem), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        {
            return;
        }
    }
    else
    {
        if (QMessageBox::question(this, "Confirm file deletion", QString("Do you want to delete this file?\n\n%1/%2\n\nThis operation cannot be undone.").arg(m_selectedContainer).arg(m_selectedItem), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        {
            return;
        }
    }

    QString errorMsg;
    if (!m_storageAccount->DeleteItem(m_selectedContainer, m_selectedItem, errorMsg))
    {
        QMessageBox::warning(this, "Deleting Item Failed", QString("The item '%1/%2' could not be deleted.\n\nReason: %3").arg(m_selectedContainer).arg(m_selectedItem).arg(errorMsg), QMessageBox::StandardButton::Ok);
    }
    else
    {
        m_storageModel.RefreshModel(false);
    }
}

void StorageBrowserWidget::on_AddFolderButton_clicked()
{
    const int lastSlash = m_selectedItem.lastIndexOf("/");
    QString dstFolder = m_selectedItem.left(lastSlash + 1);

    bool ok = false;
    QString name = QInputDialog::getText(this, "Create Folder", "Choose a name for the new folder.", QLineEdit::Normal, {}, &ok, {}, Qt::InputMethodHint::ImhUrlCharactersOnly);

    if (!ok || name.isEmpty())
    {
        return;
    }

    QString fullpath = dstFolder + name;

    if (QMessageBox::question(this, "Confirm Folder Creation", QString("Do you want to create this folder?\n\n%1/%2").arg(StorageContainer->currentText()).arg(fullpath), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
    {
        return;
    }

    QString errorMsg;
    if (!m_storageAccount->CreateFolder(StorageContainer->currentText(), fullpath, errorMsg))
    {
        QMessageBox::warning(this, "Folder Creation Failed", QString("The folder '%1/%2' could not be created.\n\nReason: %3").arg(StorageContainer->currentText()).arg(fullpath).arg(errorMsg), QMessageBox::StandardButton::Ok);
    }

    m_storageModel.RefreshModel(false);
}

static QStringList GetFullFileList(const QStringList& localPaths)
{
    QStringList fullFileList;
    for (const auto& path : localPaths)
    {
        QDir dir(path);
        if (dir.exists())
        {
            QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                it.next();
                fullFileList.append(it.filePath());
            }
        }
        else
        {
            if (QFileInfo::exists(path))
            {
                fullFileList.append(path);
            }
        }
    }
    return fullFileList;
}

void StorageBrowserWidget::UploadItems(const QStringList& files)
{
    if (files.isEmpty())
        return;

    // the parent of the first file is the root directory
    QFileInfo fileInfo(files[0]);
    QDir rootDirectory = fileInfo.dir();

    QStringList toUpload = GetFullFileList(files);

    const int lastSlash = m_selectedItem.lastIndexOf("/");
    QString dstFolder = m_selectedItem.left(lastSlash + 1);

    if (QMessageBox::question(this, "Confirm file upload", QString("%1 files will be uploaded into\n%2/%3\n\nContinue?").arg(toUpload.count()).arg(GetSelectedContainer()).arg(dstFolder), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::Yes) != QMessageBox::StandardButton::Yes)
    {
        return;
    }

    auto container = m_storageAccount->GetContainerFromName(GetSelectedContainer());
    m_storageAccount->GetFileUploader()->UploadFilesAsync(rootDirectory, toUpload, container, dstFolder);
}

void StorageBrowserWidget::on_UploadFileButton_clicked()
{
    QFileDialog fd(this);
    fd.setFileMode(QFileDialog::ExistingFiles);
    fd.setOption(QFileDialog::DontUseNativeDialog, false);
    fd.setViewMode(QFileDialog::Detail);

    if (fd.exec())
    {
        UploadItems(fd.selectedFiles());
    }
}

void StorageBrowserWidget::on_UploadFolderButton_clicked()
{
    QFileDialog fd(this);
    fd.setFileMode(QFileDialog::Directory);
    fd.setOption(QFileDialog::DontUseNativeDialog, false);

    if (fd.exec())
    {
        UploadItems(fd.selectedFiles());
    }
}

void StorageBrowserWidget::on_RefreshButton_clicked()
{
    m_storageModel.RefreshModel(false);
}
