#pragma once
#include <QWidget>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>

class StatsGraph : public QWidget
{
public:
    StatsGraph(QWidget* parent = {});

    virtual void paintEvent(QPaintEvent* event) override;
    int getPlotCount() const;

    int addPlot(StatsPageModel::PlotInfo info);
    void setPlotData(int index, std::vector<QPointF> plotData);

    void setMinMax(qreal minimum, qreal maximum);

    void setXLabel(QString text);
    void setXZoom(float zoom);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

    void setHighlightX(std::optional<float> x);
    QRect getGraphRect() const;

    void updateTransformAndGrid();

private:
    std::vector<std::vector<QPointF>> m_data;
    std::vector<StatsPageModel::PlotInfo> m_infos;
    qreal m_minimum;
    qreal m_maximum;
    QString m_xLabelText;
    QTransform m_currentTransform;
    QTransform m_currentTransformInverse;
    float m_xStep;
    float m_yStep;
    bool m_transformAndGridComputed = false;

    std::optional<float> m_highlightedXInPixels = {};
    std::optional<float> m_highlightedX = {};
    struct HighlightPoint
    {
        QColor m_color;
        QString m_text;
        QPointF m_pt;
    };
    std::vector<HighlightPoint> m_highlightedPoints;
    float m_xZoom = 2.0;
};
