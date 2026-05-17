#include "ort-session-utils.h"

#include <obs-module.h>
#include <util/platform.h>

#include <fstream>

#include "consts.h"
#include "plugin-macros.generated.h"

static bool modelFileLooksValid(const char *path)
{
	const int64_t size = os_get_file_size(path);
	if (size < 100 * 1024) {
		return false;
	}

	std::ifstream file(path, std::ios::binary);
	if (!file) {
		return false;
	}

	char magic[4] = {};
	file.read(magic, sizeof(magic));
	return file.gcount() == 4 && magic[0] == 0x08;
}

void createOrtSession(filter_data *tf)
{
	if (!tf->model || !tf->env) {
		blog(LOG_ERROR, "Model or ONNX environment is not initialized");
		return;
	}

	tf->session.reset();
	tf->inputNames.clear();
	tf->outputNames.clear();
	tf->inputTensor.clear();
	tf->outputTensor.clear();
	tf->inputTensorValues.clear();
	tf->outputTensorValues.clear();
	tf->inputDims.clear();
	tf->outputDims.clear();

	Ort::SessionOptions sessionOptions;
	sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
	sessionOptions.SetInterOpNumThreads((int)tf->numThreads);
	sessionOptions.SetIntraOpNumThreads((int)tf->numThreads);

	char *modelPath = obs_module_file(MODEL_MEDIAPIPE);
	if (!modelPath) {
		blog(LOG_ERROR, "Model file not found: %s", MODEL_MEDIAPIPE);
		return;
	}

	if (!modelFileLooksValid(modelPath)) {
		blog(LOG_ERROR, "Model file is missing or invalid: %s", modelPath);
		bfree(modelPath);
		return;
	}

	tf->modelPath = modelPath;
	bfree(modelPath);

	try {
		tf->session = std::make_unique<Ort::Session>(*tf->env, tf->modelPath.c_str(),
							     sessionOptions);
		tf->model->populateInputOutputNames(tf->session, tf->inputNames, tf->outputNames);

		if (!tf->model->populateInputOutputShapes(tf->session, tf->inputDims,
							tf->outputDims)) {
			blog(LOG_ERROR, "Invalid model input/output shapes");
			tf->session.reset();
			return;
		}

		tf->model->allocateTensorBuffers(tf->inputDims, tf->outputDims,
						 tf->outputTensorValues, tf->inputTensorValues,
						 tf->inputTensor, tf->outputTensor);
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "Failed to initialize ONNX session: %s", e.what());
		tf->session.reset();
		return;
	}

	blog(LOG_INFO, "ONNX session ready (MediaPipe)");
}

bool ensureOrtSession(filter_data *tf)
{
	if (tf->session) {
		return true;
	}
	if (tf->sessionInitFailed) {
		return false;
	}

	std::lock_guard<std::mutex> lock(tf->sessionLock);
	if (tf->session) {
		return true;
	}
	if (tf->sessionInitFailed) {
		return false;
	}

	createOrtSession(tf);
	if (!tf->session) {
		tf->sessionInitFailed = true;
		return false;
	}

	return true;
}

bool runFilterModelInference(filter_data *tf, const cv::Mat &imageBGRA, cv::Mat &output)
{
	if (!ensureOrtSession(tf) || !tf->model) {
		return false;
	}

	cv::Mat imageRGB;
	cv::cvtColor(imageBGRA, imageRGB, cv::COLOR_BGRA2RGB);

	uint32_t inputWidth = 0;
	uint32_t inputHeight = 0;
	tf->model->getNetworkInputSize(tf->inputDims, inputWidth, inputHeight);

	cv::Mat resizedImageRGB;
	cv::resize(imageRGB, resizedImageRGB, cv::Size((int)inputWidth, (int)inputHeight));

	cv::Mat resizedImage;
	cv::Mat preprocessedImage;
	resizedImageRGB.convertTo(resizedImage, CV_32F);
	tf->model->prepareInputToNetwork(resizedImage, preprocessedImage);
	tf->model->loadInputToTensor(preprocessedImage, inputWidth, inputHeight,
				     tf->inputTensorValues);

	tf->model->runNetworkInference(tf->session, tf->inputNames, tf->outputNames,
				       tf->inputTensor, tf->outputTensor);

	cv::Mat outputImage =
		tf->model->getNetworkOutput(tf->outputDims, tf->outputTensorValues);
	tf->model->postprocessOutput(outputImage);
	outputImage.convertTo(output, CV_8U, 255.0);

	return true;
}
