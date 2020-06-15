#pragma once

#include <QObject>

// Model used by the view to configure ARR rendering session video settings.

class VideoSettings : public QObject
{
private:
    Q_OBJECT

private:
    // Qt reflected properties, wrapping each field in Config
    Q_PROPERTY(int width MEMBER m_width WRITE setWidth);
    Q_PROPERTY(int height MEMBER m_height WRITE setHeight);
    Q_PROPERTY(int refreshRate MEMBER m_refreshRate NOTIFY changed);

public:
    VideoSettings(QObject* parent);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getRefreshRate() const { return m_refreshRate; }

    void operator=(const VideoSettings& rhs)
    {
        if (!(*this == rhs))
        {
            m_width = rhs.m_width;
            m_height = rhs.m_height;
            m_refreshRate = rhs.m_refreshRate;
            Q_EMIT changed();
        }
    }

    bool operator==(const VideoSettings& rhs) const
    {
        return m_width == rhs.m_width && m_height == rhs.m_height && m_refreshRate == rhs.m_refreshRate;
    }

    void loadFromJson(const QJsonObject& videoConfig);
    QJsonObject saveToJson() const;

Q_SIGNALS:
    void changed();
    void updateUi();

public:
    static constexpr int s_widthMin = 256;
    static constexpr int s_widthMax = 4096;
    static constexpr int s_resolutionStep = 16;
    static constexpr int s_heightMin = 256;
    static constexpr int s_heightMax = 2160;
    static constexpr int s_refreshRateMin = 30;
    static constexpr int s_refreshRateMax = 60;

private:
    int m_width = 1920;
    int m_height = 1080;
    int m_refreshRate = 60;

private:
    void setWidth(int width);
    void setHeight(int height);
};
