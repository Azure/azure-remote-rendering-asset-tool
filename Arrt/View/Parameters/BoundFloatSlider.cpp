#include <QHBoxLayout>
#include <QSlider>
#include <Utils/ScopedBlockers.h>
#include <View/Parameters/BoundFloatSlider.h>
#include <View/Parameters/BoundFloatSpinBox.h>

BoundFloatSlider::BoundFloatSlider(FloatSliderModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    auto* layout = new QHBoxLayout(this);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(0);
    m_slider->setMaximum(m_model->getNumberOfSteps());
    layout->addWidget(m_slider);

    m_spinBox = new BoundFloatSpinBox(model, this);
    m_spinBox->setMinimumWidth(120);
    layout->addWidget(m_spinBox);

    BoundFloatSlider::updateFromModel();

    QObject::connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
        const float delta = m_model->getMaximum() - m_model->getMinimum();
        const float t = (float)value / m_model->getNumberOfSteps();

        m_model->setValue(m_model->getMinimum() + t * delta);

        m_spinBox->updateFromModel();
    });

	// the connection is queued, so we make sure that the model update for BoundFloatSpinBox is done before updating the slider from the model
    QObject::connect(m_spinBox, static_cast<void (BoundFloatSpinBox::*)(double)>(&BoundFloatSpinBox::valueChanged), this,
                     [this](double) {
                         updateFromModel();
                     },
                     Qt::ConnectionType::QueuedConnection);
}

const ParameterModel* BoundFloatSlider::getModel() const
{
    return m_model;
}

void BoundFloatSlider::updateFromModel()
{
    const float delta = m_model->getMaximum() - m_model->getMinimum();
    ScopedBlockSignals blockSignal(m_slider);
    m_slider->setValue(m_model->getNumberOfSteps() * (m_model->getValue() - m_model->getMinimum()) / delta);
}
