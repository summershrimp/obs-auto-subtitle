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

#ifndef OBS_AUTO_SUBTITLE_GSTRANS_H
#define OBS_AUTO_SUBTITLE_GSTRANS_H

#define GS_ENDPOINT_URL "https://script.google.com/macros/s/%1/exec"

#include <QtNetwork>
#include "TransBase.h"

#define TRANS_ENDPOINT "https://itrans.xfyun.cn/v2/its"
#define NIUTRANS_ENDPOINT "https://ntrans.xfyun.cn/v2/ots"

class GScriptTrans : public TransBase {
public:
	GScriptTrans(QString deployId, QObject *parent = nullptr);
	~GScriptTrans();
public slots:
	void onResult(QNetworkReply *rep);
	void onRequestTranslate(QString, QString, QString);

private:
	QString deployId;
	QNetworkAccessManager manager;
};

#endif