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
#include "HwCloudRASRBuilder.h"

#define HWCLOUD_PROVIDER_ID 0x0002U
#define L_SP_HWCLOUD "AutoSub.SP.Hwcloud"

#define PROP_HWCLOUD_PROJID "autosub_filter_hwcloud_proj_id"
#define T_PROJECT_ID obs_module_text("AutoSub.ProjectId")

#define PROP_HWCLOUD_TOKEN "autosub_filter_hwcloud_token"


void HwCloudRASRBuilder::getProperties(obs_properties_t *props){
	obs_properties_add_text(props, PROP_HWCLOUD_PROJID, T_PROJECT_ID,
				    OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_HWCLOUD_TOKEN, T_TOKEN,
				    OBS_TEXT_MULTILINE);
}

void HwCloudRASRBuilder::showProperties(obs_properties_t *props){
    PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_PROJID);
    PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_TOKEN);
}

void HwCloudRASRBuilder::hideProperties(obs_properties_t *props){
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_PROJID);
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_TOKEN);
}

void HwCloudRASRBuilder::updateSettings(obs_data_t *settings){
    QString _project_id = obs_data_get_string(settings, PROP_HWCLOUD_PROJID);
	QString _token = obs_data_get_string(settings, PROP_HWCLOUD_TOKEN);

    CHECK_CHANGE_SET_ALL(this->project_id, _project_id, needBuild);
    CHECK_CHANGE_SET_ALL(this->token, _token, needBuild);
}

void HwCloudRASRBuilder::getDefaults(obs_data_t *settings){
    (void) settings;
}

ASRBase *HwCloudRASRBuilder::build(){
    if(!needBuild) {
        return nullptr;
    }
    needBuild = false;
    auto asr = new HwCloudRASR(project_id, token);
    return asr;
}

static HwCloudRASRBuilder hwCloudRASRBuilder; 
static ASRBuilderRegister register_hwcloud_asr(&hwCloudRASRBuilder, HWCLOUD_PROVIDER_ID, L_SP_HWCLOUD);
