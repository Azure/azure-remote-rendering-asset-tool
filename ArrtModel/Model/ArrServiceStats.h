#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QObject>
#include <QElapsedTimer>

class ArrServiceStats : public QObject
{
    Q_OBJECT
public:

	ArrServiceStats(QObject*parent = {});

    struct Stats
    {
    public:
        float m_timeSinceLastPresentAvg;
        float m_timeSinceLastPresentMax;
        uint m_videoFramesSkipped;
        uint m_videoFramesReused;
        uint m_videoFramesReceived;
        float m_videoFrameMinDelta;
        float m_videoFrameMaxDelta;
        float m_latencyPoseToReceiveAvg;
        float m_latencyReceiveToPresentAvg;
        float m_latencyPresentToDisplayAvg;
        uint m_videoFramesDiscarded;
        uint m_videoFramesDiscardedTotal;
        uint m_framesUsedForAverage;
        RR::PerformanceAssessment m_currentPerformanceAssessment;

        /*
			"Render: {FramesUsedForAverage} fps - {(TimeSinceLastPresentAvg * 1000).ToString(" F2 ")} / {(TimeSinceLastPresentMax * 1000).ToString(" F2 ")} ms (avg / max)\r\n"
			"Video frames: {VideoFramesSkipped} / {VideoFramesReused} / {VideoFramesReceived} skipped / reused / received\r\n" 
			"Video frames delta: {(VideoFrameMinDelta * 1000).ToString(" F2 ")} / {(VideoFrameMaxDelta * 1000).ToString(" F2 ")} ms (min / max)\r\n" 
			"Latency: {(LatencyPoseToReceiveAvg * 1000).ToString(" F2 ")} / {(LatencyReceiveToPresentAvg * 1000).ToString(" F2 ")} / {(LatencyPresentToDisplayAvg * 1000).ToString(" F2 ")} ms (avg) pose/receive/display  \r\n"
			"Video frames discarded: {VideoFramesDiscarded} / {VideoFramesDiscardedTotal} frames (last sec / total)\r\n"
			"Frame time CPU: {CurrentPerformanceAssessment.timeCPU.aggregate.ToString(" F2 ") } ms ({CurrentPerformanceAssessment.timeCPU.rating})"
			"Frame time GPU: {CurrentPerformanceAssessment.timeGPU.aggregate.ToString(" F2 ")} ms ({CurrentPerformanceAssessment.timeGPU.rating})\n"
			"Utilization CPU: {CurrentPerformanceAssessment.utilizationCPU.aggregate.ToString(" F2 ") } % ({CurrentPerformanceAssessment.utilizationCPU.rating})"
			"Utilization GPU: {CurrentPerformanceAssessment.utilizationGPU.aggregate.ToString(" F2 ")} % ({CurrentPerformanceAssessment.utilizationGPU.rating})"
			"Memory CPU: {CurrentPerformanceAssessment.memoryCPU.aggregate.ToString(" F2 ") } % ({CurrentPerformanceAssessment.memoryCPU.rating})"
			"Memory GPU: {CurrentPerformanceAssessment.memoryGPU.aggregate.ToString(" F2 ")} % ({CurrentPerformanceAssessment.memoryGPU.rating})"
			"Network roundtrip: {CurrentPerformanceAssessment.networkLatency.aggregate.ToString(" F2 ") } ms ({CurrentPerformanceAssessment.networkLatency.rating})\n"
			"Polygons rendered: {CurrentPerformanceAssessment.polygonsRendered.aggregate.ToString(" N0 ")} ({CurrentPerformanceAssessment.polygonsRendered.rating})";
			*/
    };

    /// Call every frame to collect statistics for given frame from the graphics binding.
	void update(RR::ApiHandle<RR::AzureSession> session);

    /// Get statistics for last second of the video stream.
	void getStats(Stats& stats);

Q_SIGNALS:
	void updated();

private:
    QElapsedTimer m_currWindowsElapsedTimer;
    //double m_currWindowStartTime = DateTime.Now.TimeOfDay.TotalSeconds;
    uint m_videoFramesDiscardedTotal = 0;
    QList<RR::FrameStatistics> m_currWindowFrameStats;
    QList<RR::FrameStatistics> m_lastWindowFrameStats;
    RR::ApiHandle<RR::PerformanceAssessmentAsync> m_runningPerformanceAssesment;
    RR::PerformanceAssessment m_lastPerformanceAssessment;

	void updateStats(RR::ApiHandle<RR::AzureSession> session);
};
