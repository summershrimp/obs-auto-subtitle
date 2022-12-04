/*
obs-auto-subtitle
 Copyright (C) 2019-2022 Yibai Zhang

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

#include "YiTuASR.h"
#include <util/platform.h>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <time.h>

YiTuASR::YiTuASR(const QString &devId, const QString &devKey, QObject *parent) : ASRBase(parent), devId(devId), devKey(devKey) {}

YiTuASR::~YiTuASR(){
}

void YiTuASR::setLang(QString _lang){
    lang = _lang;
}

void YiTuASR::setEnablePunc(bool enable){
    punc = enable;
}

void YiTuASR::setEnableNOA(bool enable){
    noa = enable;
}

void YiTuASR::onSendAudioMessage(const char *data, unsigned long size){
    if(client){
        client->sendBuffer(data, size);
    }
}

void YiTuASR::callback(YiTuASR *that, std::string result, bool isFinal){
    auto cb = that->getResultCallback();
    if(cb) {
        cb(QString::fromStdString(result), isFinal);
    }
}

void YiTuASR::onStart() {
    uint64_t ts = time(NULL);
    auto id_ts = devId + QString::number(ts);
    auto signature = QMessageAuthenticationCode::hash(id_ts.toLatin1(), devKey.toLatin1(), QCryptographicHash::Sha256).toHex();
    auto x_api_key = devId + "," + QString::number(ts) + "," + signature;
    auto c = new SpeechRecognitionClient("stream-asr-prod.yitutech.com:50051", x_api_key.toStdString());
    c->setLang(lang.toStdString());
    c->setEnablePunc(punc);
    c->setEnableNOA(noa);
    c->setCallback(std::bind(YiTuASR::callback, this, std::placeholders::_1, std::placeholders::_2));
    client.reset(c);
    client->start();
    connect(this, &ASRBase::sendAudioMessage, this,
		&YiTuASR::onSendAudioMessage);
}

void YiTuASR::onStop() {
    if(client)
        client->stop();
}
