#pragma once

// Small class used to associate a string to a boolean. Used normally as a return value
// when you want to embed the reason for which the operation failed (to display it in the UI for example)

class BoolWithReason
{
public:
    BoolWithReason(bool value, QString reason = {})
        : m_value(value)
        , m_reason(std::move(reason))
    {
    }

    BoolWithReason(const BoolWithReason&) = default;
    BoolWithReason(BoolWithReason&&) = default;
    BoolWithReason& operator=(const BoolWithReason&) = default;
    BoolWithReason& operator=(BoolWithReason&&) = default;

    operator bool() { return m_value; }
    bool getValue() const { return m_value; }
    const QString& getReason() const { return m_reason; }

private:
    bool m_value;
    QString m_reason;
};
