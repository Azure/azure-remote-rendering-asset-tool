#pragma once

#include <QSet>
#include <d3d11.h>
#include <qwidget.h>

class SceneState;

/// The central 3D widget where the remote content is presented
///
/// Also takes care of checking the keyboard/mouse input and forwarding that to the SceneState.
class ViewportWidget : public QWidget
{
public:
    ViewportWidget(QWidget* parent);
    ~ViewportWidget();

    void SetSceneState(SceneState* state);

protected:
    virtual QPaintEngine* paintEngine() const override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    void UpdateInput();

    SceneState* m_sceneState = nullptr;

    IDXGIFactory* m_dxgiFactory = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_RTView = nullptr;

    HWND m_hWnd;
    QPointF m_dragStartPoint;
    QSet<int> m_pressedKeys;
};
