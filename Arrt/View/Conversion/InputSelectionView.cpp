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
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);

InputSelectionView::InputSelectionView(InputSelectionModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);
    FlatButton* uploadButton;
    FlatButton* refreshButton;
    FlatButton* okButton;
    FlatButton* cancelButton;
    {
        uploadButton = new FlatButton(tr("Upload files"));
        uploadButton->setToolTip(tr("Upload files"), tr("Select local files and/or directories and upload them to Azure Storage, in the current directory"));
        uploadButton->setIcon(ArrtStyle::s_uploadIcon, true);

        refreshButton = new FlatButton(tr("Refresh"));
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));
        refreshButton->setIcon(ArrtStyle::s_refreshIcon, true);

        okButton = new FlatButton(tr("OK"));
        okButton->setToolTip(tr("OK"), tr("Select the input model to be converted"));

        cancelButton = new FlatButton(tr("Cancel"));
        cancelButton->setToolTip(tr("Cancel"), tr("Go back to the conversion page without changing the selection"));

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the model to convert to .arrAsset format")), 1);
        buttonLayout->addWidget(uploadButton);
        buttonLayout->addWidget(refreshButton);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        l->addLayout(buttonLayout, 0);

        auto updateCanSubmit = [this, okButton]() {
            okButton->setEnabled(m_model->canSubmit());
        };
        connect(m_model, &InputSelectionModel::canSubmitChanged, this, updateCanSubmit);
        updateCanSubmit();
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

    connect(m_model, &InputSelectionModel::submitted, this, [this]() { goBack(); });
    connect(uploadButton, &FlatButton::clicked, this, [this]() { m_explorer->selectFilesToUpload(); });
    connect(refreshButton, &FlatButton::clicked, this, [this]() { m_model->refresh(); });
    connect(okButton, &FlatButton::clicked, this, [this]() { m_model->submit(); });
    connect(cancelButton, &FlatButton::clicked, this, [this]() { goBack(); });
}


void InputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
