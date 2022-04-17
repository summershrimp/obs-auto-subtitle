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

#ifndef OBS_AUTOSUB_GSTRANS_BUILDER_H
#define OBS_AUTOSUB_GSTRANS_BUILDER_H

#include <QString>

#include "TransBuilderBase.h"
#include "../vendor/Trans/GScriptTrans.h"



class GSTransBuilder : public TransBuilderBase {
public:
    void getProperties(obs_properties_t *props);
    void showProperties(obs_properties_t *props);
    void hideProperties(obs_properties_t *props);
    void updateSettings(obs_data_t *settings);
    void getDefaults(obs_data_t *settings);
    TransBase *build();

protected:
    static const LangList *langList;

private:
    QString deployId;
    bool needBuild = false;
};

#define PROP_TRANS_GS_DEPLOYID _PROP("gs_deployid")
#define PROP_TRANS_GS_FROMLANG _PROP("gs_fromlang")
#define PROP_TRANS_GS_TOLANG _PROP("gs_tolang")

#endif

