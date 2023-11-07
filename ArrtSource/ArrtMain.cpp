#include <App/AppWindow.h>
#include <ArrtVersion.h>
#include <QApplication>
#include <QCommandLineParser>
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
        m_HighlightColorBase = QColor(100, 130, 20);
        m_HighlightColorLight = m_HighlightColorBase.lighter();
        m_HighlightColorVeryLight = m_HighlightColorLight.lighter();
    }

    QColor m_HighlightColorBase;
    QColor m_HighlightColorLight;
    QColor m_HighlightColorVeryLight;

    virtual void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
    {
        if (element == ControlElement::CE_TabBarTabLabel)
        {
            QStyleOptionTab opt = *static_cast<const QStyleOptionTab*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::WindowText, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);

                painter->setPen(QPen(m_HighlightColorLight, 1, Qt::PenStyle::DotLine));
                painter->drawRect(opt.rect.adjusted(4, 4, -4, -4));
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_PushButtonLabel)
        {
            QStyleOptionButton opt = *static_cast<const QStyleOptionButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::ButtonText, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);

                painter->setPen(QPen(m_HighlightColorLight, 1, Qt::PenStyle::DotLine));
                painter->drawRect(opt.rect.adjusted(2, 2, -2, -2));
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }


        if (element == ControlElement::CE_ToolButtonLabel)
        {
            QStyleOptionToolButton opt = *static_cast<const QStyleOptionToolButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::ButtonText, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);

                painter->setPen(QPen(m_HighlightColorLight, 1, Qt::PenStyle::DotLine));
                painter->drawRect(opt.rect.adjusted(1, 1, -1, -1));
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);
            return;
        }

        if (element == ControlElement::CE_ComboBoxLabel)
        {
            QStyleOptionComboBox opt = *static_cast<const QStyleOptionComboBox*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::ButtonText, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);

                painter->setPen(QPen(m_HighlightColorLight, 1, Qt::PenStyle::DotLine));
                painter->drawRect(opt.rect.adjusted(3, 3, -3, -3));
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_CheckBoxLabel)
        {
            QStyleOptionButton opt = *static_cast<const QStyleOptionButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::WindowText, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        if (element == ControlElement::CE_ShapedFrame)
        {
            QStyleOptionFrame opt = *static_cast<const QStyleOptionFrame*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::Window, m_HighlightColorVeryLight);
            }

            QProxyStyle::drawControl(element, &opt, painter, widget);

            return;
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override
    {
        if (element == QStyle::PrimitiveElement::PE_FrameFocusRect)
            return;

        if (element == QStyle::PrimitiveElement::PE_IndicatorCheckBox)
        {
            QStyleOptionButton opt = *static_cast<const QStyleOptionButton*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::Window, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);
            }

            QProxyStyle::drawPrimitive(element, &opt, painter, widget);
            return;
        }

        if (element == QStyle::PrimitiveElement::PE_FrameLineEdit)
        {
            QStyleOptionFrame opt = *static_cast<const QStyleOptionFrame*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::Window, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);
            }

            QProxyStyle::drawPrimitive(element, &opt, painter, widget);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                painter->setPen(QPen(m_HighlightColorLight, 1, Qt::PenStyle::SolidLine));
                painter->drawRect(opt.rect);
            }

            return;
        }

        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

    virtual void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = nullptr) const override
    {
        if (control == QStyle::CC_SpinBox)
        {
            QStyleOptionSpinBox opt = *static_cast<const QStyleOptionSpinBox*>(option);

            if (option->state.testFlag(QStyle::StateFlag::State_HasFocus))
            {
                opt.palette.setColor(QPalette::ColorRole::Window, m_HighlightColorVeryLight);
                opt.state.setFlag(QStyle::StateFlag::State_HasFocus, false);
            }

            QProxyStyle::drawComplexControl(control, &opt, painter, widget);

            return;
        }

        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }
};

static void SetStyleSheet(QApplication* /*app*/)
{
    // need to have any thing active, even if it's one that doesn't exist
    QIcon::setThemeName("none");

    if (IsHighContrastOn())
    {
        if (QApplication::palette().window().color().lightness() < 150)
        {
            // use icons from the dark theme
            QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":theme-dark");
        }
        else
        {
            // use icons from the light theme
            QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":theme-light");
        }

        // if high contrast mode is enabled, we just don't use custom colors
        return;
    }

    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":theme-dark");

    QApplication::setStyle(new ArrtStyle());
    QPalette palette;

    palette.setColor(QPalette::WindowText, QColor(230, 230, 230, 255));
    palette.setColor(QPalette::Button, QColor(60, 60, 65, 255));
    palette.setColor(QPalette::Light, QColor(97, 97, 97, 255));
    palette.setColor(QPalette::Midlight, QColor(59, 59, 59, 255));
    palette.setColor(QPalette::Dark, QColor(37, 37, 37, 255));
    palette.setColor(QPalette::Mid, QColor(45, 45, 45, 255));
    palette.setColor(QPalette::Text, QColor(220, 220, 220, 255));
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
    palette.setColor(QPalette::PlaceholderText, QColor(150, 150, 150, 255));

    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128, 255));
    palette.setColor(QPalette::Disabled, QPalette::Button, QColor(40, 40, 40, 255));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105, 255));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150, 255));

    palette.setColor(QPalette::Disabled, QPalette::Highlight, palette.highlight().color().darker());
    palette.setColor(QPalette::Inactive, QPalette::Highlight, palette.highlight().color().darker());

    QApplication::setPalette(palette);
}

/// Convert command line arguments from WinMain syntax to argc, argv
struct CommandLineArguments
{
    int argc;
    char** argv;

    CommandLineArguments()
    {
        LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
        argv = new char*[argc];

        for (int i = 0; i < argc; i++)
        {
            size_t len = wcslen(szArglist[i]);
            argv[i] = new char[len + 1];
            argv[i][len] = '\0';
            wcstombs(argv[i], szArglist[i], len);
        }
    }

    ~CommandLineArguments()
	{
        for (int i = 0; i < argc; i++)
        {
			delete[] argv[i];
		}
		delete[] argv;
	}
};

ArrtCommandLineOptions GetCommandLineOptions(const QApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Azure Remote Rendering Toolkit");
    parser.addHelpOption();
    parser.addVersionOption();

    // Mock option (-m, --mock)
    QCommandLineOption mockOption({"m", "mock"}, "Start ARRT on mock mode.");
    parser.addOption(mockOption);
    parser.process(app);

    ArrtCommandLineOptions cmdLineOptions;
    cmdLineOptions.mock = parser.isSet(mockOption);
    return cmdLineOptions;
}


int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif

    QCoreApplication::addLibraryPath(".");
    QCoreApplication::setApplicationName(VER_PRODUCTNAME);
    QCoreApplication::setOrganizationName(VER_COMPANY);
    QCoreApplication::setOrganizationDomain("https://github.com/azure/azure-remote-rendering-asset-tool");
    QCoreApplication::setApplicationVersion(ARRT_VERSION);

    CommandLineArguments cmdLineArgs;
    QApplication app(cmdLineArgs.argc, cmdLineArgs.argv);

    SetStyleSheet(&app);
    auto cmdLineOptions = GetCommandLineOptions(app);

    ArrtAppWindow* appWindow = new ArrtAppWindow(cmdLineOptions);
    appWindow->setWindowTitle("Azure Remote Rendering Toolkit v" ARRT_VERSION);
    appWindow->show();

    return app.exec();
}
