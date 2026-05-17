#ifndef ORT_SESSION_UTILS_H
#define ORT_SESSION_UTILS_H

#include "FilterData.h"

void createOrtSession(filter_data *tf);
bool ensureOrtSession(filter_data *tf);
bool runFilterModelInference(filter_data *tf, const cv::Mat &imageBGRA, cv::Mat &output);

#endif
