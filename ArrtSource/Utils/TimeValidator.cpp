#include <Utils/TimeValidator.h>

TimeValidator::TimeValidator(QObject* parent)
    : QValidator(parent)
{
}

void TimeValidator::fixup(QString& input) const
{
    int minutes = stringToMinutes(input);
    input = minutesToString(minutes);
}

QValidator::State TimeValidator::validate(QString& input, int& /*pos*/) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList numbers = input.split(tr(":"), Qt::SplitBehaviorFlags::SkipEmptyParts);
#else
    QStringList numbers = input.split(tr(":"), QString::SkipEmptyParts);
#endif

    if (numbers.size() > 2)
    {
        return State::Invalid;
    }

    bool acceptable = true;

    for (int i = 0; i < numbers.size(); ++i)
    {
        const QString& s = numbers[numbers.size() - i - 1];
        bool ok;
        int number = s.toInt(&ok);
        if (ok)
        {
            if (number < 0)
            {
                return State::Invalid;
            }
            if (
                (number >= 60 && i == 0))
            {
                acceptable = false;
            }
        }
        else
        {
            if (s.isEmpty())
            {
                acceptable = false;
            }
            else
            {
                return State::Invalid;
            }
        }
    }
    if (numbers.size() != 2)
    {
        acceptable = false;
    }

    return acceptable ? State::Acceptable : State::Intermediate;
}

QString TimeValidator::minutesToString(int minutes)
{
    return tr("%1:%2").arg(minutes / 60, 1, 10, QChar('0')).arg(minutes % 60, 2, 10, QChar('0'));
}

int TimeValidator::stringToMinutes(const QString& s)
{
    int minutes = 0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList numbers = s.split(tr(":"), Qt::SplitBehaviorFlags::SkipEmptyParts);
#else
    QStringList numbers = s.split(tr(":"), QString::SkipEmptyParts);
#endif
    if (numbers.size() >= 2)
    {
        minutes += numbers[numbers.size() - 2].toInt() * 60;
    }
    if (numbers.size() >= 1)
    {
        minutes += numbers[numbers.size() - 1].toInt();
    }
    return minutes;
}
