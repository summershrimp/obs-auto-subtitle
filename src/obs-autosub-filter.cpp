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

#define APPID ""
#define APIKEY ""

#define T_FILTER_NAME obs_module_text("AutoSub.FilterName")

#define PROP_PROVIDER "autosub_filter_sp"
#define T_PROVIDER obs_module_text("AutoSub.ServiceProvider")

#define PROP_APPID "autosub_filter_appid"
#define T_APPID obs_module_text("AutoSub.APPID")

#define PROP_APIKEY "autosub_filter_apikey"
#define T_APIKEY obs_module_text("AutoSub.APIKEY")

#include <QThread>
#include "xfyun/RTASR.h"
#include <media-io/audio-resampler.h>

using namespace std::placeholders;
enum ServiceProvider {
    SP_Default = 0,
    SP_Xfyun,
    SP_Hwcloud,
    SP_Sogou,
    SP_Aliyun
};
#define T_SP_XFYUN obs_module_text("AutoSub.SP.Xfyun")
#define T_SP_HWCLOUD obs_module_text("AutoSub.SP.Hwcloud")
#define T_SP_SOGOU obs_module_text("AutoSub.SP.Sogou")
#define T_SP_ALIYUN obs_module_text("AutoSub.SP.Aliyun")


struct autosub_filter
{
    obs_source_t* source;
    uint32_t sample_rate;
    uint32_t channels;

    RTASR *asr;
    audio_resampler_t *resampler;
};

const char* autosub_filter_getname(void* data)
{
    UNUSED_PARAMETER(data);
    return T_FILTER_NAME;
}

obs_properties_t* autosub_filter_getproperties(void* data)
{
    char nametext[256];
    auto s = (struct autosub_filter*)data;

    obs_properties_t* props = obs_properties_create();
    obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);
    auto providers = obs_properties_add_list(props, PROP_PROVIDER, T_PROVIDER, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(providers, T_SP_XFYUN, SP_Xfyun);
    obs_property_list_add_int(providers, T_SP_SOGOU, SP_Sogou);
    obs_property_list_add_int(providers, T_SP_HWCLOUD, SP_Hwcloud);
    obs_property_list_add_int(providers, T_SP_ALIYUN, SP_Aliyun);

    obs_properties_add_text(props, PROP_APPID, T_APPID, OBS_TEXT_DEFAULT);
    obs_properties_add_text(props, PROP_APIKEY, T_APIKEY, OBS_TEXT_DEFAULT);


    return props;
}

void autosub_filter_getdefaults(obs_data_t* settings)
{
    obs_data_set_default_int(settings, PROP_PROVIDER, SP_Xfyun);

}

resample_info resample_output = {
        .format = AUDIO_FORMAT_16BIT,
        .samples_per_sec = 16000,
        .speakers = SPEAKERS_MONO,
};

void autosub_filter_update(void* data, obs_data_t* settings)
{
    autosub_filter *s = (autosub_filter*)data;
    audio_output *global_audio = obs_get_audio();
    const uint32_t sample_rate =
            audio_output_get_sample_rate(global_audio);
    const size_t num_channels = audio_output_get_channels(global_audio);
    s->sample_rate = sample_rate;
    s->channels = num_channels;
    resample_info resample_input = {
            .format = AUDIO_FORMAT_FLOAT,
            .samples_per_sec = sample_rate,
            .speakers = SPEAKERS_MONO
    };
    if(s->resampler != nullptr) {
        audio_resampler_destroy(s->resampler);
        s->resampler = nullptr;
    }
    s->resampler = audio_resampler_create(&resample_output, &resample_input);
}

void autosub_filter_shown(void* data)
{

}

void autosub_filter_hidden(void* data)
{

}

void autosub_filter_activated(void* data)
{
    blog(LOG_INFO, "source activated.\n");
}

void autosub_filter_deactivated(void* data)
{
    blog(LOG_INFO, "source deactivated.\n");
}

void* autosub_filter_create(obs_data_t* settings, obs_source_t* source)
{
    auto s = (struct autosub_filter*)bzalloc(sizeof(struct autosub_filter));
    s->resampler = nullptr;
    s->asr = nullptr;

    s->source = source;
    s->asr = new RTASR(APPID, APIKEY);
    emit s->asr->start();

    autosub_filter_update(s, settings);
    return s;
}


void autosub_filter_destroy(void* data)
{
}

struct obs_audio_data * autosub_filter_audio(void *data, struct obs_audio_data *audio) {
    autosub_filter *s = (autosub_filter*)data;

    if(audio->frames == 0)
        return audio;
    uint8_t *output[MAX_AV_PLANES];
    memset(output, 0, sizeof(output));
    uint32_t out_samples = 0;
    uint64_t ts_offset = 0;
    bool ok = audio_resampler_resample(s->resampler, output, &out_samples, &ts_offset, audio->data, audio->frames);
    if(ok) {
        s->asr->sendAudioMessage(output[0], out_samples * 2);
    }
    return audio;
}

struct obs_source_info create_autosub_filter_info()
{
    struct obs_source_info autosub_filter_info = {};
    autosub_filter_info.id				= "autosub_filter";
    autosub_filter_info.type			= OBS_SOURCE_TYPE_FILTER;
    autosub_filter_info.output_flags	= OBS_SOURCE_AUDIO;
    autosub_filter_info.filter_audio    = autosub_filter_audio;
    autosub_filter_info.get_name		= autosub_filter_getname;
    autosub_filter_info.get_properties	= autosub_filter_getproperties;
    autosub_filter_info.get_defaults	= autosub_filter_getdefaults;
    autosub_filter_info.update			= autosub_filter_update;
    autosub_filter_info.show			= autosub_filter_shown;
    autosub_filter_info.hide			= autosub_filter_hidden;
    autosub_filter_info.activate		= autosub_filter_activated;
    autosub_filter_info.deactivate		= autosub_filter_deactivated;
    autosub_filter_info.create			= autosub_filter_create;
    autosub_filter_info.destroy			= autosub_filter_destroy;

    return autosub_filter_info;
}
