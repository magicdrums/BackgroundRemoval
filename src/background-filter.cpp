#include <obs-module.h>
#include <util/platform.h>

#include <opencv2/opencv.hpp>

#include "FilterData.h"
#include "consts.h"
#include "obs-utils/background-image.h"
#include "obs-utils/obs-utils.h"
#include "ort-utils/ort-session-utils.h"
#include "plugin-macros.generated.h"

enum BackgroundMode { BACKGROUND_TRANSPARENT = 0, BACKGROUND_IMAGE = 1 };

struct background_removal_filter : public filter_data {
	bool enableThreshold = true;
	float threshold = 0.5f;
	float contourFilter = 0.05f;
	float smoothContour = 0.5f;
	float feather = 0.0f;

	cv::Mat backgroundMask;
	int maskEveryXFrames = 1;
	int maskEveryXFramesCount = 0;

	int backgroundMode = BACKGROUND_TRANSPARENT;
	BackgroundImageState backgroundImage;

	gs_effect_t *effect = nullptr;
};

static const char *filter_getname(void *)
{
	return obs_module_text("BackgroundRemoval");
}

static bool enable_threshold_modified(obs_properties_t *ppts, obs_property_t *,
				      obs_data_t *settings)
{
	const bool enabled = obs_data_get_bool(settings, "enable_threshold");
	obs_property_set_visible(obs_properties_get(ppts, "threshold"), enabled);
	obs_property_set_visible(obs_properties_get(ppts, "contour_filter"), enabled);
	obs_property_set_visible(obs_properties_get(ppts, "smooth_contour"), enabled);
	obs_property_set_visible(obs_properties_get(ppts, "feather"), enabled);
	return true;
}

static bool background_mode_modified(obs_properties_t *ppts, obs_property_t *,
				     obs_data_t *settings)
{
	const int mode = (int)obs_data_get_int(settings, "background_mode");
	obs_property_t *imagePath = obs_properties_get(ppts, "background_image");
	obs_property_set_visible(imagePath, mode == BACKGROUND_IMAGE);
	return true;
}

static obs_properties_t *filter_properties(void *)
{
	obs_properties_t *props = obs_properties_create();

	obs_property_t *mode = obs_properties_add_list(props, "background_mode",
						       obs_module_text("BackgroundMode"),
						       OBS_COMBO_TYPE_LIST,
						       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(mode, obs_module_text("BackgroundTransparent"),
				BACKGROUND_TRANSPARENT);
	obs_property_list_add_int(mode, obs_module_text("BackgroundImage"), BACKGROUND_IMAGE);
	obs_property_set_modified_callback(mode, background_mode_modified);

	obs_property_t *imagePath = obs_properties_add_path(
		props, "background_image", obs_module_text("BackgroundImage"), OBS_PATH_FILE,
		obs_module_text("ImageFiles"), "*.png *.jpg *.jpeg *.bmp *.webp *.tga");
	obs_property_set_visible(imagePath, false);

	obs_property_t *p = obs_properties_add_bool(props, "enable_threshold",
						  obs_module_text("EnableThreshold"));
	obs_property_set_modified_callback(p, enable_threshold_modified);

	obs_properties_add_float_slider(props, "threshold", obs_module_text("Threshold"), 0.0,
					1.0, 0.025);
	obs_properties_add_float_slider(props, "contour_filter",
					obs_module_text("ContourFilterPercentOfImage"), 0.0, 1.0,
					0.025);
	obs_properties_add_float_slider(props, "smooth_contour",
					obs_module_text("SmoothSilhouette"), 0.0, 1.0, 0.05);
	obs_properties_add_float_slider(props, "feather", obs_module_text("FeatherBlendSilhouette"),
					0.0, 1.0, 0.05);
	obs_properties_add_int(props, "mask_every_x_frames",
			     obs_module_text("CalculateMaskEveryXFrame"), 1, 300, 1);
	obs_properties_add_int(props, "numThreads", obs_module_text("NumThreads"), 1, 8, 1);

	return props;
}

static void filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "background_mode", BACKGROUND_TRANSPARENT);
	obs_data_set_default_string(settings, "background_image", "");
	obs_data_set_default_bool(settings, "enable_threshold", true);
	obs_data_set_default_double(settings, "threshold", 0.5);
	obs_data_set_default_double(settings, "contour_filter", 0.05);
	obs_data_set_default_double(settings, "smooth_contour", 0.5);
	obs_data_set_default_double(settings, "feather", 0.0);
	obs_data_set_default_int(settings, "mask_every_x_frames", 1);
	obs_data_set_default_int(settings, "numThreads", 2);
}

static void filter_update_background_image(background_removal_filter *tf, obs_data_t *settings)
{
	const int mode = (int)obs_data_get_int(settings, "background_mode");
	const char *path = obs_data_get_string(settings, "background_image");

	tf->backgroundMode = mode;
	backgroundImageReleaseTexture(tf->backgroundImage);

	if (mode != BACKGROUND_IMAGE) {
		tf->backgroundImage.sourceBGRA.release();
		tf->backgroundImage.imagePath.clear();
		return;
	}

	if (tf->backgroundImage.imagePath == path && !tf->backgroundImage.sourceBGRA.empty()) {
		return;
	}

	backgroundImageLoadFromFile(tf->backgroundImage, path);
}

static void filter_update(void *data, obs_data_t *settings)
{
	auto *tf = reinterpret_cast<background_removal_filter *>(data);

	tf->enableThreshold = obs_data_get_bool(settings, "enable_threshold");
	tf->threshold = (float)obs_data_get_double(settings, "threshold");
	tf->contourFilter = (float)obs_data_get_double(settings, "contour_filter");
	tf->smoothContour = (float)obs_data_get_double(settings, "smooth_contour");
	tf->feather = (float)obs_data_get_double(settings, "feather");
	tf->maskEveryXFrames = (int)obs_data_get_int(settings, "mask_every_x_frames");
	tf->maskEveryXFramesCount = 0;

	filter_update_background_image(tf, settings);

	const uint32_t newNumThreads = (uint32_t)obs_data_get_int(settings, "numThreads");
	if (tf->configuredNumThreads != newNumThreads) {
		std::lock_guard<std::mutex> lock(tf->sessionLock);
		tf->configuredNumThreads = newNumThreads;
		tf->numThreads = newNumThreads;
		tf->sessionInitFailed = false;
		tf->session.reset();
		tf->inputTensor.clear();
		tf->outputTensor.clear();
		tf->inputTensorValues.clear();
		tf->outputTensorValues.clear();
		tf->inputDims.clear();
		tf->outputDims.clear();
	}

	if (!tf->model) {
		tf->model = std::make_unique<ModelMediaPipe>();
	}

	obs_enter_graphics();
	char *effect_path = obs_module_file(EFFECT_PATH);
	gs_effect_destroy(tf->effect);
	tf->effect = gs_effect_create_from_file(effect_path, nullptr);
	bfree(effect_path);
	obs_leave_graphics();
}

static void filter_activate(void *data)
{
	reinterpret_cast<background_removal_filter *>(data)->isDisabled = false;
}

static void filter_deactivate(void *data)
{
	reinterpret_cast<background_removal_filter *>(data)->isDisabled = true;
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	void *mem = bzalloc(sizeof(background_removal_filter));
	auto *tf = new (mem) background_removal_filter();

	tf->source = source;
	tf->texrender = gs_texrender_create(GS_BGRA, GS_ZS_NONE);
	tf->env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_ERROR,
					       "background-removal-inference");
	tf->model = std::make_unique<ModelMediaPipe>();

	filter_update(tf, settings);
	return tf;
}

static void filter_destroy(void *data)
{
	auto *tf = reinterpret_cast<background_removal_filter *>(data);
	if (!tf) {
		return;
	}

	backgroundImageReleaseTexture(tf->backgroundImage);

	obs_enter_graphics();
	gs_texrender_destroy(tf->texrender);
	if (tf->stagesurface) {
		gs_stagesurface_destroy(tf->stagesurface);
	}
	gs_effect_destroy(tf->effect);
	obs_leave_graphics();

	tf->~background_removal_filter();
	bfree(tf);
}

static void processImageForBackground(background_removal_filter *tf, const cv::Mat &imageBGRA,
				      cv::Mat &backgroundMask)
{
	cv::Mat outputImage;
	if (!runFilterModelInference(tf, imageBGRA, outputImage)) {
		return;
	}

	if (tf->enableThreshold) {
		const uint8_t thresholdValue = (uint8_t)(tf->threshold * 255.0f);
		backgroundMask = outputImage < thresholdValue;
	} else {
		backgroundMask = 255 - outputImage;
	}
}

static void filter_video_tick(void *data, float)
{
	auto *tf = reinterpret_cast<background_removal_filter *>(data);

	if (tf->isDisabled || !obs_source_enabled(tf->source) || tf->inputBGRA.empty()) {
		return;
	}

	if (!ensureOrtSession(tf)) {
		return;
	}

	cv::Mat imageBGRA;
	{
		std::unique_lock<std::mutex> lock(tf->inputBGRALock, std::try_to_lock);
		if (!lock.owns_lock()) {
			return;
		}
		imageBGRA = tf->inputBGRA.clone();
	}

	if (tf->backgroundMask.empty()) {
		tf->backgroundMask = cv::Mat(imageBGRA.size(), CV_8UC1, cv::Scalar(255));
	}

	tf->maskEveryXFramesCount++;
	tf->maskEveryXFramesCount %= tf->maskEveryXFrames;

	try {
		if (tf->maskEveryXFramesCount == 0 || tf->backgroundMask.empty()) {
			cv::Mat backgroundMask;
			processImageForBackground(tf, imageBGRA, backgroundMask);

			if (tf->enableThreshold) {
				if (tf->contourFilter > 0.0f && tf->contourFilter < 1.0f) {
					std::vector<std::vector<cv::Point>> contours;
					findContours(backgroundMask, contours, cv::RETR_EXTERNAL,
						     cv::CHAIN_APPROX_SIMPLE);
					std::vector<std::vector<cv::Point>> filteredContours;
					const int64_t contourSizeThreshold =
						(int64_t)(backgroundMask.total() *
							  tf->contourFilter);
					for (auto &contour : contours) {
						if (cv::contourArea(contour) >
						    contourSizeThreshold) {
							filteredContours.push_back(contour);
						}
					}
					backgroundMask.setTo(0);
					drawContours(backgroundMask, filteredContours, -1,
						     cv::Scalar(255), -1);
				}

				if (tf->smoothContour > 0.0f) {
					int kSize = (int)(3 + 11 * tf->smoothContour);
					kSize += kSize % 2 == 0 ? 1 : 0;
					cv::stackBlur(backgroundMask, backgroundMask,
						      cv::Size(kSize, kSize));
				}

				cv::resize(backgroundMask, backgroundMask, imageBGRA.size());

				if (tf->smoothContour > 0.0f) {
					backgroundMask = backgroundMask > 128;
				}

				if (tf->feather > 0.0f) {
					int kSize = (int)(40 * tf->feather);
					kSize += kSize % 2 == 0 ? 1 : 0;
					cv::dilate(backgroundMask, backgroundMask, cv::Mat(),
						   cv::Point(-1, -1), kSize / 3);
					cv::boxFilter(backgroundMask, backgroundMask,
						      tf->backgroundMask.depth(),
						      cv::Size(kSize, kSize));
				}
			}

			backgroundMask.copyTo(tf->backgroundMask);
		}
	} catch (const std::exception &e) {
		blog(LOG_ERROR, "%s", e.what());
	}
}

static void filter_video_render(void *data, gs_effect_t *)
{
	auto *tf = reinterpret_cast<background_removal_filter *>(data);

	uint32_t width = 0;
	uint32_t height = 0;
	if (!getRGBAFromStageSurface(tf, width, height)) {
		obs_source_skip_video_filter(tf->source);
		return;
	}

	if (!tf->effect) {
		obs_source_skip_video_filter(tf->source);
		return;
	}

	if (!obs_source_process_filter_begin(tf->source, GS_RGBA,
					     OBS_ALLOW_DIRECT_RENDERING)) {
		obs_source_skip_video_filter(tf->source);
		return;
	}

	if (tf->backgroundMask.empty()) {
		obs_source_skip_video_filter(tf->source);
		return;
	}

	gs_texture_t *alphaTexture = nullptr;
	{
		std::lock_guard<std::mutex> lock(tf->outputLock);
		if (!tf->backgroundMask.isContinuous()) {
			obs_source_skip_video_filter(tf->source);
			return;
		}
		alphaTexture =
			gs_texture_create(tf->backgroundMask.cols, tf->backgroundMask.rows, GS_R8,
					  1, (const uint8_t **)&tf->backgroundMask.data, 0);
		if (!alphaTexture) {
			obs_source_skip_video_filter(tf->source);
			return;
		}
	}

	const char *technique = "DrawWithoutBlur";
	if (tf->backgroundMode == BACKGROUND_IMAGE &&
	    backgroundImageEnsureTexture(tf->backgroundImage, width, height)) {
		gs_eparam_t *backgroundParam =
			gs_effect_get_param_by_name(tf->effect, "backgroundImage");
		gs_effect_set_texture(backgroundParam, tf->backgroundImage.texture);
		technique = "DrawWithBackground";
	}

	gs_eparam_t *alphamask = gs_effect_get_param_by_name(tf->effect, "alphamask");
	gs_effect_set_texture(alphamask, alphaTexture);

	gs_blend_state_push();
	gs_reset_blend_state();
	obs_source_process_filter_tech_end(tf->source, tf->effect, 0, 0, technique);
	gs_blend_state_pop();

	gs_texture_destroy(alphaTexture);
}

struct obs_source_info background_removal_filter_info = {
	.id = "background_removal_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = filter_getname,
	.create = filter_create,
	.destroy = filter_destroy,
	.get_defaults = filter_defaults,
	.get_properties = filter_properties,
	.update = filter_update,
	.activate = filter_activate,
	.deactivate = filter_deactivate,
	.video_tick = filter_video_tick,
	.video_render = filter_video_render,
};
