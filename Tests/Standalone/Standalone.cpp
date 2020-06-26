#include <gtest/gtest.h>
#include <iostream>

// dummy test. To test the framework
TEST(sample_test_case_standalone, sample_test_to_fail)
{
    EXPECT_EQ(2, 3);
}

TEST(sample_test_case_standalone, sample_test_to_succeed)
{
	EXPECT_EQ(4, 4);
}
