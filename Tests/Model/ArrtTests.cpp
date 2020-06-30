#include <gtest/gtest.h>
#include <iostream>
#include <QApplication>

// Unit tests for the Arrt model. They will use mocked ARR and Azure Storage SDKs.
// Note: the access to the model data from the ARR SDK will have to be faked, since ARRT creates and holds SDK objects (Microsoft::Azure::RemoteRendering::Entity)

TEST(sample_model_test, to_succeed)
{
	// ApplicationModel am;
	// Add tests for models
	EXPECT_EQ(3, 3);
}
