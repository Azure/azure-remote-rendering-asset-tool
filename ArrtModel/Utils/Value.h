#pragma once
#include <QVariant>

// class wrapping a QVariant value, which will emit a signal when the value is set
class ValueVariant : public QObject
{
    Q_OBJECT
public:
    ValueVariant(QVariant value, QObject* parent = nullptr)
        : QObject(parent)
    {
        m_value = std::move(value);
    }

    QVariant get() const { return m_value; }
    void set(const QVariant& value)
    {
        if (m_value != value)
        {
            m_value = value;
            Q_EMIT valueChanged();
        }
    }

signals:
    void valueChanged();

private:
    QVariant m_value;
};

// class wrapping a T value, which will emit a signal when the value is set

template <typename T>
class Value : public ValueVariant
{
public:
    Value(T value, QObject* parent = nullptr)
        : ValueVariant(QVariant::fromValue(value), parent)
    {
    }

    T get() const { return ValueVariant::get().template value<T>(); }
    void set(const T& value) const
    {
        ValueVariant::set(QVariant::fromValue(value));
    }
    void set(T value)
    {
        ValueVariant::set(QVariant::fromValue(std::move(value)));
    }
};
