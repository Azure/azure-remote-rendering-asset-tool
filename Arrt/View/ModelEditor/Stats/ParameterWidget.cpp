#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <View/ModelEditor/Stats/ParameterWidget.h>
#include <View/ModelEditor/Stats/StatsGraph.h>
#include <ViewUtils/DpiUtils.h>
#include <ViewUtils/Formatter.h>
#include <Widgets/ReadOnlyText.h>

namespace
{
    ReadOnlyText* newValueLabel(QString text = "", QWidget* parent = {})
    {
        auto* ret = new ReadOnlyText(text, parent);
        ret->setMinimumWidth(DpiUtils::size(100));
        ret->setAccessibleName(text);
        ret->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        return ret;
    }
} // namespace

class ColoredBox : public QWidget
{
public:
    ColoredBox(QColor color, QWidget* parent = {})
        : QWidget(parent)
        , m_color(color)
    {
        setContentsMargins(QMargins());
        setFixedSize(DpiUtils::size(16), DpiUtils::size(16));
    }
    virtual void paintEvent(QPaintEvent* /*event*/)
    {
        QStylePainter p(this);
        p.setBrush(m_color);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), DpiUtils::size(4), DpiUtils::size(4), Qt::AbsoluteSize);
    }

private:
    const QColor m_color;
};


ParameterWidget::ParameterWidget(QString name, QString unit, QColor color, QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(QMargins());
    m_unit = unit;

    auto* bl = new QHBoxLayout(this);
    bl->setContentsMargins(QMargins());

    m_legend = new ColoredBox(color, this);
    bl->addWidget(m_legend, 0);
    m_legend->setVisible(false);
    auto* label = new QLabel(name);
    bl->addWidget(label, 1);

    m_valueLabel = newValueLabel("", this);
    m_valueLabel->setAccessibleName(name + " " + tr("value"));
    bl->addWidget(m_valueLabel);

    m_minLabel = newValueLabel("", this);
    m_minLabel->setAccessibleName(name + " " + tr("minimum"));
    bl->addWidget(m_minLabel, 0);

    m_maxLabel = newValueLabel("", this);
    m_maxLabel->setAccessibleName(name + " " + tr("maximum"));
    bl->addWidget(m_maxLabel, 0);

    m_averageLabel = newValueLabel("", this);
    m_averageLabel->setAccessibleName(name + " " + tr("average"));
    bl->addWidget(m_averageLabel, 0);
}

void ParameterWidget::setLegendVisibility(bool visible)
{
    m_legend->setVisible(visible);
}

void ParameterWidget::setValues(float value, float minValue, float maxValue, float averageValue)
{
    m_valueLabel->setText(DoubleFormatter::toCompactString(value, m_unit));
    m_minLabel->setText(DoubleFormatter::toCompactString(minValue, m_unit));
    m_maxLabel->setText(DoubleFormatter::toCompactString(maxValue, m_unit));
    m_averageLabel->setText(DoubleFormatter::toCompactString(averageValue, m_unit));
}


QWidget* ParameterWidget::createHeader(QWidget* parent)
{
    QWidget* header = new QWidget(parent);
    auto* headerLayout = new QHBoxLayout(header);
    {
        headerLayout->addWidget(new QLabel(tr("Parameter Name")), 1);
        headerLayout->addWidget(newValueLabel(tr("Value"), header), 0);
        headerLayout->addWidget(newValueLabel(tr("Minimum"), header), 0);
        headerLayout->addWidget(newValueLabel(tr("Maximum"), header), 0);
        headerLayout->addWidget(newValueLabel(tr("Average"), header), 0);
    }
    QMargins m = header->contentsMargins();
    m.setBottom(0);
    header->setContentsMargins(m);
    return header;
}