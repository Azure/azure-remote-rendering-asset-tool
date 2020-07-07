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

    l->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the model to convert to .arrAsset format")));

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

    {
        FlatButton* okButton = new FlatButton(tr("OK"));
        FlatButton* cancelButton = new FlatButton(tr("Cancel"));

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        QObject::connect(okButton, &FlatButton::clicked, this, [this]() { m_model->submit(); });
        QObject::connect(cancelButton, &FlatButton::clicked, this, [this]() { goBack(); });
        l->addLayout(buttonLayout, 0);

        auto updateCanSubmit = [this, okButton]() {
            okButton->setEnabled(m_model->canSubmit());
        };
        connect(m_model, &InputSelectionModel::canSubmitChanged, this, updateCanSubmit);
        updateCanSubmit();
    }
}


void InputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
