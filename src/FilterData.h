#ifndef FILTERDATA_H
#define FILTERDATA_H

#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>

#include "models/ModelMediapipe.h"
#include "ort-utils/ORTModelData.h"

struct filter_data : public ORTModelData {
	uint32_t numThreads = 1;
	std::unique_ptr<ModelMediaPipe> model;

	obs_source_t *source = nullptr;
	gs_texrender_t *texrender = nullptr;
	gs_stagesurf_t *stagesurface = nullptr;

	cv::Mat inputBGRA;
	bool isDisabled = false;

	std::mutex inputBGRALock;
	std::mutex outputLock;
	std::mutex sessionLock;

	bool sessionInitFailed = false;
	uint32_t configuredNumThreads = 1;

	std::string modelPath;
};

#endif
