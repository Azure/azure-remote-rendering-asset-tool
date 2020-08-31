#include <QEvent>
#include <QHBoxLayout>
#include <QSlider>
#include <QStylePainter>
#include <QtMath>
#include <Utils/ScopedBlockers.h>
#include <View/ArrtStyle.h>
#include <View/Parameters/BoundFloatSlider.h>
#include <View/Parameters/BoundFloatSpinBox.h>

// customized slider which doesn't process wheel events, since sometimes when it's in a scrollable panel,
// it can grab wheel events that are meant to be used for scrolling

class SliderNoWheel : public QSlider
{
public:
    SliderNoWheel(Qt::Orientation orientation, QWidget* parent = {})
        : QSlider(orientation, parent)
    {
        setFocusPolicy(Qt::StrongFocus);
        setFixedHeight(ArrtStyle::controlHeight());
    }

    bool event(QEvent* e) override
    {
        // Disable handling of scrollwheel by sliders
        if (e->type() == QEvent::Wheel)
        {
            return false;
        }

        return QSlider::event(e);
    }

    virtual void paintEvent(QPaintEvent* ev) override
    {
        QSlider::paintEvent(ev);
        if (hasFocus())
        {
            QStylePainter p(this);
            const int w = ArrtStyle::s_focusedControlBorderWidth;
            p.setPen(QPen(ArrtStyle::s_focusedControlBorderColor, w));
            p.setBrush(Qt::NoBrush);
            p.drawRect(rect().adjusted(w / 2, w / 2, -w / 2, -w / 2));
        }
    }
};



BoundFloatSlider::BoundFloatSlider(FloatSliderModel* model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
    , m_sliderMinValue(model->isExpSlider() ? (qLn(model->getMinimum()) / qLn(10.0)) : model->getMinimum())
    , m_sliderMaxValue(model->isExpSlider() ? (qLn(model->getMaximum()) / qLn(10.0)) : model->getMaximum())
{
    auto* layout = new QHBoxLayout(this);

    m_slider = new SliderNoWheel(Qt::Horizontal, this);

    m_slider->setMinimum(0);
    m_slider->setMaximum(m_model->getNumberOfSteps());
    m_slider->setAccessibleName(model->getName());
    layout->addWidget(m_slider);

    m_spinBox = new BoundFloatSpinBox(model, this);
    m_spinBox->setMinimumWidth(120);
    m_spinBox->setAccessibleName(model->getName());
    layout->addWidget(m_spinBox);

    BoundFloatSlider::updateFromModel();

    QObject::connect(m_slider, &QSlider::valueChanged, this, [this]() {
        m_model->setValue(getSliderValue());
        m_spinBox->updateFromModel();
    });

    // the connection is queued, so we make sure that the model update for BoundFloatSpinBox is done before updating the slider from the model
    QObject::connect(
        m_spinBox, static_cast<void (BoundFloatSpinBox::*)(double)>(&BoundFloatSpinBox::valueChanged), this,
        [this](double) {
            setSliderValue(m_model->getValue());
        },
        Qt::ConnectionType::QueuedConnection);
}

const ParameterModel* BoundFloatSlider::getModel() const
{
    return m_model;
}

void BoundFloatSlider::updateFromModel()
{
    m_spinBox->updateFromModel();
    setSliderValue(m_model->getValue());
}

void BoundFloatSlider::setSliderValue(float val)
{
    const float v = m_model->isExpSlider() ? qLn(val) / qLn(10) : val;
    const float t = (v - m_sliderMinValue) / (m_sliderMaxValue - m_sliderMinValue);

    ScopedBlockSignals blockSignal(m_slider);
    m_slider->setValue(m_model->getNumberOfSteps() * t);
}

float BoundFloatSlider::getSliderValue() const
{
    const float t = (float)m_slider->value() / m_model->getNumberOfSteps();
    const float v = m_sliderMinValue + t * (m_sliderMaxValue - m_sliderMinValue);

    return m_model->isExpSlider() ? qPow(10.0, v) : v;
}
