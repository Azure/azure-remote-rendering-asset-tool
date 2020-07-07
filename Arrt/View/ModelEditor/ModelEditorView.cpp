#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <View/ModelEditor/MaterialEditorView.h>
#include <View/ModelEditor/MaterialsList.h>
#include <View/ModelEditor/ModelEditorView.h>
#include <View/ModelEditor/ScenePanelView.h>
#include <View/ModelEditor/ViewportView.h>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelEditor/ViewportModel.h>
#include <Widgets/FlatButton.h>

// the viewport widget has to be wrapped by a non native widget with WA_DontCreateNativeAncestors set,
// to make sure that the parent widgets won't be turned into native widgets. This is because this might break
// the accessibility for the non native widgets (the buttons in the main navigator stop handling the focus
// properly for the narrator)
class ContainerForViewport : public QWidget
{
public:
    ContainerForViewport(QWidget* parentWidget)
        : QWidget(parentWidget)
    {
        setAttribute(Qt::WA_DontCreateNativeAncestors);
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
            auto scenePanel = new ScenePanelView(modelEditorModel, this);
            splitter->addWidget(scenePanel);
        }

        {
            auto container = new ContainerForViewport(splitter);
            auto viewportModel = new ViewportView(modelEditorModel->getViewportModel(), container);
            container->setViewport(viewportModel);
            splitter->addWidget(container);
        }

        {
            auto materialPanel = new MaterialListView(modelEditorModel, this);
            splitter->addWidget(materialPanel);
        }

        {
            auto materialPanel = new MaterialEditorView(modelEditorModel->getEditingMaterial(), this);
            splitter->addWidget(materialPanel);
        }

        splitter->setSizes({10, 40, 10, 10});
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
