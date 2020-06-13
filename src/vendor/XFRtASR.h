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

#ifndef OBS_AUTO_SUBTITLE_XFRTASR_H
#define OBS_AUTO_SUBTITLE_XFRTASR_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>

#include "ASRBase.h"

#define XFYUN_RTASR_URL "ws://rtasr.xfyun.cn/v1/ws"
#define XFYUN_RTASR_GOODBYE "{\"end\": true}"

class XFRtASR : public ASRBase {
    Q_OBJECT
public:
    XFRtASR(const QString &appId, const QString &apiKey, QObject *parent = nullptr);
    QString getAppId();
    QString getApiKey();

    ~XFRtASR();

signals:
    void textMessageReceived(const QString message);
    void haveResult(QString message, int type);

private slots:
    void onStart();
    void onStop();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString message);
    void onSendAudioMessage(const void *, unsigned long);
    void onResult(QString message, int type);
    void onError(QAbstractSocket::SocketError error);

private:
    QString appId;
    QString apiKey;
    QWebSocket ws;
    bool running;
    QUrl buildQuery();

};

#endif //OBS_AUTO_SUBTITLE_XFRTASR_H
