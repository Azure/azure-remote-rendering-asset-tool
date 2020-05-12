#pragma once
#include <QMetaProperty>
#include <QObject>

// base class for all of the control models for parameters

class ParameterModel : public QObject
{
    Q_OBJECT
public:
    ParameterModel(QString name, QObject* object, const std::string_view& parameterName);

    const QString& getName() const { return m_name; }

signals:
    void changed();

protected:
    const QString m_name;
    QObject* const m_object;

    QVariant getValue() const;
    void setValue(const QVariant& value);

    const QMetaProperty m_metaProperty;

private:
    QMetaProperty getMetaProperty(QObject* object, const std::string_view& parameterName);
};
