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

#ifndef OBS_AUTOSUB_MSSAPI_BUILDER_H
#define OBS_AUTOSUB_MSSAPI_BUILDER_H

#include <QString>
#include <vector>
#include <util/dstr.hpp>

#include "../base/ASRBuilderBase.h"
#include "../../vendor/ASR/mssapi.h"

struct locale_info {
	DStr name;
	LANGID id;

	inline locale_info() {}
	inline locale_info(const locale_info &) = delete;
	inline locale_info(locale_info &&li)
		: name(std::move(li.name)), id(li.id)
	{
	}
};

class MSSAPIBuilder : public ASRBuilderBase {
public:
	MSSAPIBuilder();
	void getProperties(obs_properties_t *props);
	void showProperties(obs_properties_t *props);
	void hideProperties(obs_properties_t *props);
	void updateSettings(obs_data_t *settings);
	void getDefaults(obs_data_t *settings);
	ASRBase *build();

protected:
private:
	std::vector<locale_info> locales;
	LANGID lang_id;
	bool needBuild = false;
};

#endif OBS_AUTOSUB_MSSAPI_BUILDER_H