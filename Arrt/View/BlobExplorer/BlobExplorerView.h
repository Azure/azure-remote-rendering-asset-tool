#pragma once
#include <QLabel>
#include <QWidget>

class BlobExplorerModel;
class FlatButton;
class FormControl;

// View used for blob browsing and selection. It has an editable combobox for the directory and a list view for the blobs

class BlobExplorerView : public QWidget
{
    Q_OBJECT
public:
    enum class ExplorerType
    {
        ModelSelector,
        DirectorySelector,
        BlobExplorer
    };

    BlobExplorerView(BlobExplorerModel* model, ExplorerType explorerType, QWidget* parent);

    // opens the uploadDialog to start uploading files
    void selectFilesToUpload();

private:
    BlobExplorerModel* const m_model;
    QLabel* m_uploadStatusLabel = {};
    FlatButton* m_parentDirectoryButton = {};
    FormControl* m_directoryControl = {};

    // upload all of the files (passed as paths) to the blob storage.
    void uploadToBlobStorage(QStringList filesToUpload);
};
