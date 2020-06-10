/*
obs-auto-subtitle
 Copyright (C) 2016-2018 Yibai Zhang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; If not, see <https://www.gnu.org/licenses/>
*/

#include <string>
#include <string.h>
#include <functional>
#include <QObject>
#include <mutex>
#include <obs-module.h>
#include <obs.h>
#include <util/platform.h>
#include <util/threading.h>
#include <chrono>
#include <thread>

#include "obs-auto-subtitle.h"

#define PROP_SOURCE_IP "autosub_source_ip"
#define PROP_CUSTOM_SOURCE_IP "ssp_custom_source_ip"
#define PROP_CHECK_IP "ssp_check_ip"

#define PROP_CUSTOM_VALUE "\x01\x02custom"

#define PROP_HW_ACCEL "ssp_recv_hw_accel"
#define PROP_SYNC "ssp_sync"
#define PROP_LATENCY "latency"
#define PROP_VIDEO_RANGE "video_range"
#define PROP_EXP_WAIT_I "exp_wait_i_frame"

#define PROP_BW_HIGHEST 0
#define PROP_BW_LOWEST 1
#define PROP_BW_AUDIO_ONLY 2

#define PROP_SYNC_INTERNAL 0
#define PROP_SYNC_SSP_TIMESTAMP 1

#define PROP_LATENCY_NORMAL 0
#define PROP_LATENCY_LOW 1

#define PROP_LED_TALLY "led_as_tally_light"
#define PROP_RESOLUTION "ssp_resolution"
#define PROP_FRAME_RATE "ssp_frame_rate"
#define PROP_BITRATE "ssp_bitrate"
#define PROP_STREAM_INDEX "ssp_stream_index"
#define PROP_ENCODER "ssp_encoding"

#define SSP_IP_DIRECT "10.98.32.1"
#define SSP_IP_WIFI "10.98.33.1"
#define SSP_IP_USB "172.18.18.1"

#include <QThread>

using namespace std::placeholders;

struct autosub_source
{
	obs_source_t* source;


    int sync_mode;
    int video_range;
    int hwaccel;
    int wait_i_frame;
    int i_frame_shown;
    int tally;

	bool running;
	const char *source_ip;

	uint32_t width;
	uint32_t height;
    obs_source_frame2 frame;
    
    uint32_t sample_size;
    obs_source_audio audio;

    bool do_check;
    bool ip_checked;
};

static obs_source_frame* blank_video_frame()
{
	obs_source_frame* frame = obs_source_frame_create(VIDEO_FORMAT_NONE, 0, 0);
	frame->timestamp = os_gettime_ns();
	return frame;
}

const char* autosub_source_getname(void* data)
{
	UNUSED_PARAMETER(data);
	return obs_module_text("AutoSub.SourceProps.SourceName");
}

obs_properties_t* autosub_source_getproperties(void* data)
{
    char nametext[256];
	auto s = (struct autosub_source*)data;

	obs_properties_t* props = obs_properties_create();
	obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);

	obs_property_t* source_ip = obs_properties_add_list(props, PROP_SOURCE_IP,
		obs_module_text("AutoSub.SourceProps.SourceIp"),
        OBS_COMBO_TYPE_LIST,
        OBS_COMBO_FORMAT_STRING);

    snprintf(nametext, 256, "%s (%s)", obs_module_text("AutoSub.IP.Fixed"), SSP_IP_DIRECT);
    obs_property_list_add_string(source_ip, nametext, SSP_IP_DIRECT);

    snprintf(nametext, 256, "%s (%s)", obs_module_text("AutoSub.IP.Wifi"), SSP_IP_WIFI);
    obs_property_list_add_string(source_ip, nametext, SSP_IP_WIFI);

    snprintf(nametext, 256, "%s (%s)", obs_module_text("AutoSub.IP.USB"), SSP_IP_USB);
    obs_property_list_add_string(source_ip, nametext, SSP_IP_USB);

    int count = 0;
    

	if(count == 0)
	    obs_property_list_add_string(source_ip, obs_module_text("AutoSub.SourceProps.NotFound"), "");
    obs_property_list_add_string(source_ip, obs_module_text("AutoSub.SourceProps.Custom"), PROP_CUSTOM_VALUE);


    obs_property_t* custom_source_ip = obs_properties_add_text(props, PROP_CUSTOM_SOURCE_IP,
        obs_module_text("AutoSub.SourceProps.SourceIp"),
        OBS_TEXT_DEFAULT);

    obs_property_set_visible(custom_source_ip, false);

	obs_property_t* sync_modes = obs_properties_add_list(props, PROP_SYNC,
		obs_module_text("AutoSub.SourceProps.Sync"),
		OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(sync_modes,
		obs_module_text("AutoSub.SyncMode.Internal"),
		PROP_SYNC_INTERNAL);
	obs_property_list_add_int(sync_modes,
		obs_module_text("AutoSub.SyncMode.SSPTimestamp"),
		PROP_SYNC_SSP_TIMESTAMP);


	obs_properties_add_bool(props, PROP_HW_ACCEL,
		obs_module_text("AutoSub.SourceProps.HWAccel"));

	obs_property_t* latency_modes = obs_properties_add_list(props, PROP_LATENCY,
		obs_module_text("AutoSub.SourceProps.Latency"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	obs_property_list_add_int(latency_modes,
		obs_module_text("AutoSub.SourceProps.Latency.Normal"),
		PROP_LATENCY_NORMAL);
	obs_property_list_add_int(latency_modes,
		obs_module_text("AutoSub.SourceProps.Latency.Low"),
		PROP_LATENCY_LOW);

    obs_property_t* encoders = obs_properties_add_list(props, PROP_ENCODER,
                                                       obs_module_text("AutoSub.SourceProps.Encoder"),
                                                       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(encoders, "H264", "H264");
    obs_property_list_add_string(encoders, "H265", "H265");

    obs_properties_add_bool(props, PROP_EXP_WAIT_I,
                            obs_module_text("AutoSub.SourceProps.WaitIFrame"));

    obs_property_t* resolutions = obs_properties_add_list(props, PROP_RESOLUTION,
                            obs_module_text("AutoSub.SourceProps.Resolution"),
                            OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(resolutions, "4K-UHD", "3840*2160");
    obs_property_list_add_string(resolutions, "4K-DCI", "4096*2160");
    obs_property_list_add_string(resolutions, "1080p", "1920*1080");
    

    obs_properties_add_list(props, PROP_FRAME_RATE,
                            obs_module_text("AutoSub.SourceProps.FrameRate"),
                            OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    obs_properties_add_int(props, PROP_BITRATE,
                            obs_module_text("AutoSub.SourceProps.Bitrate"),
                            10, 300, 5);

    obs_properties_add_bool(props, PROP_LED_TALLY,
                            obs_module_text("AutoSub.SourceProps.LedAsTally"));

	return props;
}

void autosub_source_getdefaults(obs_data_t* settings)
{
	obs_data_set_default_int(settings, PROP_SYNC, PROP_SYNC_SSP_TIMESTAMP);
	obs_data_set_default_int(settings, PROP_LATENCY, PROP_LATENCY_NORMAL);
	obs_data_set_default_string(settings, PROP_SOURCE_IP, "");
    obs_data_set_default_string(settings, PROP_CUSTOM_SOURCE_IP, "");
    obs_data_set_default_int(settings, PROP_BITRATE, 20);
    obs_data_set_default_bool(settings, PROP_HW_ACCEL, false);
    obs_data_set_default_bool(settings, PROP_EXP_WAIT_I, false);
    obs_data_set_default_bool(settings, PROP_LED_TALLY, false);
    obs_data_set_default_string(settings, PROP_ENCODER, "H264");
    obs_data_set_default_string(settings, PROP_FRAME_RATE, "29.97");

}

void autosub_source_update(void* data, obs_data_t* settings)
{
	
}

void autosub_source_shown(void* data)
{
    
}

void autosub_source_hidden(void* data)
{
    
}

void autosub_source_activated(void* data)
{
    blog(LOG_INFO, "source activated.\n");
}

void autosub_source_deactivated(void* data)
{
    blog(LOG_INFO, "source deactivated.\n");
}

void* autosub_source_create(obs_data_t* settings, obs_source_t* source)
{
	auto s = (struct autosub_source*)bzalloc(sizeof(struct autosub_source));
	s->source = source;
	s->running = false;
	s->do_check = false;
	s->ip_checked = false;
    autosub_source_update(s, settings);
	return s;
}


void autosub_source_destroy(void* data)
{
}

struct obs_source_info create_autosub_source_info()
{
	struct obs_source_info autosub_source_info = {};
    autosub_source_info.id				= "autosub_source";
    autosub_source_info.type			= OBS_SOURCE_TYPE_INPUT;
    autosub_source_info.output_flags	= OBS_SOURCE_ASYNC_VIDEO;
    autosub_source_info.get_name		= autosub_source_getname;
    autosub_source_info.get_properties	= autosub_source_getproperties;
    autosub_source_info.get_defaults	= autosub_source_getdefaults;
    autosub_source_info.update			= autosub_source_update;
    autosub_source_info.show			= autosub_source_shown;
    autosub_source_info.hide			= autosub_source_hidden;
    autosub_source_info.activate		= autosub_source_activated;
    autosub_source_info.deactivate		= autosub_source_deactivated;
    autosub_source_info.create			= autosub_source_create;
    autosub_source_info.destroy			= autosub_source_destroy;

	return autosub_source_info;
}
