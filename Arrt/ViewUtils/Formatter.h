#pragma once

#include <QCursor>
#include <QSpinBox>
#include <QString>

// utility class to convert an integer to/from a formatted QString

class IntFormatter
{
public:
    IntFormatter(QString format = {})
    {
        setFormat(std::move(format));
    }

    void setFormat(QString format)
    {
        m_format = format.isEmpty() ? QString("%d") : std::move(format);
    }

    QString textFromValue(int value) const
    {
        QString result;
        return result.sprintf(m_format.toUtf8().data(), value);
    }

    int valueFromText(const QString& text, bool* ok) const
    {
        return text.toInt(ok);
    }

protected:
    QString m_format;
};

// utility class to convert a double to/from a formatted QString

class DoubleFormatter
{
public:
    DoubleFormatter(QString format = {})
        : m_removeTrailingZeros(false)
    {
        setFormat(std::move(format));
    }

    void setFormat(QString format)
    {
        m_format = format.isEmpty() ? QString("%.3fm") : std::move(format);
        if (m_format.endsWith("m"))
        {
            // m at the end of the format will remove trailing zeros at the end of the decimal part
            m_format.chop(1);
            m_removeTrailingZeros = true;
        }
    }

	static QString toString(double value, const char* format, bool removeTrailingDecimals)
	{
		QString result;
        result.sprintf(format, value);
        if (removeTrailingDecimals && !result.isEmpty())
        {
            //find the first non zero digit at the end of the formatted number
            int lastValidDigit = result.size() - 1;
            while (lastValidDigit >= 0 && result[lastValidDigit] == '0')
            {
                --lastValidDigit;
            }

            if (lastValidDigit >= 0)
            {
                //check if there is a dot before the last non zero digit
                int dotIndex;
                for (dotIndex = lastValidDigit; dotIndex >= 0 && result[dotIndex] != '.'; --dotIndex)
                {
                }

                if (dotIndex >= 0)
                {
                    if (dotIndex == lastValidDigit)
                    {
                        //no decimal digits: remove the dot
                        lastValidDigit--;
                    }

                    result.chop(result.size() - 1 - lastValidDigit);
                }
            }
        }
		return result;
	}

    QString textFromValue(double value) const
    {
        return  toString(value, m_format.toUtf8().data(), m_removeTrailingZeros);
    }

    double valueFromText(const QString& text, bool* ok) const
    {
        return text.toDouble(ok);
    }

protected:
    QString m_format;
    bool m_removeTrailingZeros;
};

// A formatter that can handle both double and int format strings.

class NumberFormatter
{
public:
    enum FormatterType
    {
        DOUBLE_FORMAT,
        INTEGER_FORMAT
    };

    NumberFormatter(QString format, FormatterType type)
    {
        setFormat(std::move(format), type);
    }

    void setFormat(QString format, FormatterType type)
    {
        m_formatterType = type;
        switch (m_formatterType)
        {
            case DOUBLE_FORMAT:
            {
                m_doubleFormatter.setFormat(std::move(format));
                break;
            }
            case INTEGER_FORMAT:
            {
                m_intFormatter.setFormat(std::move(format));
                break;
            }
        }
    }

    FormatterType getFormatterType()
    {
        return m_formatterType;
    }

    QString textFromValue(double value) const
    {
        switch (m_formatterType)
        {
            case DOUBLE_FORMAT:
            {
                return m_doubleFormatter.textFromValue(value);
            }
            case INTEGER_FORMAT:
            default:
            {
                return m_intFormatter.textFromValue((int)std::round(value));
            }
        }
    }

    double valueFromText(const QString& text, bool* ok) const
    {
        switch (m_formatterType)
        {
            case DOUBLE_FORMAT:
            {
                return m_doubleFormatter.valueFromText(text, ok);
            }
            case INTEGER_FORMAT:
            default:
            {
                return m_intFormatter.valueFromText(text, ok);
            }
        }
    }

protected:
    FormatterType m_formatterType;
    DoubleFormatter m_doubleFormatter;
    IntFormatter m_intFormatter;
};
