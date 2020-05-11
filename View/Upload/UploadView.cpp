#include <Model/IncludesAzureStorage.h>
#include <QVBoxLayout>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/Upload/UploadView.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/Upload/UploadModel.h>
#include <Widgets/FormControl.h>

UploadView::UploadView(UploadModel* model)
    : m_model(model)
{
    auto* l = new QVBoxLayout(this);
    auto* label = new QLabel(tr("<small>Upload your 3D model (fbx, gltf, glb) to Azure Storage, by dropping files in the blob list or by clicking on \"upload files\""));
    label->setWordWrap(true);
    l->addWidget(label);

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
}
