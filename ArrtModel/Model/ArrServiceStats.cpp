#include <Model/ArrServiceStats.h>
#include <Model/Log/LogHelpers.h>

ArrServiceStats::ArrServiceStats(QObject* parent)
    : QObject(parent)
{
    m_currWindowsElapsedTimer.start();
}


void ArrServiceStats::update(RR::ApiHandle<RR::AzureSession> session)
{
    updateStats(session);
}

/*
void ArrServiceStats::getStats(Stats& stats)
{
    stats = {};

    for (const RR::FrameStatistics& frameStatistics : m_lastWindowFrameStats)
    {
		stats.m_timeSinceLastPresent;
        Accumulator<SumValue<uint>> m_videoFramesSkipped;
        Accumulator<SumValue<uint>> m_videoFramesReused;
        Accumulator<SumValue<uint>> m_videoFramesReceived;
        Accumulator<MinValue<float>> m_videoFrameMinDelta;
        Accumulator<MaxValue<float>> m_videoFrameMaxDelta;
        PerFrameValue<float> m_latencyPoseToReceive;
        PerFrameValue<float> m_latencyReceiveToPresent;
        PerFrameValue<float> m_latencyPresentToDisplay;
        PerFrameValue<uint> m_videoFramesDiscarded;

        PerFrameValue<float> m_timeCPU;
        PerFrameValue<float> m_timeGPU;
        PerFrameValue<float> m_utilizationCPU;
        PerFrameValue<float> m_utilizationGPU;
        PerFrameValue<float> m_memoryCPU;
        PerFrameValue<float> m_memoryGPU;
        PerFrameValue<float> m_networkLatency;
        PerFrameValue<uint> m_polygonsRendered;

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
*/

void ArrServiceStats::updateStats(RR::ApiHandle<RR::AzureSession> session)
{
    if (!m_collecting || !session || !session->IsConnected())
    {
        return;
    }

    RR::FrameStatistics frameStatistics;
    if (*session->GetGraphicsBinding()->GetLastFrameStatistics(&frameStatistics) != RR::Result::Success)
    {
        return;
    }

    m_currentStats.m_timeSinceLastPresent.addValue(frameStatistics.timeSinceLastPresent * 1000.0);
    m_currentStats.m_videoFramesSkipped.addValue(frameStatistics.videoFramesSkipped);
    m_currentStats.m_videoFramesReused.addValue(frameStatistics.videoFrameReusedCount);
    m_currentStats.m_videoFramesReceived.addValue(frameStatistics.videoFramesReceived);
    if (frameStatistics.videoFramesReceived)
    {
        m_currentStats.m_videoFrameMinDelta.addValue(frameStatistics.videoFrameMinDelta * 1000.0);
        m_currentStats.m_videoFrameMaxDelta.addValue(frameStatistics.videoFrameMaxDelta * 1000.0);
    }
    m_currentStats.m_latencyPoseToReceive.addValue(frameStatistics.latencyPoseToReceive * 1000.0);
    m_currentStats.m_latencyReceiveToPresent.addValue(frameStatistics.latencyReceiveToPresent * 1000.0);
    m_currentStats.m_latencyPresentToDisplay.addValue(frameStatistics.latencyPresentToDisplay * 1000.0);
    m_currentStats.m_videoFramesDiscarded.addValue(frameStatistics.videoFramesDiscarded);

    if (m_runningPerformanceAssesment && m_runningPerformanceAssesment->IsCompleted())
    {
        if (m_runningPerformanceAssesment->IsRanToCompletion())
        {
            auto result = m_runningPerformanceAssesment->Result();
            if (result.has_value())
            {
                m_currentStats.m_timeCPU.addValue(result->timeCPU.aggregate);
                m_currentStats.m_timeGPU.addValue(result->timeGPU.aggregate);
                m_currentStats.m_utilizationCPU.addValue(result->utilizationCPU.aggregate);
                m_currentStats.m_utilizationGPU.addValue(result->utilizationGPU.aggregate);
                m_currentStats.m_memoryCPU.addValue(result->memoryCPU.aggregate);
                m_currentStats.m_memoryGPU.addValue(result->memoryGPU.aggregate);
                m_currentStats.m_networkLatency.addValue(result->networkLatency.aggregate);
                m_currentStats.m_polygonsRendered.addValue(result->networkLatency.aggregate);

                m_lastPerformanceAssessment = *result;
                m_runningPerformanceAssesment = {};
            }
            else if (result.error() != RR::Status::InProgress)
            {
                qWarning(LoggingCategory::renderingSession) << tr("Error retrieving the performance assessment. Failure reason: ") << result.error();
                m_runningPerformanceAssesment = {};
            }
        }
    }

    // If 1 second has past, clear the last stats list.
    //var now = QDateTime::currentDateTime();
    if (m_currWindowsElapsedTimer.elapsed() >= 1000)
    {
        m_currWindowsElapsedTimer.start();
        m_currentStats.m_timeSinceLastPresent.endWindow();
        m_currentStats.m_videoFramesSkipped.endWindow();
        m_currentStats.m_videoFramesReused.endWindow();
        m_currentStats.m_videoFramesReceived.endWindow();
        m_currentStats.m_videoFrameMinDelta.endWindow();
        m_currentStats.m_videoFrameMaxDelta.endWindow();
        m_currentStats.m_latencyPoseToReceive.endWindow();
        m_currentStats.m_latencyReceiveToPresent.endWindow();
        m_currentStats.m_latencyPresentToDisplay.endWindow();
        m_currentStats.m_videoFramesDiscarded.endWindow();
        m_currentStats.m_timeCPU.endWindow();
        m_currentStats.m_timeGPU.endWindow();
        m_currentStats.m_utilizationCPU.endWindow();
        m_currentStats.m_utilizationGPU.endWindow();
        m_currentStats.m_memoryCPU.endWindow();
        m_currentStats.m_memoryGPU.endWindow();
        m_currentStats.m_networkLatency.endWindow();
        m_currentStats.m_polygonsRendered.endWindow();

        // query the performance assessment (assuming the previous query is completed)
        if (!m_runningPerformanceAssesment)
        {
            if (auto query = session->Actions()->QueryServerPerformanceAssessmentAsync())
            {
                m_runningPerformanceAssesment = *query;
            }
        }

        Q_EMIT updated();
    }
}

void ArrServiceStats::startCollecting()
{
    m_collecting = true;
    m_currentStats = {};

    m_currWindowsElapsedTimer.start();
}

void ArrServiceStats::stopCollecting()
{
    m_collecting = false;
}
