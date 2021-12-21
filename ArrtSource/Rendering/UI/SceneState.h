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

inline std::string hrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<unsigned int>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr)
        : std::runtime_error(hrToString(hr))
        , m_hr(hr)
    {
    }
    HRESULT Error() const { return m_hr; }

private:
    const HRESULT m_hr;
};

inline void throwIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

class SceneState : public QObject
{
    Q_OBJECT

public:
    SceneState(ArrSettings* arrOptions);
    virtual ~SceneState();

    ArrSettings* GetArrOptions() const { return m_arrOptions; }

    void SetSession(RR::ApiHandle<RR::RenderingSession> session, ArrSession* arrSession);

    void MoveCamera(float lateral, float forward, float updown);
    void LerpCamera(float lateral, float forward, float updown);
    void RotateCamera(float dx, float dy);
    void FocusOnSelectedEntity();
    void FocusOnEntity(RR::ApiHandle<RR::Entity> entity);
    void FocusOnBounds(const RR::Bounds& bounds);

    void ResizeViewport(int width, int height);

    void PickEntity(int x, int y);
    RR::ApiHandle<RR::Entity> GetLastPickedEntity() const { return m_pickedEntity; }

    ID3D11Device* GetDxDevice() const { return m_device; }

    int getWidth() const { return m_proxyTextureWidth; }
    int getHeight() const { return m_proxyTextureHeight; }

    void RenderTo(ID3D11RenderTargetView* renderTarget);

Q_SIGNALS:
    void SceneRefreshed();
    void VideoResolutionChanged();
    void PickedEntity();

private:
    void UpdateProjectionMatrix();

    void SceneRefresh();

private:
    int m_proxyTextureWidth = 0;
    int m_proxyTextureHeight = 0;

    int m_width = 0;
    int m_height = 0;

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

    void InitializeClient();
    void DeinitializeClient();

    void InitializeD3D();
    void DeinitializeD3D();

    void UpdateProxyTextures();
};
