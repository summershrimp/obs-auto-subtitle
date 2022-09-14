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
#include <QString>

#include "MSSAPI.h"

using namespace std::placeholders;

MSSAPI::MSSAPI(const QString &lang, QObject *parent)
	: ASRBase(parent), lang(lang)
{
	mssapi = std::make_shared<mssapi_captions>(
		std::bind(MSSAPI::callback, this, std::placeholders::_1),
		lang.toStdString());
	connect(this, &ASRBase::sendAudioMessage, this,
		&MSSAPI::onSendAudioMessage);
}

void MSSAPI::onSendAudioMessage(const char *data, unsigned long size)
{
	this->mssapi->pcm_data(data, size);
}

void MSSAPI::onStart() {}

void MSSAPI::onStop() {}

void MSSAPI::callback(MSSAPI *their, const std::string &result)
{
	if (their->getResultCallback())
		their->getResultCallback()(QString(result.c_str()), 0);
}

MSSAPI::~MSSAPI() {}
