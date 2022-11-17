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

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>

#include "XFTrans.h"

XFTrans::XFTrans(QString appId, QString apiKey, QString apiSecret,
		 QObject *parent)
	: TransBase(parent),
	  appId(appId),
	  apiKey(apiKey),
	  apiSecret(apiSecret),
	  manager(this),
	  endpoint(TRANS_ENDPOINT)
{
	connect(&manager, &QNetworkAccessManager::finished, this,
		&XFTrans::onResult);
}

XFTrans::XFTrans(QString appId, QString apiKey, QString apiSecret,
		 QString endpoint, QObject *parent)
	: TransBase(parent),
	  appId(appId),
	  apiKey(apiKey),
	  apiSecret(apiSecret),
	  manager(this),
	  endpoint(endpoint)
{
	connect(&manager, &QNetworkAccessManager::finished, this,
		&XFTrans::onResult);
}

XFTrans::~XFTrans() {}

void XFTrans::onResult(QNetworkReply *rep)
{
	QByteArray content = rep->readAll();
	QJsonDocument body;
	body = QJsonDocument::fromJson(content);

	if (!body.isObject()) {
		callbackError("Parse response json failed");
		return;
	}
	const auto &bodyObject = body.object();
	auto iter = bodyObject.find("code");
	if (iter == bodyObject.end() || !iter.value().isDouble()) {
		iter = bodyObject.find("message");
		if (iter == bodyObject.end() || !iter.value().isString()) {
			callbackError("Response json format error");
			return;
		} else {
			callbackError(iter.value().toString());
			return;
		}
		return;
	}
	auto code = iter.value().toInt();
	if (code != 0) {
		iter = bodyObject.find("message");
		if (iter == bodyObject.end() || !iter.value().isString()) {
			callbackError("Response json format error");
			return;
		} else {
			callbackError(iter.value().toString());
			return;
		}
	}

	iter = bodyObject.find("data");
	if (iter == bodyObject.end() || !iter.value().isObject()) {
		callbackError("Response json format error");
		return;
	}
	const auto &dataObject = iter.value().toObject();

	iter = dataObject.find("result");
	if (iter == dataObject.end() || !iter.value().isObject()) {
		callbackError("Response json format error");
		return;
	}
	const auto &resultObject = iter.value().toObject();

	iter = resultObject.find("trans_result");
	if (iter == resultObject.end() || !iter.value().isObject()) {
		callbackError("Response json format error");
		return;
	}
	const auto &trans_resultObject = iter.value().toObject();
	iter = trans_resultObject.find("dst");
	if (iter == trans_resultObject.end() || !iter.value().isString()) {
		callbackError("Response json format error");
		return;
	}
	auto result = iter.value().toString();
	callbackResult(result);
	rep->deleteLater();
}

void XFTrans::onRequestTranslate(QString from, QString to, QString content)
{
	QJsonDocument doc;
	QJsonObject body, data, common, business;
	common["app_id"] = appId;
	body["common"] = common;
	business["from"] = from;
	business["to"] = to;
	body["business"] = business;
	QString text = content.toUtf8().toBase64();
	data["text"] = text;
	body["data"] = data;
	doc.setObject(body);
	QByteArray httpBody = doc.toJson(QJsonDocument::JsonFormat::Compact);
	QNetworkRequest req = assembleRequest(endpoint, httpBody);

	manager.post(req, httpBody);
}

QString XFTrans::signBody(QByteArray body)
{
	return QCryptographicHash::hash(body, QCryptographicHash::Sha256)
		.toBase64();
}

QString XFTrans::genSignature(QString host, QString date, QString httpMethod,
			      QString requestUri, QString digest)
{
	QString signatureStr;
	if (host.length() != 0) {
		signatureStr = "host: " + host + "\n";
	}
	signatureStr += "date: " + date + "\n";
	signatureStr += httpMethod + " " + requestUri + " HTTP/1.1\n";
	signatureStr += "digest: " + digest;
	return QMessageAuthenticationCode::hash(signatureStr.toLocal8Bit(),
						apiSecret.toLocal8Bit(),
						QCryptographicHash::Sha256)
		.toBase64();
}

QNetworkRequest XFTrans::assembleRequest(QString url, QByteArray body)
{
	QNetworkRequest req;
	QUrl qurl;
	qurl.setUrl(url);
	req.setUrl(qurl);
	req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
		      "application/json");
	QDateTime datetime = QDateTime::currentDateTime().toUTC();
	QLocale loc = QLocale(QLocale::English, QLocale::UnitedStates);
	QString dateStr =
		loc.toString(datetime, "ddd, dd MMM yyyy HH:mm:ss") + " GMT";
	req.setRawHeader("Date", dateStr.toLocal8Bit());
	QString digest = "SHA-256=" + signBody(body);
	req.setRawHeader("Digest", digest.toLocal8Bit());
	QString sign =
		genSignature(qurl.host(), dateStr, "POST", qurl.path(), digest);
	QString authHeader =
		"api_key=\"%1\", algorithm=\"hmac-sha256\", headers=\"host date request-line digest\", signature=\"%2\"";
	authHeader = authHeader.arg(apiKey, sign);
	req.setRawHeader("Authorization", authHeader.toLocal8Bit());
	return req;
}