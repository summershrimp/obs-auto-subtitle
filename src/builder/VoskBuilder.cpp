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

#include "VoskBuilder.h"
#include "../vendor/ASR/Vosk.h"

void VoskBuilder::getProperties(obs_properties_t *props) {
    obs_properties_add_path(props, PROP_SPEECH_VOSK_MODELPATH, _T(Vosk.ModelPath), OBS_PATH_DIRECTORY, nullptr, nullptr);
    obs_properties_add_path(props, PROP_SPEECH_VOSK_SPKMODELPATH, _T(Vosk.SpkModelPath), OBS_PATH_DIRECTORY, nullptr, nullptr);
    obs_properties_add_bool(props, PROP_SPEECH_VOSK_ENABLESPK, _T(Vosk.SpkEnabled));
}

void VoskBuilder::showProperties(obs_properties_t *props) {
    PROPERTY_SET_VISIBLE(props, PROP_SPEECH_VOSK_MODELPATH);
    PROPERTY_SET_VISIBLE(props, PROP_SPEECH_VOSK_SPKMODELPATH);
    PROPERTY_SET_VISIBLE(props, PROP_SPEECH_VOSK_ENABLESPK);
}
void VoskBuilder::hideProperties(obs_properties_t *props) {
    PROPERTY_SET_UNVISIBLE(props, PROP_SPEECH_VOSK_MODELPATH);
    PROPERTY_SET_UNVISIBLE(props, PROP_SPEECH_VOSK_SPKMODELPATH);
    PROPERTY_SET_UNVISIBLE(props, PROP_SPEECH_VOSK_ENABLESPK);
}

#define CHECK_CHANGE_SET_ALL(dst, src, changed) \
do { \
    if (dst != src) { \
        changed = true; \
        dst = src; \
    } \
} while(0) \

void VoskBuilder::updateSettings(obs_data_t *settings) {
    QString modelPath, spkModelPath;
    bool enableSpkModel;
    modelPath = obs_data_get_string(settings, PROP_SPEECH_VOSK_MODELPATH);
    spkModelPath = obs_data_get_string(settings, PROP_SPEECH_VOSK_SPKMODELPATH);
    enableSpkModel = obs_data_get_bool(settings, PROP_SPEECH_VOSK_ENABLESPK);
    CHECK_CHANGE_SET_ALL(this->modelPath, modelPath, needBuild);
    CHECK_CHANGE_SET_ALL(this->spkModelPath, spkModelPath, needBuild);
    CHECK_CHANGE_SET_ALL(this->enableSpkModel, enableSpkModel, needBuild);
}

void VoskBuilder::getDefaults(obs_data_t *settings) {
    obs_data_set_default_bool(settings, PROP_SPEECH_VOSK_ENABLESPK, false);
}

ASRBase *VoskBuilder::build() {
    blog(LOG_INFO, "VOSK model:%s, spkModel: %s", modelPath.toStdString().c_str(), spkModelPath.toStdString().c_str());
    if(modelPath.isEmpty())
        return nullptr;
    if(enableSpkModel)
        return new Vosk(modelPath, spkModelPath);
    return new Vosk(modelPath, "");
}
