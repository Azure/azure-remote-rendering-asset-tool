#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QWidget>

class AzureStorageManager;
class BlobTreeModel;
class ModelsPageModel;
class BlobExplorerView;
class QStackedLayout;
class QComboBox;
class ToolbarButton;

// Main panel used to show the content of a blob storage account (containers and blob hierarchy)
// and to trigger model loading

class ModelsPageView : public QWidget
{
public:
    enum InputMode
    {
        FROM_STORAGE_CONTAINER = 0,
        FROM_SAS_URI = 1
    };
    ModelsPageView(ModelsPageModel* modelsPageModel);

private:
    ModelsPageModel* const m_model;
    BlobExplorerView* const m_explorer;
    QStackedLayout* m_modelSelectionLayout = {};

    QLineEdit* m_modelLoading = {};
    QLabel* m_modelLoadingStatus = {};
    QProgressBar* m_progressBar = {};
    ToolbarButton* m_loadButton = {};

    QComboBox* m_inputMode;
    InputMode m_userPreferredSelection = FROM_STORAGE_CONTAINER;
    bool m_isUiChange = true;

    void setInputMode(InputMode inputMode);
    InputMode getInputMode() const;
    void updateUi();
};
