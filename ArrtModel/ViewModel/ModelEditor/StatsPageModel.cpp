#include <Model/ArrSessionManager.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>

StatsPageModel::PlotInfo StatsPageModel::m_plotInfo[] = {
    {"Polygons rendered", PlotInfo::Line, PlotInfo::Continuous, Qt::white, "", true, {}, {}},

    {"Latency (pose to receive average)", PlotInfo::Line, PlotInfo::Continuous, Qt::yellow, "ms", false, {}, {}},
    {"Latency (receive to present average)", PlotInfo::Line, PlotInfo::Continuous, Qt::cyan, "ms", false, {}, {}},
    {"Latency (present to display average)", PlotInfo::Line, PlotInfo::Continuous, Qt::magenta, "ms", true, {}, {}},

    {"Time since last present", PlotInfo::Line, PlotInfo::Continuous, Qt::white, "ms", true, {}, {}},

    {"Frames reused", PlotInfo::Line, PlotInfo::Continuous, Qt::cyan, "", false, {}, {}},
    {"Frames skipped", PlotInfo::Line, PlotInfo::Continuous, Qt::yellow, "", false, {}, {}},
    {"Frames received", PlotInfo::Line, PlotInfo::Continuous, Qt::green, "", false, {}, {}},
    {"Frames discarded", PlotInfo::Line, PlotInfo::Continuous, Qt::red, "", true, {}, {}},

    {"Frame minimum delta", PlotInfo::AreaLowerBound, PlotInfo::Continuous, Qt::white, QString("ms"), false, {}, {}},
    {"Frame maximum delta", PlotInfo::AreaHigherBound, PlotInfo::Continuous, Qt::white, QString("ms"), true, {}, {}},

    {"Network round-trip", PlotInfo::Line, PlotInfo::Continuous, Qt::white, QString("ms"), true, {}, {}},

    {"Frame time CPU", PlotInfo::Line, PlotInfo::Continuous, Qt::yellow, QString("ms"), false, {}, {}},
    {"Frame time GPU", PlotInfo::Line, PlotInfo::Continuous, Qt::magenta, QString("ms"), true, {}, {}},

    {"Utilization CPU", PlotInfo::Line, PlotInfo::Continuous, Qt::yellow, QString("%"), false, 0, 100},
    {"Utilization GPU", PlotInfo::Line, PlotInfo::Continuous, Qt::magenta, QString("%"), true, 0, 100},

    {"Memory CPU", PlotInfo::Line, PlotInfo::Continuous, Qt::yellow, QString("%"), false, 0, 100},
    {"Memory GPU", PlotInfo::Line, PlotInfo::Continuous, Qt::magenta, QString("%"), true, 0, 100}};


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
            return m_stats.m_videoFramesReused.m_perWindowStats.m_value;
        case 6:
            return m_stats.m_videoFramesSkipped.m_perWindowStats.m_value;
        case 7:
            return m_stats.m_videoFramesReceived.m_perWindowStats.m_value;
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

void StatsPageModel::getGraphData(int index, std::vector<QPointF>& graph) const
{
    switch (index)
    {
        case 0:
            m_stats.m_polygonsRendered.getGraphData(graph);
            break;
        case 1:
            m_stats.m_latencyPoseToReceive.getGraphData(graph);
            break;
        case 2:
            m_stats.m_latencyReceiveToPresent.getGraphData(graph);
            break;
        case 3:
            m_stats.m_latencyPresentToDisplay.getGraphData(graph);
            break;
        case 4:
            m_stats.m_timeSinceLastPresent.getGraphData(graph);
            break;
        case 5:
            m_stats.m_videoFramesReused.getGraphData(graph);
            break;
        case 6:
            m_stats.m_videoFramesSkipped.getGraphData(graph);
            break;
        case 7:
            m_stats.m_videoFramesReceived.getGraphData(graph);
            break;
        case 8:
            m_stats.m_videoFramesDiscarded.getGraphData(graph);
            break;
        case 9:
            m_stats.m_videoFrameMinDelta.getGraphData(graph);
            break;
        case 10:
            m_stats.m_videoFrameMaxDelta.getGraphData(graph);
            break;
        case 11:
            m_stats.m_networkLatency.getGraphData(graph);
            break;
        case 12:
            m_stats.m_timeCPU.getGraphData(graph);
            break;
        case 13:
            m_stats.m_timeGPU.getGraphData(graph);
            break;
        case 14:
            m_stats.m_utilizationCPU.getGraphData(graph);
            break;
        case 15:
            m_stats.m_utilizationGPU.getGraphData(graph);
            break;
        case 16:
            m_stats.m_memoryCPU.getGraphData(graph);
            break;
        case 17:
            m_stats.m_memoryGPU.getGraphData(graph);
            break;
    }
}


std::optional<RR::PerformanceRating> StatsPageModel::getParameterRating(int /*index*/) const
{
    /*
    switch (index)
    {
        case 10:
            return m_stats.m_currentPerformanceAssessment.timeCPU.rating;
        case 11:
            return m_stats.m_currentPerformanceAssessment.timeGPU.rating;
        case 12:
            return m_stats.m_currentPerformanceAssessment.utilizationCPU.rating;
        case 13:
            return m_stats.m_currentPerformanceAssessment.utilizationGPU.rating;
        case 14:
            return m_stats.m_currentPerformanceAssessment.memoryCPU.rating;
        case 15:
            return m_stats.m_currentPerformanceAssessment.memoryGPU.rating;
        case 16:
            return m_stats.m_currentPerformanceAssessment.networkLatency.rating;
        case 17:
            return m_stats.m_currentPerformanceAssessment.polygonsRendered.rating;
        default:
            return {};
    }
	*/
    return {};
}
