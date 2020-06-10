//
// Created by Yibai Zhang on 2020/6/10.
//

#ifndef OBS_AUTO_SUBTITLE_RTASR_H
#define OBS_AUTO_SUBTITLE_RTASR_H
#include <QObject>
#include <QString>
#include <memory>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#define XFYUN_RTASR_URL "ws://rtasr.xfyun.cn/v1/ws"
using wsclient = websocketpp::client<websocketpp::config::asio_client>;

class RTASR : public QObject {
    Q_OBJECT
public:
    RTASR(const QString &appId, const QString &apiKey, QObject *parent = nullptr);

    signals:
    void start();
    void sendAudioMessage(const void *, unsigned long);
    void textMessageReceived(const QString message);
private slots:
    void onStart();
    void onConnected();
    void onTextMessageReceived(const QString message);
    void onSendAudioMessage(const void *, unsigned long);

private:
    QString appId;
    QString apiKey;
    websocketpp::client<websocketpp::config::asio_client> client;
    wsclient::connection_ptr conn;
    std::shared_ptr<websocketpp::lib::thread> thread;
    QUrl buildQuery();
    static void onMessage(RTASR *from, websocketpp::connection_hdl hdl, wsclient::message_ptr msg);

};


#endif //OBS_AUTO_SUBTITLE_RTASR_H
