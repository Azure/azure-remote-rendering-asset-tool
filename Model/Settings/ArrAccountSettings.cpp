#include <Model/Settings/ArrAccountSettings.h>
#include <QMetaEnum>
#include <QMetaObject>
#include <Utils/JsonUtils.h>

ArrAccountSettings::ArrAccountSettings(QObject* parent)
    : QObject(parent)
{
}

void ArrAccountSettings::loadFromJson(const QJsonObject& arrAccountConfig)
{
    m_id = arrAccountConfig[QLatin1String("id")].toString();
    m_key = arrAccountConfig[QLatin1String("key")].toString();

    bool ok;
    const int value = QMetaEnum::fromType<Region>().keyToValue(arrAccountConfig[QLatin1String("region")].toString().toUtf8().data(), &ok);
    if (ok)
    {
        m_region = static_cast<Region>(value);
    }
    else
    {
        m_region = Region::westeurope;
    }
}

QJsonObject ArrAccountSettings::saveToJson() const
{
    QJsonObject arrAccountConfig;
    arrAccountConfig[QLatin1String("id")] = m_id;
    arrAccountConfig[QLatin1String("key")] = m_key;
    arrAccountConfig[QLatin1String("region")] = QMetaEnum::fromType<Region>().valueToKey(static_cast<int>(m_region));
    return arrAccountConfig;
}
