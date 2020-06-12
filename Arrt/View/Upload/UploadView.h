#pragma once
#include <QWidget>

class UploadModel;
class BlobExplorerView;

// Main panel used to upload files to a blob storage

class UploadView : public QWidget
{
public:
    UploadView(UploadModel* model);

private:
    UploadModel* const m_model;
    BlobExplorerView* m_explorer;
};
