#pragma once
#include <Widgets/FocusableContainer.h>

class MaterialProvider;
class ParameterModel;
class FormControl;

// View for the property editor for the material, exposing all of the controls

class MaterialEditorView : public FocusableContainer
{
public:
    MaterialEditorView(MaterialProvider* model, QWidget* parent = nullptr);

private:
    MaterialProvider* const m_model;
    QList<FormControl*> m_widgets;
    QLayout* m_layout = {};

    // called when the control models have changed, to regenerate the controls
    void updateFromModel();

    // called to update a control with a new model (will just update the value if there is already the correct control)
    static void applyModelToFormControl(FormControl* dest, ParameterModel* model);
};
