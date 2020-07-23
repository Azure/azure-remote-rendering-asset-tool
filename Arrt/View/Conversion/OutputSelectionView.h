#pragma once
#include <QWidget>

class OutputSelectionModel;
class BlobExplorerView;

// Main panel used to show the content of a blob storage account (containers and blob hierarchy)
// when selecting the output container/folder for conversion

class OutputSelectionView : public QWidget
{
public:
    OutputSelectionView(OutputSelectionModel* model);

private:
    OutputSelectionModel* const m_model;
    BlobExplorerView* const m_explorer;

    void goBack();
};
