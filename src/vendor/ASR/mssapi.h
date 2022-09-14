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

#ifndef OBS_AUTO_SUBTITLE_MSSAPI_H
#define OBS_AUTO_SUBTITLE_MSSAPI_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>
#include "mssapi-captions/captions-mssapi.hpp"
#include "ASRBase.h"

#define XFYUN_RTASR_URL "wss://rtasr.xfyun.cn/v1/ws"
#define XFYUN_RTASR_GOODBYE "{\"end\": true}"

class MSSAPI : public ASRBase {
	Q_OBJECT
public:
	MSSAPI(const QString &lang, QObject *parent = nullptr);
	~MSSAPI();
	static void callback(MSSAPI *their, const std::string &result);

private slots:
	void onSendAudioMessage(const char *, unsigned long);
	void onStart();
	void onStop();

private:
	std::shared_ptr<mssapi_captions> mssapi;
	const QString lang;
};

#endif //OBS_AUTO_SUBTITLE_MSSAPI_H
