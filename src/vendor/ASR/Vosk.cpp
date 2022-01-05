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
#include <QString>
#include <QDebug>

#include "Vosk.h"
#include "vosk/vosk_api_wrapper.h"

Vosk::Vosk(const QString &modelPath, const QString &spkModelPath, QObject *parent)
    : ASRBase(parent), modelPath(modelPath), spkModelPath(spkModelPath) {
        running = false;
        model = vosk_model_new(modelPath.toStdString().c_str());
        recognizer = vosk_recognizer_new(model, 16000);
        spkModel = nullptr;
        if(!spkModelPath.isEmpty()){
            spkModel = vosk_spk_model_new(spkModelPath.toStdString().c_str());
            vosk_recognizer_set_spk_model(recognizer, spkModel);
        }
        connect(this, &ASRBase::sendAudioMessage, this, &Vosk::onAudioMessage);
}

Vosk::~Vosk(){
    vosk_recognizer_free(recognizer);
    vosk_spk_model_free(spkModel);
    vosk_model_free(model);
    recognizer = nullptr;
    spkModel = nullptr;
    model = nullptr;
}

void Vosk::onStart() {
    runlock.lock();
    running = true;
    runlock.unlock();
}

void Vosk::onStop() {
    runlock.lock();
    running = false;
    runlock.unlock();
}

void Vosk::onAudioMessage(const void *data, unsigned long len){
    int final;
    QString result;
    final = vosk_recognizer_accept_waveform(recognizer, (char *)data, len);
    if (final) {
        result = vosk_recognizer_result(recognizer);
    } else {
        result = vosk_recognizer_partial_result(recognizer);
    }
    qDebug()<<result;
    getResultCallback()(result, final);
}