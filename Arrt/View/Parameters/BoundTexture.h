#pragma once

#include <QLineEdit>
#include <QPointer>
#include <View/Parameters/BoundWidget.h>
#include <ViewModel/Parameters/TextureModel.h>

// Read only line edit bound to a TextureModel

class BoundTexture : public QLineEdit, public BoundWidget
{
    Q_OBJECT
    Q_INTERFACES(BoundWidget)

public:
    BoundTexture(TextureModel* model, QWidget* parent = nullptr);
    virtual const ParameterModel* getModel() const override;
    virtual void updateFromModel() override;

private:
    QPointer<TextureModel> m_model;
};
