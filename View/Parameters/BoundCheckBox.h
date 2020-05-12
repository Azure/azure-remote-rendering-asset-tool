#pragma once

#include <QCheckBox>
#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/CheckBoxModel.h>

// Check box bound to a CheckBoxModel

class BoundCheckBox : public QCheckBox, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundCheckBox(CheckBoxModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    virtual void updateFromModel() override;

private:
    QPointer<CheckBoxModel> m_model;
};
