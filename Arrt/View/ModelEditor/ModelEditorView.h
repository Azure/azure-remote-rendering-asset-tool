#pragma once
#include <QLabel>
#include <qwidget.h>

class ModelEditorModel;

// main view with all of the panels for material editing, scene inspection, viewport

class ModelEditorView : public QWidget
{
public:
    ModelEditorView(ModelEditorModel* modelEditorModel);
    ~ModelEditorView();

private:
    ModelEditorModel* const m_model;
    QLabel* m_currentlyLoadedModel;

    void updateUi();
};
