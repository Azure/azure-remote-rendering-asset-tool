#include <View/ModelEditor/Viewport/ViewportView.h>
#include <d3d11.h>
#include <dxgi.h>

#include <QResizeEvent>
#include <ViewModel/ModelEditor/ModelEditorModel.h>
#include <ViewModel/ModelEditor/Viewport/ViewportModel.h>

ViewportView::ViewportView(ViewportModel* model, QWidget* parent)
    : QWidget(parent)
    , m_viewportModel(model)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // make sure winId is called after the attributes are set (it will cause the widget to become native)
    m_hWnd = reinterpret_cast<HWND>(winId());

    setFocusPolicy(Qt::WheelFocus);

    DXCall(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_dxgiFactory)));

    auto* dxDevice = m_viewportModel->getDxDevice();

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
    DXCall(m_dxgiFactory->CreateSwapChain(
        dxDevice,
        &sd,
        &m_swapChain));

    ID3D11Texture2D* pBackBuffer;
    // back buffer always maximum size
    DXCall(m_swapChain->ResizeBuffers(0, m_viewportModel->getWidth(), m_viewportModel->getHeight(), DXGI_FORMAT_UNKNOWN, 0));
    DXCall(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
    DXCall(dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_RTView));
    ReleaseObject(pBackBuffer);

    QObject::connect(m_viewportModel, &ViewportModel::onRefresh, this, [this]() {
        repaint();
    });

    QObject::connect(m_viewportModel, &ViewportModel::videoResolutionChanged, this, [this]() {
        ReleaseObject(m_RTView);

        auto* dxDevice = m_viewportModel->getDxDevice();
        ID3D11Texture2D* pBackBuffer;
        // back buffer always maximum size
        DXCall(m_swapChain->ResizeBuffers(0, m_viewportModel->getWidth(), m_viewportModel->getHeight(), DXGI_FORMAT_UNKNOWN, 0));
        DXCall(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)));
        DXCall(dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_RTView));
        ReleaseObject(pBackBuffer);
    });
}

ViewportView::~ViewportView()
{
    ReleaseObject(m_RTView);
    ReleaseObject(m_swapChain);
    ReleaseObject(m_dxgiFactory)
}

QPaintEngine* ViewportView::paintEngine() const
{
    return nullptr; // owner drawn, so no Qt paint engine for this widget
}

void ViewportView::resizeEvent(QResizeEvent* event)
{
    m_viewportModel->resize(event->size().width(), event->size().height());

    QWidget::resizeEvent(event);
}

void ViewportView::paintEvent(QPaintEvent* /*event*/)
{
    auto* dxDeviceContext = m_viewportModel->getDxDeviceContext();
    if (m_viewportModel->isEnabled())
    {
        dxDeviceContext->OMSetRenderTargets(1, &m_RTView, NULL);
        float backColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        dxDeviceContext->ClearRenderTargetView(m_RTView, backColor);

        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = m_viewportModel->getWidth();
        viewport.Height = m_viewportModel->getHeight();
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;

        dxDeviceContext->RSSetViewports(1, &viewport);

        m_viewportModel->render();
    }
    else
    {
        dxDeviceContext->OMSetRenderTargets(1, &m_RTView, NULL);
        float backColor[4] = {0.5f, 0.2f, 0.8f, 1.0f};
        dxDeviceContext->ClearRenderTargetView(m_RTView, backColor);
    }

    if (FAILED(m_swapChain->Present(1, 0)))
    {
        qCritical() << tr("Failure while presenting the swap chain");
    }
}

void ViewportView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        m_dragStartPoint = event->pos();
    }
    else if (event->button() == Qt::LeftButton)
    {
        //toggle pick on object
        m_viewportModel->pick(event->localPos().x(), event->localPos().y());
    }
}

void ViewportView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::RightButton)
    {
        auto diff = event->pos() - m_dragStartPoint;
        m_viewportModel->moveCameraDirection(diff.rx(), diff.ry());
        m_dragStartPoint = event->pos();
    }
}

void ViewportView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_viewportModel->doubleClick(event->localPos().x(), event->localPos().y());
    }
}

void ViewportView::updateFromKeyboard()
{
    m_cameraForwardSpeed = 0.0f;
    m_cameraLateralSpeed = 0.0f;
    m_cameraUpdownSpeed = 0.0f;
    m_cameraRotateSpeedX = 0.0f;
    m_cameraRotateSpeedY = 0.0f;

    if (m_pressedKeys.contains(Qt::Key_Up) || m_pressedKeys.contains(Qt::Key_W))
    {
        m_cameraForwardSpeed += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Down) || m_pressedKeys.contains(Qt::Key_S))
    {
        m_cameraForwardSpeed -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Left) || m_pressedKeys.contains(Qt::Key_A))
    {
        m_cameraLateralSpeed -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Right) || m_pressedKeys.contains(Qt::Key_D))
    {
        m_cameraLateralSpeed += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Minus) || m_pressedKeys.contains(Qt::Key_PageUp) || m_pressedKeys.contains(Qt::Key_Q))
    {
        m_cameraUpdownSpeed += 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_Plus) || m_pressedKeys.contains(Qt::Key_PageDown) || m_pressedKeys.contains(Qt::Key_E))
    {
        m_cameraUpdownSpeed -= 1.0f;
    }

    if (m_pressedKeys.contains(Qt::Key_4) || m_pressedKeys.contains(Qt::Key_Insert) || m_pressedKeys.contains(Qt::Key_Z))
    {
        m_cameraRotateSpeedX -= width();
    }

    if (m_pressedKeys.contains(Qt::Key_6) || m_pressedKeys.contains(Qt::Key_Home) || m_pressedKeys.contains(Qt::Key_X))
    {
        m_cameraRotateSpeedX += width();
    }

    if (m_pressedKeys.contains(Qt::Key_8) || m_pressedKeys.contains(Qt::Key_Delete) || m_pressedKeys.contains(Qt::Key_F))
    {
        m_cameraRotateSpeedY -= height();
    }

    if (m_pressedKeys.contains(Qt::Key_2) || m_pressedKeys.contains(Qt::Key_End) || m_pressedKeys.contains(Qt::Key_C))
    {
        m_cameraRotateSpeedY += height();
    }

    const float boost = m_pressedKeys.contains(Qt::Key_Shift) ? 10.f : 1.f;

    m_viewportModel->setCameraSpeed(m_cameraLateralSpeed * boost, m_cameraForwardSpeed * boost, m_cameraUpdownSpeed * boost);
    m_viewportModel->setCameraRotationSpeed(m_cameraRotateSpeedX, m_cameraRotateSpeedY);
}


void ViewportView::keyPressEvent(QKeyEvent* event)
{
    m_pressedKeys.insert(event->key());
    updateFromKeyboard();
}

void ViewportView::keyReleaseEvent(QKeyEvent* event)
{
    m_pressedKeys.remove(event->key());
    updateFromKeyboard();
}

bool ViewportView::event(QEvent* event)
{
    switch (event->type())
    {
            // Workaround for https://bugreports.qt.io/browse/QTBUG-42183 to get key strokes.
            // To make sure that we always have focus on the widget when we enter the rect area.
        case QEvent::Enter:
        case QEvent::FocusIn:
        case QEvent::FocusAboutToChange:
            if (::GetFocus() != m_hWnd)
            {
                QWidget* nativeParent = this;
                while (true)
                {
                    if (nativeParent->isWindow())
                        break;

                    QWidget* parent = nativeParent->nativeParentWidget();
                    if (!parent)
                        break;

                    nativeParent = parent;
                }

                if (nativeParent && nativeParent != this && ::GetFocus() == reinterpret_cast<HWND>(nativeParent->winId()))
                    ::SetFocus(m_hWnd);
            }
            break;
        default:
            // suppresses warning about not all cases handled
            break;
    }
    return QWidget::event(event);
}
