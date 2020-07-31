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

    virtual int getCount() const = 0;
    virtual QString getName(int index) const = 0;
    virtual QVariant getValue(int index) const = 0;

    int getIndex() const;
    void setIndex(int index);
};


class ComboBoxModelFromEnum : public ComboBoxModel
{
    Q_OBJECT

public:
    ComboBoxModelFromEnum(QString name, QObject* object, const std::string_view& parameterName);

    int getCount() const override;
    QString getName(int index) const override;
    QVariant getValue(int index) const override;

private:
    QMetaEnum m_enumerator;
};

class ComboBoxModelFromMap : public ComboBoxModel
{
    Q_OBJECT

public:
    using ComboBoxModel::ComboBoxModel;

    void addEntry(QString name, QVariant value);

    int getCount() const override;
    QString getName(int index) const override;
    QVariant getValue(int index) const override;

private:
    struct Entry
    {
        QString m_label;
        QVariant m_value;
    };
    std::vector<Entry> m_entries;
};
