#include <Model/ArrServiceStats.h>
#include <Model/Log/LogHelpers.h>

ArrServiceStats::ArrServiceStats(QObject* parent)
    : QObject(parent)
{
    m_currWindowsElapsedTimer.start();
}


void ArrServiceStats::update(RR::ApiHandle<RR::RenderingSession> session)
{
    updateStats(session);
}

void ArrServiceStats::updateStats(RR::ApiHandle<RR::RenderingSession> session)
{
    if (!m_collecting || !session || !session->GetIsConnected())
    {
        return;
    }
    ++m_tick;

    RR::FrameStatistics frameStatistics;
    if (session->GetGraphicsBinding()->GetLastFrameStatistics(&frameStatistics) != RR::Result::Success)
    {
        return;
    }

    m_currentStats.m_timeSinceLastPresent.addValue(frameStatistics.TimeSinceLastPresent * 1000.0, m_tick);
    m_currentStats.m_videoFramesSkipped.addValue(frameStatistics.VideoFramesSkipped, m_tick);
    m_currentStats.m_videoFramesReused.addValue(frameStatistics.VideoFrameReusedCount, m_tick);
    m_currentStats.m_videoFramesReceived.addValue(frameStatistics.VideoFramesReceived, m_tick);
    if (frameStatistics.VideoFramesReceived)
    {
        m_currentStats.m_videoFrameMinDelta.addValue(frameStatistics.VideoFrameMinDelta * 1000.0, m_tick);
        m_currentStats.m_videoFrameMaxDelta.addValue(frameStatistics.VideoFrameMaxDelta * 1000.0, m_tick);
    }
    m_currentStats.m_latencyPoseToReceive.addValue(frameStatistics.LatencyPoseToReceive * 1000.0, m_tick);
    m_currentStats.m_latencyReceiveToPresent.addValue(frameStatistics.LatencyReceiveToPresent * 1000.0, m_tick);
    m_currentStats.m_latencyPresentToDisplay.addValue(frameStatistics.LatencyPresentToDisplay * 1000.0, m_tick);
    m_currentStats.m_videoFramesDiscarded.addValue(frameStatistics.VideoFramesDiscarded, m_tick);

    if (m_runningPerformanceAssesment && m_runningPerformanceAssesment->GetIsCompleted())
    {
        if (m_runningPerformanceAssesment->GetIsFaulted())
        {
            qWarning(LoggingCategory::renderingSession) << tr("Error retrieving the performance assessment. Failure reason: ") << m_runningPerformanceAssesment->GetStatus();
            m_runningPerformanceAssesment = {};
        }
        else if (m_runningPerformanceAssesment->GetStatus() == RR::Result::Success)
        {
            m_lastPerformanceAssessment = m_runningPerformanceAssesment->GetResult();
            m_currentStats.m_timeCPU.addValue(m_lastPerformanceAssessment.TimeCpu.Aggregate, m_tick);
            m_currentStats.m_timeGPU.addValue(m_lastPerformanceAssessment.TimeGpu.Aggregate, m_tick);
            m_currentStats.m_utilizationCPU.addValue(m_lastPerformanceAssessment.UtilizationCpu.Aggregate, m_tick);
            m_currentStats.m_utilizationGPU.addValue(m_lastPerformanceAssessment.UtilizationGpu.Aggregate, m_tick);
            m_currentStats.m_memoryCPU.addValue(m_lastPerformanceAssessment.MemoryCpu.Aggregate, m_tick);
            m_currentStats.m_memoryGPU.addValue(m_lastPerformanceAssessment.MemoryGpu.Aggregate, m_tick);
            m_currentStats.m_networkLatency.addValue(m_lastPerformanceAssessment.NetworkLatency.Aggregate, m_tick);
            m_currentStats.m_polygonsRendered.addValue(m_lastPerformanceAssessment.PolygonsRendered.Aggregate, m_tick);
            m_runningPerformanceAssesment = {};
        }
        else
        {
            qWarning(LoggingCategory::renderingSession) << tr("what?");
        }
    }

    // If 1 second has past, clear the last stats list.
    //var now = QDateTime::currentDateTime();
    if (m_currWindowsElapsedTimer.elapsed() >= 1000)
    {
        m_secondsTick++;
        m_currWindowsElapsedTimer.start();
        m_currentStats.m_timeSinceLastPresent.endWindow(m_secondsTick);
        m_currentStats.m_videoFramesSkipped.endWindow(m_secondsTick);
        m_currentStats.m_videoFramesReused.endWindow(m_secondsTick);
        m_currentStats.m_videoFramesReceived.endWindow(m_secondsTick);
        m_currentStats.m_videoFrameMinDelta.endWindow(m_secondsTick);
        m_currentStats.m_videoFrameMaxDelta.endWindow(m_secondsTick);
        m_currentStats.m_latencyPoseToReceive.endWindow(m_secondsTick);
        m_currentStats.m_latencyReceiveToPresent.endWindow(m_secondsTick);
        m_currentStats.m_latencyPresentToDisplay.endWindow(m_secondsTick);
        m_currentStats.m_videoFramesDiscarded.endWindow(m_secondsTick);
        m_currentStats.m_timeCPU.endWindow(m_secondsTick);
        m_currentStats.m_timeGPU.endWindow(m_secondsTick);
        m_currentStats.m_utilizationCPU.endWindow(m_secondsTick);
        m_currentStats.m_utilizationGPU.endWindow(m_secondsTick);
        m_currentStats.m_memoryCPU.endWindow(m_secondsTick);
        m_currentStats.m_memoryGPU.endWindow(m_secondsTick);
        m_currentStats.m_networkLatency.endWindow(m_secondsTick);
        m_currentStats.m_polygonsRendered.endWindow(m_secondsTick);

        // query the performance assessment (assuming the previous query is completed)
        if (!m_runningPerformanceAssesment)
        {
            if (auto query = session->Connection()->QueryServerPerformanceAssessmentAsync())
            {
                m_runningPerformanceAssesment = *query;
            }
        }

        Q_EMIT updated();
    }
}

void ArrServiceStats::startCollecting()
{
    m_tick = 0;
    m_secondsTick = 0;
    m_collecting = true;
    m_currentStats = {};
    m_currWindowsElapsedTimer.start();
}

void ArrServiceStats::stopCollecting()
{
    m_collecting = false;
}
