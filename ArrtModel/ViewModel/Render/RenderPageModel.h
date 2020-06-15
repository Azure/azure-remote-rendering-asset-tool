#pragma once
#include <Model/ArrSessionManager.h>
#include <QObject>

class AzureStorageManager;
class ArrSessionManager;
class Configuration;
class NotificationButtonModel;
class NotificationButtonModelImplementation;
class ModelEditorModel;
class ModelsPageModel;
class SessionPanelModel;
class ArrSessionManager;

// model representing the "Render" page, which has session status, model loading page and material editor

class RenderPageModel : public QObject
{
    Q_OBJECT
public:
    enum class FocusedPage
    {
        MODEL_SELECTION = 0,
        MODEL_EDITING
    };

    RenderPageModel(AzureStorageManager* storageManager, ArrSessionManager* sessionManager, Configuration* configuration, QObject* parent);

    NotificationButtonModel* getNotificationButtonModel() const;

    FocusedPage getFocusedPage() const;

    SessionPanelModel* getSessionPanelModel() const;
    ModelEditorModel* getModelEditorModel() const;
    ModelsPageModel* getModelsPageModel() const;

Q_SIGNALS:
    void focusedPageChanged();

private:
    FocusedPage m_focusedPage = FocusedPage::MODEL_SELECTION;

    ArrSessionManager* const m_sessionManager;

    SessionPanelModel* const m_sessionPanelModel;
    ModelEditorModel* const m_modelEditor;
    ModelsPageModel* const m_modelsPage;

    bool m_isModelLoaded = false;
    NotificationButtonModelImplementation* const m_buttonModel;
    SessionStatus::Status m_status = SessionStatus::Status::NotActive;

    void updateFocusedPage();
};
