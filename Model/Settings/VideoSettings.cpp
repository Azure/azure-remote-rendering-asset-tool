#include <Model/Settings/VideoSettings.h>
#include <Utils/JsonUtils.h>

VideoSettings::VideoSettings(QObject* parent)
    : QObject(parent)
{
}
void VideoSettings::loadFromJson(const QJsonObject& videoConfig)
{
    setWidth(videoConfig[QLatin1String("width")].toInt());
    setHeight(videoConfig[QLatin1String("height")].toInt());
    m_refreshRate = JsonUtils::fromJson(videoConfig, QLatin1String("fps"), m_refreshRate);
    m_refreshRate = std::clamp(m_refreshRate, s_refreshRateMin, s_refreshRateMax);
}

QJsonObject VideoSettings::saveToJson() const
{
    QJsonObject videoConfig;
    videoConfig[QLatin1String("width")] = m_width;
    videoConfig[QLatin1String("height")] = m_height;
    videoConfig[QLatin1String("fps")] = m_refreshRate;
    return videoConfig;
}

namespace
{
    // Restriction in HAR: width and height have to be multiple of 16
    // Returns true if valueOut was changed
    bool setMultipleOf16(int valueIn, int& valueOut)
    {
        if (valueIn != valueOut)
        {
            valueIn &= ~(0xf);
            if (valueIn != valueOut)
            {
                valueOut = valueIn;
                return true;
            }
        }
        return false;
    }
} // namespace

void VideoSettings::setWidth(int width)
{
    if (setMultipleOf16(std::clamp(width, s_widthMin, s_widthMax), m_width))
    {
        Q_EMIT changed();
    }
    if (m_width != width)
    {
        Q_EMIT updateUi();
    }
}

void VideoSettings::setHeight(int height)
{
    if (setMultipleOf16(std::clamp(height, s_heightMin, s_heightMax), m_height))
    {
        Q_EMIT changed();
    }
    if (m_height != height)
    {
        Q_EMIT updateUi();
    }
}
