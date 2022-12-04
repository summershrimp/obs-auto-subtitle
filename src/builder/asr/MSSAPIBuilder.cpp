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
#include <string>

#include "MSSAPIBuilder.h"

#define MSSAPI_PROVIDER_ID 0x0004U
#define L_SP_MSSAPI "AutoSub.SP.MSSAPI"

#define PROP_MSSAPI_LANG _PROP("mssapi_lang")
#define T_MSSAPI_CURRENT_SYSTEM_LANG "AutoSub.MSSAPI.CurrentSystemLanguage"
static bool get_locale_name(LANGID id, char *out)
{
	wchar_t name[256];

	int size = GetLocaleInfoW(id, LOCALE_SENGLISHLANGUAGENAME, name, 256);
	if (size <= 0)
		return false;

	os_wcs_to_utf8(name, 0, out, 256);
	return true;
}

static bool valid_lang(LANGID id)
{
	ComPtr<ISpObjectToken> token;
	wchar_t lang_str[32];
	HRESULT hr;

	_snwprintf(lang_str, 31, L"language=%x", (int)id);

	hr = SpFindBestToken(SPCAT_RECOGNIZERS, lang_str, nullptr, &token);
	return SUCCEEDED(hr);
}

static void get_valid_locale_names(std::vector<locale_info> &locales)
{
	locale_info cur;
	char locale_name[256];

	static const LANGID default_locales[] = {
		0x0409, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406,
		0x0407, 0x0408, 0x040a, 0x040b, 0x040c, 0x040d, 0x040e,
		0x040f, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415,
		0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0};

	/* ---------------------------------- */

	LANGID def_id = GetUserDefaultUILanguage();
	LANGID id = def_id;
	if (valid_lang(id) && get_locale_name(id, locale_name)) {
		dstr_copy(cur.name,
			  obs_module_text(T_MSSAPI_CURRENT_SYSTEM_LANG));
		dstr_replace(cur.name, "%1", locale_name);
		cur.id = id;

		locales.push_back(std::move(cur));
	}

	/* ---------------------------------- */

	const LANGID *locale = default_locales;

	while (*locale) {
		id = *locale;

		if (id != def_id && valid_lang(id) &&
		    get_locale_name(id, locale_name)) {

			dstr_copy(cur.name, locale_name);
			cur.id = id;

			locales.push_back(std::move(cur));
		}

		locale++;
	}
}

MSSAPIBuilder::MSSAPIBuilder(){
    get_valid_locale_names(locales);
}

void MSSAPIBuilder::getProperties(obs_properties_t *props){
    auto t = obs_properties_add_list(props, PROP_MSSAPI_LANG, T_LANGUAGE,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_INT);
    for(auto &i: locales) {
        obs_property_list_add_int(t, i.name, i.id);
    }
}

void MSSAPIBuilder::showProperties(obs_properties_t *props){
    PROPERTY_SET_VISIBLE(props, PROP_MSSAPI_LANG);
}

void MSSAPIBuilder::hideProperties(obs_properties_t *props){
    PROPERTY_SET_UNVISIBLE(props, PROP_MSSAPI_LANG);
}

void MSSAPIBuilder::updateSettings(obs_data_t *settings){
    int _lang_id = obs_data_get_int(settings, PROP_MSSAPI_LANG);
    CHECK_CHANGE_SET_ALL(this->lang_id, _lang_id, needBuild);
}

void MSSAPIBuilder::getDefaults(obs_data_t *settings){
    (void) settings;
}

ASRBase *MSSAPIBuilder::build(){
    if(!needBuild) {
        return nullptr;
    }
    needBuild = false;
    wchar_t wname[256];
    if (!LCIDToLocaleName(lang_id, wname, 256, 0)) {
        blog(LOG_WARNING, "Failed to get locale name: %d",
                (int)GetLastError());
        return nullptr;
    }
    size_t len = (size_t)wcslen(wname);

    std::string lang_name;
    lang_name.resize(len);

    for (size_t i = 0; i < len; i++)
        lang_name[i] = (char)wname[i];

    try {
        auto asr = new MSSAPI(lang_name.c_str());
        return asr;
    } catch (std::string text) {
        blog(LOG_WARNING, "Failed to create handler: %s", text.c_str());
    }
    return nullptr;
}

static MSSAPIBuilder mssapiBuilder; 
static ASRBuilderRegister register_mssapi_asr( &mssapiBuilder, MSSAPI_PROVIDER_ID, L_SP_MSSAPI);
