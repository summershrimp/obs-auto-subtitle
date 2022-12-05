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
#include "AliNLSBuilder.h"

#define ALINLS_PROVIDER_ID 0x0001U
#define L_SP_ALINLS "AutoSub.SP.Aliyun"

#define PROP_ALINLS_APPKEY "autosub_filter_alinls_appkey"
#define T_APPKEY obs_module_text("AutoSub.AppKey")

#define PROP_ALINLS_TOKEN "autosub_filter_alinls_token"

#define PROP_ALINLS_PUNC "autosub_filter_alinls_punc"
#define T_ALINLS_PUNC obs_module_text("AutoSub.AliNLS.Punc")
#define PROP_ALINLS_ITN "autosub_filter_alinls_itn"
#define T_ALINLS_ITN obs_module_text("AutoSub.AliNLS.ITN")
#define PROP_ALINLS_INTRESULT "autosub_filter_alinls_InterResult"
#define T_ALINLS_INTRESULT obs_module_text("AutoSub.AliNLS.InterResult")

void AliNLSBuilder::getProperties(obs_properties_t *props){
    obs_properties_add_text(props, PROP_ALINLS_APPKEY, T_APPKEY,
			    OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_ALINLS_TOKEN, T_TOKEN,
			    OBS_TEXT_DEFAULT);
	obs_properties_add_bool(props, PROP_ALINLS_PUNC, T_ALINLS_PUNC);
	obs_properties_add_bool(props, PROP_ALINLS_ITN, T_ALINLS_ITN);
	obs_properties_add_bool(props, PROP_ALINLS_INTRESULT,
				    T_ALINLS_INTRESULT);
}

void AliNLSBuilder::showProperties(obs_properties_t *props){
    PROPERTY_SET_VISIBLE(props, PROP_ALINLS_APPKEY);
    PROPERTY_SET_VISIBLE(props, PROP_ALINLS_TOKEN);
    PROPERTY_SET_VISIBLE(props, PROP_ALINLS_PUNC);
    PROPERTY_SET_VISIBLE(props, PROP_ALINLS_ITN);
    PROPERTY_SET_VISIBLE(props, PROP_ALINLS_INTRESULT);
}

void AliNLSBuilder::hideProperties(obs_properties_t *props){
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_APPKEY);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_TOKEN);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_PUNC);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_ITN);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_INTRESULT);
}

void AliNLSBuilder::updateSettings(obs_data_t *settings){
    QString _appkey = obs_data_get_string(settings, PROP_ALINLS_APPKEY);
    QString _token = obs_data_get_string(settings, PROP_ALINLS_TOKEN);
    bool _punc = obs_data_get_bool(settings, PROP_ALINLS_PUNC);
    bool _itn = obs_data_get_bool(settings, PROP_ALINLS_ITN);
    bool _inter_result =
        obs_data_get_bool(settings, PROP_ALINLS_INTRESULT);

    CHECK_CHANGE_SET_ALL(this->appkey, _appkey, needBuild);
    CHECK_CHANGE_SET_ALL(this->token, _token, needBuild);
    CHECK_CHANGE_SET_ALL(this->punc, _punc, needBuild);
    CHECK_CHANGE_SET_ALL(this->itn, _itn, needBuild);
    CHECK_CHANGE_SET_ALL(this->inter_result, _inter_result, needBuild);
}

void AliNLSBuilder::getDefaults(obs_data_t *settings){
    (void) settings;
}

ASRBase *AliNLSBuilder::build(){
    if(!needBuild) {
        return nullptr;
    }
    needBuild = false;
    auto asr = new AliNLS(appkey, token);
    if (punc) {
        asr->setParam("enable_punctuation_prediction",
                    "true");
    }
    if (itn) {
        asr->setParam("enable_inverse_text_normalization",
                    "true");
    }
    if (inter_result) {
        asr->setParam("enable_intermediate_result", "true");
    }
    return asr;
}

static AliNLSBuilder aliNLSBuilder; 
static ASRBuilderRegister register_Alinls_asr(&aliNLSBuilder, ALINLS_PROVIDER_ID, L_SP_ALINLS);
