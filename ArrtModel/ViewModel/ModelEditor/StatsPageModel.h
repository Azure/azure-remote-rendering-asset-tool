#pragma once
#include <Model/ARRServiceStats.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QColor>
#include <optional>


// model for the stats panel, holding the statistics per frame

class StatsPageModel : public QObject
{
    Q_OBJECT

public:
    struct PlotInfo
    {
        QString m_name;
        enum GraphType
        {
            AreaLowerBound,
            AreaHigherBound,
            Line
        } m_graphType;

        enum CurveType
        {
            Continuous, //the curve is a polyline between points
            Discrete    // y is constant between the previous x and the current x and equal to the current value y
        } m_curveType;

        // plot color
        QColor m_color;

        // units of measure. In a group it will determine if you are using the same scale or not when plotting
        QString m_units;

        // when false, it means the next plot will be merged with this as a group.
        bool m_endGroup;

        std::optional<double> m_minValue;
        std::optional<double> m_maxValue;
    };


    StatsPageModel(ArrServiceStats* serviceStats, QObject* parent = nullptr);

    int getParameterCount() const;
    const PlotInfo& getPlotInfo(int index) const;
    double getParameter(int index) const;
    void getGraphData(int index, std::vector<QPointF>& graph) const;
    std::optional<RR::PerformanceRating> getParameterRating(int index) const;

Q_SIGNALS:
    void valuesChanged();

private:
    ArrServiceStats* const m_serviceStats;
    ArrServiceStats::Stats m_stats;
    static PlotInfo m_plotInfo[];
};
