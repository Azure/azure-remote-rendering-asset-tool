#pragma once
#include <QVariant>
#include <ViewModel/Parameters/ParameterModel.h>

// Model for a float with 2 to 4 parameter, using RR::Float2/3/4 as the backend

class FloatVectorModel : public ParameterModel
{
    Q_OBJECT

public:
    FloatVectorModel(QString name, QObject* object, const std::string_view& parameterName);
    void setLabels(QString label1, QString label2, QString label3 = {}, QString label4 = {});
    virtual int getCount() const = 0;

    void setFormat(QString format);
    QString getFormat() const;

    virtual float getValue(int index) const = 0;
    virtual void setValue(int index, float value) = 0;
    QString getLabel(int index) const;

private:
    QString m_labels[4];
    QString m_format = QString("%.3fm");
};

class Float2Model : public FloatVectorModel
{
public:
    Float2Model(QString name, QObject* object, const std::string_view& parameterName,
                QString label1 = tr("X"), QString label2 = tr("Y"));

    virtual int getCount() const override { return 2; }
    virtual float getValue(int index) const override;
    virtual void setValue(int index, float value) override;
};


class Float3Model : public FloatVectorModel
{
public:
    Float3Model(QString name, QObject* object, const std::string_view& parameterName,
                QString label1 = tr("X"),
                QString label2 = tr("Y"),
                QString label3 = tr("Z"));

    virtual int getCount() const override { return 3; }
    virtual float getValue(int index) const override;
    virtual void setValue(int index, float value) override;
};


class Float4Model : public FloatVectorModel
{
public:
    Float4Model(QString name, QObject* object, const std::string_view& parameterName,
                QString label1 = tr("X"),
                QString label2 = tr("Y"),
                QString label3 = tr("Z"),
                QString label4 = tr("W"));

    virtual int getCount() const override { return 4; }
    virtual float getValue(int index) const override;
    virtual void setValue(int index, float value) override;
};
