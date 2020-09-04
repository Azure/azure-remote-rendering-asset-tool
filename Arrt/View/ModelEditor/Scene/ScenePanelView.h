#pragma once

#include <QWidget>

class ModelEditorModel;

// Panel showing the scene tree in a ModelEditorModel

class ScenePanelView : public QWidget
{
public:
    ScenePanelView(ModelEditorModel* model, QWidget* parent = nullptr);

private:
    ModelEditorModel* const m_model;
};
