#include <Model/IncludesAzureStorage.h>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/Conversion/OutputSelectionView.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobExplorerModel.h>
#include <ViewModel/Conversion/OutputSelectionModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);

OutputSelectionView::OutputSelectionView(OutputSelectionModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);

    l->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the output container and directory, where the converted 3D model will be stored")));

    {
        auto* cb = new BlobContainerSelector(model->getContainersModel());
        auto* fc = new FormControl(tr("Blob container"), cb);
        fc->setToolTip(tr("Blob container"), tr("Azure Storage blob container for the output directory"));
        l->addWidget(fc);
    }

    {
        m_explorer = new BlobExplorerView(m_model->getExplorerModel(), BlobExplorerView::ExplorerType::DirectorySelector, this);
        auto* fc = new FormControl(tr("Output Directory"), m_explorer);
        fc->setToolTip(tr("Output Directory for Conversion"), tr("This is the destination directory where Conversion will write the output files"));
        l->addWidget(fc);
    }

    connect(m_model, &OutputSelectionModel::submitted, this, [this]() { goBack(); });

    {
        FlatButton* okButton = new FlatButton(tr("OK"));
        FlatButton* cancelButton = new FlatButton(tr("Cancel"));

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        QObject::connect(okButton, &FlatButton::pressed, this, [this]() { m_model->submit(); });
        QObject::connect(cancelButton, &FlatButton::pressed, this, [this]() { goBack(); });
        l->addLayout(buttonLayout, 0);
    }
}


void OutputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
