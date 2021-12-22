#include <QResizeEvent>
#include <Rendering/ArrSettings.h>
#include <Rendering/UI/SceneState.h>
#include <Rendering/UI/ViewportWidget.h>
#include <d3d11.h>
#include <dxgi.h>

ViewportWidget::ViewportWidget(QWidget* parent)
    : QWidget(parent)
{
    setAccessibleName(tr("3D Viewport"));
    setAccessibleDescription("Shows the remotely rendered image.");

    // this is needed for the D3D rendering to work properly
    setAttribute(Qt::WA_PaintOnScreen);

    // make sure winId is called after the attributes are set (it will cause the widget to become native)
    m_hWnd = reinterpret_cast<HWND>(winId());

    setFocusPolicy(Qt::StrongFocus);
}

ViewportWidget::~ViewportWidget()
{
    ReleaseObject(m_RTView);
    ReleaseObject(m_swapChain);
    ReleaseObject(m_dxgiFactory)
}

void ViewportWidget::SetSceneState(SceneState* state)
{
    m_sceneState = state;

    ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_dxgiFactory)));

    auto* dxDevice = m_sceneState->GetDxDevice();

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width();
    sd.BufferDesc.Height = height();
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(dxDevice, &sd, &m_swapChain));

    // back buffer always maximum size
    ID3D11Texture2D* pBackBuffer;
    ThrowIfFailed(m_swapChain->ResizeBuffers(0, m_sceneState->GetScreenWidth(), m_sceneState->GetScreenHeight(), DXGI_FORMAT_UNKNOWN, 0));
    ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
    ThrowIfFailed(dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_RTView));
    ReleaseObject(pBackBuffer);

    QObject::connect(m_sceneState, &SceneState::SceneRefreshed, this, [this]()
                     {
                         UpdateInput();
                         repaint();
                     });

    QObject::connect(m_sceneState, &SceneState::VideoResolutionChanged, this, [this]()
                     {
                         ReleaseObject(m_RTView);

                         auto* dxDevice = m_sceneState->GetDxDevice();
                         ID3D11Texture2D* pBackBuffer;
                         // back buffer always maximum size
                         ThrowIfFailed(m_swapChain->ResizeBuffers(0, m_sceneState->GetScreenWidth(), m_sceneState->GetScreenHeight(), DXGI_FORMAT_UNKNOWN, 0));
                         ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
                         ThrowIfFailed(dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_RTView));
                         ReleaseObject(pBackBuffer);
                     });
}

QPaintEngine* ViewportWidget::paintEngine() const
{
    return nullptr; // owner drawn, so no Qt paint engine for this widget
}

void ViewportWidget::resizeEvent(QResizeEvent* event)
{
    if (m_sceneState)
    {
        m_sceneState->ResizeViewport(event->size().width(), event->size().height());
    }

    QWidget::resizeEvent(event);
}

void ViewportWidget::paintEvent(QPaintEvent*)
{
    if (m_sceneState)
    {
        m_sceneState->RenderTo(m_RTView);
    }

    if (m_swapChain)
    {
        m_swapChain->Present(1, 0);
    }
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_sceneState == nullptr)
        return;

    if (event->button() == Qt::RightButton)
    {
        m_dragStartPoint = event->pos();
    }
    else if (event->button() == Qt::LeftButton)
    {
        m_sceneState->PickEntity(event->localPos().x(), event->localPos().y());
    }
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_sceneState == nullptr)
        return;

    if (event->buttons() & Qt::RightButton)
    {
        auto diff = event->pos() - m_dragStartPoint;
        m_sceneState->RotateCamera(diff.rx(), diff.ry());
        m_dragStartPoint = event->pos();
    }
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    const float cameraSpeed = m_sceneState->GetArrSettings()->GetCameraSpeedMetersPerSecond() * 0.3f;
    const float boost = (m_pressedKeys.contains(Qt::Key_Shift) ? 10.0f : 1.0f);
    const float factor = boost * cameraSpeed;

    if (event->angleDelta().y() > 0)
    {
        m_sceneState->LerpCamera(0, factor, 0);
    }
    else if (event->angleDelta().y() < 0)
    {
        m_sceneState->LerpCamera(0, -factor, 0);
    }
}

void ViewportWidget::UpdateInput()
{
    if (m_sceneState == nullptr)
        return;

    float camMoveForward = 0.0f;
    float camMoveSideways = 0.0f;
    float camMoveUp = 0.0f;
    bool focusOnSelected = false;

    if (m_pressedKeys.contains(Qt::Key_Up) || m_pressedKeys.contains(Qt::Key_W))
    {
        camMoveForward += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Down) || m_pressedKeys.contains(Qt::Key_S))
    {
        camMoveForward -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Left) || m_pressedKeys.contains(Qt::Key_A))
    {
        camMoveSideways -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Right) || m_pressedKeys.contains(Qt::Key_D))
    {
        camMoveSideways += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Q) || m_pressedKeys.contains(Qt::Key_PageUp))
    {
        camMoveUp += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_E) || m_pressedKeys.contains(Qt::Key_PageDown))
    {
        camMoveUp -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_F))
    {
        focusOnSelected = true;
    }

    const float cameraSpeed = m_sceneState->GetArrSettings()->GetCameraSpeedMetersPerSecond();
    const float boost = (m_pressedKeys.contains(Qt::Key_Shift) ? 10.0f : 1.0f);
    const float factor = boost * cameraSpeed;

    m_sceneState->MoveCamera(camMoveSideways * factor, camMoveForward * factor, camMoveUp * factor);

    if (focusOnSelected)
    {
        m_sceneState->FocusOnSelectedEntity();
    }
}

void ViewportWidget::keyPressEvent(QKeyEvent* event)
{
    m_pressedKeys.insert(event->key());
}

void ViewportWidget::keyReleaseEvent(QKeyEvent* event)
{
    m_pressedKeys.remove(event->key());
}

void ViewportWidget::focusOutEvent(QFocusEvent* event)
{
    m_pressedKeys.clear();
    QWidget::focusOutEvent(event);
}
