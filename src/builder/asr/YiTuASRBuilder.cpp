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
#include "YiTuASRBuilder.h"   


#define YITU_PROVIDER_ID 0x0005U
#define L_SP_YITUASR "AutoSub.SP.YiTuASR"

#define PROP_YITUASR_LANG _PROP("yitu_lang")
#define PROP_YITUASR_DEVID _PROP("yitu_devid")
#define PROP_YITUASR_DEVKEY _PROP("yitu_devkey")
#define PROP_YITUASR_PUNC _PROP("yitu_punc")
#define T_YITUASR_PUNC obs_module_text("AutoSub.YiTuASR.Punc")
#define PROP_YITUASR_NOA _PROP("yitu_noa")
#define T_YITUASR_NOA obs_module_text("AutoSub.YiTuASR.NOA")

void YiTuASRBuilder::getProperties(obs_properties_t *props){
    auto t = obs_properties_add_list(props, PROP_YITUASR_LANG, T_LANGUAGE,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(t, "English", "en");
    obs_property_list_add_string(t, "Chinese", "cn");

    obs_properties_add_text(props, PROP_YITUASR_DEVID, "DevId",
			    OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_YITUASR_DEVKEY, "DevKey",
			    OBS_TEXT_DEFAULT);
	obs_properties_add_bool(props, PROP_YITUASR_PUNC, T_YITUASR_PUNC);
	obs_properties_add_bool(props, PROP_YITUASR_NOA, T_YITUASR_NOA);

}

void YiTuASRBuilder::showProperties(obs_properties_t *props){
    PROPERTY_SET_VISIBLE(props, PROP_YITUASR_LANG);
    PROPERTY_SET_VISIBLE(props, PROP_YITUASR_DEVID);
    PROPERTY_SET_VISIBLE(props, PROP_YITUASR_DEVKEY);
    PROPERTY_SET_VISIBLE(props, PROP_YITUASR_PUNC);
    PROPERTY_SET_VISIBLE(props, PROP_YITUASR_NOA);
}

void YiTuASRBuilder::hideProperties(obs_properties_t *props){
    PROPERTY_SET_UNVISIBLE(props, PROP_YITUASR_LANG);
    PROPERTY_SET_UNVISIBLE(props, PROP_YITUASR_DEVID);
    PROPERTY_SET_UNVISIBLE(props, PROP_YITUASR_DEVKEY);
    PROPERTY_SET_UNVISIBLE(props, PROP_YITUASR_PUNC);
    PROPERTY_SET_UNVISIBLE(props, PROP_YITUASR_NOA);
}

void YiTuASRBuilder::updateSettings(obs_data_t *settings){
    QString _lang = obs_data_get_string(settings, PROP_YITUASR_LANG);
    QString _devid = obs_data_get_string(settings, PROP_YITUASR_DEVID);
    QString _devkey = obs_data_get_string(settings, PROP_YITUASR_DEVKEY);
    bool _punc = obs_data_get_bool(settings, PROP_YITUASR_PUNC);
    bool _noa = obs_data_get_bool(settings, PROP_YITUASR_NOA);
    CHECK_CHANGE_SET_ALL(this->lang, _lang, needBuild);
    CHECK_CHANGE_SET_ALL(this->devId, _devid, needBuild);
    CHECK_CHANGE_SET_ALL(this->devKey, _devkey, needBuild);
    CHECK_CHANGE_SET_ALL(this->punc, _punc, needBuild);
    CHECK_CHANGE_SET_ALL(this->noa, _noa, needBuild);
}

void YiTuASRBuilder::getDefaults(obs_data_t *settings){
    (void) settings;
}

ASRBase *YiTuASRBuilder::build(){
    if(!needBuild) {
        return nullptr;
    }
    needBuild = false;
    auto asr = new YiTuASR(devId, devKey);
    asr->setLang(lang);
    asr->setEnableNOA(noa);
    asr->setEnablePunc(punc);
    return asr;
}

static YiTuASRBuilder yituasrbuilder; 
static ASRBuilderRegister register_yituasr(&yituasrbuilder, YITU_PROVIDER_ID, L_SP_YITUASR);

