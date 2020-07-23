#pragma once
#include <QWidget>

class InputSelectionModel;
class BlobExplorerView;

// Main panel used to show the content of a blob storage account (containers and blob hierarchy)
// when selecting the input container/folder and model for conversion

class InputSelectionView : public QWidget
{
public:
    InputSelectionView(InputSelectionModel* model);

private:
    InputSelectionModel* const m_model;
    BlobExplorerView* const m_explorer;
    void goBack();
};
