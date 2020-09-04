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

SimpleGraph::SimpleGraph(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumHeight(250);
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

    QString toString(double d)
    {
        QString s;
        if (qFloor(d) == d)
        {
            s.sprintf("%.0f", d);
        }
        else
        {
            s.sprintf("%.2f", d);
        }
        return s;
    }

} // namespace

QRect SimpleGraph::getGraphRect() const
{
    QFont smallFont("Segoe UI", 8);
    QFontMetrics smallFontM(smallFont, this);

    const float unit = smallFontM.height();
    return rect().adjusted(unit * 3, unit, -unit * 3, -unit * 2);
}

void SimpleGraph::updateTransformAndGrid()
{
    if (m_transformAndGridComputed)
    {
        return;
    }
    m_transformAndGridComputed = true;

    float yMin, yMax;
    float xMin, xMax;

    QRect graphRect = getGraphRect();
    findScale(graphRect.height(), 40, m_minimum, m_maximum, yMin, yMax, m_yStep);
    findScale(graphRect.width(), 100, 0, graphRect.width() / m_xZoom, xMin, xMax, m_xStep);

    m_currentTransform.reset();
    m_currentTransform.translate(graphRect.left(), graphRect.bottom());
    m_currentTransform.scale(graphRect.width(), -graphRect.height());
    m_currentTransform.translate(0, -yMin);

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
    m_currentTransform.scale(1.0 / xrange, 1.0 / yrange);
    m_currentTransformInverse = m_currentTransform.inverted();

    {
        const auto& pd = m_data[0];
        if (pd.size() > 0)
        {
            qreal lastX = pd.front().x();
            QPointF pt = m_currentTransform.map(QPointF(lastX, 0));
            if (pt.x() > graphRect.right())
            {
                float dX = m_currentTransformInverse.map(QPointF(graphRect.right(), 0)).x() - m_currentTransformInverse.map(pt).x();
                m_currentTransform.translate(dX, 0);
                m_currentTransformInverse = m_currentTransform.inverted();
            }
        }
    }
}

void SimpleGraph::paintEvent(QPaintEvent* e)
{
    updateTransformAndGrid();

    QStylePainter p(this);
    p.fillRect(e->rect(), Qt::black);
    QRect graphRect = getGraphRect();

    float yMin = 0, yMax = 0;
    float xMin = 0, xMax = 0;

    if (m_xStep > 0)
    {
        xMin = m_currentTransformInverse.map(QPointF(graphRect.left(), 0)).x();
        xMin = qCeil(xMin / m_xStep) * m_xStep;
        xMax = m_currentTransformInverse.map(QPointF(graphRect.right(), 0)).x();
        xMax = qFloor(xMax / m_xStep) * m_xStep;
    }

    if (m_yStep > 0)
    {
        yMin = m_currentTransformInverse.map(QPointF(0, graphRect.bottom())).y();
        yMin = qCeil(yMin / m_yStep) * m_yStep;
        yMax = m_currentTransformInverse.map(QPointF(0, graphRect.top())).y();
        // yMax uses qCeil because we assume we want to include the last step, which should be always on the top of the graph
        yMax = qCeil(yMax / m_yStep) * m_yStep;
    }

    QColor linesColor = palette().mid().color();
    QPen linePen(linesColor, 0);

    QColor scaleLinesColor(70, 70, 70);
    QPen scaleLinesPen(scaleLinesColor, 0);

    QColor foregroundColor = palette().text().color();
    QPen foregroundPen(foregroundColor, 0);

    QFont smallFont("Segoe UI", 8);
    QFontMetrics smallFontM(smallFont, this);
    const float unit = smallFontM.height();

    p.setFont(smallFont);

    if (m_yStep > 0)
    {
        for (qreal y = yMin; y < yMax + m_yStep / 2; y += m_yStep)
        {
            QPoint pt;
            pt = m_currentTransform.map(QPoint(0, y));

            p.setPen(scaleLinesPen);
            p.drawLine(graphRect.left(), pt.y(), graphRect.right(), pt.y());
            QString toPrint = toString(y) + m_infos[0].m_units;
            int w = smallFontM.horizontalAdvance(toPrint);

            p.setPen(linePen);
            p.drawText(graphRect.left() - w - unit * 0.3, pt.y(), toPrint);
        }
    }
    if (m_xStep > 0)
    {
        for (qreal x = xMin; x <= xMax; x += m_xStep)
        {
            QPoint pt;
            pt = m_currentTransform.map(QPoint(x, 0));

            p.setPen(scaleLinesPen);
            p.drawLine(pt.x(), graphRect.top(), pt.x(), graphRect.bottom());
            QString toPrint = toString(x);
            int w = smallFontM.horizontalAdvance(toPrint);

            p.setPen(linePen);
            p.drawText(pt.x() - w / 2, graphRect.bottom() + smallFontM.height(), toPrint);
        }

        p.setPen(scaleLinesPen);
        p.drawLine(graphRect.topRight(), graphRect.bottomRight());
    }

    p.save();
    p.setClipRect(graphRect.adjusted(-1, -1, 1, 1));

    p.setBrush(Qt::NoBrush);

    for (int i = 0; i < getPlotCount(); ++i)
    {
        const auto& pd = m_data[i];
        if (pd.size() > 0)
        {
            p.setPen(QPen(m_infos[i].m_color, 1));
            float leftX = m_currentTransformInverse.map(QPointF(graphRect.left(), 0)).x();
            //assume the points are organized right to left
            QPolygonF polygon;
            size_t pointsToDraw = 0;
            while (pointsToDraw < pd.size() && pd[pointsToDraw].x() > leftX)
            {
                polygon.append(m_currentTransform.map(pd[pointsToDraw]));
                ++pointsToDraw;
            }
            if (++pointsToDraw < pd.size())
            {
                polygon.append(m_currentTransform.map(pd[pointsToDraw]));
            }
            p.drawPolyline(polygon);
        }
    }

    if (m_highlightedPoints.size() > 0)
    {
        //QString toDisplay;
        QPointF location;
        const int rowHeight = smallFontM.height();
        int textWidth = 0;
        int textHeight = 0;
        int legendColorSize = 8;
        int padding = 8;

        for (const HighlightPoint& pt : m_highlightedPoints)
        {
            const int rectSize = 8;
            QPoint pos = m_currentTransform.map(pt.m_pt).toPoint();
            p.setPen(foregroundPen);
            p.drawRect(pos.x() - rectSize / 2, pos.y() - rectSize / 2, rectSize, rectSize);
            textWidth = qMax(textWidth, smallFontM.horizontalAdvance(pt.m_text));
            location += pt.m_pt;
            textHeight += rowHeight;
        }

        QRect shrunkVisibleArea = graphRect.marginsRemoved(QMargins() + 2);
        const int borderWidth = 6;
        const int marginsWidth = 10;
        // draw the "tooltip" with the value
        location /= m_highlightedPoints.size();
        QPoint locationOnScreen = m_currentTransform.map(location).toPoint();
        // clamp location in the visible area
        locationOnScreen = QPoint(std::clamp(locationOnScreen.x(), shrunkVisibleArea.left(), shrunkVisibleArea.right()), std::clamp(locationOnScreen.y(), shrunkVisibleArea.top(), shrunkVisibleArea.bottom()));

        QRect textBr = QRect(0, 0, textWidth + padding + legendColorSize + marginsWidth * 2, textHeight + marginsWidth * 2);
        QRect border = textBr.marginsAdded(QMargins() + borderWidth);
        QRect borderAndMarging = border.marginsAdded(QMargins() + marginsWidth);
        QPoint delta = m_currentTransform.map(location).toPoint() - borderAndMarging.topLeft();

        // flip the rectangle position if it hits the borders
        if (borderAndMarging.bottom() + delta.y() > shrunkVisibleArea.bottom())
        {
            delta.setY(delta.y() - borderAndMarging.height());
        }
        if (borderAndMarging.right() + delta.x() > shrunkVisibleArea.right())
        {
            delta.setX(delta.x() - borderAndMarging.width());
        }

        border.translate(delta);
        textBr.translate(delta);
        p.setPen(linePen);
        p.setBrush(QColor(0, 0, 0, 200));
        p.drawRect(border);
        int yOrigin = delta.y() + marginsWidth;
        int xOrigin = delta.x() + marginsWidth;
        for (const HighlightPoint& pt : m_highlightedPoints)
        {
            p.setBrush(pt.m_color);
            p.setPen(Qt::NoPen);

            int x = xOrigin;
            p.drawRect(x, yOrigin + rowHeight / 2 - legendColorSize / 2, legendColorSize, legendColorSize);
            x += legendColorSize + padding;
            p.setPen(foregroundColor);
            p.drawText(QRect(x, yOrigin, textWidth, rowHeight), Qt::AlignVCenter, pt.m_text);
            yOrigin += rowHeight;
        }
    }
    p.restore();

    if (m_highlightedX.has_value())
    {
        float highlightedX = m_currentTransform.map(QPointF(*m_highlightedX, 0)).x();
        p.drawLine(QPointF(highlightedX, graphRect.bottom()), QPointF(highlightedX, graphRect.top()));
        QString text = toString(*m_highlightedX);
        QRect r = smallFontM.boundingRect(text);
        r.moveCenter(QPoint(highlightedX, graphRect.bottom()));
        r.moveTop(graphRect.bottom() + unit * 0.2);
        p.setPen(linePen);
        p.setBrush(QColor(0, 0, 0, 200));
        p.drawRect(r.adjusted(-10, 0, 10, 0));
        p.setPen(foregroundPen);
        p.drawText(r, text);
    }

    p.setPen(QPen(linesColor, 2));
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
    m_transformAndGridComputed = false;
    update();
}

void SimpleGraph::setXLabel(QString text)
{
    m_xLabelText = text;
}

void SimpleGraph::setXZoom(float xZoom)
{
    m_xZoom = xZoom;
}

void SimpleGraph::mouseMoveEvent(QMouseEvent* event)
{
    setHighlightX(event->pos().x());
}

void SimpleGraph::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    setHighlightX({});
}

void SimpleGraph::setHighlightX(std::optional<float> x)
{
    updateTransformAndGrid();
    QRect graphRect = getGraphRect();

    if (m_highlightedXInPixels.has_value())
    {
        update();
    }

    m_highlightedXInPixels = x;

    if (m_highlightedXInPixels.has_value())
    {
        const float newX = *m_highlightedXInPixels;
        const float xValue = m_currentTransformInverse.map(QPointF(newX, 0)).x();
        m_highlightedX = xValue;

        //find the points to highlight
        m_highlightedPoints.clear();

        float chosenPointClosestDistanceToCursor = std::numeric_limits<float>::max();

        // simple linear search.
        if (m_data.size() > 0)
        {
            for (int valIdx = 0; valIdx < getPlotCount(); ++valIdx)
            {
                auto& d = m_data[valIdx];
                float previousDistance = std::numeric_limits<float>::max();
                for (int i = 0; i < d.size(); ++i)
                {
                    const auto& pt = d[i];
                    const float dist = pt.x() - xValue;
                    if (dist < 0)
                    {
                        const QPointF& chosen = (previousDistance < -dist) ? d[i - 1] : pt;
                        const float newDistance = qAbs(chosen.x() - xValue);

                        if (chosenPointClosestDistanceToCursor != newDistance)
                        {
                            chosenPointClosestDistanceToCursor = newDistance;
                            m_highlightedPoints.clear();
                            m_highlightedX = chosen.x();
                        }
                        m_highlightedPoints.push_back({m_infos[valIdx].m_color, QString("%1: %2%3").arg(m_infos[valIdx].m_name).arg(toString(chosen.y())).arg(m_infos[valIdx].m_units), chosen});
                        break;
                    }
                    previousDistance = dist;
                }
            }
        }
        update();
    }
    else
    {
        m_highlightedPoints.clear();
        m_highlightedX.reset();
    }
}

int SimpleGraph::addPlot(StatsPageModel::PlotInfo type)
{
    const int ret = (int)m_infos.size();
    m_infos.push_back(std::move(type));
    m_data.push_back({});
    update();
    return ret;
}

void SimpleGraph::setPlotData(int index, std::vector<QPointF> plotData)
{
    m_data[index].swap(plotData);
    m_transformAndGridComputed = false;
    setHighlightX(m_highlightedXInPixels);
    update();
}


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
    m_valueLabel->setText(toString(value) + m_unit);
    m_minLabel->setText(toString(minValue) + m_unit);
    m_maxLabel->setText(toString(maxValue) + m_unit);
    m_averageLabel->setText(toString(averageValue) + m_unit);
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
