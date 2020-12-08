#include <QStylePainter>
#include <QVBoxLayout>
#include <View/ModelEditor/Stats/ParameterWidget.h>
#include <View/ModelEditor/Stats/ParametersWidget.h>
#include <View/ModelEditor/Stats/StatsGraph.h>
#include <ViewUtils/DpiUtils.h>

ParametersWidget::ParametersWidget(StatsPageModel* model, QString name, QWidget* parent)
    : m_model(model)
    , QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    if (!name.isEmpty())
    {
        setAccessibleName(name);
    }

    setContentsMargins({});
    auto* l = new QVBoxLayout(this);
    l->setSpacing(0);
    auto* mainDataLayout = new QVBoxLayout();

    m_parametersLayout = new QVBoxLayout();
    m_parametersLayout->setSpacing(0);
    mainDataLayout->addLayout(m_parametersLayout);
    mainDataLayout->addStretch(1);

    l->addLayout(mainDataLayout, 1);

    m_graph = new StatsGraph(this);
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
        p.drawRect(contentsRect().marginsRemoved(QMargins() + 1));
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

void ParametersWidget::addParameter(StatsPageModel::ValueType type)
{
    m_valueTypes.push_back(type);
    auto& info = m_model->getPlotInfo(type);
    auto* parameter = new ParameterWidget(info.m_name, info.m_units, info.m_color, this);
    m_parameters.push_back(parameter);
    m_graph->addPlot(info);

    m_parametersLayout->addWidget(parameter);

    if (accessibleName().isEmpty() && !info.m_name.isEmpty())
    {
        setAccessibleName(info.m_name);
    }
    updateUi();
}

void ParametersWidget::setGraphPerWindow(bool perWindow)
{
    m_graph->setXLabel(perWindow ? tr("seconds") : tr("frames"));
    m_graph->setXZoom(perWindow ? DpiUtils::size(6) : DpiUtils::size(2));
    m_graphPerWindow = perWindow;
}

void ParametersWidget::updateUi()
{
    MinValue<double> minValue;
    MaxValue<double> maxValue;
    AvgMinMaxValue<float> stats;
    for (int i = 0; i < m_valueTypes.size(); ++i)
    {
        auto valueType = m_valueTypes[i];

        std::vector<QPointF> plotData;
        m_model->getGraphData(valueType, m_graphPerWindow, plotData, stats);
        m_graph->setPlotData(i, std::move(plotData));

        auto& info = m_model->getPlotInfo(valueType);
        minValue.addValue(info.m_minValue.value_or(stats.m_min.m_value));
        maxValue.addValue(info.m_maxValue.value_or(stats.m_max.m_value));

        float value = m_model->getParameter(valueType);
        m_parameters[i]->setValues(value, stats.m_min.hasValue() ? stats.m_min.m_value : 0, stats.m_max.hasValue() ? stats.m_max.m_value : 0, stats.getAverage());
    }
    m_graph->setMinMax(minValue.m_value, maxValue.m_value);
}

void ParametersWidget::setAccessibleName(QString name)
{
    QWidget::setAccessibleName(name + " " + tr("stats"));
}