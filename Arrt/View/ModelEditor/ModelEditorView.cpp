#include <QApplication>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <View/ModelEditor/MaterialEditorView.h>
#include <View/ModelEditor/MaterialsList.h>
#include <View/ModelEditor/ModelEditorView.h>
#include <View/ModelEditor/ScenePanelView.h>
#include <View/ModelEditor/StatsPageView.h>
#include <View/ModelEditor/ViewportView.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelEditor/ViewportModel.h>
#include <ViewUtils/DpiUtils.h>
#include <Widgets/FlatButton.h>
#include <Widgets/FocusableContainer.h>

// the viewport widget has to be wrapped by a non native widget with WA_DontCreateNativeAncestors set,
// to make sure that the parent widgets won't be turned into native widgets. This is because this might break
// the accessibility for the non native widgets (the buttons in the main navigator stop handling the focus
// properly for the narrator)
// Remove this as soon as https://bugreports.qt.io/browse/QTBUG-81862 is fixed and released in 5.15.1 (scheduled in August)

class ContainerForViewport : public QWidget
{
public:
    ContainerForViewport(QWidget* parentWidget)
        : QWidget(parentWidget)
    {
        setContentsMargins(0, 0, 0, 0);

        // temporarily disabling it, because it causes a lot of problems with the viewport (shifted or missing rendering, z-order problems with the session panel)
        //setAttribute(Qt::WA_DontCreateNativeAncestors);
        setAttribute(Qt::WA_LayoutOnEntireRect);
        setMinimumWidth(256);
    }

    void setViewport(QWidget* viewport)
    {
        m_viewport = viewport;
        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(viewport);
        m_viewport->show();
    }
    virtual void resizeEvent(QResizeEvent* event) override
    {
        m_viewport->hide();
        QMetaObject::invokeMethod(QApplication::instance(), [this]() {m_viewport->hide(); m_viewport->show(); });
        QWidget::resizeEvent(event);
    }

private:
    QWidget* m_viewport = nullptr;
};

ModelEditorView::ModelEditorView(ModelEditorModel* modelEditorModel)
    : m_model(modelEditorModel)
{
    auto* l = new QVBoxLayout(this);

    QWidget* toolBar;
    {
        toolBar = new QWidget(this);
        auto* toolbarLayout = new QHBoxLayout(toolBar);

        m_currentlyLoadedModel = new QLabel();
        toolbarLayout->addWidget(m_currentlyLoadedModel);

        auto* unloadButton = new FlatButton(tr("Unload model"), toolBar);
        unloadButton->setToolTip(tr("Unload 3D model"), tr("Unload and go back to the panel for selecting another 3D model"));
        QObject::connect(unloadButton, &FlatButton::clicked, this, [this]() {
            m_model->unloadModel();
        });
        toolbarLayout->addWidget(unloadButton);

        toolbarLayout->addStretch(1);
    }

    l->addWidget(toolBar, 0);

    QSplitter* splitter;
    {
        splitter = new QSplitter(this);

        {
            auto scenePanel = new ScenePanelView(modelEditorModel, splitter);
            splitter->addWidget(new FocusableContainer(scenePanel, splitter));
        }

        {
            auto* viewportSplitter = new QSplitter(Qt::Vertical, splitter);

            FocusableContainer* viewportContainer;
            {
                viewportContainer = new FocusableContainer({}, viewportSplitter);
                auto* container = new ContainerForViewport(viewportContainer);
                auto* viewportView = new ViewportView(modelEditorModel->getViewportModel(), container);
                container->setViewport(viewportView);

                auto* viewportLayout = new QHBoxLayout(viewportContainer);
                viewportLayout->setContentsMargins(0, 0, 0, 0);
                viewportLayout->addWidget(container);
            }

            StatsPageView* statsPanel;
            {
                statsPanel = new StatsPageView(m_model->getStatsPageModel(), viewportSplitter);
            }

            viewportSplitter->addWidget(viewportContainer);
            viewportSplitter->addWidget(statsPanel);
            viewportSplitter->setCollapsible(0, false);
            viewportSplitter->setCollapsible(1, true);
            viewportSplitter->setStretchFactor(0, 1);
            viewportSplitter->setStretchFactor(1, 0);
            viewportSplitter->setSizes({(int)DpiUtils::size(800), 0});
            splitter->addWidget(viewportSplitter);
        }

        {
            auto* materialSplitter = new QSplitter(Qt::Vertical, splitter);
            auto* materialListView = new MaterialListView(modelEditorModel, materialSplitter);
            auto* materialEditorView = new MaterialEditorView(modelEditorModel->getEditingMaterial(), materialSplitter);

            materialSplitter->addWidget(new FocusableContainer(materialListView, materialSplitter));
            materialSplitter->addWidget(materialEditorView);
            materialSplitter->setChildrenCollapsible(false);
            materialSplitter->setMinimumWidth(DpiUtils::size(150));
            materialSplitter->setSizes({(int)DpiUtils::size(300), (int)DpiUtils::size(500)});
            materialSplitter->setStretchFactor(0, 0);
            materialSplitter->setStretchFactor(1, 1);

            splitter->addWidget(materialSplitter);
        }

        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        splitter->setStretchFactor(2, 0);
        splitter->setSizes({(int)DpiUtils::size(300), (int)DpiUtils::size(800), (int)DpiUtils::size(300)});
    }

    l->addWidget(splitter, 1);

    auto onEnabledChanged = [this]() {
        setEnabled(m_model->isEnabled());
    };
    onEnabledChanged();
    QObject::connect(m_model, &ModelEditorModel::onEnabledChanged, this, onEnabledChanged);

    QObject::connect(m_model, &ModelEditorModel::loadedModelChanged, this, [this]() { updateUi(); });
    updateUi();
}

void ModelEditorView::updateUi()
{
    m_currentlyLoadedModel->setText("<h3>" + m_model->getLoadedModeName() + "</h3>");
}

ModelEditorView::~ModelEditorView()
{
}
