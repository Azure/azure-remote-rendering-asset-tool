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
#include <Widgets/FormControl.h>
#include <Widgets/Navigator.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarButton.h>

Q_DECLARE_METATYPE(azure::storage::storage_uri);

OutputSelectionView::OutputSelectionView(OutputSelectionModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);
    l->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select the output container and directory, where the converted 3D model will be stored")));

    {
        auto* backButton = new ToolbarButton(tr("Back"), ArrtStyle::s_backIcon);
        backButton->setToolTip(tr("Back"), tr("Go back to the conversion page without changing the output"));
        connect(backButton, &ToolbarButton::clicked, this, [this]() { goBack(); });

        auto* refreshButton = new ToolbarButton(tr("Refresh"), ArrtStyle::s_refreshIcon);
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));
        connect(refreshButton, &ToolbarButton::clicked, this, [this]() { m_model->refresh(); });

        auto* toolbar = new Toolbar(this);
        toolbar->addButton(backButton);
        toolbar->addButton(refreshButton);
        l->addWidget(toolbar);
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
        l->addWidget(fc, 1);
    }

    {
        auto* okButton = new ToolbarButton(tr("Select Output"));
        okButton->setToolTip(tr("Select Output"), tr("Select the output location for the conversion"));
        connect(okButton, &ToolbarButton::clicked, this, [this]() { m_model->submit(); });

        auto* toolbar = new Toolbar(this);
        toolbar->addButton(okButton);
        l->addWidget(toolbar);
    }

    connect(m_model, &OutputSelectionModel::submitted, this, [this]() { goBack(); });
}


void OutputSelectionView::goBack()
{
    Navigator* navigator = Navigator::getNavigator(this);
    QMetaObject::invokeMethod(
        navigator, [navigator] { navigator->back(); }, Qt::QueuedConnection);
}
