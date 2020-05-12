#pragma once

#include <QWidget>

class ModelEditorModel;

// Panel used to show a list of materials in a ModelEditorModel

class MaterialListView : public QWidget
{
public:
    MaterialListView(ModelEditorModel* model, QWidget* parent = nullptr);
};
