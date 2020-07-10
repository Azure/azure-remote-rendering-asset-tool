#include <QComboBox>
#include <QProgressBar>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <Utils/ScopedBlockers.h>
#include <View/ArrtStyle.h>
#include <View/BlobExplorer/BlobContainerSelector.h>
#include <View/BlobExplorer/BlobExplorerView.h>
#include <View/ModelsPage/ModelsPageView.h>
#include <ViewModel/BlobExplorer/BlobContainerSelectorModel.h>
#include <ViewModel/BlobExplorer/BlobsListModel.h>
#include <ViewModel/ModelsPage/ModelsPageModel.h>
#include <Widgets/FormControl.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarButton.h>

Q_DECLARE_METATYPE(ModelsPageView::InputMode);

ModelsPageView::ModelsPageView(ModelsPageModel* modelsPageModel)
    : m_model(modelsPageModel)
    , m_explorer(new BlobExplorerView(modelsPageModel->getExplorerModel(), BlobExplorerView::ExplorerType::ModelSelector, this))
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(ArrtStyle::createHeaderLabel({}, tr("Select model to load for rendering")));

    {
        auto* uploadButton = new ToolbarButton(tr("Upload files"), ArrtStyle::s_uploadIcon);
        uploadButton->setToolTip(tr("Upload files"), tr("Select local files and/or directories and upload them to Azure Storage, in the current directory"));
        connect(uploadButton, &ToolbarButton::clicked, this, [this]() { m_explorer->selectFilesToUpload(); });

        auto* refreshButton = new ToolbarButton(tr("Refresh"), ArrtStyle::s_refreshIcon);
        refreshButton->setToolTip(tr("Refresh"), tr("Refresh the containers and the blob list currently visualized"));
        connect(refreshButton, &ToolbarButton::clicked, this, [this]() { m_model->refresh(); });

        auto* toolbar = new Toolbar(this);
        toolbar->addButton(uploadButton);
        toolbar->addButton(refreshButton);
        mainLayout->addWidget(toolbar);
    }

    auto onBlobStorageAvailableFunc = [this]() {
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_inputMode->model());
        Q_ASSERT(model != nullptr);
        QStandardItem* item = model->item(FROM_STORAGE_CONTAINER);
        const bool available = m_model->isBlobStorageAvailable();
        item->setFlags(available ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);

        m_isUiChange = false;
        if (!available)
        {
            m_inputMode->setCurrentIndex(FROM_SAS_URI);
        }
        else
        {
            m_inputMode->setCurrentIndex(m_userPreferredSelection);
        }
        m_isUiChange = true;
    };
    {
        m_inputMode = new QComboBox();
        m_inputMode->insertItem(FROM_STORAGE_CONTAINER, tr("From storage container"), QVariant::fromValue(FROM_STORAGE_CONTAINER));
        m_inputMode->insertItem(FROM_SAS_URI, tr("From Sas URL"), QVariant::fromValue(FROM_SAS_URI));
        connect(m_inputMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
            setInputMode(m_inputMode->itemData(index).value<InputMode>());
        });

        auto* fc = new FormControl(tr("Input Mode"), m_inputMode);
        fc->setToolTip(tr("Input Mode"), tr("Way to obtain the address of the input 3D model"));
        mainLayout->addWidget(fc, 0);

        connect(m_model, &ModelsPageModel::onBlobStorageAvailabilityChanged, this, [this, onBlobStorageAvailableFunc]() {
            onBlobStorageAvailableFunc();
            updateUi();
        });
    }

    {
        m_modelSelectionLayout = new QStackedLayout();
        m_modelSelectionLayout->setContentsMargins(0, 0, 0, 0);

        {
            auto* fromStorageContainer = new QWidget(this);
            fromStorageContainer->setContentsMargins(0, 0, 0, 0);

            auto* l = new QVBoxLayout(fromStorageContainer);
            l->setContentsMargins(0, 0, 0, 0);

            auto* cb = new BlobContainerSelector(m_model->getContainersModel());

            auto* fc = new FormControl(tr("Blob container"), cb);
            fc->setToolTip(tr("Blob container"), tr("Azure Storage blob container from where to load the 3D model"));
            l->addWidget(fc);

            fc = new FormControl(tr("3D Model to load"), m_explorer);
            fc->setToolTip(tr("3D Model to load on ARR"), tr("Select the model that needs to be loaded and rendered in ARR"));
            l->addWidget(fc);

            m_modelSelectionLayout->insertWidget(FROM_STORAGE_CONTAINER, fromStorageContainer);
        }

        {
            auto* fromSasUri = new QWidget(this);
            fromSasUri->setContentsMargins(0, 0, 0, 0);
            auto* l = new QVBoxLayout(fromSasUri);
            l->setContentsMargins(0, 0, 0, 0);

            auto* sasUriTextField = new QLineEdit();
            l->addWidget(sasUriTextField, 1);
            sasUriTextField->setText(m_model->getModelSasUri());
            connect(sasUriTextField, &QLineEdit::textChanged, this, [this](const QString& text) { m_model->setModelSasUri(text); });
            connect(sasUriTextField, &QLineEdit::returnPressed, this, [this]() { m_model->load(ModelsPageModel::FromSasUri); });

            auto* fc = new FormControl(tr("3D Model SAS URI"), sasUriTextField);
            fc->setToolTip(tr("3D Model SAS URI"), tr("Full URL for the 3D model, including a SAS. Read the user documentation to learn how to obtain a SAS URI from a blob"));
            l->addWidget(fc, 0);
            l->addStretch(1);

            m_modelSelectionLayout->insertWidget(FROM_SAS_URI, fromSasUri);
        }

        mainLayout->addLayout(m_modelSelectionLayout, 1);
    }

    {
        auto* h = new QHBoxLayout();

        m_loadButton = new ToolbarButton(tr("Load"));
        m_loadButton->setToolTip(tr("Load model"), tr("Load the selected 3D model"));
        connect(m_loadButton, &ToolbarButton::clicked, this, [this]() {
            m_model->load(getInputMode() == FROM_STORAGE_CONTAINER ? ModelsPageModel::FromExplorer : ModelsPageModel::FromSasUri);
        });

        m_modelLoading = new QLineEdit(this);
        m_modelLoading->setReadOnly(true);
        m_modelLoading->setAccessibleName(tr("Loading model"));

        m_progressBar = new QProgressBar(this);
        m_progressBar->setMinimum(0);
        m_progressBar->setMaximum(100);
        m_progressBar->setAccessibleName(tr("Loading progress"));

        m_modelLoadingStatus = new QLabel(this);
        m_modelLoadingStatus->setFixedWidth(130);
        m_modelLoadingStatus->setAccessibleName(tr("Loading status"));

        h->addWidget(new FormControl({}, m_loadButton));
        h->addWidget(m_modelLoading, 50);
        h->addWidget(m_progressBar, 50);
        h->addWidget(m_modelLoadingStatus, 0);
        h->addStretch(1);

        mainLayout->addLayout(h, 0);
    }

    onBlobStorageAvailableFunc();

    auto onEnabledChanged = [this]() {
        setEnabled(m_model->isEnabled());
    };
    onEnabledChanged();
    QObject::connect(m_model, &ModelsPageModel::onEnabledChanged, onEnabledChanged);

    connect(m_model, &ModelsPageModel::canLoadChanged, this, [this]() {
        updateUi();
    });
    connect(m_model, &ModelsPageModel::loadingStatusChanged, this, [this]() {
        updateUi();
    });
    updateUi();
}

void ModelsPageView::setInputMode(InputMode inputMode)
{
    if (m_isUiChange)
    {
        m_userPreferredSelection = inputMode;
    }
    m_modelSelectionLayout->setCurrentIndex(static_cast<int>(inputMode));
    updateUi();
}

ModelsPageView::InputMode ModelsPageView::getInputMode() const
{
    const int ci = m_modelSelectionLayout->currentIndex();
    if (ci >= 0)
    {
        return static_cast<InputMode>(ci);
    }
    else
    {
        return FROM_STORAGE_CONTAINER;
    }
}

void ModelsPageView::updateUi()
{
    m_modelLoading->setVisible(m_model->getCurrentLoadingStatus() != BlobsListModel::LoadingStatus::NOT_LOADED);
    m_progressBar->setVisible(m_model->getCurrentLoadingStatus() == BlobsListModel::LoadingStatus::LOADING);
    m_modelLoadingStatus->setVisible(m_model->getCurrentLoadingStatus() != BlobsListModel::LoadingStatus::NOT_LOADED);

    m_modelLoading->setText(m_model->getCurrentLoadingModel());
    m_progressBar->setValue(m_model->getCurrentLoadingProgress() * 100.0 + 0.4);
    m_modelLoadingStatus->setText(BlobsListModel::statusToString(m_model->getCurrentLoadingStatus()));

    m_loadButton->setEnabled(m_model->canLoad(getInputMode() == FROM_STORAGE_CONTAINER ? ModelsPageModel::FromExplorer : ModelsPageModel::FromSasUri));
}
