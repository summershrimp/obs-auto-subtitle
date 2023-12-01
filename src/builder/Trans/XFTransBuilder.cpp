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

#include "XFTransBuilder.h"

#define XF_TRANS_PROVIDER_ID 0x1002U
#define XF_TRANS_NIU_PROVIDER_ID 0x1003U

#define L_TRANS_SP_XFYUN "AutoSub.Trans.SP.Xfyun"
#define L_TRANS_SP_XFNIU "AutoSub.Trans.SP.XfyunNiu"

#define PROP_TRANS_XF_APPID _PROP("xf_appid")
#define PROP_TRANS_XF_APIKEY _PROP("xf_apikey")
#define PROP_TRANS_XF_APISECRET _PROP("xf_apisecret")
#define PROP_TRANS_XF_FROMLANG _PROP("xf_fromlang")
#define PROP_TRANS_XF_TOLANG _PROP("xf_tolang")

static LangList _langList[] = {
    {"cn",  _TKEY(Trans.Chinese)},
    {"en",  _TKEY(Trans.English)},
    {"yue", _TKEY(Trans.Cantonese)},
    {"ja",  _TKEY(Trans.Japanese)},
    {"ru",  _TKEY(Trans.Russian)},
    {"fr",  _TKEY(Trans.Frenche)},
    {"es",  _TKEY(Trans.Spanish)},
    {"ar",  _TKEY(Trans.Arabic)},
    {"th",  _TKEY(Trans.Thai)},
    {"sw",  _TKEY(Trans.Swahili)},
    {nullptr, nullptr}
};

const LangList *XFTransBuilder::langList = _langList;


void XFTransBuilder::getProperties(obs_properties_t *props) {
    obs_property_t *t;
    obs_properties_add_text(props, PROP_TRANS_XF_APPID, _T(APPID), OBS_TEXT_DEFAULT);
    obs_properties_add_text(props, PROP_TRANS_XF_APISECRET, _T(APISECRET), OBS_TEXT_DEFAULT);
    obs_properties_add_text(props, PROP_TRANS_XF_APIKEY, _T(APIKEY), OBS_TEXT_DEFAULT);

    t = obs_properties_add_list(props, PROP_TRANS_XF_FROMLANG, _T(Trans.FromLang), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    const LangList *p = XFTransBuilder::langList;
    while(p->lang_id != nullptr){
        obs_property_list_add_string(t, obs_module_text(p->lang_t), p->lang_id);
        ++p;
    }

    t = obs_properties_add_list(props, PROP_TRANS_XF_TOLANG, _T(Trans.ToLang), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    p = XFTransBuilder::langList;
    while(p->lang_id != nullptr){
        obs_property_list_add_string(t, obs_module_text(p->lang_t), p->lang_id);
        ++p;
    }

}

void XFTransBuilder::showProperties(obs_properties_t *props) {
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_XF_APPID);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_XF_APISECRET);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_XF_APIKEY);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_XF_FROMLANG);
    PROPERTY_SET_VISIBLE(props, PROP_TRANS_XF_TOLANG);
}

void XFTransBuilder::hideProperties(obs_properties_t *props) {
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_XF_APPID);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_XF_APISECRET);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_XF_APIKEY);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_XF_FROMLANG);
    PROPERTY_SET_UNVISIBLE(props, PROP_TRANS_XF_TOLANG);
}

void XFTransBuilder::updateSettings(obs_data_t *settings) {
    QString _appId, _apiKey, _apiSecret;
    _appId = obs_data_get_string(settings, PROP_TRANS_XF_APPID);
    _apiKey = obs_data_get_string(settings, PROP_TRANS_XF_APIKEY);
    _apiSecret = obs_data_get_string(settings, PROP_TRANS_XF_APISECRET);
    CHECK_CHANGE_SET_ALL(this->appId, _appId, needBuild);
    CHECK_CHANGE_SET_ALL(this->apiKey, _apiKey, needBuild);
    CHECK_CHANGE_SET_ALL(this->apiSecret, _apiSecret, needBuild);
    fromLang = obs_data_get_string(settings, PROP_TRANS_XF_FROMLANG);
    toLang = obs_data_get_string(settings, PROP_TRANS_XF_TOLANG);
}

void XFTransBuilder::getDefaults(obs_data_t *settings) {
    obs_data_set_default_string(settings, PROP_TRANS_XF_APPID, "");
    obs_data_set_default_string(settings, PROP_TRANS_XF_APISECRET, "");
    obs_data_set_default_string(settings, PROP_TRANS_XF_APIKEY, "");
}

TransBase *XFTransBuilder::build() {
    if (!needBuild){
        return nullptr;
    }
    needBuild = false;
    if (isNiuTrans){
        return new XFTrans(appId, apiKey, apiSecret, NIUTRANS_ENDPOINT);
    } else {
        return new XFTrans(appId, apiKey, apiSecret, TRANS_ENDPOINT);
    }
}


static XFTransBuilder xfTransBuilder(false); 
static TransBuilderRegister register_xf_trans(&xfTransBuilder, XF_TRANS_PROVIDER_ID, L_TRANS_SP_XFYUN);

static XFTransBuilder xfNiuTransBuilder(true); 
static TransBuilderRegister register_xf_niu_trans(&xfNiuTransBuilder, XF_TRANS_NIU_PROVIDER_ID, L_TRANS_SP_XFNIU);
