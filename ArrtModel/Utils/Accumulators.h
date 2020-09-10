#pragma once
#include <Utils/FixedCircularBuffer.h>

// accumulator holding the minimum

template <typename T>
struct MinValue
{
    void addValue(const T& value)
    {
        m_value = qMin(m_value, value);
    }
    bool hasValue() const
    {
        return m_value != std::numeric_limits<T>::max();
    }
    T m_value = std::numeric_limits<T>::max();
};

// accumulator holding the maximum

template <typename T>
struct MaxValue
{
    void addValue(const T& value)
    {
        m_value = qMax(m_value, value);
    }
    bool hasValue() const
    {
        return m_value != std::numeric_limits<T>::min();
    }
    T m_value = std::numeric_limits<T>::min();
};

// accumulator holding the sum

template <typename T>
struct SumValue
{
    void addValue(const T& value)
    {
        m_value += value;
    }
    bool hasValue() const
    {
        return m_value != 0;
    }
    T m_value = {};
};

// accumulator holding the sum and average

template <typename T>
struct AvgValue
{
    void addValue(const T& value)
    {
        m_value += value;
        ++m_values;
    }

    bool hasValue() const
    {
        return m_values > 0;
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

// accumulator holding the minimum, maximum and average

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

    bool hasValue() const
    {
        return m_values > 0;
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

// accumulator storing the values in a circular buffer and storing the accumulated values per "window" and total stats

template <typename T>
struct AccumulatorWithStorage
{
    typedef decltype(T::m_value) ValueType;
    struct GraphValue
    {
        ValueType m_value;
        uint m_tick;
    };

    T m_accumulation = {};
    FixedCircularBuffer<GraphValue, 2048> m_buffer;

    FixedCircularBuffer<GraphValue, 1024> m_perWindowBuffer;
    AvgMinMaxValue<float> m_perWindowStats;

    void addValue(const ValueType& value, uint tick)
    {
        m_accumulation.addValue(value);
        m_buffer.addFront({value, tick});
    }

    void getGraphData(std::vector<QPointF>& graph, bool perWindow) const
    {
        graph.clear();
        graph.reserve(m_perWindowBuffer.getSize());

        if (perWindow)
        {
            for (uint i = 0; i < m_perWindowBuffer.getSize(); ++i)
            {
                auto val = m_perWindowBuffer.getValue(i);
                graph.push_back({(qreal)val.m_tick, (qreal)val.m_value});
            }
        }
        else
        {
            for (uint i = 0; i < m_buffer.getSize(); ++i)
            {
                auto val = m_buffer.getValue(i);
                graph.push_back({(qreal)val.m_tick, (qreal)val.m_value});
            }
        }
    }

    void endWindow(uint tick)
    {
        if (m_accumulation.hasValue())
        {
            m_perWindowStats.addValue(m_accumulation.m_value);
            m_perWindowBuffer.addFront({m_accumulation.m_value, tick});
        }
        m_accumulation = {};
    }
};