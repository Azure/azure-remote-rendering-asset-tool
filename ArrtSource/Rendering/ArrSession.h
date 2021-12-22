#pragma once

#include <QObject>

#include <QElapsedTimer>
#include <Rendering/IncludeAzureRemoteRendering.h>
#include <deque>

namespace Microsoft::Azure::RemoteRendering
{
    class AzureSession;
    class RemoteRenderingClient;
    using LoadResult = std::function<void(Status, ApiHandle<Entity>)>;
    using LoadProgress = std::function<void(float)>;
} // namespace Microsoft::Azure::RemoteRendering


class ArrAccount;
class QTimer;
class SceneState;

struct ArrSessionStatus
{
    enum class State
    {
        NotActive,
        Stopped,
        Expired,
        Error,

        StartRequested,
        Starting,
        ReadyNotConnected,
        ReadyConnecting,
        ReadyConnected,
    };

    bool IsRunning() const;

    State m_state = State::NotActive;
    int m_elapsedTimeInMinutes = 0;
    int m_leaseTimeInMinutes = 0;
};

/// Manages all aspects of a Remote Rendering session
///
/// This includes session startup / shutdown, connect / disconnect and model loading.
class ArrSession : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void SessionStatusChanged();
    void ModelLoaded();
    void ModelLoadProgressChanged();
    void FrameStatisticsChanged();

public:
    ArrSession(ArrAccount* arrClient, SceneState* sceneState);
    ~ArrSession();

    /// Creates a new session with the provided options.
    void CreateSession(const RR::RenderingSessionCreationOptions& info);

    /// Attempts to open an already existing session with the given session ID.
    void OpenSession(const QString& sessionID);

    /// Disconnects from the current session and optionally stops the session as well.
    void CloseSession(bool keepRunning);

    ArrSessionStatus GetSessionStatus() const;

    /// Returns the session ID of the currently running session.
    QString GetSessionID() const;

    /// Starts ARR Inspector in a browser.
    void StartArrInspector();

    /// Updates the session lease time to the given duration.
    ///
    /// This may be shorter or longer than the original lease time.
    void ChangeSessionLeaseTime(int totalLeaseMinutes);

    /// Sets the auto-extension time in minutes. Whenever a session has less remaining time than this, it gets extended.
    void SetAutoExtensionMinutes(int extendByMinutes);

    struct LoadedModel
    {
        QString m_ModelName;
        RR::ApiHandle<RR::LoadModelResult> m_LoadResult;
    };

    /// Loads the model from the provided SAS URL.
    bool LoadModel(const QString& modelName, const char* assetSAS);

    /// Removes the previously loaded model with the given index.
    void RemoveModel(size_t idx);

    /// Returns the 'total' loading progress of all currently loading models.
    float GetModelLoadingProgress() const;

    /// Returns all currently loaded models.
    const std::deque<LoadedModel>& GetLoadedModels() const { return m_loadedModels; }

    /// Changes the scale of all loaded models.
    void SetModelScale(float scale);

    /// Enables or removes the selection outline on the given entity.
    void SetEntitySelected(const RR::ApiHandle<RR::Entity>& entity, bool enable);

    /// Returns all entities that are currently 'selected'.
    void GetSelectedEntities(std::vector<RR::ApiHandle<RR::Entity>>& selected) const;

    const RR::FrameStatistics& GetFrameStatistics() const { return m_frameStats; }

private:
    void OpenOrCreateSessionResult(RR::Status status, RR::ApiHandle<RR::CreateRenderingSessionResult> result);
    void ConnectToSessionRuntime();
    void ConnectToSessionRuntimeResult(RR::Status status, RR::ConnectionStatus result);
    void DisconnectFromSessionRuntime();
    void GetSessionPropertiesResult(RR::Status status, RR::ApiHandle<RR::RenderingSessionPropertiesResult> result);
    void OnSessionPropertiesUpdated();
    void OnConnectionStatusChanged(RR::ConnectionStatus status, RR::Result result);
    void ExtendSessionIfNeeded();
    void OnSceneRefresh();
    void UpdateSessionProperties();
    void UpdatePerformanceStatistics();
    void ConfigureSessionPropertiesUpdateTimer();
    void CheckEntityBounds(RR::ApiHandle<RR::Entity> entity);
    void CheckEntityBoundsResult(RR::Bounds bounds);
    RR::ApiHandle<RR::RenderingConnection> GetRenderingConnection();

    ArrAccount* m_arrClient = nullptr;
    RR::ApiHandle<RR::RenderingSession> m_arrSession = nullptr;

    int m_extensionMinutes = 0;

    SceneState* m_sceneState = nullptr;

    QElapsedTimer m_connectingElapsedTime;
    QTimer* m_updateSessionPropertiesTimer = nullptr;

    std::atomic_bool m_createSessionInProgress = false;
    std::atomic_bool m_connectingInProgress = false;
    std::atomic_bool m_connectToArrInspectorInProgress = false;
    std::atomic_bool m_renewAsyncInProgress = false;

    RR::RenderingSessionProperties m_lastProperties = {};
    RR::event_token m_statusChangedToken;
    RR::event_token m_messageLoggedToken;

    float m_modelScale = 1.0f;
    std::vector<float> m_loadingProgress;
    std::deque<LoadedModel> m_loadedModels;
    std::map<unsigned long long, RR::ApiHandle<RR::Entity>> m_selectedEntities;

    int m_frameStatsUpdateDelay = 60;
    RR::FrameStatistics m_frameStats;
};