#include "background-image.h"

#include <obs-module.h>
#include <opencv2/imgproc.hpp>

#include "core/image_loader.h"
#include "plugin-macros.generated.h"

bool backgroundImageLoadFromFile(BackgroundImageState &state, const char *path)
{
	if (!path || !*path) {
		state.sourceBGRA.release();
		state.imagePath.clear();
		return false;
	}

	cv::Mat bgra;
	if (!loadImageBGRAFromFile(path, bgra)) {
		blog(LOG_WARNING, "Could not load background image: %s", path);
		state.sourceBGRA.release();
		state.imagePath.clear();
		return false;
	}

	state.sourceBGRA = bgra;
	state.imagePath = path;
	blog(LOG_INFO, "Background image loaded: %s (%dx%d)", path, bgra.cols, bgra.rows);
	return true;
}

void backgroundImageReleaseTexture(BackgroundImageState &state)
{
	if (state.texture) {
		obs_enter_graphics();
		gs_texture_destroy(state.texture);
		obs_leave_graphics();
		state.texture = nullptr;
	}
	state.textureWidth = 0;
	state.textureHeight = 0;
}

bool backgroundImageEnsureTexture(BackgroundImageState &state, uint32_t width, uint32_t height)
{
	if (state.sourceBGRA.empty() || width == 0 || height == 0) {
		return false;
	}

	if (state.texture && state.textureWidth == width && state.textureHeight == height) {
		return true;
	}

	cv::Mat resized;
	cv::resize(state.sourceBGRA, resized, cv::Size((int)width, (int)height), 0, 0,
		   cv::INTER_LINEAR);

	if (!resized.isContinuous()) {
		resized = resized.clone();
	}

	obs_enter_graphics();
	if (state.texture) {
		gs_texture_destroy(state.texture);
		state.texture = nullptr;
	}

	state.texture = gs_texture_create(width, height, GS_BGRA, 1,
					  (const uint8_t **)&resized.data, GS_DYNAMIC);
	obs_leave_graphics();

	if (!state.texture) {
		blog(LOG_ERROR, "Failed to create background GPU texture");
		return false;
	}

	state.textureWidth = width;
	state.textureHeight = height;
	return true;
}
