#include <gtest/gtest.h>
#include <iostream>

#include <QDir>
#include <QStandardPaths>
#include <dxgi.h>
#include <d3d11.h>

// dummy test. To test the framework
TEST(sample_test_case_standalone, sample_test_to_fail)
{
    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    std::cout << appDataRootDir.path().toStdString() << std::endl;

		
    UINT iCreateFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    iCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0};

    ID3D11Device* m_device = nullptr;               // can move
    ID3D11DeviceContext* m_deviceContext = nullptr; // can move


    D3D11CreateDevice(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        iCreateFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_deviceContext);
	
    EXPECT_EQ(2, 3);
}

TEST(sample_test_case_standalone, sample_test_to_succeed)
{
	EXPECT_EQ(4, 4);
}
