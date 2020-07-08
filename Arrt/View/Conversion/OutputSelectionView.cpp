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
#include <Widgets/ToolbarButton.h>
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);

OutputSelectionView::OutputSelectionView(OutputSelectionModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);

    ToolbarButton* refreshButton;
    ToolbarButton* okButton;
    ToolbarButton* cancelButton;
    {
        refreshButton = new ToolbarButton(tr("Refresh"), ArrtStyle::s_refreshIcon);
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));

        okButton = new ToolbarButton(tr("OK"));
        okButton->setToolTip(tr("OK"), tr("Select the output location for the conversion"));

        cancelButton = new ToolbarButton(tr("Cancel"));
        cancelButton->setToolTip(tr("Cancel"), tr("Go back to the conversion page without changing the output"));

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the output container and directory, where the converted 3D model will be stored")), 1);
        buttonLayout->addWidget(refreshButton);
        buttonLayout->addWidget(okButton);
        buttonLayout->addWidget(cancelButton);

        l->addLayout(buttonLayout, 0);
    }

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
    connect(refreshButton, &ToolbarButton::clicked, this, [this]() { m_model->refresh(); });
    connect(okButton, &ToolbarButton::clicked, this, [this]() { m_model->submit(); });
    connect(cancelButton, &ToolbarButton::clicked, this, [this]() { goBack(); });
}


void OutputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
