#include <Model/ArrSessionManager.h>
#include <ViewModel/ModelEditor/StatsPageModel.h>

StatsPageModel::StatsPageModel(ArrServiceStats* serviceStats, QObject* parent)
    : QObject(parent)
    , m_serviceStats(serviceStats)
{
	connect(m_serviceStats, &ArrServiceStats::updated, this,
		[this]()
		{
			m_serviceStats->getStats(m_stats);
			Q_EMIT valuesChanged();
		});
}

int StatsPageModel::getParameterCount() const
{
    return 11;
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
            units = "";
            break;
        case 9:
            name = tr("Frame maximum delta");
            units = "";
            break;
        case 10:
            name = tr("Network roundtrip");
            units = "ms";
            break;
    }
}

double StatsPageModel::getParameter(int index) const
{
    switch (index)
    {
        case 0:
            return m_stats.m_latencyPoseToReceiveAvg;
        case 1:
            return m_stats.m_latencyReceiveToPresentAvg;
        case 2:
            return m_stats.m_latencyPresentToDisplayAvg;
        case 3:
            return m_stats.m_timeSinceLastPresentAvg;
        case 4:
            return m_stats.m_videoFramesReused;
        case 5:
            return m_stats.m_videoFramesSkipped;
        case 6:
            return m_stats.m_videoFramesReceived;
        case 7:
            return m_stats.m_videoFramesDiscarded;
        case 8:
            return m_stats.m_videoFrameMinDelta;
        case 9:
            return m_stats.m_videoFrameMaxDelta;
        case 10:
            return m_stats.m_currentPerformanceAssessment.networkLatency.aggregate;
        default:
            return 0;
    }
}

std::optional<RR::PerformanceRating> StatsPageModel::getParameterRating(int index) const
{
	switch (index)
	{
	case 10:
		return m_stats.m_currentPerformanceAssessment.networkLatency.rating;
	default: return {};
	}	
}
