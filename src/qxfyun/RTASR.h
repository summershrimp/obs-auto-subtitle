//
// Created by Yibai Zhang on 2020/6/10.
//

#ifndef OBS_AUTO_SUBTITLE_RTASR_H
#define OBS_AUTO_SUBTITLE_RTASR_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>
#include <functional>

#define XFYUN_RTASR_URL "wss://rtasr.xfyun.cn/v1/ws"
#define XFYUN_RTASR_GOODBYE "{\"end\": true}"


typedef std::function<void(QString, int)> ResultCallback;

enum ResultType {
    ResultType_End = 0,
    ResultType_Middle

};

class RTASR : public QObject {
    Q_OBJECT
public:
    RTASR(const QString &appId, const QString &apiKey, QObject *parent = nullptr);
    void setResultCallback(ResultCallback cb);
    void stop();
    QString getAppId();
    QString getApiKey();

    ~RTASR();

signals:
    void start();
    void sendAudioMessage(const void *, unsigned long);
    void textMessageReceived(const QString message);
    void haveResult(QString message, int type);

private slots:
    void onStart();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString message);
    void onSendAudioMessage(const void *, unsigned long);
    void onResult(QString message, int type);

private:
    QString appId;
    QString apiKey;
    QWebSocket ws;
    ResultCallback callback;
    bool running;
    QUrl buildQuery();

};


#endif //OBS_AUTO_SUBTITLE_RTASR_H
