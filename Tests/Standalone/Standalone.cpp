#include <gtest/gtest.h>
#include <iostream>

#include <QDir>
#include <QStandardPaths>

// dummy test. To test the framework
TEST(sample_test_case_standalone, sample_test_to_fail)
{
    QDir appDataRootDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    std::cout << appDataRootDir.path().toStdString() << std::endl;
	
    EXPECT_EQ(2, 3);
}

TEST(sample_test_case_standalone, sample_test_to_succeed)
{
	EXPECT_EQ(4, 4);
}
