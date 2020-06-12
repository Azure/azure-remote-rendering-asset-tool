#pragma once
#include <QValidator>

// implementation of QValidator used for time in the format HH:MM. Used by HoursMinutesControl

class TimeValidator : public QValidator
{
public:
    TimeValidator(QObject* parent = nullptr);

    virtual void fixup(QString& input) const override;

    virtual QValidator::State validate(QString& input, int& pos) const override;
    static QString minutesToString(int minutes);

    static int stringToMinutes(const QString& s);
};
