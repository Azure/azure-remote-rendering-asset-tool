#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QElapsedTimer>
#include <QObject>
#include <Utils/Accumulators.h>


// service statistics object, collecting and analyzing per-frame stats in an ARR session
class ArrServiceStats : public QObject
{
    Q_OBJECT
public:
    ArrServiceStats(QObject* parent = {});

    struct Stats
    {
    public:
        AccumulatorWithStorage<AvgMinMaxValue<float>> m_timeSinceLastPresent;
        AccumulatorWithStorage<SumValue<uint>> m_videoFramesSkipped;
        AccumulatorWithStorage<SumValue<uint>> m_videoFramesReused;
        AccumulatorWithStorage<SumValue<uint>> m_videoFramesReceived;
        AccumulatorWithStorage<MinValue<float>> m_videoFrameMinDelta;
        AccumulatorWithStorage<MaxValue<float>> m_videoFrameMaxDelta;
        AccumulatorWithStorage<AvgMinMaxValue<float>> m_latencyPoseToReceive;
        AccumulatorWithStorage<AvgMinMaxValue<float>> m_latencyReceiveToPresent;
        AccumulatorWithStorage<AvgMinMaxValue<float>> m_latencyPresentToDisplay;
        AccumulatorWithStorage<SumValue<uint>> m_videoFramesDiscarded;

        AccumulatorWithStorage<AvgValue<float>> m_timeCPU;
        AccumulatorWithStorage<AvgValue<float>> m_timeGPU;
        AccumulatorWithStorage<AvgValue<float>> m_utilizationCPU;
        AccumulatorWithStorage<AvgValue<float>> m_utilizationGPU;
        AccumulatorWithStorage<AvgValue<float>> m_memoryCPU;
        AccumulatorWithStorage<AvgValue<float>> m_memoryGPU;
        AccumulatorWithStorage<AvgValue<float>> m_networkLatency;
        AccumulatorWithStorage<AvgValue<uint>> m_polygonsRendered;
    };

    void startCollecting();
    void stopCollecting();

    bool isCollecting() const
    {
        return m_collecting;
    }

    /// Call every frame to collect statistics for given frame from the graphics binding.
    void update(RR::ApiHandle<RR::RenderingSession> session);

    /// Get current statistics
    const Stats& getStats()
    {
        return m_currentStats;
    }

Q_SIGNALS:
    void updated();

private:
    QElapsedTimer m_currWindowsElapsedTimer;

    Stats m_currentStats;

    std::atomic_bool m_assessmentAsyncHasNewResult = false;
    std::atomic_bool m_assessmentAsyncRunning = false;
    RR::Status m_assessmentAsyncStatus = RR::Status::InProgress;
    RR::PerformanceAssessment m_newPerformanceAssessmentResult;

    bool m_collecting = false;
    uint m_tick;
    uint m_secondsTick;

    void updateStats(RR::ApiHandle<RR::RenderingSession> session);
};
