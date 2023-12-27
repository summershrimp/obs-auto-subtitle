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

#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <QUuid>

#include "FunASR.h"

using namespace std::placeholders;

FunASR::FunASR(const QString &endpoint, QObject *parent)
	: ASRBase(parent), endpoint(endpoint)
{
	connect(&ws, &QWebSocket::connected, this, &FunASR::onConnected);
	connect(&ws, &QWebSocket::disconnected, this, &FunASR::onDisconnected);
	connect(&ws, &QWebSocket::textMessageReceived, this,
		&FunASR::onTextMessageReceived);
	connect(this, &FunASR::haveResult, this, &FunASR::onResult);
	connect(&ws, SIGNAL(error(QAbstractSocket::SocketError)), this,
		SLOT(onError(QAbstractSocket::SocketError)));
	running = false;
}

void FunASR::onStart()
{
	QNetworkRequest request;
	QUrl url(endpoint);
	qDebug() << url.toString();
	request.setUrl(url);
	ws.open(request);
}

void FunASR::onError(QAbstractSocket::SocketError error)
{
	auto errorCb = getErrorCallback();
	if (errorCb)
		errorCb(ERROR_SOCKET, ws.errorString());
	qDebug() << ws.errorString();
}

void FunASR::onConnected()
{
	QJsonObject _payload;
	_payload["mode"] = "2pass";
	_payload["wav_name"] = "wav_name";
	_payload["is_speaking"] = true;
	_payload["wav_format"] = "pcm";
	QJsonArray chunk_size = {3, 5, 3};
	_payload["chunk_size"] = chunk_size;
	_payload["itn"] = true;
	_payload["audio_fs"] = 16000;

	QJsonDocument doc(_payload);
	ws.sendTextMessage(doc.toJson());
	connect(this, &ASRBase::sendAudioMessage, this,
		&FunASR::onSendAudioMessage);
	auto connectCb = getConnectedCallback();
	if (connectCb)
		connectCb();
	running = true;
	qDebug() << "WebSocket connected";
}

void FunASR::onDisconnected()
{
	running = false;
	auto disconnectCb = getDisconnectedCallback();
	if (disconnectCb)
		disconnectCb();
	qDebug() << "WebSocket disconnected";
}

void FunASR::onSendAudioMessage(const char *data, unsigned long size)
{
	if (!running) {
		return;
	}
	ws.sendBinaryMessage(QByteArray::fromRawData(data, size));
}

void FunASR::onTextMessageReceived(const QString message)
{
	QJsonDocument doc(QJsonDocument::fromJson(message.toUtf8()));
	qDebug() << "FunASR receive:" << message;
	bool ok = false;
	if (!doc.isObject()) {
		return;
	}
	if (!doc.object().contains("text") ||
	    !doc.object().value("text").isString()) {
		return;
	}

	if (!doc.object().contains("mode") ||
	    !doc.object().value("mode").isString()) {
		return;
	}
	QString text = doc.object().value("text").toString();
	QString output;
	QString mode = doc.object().value("mode").toString();
	bool is_final = mode == "2pass-offline";
	if (is_final) {
		middleResult = "";
		output = text;
	} else {
		middleResult += text;
		output = middleResult;
	}
	emit haveResult(output, !is_final);
}

void FunASR::onResult(QString message, int type)
{
	auto callback = getResultCallback();
	if (callback)
		callback(message, type);
}

void FunASR::onStop()
{
	QJsonObject _payload = {{"is_speaking", false}};
	QJsonDocument doc(_payload);
	ws.sendTextMessage(doc.toJson());
	ws.close();
}

FunASR::~FunASR()
{
	stop();
}
