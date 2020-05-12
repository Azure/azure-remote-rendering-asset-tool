#include <View/BlobExplorer/BlobContainerSelector.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>

BlobContainerSelector::BlobContainerSelector(BlobContainerSelectorModel* model, QWidget* parent)
    : QComboBox(parent)
    , m_model(model)
{
    setModel(m_model->getAvailableContainersModel());

    auto onModelContainerChanged = [this]() {
        setCurrentText(m_model->getCurrentContainer());
    };
    connect(model, &BlobContainerSelectorModel::currentContainerChanged, this, onModelContainerChanged);
    onModelContainerChanged();

    auto onIndexChanged = [this](const QString& container) {
        m_model->setCurrentContainer(container);
    };
    connect(this, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, onIndexChanged);
}
