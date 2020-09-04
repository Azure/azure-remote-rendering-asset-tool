#include <QFormLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <QTransform>
#include <QVBoxLayout>
#include <QtMath>
#include <View/ModelEditor/Stats/StatsGraph.h>
#include <View/ModelEditor/Stats/StatsPageView.h>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>
#include <ViewUtils/DpiUtils.h>
#include <ViewUtils/Formatter.h>
#include <Widgets/FlatButton.h>
#include <Widgets/VerticalScrollArea.h>

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

    m_valueLabel = new QLabel(this);
    m_valueLabel->setMinimumWidth(150);
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
    m_valueLabel->setText(DoubleFormatter::toString(value, "%.2f", true) + m_unit);
    m_minLabel->setText(DoubleFormatter::toString(minValue, "%.2f", true) + m_unit);
    m_maxLabel->setText(DoubleFormatter::toString(maxValue, "%.2f", true) + m_unit);
    m_averageLabel->setText(DoubleFormatter::toString(averageValue, "%.2f", true) + m_unit);
}

ParametersWidget::ParametersWidget(StatsPageModel* model, QWidget* parent)
    : m_model(model)
    , QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    setContentsMargins(QMargins() + 8);
    auto* l = new QVBoxLayout(this);
    auto* mainDataLayout = new QVBoxLayout();

    m_parametersLayout = new QVBoxLayout();
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
        p.drawRect(contentsRect());
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

        std::vector<QPointF> plotData;
        m_model->getGraphData(idx, m_graphPerWindow, plotData, stats);
        m_graph->setPlotData(i, std::move(plotData));

        auto& info = m_model->getPlotInfo(idx);
        minValue.addValue(info.m_minValue.value_or(stats.m_min.m_value));
        maxValue.addValue(info.m_maxValue.value_or(stats.m_max.m_value));

        float value = m_model->getParameter(idx);
        m_parameters[i]->setValues(value, stats.m_min.hasValue() ? stats.m_min.m_value : 0, stats.m_max.hasValue() ? stats.m_max.m_value : 0, stats.getAverage());
    }
    m_graph->setMinMax(minValue.m_value, maxValue.m_value);
}


StatsPageView::StatsPageView(StatsPageModel* statsPageModel, QWidget* parent)
    : QWidget(parent)
    , m_model(statsPageModel)
{
    setContentsMargins(QMargins());

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());

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
        l->setFixedWidth(150);
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
