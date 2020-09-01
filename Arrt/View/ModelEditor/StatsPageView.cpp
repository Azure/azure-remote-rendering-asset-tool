#include <QFormLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <QVBoxLayout>
#include <View/ModelEditor/StatsPageView.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>
#include <ViewUtils/DpiUtils.h>

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
    setMinimumHeight(80);
}

void SimpleGraph::paintEvent(QPaintEvent* e)
{
    QStylePainter p(this);
    p.fillRect(e->rect(), Qt::black);
    p.translate(QPoint(width(), height()));
    p.scale(-1, -height());
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
    auto* l = new QHBoxLayout(this);
    auto* mainDataLayout = new QVBoxLayout();

    m_parametersLayout = new QVBoxLayout();
    mainDataLayout->addLayout(m_parametersLayout);
    mainDataLayout->addStretch(1);

    l->addLayout(mainDataLayout, 1);

    m_graph = new SimpleGraph(this);
    l->addWidget(m_graph, 1);
}

void ParametersWidget::addParameter(int index)
{
    m_indices.push_back(index);
    auto& info = m_model->getPlotInfo(index);
    auto* bl = new QHBoxLayout();

    auto* coloredBox = new ColoredBox(info.m_color, this);
    bl->addWidget(coloredBox);
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

void ParametersWidget::updateUi()
{
    for (int i = 0; i < m_indices.size(); ++i)
    {
        const int idx = m_indices[i];
        QString toPrint = QString::number(m_model->getParameter(idx));
        const QString& unit = m_model->getPlotInfo(idx).m_units;
        if (!unit.isEmpty())
        {
            toPrint += " " + unit;
        }
        m_values[i]->setText(toPrint);

        m_model->getGraphData(idx, m_graph->accessPlotData(i));
    }
    m_graph->update();
}


std::vector<QPointF>& SimpleGraph::accessPlotData(int index)
{
    return m_data[index];
}

StatsPageView::StatsPageView(StatsPageModel* statsPageModel)
    : m_model(statsPageModel)
{
    auto* l = new QVBoxLayout(this);

    ParametersWidget* widget = nullptr;
    for (int i = 0; i < m_model->getParameterCount(); ++i)
    {
        if (!widget)
        {
            widget = new ParametersWidget(m_model, this);
            l->addWidget(widget);
            m_graphs.push_back(widget);
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
