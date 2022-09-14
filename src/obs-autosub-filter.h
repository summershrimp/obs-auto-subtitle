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

#ifndef OBS_AUTOSUB_FILTER_H
#define OBS_AUTOSUB_FILTER_H

#include "builder/base/TransBuilderBase.h"
#include "builder/base/ASRBuilderBase.h"

#define T_FILTER_NAME obs_module_text("AutoSub.FilterName")

#define PROP_MAX_COUNT "autosub_filter_max_count"
#define T_MAX_CHAR_COUNT obs_module_text("AutoSub.MaxCharCount")

#define PROP_CLEAR_TIMEOUT "autosub_filter_clear_timeout"
#define T_CLEAR_TIMEOUT obs_module_text("AutoSub.ClearTimeout")

#define PROP_PROVIDER "autosub_filter_sp"
#define T_PROVIDER obs_module_text("AutoSub.ServiceProvider")

#define PROP_TARGET_TEXT_SOURCE "autosub_filter_target_source"
#define T_TARGET_TEXT_SOURCE obs_module_text("AutoSub.Target.Source")

#define PROP_TRANS_PROVIDER "autosub_filter_trans_sp"
#define PROP_TRANS_ENABLED "autosub_filter_enable_trans"
#define T_TRANS_ENABLE obs_module_text("AutoSub.EnableTrans")

#define PROP_TRANS_TARGET_TEXT_SOURCE "autosub_filter_trans_target_source"

using namespace std::placeholders;

struct autosub_filter {
	obs_source_t *source = nullptr;
	uint32_t sample_rate = 48000;
	uint32_t channels = 2;
	int max_count = 0;
	int clear_timeout = 0;
	bool running = false;

	int provider;
	struct {
		QString appId;
		QString apiKey;
		bool punc;
		QString pd;
	} xfyun;

	struct {
		QString project_id;
		QString token;
	} hwcloud;

	struct {
		QString appKey;
		QString token;
		bool punc;
		bool itn;
		bool int_result;
	} alinls;

	int refresh = false;

	std::shared_ptr<ASRBase> asr = nullptr;
	std::mutex lock_asr;
	audio_resampler_t *resampler = nullptr;
	std::mutex resampler_update_lock;

	const char *text_source_name = nullptr;
	std::mutex text_source_update_lock;
	obs_weak_source_t *text_source = nullptr;
	uint64_t text_source_last_update = 0;
	uint64_t last_update_time = os_gettime_ns();

	bool enable_trans;
	int trans_provider;

	std::shared_ptr<TransBase> translator = nullptr;
	std::mutex lock_trans;

	const char *trans_source_name = nullptr;
	std::mutex trans_source_update_lock;
	obs_weak_source_t *trans_source = nullptr;
	uint64_t trans_source_last_update = 0;
};

#endif