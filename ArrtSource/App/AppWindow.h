#pragma once

#include "ui_AppWindow.h"
#include <Conversion/ConversionManager.h>
#include <QMainWindow>
#include <memory>

class QStatusBar;
class QToolBar;
class QLabel;
class ArrSession;
class SceneState;
class ArrAccount;
class StorageAccount;
class FileUploader;
class ScenegraphModel;
class QProgressBar;
class ArrSettings;

class ArrtAppWindow : public QMainWindow, Ui_AppWindow
{
    Q_OBJECT

public:
    ArrtAppWindow();
    ~ArrtAppWindow();

private Q_SLOTS:
    void onUpdateStatusBar();
    void on_ChangeModelButton_clicked();
    void on_EditSessionButton_clicked();
    void on_SelectSourceButton_clicked();
    void on_SelectOutputFolderButton_clicked();
    void on_ConversionList_currentRowChanged(int row);
    void on_StartConversionButton_clicked();
    void on_ConversionOptionsCheckbox_stateChanged(int);
    void on_ResetAdvancedButton_clicked();
    void on_InspectorButton_clicked();
    void onEntitySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onEntityDoubleClicked(const QModelIndex& index);
    void onEntityPicked();
    void on_MaterialsList_itemSelectionChanged();
    void on_ClearModelsButton_clicked();
    void on_LoadModelSasButton_clicked();
    void on_CameraOptionsButton_clicked();
    void on_ClearLogButton_clicked();

    // Material UI
    void on_AlbedoColorPicker_ColorChanged(const QColor& newColor);
    void on_TransparentCheck_stateChanged(int state);
    void on_WriteDepthCheck_stateChanged(int state);
    void on_TransparencyModeCombo_currentIndexChanged(int index);
    void on_AlphaClipCheck_stateChanged(int state);
    void on_AlphaThresholdSpinner_valueChanged(double d);
    void on_DoubleSidedCheck_stateChanged(int state);
    void on_SpecularCheck_stateChanged(int state);
    void on_VertexColorCheck_stateChanged(int state);
    void on_VertexMixSpinner_valueChanged(double d);
    void on_VertexAlphaModeCombo_currentIndexChanged(int index);
    void on_RoughnessSpinner_valueChanged(double d);
    void on_MetalnessSpinner_valueChanged(double d);
    void on_AoScaleSpinner_valueChanged(double d);
    void on_TextureScaleX_valueChanged(double d);
    void on_TextureScaleY_valueChanged(double d);
    void on_TextureOffsetX_valueChanged(double d);
    void on_TextureOffsetY_valueChanged(double d);
    void on_FadeCheck_stateChanged(int state);
    void on_FadeOutSpinner_valueChanged(double d);
    void on_FresnelCheck_stateChanged(int state);
    void on_FresnelColorPicker_ColorChanged(const QColor& newColor);
    void on_FresnelExponentSpinner_valueChanged(double d);


private:
    void LoadSettings();
    void SaveSettings();

    void FileUploadStatusCallback(int numFiles, bool hadErrors);
    void UpdateConversionsList();
    void UpdateConversionPane();
    void UpdateConversionStartButton();
    void RetrieveConversionOptions();
    void UpdateMaterialsList();
    void UpdateFrameStatisticsUI();

    QStatusBar* m_statusBar = nullptr;
    QLabel* m_statusStorageAccount = nullptr;
    QLabel* m_statusArrAccount = nullptr;
    QLabel* m_statusArrSession = nullptr;
    QProgressBar* m_statusLoadProgress = nullptr;
    int m_numFileUploads = 0;
    QtMsgType m_maxLogType = QtMsgType::QtDebugMsg;

    std::unique_ptr<StorageAccount> m_storageAccount;
    std::unique_ptr<ArrAccount> m_arrAclient;
    std::unique_ptr<ArrSession> m_arrSession;
    std::unique_ptr<SceneState> m_sceneState;
    std::unique_ptr<ConversionManager> m_conversionManager;
    std::unique_ptr<ScenegraphModel> m_scenegraphModel;
    std::unique_ptr<ArrSettings> m_arrSettings;

    QString m_lastStorageDisplayContainer;
    QString m_lastStorageSelectSrcContainer;
    QString m_lastStorageSelectDstContainer;
    QString m_lastStorageLoadModelContainer;

    int m_selectedMaterial = -1;
    std::vector<RR::ApiHandle<RR::Material>> m_materialsList;
    std::map<unsigned long long, RR::ApiHandle<RR::Material>> m_allMaterialsPreviously;

    void SetMaterialUI();
    void ShowMaterialUI();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    static ArrtAppWindow* s_instance;
    static void LogMessageHandlerStatic(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    void LogMessageHandler(QtMsgType type, const QString& category, const QString& msg);
};
