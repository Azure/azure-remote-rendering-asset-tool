#include <QFormLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QStylePainter>
#include <QVBoxLayout>
#include <View/ModelEditor/StatsPageView.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>

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
    if (m_data.size() > 0)
    {
        p.translate(QPoint(width(), height()));
        p.scale(-1, -height());
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::white, 0));
        p.drawPolyline(m_data.data(), (int)m_data.size());
    }
}

std::vector<QPointF>& SimpleGraph::accessPlotData()
{
    return m_data;
}

StatsPageView::StatsPageView(StatsPageModel* statsPageModel)
    : m_model(statsPageModel)
{
    auto* l = new QVBoxLayout(this);

    connect(m_model, &StatsPageModel::valuesChanged, this, [this]() { updateUi(); });

    auto* fl = new QFormLayout();

    QString name;
    QString units;
    std::optional<double> minValue;
    std::optional<double> maxValue;

    for (int i = 0; i < m_model->getParameterCount(); ++i)
    {
        m_model->getParameterInfo(i, name, units, minValue, maxValue);
        QVBoxLayout* bl = new QVBoxLayout();
        m_values.push_back(new QLabel(this));
        m_graphs.push_back(new SimpleGraph(this));
        bl->addWidget(m_values.back());
        bl->addWidget(m_graphs.back());

        fl->addRow(name, bl);

        m_units.push_back(units);
    }
    l->addLayout(fl);
}

StatsPageView::~StatsPageView()
{
}

void StatsPageView::updateUi()
{
    assert(m_model->getParameterCount() == m_values.size());
    for (int i = 0; i < m_model->getParameterCount(); ++i)
    {
        QString toPrint = QString::number(m_model->getParameter(i));
        if (!m_units[i].isEmpty())
        {
            toPrint += " " + m_units[i];
        }
        m_values[i]->setText(toPrint);

        m_model->getGraphData(i, m_graphs[i]->accessPlotData());
        m_graphs[i]->update();
    }
}
