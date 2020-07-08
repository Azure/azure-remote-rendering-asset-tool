#include <Model/IncludesAzureStorage.h>
#include <QVBoxLayout>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/Upload/UploadView.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/Upload/UploadModel.h>
#include <Widgets/ToolbarButton.h>
#include <Widgets/FormControl.h>

UploadView::UploadView(UploadModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);

    ToolbarButton* refreshButton;
    {
        refreshButton = new ToolbarButton(tr("Refresh"));
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));
        refreshButton->setIcon(ArrtStyle::s_refreshIcon, true);

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(ArrtStyle::createHeaderLabel({}, tr("Upload your 3D model (fbx, gltf, glb) to Azure Storage, by dropping files in the blob list or by clicking on \"upload files\"")), 1);
        buttonLayout->addWidget(refreshButton);

        l->addLayout(buttonLayout, 0);
    }

    {
        auto* cb = new BlobContainerSelector(model->getContainersModel());
        auto* fc = new FormControl(tr("Blob container"), cb);
        fc->setToolTip(tr("Blob container"), tr("Azure Storage blob container where the input 3D model is located"));
        l->addWidget(fc);
    }

    {
        m_explorer = new BlobExplorerView(m_model->getExplorerModel(), BlobExplorerView::ExplorerType::BlobExplorer, this);
        auto* fc = new FormControl(tr("Blob list"), m_explorer);
        fc->setToolTip(tr("Blob list"), tr("Azure Storage blob container where the input 3D model is located"));
        l->addWidget(fc);
    }

    connect(refreshButton, &ToolbarButton::clicked, this, [this]() { m_model->refresh(); });
}
