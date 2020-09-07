#include <Model/ArrSessionManager.h>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>

StatsPageModel::PlotInfo StatsPageModel::m_plotInfo[] = {
    {"Polygons rendered", Qt::white, "", true, 0, {}},

    {"Latency (pose to receive average)", Qt::yellow, " ms", false, 0, {}},
    {"Latency (receive to present average)", Qt::cyan, " ms", false, 0, {}},
    {"Latency (present to display average)", Qt::magenta, " ms", true, 0, {}},

    {"Time since last present", Qt::white, " ms", true, 0, {}},

    {"Frames received", Qt::green, "", true, 0, {}},

    {"Frames reused", Qt::cyan, "", false, 0, {}},
    {"Frames skipped", Qt::yellow, "", false, 0, {}},
    {"Frames discarded", Qt::red, "", true, 0, {}},

    {"Frame minimum delta", Qt::yellow, QString(" ms"), false, 0, {}},
    {"Frame maximum delta", Qt::white, QString(" ms"), true, 0, {}},

    {"Network round-trip", Qt::white, QString(" ms"), true, 0, {}},

    {"Frame time CPU", Qt::yellow, QString(" ms"), false, 0, {}},
    {"Frame time GPU", Qt::magenta, QString(" ms"), true, 0, {}},

    {"Utilization CPU", Qt::yellow, QString(" %"), false, 0, 100},
    {"Utilization GPU", Qt::magenta, QString(" %"), true, 0, 100},

    {"Memory CPU", Qt::yellow, QString(" %"), false, 0, 100},
    {"Memory GPU", Qt::magenta, QString(" %"), true, 0, 100}};


StatsPageModel::StatsPageModel(ArrServiceStats* serviceStats, QObject* parent)
    : QObject(parent)
    , m_serviceStats(serviceStats)
{
    m_serviceStats->startCollecting();
    connect(m_serviceStats, &ArrServiceStats::updated, this,
            [this]() {
                m_stats = m_serviceStats->getStats();
                Q_EMIT valuesChanged();
            });
}

void StatsPageModel::startCollecting()
{
    m_serviceStats->startCollecting();
}

void StatsPageModel::stopCollecting()
{
    m_serviceStats->stopCollecting();
}

int StatsPageModel::getParameterCount() const
{
    return sizeof(m_plotInfo) / sizeof(m_plotInfo[0]);
}


const StatsPageModel::PlotInfo& StatsPageModel::getPlotInfo(int index) const
{
    return m_plotInfo[index];
}

double StatsPageModel::getParameter(int index) const
{
    switch (index)
    {
        case 0:
            return m_stats.m_polygonsRendered.m_perWindowStats.m_value;
        case 1:
            return m_stats.m_latencyPoseToReceive.m_perWindowStats.m_value;
        case 2:
            return m_stats.m_latencyReceiveToPresent.m_perWindowStats.m_value;
        case 3:
            return m_stats.m_latencyPresentToDisplay.m_perWindowStats.m_value;
        case 4:
            return m_stats.m_timeSinceLastPresent.m_perWindowStats.m_value;
        case 5:
            return m_stats.m_videoFramesReceived.m_perWindowStats.m_value;
        case 6:
            return m_stats.m_videoFramesReused.m_perWindowStats.m_value;
        case 7:
            return m_stats.m_videoFramesSkipped.m_perWindowStats.m_value;
        case 8:
            return m_stats.m_videoFramesDiscarded.m_perWindowStats.m_value;
        case 9:
            return m_stats.m_videoFrameMinDelta.m_perWindowStats.m_value;
        case 10:
            return m_stats.m_videoFrameMaxDelta.m_perWindowStats.m_value;
        case 11:
            return m_stats.m_networkLatency.m_perWindowStats.m_value;
        case 12:
            return m_stats.m_timeCPU.m_perWindowStats.m_value;
        case 13:
            return m_stats.m_timeGPU.m_perWindowStats.m_value;
        case 14:
            return m_stats.m_utilizationCPU.m_perWindowStats.m_value;
        case 15:
            return m_stats.m_utilizationGPU.m_perWindowStats.m_value;
        case 16:
            return m_stats.m_memoryCPU.m_perWindowStats.m_value;
        case 17:
            return m_stats.m_memoryGPU.m_perWindowStats.m_value;
        default:
            return 0;
    }
}

#define GRAPH_DATA(PARAM)                           \
    m_stats.##PARAM.getGraphData(graph, perWindow); \
    globalStats = m_stats.##PARAM.m_perWindowStats; \
    break

void StatsPageModel::getGraphData(int index, bool perWindow, std::vector<QPointF>& graph, AvgMinMaxValue<float>& globalStats) const
{
    switch (index)
    {
        case 0:
            GRAPH_DATA(m_polygonsRendered);
        case 1:
            GRAPH_DATA(m_latencyPoseToReceive);
        case 2:
            GRAPH_DATA(m_latencyReceiveToPresent);
        case 3:
            GRAPH_DATA(m_latencyPresentToDisplay);
        case 4:
            GRAPH_DATA(m_timeSinceLastPresent);
        case 5:
            GRAPH_DATA(m_videoFramesReceived);
        case 6:
            GRAPH_DATA(m_videoFramesReused);
        case 7:
            GRAPH_DATA(m_videoFramesSkipped);
        case 8:
            GRAPH_DATA(m_videoFramesDiscarded);
        case 9:
            GRAPH_DATA(m_videoFrameMinDelta);
        case 10:
            GRAPH_DATA(m_videoFrameMaxDelta);
        case 11:
            GRAPH_DATA(m_networkLatency);
        case 12:
            GRAPH_DATA(m_timeCPU);
        case 13:
            GRAPH_DATA(m_timeGPU);
        case 14:
            GRAPH_DATA(m_utilizationCPU);
        case 15:
            GRAPH_DATA(m_utilizationGPU);
        case 16:
            GRAPH_DATA(m_memoryCPU);
        case 17:
            GRAPH_DATA(m_memoryGPU);
    }
}
