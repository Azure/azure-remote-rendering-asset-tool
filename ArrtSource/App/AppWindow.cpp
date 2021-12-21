#include <App/AppWindow.h>
#include <App/SettingsDlg.h>
#include <ArrtVersion.h>
#include <QDesktopServices>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <Rendering/ArrAccount.h>
#include <Rendering/ArrSession.h>
#include <Rendering/ArrSettings.h>
#include <Rendering/UI/SceneState.h>
#include <Rendering/UI/ScenegraphModel.h>
#include <Rendering/UI/StartSessionDlg.h>
#include <Rendering/UI/ViewportWidget.h>
#include <Storage/FileUploader.h>
#include <Storage/StorageAccount.h>
#include <Utils/LogHelpers.h>
#include <qevent.h>

ArrtAppWindow* ArrtAppWindow::s_instance = nullptr;

ArrtAppWindow::ArrtAppWindow()
{
    setupUi(this);

    ArrtAppWindow::s_instance = this;
    qInstallMessageHandler(&ArrtAppWindow::LogMessageHandlerStatic);

    bool displaySettingsDialog = false;

    LoadSettings();

    m_storageAccount = std::make_unique<StorageAccount>([this](int numFiles, bool errors)
                                                        { FileUploadStatusCallback(numFiles, errors); });
    m_arrSettings = std::make_unique<ArrSettings>();
    m_arrSettings->LoadSettings();

    m_arrAclient = std::make_unique<ArrAccount>();
    m_sceneState = std::make_unique<SceneState>(m_arrSettings.get());
    m_arrSession = std::make_unique<ArrSession>(m_arrAclient.get(), m_sceneState.get());
    m_conversionManager = std::make_unique<ConversionManager>(m_storageAccount.get(), m_arrAclient.get());

    // setup menu bar
    {
        QMenuBar* menuBar = new QMenuBar();
        setMenuBar(menuBar);

        // settings menu
        {
            QMenu* settingsMenu = menuBar->addMenu("Settings");

            QAction* act = settingsMenu->addAction(tr("Account Settings"), [this]()
                                                   {
                                                       SettingsDlg dlg(m_storageAccount.get(), m_arrAclient.get(), this);
                                                       dlg.exec();
                                                   });
            act->setIcon(QIcon(":/ArrtApplication/Icons/settings.svg"));
        }

        // help menu
        {
            QMenu* helpMenu = menuBar->addMenu("Help");

            helpMenu->addAction(tr("Send Feedback"), [this]()
                                { QDesktopServices::openUrl(QUrl("https://feedback.azure.com/d365community/forum/46aa4cc0-fd24-ec11-b6e6-000d3a4f07b8")); });
            helpMenu->addAction(tr("File an Issue"), [this]()
                                { QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/issues/new")); });
            helpMenu->addAction(tr("Open Releases"), [this]()
                                { QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/releases")); });
            helpMenu->addAction(tr("Open Documentation"), [this]()
                                { QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/blob/main/Documentation/index.md")); });
            helpMenu->addAction(tr("Privacy Statement"), [this]()
                                { QDesktopServices::openUrl(QUrl("https://privacy.microsoft.com/privacystatement")); });
            helpMenu->addAction(tr("About ARRT"), [this]()
                                { QMessageBox::information(this, "About ARRT", VER_PRODUCTNAME "\n\n" VER_COPYRIGHT " " VER_COMPANY "\n\nVersion: " ARRT_VERSION, QMessageBox::Ok); });
        }
    }

    // setup the status bar
    {
        m_statusBar = new QStatusBar(this);
        setStatusBar(m_statusBar);

        auto addSeperator = [&]()
        {
            QFrame* f = new QFrame(m_statusBar);
            f->setFrameShape(QFrame::Shape::VLine);
            f->setFrameShadow(QFrame::Sunken);
            m_statusBar->addWidget(f);
        };

        m_statusStorageAccount = new QLabel(m_statusBar);
        m_statusStorageAccount->setTextFormat(Qt::TextFormat::RichText);
        m_statusBar->addWidget(m_statusStorageAccount);
        addSeperator();

        m_statusArrAccount = new QLabel(m_statusBar);
        m_statusArrAccount->setTextFormat(Qt::TextFormat::RichText);
        m_statusBar->addWidget(m_statusArrAccount);
        addSeperator();

        m_statusArrSession = new QLabel(m_statusBar);
        m_statusArrSession->setTextFormat(Qt::TextFormat::RichText);
        m_statusBar->addWidget(m_statusArrSession);
        addSeperator();

        m_statusLoadProgress = new QProgressBar(m_statusBar);
        m_statusBar->addWidget(m_statusLoadProgress);
    }

    connect(m_storageAccount.get(), &StorageAccount::ConnectionStatusChanged, this, &ArrtAppWindow::onUpdateStatusBar);
    connect(m_arrAclient.get(), &ArrAccount::ConnectionStatusChanged, this, &ArrtAppWindow::onUpdateStatusBar);
    connect(m_arrSession.get(), &ArrSession::SessionStatusChanged, this, &ArrtAppWindow::onUpdateStatusBar);
    connect(m_arrSession.get(), &ArrSession::ModelLoadProgressChanged, this, &ArrtAppWindow::onUpdateStatusBar);

    if (!m_arrAclient->LoadSettings() || !m_storageAccount->LoadSettings())
    {
        displaySettingsDialog = true;
    }

    m_storageAccount->ConnectToStorageAccount();
    m_arrAclient->ConnectToArrAccount();

    StorageBrowser->SetStorageAccount(m_storageAccount.get(), StorageEntry::Type::Other, m_lastStorageDisplayContainer);

    Viewport->SetSceneState(m_sceneState.get());

    m_scenegraphModel = std::make_unique<ScenegraphModel>(m_arrSession.get());
    ScenegraphView->setModel(m_scenegraphModel.get());

    connect(ScenegraphView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ArrtAppWindow::onEntitySelectionChanged);
    connect(ScenegraphView, &QTreeView::doubleClicked, this, &ArrtAppWindow::onEntityDoubleClicked);

    connect(m_sceneState.get(), &SceneState::PickedEntity, this, &ArrtAppWindow::onEntityPicked);

    // when a model gets loaded, the scenegraph model needs to be refreshed
    connect(m_arrSession.get(), &ArrSession::ModelLoaded, this, [this]()
            {
                ScenegraphView->selectionModel()->clearSelection();
                m_scenegraphModel->RefreshModel();
                ClearModelsButton->setEnabled(m_arrSession->GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected && m_arrSession->GetLoadedModels().size() > 0);

                if (m_arrSession->GetLoadedModels().size() == 1)
                {
                    m_sceneState->FocusOnSelectedEntity();
                }
            });

    connect(m_arrSession.get(), &ArrSession::SessionStatusChanged, this, [this]()
            {
                if (!m_arrSession || !m_arrSession->GetSessionStatus().IsRunning())
                {
                    ScenegraphView->selectionModel()->clearSelection();
                    m_scenegraphModel->RefreshModel();
                }
            });

    // when the selected conversion changes, the conversion pane (showing the details) has to be updated
    connect(m_conversionManager.get(), &ConversionManager::SelectedChanged, this, [this]()
            { UpdateConversionPane(); });

    // when the number of conversion changes (one was added), the list of conversions needs to be updated
    connect(m_conversionManager.get(), &ConversionManager::ListChanged, this, [this]()
            {
                UpdateConversionsList();
                onUpdateStatusBar();
            });

    connect(m_arrSession.get(), &ArrSession::FrameStatisticsChanged, this, [this]()
            { UpdateFrameStatisticsUI(); });

    // make sure the UI is properly initialized for the first time, before any data is read from it
    UpdateConversionPane();
    UpdateConversionsList();
    ConversionList->setCurrentRow(0);

    ShowMaterialUI();

    // give the scenegraph / 3D view / material panel a proper ratio
    // based on https://stackoverflow.com/questions/43831474/how-to-equally-distribute-the-width-of-qsplitter/43835396
    ((QSplitter*)RenderSplitter)->setSizes(QList<int>({800, 2000, 800}));

    onUpdateStatusBar();

    if (displaySettingsDialog)
    {
        QTimer::singleShot(500, this, [this]()
                           {
                               SettingsDlg dlg(m_storageAccount.get(), m_arrAclient.get(), this);
                               dlg.exec();
                           });
    }

#ifdef NDEBUG
    QTimer::singleShot(500, this, [this]()
                       { CheckForNewVersion(); });
#endif
}

ArrtAppWindow::~ArrtAppWindow()
{
    m_conversionManager = nullptr;
    m_sceneState = nullptr;
    m_arrSession = nullptr;
    m_arrAclient = nullptr;
    m_storageAccount = nullptr;
}

void ArrtAppWindow::onUpdateStatusBar()
{
    if (m_arrSession == nullptr)
        return;

    switch (m_storageAccount->GetConnectionStatus())
    {
        case AccountConnectionStatus::Authenticated:
            if (m_numFileUploads > 0)
            {
                m_statusStorageAccount->setText(QString("<html><head/><body><p>Storage Account: <span style=\"color:#ffaa00;\">Uploading %1 files</span></p></body></html>").arg(m_numFileUploads));
            }
            else
            {
                m_statusStorageAccount->setText("<html><head/><body><p>Storage Account: <span style=\"color:#00aa00;\">Connected</span></p></body></html>");
            }
            break;
        case AccountConnectionStatus::CheckingCredentials:
            m_statusStorageAccount->setText("<html><head/><body><p>Storage Account: <span style=\"color:#ffaa00;\">Checking...</span></p></body></html>");
            break;
        case AccountConnectionStatus::InvalidCredentials:
            m_statusStorageAccount->setText("<html><head/><body><p>Storage Account: <span style=\"color:#aa0000;\">Invalid Credentials</span></p></body></html>");
            break;
        case AccountConnectionStatus::NotAuthenticated:
            m_statusStorageAccount->setText("<html><head/><body><p>Storage Account: <span style=\"color:#7e7e7e;\">Not Connected</span></p></body></html>");
            break;
        default:
            break;
    }


    switch (m_arrAclient->GetConnectionStatus())
    {
        case AccountConnectionStatus::Authenticated:
        {
            const uint32_t activeConversions = m_conversionManager->GetNumActiveConversions();

            if (activeConversions > 0)
            {
                m_statusArrAccount->setText(QString("<html><head/><body><p>ARR Account: <span style=\"color:#ffaa00;\">%1 conversions running</span></p></body></html>").arg(activeConversions));
            }
            else
            {
                m_statusArrAccount->setText("<html><head/><body><p>ARR Account: <span style=\"color:#00aa00;\">Connected</span></p></body></html>");
            }
            break;
        }

        case AccountConnectionStatus::CheckingCredentials:
            m_statusArrAccount->setText("<html><head/><body><p>ARR Account: <span style=\"color:#ffaa00;\">Checking...</span></p></body></html>");
            break;
        case AccountConnectionStatus::InvalidCredentials:
            m_statusArrAccount->setText("<html><head/><body><p>ARR Account: <span style=\"color:#aa0000;\">Invalid Credentials</span></p></body></html>");
            break;
        case AccountConnectionStatus::NotAuthenticated:
            m_statusArrAccount->setText("<html><head/><body><p>ARR: <span style=\"color:#7e7e7e;\">Not Connected</span></p></body></html>");
            break;
    }

    switch (m_arrSession->GetSessionStatus().m_state)
    {
        case ArrSessionStatus::State::NotActive:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#7e7e7e;\">Not Active</span></p></body></html>");
            break;
        case ArrSessionStatus::State::Stopped:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#aaaaff;\">Stopped</span></p></body></html>");
            break;
        case ArrSessionStatus::State::Expired:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#aaaaff;\">Expired</span></p></body></html>");
            break;
        case ArrSessionStatus::State::Error:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#aa0000;\">Error!</span></p></body></html>");
            break;
        case ArrSessionStatus::State::StartRequested:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#ffaa00;\">Start Requested...</span></p></body></html>");
            break;
        case ArrSessionStatus::State::Starting:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#ffaa00;\">Starting...</span></p></body></html>");
            break;
        case ArrSessionStatus::State::ReadyNotConnected:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#00aa00;\">Ready</span></p></body></html>");
            break;
        case ArrSessionStatus::State::ReadyConnecting:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#ffaa00;\">Connecting...</span></p></body></html>");
            break;
        case ArrSessionStatus::State::ReadyConnected:
            m_statusArrSession->setText("<html><head/><body><p>Session: <span style=\"color:#00aa00;\">Connected</span></p></body></html>");
            break;
    }

    EditSessionButton->setEnabled(m_arrAclient->GetConnectionStatus() == AccountConnectionStatus::Authenticated);

    ChangeModelButton->setEnabled(m_arrSession->GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected);
    LoadModelSasButton->setEnabled(m_arrSession->GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected);
    CameraOptionsButton->setEnabled(m_arrSession->GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected);
    ClearModelsButton->setEnabled(m_arrSession->GetSessionStatus().m_state == ArrSessionStatus::State::ReadyConnected && m_arrSession->GetLoadedModels().size() > 0);

    if (m_arrSession->GetSessionStatus().IsRunning())
    {
        EditSessionButton->setIcon(QIcon(":/ArrtApplication/Icons/stop.svg"));
        InspectorButton->setEnabled(true);
    }
    else
    {
        EditSessionButton->setIcon(QIcon(":/ArrtApplication/Icons/start.svg"));
        InspectorButton->setEnabled(false);
    }

    float fModelLoad = m_arrSession->GetModelLoadingProgress();

    m_statusLoadProgress->setFormat("Loading Model: %p%");
    m_statusLoadProgress->setVisible(fModelLoad > 0 && fModelLoad < 1.0f && m_arrSession->GetSessionStatus().IsRunning());
    m_statusLoadProgress->setValue((int)(fModelLoad * 100));
}

void ArrtAppWindow::LoadSettings()
{
    QSettings s;
    s.beginGroup("AppSettings");
    m_lastStorageDisplayContainer = s.value("DisplayContainer").toString();
    m_lastStorageSelectSrcContainer = s.value("SelectSrcContainer").toString();
    m_lastStorageSelectDstContainer = s.value("SelectDstContainer").toString();
    m_lastStorageLoadModelContainer = s.value("SelectModelContainer").toString();
    s.endGroup();
}

void ArrtAppWindow::SaveSettings()
{
    m_lastStorageDisplayContainer = StorageBrowser->GetSelectedContainer();

    QSettings s;
    s.beginGroup("AppSettings");
    s.setValue("DisplayContainer", m_lastStorageDisplayContainer);
    s.setValue("SelectSrcContainer", m_lastStorageSelectSrcContainer);
    s.setValue("SelectDstContainer", m_lastStorageSelectDstContainer);
    s.setValue("SelectModelContainer", m_lastStorageLoadModelContainer);
    s.endGroup();
}

void ArrtAppWindow::closeEvent(QCloseEvent* e)
{
    SaveSettings();

    if (m_numFileUploads > 0)
    {
        if (QMessageBox::question(this, "Cancel File Uploads?", QString("%1 files are currently being uploaded. Closing ARRT will cancel all uploads.\n\nContinue anyway?").arg(m_numFileUploads), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            e->setAccepted(false);
            return;
        }
    }

    m_arrSession = nullptr;

    m_arrAclient->DisconnectFromArrAccount();
    m_storageAccount->DisconnectFromStorageAccount();

    QMainWindow::closeEvent(e);
}

void ArrtAppWindow::CheckForNewVersion()
{
    QPointer<ArrtAppWindow> thisPtr = this;

    using namespace web::http;
    client::http_client client(L"https://api.github.com/repos/Azure/azure-remote-rendering-asset-tool/releases/latest");
    client.request(methods::GET).then([thisPtr, this](const pplx::task<http_response>& previousTask)
                                      {
                                          try
                                          {
                                              auto response = previousTask.get();
                                              if (response.status_code() == status_codes::OK && thisPtr)
                                              {
                                                  auto json = response.extract_json().get();
                                                  QString latestVersion = QString::fromStdWString(json.at(L"tag_name").as_string());

                                                  QMetaObject::invokeMethod(QApplication::instance(), [this, latestVersion]()
                                                                            { OnCheckForNewVersionResult(latestVersion); });
                                              }
                                          }
                                          catch (...)
                                          {
                                              // in case of any exception, don't show the dialog
                                          }
                                      });
}

void ArrtAppWindow::OnCheckForNewVersionResult(QString latestVersion)
{
    const QString currentVersion(ARRT_VERSION);

    if (latestVersion.startsWith('v', Qt::CaseInsensitive))
    {
        latestVersion = latestVersion.mid(1);
    }

    if (currentVersion != latestVersion)
    {
        if (QMessageBox::question(this, "New Version Available", QString("ARRT version %1 is now available.\n\nDo you want to open the GitHub release page?").arg(latestVersion), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl("https://github.com/Azure/azure-remote-rendering-asset-tool/releases"));
        }
    }
}
