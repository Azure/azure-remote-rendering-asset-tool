#pragma once
#include <Model/IncludesAzureRemoteRendering.h>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <functional>

class Configuration;
namespace Microsoft::Azure::RemoteRendering
{
    class AzureSession;
    class RemoteRenderingClient;
    using LoadResult = std::function<void(Status, ApiHandle<Entity>)>;
    using LoadProgress = std::function<void(float)>;
} // namespace Microsoft::Azure::RemoteRendering

class ArrFrontend;
class ViewportModel;
class ArrServiceStats;

// struct with information on the session which are determined on creation and don't change over time

struct SessionDescriptor
{
    // these parameters are retrieved from RR::RenderingSession
    std::string m_hostName;
    RR::RenderingSessionVmSize m_size = RR::RenderingSessionVmSize::Standard;
    int m_maxLeaseTimeInMinutes = 0;
};

// struct holding the status of the session, which needs to be polled regularly

struct SessionStatus
{
    enum class Status
    {
        NotActive,
        Stopped,
        Expired,
        Error,

        // this is only to separate non running states from running states
        RunningMarker = 100,

        StartRequested = RunningMarker,
        Starting,
        ReadyNotConnected,
        ReadyConnecting,
        ReadyConnected,
        StopRequested
    };

    bool isRunning() const
    {
        return m_status >= Status::RunningMarker;
    }

    Status m_status = Status::NotActive;

    // these parameters are retrieved from RR::RenderingSession

    int m_elapsedTimeInMinutes = 0;
    std::string m_sessionMessage;
};

// Manager for the remote rendering sessions. It will handle only one session at a time, exposing its status and allowing start/stop/lease extension.
// It will try to connect whenever the session is ready, and the current session is persisted in the configuration, to be restored when the application
// is closed and restarted

class ArrSessionManager : public QObject
{
    Q_OBJECT

public:
    ArrSessionManager(ArrFrontend* frontEnd, Configuration* configuration, QObject* parent);
    ~ArrSessionManager();

    // start a session. It only works if no session is currently running
    bool startSession(const RR::RenderingSessionCreationOptions& info);
    // stop a session. It only works if the session is running, or it's starting
    bool stopSession();

    // return the status of the active session, if there is one. Otherwise a default constructed status
    SessionStatus getSessionStatus() const;

    // return the descriptor of the active session (name/size/lease time/expiration). If no session is active, returns the default constructed descriptor
    SessionDescriptor getSessionDescriptor() const;

    void setExtensionTime(uint minutesToAdd, bool extendAutomatically);
    void getExtensionTime(uint& outMinutesToAdd, bool& outExtendAutomatically) const;

    int getRemainingMinutes() const;

    // extend the lease time asynchronously based on the extension time set with setExtensionTime, and call the callback on completion
    bool extendMaxSessionTime();

    // Result of the currently loaded model
    RR::ApiHandle<RR::LoadModelResult> loadedModel() const { return m_loadedModel; }

    // Load a model asynchronously. Unloads the currently loaded model, and on completion the RootID will be changed
    RR::Result loadModelAsync(const QString& modelName, const char* assetSAS, RR::LoadResult result, RR::LoadProgress progressCallback = {});

    // Unload the currently loaded model
    void unloadModel();

    // return the current loaded model name
    QString getModelName() const;

    // return the model used for the viewport
    ViewportModel* getViewportModel() const { return m_viewportModel; }

    // return the object used to retrieve remote rendering statistics on the current session
    ArrServiceStats* getServiceStats() const { return m_serviceStats; }

    RR::ApiHandle<RR::RenderingConnection> getClientApi();

    // start the arrInspector on the running session
    void startInspector();

    bool isEnabled() const { return m_isEnabled; }

    void reconnectToSessionRuntime();

    RR::Status getLastError() const { return m_lastError; }

    // return the string of the current session uuid
    std::string getSessionUuid() const;

    // return the current session
    RR::ApiHandle<RR::RenderingSession> getCurrentSession() const;

    bool getAutoRotateRoot() const;
    void setAutoRotateRoot(bool autoRotateRoot);

Q_SIGNALS:
    void onEnabledChanged();
    void changed();

    // emitted when the root ID is changed because the current loaded model changed
    void rootIdChanged();

    void sessionAboutToChange();
    void sessionChanged();
    void autoRotateRootChanged();

private:
    static SessionStatus::Status convertStatus(RR::RenderingSessionStatus status);

    void setLoadedModel(RR::ApiHandle<RR::LoadModelResult> loadResult);

    // called from the main thread, whenever the status is returned by service.
    void updateSessionProperties(RR::RenderingSessionProperties props);

    // called from the main thread, whenever the status is changed
    void onStatusUpdated();

    // set the current running session, which is also persisted in the configuration
    void setRunningSession(const RR::ApiHandle<RR::RenderingSession>& session);

    // update the status asynchronously and execute the callback when the status is updated
    // It has to be called from the main thread
    void updateStatus();

    void connectToSessionRuntime();
    void disconnectFromSessionRuntime();

    void initializeSession();
    void deinitializeSession();

    // start/stop internal timers based on the session state
    void updateTimers();

    bool m_isEnabled = false;

    ArrFrontend* const m_frontend;
    Configuration* const m_configuration;
    RR::ApiHandle<RR::RenderingConnection> m_api = nullptr;
    RR::ApiHandle<RR::RenderingSession> m_session = nullptr;

    QElapsedTimer m_connectingElapsedTime;

    // cached properties coming from RR::RenderingSession::GetRenderingSessionPropertiesAsync
    RR::RenderingSessionProperties m_lastProperties = {};

    int m_extensionMinutes;
    bool m_extendAutomatically;

    QTimer* m_updateTimer = nullptr;

    RR::ApiHandle<RR::LoadModelResult> m_loadedModel = nullptr;

    QString m_modelName;

    ViewportModel* m_viewportModel;

    ArrServiceStats* m_serviceStats;

    // Async status
    std::atomic_bool m_renewAsyncInProgress = false;
    std::atomic_bool m_connectToArrInspectorInProgress = false;
    std::atomic_bool m_createSessionInProgress = false;
    std::atomic_bool m_stopRequestInProgress = false;
    std::atomic_bool m_connectingInProgress = false;

    // Registered callback tokens
    RR::event_token m_statusChangedToken;
    RR::event_token m_messageLoggedToken;

    bool m_reconnecting = false;
    bool m_waitForVideoFormatChange = false;
    RR::Status m_lastError = RR::Status::OK;

    bool m_autoRotateRoot = false;
};
