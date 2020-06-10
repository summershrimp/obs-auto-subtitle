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

#ifndef OBSSSP_H
#define OBSSSP_H

#include <string>

#ifndef OBS_AUTOSUB_VERSION
#define OBS_AUTOSUB_VERSION "unknown"
#endif

#define blog(level, msg, ...) blog(level, "[obs-autosub] " msg, ##__VA_ARGS__)

#endif // OBSSSP_H
