/*
obs-auto-subtitle
 Copyright (C) 2019-2020 Yibai Zhang

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
#include <memory>
#include <QObject>
#include <obs-module.h>
#include <obs.h>
#include <util/threading.h>
#include <util/platform.h>
#include <chrono>
#include <QThread>
#include <media-io/audio-resampler.h>

#include "obs-auto-subtitle.h"
#include "obs-autosub-filter.h"

const char *autosub_filter_getname(void *data)
{
	UNUSED_PARAMETER(data);
	return T_FILTER_NAME;
}

static bool add_sources(void *data, obs_source_t *source)
{
	obs_property_t *sources = (obs_property_t *)data;
	auto source_id = obs_source_get_id(source);
	if (strcmp(source_id, "text_ft2_source_v2") != 0 &&
	    strcmp(source_id, "text_gdiplus_v2") != 0) {
		return true;
	}

	const char *name = obs_source_get_name(source);
	obs_property_list_add_string(sources, name, name);
	return true;
}

#ifdef PROPERTY_SET_UNVISIBLE
#undef PROPERTY_SET_UNVISIBLE
#endif

#define PROPERTY_SET_UNVISIBLE(props, prop_name)                           \
	do {                                                               \
		obs_property *prop = obs_properties_get(props, prop_name); \
		obs_property_set_visible(prop, false);                     \
	} while (0)

#ifdef PROPERTY_SET_VISIBLE
#undef PROPERTY_SET_VISIBLE
#endif
#define PROPERTY_SET_VISIBLE(props, prop_name)                             \
	do {                                                               \
		obs_property *prop = obs_properties_get(props, prop_name); \
		obs_property_set_visible(prop, true);                      \
	} while (0)

static bool provider_modified(obs_properties_t *props, obs_property_t *property,
			      obs_data_t *settings)
{
	int cur_provider = obs_data_get_int(settings, PROP_PROVIDER);

	for (auto iter : ASRBuilders.getAllBuilder()) {
		iter.second->hideProperties(props);
	}

	auto cur_builder = ASRBuilders.getBuilder(cur_provider);
	if (cur_builder) {
		cur_builder->showProperties(props);
	}

	return true;
}

static bool translate_provider_modified(void *priv, obs_properties_t *props,
					obs_property_t *property,
					obs_data_t *settings)
{
	int cur_provider = obs_data_get_int(settings, PROP_TRANS_PROVIDER);
	auto s = (struct autosub_filter *)priv;
	for (auto iter : TransBuilders.getAllBuilder()) {
		iter.second->hideProperties(props);
	}
	auto cur_builder = TransBuilders.getBuilder(cur_provider);
	if (cur_builder) {
		cur_builder->showProperties(props);
	}
	return true;
}

static bool translate_enable_modified(void *priv, obs_properties_t *props,
				      obs_property_t *property,
				      obs_data_t *settings)
{
	int enabled = obs_data_get_bool(settings, PROP_TRANS_ENABLED);
	auto s = (struct autosub_filter *)priv;
	if (!enabled) {
		PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_PROVIDER);
		PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_TARGET_TEXT_SOURCE);
		for (auto iter : TransBuilders.getAllBuilder()) {
			iter.second->hideProperties(props);
		}
	} else {
		PROPERTY_SET_VISIBLE(props, PROP_TRANS_PROVIDER);
		PROPERTY_SET_VISIBLE(props, PROP_TRANS_TARGET_TEXT_SOURCE);
		translate_provider_modified(priv, props, property, settings);
	}
	return true;
}

#undef PROPERTY_SET_UNVISIBLE
#undef PROPERTY_SET_VISIBLE

obs_properties_t *autosub_filter_getproperties(void *data)
{
	auto s = (struct autosub_filter *)data;

	obs_properties_t *props = obs_properties_create();

	obs_property_t *sources = obs_properties_add_list(
		props, PROP_TARGET_TEXT_SOURCE, T_TARGET_TEXT_SOURCE,
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	obs_properties_add_int(props, PROP_MAX_COUNT, T_MAX_CHAR_COUNT, 0,
			       100000, 1);
	obs_properties_add_int(props, PROP_CLEAR_TIMEOUT, T_CLEAR_TIMEOUT, 0,
			       100000, 100);

	obs_property_list_add_string(sources, obs_module_text("None"), "none");

	obs_enum_sources(add_sources, sources);

	auto providers = obs_properties_add_list(props, PROP_PROVIDER,
						 T_PROVIDER,
						 OBS_COMBO_TYPE_LIST,
						 OBS_COMBO_FORMAT_INT);
	for (auto iter : ASRBuilders.getAllBuilder()) {
		obs_property_list_add_int(
			providers,
			obs_module_text(ASRBuilders.getLocaleLabel(iter.first)),
			iter.first);
	}

	obs_property_set_modified_callback(providers, provider_modified);

	//ASR provider properties
	for (auto iter : ASRBuilders.getAllBuilder()) {
		iter.second->getProperties(props);
	}

	//Translate
	obs_property_t *t = obs_properties_add_bool(props, PROP_TRANS_ENABLED,
						    T_TRANS_ENABLE);
	obs_property_set_modified_callback2(t, translate_enable_modified, data);
	providers = obs_properties_add_list(props, PROP_TRANS_PROVIDER,
					    T_PROVIDER, OBS_COMBO_TYPE_LIST,
					    OBS_COMBO_FORMAT_INT);

	for (auto iter : TransBuilders.getAllBuilder()) {
		obs_property_list_add_int(
			providers,
			obs_module_text(
				TransBuilders.getLocaleLabel(iter.first)),
			iter.first);
	}

	obs_property_set_modified_callback2(providers,
					    translate_provider_modified, data);

	sources = obs_properties_add_list(props, PROP_TRANS_TARGET_TEXT_SOURCE,
					  T_TARGET_TEXT_SOURCE,
					  OBS_COMBO_TYPE_LIST,
					  OBS_COMBO_FORMAT_STRING);

	obs_property_list_add_string(sources, obs_module_text("None"), "none");

	obs_enum_sources(add_sources, sources);

	//Translate provider properties
	for (auto iter : TransBuilders.getAllBuilder()) {
		iter.second->getProperties(props);
	}
	return props;
}

void autosub_filter_getdefaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, PROP_MAX_COUNT, 0);
	obs_data_set_default_int(settings, PROP_CLEAR_TIMEOUT, 0);
}

struct resample_info resample_output = {16000, AUDIO_FORMAT_16BIT,
					SPEAKERS_MONO};

void autosub_filter_update(void *data, obs_data_t *settings)
{
	autosub_filter *s = (autosub_filter *)data;
	obs_audio_info ai;
	if (!obs_get_audio_info(&ai))
		throw std::string("Failed to get OBS audio info");
	s->sample_rate = ai.samples_per_sec;
	s->channels = ai.speakers;
	resample_info resample_input = {ai.samples_per_sec,
					AUDIO_FORMAT_FLOAT_PLANAR, ai.speakers};
	s->resampler_update_lock.lock();
	if (s->resampler != nullptr) {
		audio_resampler_destroy(s->resampler);
		s->resampler = nullptr;
	}
	s->resampler =
		audio_resampler_create(&resample_output, &resample_input);
	s->resampler_update_lock.unlock();

	const char *text_source_name =
		obs_data_get_string(settings, PROP_TARGET_TEXT_SOURCE);
	bool valid_text_source = strcmp(text_source_name, "none") != 0;
	obs_weak_source_t *old_weak_text_source = NULL;
	s->text_source_update_lock.lock();
	if (!valid_text_source) {
		if (s->text_source) {
			old_weak_text_source = s->text_source;
			s->text_source = NULL;
		}
	} else {
		if (!s->text_source_name ||
		    strcmp(s->text_source_name, text_source_name) != 0) {
			if (s->text_source) {
				old_weak_text_source = s->text_source;
				s->text_source = NULL;
			}
			s->text_source_name = text_source_name;
			s->text_source_last_update =
				os_gettime_ns() - 3000000000;
		}
	}
	s->text_source_update_lock.unlock();

	if (old_weak_text_source) {
		obs_weak_source_release(old_weak_text_source);
	}

	s->max_count = obs_data_get_int(settings, PROP_MAX_COUNT);
	s->clear_timeout = obs_data_get_int(settings, PROP_CLEAR_TIMEOUT);

	int provider = obs_data_get_int(settings, PROP_PROVIDER);
	if (provider != s->provider) {
		s->provider = provider;
		s->refresh = true;
	}
	ASRBuilderBase *asrBuilder =
		static_cast<ASRBuilderBase *>(ASRBuilders.getBuilder(provider));
	std::shared_ptr<ASRBase> new_asr;
	s->lock_asr.lock();
	if (asrBuilder) {
		asrBuilder->updateSettings(settings);
		new_asr.reset(asrBuilder->build());
		if (new_asr) {
			s->asr = new_asr;
		}
	}

	std::function<void(QString)> setText([=](QString str) {
		obs_weak_source_t *text_source = nullptr;
		s->text_source_update_lock.lock();
		text_source = s->text_source;
		s->text_source_update_lock.unlock();
		if (!text_source) {
			return;
		}
		auto target = obs_weak_source_get_source(text_source);
		if (!target) {
			return;
		}
		auto text_settings = obs_source_get_settings(target);
		obs_data_set_string(text_settings, "text",
				    str.toUtf8().toStdString().c_str());
		obs_source_update(target, text_settings);
		obs_source_release(target);
	});
	if (s->asr) {
		s->asr->setErrorCallback(
			[=](ASRBase::ErrorType type, QString msg) {
				static bool prevIsApiError = false;
				blog(LOG_INFO, "Websocket error: %d, %s", type,
				     msg.toStdString().c_str());
				QString errorMsg;
				switch (type) {
				case ASRBase::ERROR_API:
					errorMsg = "API Error";
					prevIsApiError = true;
					break;
				case ASRBase::ERROR_SOCKET:
					if (prevIsApiError) {
						return;
					}
					errorMsg = "Websocket Error";
					break;
				default:
					errorMsg = "Unknown error";
					break;
				}
				if (!msg.isEmpty()) {
					errorMsg = errorMsg + ": " + msg;
				}
				setText(errorMsg);
			});

		s->asr->setConnectedCallback([=]() {
			blog(LOG_INFO, "Websocket connected.");
			setText("Connected");
		});

		s->asr->setDisconnectedCallback(
			[=]() { blog(LOG_INFO, "Websocket disconnected."); });

		s->asr->start();
		s->running = true;
	}
	s->lock_asr.unlock();

	bool trans_enabled = obs_data_get_bool(settings, PROP_TRANS_ENABLED);
	s->enable_trans = trans_enabled;

	int trans_sp = obs_data_get_int(settings, PROP_TRANS_PROVIDER);
	s->trans_provider = trans_sp;
	TransBuilderBase *transBuilder = static_cast<TransBuilderBase *>(
		TransBuilders.getBuilder(trans_sp));
	std::shared_ptr<TransBase> new_translator;
	if (transBuilder) {
		transBuilder->updateSettings(settings);
		new_translator.reset(transBuilder->build());
		if (new_translator) {
			s->lock_trans.lock();
			s->translator = new_translator;
			s->lock_trans.unlock();
		}
	}

	const char *trans_source_name =
		obs_data_get_string(settings, PROP_TRANS_TARGET_TEXT_SOURCE);
	valid_text_source = strcmp(trans_source_name, "none") != 0;
	obs_weak_source_t *old_weak_trans_source = NULL;
	s->trans_source_update_lock.lock();
	if (!valid_text_source) {
		if (s->trans_source) {
			old_weak_trans_source = s->trans_source;
			s->trans_source = NULL;
		}
	} else {
		if (!s->trans_source_name ||
		    strcmp(s->trans_source_name, trans_source_name) != 0) {
			if (s->trans_source) {
				old_weak_trans_source = s->trans_source;
				s->trans_source = NULL;
			}
			s->trans_source_name = trans_source_name;
			s->trans_source_last_update =
				os_gettime_ns() - 3000000000;
		}
	}
	s->trans_source_update_lock.unlock();

	if (old_weak_trans_source) {
		obs_weak_source_release(old_weak_trans_source);
	}

	std::function<void(QString)> setTransText([=](QString str) {
		obs_weak_source_t *trans_source = nullptr;
		blog(LOG_INFO, "TransResult: %s", str.toStdString().c_str());
		s->trans_source_update_lock.lock();
		trans_source = s->trans_source;
		s->trans_source_update_lock.unlock();
		if (!trans_source) {
			return;
		}
		auto target = obs_weak_source_get_source(trans_source);
		if (!target) {
			return;
		}
		auto text_settings = obs_source_get_settings(target);
		obs_data_set_string(text_settings, "text",
				    str.toUtf8().toStdString().c_str());
		obs_source_update(target, text_settings);
		obs_source_release(target);
	});

	if (s->translator) {
		s->translator->setResultCallback(
			[=](QString data) { setTransText(data); });

		s->translator->setErrorCallback(
			[=](QString data) { setTransText(data); });
	}

	s->lock_asr.lock();
	if (s->asr) {
		s->asr->setResultCallback([=](QString str, int typ) {
			if (typ == 0) {
				if (s->enable_trans) {
					s->lock_trans.lock();
					if (s->translator && transBuilder)
						emit s->translator->requestTranslate(
							transBuilder
								->getFromLang(),
							transBuilder
								->getToLang(),
							str);
					s->lock_trans.unlock();
				}
				blog(LOG_INFO, "Result: %s",
				     str.toStdString().c_str());
				s->last_update_time = os_gettime_ns();
			}
			int t = s->max_count;
			if (t != 0 && str.count() > t) {
				str = str.right(t);
			}
			setText(str);
		});
	}
	s->lock_asr.unlock();
}

void autosub_filter_shown(void *data) {}

void autosub_filter_hidden(void *data) {}

void autosub_filter_activated(void *data)
{
	blog(LOG_INFO, "source activated.\n");
}

void autosub_filter_deactivated(void *data)
{
	blog(LOG_INFO, "source deactivated.\n");
}

void *autosub_filter_create(obs_data_t *settings, obs_source_t *source)
{
	auto s = new autosub_filter;
	s->source = source;
	autosub_filter_update(s, settings);
	return s;
}

void autosub_filter_destroy(void *data)
{
	autosub_filter *s = (autosub_filter *)data;
	s->lock_asr.lock();
	s->running = false;
	if (s->asr) {
		s->asr->stop();
	}
	s->lock_asr.unlock();
	if (s->resampler) {
		audio_resampler_destroy(s->resampler);
		s->resampler = nullptr;
	}

	if (s->text_source) {
		obs_weak_source_release(s->text_source);
		s->text_source = nullptr;
	}

	delete s;
}

struct obs_audio_data *autosub_filter_audio(void *data,
					    struct obs_audio_data *audio)
{
	autosub_filter *s = (autosub_filter *)data;

	s->resampler_update_lock.lock();
	if (audio->frames == 0 || !s->running || s->resampler == nullptr) {
		s->resampler_update_lock.unlock();
		return audio;
	}

	uint8_t *output[MAX_AV_PLANES];
	memset(output, 0, sizeof(output));
	uint32_t out_samples = 0;
	uint64_t ts_offset = 0;
	bool ok = audio_resampler_resample(s->resampler, output, &out_samples,
					   &ts_offset, audio->data,
					   audio->frames);
	s->resampler_update_lock.unlock();
	if (ok) {
		s->lock_asr.lock();
		emit s->asr->sendAudioMessage((const char *)(output[0]),
					      out_samples * 2);
		s->lock_asr.unlock();
	}
	return audio;
}

static void autosub_filter_tick(void *data, float seconds)
{
	autosub_filter *s = (autosub_filter *)data;
	char *new_name = nullptr;
	uint64_t t = os_gettime_ns();

	s->text_source_update_lock.lock();

	if (s->text_source_name && !s->text_source) {
		if (t - s->text_source_last_update > 3000000000) {
			new_name = bstrdup(s->text_source_name);
			s->text_source_last_update = t;
		}
	}

	s->text_source_update_lock.unlock();

	if (new_name) {
		obs_source_t *target_source =
			*new_name ? obs_get_source_by_name(new_name) : NULL;
		obs_weak_source_t *weak_target_source =
			target_source
				? obs_source_get_weak_source(target_source)
				: NULL;

		s->text_source_update_lock.lock();

		if (s->text_source_name &&
		    strcmp(s->text_source_name, new_name) == 0) {
			s->text_source = weak_target_source;
			weak_target_source = NULL;
		}

		s->text_source_update_lock.unlock();

		if (target_source) {
			obs_weak_source_release(weak_target_source);
			obs_source_release(target_source);
		}

		bfree(new_name);
		new_name = nullptr;
	}

	s->trans_source_update_lock.lock();

	if (s->trans_source_name && !s->trans_source) {
		if (t - s->trans_source_last_update > 3000000000) {
			new_name = bstrdup(s->trans_source_name);
			s->trans_source_last_update = t;
		}
	}

	s->trans_source_update_lock.unlock();

	if (new_name) {
		obs_source_t *target_source =
			*new_name ? obs_get_source_by_name(new_name) : NULL;
		obs_weak_source_t *weak_target_source =
			target_source
				? obs_source_get_weak_source(target_source)
				: NULL;

		s->trans_source_update_lock.lock();

		if (s->trans_source_name &&
		    strcmp(s->trans_source_name, new_name) == 0) {
			s->trans_source = weak_target_source;
			weak_target_source = NULL;
		}

		s->trans_source_update_lock.unlock();

		if (target_source) {
			obs_weak_source_release(weak_target_source);
			obs_source_release(target_source);
		}

		bfree(new_name);
		new_name = nullptr;
	}
	uint64_t clear_timeout_ns = (uint64_t)s->clear_timeout * 1000000UL;
	if (s->last_update_time != 0 && clear_timeout_ns != 0 &&
	    t - s->last_update_time > clear_timeout_ns) {
		s->last_update_time = 0;
		obs_weak_source_t *text_source = nullptr;
		s->text_source_update_lock.lock();
		text_source = s->text_source;
		s->text_source_update_lock.unlock();
		if (text_source) {
			auto target = obs_weak_source_get_source(text_source);
			if (target) {
				auto text_settings =
					obs_source_get_settings(target);
				obs_data_set_string(text_settings, "text", "");
				obs_source_update(target, text_settings);
				obs_source_release(target);
			}
		}

		obs_weak_source_t *trans_source = nullptr;
		s->trans_source_update_lock.lock();
		trans_source = s->trans_source;
		s->trans_source_update_lock.unlock();
		if (trans_source) {
			auto target = obs_weak_source_get_source(trans_source);
			if (target) {
				auto text_settings =
					obs_source_get_settings(target);
				obs_data_set_string(text_settings, "text", "");
				obs_source_update(target, text_settings);
				obs_source_release(target);
			}
		}
	}
}

struct obs_source_info create_autosub_filter_info()
{
	struct obs_source_info autosub_filter_info = {};
	autosub_filter_info.id = "autosub_filter";
	autosub_filter_info.type = OBS_SOURCE_TYPE_FILTER;
	autosub_filter_info.output_flags = OBS_SOURCE_AUDIO;
	autosub_filter_info.filter_audio = autosub_filter_audio;
	autosub_filter_info.get_name = autosub_filter_getname;
	autosub_filter_info.get_properties = autosub_filter_getproperties;
	autosub_filter_info.get_defaults = autosub_filter_getdefaults;
	autosub_filter_info.update = autosub_filter_update;
	autosub_filter_info.show = autosub_filter_shown;
	autosub_filter_info.hide = autosub_filter_hidden;
	autosub_filter_info.activate = autosub_filter_activated;
	autosub_filter_info.deactivate = autosub_filter_deactivated;
	autosub_filter_info.create = autosub_filter_create;
	autosub_filter_info.destroy = autosub_filter_destroy;
	autosub_filter_info.video_tick = autosub_filter_tick;

	return autosub_filter_info;
}
