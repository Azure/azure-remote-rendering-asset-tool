#pragma once
#include <QSet>
#include <d3d11.h>
#include <qwidget.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
class ViewportModel;

// Direct X test widget, to be used to bind a DX swap chain to a widget

class ViewportView : public QWidget
{
public:
    ViewportView(ViewportModel* model, QWidget* parent /* = nullptr */);
    ~ViewportView();

protected:
    virtual QPaintEngine* paintEngine() const override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual bool event(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    ViewportModel* m_viewportModel;

    IDXGIFactory* m_dxgiFactory = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_RTView = nullptr;

    HWND m_hWnd;
    QPointF m_dragStartPoint;
    float m_cameraForwardSpeed = 0.0f;
    float m_cameraLateralSpeed = 0.0f;
    float m_cameraUpdownSpeed = 0.0f;
    float m_cameraRotateSpeedX = 0.0f;
    float m_cameraRotateSpeedY = 0.0f;
    QSet<int> m_pressedKeys;

    void updateFromKeyboard();
};
