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

#ifndef OBS_AUTO_SUBTITLE_TRANSBASE_H
#define OBS_AUTO_SUBTITLE_TRANSBASE_H
#include <functional>
#include <QString>
#include <QObject>
#include <QMap>

class TransBase : public QObject {
	Q_OBJECT

public:
	typedef std::function<void(QString)> ResultCallback;
	typedef std::function<void(QString)> ErrorCallback;
	TransBase(QObject *parent);
	void setResultCallback(ResultCallback cb);
	void setErrorCallback(ErrorCallback cb);
	void setParam(QString key, QString value);
	virtual ~TransBase(){};

protected:
	void callbackResult(QString);
	void callbackError(QString);
	QMap<QString, QString> params;

signals:
	void requestTranslate(QString, QString, QString);

public slots:
	virtual void onRequestTranslate(QString, QString, QString) = 0;

private:
	ResultCallback resultCallback;
	ErrorCallback errorCallback;
};

#endif //OBS_AUTO_SUBTITLE_TRANSBASE_H
