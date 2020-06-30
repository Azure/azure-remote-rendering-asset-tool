#include <gtest/gtest.h>

// tests to be done in isolation. For classes that don't require any dependency (except Qt)

// dummy test. To test the framework
TEST(sample_test_case_standalone, sample_test_to_succeed)
{
	EXPECT_EQ(3, 3);
}
