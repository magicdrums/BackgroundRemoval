#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "core/onnx_model_validate.h"

namespace {

std::string tempPath(const char *filename)
{
	return (std::filesystem::temp_directory_path() / filename).string();
}

} // namespace

TEST(OnnxModelValidate, RejectsMissingFile)
{
	EXPECT_FALSE(onnxModelFileLooksValid("/nonexistent/model.onnx"));
}

TEST(OnnxModelValidate, RejectsSmallFile)
{
	const std::string path = tempPath("obs-bg-tiny.onnx");
	{
		std::ofstream out(path, std::ios::binary);
		out << "x";
	}
	EXPECT_FALSE(onnxModelFileLooksValid(path));
}

TEST(OnnxModelValidate, RejectsInvalidMagic)
{
	const std::string path = tempPath("obs-bg-bad-magic.onnx");
	{
		std::ofstream out(path, std::ios::binary);
		std::string blob(200 * 1024, '\0');
		out.write(blob.data(), static_cast<std::streamsize>(blob.size()));
	}
	EXPECT_FALSE(onnxModelFileLooksValid(path));
}

TEST(OnnxModelValidate, AcceptsDownloadedModel)
{
	const char *env = std::getenv("OBS_BGREMOVAL_MODEL_PATH");
	if (!env || !*env) {
		GTEST_SKIP() << "OBS_BGREMOVAL_MODEL_PATH not set";
	}
	EXPECT_TRUE(onnxModelFileLooksValid(env));
}
