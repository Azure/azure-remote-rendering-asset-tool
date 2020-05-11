#include <QHBoxLayout>
#include <QLabel>
#include <View/Parameters/BoundFloatVector.h>
#include <Widgets/FormatDoubleSpinBox.h>

BoundFloatVector::BoundFloatVector(FloatVectorModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_numComponents(m_model->getCount())
{
    auto* l = new QHBoxLayout();
    setLayout(l);

    for (int i = 0; i < m_numComponents; ++i)
    {
        l->addWidget(new QLabel(model->getLabel(i)), 0);
        m_spinBoxes[i] = new FormatDoubleSpinBox(this, model->getFormat());
        l->addWidget(m_spinBoxes[i], 1);
        QObject::connect(m_spinBoxes[i], &FormatDoubleSpinBox::edited, this, [this, i]() {
            m_model->setValue(i, m_spinBoxes[i]->value());
        });
    }

    BoundFloatVector::updateFromModel();
}


const ParameterModel* BoundFloatVector::getModel() const
{
    return m_model;
}

void BoundFloatVector::updateFromModel()
{
    for (int i = 0; i < m_numComponents; ++i)
    {
        m_spinBoxes[i]->setValue(m_model->getValue(i));
    }
}
