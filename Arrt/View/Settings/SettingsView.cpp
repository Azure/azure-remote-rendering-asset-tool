#include <QVBoxLayout>
#include <View/Settings/ArrAccountSettingsView.h>
#include <View/Settings/CameraSettingsView.h>
#include <View/Settings/SettingsView.h>
#include <View/Settings/StorageAccountSettingsView.h>
#include <View/Settings/VideoSettingsView.h>
#include <ViewModel/Settings/SettingsModel.h>
#include <Widgets/FormControl.h>
#include <Widgets/ReadOnlyText.h>
#include <Widgets/VerticalScrollArea.h>

SettingsView::SettingsView(SettingsModel* settingsModel, QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    auto* listLayout = new QVBoxLayout(this);
    listLayout->setContentsMargins(0, 0, 0, 0);
    auto* scrollArea = new VerticalScrollArea(this);
    listLayout->addWidget(scrollArea, 1);

    {
        // version information
        auto* versionLabel = new ReadOnlyText(settingsModel->getVersion());
        scrollArea->getContentLayout()->addWidget(new FormControl(tr("Application version"), versionLabel));
    }

    auto* const settingsViewParent = scrollArea->widget();
    {
        auto* view = new ArrAccountSettingsView(settingsModel->getArrAccountSettingsModel(), settingsViewParent);
        auto* fc = new FormControl(tr("Azure Remote Rendering account"), view);
        fc->setToolTip(tr("Azure Remote Rendering Account Settings"), tr("Account settings for Azure Remote Rendering. Changing the values will cause ARRT to re-connect, and any loaded model will be unloaded.<br>See the user documentation to know how to retrieve these parameters"));
        scrollArea->getContentLayout()->addWidget(fc);
    }
    {
        auto* view = new StorageAccountSettingsView(settingsModel->getStorageAccountSettingsModel(), settingsViewParent);
        auto* fc = new FormControl(tr("Azure Storage account"), view);
        fc->setToolTip(tr("Azure Storage Account Settings"),
                       tr("Account settings for Azure Storage. The blob explorers will operate on the blob containers in this account when selecting input/output or 3D model to load.<br>See the user documentation to know how to retrieve these settings from a storage account"));
        scrollArea->getContentLayout()->addWidget(fc);
    }
    {
        auto view = new VideoSettingsView(settingsModel->getVideoSettingsModel(), settingsViewParent);
        auto* fc = new FormControl(tr("Video streaming"), view);
        fc->setToolTip(tr("Video streaming settings"), tr("Video settings affecting the remote streaming in the 3D viewport. Changing these parameters will require ARRT to re-connect, and any loaded model will be unloaded"));
        scrollArea->getContentLayout()->addWidget(fc);
    }
    {
        auto view = new CameraSettingsView(settingsModel->getCameraSettingsModel(), settingsViewParent);
        auto* fc = new FormControl(tr("Camera"), view);
        fc->setToolTip(tr("Camera Settings"), tr("These settings will immediately affect the navigation in the 3D viewport and the 3D projection"));
        scrollArea->getContentLayout()->addWidget(fc);
    }
}
