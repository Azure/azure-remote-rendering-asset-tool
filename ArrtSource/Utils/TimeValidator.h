#pragma once

#include <QValidator>

/// implementation of QValidator used for time in the format HH:MM.
class TimeValidator : public QValidator
{
public:
    TimeValidator(QObject* parent = nullptr);

    static QString minutesToString(int minutes);
    static int stringToMinutes(const QString& s);

    virtual void fixup(QString& input) const override;
    virtual QValidator::State validate(QString& input, int& pos) const override;
};
