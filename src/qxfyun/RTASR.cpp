//
// Created by Yibai Zhang on 2020/6/10.
//
#include <time.h>
#include "RTASR.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <functional>
#include <utility>

using namespace std::placeholders;

static QString hmacSha1(const QByteArray &key, const QByteArray &baseString);

RTASR::RTASR(const QString &appId, const QString &apiKey, QObject *parent)  : QObject(parent), appId(appId), apiKey(apiKey){
    qRegisterMetaType<ResultCallback>("ResultCallback");
    connect(this, &RTASR::start, this, &RTASR::onStart);
    connect(&ws, &QWebSocket::connected, this, &RTASR::onConnected);
    connect(&ws, &QWebSocket::disconnected, this, &RTASR::onDisconnected);
    connect(&ws, &QWebSocket::textMessageReceived, this, &RTASR::onTextMessageReceived);
    connect(this, &RTASR::sendAudioMessage, this, &RTASR::onSendAudioMessage);
    connect(this, &RTASR::haveResult, this, &RTASR::onResult);

    running = false;
}

void RTASR::onStart(){

    QUrl url(buildQuery());
    ws.open(url);
}

void RTASR::onConnected() {
    running = true;
    qDebug() << "WebSocket connected";
}

void RTASR::onDisconnected() {
    running = false;
    qDebug() << "WebSocket disconnected";

}


void RTASR::onSendAudioMessage(const void *data, unsigned long size){
    if(! running){
        return;
    }
    ws.sendBinaryMessage(QByteArray::fromRawData((const char*)data, size));
}


void RTASR::onTextMessageReceived(const QString message) {
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
    emit onResult(output, typeStr.toInt());
}

void RTASR::setResultCallback(ResultCallback cb) {
    callback = std::move(cb);
}

void RTASR::onResult(QString message, int type) {
    if(callback) callback(message, type);
}

void RTASR::stop() {
    ws.sendBinaryMessage(QString(XFYUN_RTASR_GOODBYE).toUtf8());
    ws.close();
}

QUrl RTASR::buildQuery() {
    QString ts;
    QString signa;
    ts = QString::number(time(NULL));
    QString baseString = appId + ts;
    QString signData = QCryptographicHash::hash(baseString.toLocal8Bit(), QCryptographicHash::Md5).toHex();
    signa = hmacSha1(apiKey.toLocal8Bit(), signData.toLocal8Bit());
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

QString RTASR::getAppId() {
    return appId;
}

QString RTASR::getApiKey() {
    return apiKey;
}


static QString hmacSha1(const QByteArray &inkey, const QByteArray &baseString)
{
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    QByteArray key = inkey;
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "quot;
    // ascii characters 0x36 ("6") and 0x5c ("quot;) are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

// result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

RTASR::~RTASR() {
    stop();
}