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

#ifndef OBS_AUTOSUB_PROP_BUILDER_BASE_H
#define OBS_AUTOSUB_PROP_BUILDER_BASE_H

#include <obs.h>
#include <obs-module.h>

template<class T>
class PropBuilderBase {
public:
    virtual void getProperties(obs_properties_t *props) = 0;
    virtual void showProperties(obs_properties_t *props) = 0;
    virtual void hideProperties(obs_properties_t *props) = 0;
    virtual void updateSettings(obs_data_t *settings) = 0;
    virtual void getDefaults(obs_data_t *settings) = 0;
    virtual T* build() = 0;
};

#define _T(name) obs_module_text("AutoSub." # name)
#define _TKEY(name) "AutoSub." # name

#define PROPERTY_SET_UNVISIBLE(props, prop_name) \
    do {\
        obs_property *prop = obs_properties_get(props, prop_name);\
        obs_property_set_visible(prop, false);\
    }while(0)

#define PROPERTY_SET_VISIBLE(props, prop_name) \
    do {\
        obs_property *prop = obs_properties_get(props, prop_name);\
        obs_property_set_visible(prop, true);\
    }while(0)

#endif
