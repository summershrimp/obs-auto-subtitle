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

#ifndef OBS_AUTO_SUBTITLE_HW_RASR_H
#define OBS_AUTO_SUBTITLE_HW_RASR_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>

#include "ASRBase.h"

#define HWCLOUD_SIS_ENDPOINT "sis-ext.cn-north-4.myhuaweicloud.com"
#define HWCLOUD_SIS_RASR_URI "/v1/%1/rasr/continue-stream"

class HwCloudRASR : public ASRBase {
	Q_OBJECT
public:
	HwCloudRASR(const QString &project_id, const QString &token,
		    QObject *parent = nullptr);
	QString getProjectId();
	QString getToken();

	~HwCloudRASR();

signals:
	void textMessageReceived(const QString message);
	void haveResult(QString message, int type);

private slots:
	void onStart();
	void onStop();
	void onConnected();
	void onDisconnected();
	void onTextMessageReceived(const QString message);
	void onSendAudioMessage(const char *, unsigned long);
	void onResult(QString message, int type);
	void onError(QAbstractSocket::SocketError error);

private:
	QString project_id;
	QString token;
	QWebSocket ws;
	bool running;
};

#endif //OBS_AUTO_SUBTITLE_HW_RASR_H
