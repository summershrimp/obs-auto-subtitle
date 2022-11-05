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

#ifndef OBS_AUTO_SUBTITLE_ALINLS_H
#define OBS_AUTO_SUBTITLE_ALINLS_H
#include <QObject>
#include <QMap>
#include <QString>
#include <QWebSocket>
#include <memory>

#include "ASRBase.h"

#define ALINLS_URL "wss://nls-gateway.cn-shanghai.aliyuncs.com/ws/v1"
#define ALINLS_TOKEN_HEADER "X-NLS-Token"

class AliNLS : public ASRBase {
    Q_OBJECT
public:
    AliNLS(const QString &appKey, const QString &token, QObject *parent = nullptr);
    QString getAppKey();
    QString getToken();

    ~AliNLS();

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
    QString appKey;
    QString token;
    QWebSocket ws;
    bool running;
    QHash<QString, QString> _header;
    QHash<QString, QJsonValue> _payload;
    QString serializeReq();
    QString task_id;
};


#endif //OBS_AUTO_SUBTITLE_ALINLS_H
