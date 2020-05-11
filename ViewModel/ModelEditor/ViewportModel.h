#pragma once
#include <Model/Configuration.h>
#include <Model/IncludesAzureRemoteRendering.h>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QObject>
#include <QPointer>
#include <QQuaternion>
#include <QTimer>
#include <Utils/Value.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
class ModelEditorModel;
class ViewportView;
class EntitySelection;
class CameraSettings;
class VideoSettings;

namespace Microsoft::Azure::RemoteRendering::Internal
{
    class GraphicsBindingSimD3d11;
    class RemoteRenderingClient;
} // namespace Microsoft::Azure::RemoteRendering::Internal

#define ReleaseObject(object) \
    if ((object) != nullptr)  \
    {                         \
        object->Release();    \
        object = nullptr;     \
    }
#define ReleaseHandle(object) \
    if ((object) != nullptr)  \
    {                         \
        CloseHandle(object);  \
        object = nullptr;     \
    }
#define DXCall(func) throwIfFailed(func)

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

// Model that manages all the DirectX11 level functionality required for remote rendering

class ViewportModel : public QObject
{
    Q_OBJECT

public:
    ViewportModel(VideoSettings* videoSettings, CameraSettings* cameraSettings, QObject* parent);
    virtual ~ViewportModel();

    void setClient(RR::RemoteManager* client);

    void setCameraSpeed(float lateral, float forward, float updown);
    void setCameraRotationSpeed(float x, float y);
    void moveCameraDirection(float dx, float dy);

    // Triggers update on Widget
    void updateD3D();

    void resize(int width, int height);

    void pick(int x, int y);

    void render();

    void setSelectionModel(EntitySelection* selectionModel);

    ID3D11Device* getDxDevice() const { return m_device; }
    ID3D11DeviceContext* getDxDeviceContext() const { return m_deviceContext; }

    int getWidth() const { return m_proxyTextureWidth; }
    int getHeight() const { return m_proxyTextureHeight; }

    bool isEnabled() const { return m_desktopSimCreated; }

Q_SIGNALS:
    void onRefresh();
    void videoResolutionChanged();

private:
    void updateProjection();

    void onRefreshTimer();
    RR::GraphicsBindingSimD3d11* getBinding() const;

    void stepCamera();

    int m_queryCounter = 0;
    QMap<int, std::shared_ptr<Microsoft::Azure::RemoteRendering::RaycastQueryAsync>> m_rayCastQueries;

private:
    int m_proxyTextureWidth = 0;
    int m_proxyTextureHeight = 0;

    int m_width = 0;
    int m_height = 0;

    CameraSettings* m_cameraSettings = nullptr;
    VideoSettings* m_videoSettings = nullptr;

    QTimer* m_refreshTimer = nullptr;

    RR::SimulationUpdate m_simUpdate;
    bool m_desktopSimCreated = false;

    ID3D11Device* m_device = nullptr;               // can move
    ID3D11DeviceContext* m_deviceContext = nullptr; // can move

    ID3D11Texture2D* m_proxyColorTarget = nullptr;
    ID3D11Texture2D* m_proxyDepthTarget = nullptr;
    ID3D11RenderTargetView* m_proxyColorView = nullptr;
    ID3D11DepthStencilView* m_proxyDepthView = nullptr;

    RR::RemoteManager* m_client = nullptr;
    std::shared_ptr<RR::GraphicsBindingSimD3d11> m_graphicsBinding = nullptr;
    float m_targetCameraLateralSpeed = 0.0f;
    float m_targetCameraForwardSpeed = 0.0f;
    float m_targetCameraUpdownSpeed = 0.0f;

    float m_cameraLateralSpeed = 0.0f;
    float m_cameraForwardSpeed = 0.0f;
    float m_cameraUpdownSpeed = 0.0f;

    float m_cameraRotationSpeedX = 0.0f;
    float m_cameraRotationSpeedY = 0.0f;

    QQuaternion m_cameraRotation;
    QMatrix4x4 m_perspectiveMatrixInverse;
    QMatrix4x4 m_viewMatrixInverse;
    QVector3D m_cameraPosition;
    float m_yaw = 0.0f;   //yaw in radians
    float m_pitch = 0.0f; //pitch in radians

    QElapsedTimer m_timeFromLastUpdate;

    EntitySelection* m_selectionModel = nullptr;


    void update();
    void initializeClient();
    void deinitializeClient();

    void updateSelection(const QList<std::shared_ptr<RR::Entity>>& selected, const QList<std::shared_ptr<RR::Entity>>& deselected);

    void initializeD3D();
    void deinitializeD3D();

    void updateProxyTextures();
};
