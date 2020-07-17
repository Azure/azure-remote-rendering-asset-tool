#pragma once

#include <QtWidgets/QMainWindow>

class ApplicationModel;
class FlatButton;
class Navigator;
class SettingsView;
class LogView;
class UploadView;
class SessionPanelView;
class NewVersionModel;

    // Main application UI, holds the root navigation panel, handles the back button and the navigation to the different editors

    class ApplicationView : public QMainWindow
{
public:
    ApplicationView(ApplicationModel* model, QWidget* parent = Q_NULLPTR);
    ~ApplicationView();

    enum TopLevelPageType
    {
        TOPLEVEL_UPLOAD = 0,
        TOPLEVEL_CONVERSION,
        TOPLEVEL_RENDERING,
        TOPLEVEL_COUNT
    };

    enum ArrPageType
    {
        ARRPAGE_MODELS = 0, // model loading panel (ModelsPageView)
        ARRPAGE_MATERIALS,  // viewport and material editing panel (ModelEditorView)
        ARRPAGE_COUNT
    };

    enum ConversionPageType
    {
        CONV_CONVERSION = 0
    };

protected:
    FlatButton* m_mainButtons[(int)TopLevelPageType::TOPLEVEL_COUNT] = {};

    ApplicationModel* m_model = {};

    QTabWidget* m_mainTab = {};

    Navigator* m_topLevelNavigator = {};

    UploadView* m_uploadView = {};
    Navigator* m_conversionNavigator = {};
    Navigator* m_arrNavigator = {};

    SettingsView* m_settingsView = {};

    SessionPanelView* m_sessionPanelView = {};
    LogView* m_logView = {};

    void openAboutDialog();
    void openNewVersionDialog(NewVersionModel* unparentedNewVersionModel);
};
