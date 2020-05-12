#pragma once
#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/MultiComboBoxModel.h>
#include <Widgets/CheckComboBox.h>

// Combo box with multiple selection, bound to a MultiComboBoxModel

class BoundMultiComboBox : public CheckComboBox, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundMultiComboBox(MultiComboBoxModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    void updateFromModel() override;

private:
    QPointer<MultiComboBoxModel> m_model;
};
