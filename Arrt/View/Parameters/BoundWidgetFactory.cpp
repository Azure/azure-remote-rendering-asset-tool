#include <View/Parameters/BoundCheckBox.h>
#include <View/Parameters/BoundColor.h>
#include <View/Parameters/BoundComboBox.h>
#include <View/Parameters/BoundFloatSlider.h>
#include <View/Parameters/BoundFloatSpinBox.h>
#include <View/Parameters/BoundFloatVector.h>
#include <View/Parameters/BoundIntSpinBox.h>
#include <View/Parameters/BoundMultiComboBox.h>
#include <View/Parameters/BoundText.h>
#include <View/Parameters/BoundTexture.h>
#include <View/Parameters/BoundToggleButton.h>
#include <View/Parameters/BoundWidgetFactory.h>

#include <QDataWidgetMapper>

QWidget* BoundWidgetFactory::createBoundWidget(ParameterModel* model, QWidget* parent)

{
    {
        if (auto* m = qobject_cast<MultiComboBoxModel*>(model))
        {
            return new BoundMultiComboBox(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<ComboBoxModel*>(model))
        {
            return new BoundCombobox(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<FloatVectorModel*>(model))
        {
            return new BoundFloatVector(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<FloatSliderModel*>(model))
        {
            return new BoundFloatSlider(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<FloatModel*>(model))
        {
            return new BoundFloatSpinBox(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<IntegerModel*>(model))
        {
            return new BoundIntSpinBox(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<ColorModel*>(model))
        {
            return new BoundColor(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<TextModel*>(model))
        {
            return new BoundText(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<CheckBoxModel*>(model))
        {
            return new BoundCheckBox(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<ToggleButtonModel*>(model))
        {
            return new BoundToggleButton(m, parent);
        }
    }
    {
        if (auto* m = qobject_cast<TextureModel*>(model))
        {
            return new BoundTexture(m, parent);
        }
    }

    return nullptr;
}
