#pragma once

#include <ViewModel/Parameters/ParameterModel.h>

// model representing flags. The property passed needs to be an enum type with values that are single bits, stored as an int.

class MultiComboBoxModel : public ParameterModel
{
    Q_OBJECT

public:
    MultiComboBoxModel(QString name, QObject* object, const std::string_view& parameterName);

    int getCount() const;
    QString getEnumName(int index) const;
    int getEnumValue(int index) const;

    int getMask() const;
    void setMask(int mask);

public:
    QMetaEnum m_enumerator;
};
