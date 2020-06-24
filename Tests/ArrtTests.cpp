#include <gtest/gtest.h>
#include <iostream>

#include <ViewModel/ApplicationModel.h>
#include <ViewModel/Parameters/ParameterModel.h>
#include <ViewModel/Settings/CameraSettingsModel.h>
#include <ViewModel/Settings/SettingsModel.h>

// dummy test. To test the framework
TEST(sample_test_case, sample_test)
{
    ApplicationModel am;
    const QList<ParameterModel*>& controls = am.getSettingsModel()->getCameraSettingsModel()->getControls();

    EXPECT_EQ(controls.size(), 4);
}