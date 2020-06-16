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
    auto cb = getErrorCallback();
    if(cb)
        cb(ERROR_SOCKET, ws.errorString());
    qDebug()<< ws.errorString();
}

void XFRtASR::onConnected() {
    running = true;
    auto cb = getConnectedCallback();
    if (cb)
        cb();
    qDebug() << "WebSocket connected";
}

void XFRtASR::onDisconnected() {
    running = false;
    auto cb = getDisconnectedCallback();
    if(cb)
        cb();
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
        if(doc["action"].toString() == "error"){
            auto errorCb = getErrorCallback();
            if(errorCb)
                errorCb(ERROR_API, doc["desc"].toString());
        }
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
        auto ws = i.toObject().value("ws");
        if(!ws.isArray()) {
            return;
        }

        for(auto w: ws.toArray()){
            auto cw = w.toObject().value("cw");
            if(!cw.isArray()){
                return;
            }
            for(auto c: cw.toArray()){
                output += c.toObject().value("w").toString();
            }
        }
    }
    emit haveResult(output, typeStr.toInt());
}

void XFRtASR::onResult(QString message, int type) {
    auto callback = getResultCallback();
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
