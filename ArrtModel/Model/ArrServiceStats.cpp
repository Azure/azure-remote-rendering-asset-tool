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

void ArrServiceStats::updateStats(RR::ApiHandle<RR::AzureSession> session)
{
    if (!m_collecting || !session || !session->IsConnected())
    {
        return;
    }
    ++m_tick;

    RR::FrameStatistics frameStatistics;
    if (*session->GetGraphicsBinding()->GetLastFrameStatistics(&frameStatistics) != RR::Result::Success)
    {
        return;
    }

    m_currentStats.m_timeSinceLastPresent.addValue(frameStatistics.timeSinceLastPresent * 1000.0, m_tick);
    m_currentStats.m_videoFramesSkipped.addValue(frameStatistics.videoFramesSkipped, m_tick);
    m_currentStats.m_videoFramesReused.addValue(frameStatistics.videoFrameReusedCount, m_tick);
    m_currentStats.m_videoFramesReceived.addValue(frameStatistics.videoFramesReceived, m_tick);
    if (frameStatistics.videoFramesReceived)
    {
        m_currentStats.m_videoFrameMinDelta.addValue(frameStatistics.videoFrameMinDelta * 1000.0, m_tick);
        m_currentStats.m_videoFrameMaxDelta.addValue(frameStatistics.videoFrameMaxDelta * 1000.0, m_tick);
    }
    m_currentStats.m_latencyPoseToReceive.addValue(frameStatistics.latencyPoseToReceive * 1000.0, m_tick);
    m_currentStats.m_latencyReceiveToPresent.addValue(frameStatistics.latencyReceiveToPresent * 1000.0, m_tick);
    m_currentStats.m_latencyPresentToDisplay.addValue(frameStatistics.latencyPresentToDisplay * 1000.0, m_tick);
    m_currentStats.m_videoFramesDiscarded.addValue(frameStatistics.videoFramesDiscarded, m_tick);

    if (m_runningPerformanceAssesment && m_runningPerformanceAssesment->IsCompleted())
    {
        if (m_runningPerformanceAssesment->IsRanToCompletion())
        {
            auto result = m_runningPerformanceAssesment->Result();
            if (result.has_value())
            {
                m_currentStats.m_timeCPU.addValue(result->timeCPU.aggregate, m_tick);
                m_currentStats.m_timeGPU.addValue(result->timeGPU.aggregate, m_tick);
                m_currentStats.m_utilizationCPU.addValue(result->utilizationCPU.aggregate, m_tick);
                m_currentStats.m_utilizationGPU.addValue(result->utilizationGPU.aggregate, m_tick);
                m_currentStats.m_memoryCPU.addValue(result->memoryCPU.aggregate, m_tick);
                m_currentStats.m_memoryGPU.addValue(result->memoryGPU.aggregate, m_tick);
                m_currentStats.m_networkLatency.addValue(result->networkLatency.aggregate, m_tick);
                m_currentStats.m_polygonsRendered.addValue(result->networkLatency.aggregate, m_tick);

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
