#pragma once
#include <QLineEdit>

// control used to enter/view time, formatted as HH:SS

class HoursMinutesControl : public QLineEdit
{
public:
    HoursMinutesControl();

    void setMinutes(int minutes);
    int getMinutes() const;
};
