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

#ifndef OBS_AUTOSUB_TRANS_BUILDER_BASE_H
#define OBS_AUTOSUB_TRANS_BUILDER_BASE_H

#include "PropBuilderBase.h"
#include "../../vendor/Trans/TransBase.h"

class TransBuilderBase: public PropBuilderBase<TransBase> { 
public:
    QString getFromLang() {
        return fromLang;
    }
    QString getToLang() {
        return toLang;
    }
protected:
    QString fromLang;
    QString toLang;
};

#define _PROP(name) "autosub_filter_trans_" # name

struct LangList {
    const char *lang_id;
    const char *lang_t;
};

#define TransBuilders (*BuilderRegister<TransBase>::builders)

typedef BuilderRegister<TransBase> TransBuilderRegister;
#endif