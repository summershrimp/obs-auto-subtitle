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

#include <obs-module.h>
#include <util/platform.h>

#include "obs-auto-subtitle.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Yibai Zhang")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-auto-subtitle", "zh-CN")

extern struct obs_source_info create_autosub_filter_info();
struct obs_source_info autosub_filter_info;

bool obs_module_load(void)
{
    autosub_filter_info = create_autosub_filter_info();
    obs_register_source(&autosub_filter_info);

	return true;
}

void obs_module_unload()
{
	blog(LOG_INFO, "goodbye !");
}

const char* obs_module_name()
{
	return "obs-auto-subtitle";
}

const char* obs_module_description()
{
	return "Online STT to Subtitle plugin for OBS Studio";
}
