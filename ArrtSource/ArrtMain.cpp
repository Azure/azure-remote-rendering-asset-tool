#include <App/AppWindow.h>
#include <ArrtVersion.h>
#include <QApplication>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleFactory>
#include <windows.h>

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

/// Overrides some colors of the Fusion style for improved visibility of elements with keyboard focus
class ArrtStyle : public QProxyStyle
{
public:
    ArrtStyle()
        : QProxyStyle("fusion")
    {
    }

    virtual void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
    {
        QColor c(100, 130, 20);
        c = c.lighter();
        c = c.lighter();

        if (element == ControlElement::CE_TabBarTabLabel)
        {
            QStyleOptionTab opt = *static_cast<const QStyleOptionTab*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::WindowText, c);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_PushButtonLabel)
        {
            QStyleOptionButton opt = *static_cast<const QStyleOptionButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::ButtonText, c);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_ComboBoxLabel)
        {
            QStyleOptionComboBox opt = *static_cast<const QStyleOptionComboBox*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::ButtonText, c);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_CheckBoxLabel)
        {
            QStyleOptionButton opt = *static_cast<const QStyleOptionButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::WindowText, c);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_ShapedFrame)
        {
            QStyleOptionFrame opt = *static_cast<const QStyleOptionFrame*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::Window, c);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }
};

static void SetStyleSheet(QApplication* /*app*/)
{
    if (IsHighContrastOn())
    {
        // if high contrast mode is enabled, we just don't use a custom theme
        return;
    }

    QApplication::setStyle(new ArrtStyle());
    QPalette palette;

    palette.setColor(QPalette::WindowText, QColor(230, 230, 230, 255));
    palette.setColor(QPalette::Button, QColor(60, 60, 65, 255));
    palette.setColor(QPalette::Light, QColor(97, 97, 97, 255));
    palette.setColor(QPalette::Midlight, QColor(59, 59, 59, 255));
    palette.setColor(QPalette::Dark, QColor(37, 37, 37, 255));
    palette.setColor(QPalette::Mid, QColor(45, 45, 45, 255));
    palette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
    palette.setColor(QPalette::BrightText, QColor(37, 37, 37, 255));
    palette.setColor(QPalette::ButtonText, QColor(230, 230, 230, 255));
    palette.setColor(QPalette::Base, QColor(25, 25, 30, 255));
    palette.setColor(QPalette::Window, QColor(43, 48, 52, 255));
    palette.setColor(QPalette::Shadow, QColor(0, 0, 0, 255));
    palette.setColor(QPalette::Highlight, QColor(80, 110, 20, 255));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 255));
    palette.setColor(QPalette::Link, QColor(0, 148, 255, 255));
    palette.setColor(QPalette::LinkVisited, QColor(255, 0, 220, 255));
    palette.setColor(QPalette::AlternateBase, QColor(46, 46, 46, 255));
    QBrush NoRoleBrush(QColor(0, 0, 0, 255), Qt::NoBrush);
    palette.setBrush(QPalette::NoRole, NoRoleBrush);
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220, 255));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0, 255));
    palette.setColor(QPalette::PlaceholderText, QColor(97, 97, 97, 255));

    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128, 255));
    palette.setColor(QPalette::Disabled, QPalette::Button, QColor(40, 40, 40, 255));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105, 255));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150, 255));

    palette.setColor(QPalette::Disabled, QPalette::Highlight, palette.highlight().color().darker());
    palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.highlight().color().darker());

    QApplication::setPalette(palette);

    // app->setStyleSheet("QTabBar::tab {color: #BBBBBB; selection-background-color: rgb(255, 255, 0);} QTabBar::tab:selected {color: #FFFFFF;}");
}

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif

    QCoreApplication::setApplicationName(VER_PRODUCTNAME);
    QCoreApplication::setOrganizationName(VER_COMPANY);
    QCoreApplication::setOrganizationDomain("https://github.com/azure/azure-remote-rendering-asset-tool");
    QCoreApplication::setApplicationVersion(ARRT_VERSION);

    int argc = 0;
    QApplication app(argc, 0);

    SetStyleSheet(&app);

    ArrtAppWindow* appWindow = new ArrtAppWindow();
    appWindow->setWindowTitle("Azure Remote Rendering Toolkit v" ARRT_VERSION);
    appWindow->show();

    return app.exec();
}
