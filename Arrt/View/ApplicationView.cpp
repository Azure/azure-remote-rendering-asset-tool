#include <QBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include <ViewUtils/DpiUtils.h>
#include <View/ApplicationView.h>
#include <View/ArrtStyle.h>
#include <View/Conversion/ConversionPageView.h>
#include <View/Log/LogView.h>
#include <View/ModelEditor/ModelEditorView.h>
#include <View/ModelsPage/ModelsPageView.h>
#include <View/NotificationButtonView.h>
#include <View/Render/RenderPageView.h>
#include <View/Session/SessionCreationView.h>
#include <View/Session/SessionInfoButton.h>
#include <View/Session/SessionInfoView.h>
#include <View/Settings/SettingsView.h>
#include <View/Upload/UploadView.h>
#include <ViewModel/ApplicationModel.h>
#include <ViewModel/Conversion/ConversionPageModel.h>
#include <ViewModel/Log/LogModel.h>
#include <ViewModel/Render/RenderPageModel.h>
#include <ViewModel/Settings/ArrAccountSettingsModel.h>
#include <ViewModel/Settings/SettingsModel.h>
#include <ViewModel/Upload/UploadModel.h>
#include <Widgets/FlatButton.h>
#include <Widgets/Navigator.h>

ApplicationView::ApplicationView(ApplicationModel* model, QWidget* parent)
    : QMainWindow(parent)
{
    // initial size
    resize(1200, 800);
    m_model = model;

    m_uploadView = new UploadView(model->getUploadModel());

    // Setup of conversion main panel
    {
        m_conversionNavigator = new Navigator(nullptr);
        m_conversionNavigator->addPage(new ConversionPageView(m_model->getConversionPageModel()), CONV_CONVERSION);
    }

    // Setup of rendering main panel
    {
        m_arrNavigator = new RenderPageView(m_model->getRenderPageModel(), nullptr);
    }

    auto settingsModel = m_model->getSettingsModel();
    m_settingsView = new SettingsView(settingsModel, this);

    // Setup of top level navigator
    NotificationButtonView* settingsButton;
    {
        m_topLevelNavigator = new Navigator(nullptr);

        m_topLevelNavigator->setPageFactory([this](int index) -> QWidget* {
            switch (index)
            {
                case TOPLEVEL_UPLOAD:
                    return m_uploadView;
                case TOPLEVEL_CONVERSION:
                    return m_conversionNavigator;
                case TOPLEVEL_RENDERING:
                    return m_arrNavigator;
                default:
                    return nullptr;
            }
        });

        {
            auto* button = new NotificationButtonView(tr("Upload"), m_model->getUploadModel()->getNotificationButtonModel());
            button->setToolTip(tr("Upload"), tr("Panel to upload the 3D model files to Azure Storage before conversion."));
            connect(button, &FlatButton::pressed, this, [this]() { m_topLevelNavigator->navigateToPage(TOPLEVEL_UPLOAD); });
            button->setIcon(ArrtStyle::s_uploadIcon, true);
            m_mainButtons[TOPLEVEL_UPLOAD] = button;
            button->setAutoExclusive(true);
        }

        {
            auto* button = new NotificationButtonView(tr("Convert"), m_model->getConversionPageModel()->getNotificationButtonModel());
            button->setToolTip(tr("Convert"), tr("Main panel to control the 3D model conversion tasks. The result of the conversion is a model in the Azure Remote Rendering internal format, which can be loaded and rendered by the service"));
            connect(button, &FlatButton::pressed, this, [this]() { m_topLevelNavigator->navigateToPage(TOPLEVEL_CONVERSION); });
            button->setIcon(ArrtStyle::s_conversionIcon, true);
            m_mainButtons[TOPLEVEL_CONVERSION] = button;
            button->setAutoExclusive(true);
        }

        {
            auto* button = new NotificationButtonView(tr("Render"), m_model->getRenderPageModel()->getNotificationButtonModel());
            button->setToolTip(tr("Render"), tr("Main panel to control the remote rendering with ARR. Here you can start a session, load a model and visualize and navigate the remotely rendered 3D model"));
            connect(button, &FlatButton::pressed, this, [this]() { m_topLevelNavigator->navigateToPage(TOPLEVEL_RENDERING); });
            button->setIcon(ArrtStyle::s_renderingIcon, true);
            m_mainButtons[TOPLEVEL_RENDERING] = button;
            button->setAutoExclusive(true);
        }

        settingsButton = new NotificationButtonView(tr("Settings"), settingsModel->getNotificationButtonModel());
        settingsButton->setToolTip(tr("Settings panel"), tr("Access the settings for connecting to the Azure Remote Rendering and Azure Storage and to configure video streaming and camera controls"));
        settingsButton->setIcon(ArrtStyle::s_settingsIcon);
        settingsButton->setIconSize(QSize(DpiUtils::size(30), DpiUtils::size(30)));

        m_settingsView->setVisible(false);
        connect(settingsButton, &FlatButton::toggled, this, [this](bool checked) {
            m_settingsView->setVisible(checked);
        });

        // if the ARR account is not set up, start with the settings panel visible
        auto arrStatus = settingsModel->getArrAccountSettingsModel()->getStatus();
        if (arrStatus == AccountConnectionStatus::Disconnected || arrStatus == AccountConnectionStatus::FailedToConnect)
        {
            settingsButton->toggle();
        }

        connect(m_topLevelNavigator, &Navigator::pageNavigated, this,
                [this](int index) {
                    m_mainButtons[index]->setChecked(true);
                });
    }

    // Log button and view
    NotificationButtonView* logButton;
    {
        m_logView = new LogView(m_model->getLogModel());

        logButton = new NotificationButtonView(tr("Logs"), m_model->getLogModel()->getNotificationButtonModel());
        logButton->setToolTip(tr("Log Panel"), tr("Toggle visualization of the log panel."));
        logButton->setIcon(ArrtStyle::s_logIcon, true);


        QObject::connect(logButton, &NotificationButtonView::toggled, this, [this](bool checked) {
            m_logView->setVisible(checked);
        });

        m_logView->setVisible(false);
    }

    // Tab bar for top level navigator
    QHBoxLayout* tabBarLayout;
    {
        tabBarLayout = new QHBoxLayout();
        tabBarLayout->setSpacing(5);

        tabBarLayout->addWidget(settingsButton);

        // since the buttons are "autoExclusive", they should be grouped in the same widget to work properly.
        QWidget* buttonGroup = new QWidget();
        buttonGroup->setContentsMargins(0, 0, 0, 0);
        auto* buttonGroupLayout = new QHBoxLayout(buttonGroup);
        buttonGroupLayout->setContentsMargins(0, 0, 0, 0);
        buttonGroupLayout->addWidget(m_mainButtons[TOPLEVEL_UPLOAD]);
        buttonGroupLayout->addWidget(m_mainButtons[TOPLEVEL_CONVERSION]);
        buttonGroupLayout->addWidget(m_mainButtons[TOPLEVEL_RENDERING]);

        tabBarLayout->addWidget(buttonGroup);
        tabBarLayout->addStretch(1);
        tabBarLayout->addWidget(logButton);
    }

    // Top level widget
    {
        QWidget* topLevelWidget = new QWidget(this);
        QVBoxLayout* mainVLayout = new QVBoxLayout(topLevelWidget);
        mainVLayout->setSpacing(10);
        mainVLayout->addLayout(tabBarLayout, 0);


        QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal, topLevelWidget);

        horizontalSplitter->addWidget(m_settingsView);
        horizontalSplitter->addWidget(m_topLevelNavigator);

        QSplitter* topLevelSplitter = new QSplitter(Qt::Vertical, topLevelWidget);
        topLevelSplitter->addWidget(horizontalSplitter);
        topLevelSplitter->addWidget(m_logView);

        mainVLayout->addWidget(topLevelSplitter, 1);

        setCentralWidget(topLevelWidget);
    }

    // Init navigators
    m_conversionNavigator->navigateToPage(CONV_CONVERSION);
    m_topLevelNavigator->navigateToPage(TOPLEVEL_RENDERING);
}

ApplicationView::~ApplicationView()
{
}
