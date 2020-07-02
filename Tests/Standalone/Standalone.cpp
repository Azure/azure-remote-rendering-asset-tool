#include <QDir>
#include <gtest/gtest.h>

// tests to be done in isolation. For classes that don't require any dependency (except Qt)

// dummy test. To test the framework
TEST(sample_test_case_standalone, sample_test_to_succeed)
{
    // added dummy Qt class to avoid error when deploying. We treat the project as a Qt project, so it expects at least a Qt class
    QDir d;
    EXPECT_EQ(3, 3);
}
