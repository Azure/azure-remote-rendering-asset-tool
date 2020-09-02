#include <QFormLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <QVBoxLayout>
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

void SimpleGraph::paintEvent(QPaintEvent* e)
{
    QStylePainter p(this);
    p.fillRect(e->rect(), Qt::black);
    p.translate(QPoint(width(), height()));
    p.scale(-2, -height());
    p.translate(1, -m_minimum);
    qreal yrange = m_maximum - m_minimum;
    if (yrange < 0.001)
    {
        yrange = 1;
    }
    p.scale(1, 1.0 / yrange);

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
