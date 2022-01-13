#pragma once

#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QObject>
#include <QPointer>
#include <QQuaternion>
#include <QTimer>
#include <Rendering/IncludeAzureRemoteRendering.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
class ArrSettings;
class ArrSession;

namespace Microsoft::Azure::RemoteRendering::Internal
{
    class GraphicsBindingSimD3d11;
} // namespace Microsoft::Azure::RemoteRendering::Internal

#define ReleaseObject(object) \
    if ((object) != nullptr)  \
    {                         \
        object->Release();    \
        object = nullptr;     \
    }

#include <Winerror.h>

std::string HResultToString(HRESULT hr);

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr)
        : std::runtime_error(HResultToString(hr))
        , m_hr(hr)
    {
    }
    HRESULT Error() const { return m_hr; }

private:
    const HRESULT m_hr;
};

void ThrowIfFailed(HRESULT hr);


/// Takes care of the scene state (mostly camera movement) and interactions with the D3D device.
class SceneState : public QObject
{
    Q_OBJECT

public:
    SceneState(ArrSettings* arrOptions);
    ~SceneState();

    ArrSettings* GetArrSettings() const { return m_arrOptions; }

    void SetSession(RR::ApiHandle<RR::RenderingSession> session, ArrSession* arrSession);

    /// Moves the camera by the exact amount this frame.
    void MoveCamera(float lateral, float forward, float updown);

    /// Adds to the target lerp position, moving the camera smoothly over the next frames.
    void LerpCamera(float lateral, float forward, float updown);

    /// Rotates the camera by the given amount.
    void RotateCamera(float dx, float dy);

    /// Moves the camera to focus on the selected entity.
    void FocusOnSelectedEntity();

    /// Moves the camera to focus on the specified entity.
    void FocusOnEntity(RR::ApiHandle<RR::Entity> entity);

    /// Moves the camera to focus on the given bounding box.
    void FocusOnBounds(const RR::Bounds& bounds);

    void ResizeViewport(int width, int height);

    /// Checks which entity is visible at the given screen pixel coordinate.
    void PickEntity(int x, int y);

    /// Returns the entity that was picked recently (if any).
    RR::ApiHandle<RR::Entity> GetLastPickedEntity() const { return m_pickedEntity; }

    ID3D11Device* GetDxDevice() const { return m_device; }

    int GetTextureWidth() const { return m_proxyTextureWidth; }
    int GetTextureHeight() const { return m_proxyTextureHeight; }

    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }

    void RenderTo(ID3D11RenderTargetView* renderTarget);

Q_SIGNALS:
    void SceneRefreshed();
    void VideoResolutionChanged();
    void PickedEntity();

private:
    void UpdateProjectionMatrix();
    void SceneRefresh();
    void InitializeClient();
    void DeinitializeClient();
    void InitializeD3D();
    void DeinitializeD3D();
    void UpdateProxyTextures();

    int m_proxyTextureWidth = 0;
    int m_proxyTextureHeight = 0;

    int m_screenWidth = 0;
    int m_screenHeight = 0;

    ArrSettings* m_arrOptions = nullptr;
    ArrSession* m_arrSession = nullptr;
    QTimer* m_refreshTimer = nullptr;

    RR::SimulationUpdateParameters m_simUpdate;

    ID3D11Device* m_device = nullptr;               // can move
    ID3D11DeviceContext* m_deviceContext = nullptr; // can move

    ID3D11Texture2D* m_proxyColorTarget = nullptr;
    ID3D11Texture2D* m_proxyDepthTarget = nullptr;
    ID3D11RenderTargetView* m_proxyColorView = nullptr;
    ID3D11DepthStencilView* m_proxyDepthView = nullptr;

    RR::ApiHandle<RR::RenderingSession> m_session = nullptr;
    RR::ApiHandle<RR::RenderingConnection> m_client = nullptr;
    RR::ApiHandle<RR::GraphicsBindingSimD3d11> m_graphicsBinding = nullptr;

    QQuaternion m_cameraRotation;
    QMatrix4x4 m_perspectiveMatrixInverse;
    QMatrix4x4 m_viewMatrixInverse;
    QVector3D m_cameraPosition = QVector3D(0, 0, 0);
    QVector3D m_cameraMoveDirection = QVector3D(0, 0, 0);
    QVector3D m_cameraMoveTargetDirection = QVector3D(0, 0, 0);
    QVector3D m_lerpCameraPosition = QVector3D(0, 0, 0);

    float m_cameraYaw = 0.0f;
    float m_cameraPitch = 0.0f;

    int m_refreshRate = 60;

    RR::ApiHandle<RR::Entity> m_pickedEntity;
};
