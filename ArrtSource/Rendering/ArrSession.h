#pragma once

#include <QObject>

#include <Rendering/ArrConnectionLogic.h>
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

// struct RemoteRenderingState
//{
//     enum class State
//     {
//         Inactive,
//         OpeningSession,
//         SessionReady,
//         SessionStopped,
//         SessionExpired,
//         SessionError,
//
//         RuntimeConnecting,
//         RuntimeConnected,
//
//         Disconnecting,
//     };
//
//     bool IsBusy() const;
//     bool IsStoppable() const;
//     bool IsFullyConnected() const;
//     void Update(const RR::RenderingSessionProperties& properties, RR::ConnectionStatus connection);
//
//     State m_CurrentState = State::Inactive;
//     int m_elapsedTimeInMinutes = 0;
//     int m_leaseTimeInMinutes = 0;
// };

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

private Q_SLOTS:
    void OnConnectionStateChanged();
    void OnInitGrahpcs();
    void OnDeinitGrahpcs();

public:
    ArrSession(ArrAccount* arrClient, SceneState* sceneState);
    ~ArrSession();

    const ArrConnectionLogic& GetConnectionState() const { return m_ConnectionLogic; }

    /// Creates a new session with the provided options.
    void CreateSession(const RR::RenderingSessionCreationOptions& info);

    /// Attempts to open an already existing session with the given session ID.
    void OpenSession(const QString& sessionID);

    /// Disconnects from the current session and optionally stops the session as well.
    void CloseSession(bool keepRunning);

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
    void ExtendSessionIfNeeded();
    void OnSceneRefresh();
    void UpdatePerformanceStatistics();
    void CheckEntityBounds(RR::ApiHandle<RR::Entity> entity);
    void CheckEntityBoundsResult(RR::Bounds bounds);
    RR::ApiHandle<RR::RenderingConnection> GetRenderingConnection();

    ArrConnectionLogic m_ConnectionLogic;

    ArrAccount* m_arrAccount = nullptr;

    int m_extensionMinutes = 0;

    SceneState* m_sceneState = nullptr;

    std::atomic_bool m_connectToArrInspectorInProgress = false;
    std::atomic_bool m_renewAsyncInProgress = false;

    float m_modelScale = 1.0f;
    std::vector<float> m_loadingProgress;
    std::deque<LoadedModel> m_loadedModels;
    std::map<unsigned long long, RR::ApiHandle<RR::Entity>> m_selectedEntities;

    int m_frameStatsUpdateDelay = 60;
    RR::FrameStatistics m_frameStats;

    ArrConnectionLogic::State m_previousState = ArrConnectionLogic::State::Inactive;
};