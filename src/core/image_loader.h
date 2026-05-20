#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <opencv2/core.hpp>
#include <string>

bool loadImageBGRAFromFile(const std::string &path, cv::Mat &outBGRA);

#endif
