#ifndef MODELMEDIAPIPE_H
#define MODELMEDIAPIPE_H

#include "Model.h"

class ModelMediaPipe : public Model {
public:
	bool populateInputOutputShapes(const std::unique_ptr<Ort::Session> &session,
				       std::vector<std::vector<int64_t>> &inputDims,
				       std::vector<std::vector<int64_t>> &outputDims) override
	{
		UNUSED_PARAMETER(session);
		inputDims = {{1, 144, 256, 3}};
		outputDims = {{1, 144, 256, 2}};
		return true;
	}

	cv::Mat getNetworkOutput(const std::vector<std::vector<int64_t>> &outputDims,
				 std::vector<std::vector<float>> &outputTensorValues) override
	{
		uint32_t outputWidth = (uint32_t)outputDims[0].at(2);
		uint32_t outputHeight = (uint32_t)outputDims[0].at(1);
		int32_t outputChannels = CV_32FC2;

		return cv::Mat((int)outputHeight, (int)outputWidth, outputChannels,
			       outputTensorValues[0].data());
	}

	void postprocessOutput(cv::Mat &outputImage) override
	{
		std::vector<cv::Mat> channels;
		cv::split(outputImage, channels);
		outputImage = channels[1];
	}
};

#endif
