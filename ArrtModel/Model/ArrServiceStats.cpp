#include <Model/ArrServiceStats.h>

ArrServiceStats::ArrServiceStats(QObject* parent)
	: QObject(parent)
{
	m_currWindowsElapsedTimer.start();
}


void ArrServiceStats::update(RR::ApiHandle<RR::AzureSession> session)
{
    updateStats(session);
}

void ArrServiceStats::getStats(Stats& stats)
{
	stats = {};

    for (const RR::FrameStatistics& frameStatistics : m_lastWindowFrameStats)
    {
        stats.m_timeSinceLastPresentAvg += frameStatistics.timeSinceLastPresent;
        stats.m_timeSinceLastPresentMax = qMax(stats.m_timeSinceLastPresentMax, frameStatistics.timeSinceLastPresent);

        stats.m_videoFramesSkipped += frameStatistics.videoFramesSkipped;
        stats.m_videoFramesReused += frameStatistics.videoFrameReusedCount > 0u ? 1u : 0u;
        stats.m_videoFramesReceived += frameStatistics.videoFramesReceived;

        if (frameStatistics.videoFramesReceived > 0)
        {
            if (stats.m_videoFrameMinDelta == 0.0f)
            {
                stats.m_videoFrameMinDelta = frameStatistics.videoFrameMinDelta;
                stats.m_videoFrameMaxDelta = frameStatistics.videoFrameMaxDelta;
            }
            else
            {
                stats.m_videoFrameMinDelta = qMin(stats.m_videoFrameMinDelta, frameStatistics.videoFrameMinDelta);
                stats.m_videoFrameMaxDelta = qMax(stats.m_videoFrameMaxDelta, frameStatistics.videoFrameMaxDelta);
            }
        }

        stats.m_latencyPoseToReceiveAvg += frameStatistics.latencyPoseToReceive;
        stats.m_latencyReceiveToPresentAvg += frameStatistics.latencyReceiveToPresent;
        stats.m_latencyPresentToDisplayAvg += frameStatistics.latencyPresentToDisplay;
        stats.m_videoFramesDiscarded += frameStatistics.videoFramesDiscarded;
    }

    int frameStatsCount = m_lastWindowFrameStats.size();
    if (frameStatsCount > 0)
    {
        stats.m_timeSinceLastPresentAvg /= (float)frameStatsCount;
        stats.m_latencyPoseToReceiveAvg /= (float)frameStatsCount;
        stats.m_latencyReceiveToPresentAvg /= (float)frameStatsCount;
        stats.m_latencyPresentToDisplayAvg /= (float)frameStatsCount;
    }
    stats.m_videoFramesDiscardedTotal = m_videoFramesDiscardedTotal;
    stats.m_framesUsedForAverage = (uint)frameStatsCount;

	stats.m_currentPerformanceAssessment = m_lastPerformanceAssessment;
}

void ArrServiceStats::updateStats(RR::ApiHandle<RR::AzureSession> session)
{
    if (!session || !session->IsConnected())
    {
        return;
    }

    RR::FrameStatistics frameStatistics;
    if (*session->GetGraphicsBinding()->GetLastFrameStatistics(&frameStatistics) != RR::Result::Success)
    {
        return;
    }

    if (!m_runningPerformanceAssesment)
    {
        if (auto query = session->Actions()->QueryServerPerformanceAssessmentAsync())
        {
            m_runningPerformanceAssesment = *query;
        }
    }
    else if (m_runningPerformanceAssesment->IsCompleted())
    {
        if (m_runningPerformanceAssesment->IsRanToCompletion())
        {
			if (auto result = m_runningPerformanceAssesment->Result())
			{
				m_lastPerformanceAssessment = *result;
			}
        }
        m_runningPerformanceAssesment = {};
    }

    // If 1 second has past, clear the last stats list.
    //var now = QDateTime::currentDateTime();
    if (m_currWindowsElapsedTimer.elapsed() >= 1000)
    {
        m_lastWindowFrameStats.swap(m_currWindowFrameStats);
        m_currWindowFrameStats.clear();

        // Next list clearing should happen at least 1 second from now
        m_currWindowsElapsedTimer.start();
    }

    m_currWindowFrameStats.push_back(frameStatistics);
    m_videoFramesDiscardedTotal += frameStatistics.videoFramesDiscarded;
}
