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

#ifndef OBS_AUTO_SUBTITLE_ASRBASE_H
#define OBS_AUTO_SUBTITLE_ASRBASE_H
#include <functional>
#include <QString>
#include <QObject>

typedef std::function<void(QString, int)> ResultCallback;

enum ResultType {
    ResultType_End = 0,
    ResultType_Middle

};

class ASRBase : public QObject {
    Q_OBJECT
public:
    ASRBase(QObject *parent);
    void setResultCallback(ResultCallback cb);
    virtual ~ASRBase(){};

protected:
    ResultCallback getCallback();
signals:
    void sendAudioMessage(const void *, unsigned long);
    void start();
    void stop();

public slots:
    virtual void onStart() = 0;
    virtual void onStop() = 0;
private:
    ResultCallback cb;
};


#endif //OBS_AUTO_SUBTITLE_ASRBASE_H
