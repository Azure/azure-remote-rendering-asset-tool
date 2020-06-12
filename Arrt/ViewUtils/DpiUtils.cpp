#include <QApplication>
#include <ViewUtils/DpiUtils.h>

//from qstylehelper.cpp
Q_GUI_EXPORT int qt_defaultDpiX();

// scale factor used to convert logical sizes into dpi scaled logical sizes
static qreal gScaleFactor = 0.0f;

qreal DpiUtils::size(qreal value)
{
#ifdef Q_OS_MAC
    // On mac the DPI is always 72 so we should not scale it
    return value;
#else
    if (gScaleFactor == 0)
    {
        bool dpiScaling = QApplication::testAttribute(Qt::AA_EnableHighDpiScaling) && !QApplication::testAttribute(Qt::AA_DisableHighDpiScaling);
        gScaleFactor = dpiScaling ? 1.0f : (qreal(qt_defaultDpiX()) / 96.0);
    }
    return value * gScaleFactor;
#endif
}
