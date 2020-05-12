#include <View/ModelEditor/ModelEditorView.h>
#include <View/ModelsPage/ModelsPageView.h>
#include <View/Render/RenderPageView.h>
#include <View/Session/SessionInfoView.h>
#include <ViewModel/Render/RenderPageModel.h>


RenderPageView::RenderPageView(RenderPageModel* model, QWidget* parent)
    : Navigator(parent)
    , m_model(model)
    , m_sessionPanelView(new SessionPanelView(model->getSessionPanelModel(), this))
{
    setPageFactory([this](int index) -> QWidget* {
        switch (index)
        {
            case (int)RenderPageModel::FocusedPage::MODEL_SELECTION:
                return new ModelsPageView(m_model->getModelsPageModel());
            case (int)RenderPageModel::FocusedPage::MODEL_EDITING:
                return new ModelEditorView(m_model->getModelEditorModel());
            default:
                return nullptr;
        }
    });

    m_sessionPanelView->show();

    connect(this, &Navigator::pageNavigated, this, [this]() {
        m_sessionPanelView->raise();
    });

    auto onPageChanged = [this]() {
        navigateToPage((int)m_model->getFocusedPage());
    };
    QObject::connect(m_model, &RenderPageModel::focusedPageChanged, this, onPageChanged);
    onPageChanged();
}
