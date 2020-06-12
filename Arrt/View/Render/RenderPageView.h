#pragma once
#include <Widgets/Navigator.h>

class RenderPageModel;
class SessionPanelView;

// view for the render page, which shows either the model loading page, or the material editor

class RenderPageView : public Navigator
{
    Q_OBJECT
public:
    RenderPageView(RenderPageModel* model, QWidget* parent = {});

private:
    RenderPageModel* const m_model;
    SessionPanelView* const m_sessionPanelView;
};
