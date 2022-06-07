#include <Rendering/UI/SceneState.h>

#include <QMatrix4x4>
#include <QtMath>
#include <Rendering/ArrSession.h>
#include <Rendering/ArrSettings.h>
#include <d3d11.h>
#include <dxgi.h>
#include <utility>

using namespace std::chrono_literals;

namespace
{
    RR::Double3 ToDouble3(const QVector4D& v)
    {
        return {v.x(), v.y(), v.z()};
    }

    RR::Double3 ToDouble3(const QVector3D& v)
    {
        return {v.x(), v.y(), v.z()};
    }

    QVector4D qVectorNormalizeW(const QVector4D& v)
    {
        return {v.x() / v.w(), v.y() / v.w(), v.z() / v.w(), 1.0};
    }

    void ConvertMatrix(RR::Matrix4x4& dest, const QMatrix4x4& src)
    {
        memcpy((float*)&dest, src.constData(), sizeof(float) * 16);
    }

} // namespace

std::string HResultToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<unsigned int>(hr));
    return std::string(s_str);
}

void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}

SceneState::SceneState(ArrSettings* arrOptions)
    : QObject(nullptr)
{
    m_arrOptions = arrOptions;
    m_refreshTimer = new QTimer(this);

    QObject::connect(m_refreshTimer, &QTimer::timeout, this, [this]()
                     { SceneRefresh(); });

    QObject::connect(m_arrOptions, &ArrSettings::OptionsChanged, this, [this]()
                     { 
                        UpdateProjectionMatrix();
                        UpdatePointSize(); });

    InitializeD3D();
}

SceneState::~SceneState()
{
    DeinitializeD3D();
}

void SceneState::SetSession(RR::ApiHandle<RR::RenderingSession> session, ArrSession* arrSession)
{
    if (m_session == session)
        return;

    if (m_session)
    {
        DeinitializeClient();
    }

    const int screenWidth = m_arrOptions->GetVideoWidth();
    const int screenHeight = m_arrOptions->GetVideoHeight();

    if (m_proxyTextureWidth != screenWidth || m_proxyTextureHeight != screenHeight)
    {
        m_proxyTextureWidth = screenWidth;
        m_proxyTextureHeight = screenHeight;

        UpdateProxyTextures();

        Q_EMIT VideoResolutionChanged();
    }

    m_session = session;
    m_arrSession = arrSession;

    if (m_session)
    {
        InitializeClient();
    }
}

void SceneState::InitializeClient()
{
    m_client = m_session->Connection();
    m_graphicsBinding = m_session->GetGraphicsBinding().as<RR::GraphicsBindingSimD3d11>();

    if (m_graphicsBinding)
    {
        m_refreshRate = m_arrOptions->GetVideoRefreshRate();
        auto result = m_graphicsBinding->InitSimulation(m_device, m_proxyDepthTarget, m_proxyColorTarget, m_refreshRate, false, false, false);
        if (result != RR::Result::Success)
        {
            return;
        }

        ZeroMemory(&m_simUpdate, sizeof(m_simUpdate));

        UpdateProjectionMatrix();
        UpdatePointSize();
        m_refreshTimer->start(1000ms / m_refreshRate);
    }
}

void SceneState::DeinitializeClient()
{
    if (m_graphicsBinding)
    {
        m_graphicsBinding->DeinitSimulation();
    }

    m_refreshTimer->stop();
    m_graphicsBinding = nullptr;
}

void SceneState::InitializeD3D()
{
    // instead of using the control's width/height, we operate on the maximum size (i.e. 1980x1080) because we cannot
    // scale the video resolution dynamically, once initialized

    UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    iCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

retry:
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0};

    HRESULT res = D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        iCreateFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_deviceContext);

    if (res != S_OK)
    {
        if (iCreateFlags & D3D11_CREATE_DEVICE_DEBUG)
        {
            // it is common that D3D devices can't be created with the debug device flag
            // so if this fails, remove the debug device flag and try again
            iCreateFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
            goto retry;
        }

        throw 0;
    }
}

void SceneState::UpdateProxyTextures()
{
    ReleaseObject(m_proxyColorTarget);
    ReleaseObject(m_proxyDepthTarget);
    ReleaseObject(m_proxyColorView);
    ReleaseObject(m_proxyDepthView);

    // create proxy textures
    D3D11_TEXTURE2D_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));

    bufferDesc.ArraySize = 2;
    bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.Width = m_proxyTextureWidth;
    bufferDesc.Height = m_proxyTextureHeight;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.MipLevels = 1;
    bufferDesc.MiscFlags = 0;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    // color texture
    ThrowIfFailed(m_device->CreateTexture2D(&bufferDesc, 0, &m_proxyColorTarget));
    ThrowIfFailed(m_device->CreateRenderTargetView(m_proxyColorTarget, NULL, &m_proxyColorView));

    // depth stencil
    bufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ThrowIfFailed(m_device->CreateTexture2D(&bufferDesc, 0, &m_proxyDepthTarget));
    ThrowIfFailed(m_device->CreateDepthStencilView(m_proxyDepthTarget, NULL, &m_proxyDepthView));
}

void SceneState::DeinitializeD3D()
{
    ReleaseObject(m_proxyColorTarget);
    ReleaseObject(m_proxyDepthTarget);
    ReleaseObject(m_proxyColorView);
    ReleaseObject(m_proxyDepthView);

    ReleaseObject(m_deviceContext);
    ReleaseObject(m_device);
}

void SceneState::ResizeViewport(int width, int height)
{
    if (width == 0 || height == 0 || (width == m_screenWidth && height == m_screenHeight))
    {
        return;
    }

    m_screenWidth = width;
    m_screenHeight = height;

    UpdateProjectionMatrix();
    UpdatePointSize();
}

void SceneState::PickEntity(int x, int y)
{
    if (m_screenWidth <= 0 || m_screenHeight <= 0)
    {
        return;
    }

    RR::RayCast rc;

    // normalize (x,y) to [-1,1] range
    const float normX = 2.0f * x / float(m_screenWidth) - 1.0f;
    const float normY = -(2.0f * y / float(m_screenHeight) - 1.0f);

    const QMatrix4x4 inverse = m_viewMatrixInverse * m_perspectiveMatrixInverse;

    // find the 3d position of the pixel on the near plane, and the the position of the same pixel on the far plane
    const QVector4D p1 = qVectorNormalizeW(inverse * QVector4D(normX, normY, -1, 1));
    const QVector4D p2 = qVectorNormalizeW(inverse * QVector4D(normX, normY, 1, 1));

    const QVector4D dir = (p2 - p1).normalized();

    rc.StartPos = ToDouble3(m_cameraPosition);
    rc.EndPos = ToDouble3(p2);

    rc.HitCollection = RR::HitCollectionPolicy::ClosestHit;
    rc.MaxHits = 1;
    rc.CollisionMask = 0xffffffff;

    auto onRaycastResult = [this](RR::Status status, RR::ApiHandle<RR::RayCastQueryResult> result)
    {
        RR::ApiHandle<RR::Entity> hit;

        if (status == RR::Status::OK)
        {
            std::vector<RR::RayCastHit> rayCastHits;
            result->GetHits(rayCastHits);
            if (rayCastHits.size() > 0)
            {
                hit = rayCastHits[0].HitObject;
            }
        }

        m_pickedEntity = hit;
        Q_EMIT PickedEntity();
    };

    m_client->RayCastQueryAsync(rc, onRaycastResult);
}

void SceneState::RenderTo(ID3D11RenderTargetView* renderTarget)
{
    if (m_client)
    {
        auto opt = GetArrSettings();
        m_client->GetCameraSettings()->SetNearAndFarPlane(opt->GetNearPlaneCM() / 100.0f, opt->GetFarPlaneCM() / 100.0f);
    }

    m_deviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);

    float backColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_deviceContext->ClearRenderTargetView(renderTarget, backColor);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = m_proxyTextureWidth;
    viewport.Height = m_proxyTextureHeight;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    m_deviceContext->RSSetViewports(1, &viewport);

    if (m_graphicsBinding)
    {
        m_graphicsBinding->ReprojectProxy();
    }
}

void SceneState::MoveCamera(float lateral, float forward, float updown)
{
    QVector3D xAxis, yAxis, zAxis;
    m_cameraRotation.getAxes(&xAxis, &yAxis, &zAxis);
    m_cameraMoveTargetDirection = QVector3D(0, 0, 0);
    m_cameraMoveTargetDirection += xAxis * lateral;
    m_cameraMoveTargetDirection += zAxis * forward;
    m_cameraMoveTargetDirection += yAxis * updown;
}

void SceneState::LerpCamera(float lateral, float forward, float updown)
{
    QVector3D xAxis, yAxis, zAxis;
    m_cameraRotation.getAxes(&xAxis, &yAxis, &zAxis);
    m_lerpCameraPosition += xAxis * lateral;
    m_lerpCameraPosition += zAxis * forward;
    m_lerpCameraPosition += yAxis * updown;
}

void SceneState::RotateCamera(float dx, float dy)
{
    m_cameraYaw += dx;
    m_cameraPitch += dy;

    // clamp up/down rotation
    if (m_cameraPitch > 80)
        m_cameraPitch = 80;
    if (m_cameraPitch < -80)
        m_cameraPitch = -80;

    m_cameraRotation = QQuaternion::fromEulerAngles(m_cameraPitch, m_cameraYaw, 0);
}

void SceneState::UpdateProjectionMatrix()
{
    // even if the ratio is not valid, which might happen when the viewport is collapsed, or not shown yet, we still need
    // to provide a valid projection matrix, to avoid problems in har
    float ratio = 1.0;
    if (m_screenHeight > 0 && m_screenWidth > 0)
    {
        ratio = (float)m_screenWidth / (float)m_screenHeight;
    }

    const float fovAngle = m_arrOptions->GetFovAngle();
    const float nearPlane = m_arrOptions->GetNearPlaneCM() / 100.0f;
    const float farPlane = m_arrOptions->GetFarPlaneCM() / 100.0f;

    // update projection when viewport size changes
    QMatrix4x4 m;
    m.perspective(fovAngle, ratio, nearPlane, farPlane);

    // Compute horizontal and vertical angles from the full vertical fov in the camera settings.
    // Alternatively, the same data can be extracted from the projection matrix 'm' using RR::FovFromProjectionMatrix
    const float halfFovX = qAtan(qTan(qDegreesToRadians(fovAngle) * 0.5f) * ratio);
    m_simUpdate.FieldOfView.Left.AngleLeft = -halfFovX;
    m_simUpdate.FieldOfView.Left.AngleRight = halfFovX;

    const float halfFovY = qDegreesToRadians(fovAngle) / 2;
    m_simUpdate.FieldOfView.Left.AngleUp = halfFovY;
    m_simUpdate.FieldOfView.Left.AngleDown = -halfFovY;

    // the inverse projection used for picking needs -z
    m.scale(1, 1, -1);
    m_perspectiveMatrixInverse = m.inverted();
}

void SceneState::UpdatePointSize()
{
    if (m_client)
    {
        m_client->GetPointCloudSettingsExperimental()->SetPointSizeScale(m_arrOptions->GetPointSizeFloat());
    }
}

void SceneState::SceneRefresh()
{
    if (m_graphicsBinding)
    {
        m_simUpdate.FrameId++;

        {
            // The camera position update gets some inertia to not change rapidly from one frame to the next.
            // This is because the ARR server has to do prediction to render the next frame and assumes a somewhat continuous head movement.
            // If the changes are too rapid (which isn't possible when you wear a HoloLens), the predicted frame might overshoot and
            // look bad. With a smoothed out camera movement, such artifacts don't appear.
            const double timeDelta = 1.0 / m_refreshRate;
            m_cameraPosition += timeDelta * m_cameraMoveDirection;
            m_cameraMoveDirection = (m_cameraMoveDirection * 0.6f + m_cameraMoveTargetDirection * 0.4f);
            m_cameraMoveTargetDirection = QVector3D(0, 0, 0);

            const QVector3D moveCam = m_lerpCameraPosition * 0.1f;
            m_lerpCameraPosition -= moveCam;
            m_cameraPosition += moveCam;
        }

        QMatrix4x4 m;
        m.setToIdentity();
        m.rotate(m_cameraRotation.inverted());
        m.translate(-m_cameraPosition);

        ConvertMatrix(m_simUpdate.ViewTransform.Left, m);
        ConvertMatrix(m_simUpdate.ViewTransform.Right, m);

        m_viewMatrixInverse = m.inverted();

        // ARRT is not doing any rendering of local content, so we can discard the outputUpdate
        // which would normally be used to feed the renderer with the projection / transform data
        // for the current frame to align local and remote content correctly in the proxy render target.
        RR::SimulationUpdateResult outputUpdate;
        m_graphicsBinding->Update(m_simUpdate, &outputUpdate);

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = m_proxyTextureWidth;
        viewport.Height = m_proxyTextureHeight;
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;

        m_deviceContext->RSSetViewports(1, &viewport);

        m_deviceContext->OMSetRenderTargets(1, &m_proxyColorView, m_proxyDepthView);
        m_graphicsBinding->BlitRemoteFrameToProxy();
    }

    Q_EMIT SceneRefreshed();
}

void SceneState::FocusOnSelectedEntity()
{
    if (!m_graphicsBinding)
        return;

    std::vector<RR::ApiHandle<RR::Entity>> selected;
    m_arrSession->GetSelectedEntities(selected);

    if (!selected.empty())
    {
        // could focus on all entities, but then we'd need to retrieve the bbox of all objects, merge that and then move the camera
        FocusOnEntity(selected[0]);
    }
    else
    {
        if (!m_arrSession->GetLoadedModels().empty())
        {
            FocusOnEntity(m_arrSession->GetLoadedModels()[0].m_LoadResult->GetRoot());
        }
    }
}

void SceneState::FocusOnEntity(RR::ApiHandle<RR::Entity> entity)
{
    entity->QueryWorldBoundsAsync([this](RR::Status status, RR::Bounds bounds)
                                  {
                                      if (status == RR::Status::OK && bounds.IsValid())
                                      {
                                          FocusOnBounds(bounds);
                                      } });
}

void SceneState::FocusOnBounds(const RR::Bounds& bounds)
{
    const float maxX = (float)bounds.Max.X;
    const float minX = (float)bounds.Min.X;
    const float maxY = (float)bounds.Max.Y;
    const float minY = (float)bounds.Min.Y;
    const float maxZ = (float)bounds.Max.Z;
    const float minZ = (float)bounds.Min.Z;

    const QVector3D center = QVector3D(minX, minY, minZ) + (QVector3D(maxX, maxY, maxZ) - QVector3D(minX, minY, minZ)) * 0.5f;

    float maxRadius = maxX - minX;
    maxRadius = std::max(maxRadius, maxY - minY);
    maxRadius = std::max(maxRadius, maxZ - minZ);

    QVector3D xAxis, yAxis, zAxis;
    m_cameraRotation.getAxes(&xAxis, &yAxis, &zAxis);

    QVector3D newTargetPos = center - maxRadius * zAxis;

    m_lerpCameraPosition = newTargetPos - m_cameraPosition;
}
