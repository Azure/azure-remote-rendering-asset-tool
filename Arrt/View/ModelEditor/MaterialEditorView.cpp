#include <Model/ModelEditor/MaterialProvider.h>
#include <QVBoxLayout>
#include <View/ModelEditor/MaterialEditorView.h>
#include <View/Parameters/BoundWidget.h>
#include <View/Parameters/BoundWidgetFactory.h>
#include <ViewModel/Parameters/ParameterModel.h>
#include <Widgets/FormControl.h>
#include <Widgets/VerticalScrollArea.h>

MaterialEditorView::MaterialEditorView(MaterialProvider* model, QWidget* parent)
    : FocusableContainer({}, parent)
    , m_model(model)
{
    auto* scrollArea = new VerticalScrollArea(this);
    m_layout = scrollArea->getContentLayout();
    setChild(scrollArea);

    QObject::connect(m_model, &MaterialProvider::materialChanged, this, [this]() { updateFromModel(); });
    updateFromModel();
}

void MaterialEditorView::updateFromModel()
{
    const auto& controls = m_model->getControls();
    setVisible(!controls.empty());

    for (int i = 0; i < controls.size(); ++i)
    {
        if (m_widgets.size() == i)
        {
            m_widgets.push_back(new FormControl(this));
            m_layout->addWidget(m_widgets.back());
        }
        applyModelToFormControl(m_widgets[i], controls[i]);
    }

    for (int i = m_widgets.count() - 1; i >= controls.size(); --i)
    {
        delete m_widgets.last();
        m_widgets.removeLast();
    }
}

void MaterialEditorView::applyModelToFormControl(FormControl* dest, ParameterModel* model)
{
    QWidget* editor = dest->getWidget();
    BoundWidget* bw = qobject_cast<BoundWidget*>(editor);
    if (bw && bw->getModel() == model)
    {
        bw->updateFromModel();
    }
    else
    {
        dest->setHeader(model->getName());
        dest->setWidget(BoundWidgetFactory::createBoundWidget(model, dest));
    }
}
