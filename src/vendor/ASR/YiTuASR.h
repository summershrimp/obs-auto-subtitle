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

#ifndef OBS_AUTO_SUBTITLE_YITUASR_H
#define OBS_AUTO_SUBTITLE_YITUASR_H
#include <QObject>
#include <QString>
#include <QWebSocket>
#include <memory>
#include "yitu/SpeechRecognitionClient.h"
#include "ASRBase.h"

class YiTuASR : public ASRBase {
	Q_OBJECT
public:
	YiTuASR(const QString &devId, const QString &devKey, QObject *parent = nullptr);
	~YiTuASR();
	void setLang(QString _lang);
    void setEnablePunc(bool enable);
    void setEnableNOA(bool enable);

	static void callback(YiTuASR *that, std::string result, bool isFinal);

private slots:
	void onSendAudioMessage(const char *, unsigned long);
	void onStart();
	void onStop();

private:
	QString devId;
    QString devKey;
	QString lang = "";
	bool punc = true;
    bool noa = true;
    std::unique_ptr<SpeechRecognitionClient> client;
};

#endif //OBS_AUTO_SUBTITLE_YITUASR_H
