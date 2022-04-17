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

#include "vendor/ASR/XFRtASR.h"
#include "vendor/ASR/HwCloudRASR.h"
#include "vendor/ASR/AliNLS.h"
#include "vendor/Trans/XFTrans.h"
#include "builder/XFTransBuilder.h"
#include "builder/GSTransBuilder.h"

#define T_FILTER_NAME obs_module_text("AutoSub.FilterName")


#define PROP_MAX_COUNT "autosub_filter_max_count"
#define T_MAX_CHAR_COUNT obs_module_text("AutoSub.MaxCharCount")

#define PROP_CLEAR_TIMEOUT "autosub_filter_clear_timeout"
#define T_CLEAR_TIMEOUT obs_module_text("AutoSub.ClearTimeout")

#define PROP_PROVIDER "autosub_filter_sp"
#define T_PROVIDER obs_module_text("AutoSub.ServiceProvider")

#define PROP_XF_APPID "autosub_filter_xf_appid"
#define T_APPID obs_module_text("AutoSub.APPID")

#define PROP_XF_APIKEY "autosub_filter_xf_apikey"
#define T_APIKEY obs_module_text("AutoSub.APIKEY")

#define PROP_XF_APISECRET "autosub_filter_xf_apisecret"
#define T_APISECRET obs_module_text("AutoSub.APISECRET")

#define PROP_XF_PUNC "autosub_filter_xf_punc"
#define T_XF_PUNC obs_module_text("AutoSub.XF.Punc")

#define PROP_XF_PD "autosub_filter_xf_pd"
#define T_XF_PD obs_module_text("AutoSub.XF.PD")
#define T_XF_PD_NONE obs_module_text("AutoSub.XF.PD.None")
#define T_XF_PD_COURT obs_module_text("AutoSub.XF.PD.Court")
#define T_XF_PD_EDU obs_module_text("AutoSub.XF.PD.Edu")
#define T_XF_PD_FINANCE obs_module_text("AutoSub.XF.PD.Finance")
#define T_XF_PD_MEDICAL obs_module_text("AutoSub.XF.PD.Medical")
#define T_XF_PD_TECH obs_module_text("AutoSub.XF.PD.Tech")

#define PROP_HWCLOUD_PROJID "autosub_filter_hwcloud_proj_id"
#define T_PROJECT_ID obs_module_text("AutoSub.ProjectId")

#define PROP_HWCLOUD_TOKEN "autosub_filter_hwcloud_token"
#define T_TOKEN obs_module_text("AutoSub.Token")

#define PROP_ALINLS_APPKEY "autosub_filter_alinls_appkey"
#define T_APPKEY obs_module_text("AutoSub.AppKey")

#define PROP_ALINLS_TOKEN "autosub_filter_alinls_token"

#define PROP_ALINLS_PUNC "autosub_filter_alinls_punc"
#define T_ALINLS_PUNC obs_module_text("AutoSub.AliNLS.Punc")
#define PROP_ALINLS_ITN "autosub_filter_alinls_itn"
#define T_ALINLS_ITN obs_module_text("AutoSub.AliNLS.ITN")
#define PROP_ALINLS_INTRESULT "autosub_filter_alinls_InterResult"
#define T_ALINLS_INTRESULT obs_module_text("AutoSub.AliNLS.InterResult")

#define PROP_TARGET_TEXT_SOURCE "autosub_filter_target_source"
#define T_TARGET_TEXT_SOURCE obs_module_text("AutoSub.Target.Source")

#define PROP_TRANS_PROVIDER "autosub_filter_trans_sp"
#define PROP_TRANS_ENABLED "autosub_filter_enable_trans"
#define T_TRANS_ENABLE obs_module_text("AutoSub.EnableTrans")

#define PROP_TRANS_TARGET_TEXT_SOURCE "autosub_filter_trans_target_source"


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

enum TransServiceProvider {
    Trans_SP_Default = 0,
    Trans_SP_Xfyun,
    Trans_SP_XfNiu,
    Trans_SP_GScript,
};

#define T_TRANS_SP_XFYUN obs_module_text("AutoSub.Trans.SP.Xfyun")
#define T_TRANS_SP_XFNIU obs_module_text("AutoSub.Trans.SP.XfyunNiu")
#define T_TRANS_SP_GSCRIPT obs_module_text("AutoSub.Trans.SP.GoogleScript")


struct autosub_filter
{
    obs_source_t* source = nullptr;
    uint32_t sample_rate = 48000;
    uint32_t channels = 2;
    int max_count = 0;
    int clear_timeout = 0;
    bool running = false;

    int provider = SP_Default;
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

    ASRBase *asr = nullptr;
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

    XFTransBuilder xfTransBuilder;
    GSTransBuilder gsTransBuilder;

    std::shared_ptr<TransBase> translator = nullptr;
    std::mutex lock_trans;

    const char *trans_source_name = nullptr;
    std::mutex trans_source_update_lock;
    obs_weak_source_t *trans_source = nullptr;
    uint64_t trans_source_last_update = 0;
};


#endif