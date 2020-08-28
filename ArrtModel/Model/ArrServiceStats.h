#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QElapsedTimer>
#include <QObject>

template <typename T, int BufferSize>
class FixedCircularBuffer
{
public:
    const T& getValue(uint index) const
    {
        assert(index < m_size);
        index = (BufferSize + m_start - index) % BufferSize;
        return m_buffer[index];
    }

    void addFront(T value)
    {
        m_start = (m_start + 1) % BufferSize;
        m_buffer[m_start] = std::move(value);
        if (m_size != BufferSize)
        {
            ++m_size;
        }
    }

    uint getSize() const
    {
        return m_size;
    }

    void clear()
    {
        m_size = 0;
    }

private:
    T m_buffer[BufferSize];
    uint m_start = 0;
    uint m_size = 0;
};


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
    typedef decltype(T::m_value) ValueType;
    struct GraphValue
    {
        ValueType m_value;
        uint m_tick;
    };

    T m_accumulation = {};
    FixedCircularBuffer<GraphValue, 2048> m_buffer;

    void addValue(const ValueType& value, uint tick)
    {
        m_accumulation.addValue(value);
        m_buffer.addFront({value, tick});
    }

    void getGraphData(std::vector<QPointF>& graph) const
    {
        graph.clear();

        if (m_buffer.getSize() > 0)
        {
            const ValueType minValue = m_perWindowStats.m_min.m_value;
            const ValueType maxValue = m_perWindowStats.m_max.m_value;
            const qreal rangeY = qMax(qreal(maxValue - minValue), qreal(0.001f));
            const uint tickOffset = m_buffer.getValue(0).m_tick;

            for (uint i = 0; i < m_buffer.getSize(); ++i)
            {
                auto val = m_buffer.getValue(i);
                graph.push_back(QPointF(tickOffset - val.m_tick, qreal(val.m_value - minValue) / rangeY));
            }
        }
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

    Stats m_currentStats;

    RR::ApiHandle<RR::PerformanceAssessmentAsync> m_runningPerformanceAssesment;
    RR::PerformanceAssessment m_lastPerformanceAssessment;
    bool m_collecting = false;
    uint m_tick;

    void updateStats(RR::ApiHandle<RR::AzureSession> session);
};
