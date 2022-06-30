#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <Rendering/IncludeAzureRemoteRendering.h>

class QTimer;

class ArrConnectionLogic : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void ConnectionStateChanged();
    void InitGraphics();
    void DeinitGraphics();

public:
    enum class State
    {
        // inactive states
        Inactive,
        Error,
        Stopped,
        Expired,

        // active states
        OpeningSession,
        SessionOpen,
        RuntimeConnecting,
        RuntimeConnected,
        Disconnecting,
    };

    ArrConnectionLogic();
    ~ArrConnectionLogic();

    State GetCurrentState() const { return m_currentState; }

    /// Whether the connection is busy in any way (even disconnecting).
    bool IsConnectionActive() const;

    /// Whether the connection can be stopped (e.g. is not already stopping).
    bool IsConnectionStoppable() const;

    /// Whether the connection is in a fully operating state.
    bool IsConnectionRendering() const;

    void CreateNewSession(RR::ApiHandle<RR::RemoteRenderingClient>& client, const RR::RenderingSessionCreationOptions& info);
    void OpenExistingSession(RR::ApiHandle<RR::RemoteRenderingClient>& client, const QString& sessionID);
    void CloseSession(bool keepRunning);

    int GetElapsedTimeInMinutes() const { return m_elapsedTimeInMinutes; }
    int GetLeaseTimeInMinutes() const { return m_leaseTimeInMinutes; }

    RR::ApiHandle<RR::RenderingSession> GetArrSession() const;
    RR::ApiHandle<RR::RenderingConnection> GetArrRenderingConnection();

    static QString GetStateString(ArrConnectionLogic::State state);

private:
    State m_currentState = State::Inactive;
    int m_elapsedTimeInMinutes = 0;
    int m_leaseTimeInMinutes = 0;

    RR::ApiHandle<RR::RenderingSession> m_arrSession = nullptr;
    RR::event_token m_arrConnectionStatusChangedToken;
    RR::event_token m_messageLoggedToken;
    QTimer* m_updateSessionPropertiesTimer = nullptr;
    QElapsedTimer m_connectToRuntimeElapsedTime;

    std::atomic_bool m_updateSessionPropertiesInProgress = false;

private:
    void OpenOrCreateSessionResult(RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result);
    void OnArrConnectionStatusChanged(RR::ConnectionStatus status, RR::Result result);
    void ConfigureSessionPropertiesUpdateTimer();
    void UpdateSessionProperties();
    void SessionPropertiesResult(RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result);
    void UpdateState(const RR::RenderingSessionProperties& properties, RR::ConnectionStatus connection);
    void ConnectToRuntime();
    void ConnectToRuntimeResult(RR::Status status, RR::ConnectionStatus result);
    void DisconnectFromRuntime();
};