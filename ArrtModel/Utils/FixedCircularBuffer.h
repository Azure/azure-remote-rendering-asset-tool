#pragma once

// simple fixed size circular buffer

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
