//
// Created by Yibai Zhang on 2020/6/10.
//
#include <time.h>
#include <QDebug>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <functional>
#include <utility>

#include "XFRtASR.h"

using namespace std::placeholders;

XFRtASR::XFRtASR(const QString &appId, const QString &apiKey, QObject *parent)  : ASRBase(parent), appId(appId), apiKey(apiKey){
    connect(&ws, &QWebSocket::connected, this, &XFRtASR::onConnected);
    connect(&ws, &QWebSocket::disconnected, this, &XFRtASR::onDisconnected);
    connect(&ws, &QWebSocket::textMessageReceived, this, &XFRtASR::onTextMessageReceived);
    connect(this, &ASRBase::sendAudioMessage, this, &XFRtASR::onSendAudioMessage);
    connect(this, &XFRtASR::haveResult, this, &XFRtASR::onResult);
    connect(&ws, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    running = false;
}

void XFRtASR::onStart(){
    QUrl url(buildQuery());
    ws.open(url);
}

void XFRtASR::onError(QAbstractSocket::SocketError error) {
    qDebug()<< ws.errorString();
    qDebug() << error;
}

void XFRtASR::onConnected() {
    running = true;
    qDebug() << "WebSocket connected";
}

void XFRtASR::onDisconnected() {
    running = false;
    qDebug() << "WebSocket disconnected";

}


void XFRtASR::onSendAudioMessage(const void *data, unsigned long size){
    if(! running){
        return;
    }
    ws.sendBinaryMessage(QByteArray::fromRawData((const char*)data, size));
}


void XFRtASR::onTextMessageReceived(const QString message) {
    QJsonDocument doc(QJsonDocument::fromJson(message.toUtf8()));
    if(doc["action"].toString() != "result" || doc["code"].toString() != "0") {
        qDebug() << message;
        return;
    }
    auto data = doc["data"];
    if(!data.isString()) {
        return;
    }
    QJsonDocument dataDoc(QJsonDocument::fromJson(data.toString().toUtf8()));
    auto cn = dataDoc["cn"];
    if(!cn.isObject()) {
        return;
    }
    QString output;
    auto type = cn["st"]["type"];
    if(!type.isString()) {
        return;
    }
    auto typeStr = type.toString();
    auto rt = cn["st"]["rt"];
    if(!rt.isArray()){
        return;
    }
    for(auto i: rt.toArray()){
        auto ws = i.toObject()["ws"];
        if(!ws.isArray()) {
            return;
        }
        for(auto w: ws.toArray()){
            auto cw = w.toObject()["cw"];
            if(!cw.isArray()){
                return;
            }
            for(auto c: cw.toArray()){
                if(!c.toObject()["w"].isString()) {
                    return;
                }
                output += c.toObject()["w"].toString();
            }
        }
    }
    emit haveResult(output, typeStr.toInt());
}

void XFRtASR::onResult(QString message, int type) {
    auto callback = getCallback();
    if(callback)
        callback(message, type);
}


void XFRtASR::onStop() {
    ws.sendBinaryMessage(QString(XFYUN_RTASR_GOODBYE).toUtf8());
    ws.close();
}

QUrl XFRtASR::buildQuery() {
    QString ts;
    QString signa;
    ts = QString::number(time(NULL));
    QString baseString = appId + ts;
    QString signData = QCryptographicHash::hash(baseString.toLocal8Bit(), QCryptographicHash::Md5).toHex();
    qDebug() <<signData <<apiKey;
    signa = QMessageAuthenticationCode::hash(signData.toLocal8Bit(), apiKey.toLocal8Bit(), QCryptographicHash::Sha1)
                .toBase64();
    QUrl url(XFYUN_RTASR_URL);
    QUrlQuery query;
    query.addQueryItem("appid", appId);
    query.addQueryItem("ts", ts);
    query.addQueryItem("signa", signa);
    query.addQueryItem("punc", "0");
    query.addQueryItem("pd", "tech");
    url.setQuery(query);
    qDebug() << url;
    return url;
}

QString XFRtASR::getAppId() {
    return appId;
}

QString XFRtASR::getApiKey() {
    return apiKey;
}


XFRtASR::~XFRtASR() {
    emit stop();
}
