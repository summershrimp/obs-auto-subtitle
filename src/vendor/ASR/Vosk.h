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

#ifndef OBS_AUTO_SUBTITLE_VOSK_H
#define OBS_AUTO_SUBTITLE_VOSK_H
#include <QObject>
#include <QMap>
#include <QString>
#include <memory>
#include <mutex>

#include "ASRBase.h"
#include "vosk/vosk_api_wrapper.h"

class Vosk : public ASRBase {
    Q_OBJECT
public:
    Vosk(const QString &modelPath, const QString &spkModelPath, QObject *parent = nullptr);
    ~Vosk();

private slots:
    void onStart();
    void onStop();
    void onAudioMessage(const void *, unsigned long);

private:
    QString modelPath;
    QString spkModelPath;

    VoskRecognizer *recognizer;
    VoskSpkModel *spkModel;
    VoskModel *model;

    std::mutex runlock;
    bool running;
};


#endif //OBS_AUTO_SUBTITLE_VOSK_H
