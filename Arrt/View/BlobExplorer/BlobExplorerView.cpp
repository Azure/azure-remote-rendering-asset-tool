#include <QMessageBox>
#include <QVBoxLayout>
#include <Utils/ScopedBlockers.h>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/BlobExplorer/BlobListView.h>
#include <View/BlobExplorer/DirectorySelector/DirectorySelector.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <Widgets/FileDialogMultiSelection.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FocusableContainer.h>
#include <Widgets/FormControl.h>
#include <Widgets/ReadOnlyText.h>

BlobExplorerView::BlobExplorerView(BlobExplorerModel* model, ExplorerType explorerType, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    setContentsMargins(0, 0, 0, 0);
    auto* l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    auto* topLayout = new QHBoxLayout();

    {
        auto* directorySelector = new DirectorySelector(model->getDirectoryModel(), this);
        directorySelector->setToolTip(ArrtStyle::formatToolTip(tr("Current directory"), tr("When the blob list is set to <i>show all models</i> it shows all of the blobs in the directory or sub-directory, otherwise it show the content of the current directory.")));
        topLayout->addWidget(directorySelector, 1);
    }

    if (explorerType == ExplorerType::DirectorySelector)
    {
        auto* showAllButton = new FlatButton(tr("Show all"));
        showAllButton->setToolTip(tr("Show all blobs"), tr("Toggles between showing just directories and showing all of the blobs"));
        showAllButton->setIcon(ArrtStyle::s_showblobsIcon, true);
        showAllButton->setCheckable(true);

        showAllButton->setChecked(m_model->getBlobsModel()->getFilterType() == BlobsListModel::FilterType::AllFilesAndDirectories);

        connect(showAllButton, &FlatButton::toggled, this,
                [this](bool checked) {
                    m_model->getBlobsModel()->setFilterType(checked ? BlobsListModel::FilterType::AllFilesAndDirectories : BlobsListModel::FilterType::OnlySubDirectories);
                });
        topLayout->addWidget(showAllButton, 0);
    }
    else if (explorerType == ExplorerType::ModelSelector)
    {
        auto* buttonDirectoryMode = new FlatButton(tr("Show all models"));
        buttonDirectoryMode->setToolTip(tr("Show all models"), tr("When active, this shows all of the models from the current directory or its subdirectories"));
        buttonDirectoryMode->setIcon(ArrtStyle::s_directorymodeIcon, true);
        buttonDirectoryMode->setCheckable(true);

        buttonDirectoryMode->setChecked(m_model->getBlobsModel()->getFilterType() == BlobsListModel::FilterType::JustAllowedExtensions);

        connect(buttonDirectoryMode, &FlatButton::toggled, this,
                [this](bool checked) {
                    m_model->getBlobsModel()->setFilterType(checked ? BlobsListModel::FilterType::JustAllowedExtensions : BlobsListModel::FilterType::AllFilesAndDirectories);
                });
        topLayout->addWidget(buttonDirectoryMode, 0);
    }

    l->addLayout(topLayout, 0);

    {
        auto* blobListView = new BlobListView(m_model->getBlobsModel());

        l->addWidget(new FocusableContainer(blobListView, this), 1);

        if (m_model->allowsUpload())
        {
            blobListView->setAcceptFileDrops(true);
            connect(blobListView, &BlobListView::filesDropped, this, [this](const QStringList& filesToUpload) {
                uploadToBlobStorage(filesToUpload);
            });

            auto hLayout = new QHBoxLayout();

            m_uploadStatusLabel = new ReadOnlyText(this);
            m_uploadStatusLabel->setAccessibleName(tr("Upload status"));
            hLayout->addWidget(m_uploadStatusLabel);

            auto updateLabelCallback = [this]() {
                auto newText = m_model->getUploadStatus();
                m_uploadStatusLabel->setVisible(!newText.isEmpty());
                m_uploadStatusLabel->setText(newText);
            };
            QObject::connect(m_model, &BlobExplorerModel::uploadStatusChanged, this, updateLabelCallback);
            updateLabelCallback();

            l->addLayout(hLayout);
        }
    }
}

void BlobExplorerView::uploadToBlobStorage(QStringList filesToUpload)
{
    if (filesToUpload.empty())
    {
        return;
    }
    // the parent of the first file is the root directory
    QFileInfo fileInfo(filesToUpload[0]);
    QDir rootDirectory = fileInfo.dir();

    QStringList filesToUploadFullList = m_model->getFullFileList(filesToUpload);

    if (filesToUploadFullList.count() == 1)
    {
        m_model->uploadBlobs(rootDirectory, filesToUploadFullList);
    }
    else if (filesToUploadFullList.count() > 1)
    {
        QMessageBox mb(this);
        mb.setText(tr("About to upload %0 files. Proceed?").arg(filesToUploadFullList.count()));
        mb.setIcon(QMessageBox::Warning);
        mb.setWindowTitle(tr("Upload to blob storage"));
        mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        mb.setDefaultButton(QMessageBox::Ok);
        if (mb.exec() == QMessageBox::Ok)
        {
            m_model->uploadBlobs(rootDirectory, filesToUploadFullList);
        }
    }
}


void BlobExplorerView::selectFilesToUpload()
{
    FileDialogMultiSelection fd(this);
    fd.setNameFilter(m_model->getUploadFileFilters());
    fd.setViewMode(QFileDialog::Detail);

    if (fd.exec())
    {
        uploadToBlobStorage(fd.selectedFiles());
    }
}
