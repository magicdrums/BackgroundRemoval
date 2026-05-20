#include "image_loader.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

bool loadImageBGRAFromFile(const std::string &path, cv::Mat &outBGRA)
{
	if (path.empty()) {
		return false;
	}

	cv::Mat loaded = cv::imread(path, cv::IMREAD_UNCHANGED);
	if (loaded.empty()) {
		return false;
	}

	switch (loaded.channels()) {
	case 1:
		cv::cvtColor(loaded, outBGRA, cv::COLOR_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(loaded, outBGRA, cv::COLOR_BGR2BGRA);
		break;
	case 4:
		outBGRA = loaded;
		break;
	default:
		return false;
	}

	return !outBGRA.empty();
}
