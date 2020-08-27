#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QElapsedTimer>
#include <QObject>

template <typename T>
struct MinValue
{
    void addValue(const T& value)
    {
        m_value = qMin(m_value, value);
    }
    T m_value = std::numeric_limits<T>::max();
};

template <typename T>
struct MaxValue
{
    void addValue(const T& value)
    {
        m_value = qMax(m_value, value);
    }
    T m_value = std::numeric_limits<T>::min();
};

template <typename T>
struct SumValue
{
    void addValue(const T& value)
    {
        m_value += value;
    }
    T m_value = {};
};

template <typename T>
struct AvgValue
{
    void addValue(const T& value)
    {
        m_value += value;
        ++m_values;
    }
    double getAverage() const
    {
        if (m_values > 0)
        {
            return (double)m_value / m_values;
        }
        return {};
    }
    T m_value;
    uint m_values = {};
};

template <typename T>
struct AvgMinMaxValue
{
    void addValue(const T& value)
    {
        m_value = value;
        m_sum.addValue(value);
        m_min.addValue(value);
        m_max.addValue(value);
        ++m_values;
    }
    double getAverage() const
    {
        if (m_values > 0)
        {
            return (double)m_sum.m_value / m_values;
        }
        return {};
    }
    T m_value = {};
    SumValue<T> m_sum;
    MinValue<T> m_min;
    MaxValue<T> m_max;
    uint m_values = {};
};

template <typename T>
struct Accumulator
{
    T m_accumulation = {};
    typedef decltype(m_accumulation.m_value) ValueType;

    void addValue(const ValueType& value)
    {
        m_accumulation.addValue(value);
    }

    AvgMinMaxValue<ValueType> m_perWindowStats;
    void endWindow()
    {
        m_perWindowStats.addValue(m_accumulation.m_value);
        m_accumulation = {};
    }
};

class ArrServiceStats : public QObject
{
    Q_OBJECT
public:
    ArrServiceStats(QObject* parent = {});

    struct Stats
    {
    public:
        Accumulator<AvgMinMaxValue<float>> m_timeSinceLastPresent;
        Accumulator<SumValue<uint>> m_videoFramesSkipped;
        Accumulator<SumValue<uint>> m_videoFramesReused;
        Accumulator<SumValue<uint>> m_videoFramesReceived;
        Accumulator<MinValue<float>> m_videoFrameMinDelta;
        Accumulator<MaxValue<float>> m_videoFrameMaxDelta;
        Accumulator<AvgMinMaxValue<float>> m_latencyPoseToReceive;
        Accumulator<AvgMinMaxValue<float>> m_latencyReceiveToPresent;
        Accumulator<AvgMinMaxValue<float>> m_latencyPresentToDisplay;
        Accumulator<SumValue<uint>> m_videoFramesDiscarded;

        Accumulator<AvgValue<float>> m_timeCPU;
        Accumulator<AvgValue<float>> m_timeGPU;
        Accumulator<AvgValue<float>> m_utilizationCPU;
        Accumulator<AvgValue<float>> m_utilizationGPU;
        Accumulator<AvgValue<float>> m_memoryCPU;
        Accumulator<AvgValue<float>> m_memoryGPU;
        Accumulator<AvgValue<float>> m_networkLatency;
        Accumulator<AvgValue<uint>> m_polygonsRendered;
    };

    void startCollecting();
    void stopCollecting();

    bool isCollecting() const
    {
        return m_collecting;
    }


    /// Call every frame to collect statistics for given frame from the graphics binding.
    void update(RR::ApiHandle<RR::AzureSession> session);

    /// Get current statistics
    const Stats& getStats()
    {
        return m_currentStats;
    }

Q_SIGNALS:
    void updated();

private:
    QElapsedTimer m_currWindowsElapsedTimer;
    //uint m_videoFramesDiscardedTotal = 0;
    //QVector<RR::FrameStatistics> m_currWindowFrameStats;
    //QVector<RR::FrameStatistics> m_lastWindowFrameStats;

    Stats m_currentStats;

    RR::ApiHandle<RR::PerformanceAssessmentAsync> m_runningPerformanceAssesment;
    RR::PerformanceAssessment m_lastPerformanceAssessment;
    bool m_collecting = false;

    void updateStats(RR::ApiHandle<RR::AzureSession> session);
};
