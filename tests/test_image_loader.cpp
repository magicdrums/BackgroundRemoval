#include <filesystem>
#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>

#include "core/image_loader.h"

namespace {

std::string tempPath(const char *filename)
{
	return (std::filesystem::temp_directory_path() / filename).string();
}

} // namespace

class ImageLoaderTest : public ::testing::Test {
protected:
	void SetUp() override
	{
		bgrPath = tempPath("obs-bg-fixture.bgr.png");
		grayPath = tempPath("obs-bg-fixture.gray.png");

		cv::Mat bgr(32, 48, CV_8UC3, cv::Scalar(10, 20, 30));
		cv::Mat gray(32, 48, CV_8UC1, cv::Scalar(128));

		ASSERT_TRUE(cv::imwrite(bgrPath, bgr));
		ASSERT_TRUE(cv::imwrite(grayPath, gray));
	}

	std::string bgrPath;
	std::string grayPath;
};

TEST_F(ImageLoaderTest, LoadsBGRImage)
{
	cv::Mat bgra;
	ASSERT_TRUE(loadImageBGRAFromFile(bgrPath, bgra));
	EXPECT_EQ(bgra.cols, 48);
	EXPECT_EQ(bgra.rows, 32);
	EXPECT_EQ(bgra.channels(), 4);
}

TEST_F(ImageLoaderTest, LoadsGrayscaleImage)
{
	cv::Mat bgra;
	ASSERT_TRUE(loadImageBGRAFromFile(grayPath, bgra));
	EXPECT_EQ(bgra.channels(), 4);
}

TEST_F(ImageLoaderTest, RejectsMissingFile)
{
	cv::Mat bgra;
	EXPECT_FALSE(loadImageBGRAFromFile("/nonexistent/image.png", bgra));
}
