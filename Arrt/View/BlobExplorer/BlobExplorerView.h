#pragma once
#include <QWidget>

class BlobExplorerModel;
class FlatButton;
class ToolbarButton;
class FormControl;
class ReadOnlyText;

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

    BlobExplorerView(BlobExplorerModel* model, ExplorerType explorerType, QWidget* parent = {});

    // create the button to select files to upload
    ToolbarButton* createFilesUploadButton(QWidget* parent = {});

    // create the button to select a directory to upload
    ToolbarButton* createDirectoryUploadButton(QWidget* parent = {});

private:
    BlobExplorerModel* const m_model;
    ReadOnlyText* m_uploadStatusLabel = {};
    FlatButton* m_parentDirectoryButton = {};
    FormControl* m_directoryControl = {};

    // opens the uploadDialog to start uploading files
    void selectFilesToUpload();

    //	opens the uploadDialog to start uploading a directory
    void selectDirectoryToUpload();


    // upload all of the files (passed as paths) to the blob storage.
    void uploadToBlobStorage(QStringList filesToUpload);
};
