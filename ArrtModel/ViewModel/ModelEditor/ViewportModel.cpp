#include <ViewModel/ModelEditor/ViewportModel.h>

#include <QMatrix4x4>
#include <QtMath>
#include <d3d11.h>
#include <dxgi.h>
#include <utility>

#include <Model/ModelEditor/EntitySelection.h>
#include <Model/Settings/CameraSettings.h>
#include <Model/Settings/VideoSettings.h>

using namespace std::chrono_literals;


// based on https://github.com/giladreich/QtDirect3D/blob/master/src/QDirect3D11Widget

namespace
{
    RR::Double3 qVectorToWorldPosition(const QVector4D& v)
    {
        return {v.x(), v.y(), v.z()};
    }

    QVector4D qVectorNormalizeW(const QVector4D& v)
    {
        return {v.x() / v.w(), v.y() / v.w(), v.z() / v.w(), 1.0};
    }
} // namespace

namespace
{
    void convertMatrix(RR::Matrix4x4& dest, const QMatrix4x4& src)
    {
        src.transposed().copyDataTo((float*)&dest);
    }


    float cameraCurve(float currentSpeed, float targetSpeed, float inertia)
    {
        const float acceleration = targetSpeed - currentSpeed;
        if (abs(acceleration) < 0.01f || inertia <= 0.f)
        {
            return targetSpeed;
        }

        return currentSpeed + acceleration * (1.01 - inertia);
    }
} // namespace


ViewportModel::ViewportModel(VideoSettings* videoSettings, CameraSettings* cameraSettings, QObject* parent)
    : QObject(parent)
    , m_cameraSettings(cameraSettings)
    , m_videoSettings(videoSettings)
{
    // static one-time initialization
    moveCameraDirection(0.0, 0.0);

    m_refreshTimer = new QTimer(this);
    QObject::connect(m_refreshTimer, &QTimer::timeout, this, [this]() {
        onRefreshTimer();
    });

    QObject::connect(m_cameraSettings, &CameraSettings::changed, this, [this]() {
        updateProjection();
    });

    initializeD3D();
}

ViewportModel::~ViewportModel()
{
    deinitializeD3D();
}

void ViewportModel::setSession(RR::ApiHandle<RR::AzureSession> session)
{
    if (m_session != session)
    {
        if (m_session)
        {
            deinitializeClient();
        }
        if (m_proxyTextureWidth != m_videoSettings->getWidth() || m_proxyTextureHeight != m_videoSettings->getHeight())
        {
            m_proxyTextureWidth = m_videoSettings->getWidth();
            m_proxyTextureHeight = m_videoSettings->getHeight();
            updateProxyTextures();
            Q_EMIT videoResolutionChanged();
        }
        m_session = std::move(session);
        if (m_session)
        {
            initializeClient();
        }
    }
}

void ViewportModel::initializeClient()
{
    m_client = m_session->Actions();
    m_graphicsBinding = m_session->GetGraphicsBinding().as<RR::GraphicsBindingSimD3d11>();
    if (auto&& binding = getBinding())
    {
        const auto refreshRate = m_videoSettings->getRefreshRate();
        auto result = binding->InitSimulation(m_device, m_proxyDepthTarget, m_proxyColorTarget, refreshRate, false, false);
        if (!result || result.value() != RR::Result::Success)
        {
            return;
        }

        ZeroMemory(&m_simUpdate, sizeof(m_simUpdate));
        m_simUpdate.nearPlaneDistance = 0.1f;
        m_simUpdate.farPlaneDistance = 100.f;
        m_simUpdate.renderTargetWidth = m_proxyTextureWidth;
        m_simUpdate.renderTargetHeight = m_proxyTextureHeight;

        updateProjection();
        m_refreshTimer->start(1000ms / refreshRate);
    }
}

void ViewportModel::deinitializeClient()
{
    if (auto&& binding = getBinding())
    {
        binding->DeinitSimulation();
    }
    m_refreshTimer->stop();
    m_graphicsBinding = nullptr;
}

void ViewportModel::initializeD3D()
{
    // instead of using the control's width/height, we operate on the maximum size (i.e. 1980x1080) because we cannot
    // scale the video resolution dynamically, once initialized

    UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    iCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
        throw 0;
    }

    m_desktopSimCreated = true;
}

void ViewportModel::updateProxyTextures()
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
    throwIfFailed(m_device->CreateTexture2D(&bufferDesc, 0, &m_proxyColorTarget));
    throwIfFailed(m_device->CreateRenderTargetView(m_proxyColorTarget, NULL, &m_proxyColorView));

    // depth stencil
    bufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    throwIfFailed(m_device->CreateTexture2D(&bufferDesc, 0, &m_proxyDepthTarget));
    throwIfFailed(m_device->CreateDepthStencilView(m_proxyDepthTarget, NULL, &m_proxyDepthView));
}

void ViewportModel::deinitializeD3D()
{
    ReleaseObject(m_proxyColorTarget);
    ReleaseObject(m_proxyDepthTarget);
    ReleaseObject(m_proxyColorView);
    ReleaseObject(m_proxyDepthView);

    ReleaseObject(m_deviceContext);
    ReleaseObject(m_device);
}

void ViewportModel::updateD3D()
{
    if (m_client)
    {
        deinitializeClient();
    }

    deinitializeD3D();
    initializeD3D();

    if (m_client)
    {
        initializeClient();
    }
}

void ViewportModel::resize(int width, int height)
{
    if (width == 0 || height == 0 || (width == m_width && height == m_height))
    {
        return;
    }

    m_width = width;
    m_height = height;


    updateProjection();
}

void ViewportModel::pick(int x, int y)
{
    pick(x, y, false);
}

void ViewportModel::doubleClick(int x, int y)
{
    pick(x, y, true);
}

void ViewportModel::pick(int x, int y, bool doubleClick)
{
    if (m_width <= 0 || m_height <= 0)
    {
        return;
    }

    RR::RayCast rc;

    QVector4D v(x, y, 0, 1);
    v.setX(2.0f * v.x() / float(m_width) - 1.0f);
    v.setY(2.0f * v.y() / float(m_height) - 1.0f);
    v.setX(-v.x());

    const QVector4D p1 = qVectorNormalizeW(m_viewMatrixInverse * m_perspectiveMatrixInverse * v);
    v.setZ(v.z() + 0.1f);
    const QVector4D p2 = qVectorNormalizeW(m_viewMatrixInverse * m_perspectiveMatrixInverse * v);

    const QVector4D dir = (p1 - p2).normalized();

    rc.StartPos = qVectorToWorldPosition(p1 + dir * m_simUpdate.nearPlaneDistance);
    rc.EndPos = qVectorToWorldPosition(p1 + dir * m_simUpdate.farPlaneDistance);

    rc.HitCollection = RR::HitCollectionPolicy::ClosestHit;
    rc.MaxHits = 1;
    rc.CollisionMask = 0xffffffff;
    QPointer<ViewportModel> thisPtr = this;
    if (auto async = m_client->RayCastQueryAsync(rc))
    {
        (*async)->Completed([thisPtr, doubleClick](const RR::ApiHandle<RR::RaycastQueryAsync>& finishedAsync) {
            RR::ApiHandle<RR::Entity> hit = nullptr;
            std::vector<RR::RayCastHit> rayCastHits;
            if (finishedAsync->Result(rayCastHits))
            {
                if (rayCastHits.size() > 0)
                {
                    hit = rayCastHits[0].HitObject;
                }
            }

            if (thisPtr && thisPtr->m_selectionModel)
            {
                if (hit == nullptr)
                {
                    thisPtr->m_selectionModel->deselectAll();
                }
                else
                {
                    thisPtr->m_selectionModel->select(hit);
                    if (doubleClick)
                    {
                        thisPtr->m_selectionModel->focusEntity(hit);
                    }
                }
            }
        });
    }
}

void ViewportModel::setCameraSpeed(float lateral, float forward, float updown)
{
    m_targetCameraLateralSpeed = lateral;
    m_targetCameraForwardSpeed = -forward;
    m_targetCameraUpdownSpeed = updown;
}

void ViewportModel::setCameraRotationSpeed(float x, float y)
{
    m_cameraRotationSpeedX = x;
    m_cameraRotationSpeedY = y;
}

void ViewportModel::moveCameraDirection(float dx, float dy)
{
    m_yaw += dx * m_cameraSettings->getCameraRotationSpeed();
    m_pitch += dy * m_cameraSettings->getCameraRotationSpeed();
    m_cameraRotation = QQuaternion::fromEulerAngles(m_pitch, m_yaw, 0);
}

void ViewportModel::updateProjection()
{
    // even if the ratio is not valid, which might happen when the viewport is collapsed, or not shown yet, we still need
    // to provide a valid projection matrix, to avoid problems in har
    float ratio = 1.0;
    if (m_height > 0 && m_width > 0)
    {
        ratio = (float)m_width / (float)m_height;
    }
    // update projection when viewport size changes
    QMatrix4x4 m;
    m.perspective(m_cameraSettings->getFovAngle(), ratio, m_simUpdate.nearPlaneDistance, m_simUpdate.farPlaneDistance);

    convertMatrix(m_simUpdate.projection, m);
    bool success = false;
    m_perspectiveMatrixInverse = m.inverted(&success);
    if (!success)
    {
        qWarning() << tr("Projection matrix is not invertible");
    }
}

void ViewportModel::stepCamera()
{
    float timeDelta = 0;
    if (!m_timeFromLastUpdate.isValid())
    {
        m_timeFromLastUpdate.start();
    }
    else
    {
        timeDelta = float(double(m_timeFromLastUpdate.restart()) / 1000.0);
    }

    // only step if the time delta is 0 < d < 1s
    if (timeDelta > 0 && timeDelta < 1.0f)
    {
        m_cameraLateralSpeed = cameraCurve(m_cameraLateralSpeed, m_targetCameraLateralSpeed * m_cameraSettings->getCameraSpeed(), m_cameraSettings->getCameraInertia());
        m_cameraForwardSpeed = cameraCurve(m_cameraForwardSpeed, m_targetCameraForwardSpeed * m_cameraSettings->getCameraSpeed(), m_cameraSettings->getCameraInertia());
        m_cameraUpdownSpeed = cameraCurve(m_cameraUpdownSpeed, m_targetCameraUpdownSpeed * m_cameraSettings->getCameraSpeed(), m_cameraSettings->getCameraInertia());

        moveCameraDirection(m_cameraRotationSpeedX * m_cameraSettings->getCameraRotationSpeed() * timeDelta,
                            m_cameraRotationSpeedY * m_cameraSettings->getCameraRotationSpeed() * timeDelta);

        QVector3D xAxis, yAxis, zAxis;
        m_cameraRotation.getAxes(&xAxis, &yAxis, &zAxis);
        m_cameraPosition += xAxis * m_cameraLateralSpeed * timeDelta;
        m_cameraPosition -= zAxis * m_cameraForwardSpeed * timeDelta;
        m_cameraPosition += yAxis * m_cameraUpdownSpeed * timeDelta;
    }
}


void ViewportModel::onRefreshTimer()
{
    if (m_desktopSimCreated)
    {
        stepCamera();
        update();

        if (auto&& binding = getBinding())
        {
            D3D11_VIEWPORT viewport;
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = m_proxyTextureWidth;
            viewport.Height = m_proxyTextureHeight;
            viewport.MinDepth = 0.f;
            viewport.MaxDepth = 1.f;

            m_deviceContext->RSSetViewports(1, &viewport);

            m_deviceContext->OMSetRenderTargets(1, &m_proxyColorView, m_proxyDepthView);
            binding->BlitRemoteFrameToProxy();
        }

        Q_EMIT onRefresh();
    }
}

void ViewportModel::render()
{
    if (auto&& binding = getBinding())
    {
        binding->ReprojectProxy();
    }
}

void ViewportModel::setSelectionModel(EntitySelection* selectionModel)
{
    if (m_selectionModel)
    {
        qCritical() << tr("Can't assign a selection model twice");
        return;
    }
    m_selectionModel = selectionModel;
    if (m_selectionModel)
    {
        QList<RR::ApiHandle<RR::Entity>> selected;
        for (auto&& entity : *m_selectionModel)
        {
            selected.push_back(entity);
        }
        updateSelection(selected, {});

        connect(m_selectionModel, &EntitySelection::selectionChanged, this,
                [this](const QList<RR::ApiHandle<RR::Entity>>& selected, const QList<RR::ApiHandle<RR::Entity>>& deselected) {
                    updateSelection(selected, deselected);
                });
        connect(m_selectionModel, &EntitySelection::entityFocused, this,
                [this](RR::ApiHandle<RR::Entity> entity) {
                    zoomOnEntity(entity);
                });
    }
}

void ViewportModel::update()
{
    if (auto&& binding = getBinding())
    {
        RR::SimulationUpdate outputUpdate;
        m_simUpdate.frameId++;

        QMatrix4x4 m;
        m.rotate(m_cameraRotation.inverted());
        m.translate(-m_cameraPosition);

        convertMatrix(m_simUpdate.viewTransform, m);
        bool success = false;
        m_viewMatrixInverse = m.inverted(&success);
        if (!success)
        {
            qWarning() << tr("View matrix not invertible");
        }

        binding->Update(m_simUpdate, &outputUpdate);
    }
}

RR::ApiHandle<RR::GraphicsBindingSimD3d11>& ViewportModel::getBinding()
{
    return m_graphicsBinding;
}

void ViewportModel::updateSelection(const QList<RR::ApiHandle<RR::Entity>>& selected, const QList<RR::ApiHandle<RR::Entity>>& deselected)
{
    if (!m_client)
    {
        return;
    }

    for (const RR::ApiHandle<RR::Entity>& entity : deselected)
    {
        const auto validEx = entity->Valid();
        if (validEx && validEx.value())
        {
            std::vector<RR::ApiHandle<RR::ComponentBase>> components;
            if (entity->Components(components))
            {
                for (auto&& comp : components)
                {
                    if (comp->Type().value() == RR::ObjectType::HierarchicalStateOverrideComponent)
                    {
                        comp->Destroy();
                        break;
                    }
                }
            }
        }
    }

    for (const RR::ApiHandle<RR::Entity>& entity : selected)
    {
        if (auto component = m_client->CreateComponent(
                RR::ObjectType::HierarchicalStateOverrideComponent, entity))
        {
            if (auto hierarchicalComp = component->as<RR::HierarchicalStateOverrideComponent>())
            {
                hierarchicalComp->SetState(RR::HierarchicalStates::Selected, RR::HierarchicalEnableState::ForceOn);
            }
        }
    }
}

void ViewportModel::zoomOnEntity(RR::ApiHandle<RR::Entity> entity)
{
    //Expected<Microsoft::Azure::RemoteRendering::ApiHandle<Microsoft::Azure::RemoteRendering::BoundsQueryAsync>, Microsoft::Azure::RemoteRendering::Status>
    QPointer<ViewportModel> thisPtr = this;
    if (auto async = entity->QueryWorldBoundsAsync())
    {
        (*async)->Completed([thisPtr](const RR::ApiHandle<RR::BoundsQueryAsync>& finishedAsync) {
            auto result = finishedAsync->Result();
            if (result && result->IsValid())
            {
                auto minBB = result->min;
                auto maxBB = result->max;
                thisPtr->zoomOnBoundingBox(QVector3D(minBB.x, minBB.y, minBB.z), QVector3D(maxBB.x, maxBB.y, maxBB.z));
            }
        });
    }
}

void ViewportModel::zoomOnBoundingBox(const QVector3D& minBB, const QVector3D& maxBB)
{

    QVector3D axisX, axisY, axisZ;
    m_cameraRotation.getAxes(&axisX, &axisY, &axisZ);

    // calculates a sphere including the bounding box

    QVector3D center = (minBB + maxBB) / 2;
    QVector3D diagonal = maxBB - minBB;
    qreal bbRadius = qMax(qMax(diagonal.x(), diagonal.y()), diagonal.z()) * 1.5 / 2.0;

    // the distance is so that the bounding sphere is all inside the fov angle
    qreal dist = bbRadius / qTan(M_PI * m_cameraSettings->getFovAngle() / 360.0);

    // keeps a minimum distance to avoid clipping
    if (dist < m_simUpdate.nearPlaneDistance * 2)
    {
        dist = m_simUpdate.nearPlaneDistance * 2;
    }

    // place the camera
    m_cameraPosition = center - axisZ * dist;
    update();
}