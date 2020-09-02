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
        for (auto&& w : m_legendColors)
        {
            w->setVisible(m_isSelected);
        }
    }
}

void ParametersWidget::addParameter(int index)
{
    m_indices.push_back(index);
    auto& info = m_model->getPlotInfo(index);
    auto* bl = new QHBoxLayout();

    auto* coloredBox = new ColoredBox(info.m_color, this);
    m_legendColors.push_back(coloredBox);
    bl->addWidget(coloredBox);
    coloredBox->setVisible(m_isSelected);
    auto* label = new QLabel(info.m_name);

    bl->addWidget(label);
    auto* value = new QLabel(this);
    value->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_values.push_back(value);
    bl->addWidget(m_values.back());

    m_graph->addPlot(info);

    m_parametersLayout->addLayout(bl);
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
        QString toPrint = QString::number(m_model->getParameter(idx));
        const auto& info = m_model->getPlotInfo(idx);

        const QString& unit = info.m_units;
        if (!unit.isEmpty())
        {
            toPrint += " " + unit;
        }
        m_values[i]->setText(toPrint);

        m_model->getGraphData(idx, m_graphPerWindow, m_graph->accessPlotData(i), stats);
        minValue.addValue(info.m_minValue.value_or(stats.m_min.m_value));
        maxValue.addValue(info.m_maxValue.value_or(stats.m_max.m_value));
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
