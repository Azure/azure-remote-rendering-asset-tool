#include <Utils/TimeValidator.h>
#include <Widgets/HoursMinutesControl.h>

HoursMinutesControl::HoursMinutesControl()
{
    setValidator(new TimeValidator(this));
    setText(TimeValidator::minutesToString(0));
}

void HoursMinutesControl::setMinutes(int minutes)
{
    setText(TimeValidator::minutesToString(minutes));
}

int HoursMinutesControl::getMinutes() const
{
    return TimeValidator::stringToMinutes(text());
};
