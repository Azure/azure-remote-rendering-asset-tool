#include <Model/ArrSessionManager.h>
#include <ViewModel/ModelEditor/Stats/StatsPageModel.h>

using namespace std::literals;

StatsPageModel::PlotInfo StatsPageModel::s_plotInfos[] = {
    {"Polygons rendered", Qt::white, "", 0, {}},
    {"Latency (pose to receive average)", Qt::yellow, " ms", 0, {}},
    {"Latency (receive to present average)", Qt::cyan, " ms", 0, {}},
    {"Latency (present to display average)", Qt::magenta, " ms", 0, {}},
    {"Time since last present", Qt::white, " ms", 0, {}},
    {"Frames received", Qt::green, "", 0, {}},
    {"Frames reused", Qt::cyan, "", 0, {}},
    {"Frames skipped", Qt::yellow, "", 0, {}},
    {"Frames discarded", Qt::red, "", 0, {}},
    {"Frame minimum delta", Qt::yellow, QString(" ms"), 0, {}},
    {"Frame maximum delta", Qt::white, QString(" ms"), 0, {}},
    {"Network round-trip", Qt::white, QString(" ms"), 0, {}},
    {"Frame time CPU", Qt::yellow, QString(" ms"), 0, {}},
    {"Frame time GPU", Qt::magenta, QString(" ms"), 0, {}},
    {"Utilization CPU", Qt::yellow, QString(" %"), 0, 100},
    {"Utilization GPU", Qt::magenta, QString(" %"), 0, 100},
    {"Memory CPU", Qt::yellow, QString(" %"), 0, 100},
    {"Memory GPU", Qt::magenta, QString(" %"), 0, 100}};

StatsPageModel::PlotGroup StatsPageModel::s_plotGroups[] = {
    {"Polygon count", {POLYGONS_RENDERED}},
    {"Latency", {LATENCY_POSE_RECEIVE, LATENCY_RECEIVE_PRESENT, LATENCY_PRESENT_DISPLAY}},
    {{}, {TIME_SINCE_LAST_PRESENT}},
    {{}, {FRAMES_RECEIVED}},
    {"Frames information", {FRAMES_REUSED, FRAMES_SKIPPED, FRAMES_DISCARDED}},
    {"Frame delta", {FRAME_MINIMUM_DELTA, FRAME_MAXIMUM_DELTA}},
    {{}, {NETWORK_ROUNDTRIP}},
    {"Frame time", {FRAME_TIME_CPU, FRAME_TIME_GPU}},
    {"Resources", {UTILIZATION_CPU, UTILIZATION_GPU}},
    {"Memory", {MEMORY_CPU, MEMORY_GPU}}};


StatsPageModel::StatsPageModel(ArrServiceStats* serviceStats, ArrSessionManager* sessionManager, QObject* parent)
    : QObject(parent)
    , m_serviceStats(serviceStats)
    , m_sessionManager(sessionManager)
{
    connect(m_serviceStats, &ArrServiceStats::updated, this,
            [this]() {
                m_stats = m_serviceStats->getStats();
                Q_EMIT valuesChanged();
            });

    m_autoCollectUpdateTimer = new QTimer(this);
    connect(m_autoCollectUpdateTimer, &QTimer::timeout, [this]() {
        --m_autoCollectRemainingSeconds;
        if (m_autoCollectRemainingSeconds == 0)
        {
            stopCollecting();
        }
        Q_EMIT autoCollectStateChanged();
    });
}

void StatsPageModel::startCollecting()
{
    m_serviceStats->startCollecting();
    Q_EMIT collectingStateChanged();
}

void StatsPageModel::stopCollecting()
{
    m_serviceStats->stopCollecting();
    m_autoCollectUpdateTimer->stop();
    m_autoCollectRemainingSeconds = 0;
    Q_EMIT collectingStateChanged();
    Q_EMIT autoCollectStateChanged();
}

bool StatsPageModel::isCollecting() const
{
    return m_serviceStats->isCollecting();
}

void StatsPageModel::startAutoCollect()
{
    m_autoCollectRemainingSeconds = 15;
    m_autoCollectUpdateTimer->start(1s);
    m_sessionManager->setAutoRotateRoot(true);
    startCollecting();
    Q_EMIT autoCollectStateChanged();
}

void StatsPageModel::stopAutoCollect()
{
    m_sessionManager->setAutoRotateRoot(false);
    stopCollecting();
}

bool StatsPageModel::isAutoCollecting() const
{
    return m_autoCollectUpdateTimer->isActive();
}

QString StatsPageModel::getAutoCollectText() const
{
    if (!isAutoCollecting())
    {
        return tr("Start auto-collecting");
    }
    else
    {
        return tr("Collecting: remaining %1 seconds").arg(m_autoCollectRemainingSeconds);
    }
}

int StatsPageModel::getPlotGroupCount() const
{
    return sizeof(s_plotGroups) / sizeof(s_plotGroups[0]);
}

const StatsPageModel::PlotGroup& StatsPageModel::getPlotGroup(int index) const
{
    return s_plotGroups[index];
}

const StatsPageModel::PlotInfo& StatsPageModel::getPlotInfo(ValueType type) const
{
    return s_plotInfos[type];
}

double StatsPageModel::getParameter(ValueType type) const
{
    switch (type)
    {
        case POLYGONS_RENDERED:
            return m_stats.m_polygonsRendered.m_perWindowStats.m_value;
        case LATENCY_POSE_RECEIVE:
            return m_stats.m_latencyPoseToReceive.m_perWindowStats.m_value;
        case LATENCY_RECEIVE_PRESENT:
            return m_stats.m_latencyReceiveToPresent.m_perWindowStats.m_value;
        case LATENCY_PRESENT_DISPLAY:
            return m_stats.m_latencyPresentToDisplay.m_perWindowStats.m_value;
        case TIME_SINCE_LAST_PRESENT:
            return m_stats.m_timeSinceLastPresent.m_perWindowStats.m_value;
        case FRAMES_RECEIVED:
            return m_stats.m_videoFramesReceived.m_perWindowStats.m_value;
        case FRAMES_REUSED:
            return m_stats.m_videoFramesReused.m_perWindowStats.m_value;
        case FRAMES_SKIPPED:
            return m_stats.m_videoFramesSkipped.m_perWindowStats.m_value;
        case FRAMES_DISCARDED:
            return m_stats.m_videoFramesDiscarded.m_perWindowStats.m_value;
        case FRAME_MINIMUM_DELTA:
            return m_stats.m_videoFrameMinDelta.m_perWindowStats.m_value;
        case FRAME_MAXIMUM_DELTA:
            return m_stats.m_videoFrameMaxDelta.m_perWindowStats.m_value;
        case NETWORK_ROUNDTRIP:
            return m_stats.m_networkLatency.m_perWindowStats.m_value;
        case FRAME_TIME_CPU:
            return m_stats.m_timeCPU.m_perWindowStats.m_value;
        case FRAME_TIME_GPU:
            return m_stats.m_timeGPU.m_perWindowStats.m_value;
        case UTILIZATION_CPU:
            return m_stats.m_utilizationCPU.m_perWindowStats.m_value;
        case UTILIZATION_GPU:
            return m_stats.m_utilizationGPU.m_perWindowStats.m_value;
        case MEMORY_CPU:
            return m_stats.m_memoryCPU.m_perWindowStats.m_value;
        case MEMORY_GPU:
            return m_stats.m_memoryGPU.m_perWindowStats.m_value;
        default:
            return 0;
    }
}

#define GRAPH_DATA(PARAM)                           \
    m_stats.##PARAM.getGraphData(graph, perWindow); \
    globalStats = m_stats.##PARAM.m_perWindowStats; \
    break

void StatsPageModel::getGraphData(ValueType type, bool perWindow, std::vector<QPointF>& graph, AvgMinMaxValue<float>& globalStats) const
{
    switch (type)
    {
        case POLYGONS_RENDERED:
            GRAPH_DATA(m_polygonsRendered);
        case LATENCY_POSE_RECEIVE:
            GRAPH_DATA(m_latencyPoseToReceive);
        case LATENCY_RECEIVE_PRESENT:
            GRAPH_DATA(m_latencyReceiveToPresent);
        case LATENCY_PRESENT_DISPLAY:
            GRAPH_DATA(m_latencyPresentToDisplay);
        case TIME_SINCE_LAST_PRESENT:
            GRAPH_DATA(m_timeSinceLastPresent);
        case FRAMES_RECEIVED:
            GRAPH_DATA(m_videoFramesReceived);
        case FRAMES_REUSED:
            GRAPH_DATA(m_videoFramesReused);
        case FRAMES_SKIPPED:
            GRAPH_DATA(m_videoFramesSkipped);
        case FRAMES_DISCARDED:
            GRAPH_DATA(m_videoFramesDiscarded);
        case FRAME_MINIMUM_DELTA:
            GRAPH_DATA(m_videoFrameMinDelta);
        case FRAME_MAXIMUM_DELTA:
            GRAPH_DATA(m_videoFrameMaxDelta);
        case NETWORK_ROUNDTRIP:
            GRAPH_DATA(m_networkLatency);
        case FRAME_TIME_CPU:
            GRAPH_DATA(m_timeCPU);
        case FRAME_TIME_GPU:
            GRAPH_DATA(m_timeGPU);
        case UTILIZATION_CPU:
            GRAPH_DATA(m_utilizationCPU);
        case UTILIZATION_GPU:
            GRAPH_DATA(m_utilizationGPU);
        case MEMORY_CPU:
            GRAPH_DATA(m_memoryCPU);
        case MEMORY_GPU:
            GRAPH_DATA(m_memoryGPU);
    }
}
