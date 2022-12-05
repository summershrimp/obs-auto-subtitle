/*
obs-auto-subtitle
 Copyright (C) 2019-2022 Yibai Zhang

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
#include "XFRtASRBuilder.h"

#define XFYUN_PROVIDER_ID 0x0003U
#define L_SP_XFYUN "AutoSub.SP.Xfyun"

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

void XFRtASRBuilder::getProperties(obs_properties_t *props){
	obs_properties_add_text(props, PROP_XF_APPID, T_APPID,
					 OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_XF_APIKEY, T_APIKEY,
			    OBS_TEXT_DEFAULT);
	obs_properties_add_bool(props, PROP_XF_PUNC, T_XF_PUNC);

	auto t = obs_properties_add_list(props, PROP_XF_PD, T_XF_PD,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(t, T_XF_PD_NONE, "none");
	obs_property_list_add_string(t, T_XF_PD_COURT, "court");
	obs_property_list_add_string(t, T_XF_PD_EDU, "edu");
	obs_property_list_add_string(t, T_XF_PD_FINANCE, "finance");
	obs_property_list_add_string(t, T_XF_PD_MEDICAL, "medical");
	obs_property_list_add_string(t, T_XF_PD_TECH, "tech");
}

void XFRtASRBuilder::showProperties(obs_properties_t *props){
    PROPERTY_SET_VISIBLE(props, PROP_XF_APPID);
    PROPERTY_SET_VISIBLE(props, PROP_XF_APIKEY);
    PROPERTY_SET_VISIBLE(props, PROP_XF_PUNC);
    PROPERTY_SET_VISIBLE(props, PROP_XF_PD);
}

void XFRtASRBuilder::hideProperties(obs_properties_t *props){
    PROPERTY_SET_UNVISIBLE(props, PROP_XF_APPID);
	PROPERTY_SET_UNVISIBLE(props, PROP_XF_APIKEY);
	PROPERTY_SET_UNVISIBLE(props, PROP_XF_PUNC);
	PROPERTY_SET_UNVISIBLE(props, PROP_XF_PD);
}

void XFRtASRBuilder::updateSettings(obs_data_t *settings){
    QString _appid = obs_data_get_string(settings, PROP_XF_APPID);
    QString _apikey = obs_data_get_string(settings, PROP_XF_APIKEY);
    bool _punc = obs_data_get_bool(settings, PROP_XF_PUNC);
    QString _pd = obs_data_get_string(settings, PROP_XF_PD);
    CHECK_CHANGE_SET_ALL(this->appid, _appid, needBuild);
    CHECK_CHANGE_SET_ALL(this->apikey, _apikey, needBuild);
    CHECK_CHANGE_SET_ALL(this->punc, _punc, needBuild);
    CHECK_CHANGE_SET_ALL(this->pd, _pd, needBuild);
}

void XFRtASRBuilder::getDefaults(obs_data_t *settings){
    (void) settings;
}

ASRBase *XFRtASRBuilder::build(){
    if(!needBuild) {
        return nullptr;
    }
    needBuild = false;
    auto asr = new XFRtASR(appid, apikey);
    if (pd != "none") {
        asr->setParam("pd", pd);
    }
    if (!punc) {
        asr->setParam("punc", "0");
    }
    return asr;
}

static XFRtASRBuilder xfrtasrBuilder; 
static ASRBuilderRegister register_xfrtasr_asr( &xfrtasrBuilder, XFYUN_PROVIDER_ID, L_SP_XFYUN);
