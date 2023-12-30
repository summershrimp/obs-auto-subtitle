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
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QEventLoop>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <cpr/cpr.h>

#include "AliNLSBuilder.h"

#define ALINLS_PROVIDER_ID 0x0001U
#define L_SP_ALINLS "AutoSub.SP.Aliyun"

#define PROP_ALINLS_APPKEY "autosub_filter_alinls_appkey"
#define T_APPKEY obs_module_text("AutoSub.AppKey")

#define PROP_ALINLS_AK "autosub_filter_alinls_accesskey"
#define PROP_ALINLS_SK "autosub_filter_alinls_secret"

#define PROP_ALINLS_PUNC "autosub_filter_alinls_punc"
#define T_ALINLS_PUNC obs_module_text("AutoSub.AliNLS.Punc")
#define PROP_ALINLS_ITN "autosub_filter_alinls_itn"
#define T_ALINLS_ITN obs_module_text("AutoSub.AliNLS.ITN")
#define PROP_ALINLS_INTRESULT "autosub_filter_alinls_InterResult"
#define T_ALINLS_INTRESULT obs_module_text("AutoSub.AliNLS.InterResult")

void AliNLSBuilder::getProperties(obs_properties_t *props)
{
	obs_properties_add_text(props, PROP_ALINLS_APPKEY, T_APPKEY,
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_ALINLS_AK, T_ACCEESSKEY,
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_ALINLS_SK, T_SECRET,
				OBS_TEXT_DEFAULT);
	obs_properties_add_bool(props, PROP_ALINLS_PUNC, T_ALINLS_PUNC);
	obs_properties_add_bool(props, PROP_ALINLS_ITN, T_ALINLS_ITN);
	obs_properties_add_bool(props, PROP_ALINLS_INTRESULT,
				T_ALINLS_INTRESULT);
}

void AliNLSBuilder::showProperties(obs_properties_t *props)
{
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_APPKEY);
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_AK);
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_SK);
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_PUNC);
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_ITN);
	PROPERTY_SET_VISIBLE(props, PROP_ALINLS_INTRESULT);
}

void AliNLSBuilder::hideProperties(obs_properties_t *props)
{
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_APPKEY);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_AK);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_SK);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_PUNC);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_ITN);
	PROPERTY_SET_UNVISIBLE(props, PROP_ALINLS_INTRESULT);
}

void AliNLSBuilder::updateSettings(obs_data_t *settings)
{
	QString _appkey = obs_data_get_string(settings, PROP_ALINLS_APPKEY);
	QString _ak = obs_data_get_string(settings, PROP_ALINLS_AK);
	QString _sk = obs_data_get_string(settings, PROP_ALINLS_SK);
	bool _punc = obs_data_get_bool(settings, PROP_ALINLS_PUNC);
	bool _itn = obs_data_get_bool(settings, PROP_ALINLS_ITN);
	bool _inter_result = obs_data_get_bool(settings, PROP_ALINLS_INTRESULT);

	CHECK_CHANGE_SET_ALL(this->access_key, _ak, needRefresh);
	CHECK_CHANGE_SET_ALL(this->secret, _sk, needRefresh);
	CHECK_CHANGE_SET_ALL(this->appkey, _appkey, needBuild);
	CHECK_CHANGE_SET_ALL(this->punc, _punc, needBuild);
	CHECK_CHANGE_SET_ALL(this->itn, _itn, needBuild);
	CHECK_CHANGE_SET_ALL(this->inter_result, _inter_result, needBuild);
}

void AliNLSBuilder::getDefaults(obs_data_t *settings)
{
	(void)settings;
}

void AliNLSBuilder::refreshToken()
{

	if (access_key.isEmpty() || secret.isEmpty()) {
		return;
	}

	QUrl meta(ALINLS_TOKEN_META);
	QUrlQuery query;
	query.addQueryItem("AccessKeyId", access_key);
	query.addQueryItem("Action", "CreateToken");
	query.addQueryItem("Format", "JSON");
	query.addQueryItem("RegionId", "cn-shanghai");
	query.addQueryItem("SignatureMethod", "HMAC-SHA1");
	query.addQueryItem("SignatureNonce",
			   QUuid::createUuid().toString(QUuid::WithoutBraces));
	query.addQueryItem("SignatureVersion", "1.0");
	query.addQueryItem("Timestamp",
			   QDateTime::currentDateTimeUtc()
				   .toString("yyyy-MM-ddThh:mm:ssZ")
				   .replace(":", "%3A"));
	query.addQueryItem("Version", "2019-02-28");

	QString querystr = query.query(QUrl::FullyEncoded);
	querystr = "GET&%2F&" + QUrl::toPercentEncoding(querystr);
	;

	QString key = secret + "&";
	qDebug() << querystr.toLatin1();
	QString signature = QMessageAuthenticationCode::hash(
				    querystr.toLatin1(), key.toLocal8Bit(),
				    QCryptographicHash::Sha1)
				    .toBase64();

	query.addQueryItem("Signature", signature);

	meta.setQuery(query);

	cpr::Response resp = cpr::Get(cpr::Url{meta.toString().toStdString()},
				      cpr::Timeout{3000});
	if (resp.status_code == 0) {
		blog(LOG_WARNING, "network error: %s",
		     resp.error.message.c_str());
		return;
	}
	if (resp.status_code != 200) {
		blog(LOG_WARNING, "http error: %d, %s", (int)resp.status_code,
		     resp.text.c_str());
		return;
	}

	QJsonObject jsonObject =
		QJsonDocument::fromJson(QByteArray::fromStdString(resp.text))
			.object();
	token = jsonObject.find("Token")
			.value()
			.toObject()
			.find("Id")
			.value()
			.toString();
	qDebug() << "Token:" << token;
	needRefresh = false;
}

ASRBase *AliNLSBuilder::build()
{
	if (!needBuild && !needRefresh) {
		return nullptr;
	}
	if (needRefresh) {
		token = "";
		refreshToken();
	}
	needBuild = false;
	if (token.isEmpty()) {
		return nullptr;
	}
	auto asr = new AliNLS(appkey, token);
	if (punc) {
		asr->setParam("enable_punctuation_prediction", "true");
	}
	if (itn) {
		asr->setParam("enable_inverse_text_normalization", "true");
	}
	if (inter_result) {
		asr->setParam("enable_intermediate_result", "true");
	}
	return asr;
}

static AliNLSBuilder aliNLSBuilder;
static ASRBuilderRegister register_Alinls_asr(&aliNLSBuilder,
					      ALINLS_PROVIDER_ID, L_SP_ALINLS);
