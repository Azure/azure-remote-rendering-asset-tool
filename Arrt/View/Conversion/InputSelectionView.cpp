#include <Model/IncludesAzureStorage.h>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/Conversion/InputSelectionView.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/Conversion/InputSelectionModel.h>
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarButton.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);

InputSelectionView::InputSelectionView(InputSelectionModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);
    l->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the model to convert to .arrAsset format")));

    {
        ToolbarButton* uploadButton = new ToolbarButton(tr("Upload files"), ArrtStyle::s_uploadIcon);
        uploadButton->setToolTip(tr("Upload files"), tr("Select local files and/or directories and upload them to Azure Storage, in the current directory"));
        connect(uploadButton, &ToolbarButton::clicked, this, [this]() { m_explorer->selectFilesToUpload(); });

        ToolbarButton* refreshButton = new ToolbarButton(tr("Refresh"), ArrtStyle::s_refreshIcon);
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));
        connect(refreshButton, &ToolbarButton::clicked, this, [this]() { m_model->refresh(); });

        auto* toolbar = new Toolbar(this);
        toolbar->addButton(uploadButton);
        toolbar->addButton(refreshButton);
        l->addWidget(toolbar);
    }

    {
        auto* cb = new BlobContainerSelector(model->getContainersModel());
        auto* fc = new FormControl(tr("Blob container"), cb);
        fc->setToolTip(tr("Blob container"), tr("Azure Storage blob container where the input 3D model is located"));
        l->addWidget(fc);
    }

    {
        m_explorer = new BlobExplorerView(m_model->getExplorerModel(), BlobExplorerView::ExplorerType::ModelSelector, this);
        auto* fc = new FormControl(tr("Blob list"), m_explorer);
        fc->setToolTip(tr("Blob list"), tr("Azure Storage blob container where the input 3D model is located"));
        l->addWidget(fc);
    }

    {
        ToolbarButton* okButton;
        ToolbarButton* backButton;

        backButton = new ToolbarButton(tr("Back"), ArrtStyle::s_backIcon);
        backButton->setToolTip(tr("Back"), tr("Go back to the conversion page without changing the selection"));
        connect(backButton, &ToolbarButton::clicked, this, [this]() { goBack(); });

        okButton = new ToolbarButton(tr("Select Input"));
        okButton->setToolTip(tr("OK"), tr("Select the input model to be converted"));
        connect(okButton, &ToolbarButton::clicked, this, [this]() { m_model->submit(); });

        auto* toolbar = new Toolbar(this);
        toolbar->addButton(backButton);
        toolbar->addButton(okButton);
        l->addWidget(toolbar);

        auto updateCanSubmit = [this, okButton]() {
            okButton->setEnabled(m_model->canSubmit());
        };
        connect(m_model, &InputSelectionModel::canSubmitChanged, this, updateCanSubmit);
        updateCanSubmit();
    }

    connect(m_model, &InputSelectionModel::submitted, this, [this]() { goBack(); });
}


void InputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
