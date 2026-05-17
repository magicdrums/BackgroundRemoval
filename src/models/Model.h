#ifndef MODEL_H
#define MODEL_H

#include <obs-module.h>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <vector>

inline bool tensorElementCount(const std::vector<int64_t> &dims, size_t &outCount)
{
	constexpr size_t kMaxElements = 64 * 1024 * 1024;
	size_t count = 1;

	for (int64_t dim : dims) {
		if (dim <= 0) {
			return false;
		}
		if (static_cast<uint64_t>(dim) > kMaxElements / count) {
			return false;
		}
		count *= static_cast<size_t>(dim);
	}

	outCount = count;
	return true;
}

class Model {
public:
	virtual ~Model() = default;

	virtual void populateInputOutputNames(const std::unique_ptr<Ort::Session> &session,
					     std::vector<Ort::AllocatedStringPtr> &inputNames,
					     std::vector<Ort::AllocatedStringPtr> &outputNames)
	{
		Ort::AllocatorWithDefaultOptions allocator;
		inputNames.clear();
		outputNames.clear();
		inputNames.push_back(session->GetInputNameAllocated(0, allocator));
		outputNames.push_back(session->GetOutputNameAllocated(0, allocator));
	}

	virtual bool populateInputOutputShapes(const std::unique_ptr<Ort::Session> &session,
					       std::vector<std::vector<int64_t>> &inputDims,
					       std::vector<std::vector<int64_t>> &outputDims)
	{
		inputDims.clear();
		outputDims.clear();
		inputDims.push_back({});
		outputDims.push_back({});

		const auto outputInfo = session->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo();
		outputDims[0] = outputInfo.GetShape();

		const auto inputInfo = session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
		inputDims[0] = inputInfo.GetShape();

		for (auto &i : inputDims[0]) {
			if (i == -1) {
				i = 1;
			}
		}
		for (auto &i : outputDims[0]) {
			if (i == -1) {
				i = 1;
			}
		}

		return inputDims[0].size() >= 3 && outputDims[0].size() >= 3;
	}

	virtual void allocateTensorBuffers(const std::vector<std::vector<int64_t>> &inputDims,
					   const std::vector<std::vector<int64_t>> &outputDims,
					   std::vector<std::vector<float>> &outputTensorValues,
					   std::vector<std::vector<float>> &inputTensorValues,
					   std::vector<Ort::Value> &inputTensor,
					   std::vector<Ort::Value> &outputTensor)
	{
		outputTensorValues.clear();
		outputTensor.clear();
		inputTensorValues.clear();
		inputTensor.clear();

		Ort::MemoryInfo memoryInfo =
			Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtDeviceAllocator,
						   OrtMemType::OrtMemTypeDefault);

		for (size_t i = 0; i < inputDims.size(); i++) {
			size_t elementCount = 0;
			if (!tensorElementCount(inputDims[i], elementCount)) {
				throw std::runtime_error("Invalid input tensor shape");
			}
			inputTensorValues.emplace_back(elementCount, 0.0f);
			inputTensor.push_back(Ort::Value::CreateTensor<float>(
				memoryInfo, inputTensorValues[i].data(),
				inputTensorValues[i].size(), inputDims[i].data(),
				inputDims[i].size()));
		}

		for (size_t i = 0; i < outputDims.size(); i++) {
			size_t elementCount = 0;
			if (!tensorElementCount(outputDims[i], elementCount)) {
				throw std::runtime_error("Invalid output tensor shape");
			}
			outputTensorValues.emplace_back(elementCount, 0.0f);
			outputTensor.push_back(Ort::Value::CreateTensor<float>(
				memoryInfo, outputTensorValues[i].data(),
				outputTensorValues[i].size(), outputDims[i].data(),
				outputDims[i].size()));
		}
	}

	virtual void getNetworkInputSize(const std::vector<std::vector<int64_t>> &inputDims,
					 uint32_t &inputWidth, uint32_t &inputHeight)
	{
		inputWidth = (uint32_t)inputDims[0][2];
		inputHeight = (uint32_t)inputDims[0][1];
	}

	virtual void prepareInputToNetwork(cv::Mat &resizedImage, cv::Mat &preprocessedImage)
	{
		preprocessedImage = resizedImage / 255.0;
	}

	virtual void postprocessOutput(cv::Mat &output) { UNUSED_PARAMETER(output); }

	virtual void loadInputToTensor(const cv::Mat &preprocessedImage, uint32_t inputWidth,
				       uint32_t inputHeight,
				       std::vector<std::vector<float>> &inputTensorValues)
	{
		preprocessedImage.copyTo(
			cv::Mat((int)inputHeight, (int)inputWidth, CV_32FC3,
				&(inputTensorValues[0][0])));
	}

	virtual cv::Mat getNetworkOutput(const std::vector<std::vector<int64_t>> &outputDims,
					 std::vector<std::vector<float>> &outputTensorValues)
	{
		uint32_t outputWidth = (uint32_t)outputDims[0].at(2);
		uint32_t outputHeight = (uint32_t)outputDims[0].at(1);
		int32_t outputChannels = CV_MAKE_TYPE(CV_32F, (int)outputDims[0].at(3));

		return cv::Mat((int)outputHeight, (int)outputWidth, outputChannels,
			       outputTensorValues[0].data());
	}

	virtual void runNetworkInference(const std::unique_ptr<Ort::Session> &session,
					 const std::vector<Ort::AllocatedStringPtr> &inputNames,
					 const std::vector<Ort::AllocatedStringPtr> &outputNames,
					 const std::vector<Ort::Value> &inputTensor,
					 std::vector<Ort::Value> &outputTensor)
	{
		if (inputNames.empty() || outputNames.empty()) {
			return;
		}

		std::vector<const char *> rawInputNames;
		for (auto &name : inputNames) {
			rawInputNames.push_back(name.get());
		}

		std::vector<const char *> rawOutputNames;
		for (auto &name : outputNames) {
			rawOutputNames.push_back(name.get());
		}

		session->Run(Ort::RunOptions{nullptr}, rawInputNames.data(),
			     inputTensor.data(), inputNames.size(), rawOutputNames.data(),
			     outputTensor.data(), outputNames.size());
	}
};

#endif
