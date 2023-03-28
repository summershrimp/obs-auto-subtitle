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

#include <obs-module.h>
#include <map>
#include <memory>

#include <plugin-macros.generated.h>

#define T_TOKEN obs_module_text("AutoSub.Token")
#define T_ACCEESSKEY obs_module_text("AutoSub.AccessKey")
#define T_SECRET obs_module_text("AutoSub.Secret")
#define T_LANGUAGE obs_module_text("AutoSub.Language")

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

#define PROVIDER_ID_INVALID 0xFFFFFFFFU

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

#define CHECK_CHANGE_SET_ALL(dst, src, changed) \
do { \
    if (dst != src) { \
        changed = true; \
        dst = src; \
    } \
} while(0) \


typedef unsigned int provider_id_t;

template<class T>
class BuilderRegister;

template<class T>
class BuilderHolder {
    friend class BuilderRegister<T>;
public:
    PropBuilderBase<T> *getBuilder(provider_id_t);
    std::map<provider_id_t, PropBuilderBase<T>*> getAllBuilder();
    const char * getLocaleLabel(provider_id_t);
protected:
    void addBuilder(provider_id_t, PropBuilderBase<T> *);
    void addLocaleLabel(provider_id_t, const char* locale_label);
private:
    std::map<provider_id_t, PropBuilderBase<T>*> _builders;
    std::map<provider_id_t, const char*> _localeLabels;
};

template<class T>
class BuilderRegister {
public:
    BuilderRegister(PropBuilderBase<T> *, provider_id_t, const char* locale_label);
    static BuilderHolder<T> *builders;
};

template<class T>
BuilderHolder<T> *BuilderRegister<T>::builders = nullptr;

template<class T>
const char *BuilderHolder<T>::getLocaleLabel(provider_id_t id) {
    auto iter = this->_localeLabels.find(id);
    if(iter == this->_localeLabels.end()) {
        return nullptr;
    }
    return iter->second;
}

template<class T>
void BuilderHolder<T>::addBuilder(provider_id_t id, PropBuilderBase<T> *builder){
    if(builder == nullptr) {
        return;
    }
    this->_builders.insert(std::make_pair(id, builder));
}

template<class T>
void BuilderHolder<T>::addLocaleLabel(provider_id_t id, const char *locale_label) {
    if(locale_label == nullptr) {
        return;
    }
    this->_localeLabels.insert(std::make_pair(id, locale_label));
}

template<class T>
PropBuilderBase<T> *BuilderHolder<T>::getBuilder(provider_id_t id){
    auto iter = this->_builders.find(id);
    if(iter == this->_builders.end()) {
        return nullptr;
    }
    return iter->second;
}

template<class T>
std::map<provider_id_t, PropBuilderBase<T>*> BuilderHolder<T>::getAllBuilder(){
    return this->_builders;
}


template<class T>
BuilderRegister<T>::BuilderRegister(PropBuilderBase<T> *builder, provider_id_t id, const char* locale_label){
    if(!builders) {
        builders = new BuilderHolder<T>();
    }
    builders->addBuilder(id, builder);
    builders->addLocaleLabel(id, locale_label);
}



#endif

