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

#include "GSTransBuilder.h"

static LangList _langList[] = {
    {"",  _TKEY(Trans.Auto)},
    {"zh-cn",  _TKEY(Trans.Chinese)},
    {"en",  _TKEY(Trans.English)},
    {"ja",  _TKEY(Trans.Japanese)},
    {"ru",  _TKEY(Trans.Russian)},
    {"fr",  _TKEY(Trans.Frenche)},
    {"es",  _TKEY(Trans.Spanish)},
    {"ar",  _TKEY(Trans.Arabic)},
    {"th",  _TKEY(Trans.Thai)},
    {nullptr, nullptr}
};

const LangList *GSTransBuilder::langList = _langList;

void GSTransBuilder::getProperties(obs_properties_t *props) {
    obs_property_t *t;
    obs_properties_add_text(props, PROP_TRANS_GS_DEPLOYID, _T(Trans.GS.DeployId), OBS_TEXT_DEFAULT);

    t = obs_properties_add_list(props, PROP_TRANS_GS_FROMLANG, _T(Trans.FromLang), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    const LangList *p = GSTransBuilder::langList;
    while(p->lang_id != nullptr){
        obs_property_list_add_string(t, obs_module_text(p->lang_t), p->lang_id);
        ++p;
    }

    t = obs_properties_add_list(props, PROP_TRANS_GS_TOLANG, _T(Trans.ToLang), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    p = GSTransBuilder::langList + 1;
    while(p->lang_id != nullptr){
        obs_property_list_add_string(t, obs_module_text(p->lang_t), p->lang_id);
        ++p;
    }

}
void GSTransBuilder::showProperties(obs_properties_t *props) {
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_GS_DEPLOYID);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_GS_FROMLANG);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_GS_TOLANG);
}
void GSTransBuilder::hideProperties(obs_properties_t *props) {
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_GS_DEPLOYID);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_GS_FROMLANG);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_GS_TOLANG);
}

#define CHECK_CHANGE_SET_ALL(dst, src, changed) \
do { \
    if (dst != src) { \
        changed = true; \
        dst = src; \
    } \
} while(0) \

void GSTransBuilder::updateSettings(obs_data_t *settings) {
    QString deployId, apiKey, apiSecret;
    deployId = obs_data_get_string(settings, PROP_TRANS_GS_DEPLOYID);
    CHECK_CHANGE_SET_ALL(this->deployId, deployId, needBuild);
    fromLang = obs_data_get_string(settings, PROP_TRANS_GS_FROMLANG);
    toLang = obs_data_get_string(settings, PROP_TRANS_GS_TOLANG);
}

void GSTransBuilder::getDefaults(obs_data_t *settings) {
    obs_data_set_default_string(settings, PROP_TRANS_GS_DEPLOYID, "");
}

TransBase *GSTransBuilder::build() {
    if (!needBuild){
        return nullptr;
    }
    needBuild = false;
    return new GScriptTrans(deployId);
}
