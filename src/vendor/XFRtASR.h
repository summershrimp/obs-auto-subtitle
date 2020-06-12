//
// Created by Yibai Zhang on 2020/6/10.
//

#ifndef OBS_AUTO_SUBTITLE_XFRTASR_H
#define OBS_AUTO_SUBTITLE_XFRTASR_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>

#include "ASRBase.h"

#define XFYUN_RTASR_URL "wss://rtasr.xfyun.cn/v1/ws"
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
