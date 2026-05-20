#include <gtest/gtest.h>

#include "core/mediapipe_shapes.h"

TEST(MediaPipeShapes, TensorCounts)
{
	size_t inputCount = 0;
	size_t outputCount = 0;
	ASSERT_TRUE(mediapipeTensorSizesValid(inputCount, outputCount));
	EXPECT_EQ(inputCount, 1u * 144u * 256u * 3u);
	EXPECT_EQ(outputCount, 1u * 144u * 256u * 2u);
}

TEST(MediaPipeShapes, NetworkSize)
{
	uint32_t width = 0;
	uint32_t height = 0;
	mediapipeNetworkSize(width, height);
	EXPECT_EQ(width, 256u);
	EXPECT_EQ(height, 144u);
}
