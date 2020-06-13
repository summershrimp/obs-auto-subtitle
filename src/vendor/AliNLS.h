//
// Created by Yibai Zhang on 2020/6/10.
//

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
    void onSendAudioMessage(const void *, unsigned long);
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
