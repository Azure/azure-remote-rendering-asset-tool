#pragma once

#include <QLineEdit>
#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/TextModel.h>

// Check box bound to a CheckBoxModel

class BoundText : public QLineEdit, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundText(TextModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    virtual void updateFromModel() override;

private:
    QPointer<TextModel> m_model;
};
