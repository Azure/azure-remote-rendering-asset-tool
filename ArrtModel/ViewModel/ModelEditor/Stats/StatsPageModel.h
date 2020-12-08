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
    enum ValueType
    {
        POLYGONS_RENDERED,
        LATENCY_POSE_RECEIVE,
        LATENCY_RECEIVE_PRESENT,
        LATENCY_PRESENT_DISPLAY,
        TIME_SINCE_LAST_PRESENT,
        FRAMES_RECEIVED,
        FRAMES_REUSED,
        FRAMES_SKIPPED,
        FRAMES_DISCARDED,
        FRAME_MINIMUM_DELTA,
        FRAME_MAXIMUM_DELTA,
        NETWORK_ROUNDTRIP,
        FRAME_TIME_CPU,
        FRAME_TIME_GPU,
        UTILIZATION_CPU,
        UTILIZATION_GPU,
        MEMORY_CPU,
        MEMORY_GPU,

        VALUE_TYPE_COUNT
    };

    struct PlotInfo
    {
        QString m_name;

        // plot color
        QColor m_color;

        // units of measure. In a group it will determine if you are using the same scale or not when plotting
        QString m_units;

        std::optional<double> m_minValue;
        std::optional<double> m_maxValue;
    };

    struct PlotGroup
    {
        QString m_name;
        std::vector<ValueType> m_plots;
    };

    StatsPageModel(ArrServiceStats* serviceStats, ArrSessionManager* sessionManager, QObject* parent = nullptr);

    void startCollecting();
    void stopCollecting();
    bool isCollecting() const;

    void startAutoCollect();
    void stopAutoCollect();
    bool isAutoCollecting() const;
    QString getAutoCollectText() const;

    int getPlotGroupCount() const;
    const PlotGroup& getPlotGroup(int index) const;

    const PlotInfo& getPlotInfo(ValueType type) const;
    double getParameter(ValueType type) const;
    void getGraphData(ValueType type, bool perWindow, std::vector<QPointF>& graph, AvgMinMaxValue<float>& globalStats) const;

Q_SIGNALS:
    void valuesChanged();
    void collectingStateChanged();
    void autoCollectStateChanged();

private:
    ArrServiceStats* const m_serviceStats;
    ArrSessionManager* const m_sessionManager;
    ArrServiceStats::Stats m_stats;
    static PlotGroup s_plotGroups[];
    static PlotInfo s_plotInfos[];
    QTimer* m_autoCollectUpdateTimer = {};
    int m_autoCollectRemainingSeconds = 0;
};
