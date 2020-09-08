#pragma once
#include <Model/ARRServiceStats.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QColor>
#include <optional>


// model for the stats panel, holding the statistics per frame and per second

class StatsPageModel : public QObject
{
    Q_OBJECT

public:
    struct PlotInfo
    {
        QString m_name;

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

    void startCollecting();
    void stopCollecting();

    int getParameterCount() const;
    const PlotInfo& getPlotInfo(int index) const;
    double getParameter(int index) const;
    void getParameterStats(int index, float& minValue, float& maxValue, float& averageValue);
    void getGraphData(int index, bool perWindow, std::vector<QPointF>& graph, AvgMinMaxValue<float>& globalStats) const;

Q_SIGNALS:
    void valuesChanged();

private:
    ArrServiceStats* const m_serviceStats;
    ArrServiceStats::Stats m_stats;
    static PlotInfo m_plotInfo[];
};
