#pragma once

#include <View/Settings/SettingsBaseView.h>

class VideoSettingsModel;
class FlatButton;

// View for ARR video settings configuration.

class VideoSettingsView : public SettingsBaseView
{
public:
    VideoSettingsView(VideoSettingsModel* model, QWidget* parent = nullptr);

private:
    VideoSettingsModel* const m_model;

    FlatButton* m_applyButton = {};
    FlatButton* m_resetButton = {};

    void updateUi();
};
