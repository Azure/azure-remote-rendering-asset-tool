#include <App/AppWindow.h>
#include <QApplication>
#include <QStyleFactory>
#include <windows.h>
#include <ArrtVersion.h>

static bool IsHighContrastOn()
{
    HIGHCONTRAST info = {0};
    info.cbSize = sizeof(HIGHCONTRAST);
    BOOL ok = ::SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &info, 0);

    if (ok)
    {
        return info.dwFlags & HCF_HIGHCONTRASTON;
    }
    else
    {
        return false;
    }
}

static void SetStyleSheet()
{
    if (IsHighContrastOn())
    {
        // if high contrast mode is enabled, we just don't use a custom theme
        return;
    }

    QApplication::setStyle(QStyleFactory::create("fusion"));
    QPalette palette;

    palette.setColor(QPalette::WindowText, QColor(200, 200, 200, 255));
    palette.setColor(QPalette::Button, QColor(100, 100, 100, 255));
    palette.setColor(QPalette::Light, QColor(97, 97, 97, 255));
    palette.setColor(QPalette::Midlight, QColor(59, 59, 59, 255));
    palette.setColor(QPalette::Dark, QColor(37, 37, 37, 255));
    palette.setColor(QPalette::Mid, QColor(45, 45, 45, 255));
    palette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
    palette.setColor(QPalette::BrightText, QColor(37, 37, 37, 255));
    palette.setColor(QPalette::ButtonText, QColor(200, 200, 200, 255));
    palette.setColor(QPalette::Base, QColor(42, 42, 42, 255));
    palette.setColor(QPalette::Window, QColor(68, 68, 68, 255));
    palette.setColor(QPalette::Shadow, QColor(0, 0, 0, 255));
    palette.setColor(QPalette::Highlight, QColor(143, 58, 255, 255));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 255));
    palette.setColor(QPalette::Link, QColor(0, 148, 255, 255));
    palette.setColor(QPalette::LinkVisited, QColor(255, 0, 220, 255));
    palette.setColor(QPalette::AlternateBase, QColor(46, 46, 46, 255));
    QBrush NoRoleBrush(QColor(0, 0, 0, 255), Qt::NoBrush);
    palette.setBrush(QPalette::NoRole, NoRoleBrush);
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220, 255));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0, 255));

    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128, 255));
    palette.setColor(QPalette::Disabled, QPalette::Button, QColor(80, 80, 80, 255));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105, 255));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128, 255));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(86, 117, 148, 255));

    QApplication::setPalette(palette);
}

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);

    QCoreApplication::setApplicationName("ARRT");
    QCoreApplication::setOrganizationName(VER_COMPANY);
    QCoreApplication::setOrganizationDomain("https://github.com/Azure/azure-remote-rendering-asset-tool");
    QCoreApplication::setApplicationVersion(ARRT_VERSION);
    SetStyleSheet();

    int argc = 0;
    QApplication app(argc, 0);

    ArrtAppWindow* appWindow = new ArrtAppWindow();
    appWindow->show();

    int ret = app.exec();


    return ret;
}
