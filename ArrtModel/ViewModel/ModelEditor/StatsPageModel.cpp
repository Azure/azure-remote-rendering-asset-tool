#include <Model/ArrSessionManager.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>

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
    return 18;
}


void StatsPageModel::getParameterInfo(int index, QString& name, QString& units, std::optional<double>& minValue, std::optional<double>& maxValue) const
{
    minValue = 0;
    maxValue = {};

    switch (index)
    {
        case 0:
            name = tr("Latency (pose to receive average)");
            units = tr("ms");
            break;
        case 1:
            name = tr("Latency (receive to present average)");
            units = tr("ms");
            break;
        case 2:
            name = tr("Latency (present to display average)");
            units = tr("ms");
            break;
        case 3:
            name = tr("Time since last present");
            units = tr("ms");
            break;
        case 4:
            name = tr("Frames reused");
            units = "";
            break;
        case 5:
            name = tr("Frames skipped");
            units = "";
            break;
        case 6:
            name = tr("Frames received");
            units = "";
            break;
        case 7:
            name = tr("Frames discarded");
            units = "";
            break;
        case 8:
            name = tr("Frame minimum delta");
            units = "ms";
            break;
        case 9:
            name = tr("Frame maximum delta");
            units = "ms";
            break;
        case 10:
            name = tr("Frame time CPU");
            units = "ms";
            break;
        case 11:
            name = tr("Frame time GPU");
            units = "ms";
            break;
        case 12:
            name = tr("Utilization CPU");
            units = "%";
            break;
        case 13:
            name = tr("Utilization GPU");
            units = "%";
            break;
        case 14:
            name = tr("Memory CPU");
            units = "%";
            break;
        case 15:
            name = tr("Memory GPU");
            units = "%";
            break;
        case 16:
            name = tr("Network round-trip");
            units = "ms";
            break;
        case 17:
            name = tr("Polygons rendered");
            units = "";
            break;
    }
}

double StatsPageModel::getParameter(int index) const
{
    switch (index)
    {
        case 0:
            return m_stats.m_latencyPoseToReceive.m_perWindowStats.m_value;
        case 1:
            return m_stats.m_latencyReceiveToPresent.m_perWindowStats.m_value;
        case 2:
            return m_stats.m_latencyPresentToDisplay.m_perWindowStats.m_value;
        case 3:
            return m_stats.m_timeSinceLastPresent.m_perWindowStats.m_value;
        case 4:
            return m_stats.m_videoFramesReused.m_perWindowStats.m_value;
        case 5:
            return m_stats.m_videoFramesSkipped.m_perWindowStats.m_value;
        case 6:
            return m_stats.m_videoFramesReceived.m_perWindowStats.m_value;
        case 7:
            return m_stats.m_videoFramesDiscarded.m_perWindowStats.m_value;
        case 8:
            return m_stats.m_videoFrameMinDelta.m_perWindowStats.m_value;
        case 9:
            return m_stats.m_videoFrameMaxDelta.m_perWindowStats.m_value;
        case 10:
            return m_stats.m_timeCPU.m_perWindowStats.m_value;
        case 11:
            return m_stats.m_timeGPU.m_perWindowStats.m_value;
        case 12:
            return m_stats.m_utilizationCPU.m_perWindowStats.m_value;
        case 13:
            return m_stats.m_utilizationGPU.m_perWindowStats.m_value;
        case 14:
            return m_stats.m_memoryCPU.m_perWindowStats.m_value;
        case 15:
            return m_stats.m_memoryGPU.m_perWindowStats.m_value;
        case 16:
            return m_stats.m_networkLatency.m_perWindowStats.m_value;
        case 17:
            return m_stats.m_polygonsRendered.m_perWindowStats.m_value;
        default:
            return 0;
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
