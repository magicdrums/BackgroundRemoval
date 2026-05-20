#ifndef BACKGROUND_IMAGE_H
#define BACKGROUND_IMAGE_H

#include <graphics/graphics.h>
#include <opencv2/core.hpp>
#include <string>

struct BackgroundImageState {
	std::string imagePath;
	cv::Mat sourceBGRA;
	gs_texture_t *texture = nullptr;
	uint32_t textureWidth = 0;
	uint32_t textureHeight = 0;
};

bool backgroundImageLoadFromFile(BackgroundImageState &state, const char *path);
void backgroundImageReleaseTexture(BackgroundImageState &state);
bool backgroundImageEnsureTexture(BackgroundImageState &state, uint32_t width, uint32_t height);

#endif
