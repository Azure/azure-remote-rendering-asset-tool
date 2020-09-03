#include <QFormLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <QTransform>
#include <QVBoxLayout>
#include <QtMath>
#include <View/ModelEditor/StatsPageView.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>
#include <ViewUtils/DpiUtils.h>
#include <Widgets/FlatButton.h>
#include <Widgets/VerticalScrollArea.h>

class ColoredBox : public QWidget
{
public:
    ColoredBox(QColor color, QWidget* parent = {})
        : QWidget(parent)
        , m_color(color)
    {
        setContentsMargins(0, 0, 0, 0);
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

SimpleGraph::SimpleGraph(QWidget* parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumHeight(200);
}

namespace
{
    void findScale(int pixels, int minStepPixelDistance, float minimum, float maximum, float& outMinimum, float& outMaximum, float& outStep)
    {
        if (maximum - minimum < 0.001)
        {
            outMinimum = minimum;
            outMaximum = maximum;
            outStep = 0;
            return;
        }
        const int maxSteps = qFloor(pixels / minStepPixelDistance);

        float step = (maximum - minimum) / maxSteps;
        const float nextPowerOf10 = qPow(10.0, qCeil(qLn(step) / qLn(10.0)));
        const float stepRatio = step / nextPowerOf10;
        if (stepRatio > 0.5)
        {
            step = nextPowerOf10;
        }
        else if (stepRatio > 0.2)
        {
            step = nextPowerOf10 / 2.0;
        }
        else if (stepRatio > 0.1)
        {
            step = nextPowerOf10 / 5.0;
        }
        else
        {
            step = nextPowerOf10 / 10.0;
        }
        outMinimum = qFloor(minimum / step) * step;
        outMaximum = qCeil(maximum / step) * step;
        outStep = step;
    }
} // namespace

void SimpleGraph::paintEvent(QPaintEvent* e)
{
    QStylePainter p(this);
    p.fillRect(e->rect(), Qt::black);

    float yMin, yMax, yStep;
    float xMin, xMax, xStep;

    QFont smallFont("Segoe UI", 8);
    QFontMetrics smallFontM(smallFont, this);

    const float unit = smallFontM.height();
    QRect graphRect = rect().adjusted(unit * 3, unit, -unit * 3, -unit * 2);
    findScale(graphRect.height(), 40, m_minimum, m_maximum, yMin, yMax, yStep);
    findScale(graphRect.width(), 100, 0, graphRect.width() / m_xZoom, xMin, xMax, xStep);

    QTransform transform;
    transform.translate(graphRect.right(), graphRect.bottom());
    transform.scale(-graphRect.width(), -graphRect.height());
    transform.translate(-xMin, -yMin);
    qreal yrange = yMax - yMin;
    if (yrange < 0.001)
    {
        yrange = 1;
    }
    qreal xrange = xMax - xMin;
    if (xrange < 0.001)
    {
        xrange = 1;
    }
    transform.scale(1.0 / xrange, 1.0 / yrange);

    p.save();
    p.setClipRect(graphRect.adjusted(-1, -1, 1, 1));
    p.setTransform(transform);

    p.setBrush(Qt::NoBrush);

    for (int i = 0; i < getPlotCount(); ++i)
    {
        const auto& pd = accessPlotData(i);
        if (pd.size() > 0)
        {
            p.setPen(QPen(m_infos[i].m_color, 0));
            p.drawPolyline(pd.data(), (int)pd.size());
        }
    }
    p.restore();

    QColor linesColor = palette().mid().color();
    QPen linePen(linesColor, 0);

    QColor scaleLinesColor(70, 70, 70);
    QPen scaleLinesPen(scaleLinesColor, 0);

    p.setFont(smallFont);

    if (yStep > 0)
    {
        for (qreal y = yMin; y < yMax + yStep / 2; y += yStep)
        {
            QPoint pt;
            pt = transform.map(QPoint(0, y));

            p.setPen(scaleLinesPen);
            p.drawLine(graphRect.left(), pt.y(), graphRect.right(), pt.y());
            QString toPrint = QString::number(y) + m_infos[0].m_units;
            int w = smallFontM.horizontalAdvance(toPrint);

            p.setPen(linePen);
            p.drawText(graphRect.left() - w - unit * 0.3, pt.y(), toPrint);
        }
    }
    if (xStep > 0)
    {
        for (qreal x = xMin; x <= xMax; x += xStep)
        {
            QPoint pt;
            pt = transform.map(QPoint(x, 0));

            p.setPen(scaleLinesPen);
            p.drawLine(pt.x(), graphRect.top(), pt.x(), graphRect.bottom());
            QString toPrint = QString::number(x);
            int w = smallFontM.horizontalAdvance(toPrint);

            p.setPen(linePen);
            p.drawText(pt.x() - w / 2, graphRect.bottom() + smallFontM.height(), toPrint);
        }
    }

    p.setPen(linePen);
    p.drawLine(graphRect.bottomLeft(), graphRect.bottomRight());
    p.drawLine(graphRect.bottomLeft(), graphRect.topLeft());

    {
        int w = smallFontM.horizontalAdvance(m_xLabelText);
        p.drawText((graphRect.right() - graphRect.left()) / 2 - w / 2, height() - unit * 0.2, m_xLabelText);
    }
}

int SimpleGraph::getPlotCount() const
{
    return (int)m_infos.size();
}

void SimpleGraph::setMinMax(qreal minimum, qreal maximum)
{
    m_minimum = minimum;
    m_maximum = maximum;
}

void SimpleGraph::setXLabel(QString text)
{
    m_xLabelText = text;
}

void SimpleGraph::setXZoom(float xZoom)
{
    m_xZoom = xZoom;
}

int SimpleGraph::addPlot(StatsPageModel::PlotInfo type)
{
    const int ret = (int)m_infos.size();
    m_infos.push_back(std::move(type));
    m_data.push_back({});
    return ret;
}

ParameterWidget::ParameterWidget(QString name, QString unit, QColor color, QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    m_unit = unit.isEmpty() ? "" : (" " + unit);

    auto* bl = new QHBoxLayout(this);
    bl->setContentsMargins(0, 0, 0, 0);

    m_legend = new ColoredBox(color, this);
    bl->addWidget(m_legend, 0);
    m_legend->setVisible(false);
    auto* label = new QLabel(name);
    bl->addWidget(label, 1);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setMinimumWidth(200);
    m_valueLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    bl->addWidget(m_valueLabel);

    m_minLabel = new QLabel(this);
    m_minLabel->setMinimumWidth(150);
    m_minLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    bl->addWidget(m_minLabel, 0);

    m_maxLabel = new QLabel(this);
    m_maxLabel->setMinimumWidth(150);
    m_maxLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    bl->addWidget(m_maxLabel, 0);

    m_averageLabel = new QLabel(this);
    m_averageLabel->setMinimumWidth(150);
    m_averageLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    bl->addWidget(m_averageLabel, 0);
}

void ParameterWidget::setLegendVisibility(bool visible)
{
    m_legend->setVisible(visible);
}

void ParameterWidget::setValues(float value, float minValue, float maxValue, float averageValue)
{
    m_valueLabel->setText(QString::number(value) + m_unit);
    m_minLabel->setText(QString::number(minValue) + m_unit);
    m_maxLabel->setText(QString::number(maxValue) + m_unit);
    m_averageLabel->setText(QString::number(averageValue) + m_unit);
}

ParametersWidget::ParametersWidget(StatsPageModel* model, QWidget* parent)
    : m_model(model)
    , QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    setContentsMargins(0, 0, 0, 0);
    auto* l = new QVBoxLayout(this);
    auto* mainDataLayout = new QVBoxLayout();

    m_parametersLayout = new QVBoxLayout();
    mainDataLayout->addLayout(m_parametersLayout);
    mainDataLayout->addStretch(1);

    l->addLayout(mainDataLayout, 1);

    m_graph = new SimpleGraph(this);
    m_graph->setVisible(m_isSelected);
    l->addWidget(m_graph, 1);

    setGraphPerWindow(false);
}

void ParametersWidget::focusInEvent(QFocusEvent* /*event*/)
{
    Q_EMIT onFocus(true);
}

void ParametersWidget::focusOutEvent(QFocusEvent* /*event*/)
{
    Q_EMIT onFocus(false);
}

void ParametersWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    if (m_isSelected)
    {
        QStylePainter p(this);
        p.setPen(QPen(palette().highlight(), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect().adjusted(1, 1, -2, -2));
    }
}

void ParametersWidget::setSelected(bool selected)
{
    if (m_isSelected != selected)
    {
        m_isSelected = selected;
        m_graph->setVisible(m_isSelected);
        for (auto&& w : m_parameters)
        {
            w->setLegendVisibility(m_isSelected);
        }
    }
}

void ParametersWidget::addParameter(int index)
{
    m_indices.push_back(index);
    auto& info = m_model->getPlotInfo(index);
    auto* parameter = new ParameterWidget(info.m_name, info.m_units, info.m_color, this);
    m_parameters.push_back(parameter);
    m_graph->addPlot(info);

    m_parametersLayout->addWidget(parameter);
    updateUi();
}

void ParametersWidget::setGraphPerWindow(bool perWindow)
{
    m_graph->setXLabel(perWindow ? tr("seconds") : tr("frames"));
    m_graph->setXZoom(perWindow ? 6 : 2);
    m_graphPerWindow = perWindow;
}

void ParametersWidget::updateUi()
{
    MinValue<double> minValue;
    MaxValue<double> maxValue;
    AvgMinMaxValue<float> stats;
    for (int i = 0; i < m_indices.size(); ++i)
    {
        const int idx = m_indices[i];

        m_model->getGraphData(idx, m_graphPerWindow, m_graph->accessPlotData(i), stats);

        auto& info = m_model->getPlotInfo(idx);
        minValue.addValue(info.m_minValue.value_or(stats.m_min.m_value));
        maxValue.addValue(info.m_maxValue.value_or(stats.m_max.m_value));

        float value = m_model->getParameter(idx);
        m_parameters[i]->setValues(value, stats.m_min.hasValue() ? stats.m_min.m_value : 0, stats.m_max.hasValue() ? stats.m_max.m_value : 0, stats.getAverage());
    }
    m_graph->setMinMax(minValue.m_value, maxValue.m_value);
    m_graph->update();
}

std::vector<QPointF>& SimpleGraph::accessPlotData(int index)
{
    return m_data[index];
}

StatsPageView::StatsPageView(StatsPageModel* statsPageModel)
    : m_model(statsPageModel)
{
    setContentsMargins(0, 0, 0, 0);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* buttonsLayout = new QHBoxLayout();

    auto* showButton = new FlatButton({}, this);
    buttonsLayout->addWidget(showButton);

    auto* perSecond = new FlatButton(tr("Per second stats"), this);
    buttonsLayout->addWidget(perSecond);

    mainLayout->addLayout(buttonsLayout);

    auto* statsPanel = new VerticalScrollArea(this);

    auto* headerLayout = new QHBoxLayout();
    {
        headerLayout->addWidget(new QLabel(tr("Parameter Name")), 1);

        auto* l = new QLabel(tr("Value"), this);
        l->setFixedWidth(200);
        l->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        headerLayout->addWidget(l, 0);

        l = new QLabel(tr("Minimum"), this);
        l->setFixedWidth(150);
        l->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        headerLayout->addWidget(l, 0);

        l = new QLabel(tr("Maximum"), this);
        l->setFixedWidth(150);
        l->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        headerLayout->addWidget(l, 0);

        l = new QLabel(tr("Average"), this);
        l->setFixedWidth(150);
        l->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        headerLayout->addWidget(l, 0);
    }
    mainLayout->addLayout(headerLayout);

    {
        auto* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(separator);
    }

    {
        auto* l = statsPanel->getContentLayout();

        ParametersWidget* widget = nullptr;
        for (int i = 0; i < m_model->getParameterCount(); ++i)
        {
            if (!widget)
            {
                const int idx = m_graphs.size();
                widget = new ParametersWidget(m_model, statsPanel);
                l->addWidget(widget);
                m_graphs.push_back(widget);

                connect(widget, &ParametersWidget::onFocus, [this, idx](bool focused) {
                    if (focused)
                    {
                        setSelectedGraph(idx);
                    }
                });
            }

            widget->addParameter(i);
            if (m_model->getPlotInfo(i).m_endGroup)
            {
                widget = nullptr;
            }
        }

        connect(m_model, &StatsPageModel::valuesChanged, this, [this]() { updateUi(); });

        updateUi();
    }

    mainLayout->addWidget(statsPanel);


    mainLayout->addWidget(statsPanel);
    auto toggleCollection = [this, showButton, statsPanel](bool checked) {
        if (checked)
        {
            m_model->startCollecting();
            showButton->setText(tr("Stop statistics collection"));
        }
        else
        {
            m_model->stopCollecting();
            showButton->setText(tr("Start statistics collection"));
        }
    };
    showButton->setCheckable(true);
    showButton->setChecked(false);
    connect(showButton, &FlatButton::toggled, this, toggleCollection);
    toggleCollection(false);

    auto togglePerSecond = [this, statsPanel](bool checked) {
        for (auto&& graph : m_graphs)
        {
            graph->setGraphPerWindow(checked);
        }
        updateUi();
    };
    perSecond->setCheckable(true);
    perSecond->setChecked(false);
    connect(perSecond, &FlatButton::toggled, this, togglePerSecond);
    togglePerSecond(false);

    setSelectedGraph(0);
}

StatsPageView::~StatsPageView()
{
}

void StatsPageView::updateUi()
{
    for (auto&& graph : m_graphs)
    {
        graph->updateUi();
    }
}

void StatsPageView::setSelectedGraph(int idx)
{
    if (m_selectedGraph != idx)
    {
        if (m_selectedGraph >= 0)
        {
            m_graphs[m_selectedGraph]->setSelected(false);
        }

        m_selectedGraph = idx;

        if (m_selectedGraph >= 0)
        {
            m_graphs[m_selectedGraph]->setSelected(true);
        }
    }
}
