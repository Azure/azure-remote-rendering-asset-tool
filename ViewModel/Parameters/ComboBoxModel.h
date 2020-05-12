#pragma once
#include <QColor>
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// model for an enum parameter, converting an int enum to a Qt reflected enum

class ComboBoxModel : public ParameterModel
{
    Q_OBJECT

public:
    ComboBoxModel(QString name, QObject* object, const std::string_view& parameterName);

    int getCount() const;
    QString getName(int index) const;
    int getValue(int index) const;

    int getIndex() const;
    void setIndex(int index);

private:
    QMetaEnum m_enumerator;
    int m_index = 0;
};
