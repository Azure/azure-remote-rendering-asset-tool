#include <QPaintEvent>
#include <QStylePainter>
#include <QTransform>
#include <QtMath>
#include <View/ArrtStyle.h>
#include <View/ModelEditor/Stats/StatsGraph.h>
#include <ViewUtils/DpiUtils.h>
#include <ViewUtils/Formatter.h>

StatsGraph::StatsGraph(QWidget* parent)
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

    // This function converts a number to a human readable string
    QString toHumanReadableString(double d)
    {
        // Print large numbers in a humand-friendly way. We add a space
        // at the end so numbers with units don't look weird, e.g. "1M ms"
        QString suffix;
        if (d > 1000000)
        {
            d /= 1000000;
            suffix = "M ";
        }
        else if (d > 1000)
        {
            d /= 1000;
            suffix = "K ";
        }
        return DoubleFormatter::toString(d, "%.2f", true) + suffix;
    }

    QString toString(double d)
    {
        return DoubleFormatter::toString(d, "%.2f", true);
    }
} // namespace

QRect StatsGraph::getGraphRect() const
{
    QFontMetrics smallFontM(ArrtStyle::s_graphFont, this);

    const float unit = smallFontM.height();
    return rect().adjusted(unit * 3, unit, -unit * 3, -unit * 2);
}

void StatsGraph::updateTransformAndGrid()
{
    if (m_transformAndGridComputed)
    {
        return;
    }
    m_transformAndGridComputed = true;

    float yMin, yMax;
    float xMin, xMax;

    QRect graphRect = getGraphRect();
    findScale(graphRect.height(), DpiUtils::size(40), m_minimum, m_maximum, yMin, yMax, m_yStep);
    findScale(graphRect.width(), DpiUtils::size(100), 0, graphRect.width() / m_xZoom, xMin, xMax, m_xStep);

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

void StatsGraph::paintEvent(QPaintEvent* e)
{
    updateTransformAndGrid();

    QStylePainter p(this);
    p.fillRect(e->rect(), ArrtStyle::s_graphBackgroundColor);
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

    QPen linePen(ArrtStyle::s_graphForegroundColor, 0);
    QPen scaleLinesPen(ArrtStyle::s_graphLinesColor, 0);
    QPen textPen(ArrtStyle::s_graphTextColor, 0);

    QFontMetrics smallFontM(ArrtStyle::s_graphFont, this);
    const float unit = smallFontM.height();

    p.setFont(ArrtStyle::s_graphFont);

    if (m_yStep > 0)
    {
        for (qreal y = yMin; y < yMax + m_yStep / 2; y += m_yStep)
        {
            QPoint pt;
            pt = m_currentTransform.map(QPoint(0, y));

            p.setPen(scaleLinesPen);
            p.drawLine(graphRect.left(), pt.y(), graphRect.right(), pt.y());
            QString toPrint = toHumanReadableString(y) + m_infos[0].m_units;
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
        const int legendColorSize = DpiUtils::size(8);
        const int padding = DpiUtils::size(6);
        const int externalMargin = DpiUtils::size(7);
        const int contentMargin = DpiUtils::size(6);

        const int rowHeight = smallFontM.height();
        int textWidth = 0;
        int textHeight = 0;

        for (const HighlightPoint& pt : m_highlightedPoints)
        {
            const int rectSize = 8;
            QPoint pos = m_currentTransform.map(pt.m_pt).toPoint();
            p.setPen(textPen);
            p.drawRect(pos.x() - rectSize / 2, pos.y() - rectSize / 2, rectSize, rectSize);
            textWidth = qMax(textWidth, smallFontM.horizontalAdvance(pt.m_text));
            location += pt.m_pt;
            textHeight += rowHeight;
        }

        QRect shrunkVisibleArea = graphRect.marginsRemoved(QMargins() + 1);
        // draw the "tooltip" with the value
        location /= m_highlightedPoints.size();
        QPoint locationOnScreen = m_currentTransform.map(location).toPoint();
        // clamp location in the visible area
        locationOnScreen = QPoint(std::clamp(locationOnScreen.x(), shrunkVisibleArea.left(), shrunkVisibleArea.right()), std::clamp(locationOnScreen.y(), shrunkVisibleArea.top(), shrunkVisibleArea.bottom()));

        QRect tooltipBr = QRect(0, 0, externalMargin * 2 + textWidth + padding + legendColorSize + contentMargin * 2, externalMargin * 2 + textHeight + contentMargin * 2);
        QPoint locInPixel = m_currentTransform.map(location).toPoint();
        tooltipBr.moveTopLeft(locInPixel);

        // flip the rectangle position if it hits the borders
        if (tooltipBr.bottom() > shrunkVisibleArea.bottom())
        {
            tooltipBr.moveBottom(tooltipBr.top());
        }
        if (tooltipBr.right() > shrunkVisibleArea.right())
        {
            tooltipBr.moveRight(tooltipBr.left());
        }
        tooltipBr.moveTop(qMax(tooltipBr.top(), shrunkVisibleArea.top()));
        tooltipBr.moveBottom(qMin(tooltipBr.bottom(), shrunkVisibleArea.bottom()));

        p.setPen(linePen);
        p.setBrush(ArrtStyle::s_graphTooltipBackgroundColor);
        QRect content = tooltipBr.marginsRemoved(QMargins() + externalMargin);
        p.drawRect(content);
        int yOrigin = content.top() + contentMargin;
        int xOrigin = content.left() + contentMargin;
        for (const HighlightPoint& pt : m_highlightedPoints)
        {
            p.setBrush(pt.m_color);
            p.setPen(Qt::NoPen);

            int x = xOrigin;
            p.drawRect(x, yOrigin + rowHeight / 2 - legendColorSize / 2, legendColorSize, legendColorSize);
            x += legendColorSize + padding;
            p.setPen(textPen);
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
        p.setBrush(ArrtStyle::s_graphTooltipBackgroundColor);
        p.drawRect(r.adjusted(-10, 0, 10, 0));
        p.setPen(textPen);
        p.drawText(r, text);
    }

    p.setPen(QPen(ArrtStyle::s_graphForegroundColor, 2));
    p.drawLine(graphRect.bottomLeft(), graphRect.bottomRight());
    p.drawLine(graphRect.bottomLeft(), graphRect.topLeft());

    {
        int w = smallFontM.horizontalAdvance(m_xLabelText);
        p.drawText((graphRect.right() - graphRect.left()) / 2 - w / 2, height() - unit * 0.2, m_xLabelText);
    }
}

int StatsGraph::getPlotCount() const
{
    return (int)m_infos.size();
}

void StatsGraph::setMinMax(qreal minimum, qreal maximum)
{
    m_minimum = minimum;
    m_maximum = maximum;
    m_transformAndGridComputed = false;
    update();
}

void StatsGraph::setXLabel(QString text)
{
    m_xLabelText = text;
}

void StatsGraph::setXZoom(float xZoom)
{
    m_xZoom = xZoom;
}

void StatsGraph::mouseMoveEvent(QMouseEvent* event)
{
    setHighlightX(event->pos().x());
}

void StatsGraph::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    setHighlightX({});
}

void StatsGraph::setHighlightX(std::optional<float> x)
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
                        m_highlightedPoints.push_back({m_infos[valIdx].m_color, QString("%1: %2%3").arg(m_infos[valIdx].m_name).arg(toHumanReadableString(chosen.y())).arg(m_infos[valIdx].m_units), chosen});
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

int StatsGraph::addPlot(StatsPageModel::PlotInfo type)
{
    const int ret = (int)m_infos.size();
    m_infos.push_back(std::move(type));
    m_data.push_back({});
    update();
    return ret;
}

void StatsGraph::setPlotData(int index, std::vector<QPointF> plotData)
{
    m_data[index].swap(plotData);
    m_transformAndGridComputed = false;
    setHighlightX(m_highlightedXInPixels);
    update();
}
