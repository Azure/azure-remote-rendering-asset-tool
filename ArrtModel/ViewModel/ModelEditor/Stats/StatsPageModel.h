#pragma once
#include <Model/ARRServiceStats.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QColor>
#include <optional>

class ArrSessionManager;

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

    StatsPageModel(ArrServiceStats* serviceStats, ArrSessionManager* sessionManager, QObject* parent = nullptr);

    void startCollecting();
    void stopCollecting();
    bool isCollecting() const;

    void startAutoCollect();
    void stopAutoCollect();
    bool isAutoCollecting() const;
    QString getAutoCollectText() const;

    int getParameterCount() const;
    const PlotInfo& getPlotInfo(int index) const;
    double getParameter(int index) const;
    void getParameterStats(int index, float& minValue, float& maxValue, float& averageValue);
    void getGraphData(int index, bool perWindow, std::vector<QPointF>& graph, AvgMinMaxValue<float>& globalStats) const;

Q_SIGNALS:
    void valuesChanged();
    void collectingStateChanged();
    void autoCollectStateChanged();

private:
    ArrServiceStats* const m_serviceStats;
    ArrSessionManager* const m_sessionManager;
    ArrServiceStats::Stats m_stats;
    static PlotInfo m_plotInfo[];
    QTimer* m_autoCollectUpdateTimer = {};
    int m_autoCollectRemainingSeconds = 0;
};
