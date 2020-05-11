#include <QVBoxLayout>
#include <View/Settings/CameraSettingsView.h>
#include <ViewModel/Settings/CameraSettingsModel.h>

CameraSettingsView::CameraSettingsView(CameraSettingsModel* model, QWidget* parent)
    : SettingsBaseView(model, parent)
{
    m_statusLayout->parentWidget()->setVisible(false);
}
