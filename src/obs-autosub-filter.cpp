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

#include <string.h>
#include <functional>
#include <QObject>
#include <obs-module.h>
#include <obs.h>
#include <util/threading.h>
#include <chrono>
#include <QThread>
#include <media-io/audio-resampler.h>

#include "obs-auto-subtitle.h"
#include "src/vendor/XFRtASR.h"
#include "src/vendor/HwCloudRASR.h"

#define T_FILTER_NAME obs_module_text("AutoSub.FilterName")

#define PROP_PROVIDER "autosub_filter_sp"
#define T_PROVIDER obs_module_text("AutoSub.ServiceProvider")

#define PROP_XF_APPID "autosub_filter_xf_appid"
#define T_APPID obs_module_text("AutoSub.APPID")

#define PROP_XF_APIKEY "autosub_filter_xf_apikey"
#define T_APIKEY obs_module_text("AutoSub.APIKEY")

#define PROP_HWCLOUD_PROJID "autosub_filter_hwcloud_proj_id"
#define T_PROJECT_ID obs_module_text("AutoSub.ProjectId")

#define PROP_HWCLOUD_TOKEN "autosub_filter_hwcloud_token"
#define T_TOKEN obs_module_text("AutoSub.Token")

#define PROP_TARGET_TEXT_SOURCE "autosub_filter_target_source"
#define T_TARGET_TEXT_SOURCE obs_module_text("AutoSub.Target.Source")

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
    bool running;
    const char *target_source_name;

    int provider;
    struct {
        QString appId;
        QString apiKey;
    }xfyun;

    struct {
        QString project_id;
        QString token;
    }hwcloud;

    int refresh;

    ASRBase *asr;
    std::mutex lock_asr;
    audio_resampler_t *resampler;

    std::mutex lock_target_text;
    obs_weak_source_t *target_text;
};

const char* autosub_filter_getname(void* data)
{
    UNUSED_PARAMETER(data);
    return T_FILTER_NAME;
}

static bool add_sources(void *data, obs_source_t *source)
{
    obs_property_t *sources = (obs_property_t *)data;

    if(strcmp(obs_source_get_id(source), "text_ft2_source_v2") != 0){
        return true;
    }

    const char *name = obs_source_get_name(source);
    obs_property_list_add_string(sources, name, name);
    return true;
}

#define PROPERTY_SET_UNVISIBLE(props, prop_name) \
    do {\
        obs_property *prop = obs_properties_get(props, prop_name);\
        obs_property_set_visible(prop, false);\
    }while(0)

#define PROPERTY_SET_VISIBLE(props, prop_name) \
    do {\
        obs_property *prop = obs_properties_get(props, prop_name);\
        obs_property_set_visible(prop, true);\
    }while(0)

static bool provider_modified(obs_properties_t *props,
                            obs_property_t *property,
                            obs_data_t *settings){
    int cur_provider = obs_data_get_int(settings, PROP_PROVIDER);
    PROPERTY_SET_UNVISIBLE(props, PROP_XF_APPID);
    PROPERTY_SET_UNVISIBLE(props, PROP_XF_APIKEY);
    PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_PROJID);
    PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_TOKEN);

    switch(cur_provider) {
        case SP_Hwcloud:
            PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_PROJID);
            PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_TOKEN);
            break;
        case SP_Xfyun:
            PROPERTY_SET_VISIBLE(props, PROP_XF_APPID);
            PROPERTY_SET_VISIBLE(props, PROP_XF_APIKEY);
            break;
        case SP_Aliyun:
            break;
        case SP_Sogou:
            break;
        default:
            break;
    }

    return true;
}
#undef PROPERTY_SET_UNVISIBLE

obs_properties_t* autosub_filter_getproperties(void* data)
{
    char nametext[256];
    auto s = (struct autosub_filter*)data;

    obs_properties_t* props = obs_properties_create();

    obs_property_t *sources = obs_properties_add_list(
            props, PROP_TARGET_TEXT_SOURCE, T_TARGET_TEXT_SOURCE,
            OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    obs_property_list_add_string(sources, obs_module_text("None"), "none");

    obs_enum_sources(add_sources, sources);


    auto providers = obs_properties_add_list(props, PROP_PROVIDER, T_PROVIDER, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(providers, T_SP_XFYUN, SP_Xfyun);
    obs_property_list_add_int(providers, T_SP_SOGOU, SP_Sogou);
    obs_property_list_add_int(providers, T_SP_HWCLOUD, SP_Hwcloud);
    obs_property_list_add_int(providers, T_SP_ALIYUN, SP_Aliyun);

    obs_property_set_modified_callback(providers, provider_modified);

    auto t = obs_properties_add_text(props, PROP_XF_APPID, T_APPID, OBS_TEXT_DEFAULT);
    obs_property_set_visible(t, false);
    t = obs_properties_add_text(props, PROP_XF_APIKEY, T_APIKEY, OBS_TEXT_DEFAULT);
    obs_property_set_visible(t, false);


    t = obs_properties_add_text(props, PROP_HWCLOUD_PROJID, T_PROJECT_ID, OBS_TEXT_DEFAULT);
    obs_property_set_visible(t, false);
    t = obs_properties_add_text(props, PROP_HWCLOUD_TOKEN, T_TOKEN, OBS_TEXT_MULTILINE);
    obs_property_set_visible(t, false);


    return props;
}

void autosub_filter_getdefaults(obs_data_t* settings)
{
    obs_data_set_default_int(settings, PROP_PROVIDER, SP_Xfyun);

}

struct resample_info resample_output = {
    16000,
    AUDIO_FORMAT_16BIT,
    SPEAKERS_MONO
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
        sample_rate,
        AUDIO_FORMAT_FLOAT,
        SPEAKERS_MONO
    };
    if(s->resampler != nullptr) {
        audio_resampler_destroy(s->resampler);
        s->resampler = nullptr;
    }
    s->resampler = audio_resampler_create(&resample_output, &resample_input);
    s->target_source_name = obs_data_get_string(settings, PROP_TARGET_TEXT_SOURCE);

    int provider = obs_data_get_int(settings, PROP_PROVIDER);
    if(provider != s->provider) {
        s->provider = provider;
        s->refresh = true;
    }
    const char *appid, *apikey, *project_id, *token;
    switch (s->provider) {
        case SP_Xfyun:
            appid = obs_data_get_string(settings, PROP_XF_APPID);
            apikey = obs_data_get_string(settings, PROP_XF_APIKEY);
            if(strcmp(appid, "") == 0 || strcmp(apikey, "") == 0) {
                s->refresh = false;
                break;
            }
            if(s->xfyun.apiKey == apikey && s->xfyun.appId == appid){
                s->refresh = s->refresh || false;
                break;
            }
            s->xfyun.appId = appid;
            s->xfyun.apiKey = apikey;
            s->refresh = true;
            qDebug() << "Xunfei: " << s->xfyun.appId << s->xfyun.apiKey;
            break;
        case SP_Hwcloud:
            project_id = obs_data_get_string(settings, PROP_HWCLOUD_PROJID);
            token = obs_data_get_string(settings, PROP_HWCLOUD_TOKEN);
            if(strcmp(project_id, "") == 0 || strcmp(token, "") == 0) {
                s->refresh = false;
                break;
            }
            if(s->hwcloud.project_id == project_id && s->hwcloud.token == token){
                s->refresh = s->refresh || false;
                break;
            }
            s->hwcloud.project_id = project_id;
            s->hwcloud.token = token;
            s->refresh = true;
            qDebug() << "HwCloud: " << s->hwcloud.project_id << s->hwcloud.token;
            break;
    }

    if(!s->refresh)
        return;
    s->refresh = false;
    s->lock_asr.lock();
    if(s->asr){
        s->asr->stop();
        delete s->asr;
        s->asr = nullptr;
        s->running = false;
    }
    switch(s->provider){
        case SP_Xfyun:
            s->asr = new XFRtASR(s->xfyun.appId, s->xfyun.apiKey);
            break;
        case SP_Hwcloud:
            s->asr = new HwCloudRASR(s->hwcloud.project_id, s->hwcloud.token);
            break;
        default:
            blog(LOG_WARNING, "Unsupported ASR provider id: %d", s->provider);
            break;
    }
    if(!s->asr) {
        s->lock_asr.unlock();
        return;
    }
    s->asr->setResultCallback([=](QString str, int typ){
        if(typ == 0)
            blog(LOG_INFO, "Result: %d, %s", typ, str.toStdString().c_str());
        s->lock_target_text.lock();
        if(!s->target_text) {
            s->lock_target_text.unlock();
            return;
        }
        auto target = obs_weak_source_get_source(s->target_text);
        s->lock_target_text.unlock();
        if(!target){
            return;
        }
        auto text_settings = obs_source_get_settings(target);
        obs_data_set_string(text_settings, "text", str.toUtf8().toStdString().c_str());
        obs_source_update(target, text_settings);
    });
    s->asr->start();
    s->running = true;
    s->lock_asr.unlock();

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
    auto s = new autosub_filter;
    s->resampler = nullptr;
    s->asr = nullptr;
    s->running = false;
    s->target_text = nullptr;
    s->provider = SP_Default;

    s->source = source;

    autosub_filter_update(s, settings);
    return s;
}


void autosub_filter_destroy(void* data)
{
    autosub_filter *s = (autosub_filter*)data;
    s->lock_asr.lock();
    s->running = false;
    if(s->asr) {
        s->asr->stop();
        delete s->asr;
    }
    s->lock_asr.unlock();
    if(s->resampler) {
        audio_resampler_destroy(s->resampler);
    }
    delete s;
}

struct obs_audio_data * autosub_filter_audio(void *data, struct obs_audio_data *audio) {
    autosub_filter *s = (autosub_filter*)data;

    if(audio->frames == 0 || !s->running)
        return audio;

    uint8_t *output[MAX_AV_PLANES];
    memset(output, 0, sizeof(output));
    uint32_t out_samples = 0;
    uint64_t ts_offset = 0;
    bool ok = audio_resampler_resample(s->resampler, output, &out_samples, &ts_offset, audio->data, audio->frames);
    if(ok) {
        s->lock_asr.lock();
        emit s->asr->sendAudioMessage(output[0], out_samples * 2);
        s->lock_asr.unlock();
    }
    return audio;
}

static void autosub_filter_tick(void *data, float seconds){
    autosub_filter *s = (autosub_filter*)data;
    obs_source_t *target_source = obs_get_source_by_name(s->target_source_name);
    if(s->target_text != nullptr){
        obs_source_t *original_source = obs_weak_source_get_source(s->target_text);
        if(strcmp(obs_source_get_name(original_source), s->target_source_name) == 0){
            return ;
        }
    }
    s->lock_target_text.lock();
    if(!target_source) {
        s->target_text = nullptr;
    } else {
        s->target_text = obs_source_get_weak_source(target_source);
        auto text_settings = obs_source_get_settings(target_source);
        obs_data_set_string(text_settings, "text", "Preparing...");
        obs_source_update(target_source, text_settings);
    }
    s->lock_target_text.unlock();
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
    autosub_filter_info.video_tick      = autosub_filter_tick;

    return autosub_filter_info;
}
