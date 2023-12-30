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

#include "GScriptTrans.h"

GScriptTrans::GScriptTrans(QString deployId, QObject *parent)
	: TransBase(parent), deployId(deployId)
{
	connect(&manager, &QNetworkAccessManager::finished, this,
		&GScriptTrans::onResult);
	manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

GScriptTrans::~GScriptTrans() {}

void GScriptTrans::onRequestTranslate(QString from, QString to, QString content)
{
	QNetworkRequest req;
	QUrl url;
	QUrlQuery query;
	query.addQueryItem("from", from);
	query.addQueryItem("to", to);
	query.addQueryItem("content", content);
	url.setUrl(QString(GS_ENDPOINT_URL).arg(deployId));
	url.setQuery(query);
	req.setUrl(url);
	manager.get(req);
}

void GScriptTrans::onResult(QNetworkReply *rep)
{
	QByteArray content = rep->readAll();
	QJsonDocument body;
	body = QJsonDocument::fromJson(content);

	if (!body.isObject()) {
		callbackError("Parse response json failed");
		return;
	}
	const auto &bodyObject = body.object();
	auto iter = bodyObject.find("result");

	if (iter == bodyObject.end() || !iter.value().isString()) {
		callbackError("reply error: " + content);
		return;
	}
	auto data = iter.value().toString();
	callbackResult(data);
}
