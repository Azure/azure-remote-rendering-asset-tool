#include <Model/Configuration.h>
#include <QDebug>
#include <ViewModel/Session/SessionCreationModel.h>

SessionCreationModel::SessionCreationModel(ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent)
    : SessionModel(sessionManager, configuration, parent)
{
    m_size = static_cast<Size>(m_configuration->getUiState("sessionCreation:size", (int)Size::Standard));
    m_leaseTime = m_configuration->getUiState("sessionCreation:leaseTime", Time(1, 0).m_totalMinutes);
    m_extendAutomatically = m_configuration->getUiState("sessionCreation:extendAutomatically", true);
    m_extensionMinutes = m_configuration->getUiState("sessionCreation:extensionMinutes", 30);

    connect(m_sessionManager, &ArrSessionManager::onEnabledChanged, this, [this]() {
        Q_EMIT onEnabledChanged();
    });
}

SessionCreationModel::~SessionCreationModel()
{
    m_configuration->setUiState("sessionCreation:size", (int)m_size);
    m_configuration->setUiState("sessionCreation:leaseTime", m_leaseTime.m_totalMinutes);
    m_configuration->setUiState("sessionCreation:extendAutomatically", m_extendAutomatically);
    m_configuration->setUiState("sessionCreation:extensionMinutes", m_extensionMinutes);
}

SessionModel::Size SessionCreationModel::getSize() const
{
    return m_size;
}

void SessionCreationModel::setSize(Size size)
{
    m_size = size;
}

SessionModel::Time SessionCreationModel::getLeaseTime() const
{
    return m_leaseTime;
}

void SessionCreationModel::setLeaseTime(const Time& leaseTime)
{
    m_leaseTime = leaseTime;
}

bool SessionCreationModel::isAutomaticallyExtended() const
{
    return m_extendAutomatically;
}

void SessionCreationModel::setAutomaticallyExtended(bool autoExtension)
{
    m_extendAutomatically = autoExtension;
}

SessionModel::Time SessionCreationModel::getExtensionTime() const
{
    return m_extensionMinutes;
}

void SessionCreationModel::setExtensionTime(Time extensionTime)
{
    m_extensionMinutes = extensionTime.m_totalMinutes;
}

// try to start a session
bool SessionCreationModel::start()
{
    if (!isRunning())
    {
        RR::RenderingSessionCreationOptions param;
        param.Size = RR::RenderingSessionVmSize(m_size);
        param.MaxLeaseInMinutes = (int)(m_leaseTime.getHours() * 60 + m_leaseTime.getMinutes());

        //set also the automatic extension
        m_sessionManager->setExtensionTime(m_extensionMinutes, m_extendAutomatically);

        const bool succeeded = m_sessionManager->startSession(param);
        changed();
        if (!succeeded)
        {
            qWarning() << tr("Session didn't successfully start");
        }
        return succeeded;
    }
    else
    {
        // already running. Cannot start again
        return false;
    }
}

bool SessionCreationModel::isEnabled() const
{
    return m_sessionManager->isEnabled();
}
