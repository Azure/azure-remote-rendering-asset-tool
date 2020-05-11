#pragma once
#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/ComboBoxModel.h>
#include <Widgets/CustomComboBox.h>

// Combobox bound to a ComboBoxModel

class BoundCombobox : public CustomComboBox, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundCombobox(ComboBoxModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<ComboBoxModel> m_model;
};
