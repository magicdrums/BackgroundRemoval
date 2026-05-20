#include <gtest/gtest.h>
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgproc.hpp>

#include "core/mediapipe_shapes.h"

TEST(OnnxInference, MediaPipeSessionRuns)
{
	const char *envPath = std::getenv("OBS_BGREMOVAL_MODEL_PATH");
	if (!envPath || !*envPath) {
		GTEST_SKIP() << "OBS_BGREMOVAL_MODEL_PATH not set";
	}

	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "obs-background-removal-test");
	Ort::SessionOptions options;
	options.SetIntraOpNumThreads(1);
	options.SetInterOpNumThreads(1);

	Ort::Session session(env, envPath, options);

	const auto inputShape = mediapipeInputShape();
	const size_t inputElements = 1u * 144u * 256u * 3u;

	std::vector<float> inputData(inputElements, 0.5f);
	Ort::MemoryInfo memoryInfo =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtDeviceAllocator,
					   OrtMemType::OrtMemTypeDefault);

	std::vector<Ort::Value> inputTensor;
	inputTensor.push_back(Ort::Value::CreateTensor<float>(
		memoryInfo, inputData.data(), inputData.size(), inputShape.data(),
		inputShape.size()));

	auto inputName = session.GetInputNameAllocated(0, Ort::AllocatorWithDefaultOptions());
	auto outputName = session.GetOutputNameAllocated(0, Ort::AllocatorWithDefaultOptions());

	const char *inputNames[] = {inputName.get()};
	const char *outputNames[] = {outputName.get()};

	auto outputs = session.Run(Ort::RunOptions{nullptr}, inputNames, inputTensor.data(), 1,
				   outputNames, 1);

	ASSERT_EQ(outputs.size(), 1u);
	auto shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
	ASSERT_GE(shape.size(), 3u);
	EXPECT_GT(shape[1], 0);
	EXPECT_GT(shape[2], 0);
}
