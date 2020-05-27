#pragma once

#include <QObject>

// ARR account settings.
// See Microsoft::Azure::RemoteRendering::Internal::ClientInit for the documentation on the ARR account parameters

class ArrAccountSettings : public QObject
{
    Q_OBJECT

public:
    enum class Region
    {
        westus2,
        eastus,
        westeurope,
        southeastasia
#ifdef EXTRA_ARR_ZONE
        ,
        EXTRA_ARR_ZONE
#endif
    };
    Q_ENUM(Region);

private:
    Q_PROPERTY(QString id MEMBER m_id NOTIFY changed);
    Q_PROPERTY(QString key READ getKey WRITE setKey);
    Q_PROPERTY(Region region MEMBER m_region NOTIFY changed);

public:
    ArrAccountSettings(QObject* parent);

    const QString& getId() const { return m_id; }
    QString getKey() const;
    bool setKey(QString key);
    Region getRegion() const { return m_region; }

    void loadFromJson(const QJsonObject& arrAccountConfig);
    QJsonObject saveToJson() const;

Q_SIGNALS:
    void changed();

private:
    QString m_id = {};
    QString m_key = {};
    Region m_region = Region::westeurope;
};
