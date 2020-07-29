#include <View/Parameters/BoundCheckBox.h>
#include <View/Parameters/BoundColor.h>
#include <View/Parameters/BoundComboBox.h>
#include <View/Parameters/BoundFloatSlider.h>
#include <View/Parameters/BoundFloatSpinBox.h>
#include <View/Parameters/BoundFloatVector.h>
#include <View/Parameters/BoundIntSpinBox.h>
#include <View/Parameters/BoundMultiComboBox.h>
#include <View/Parameters/BoundText.h>
#include <View/Parameters/BoundToggleButton.h>
#include <View/Parameters/BoundWidgetFactory.h>

#include <QDataWidgetMapper>

QWidget* BoundWidgetFactory::createBoundWidget(ParameterModel* model, QWidget* parent)

{
    if (auto* cb = qobject_cast<MultiComboBoxModel*>(model))
    {
        return new BoundMultiComboBox(cb, parent);
    }
    if (auto* cb = qobject_cast<ComboBoxModel*>(model))
    {
        return new BoundCombobox(cb, parent);
    }
    if (auto* f = qobject_cast<FloatVectorModel*>(model))
    {
        return new BoundFloatVector(f, parent);
    }
    if (auto* f = qobject_cast<FloatSliderModel*>(model))
    {
        return new BoundFloatSlider(f, parent);
    }
    if (auto* f = qobject_cast<FloatModel*>(model))
    {
        return new BoundFloatSpinBox(f, parent);
    }
    if (auto* i = qobject_cast<IntegerModel*>(model))
    {
        return new BoundIntSpinBox(i, parent);
    }
    if (auto* c = qobject_cast<ColorModel*>(model))
    {
        return new BoundColor(c, parent);
    }
    if (auto* tm = qobject_cast<TextModel*>(model))
    {
        return new BoundText(tm, parent);
    }
    if (auto* cbm = qobject_cast<CheckBoxModel*>(model))
    {
        return new BoundCheckBox(cbm, parent);
    }
    if (auto* tbm = qobject_cast<ToggleButtonModel*>(model))
    {
        return new BoundToggleButton(tbm, parent);
    }
    return nullptr;
}
