#include <obs-module.h>

#include "plugin-macros.generated.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

extern struct obs_source_info background_removal_filter_info;

MODULE_EXPORT const char *obs_module_description(void)
{
	return obs_module_text("PortraitBackgroundFilterPlugin");
}

bool obs_module_load(void)
{
	obs_register_source(&background_removal_filter_info);
	blog(LOG_INFO, "loaded (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	blog(LOG_INFO, "unloaded");
}
