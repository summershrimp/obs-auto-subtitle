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

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QEventLoop>
#include <cpr/cpr.h>

#include "HwCloudRASRBuilder.h"

#define HWCLOUD_PROVIDER_ID 0x0002U
#define L_SP_HWCLOUD "AutoSub.SP.Hwcloud"

#define PROP_HWCLOUD_PROJID "autosub_filter_hwcloud_proj_id"
#define T_PROJECT_ID obs_module_text("AutoSub.ProjectId")

#define PROP_HWCLOUD_REGION "autosub_filter_hwcloud_region"
#define T_REGION obs_module_text("AutoSub.Region")

#define PROP_HWCLOUD_USERNAME "autosub_filter_hwcloud_username"
#define T_HW_USERNAME obs_module_text("AutoSub.HWCloud.Username")
#define PROP_HWCLOUD_DOMAINNAME "autosub_filter_hwcloud_domainname"
#define T_HW_DOMAINNAME obs_module_text("AutoSub.HWCloud.DomainName")
#define PROP_HWCLOUD_PASSWORD "autosub_filter_hwcloud_password"
#define T_PASSWORD obs_module_text("AutoSub.Password")

void HwCloudRASRBuilder::getProperties(obs_properties_t *props)
{
	obs_property_t *p = obs_properties_add_list(props, PROP_HWCLOUD_REGION,
						    T_REGION,
						    OBS_COMBO_TYPE_LIST,
						    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, "cn-north-1", "cn-north-1");
	obs_property_list_add_string(p, "cn-north-4", "cn-north-4");
	obs_property_list_add_string(p, "cn-east-3", "cn-east-3");

	obs_properties_add_text(props, PROP_HWCLOUD_USERNAME, T_HW_USERNAME,
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_HWCLOUD_DOMAINNAME, T_HW_DOMAINNAME,
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_HWCLOUD_PASSWORD, T_PASSWORD,
				OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, PROP_HWCLOUD_PROJID, T_PROJECT_ID,
				OBS_TEXT_DEFAULT);
}

void HwCloudRASRBuilder::showProperties(obs_properties_t *props)
{
	PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_REGION);
	PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_USERNAME);
	PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_DOMAINNAME);
	PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_PASSWORD);
	PROPERTY_SET_VISIBLE(props, PROP_HWCLOUD_PROJID);
}

void HwCloudRASRBuilder::hideProperties(obs_properties_t *props)
{
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_REGION);
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_USERNAME);
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_DOMAINNAME);
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_PASSWORD);
	PROPERTY_SET_UNVISIBLE(props, PROP_HWCLOUD_PROJID);
}

void HwCloudRASRBuilder::updateSettings(obs_data_t *settings)
{
	QString _project_id =
		obs_data_get_string(settings, PROP_HWCLOUD_PROJID);
	QString _region = obs_data_get_string(settings, PROP_HWCLOUD_REGION);
	QString _username =
		obs_data_get_string(settings, PROP_HWCLOUD_USERNAME);
	QString _domainname =
		obs_data_get_string(settings, PROP_HWCLOUD_DOMAINNAME);
	QString _password =
		obs_data_get_string(settings, PROP_HWCLOUD_PASSWORD);

	CHECK_CHANGE_SET_ALL(this->project_id, _project_id, needBuild);
	CHECK_CHANGE_SET_ALL(this->region, _region, needRefresh);
	CHECK_CHANGE_SET_ALL(this->username, _username, needRefresh);
	CHECK_CHANGE_SET_ALL(this->domain_name, _domainname, needRefresh);
	CHECK_CHANGE_SET_ALL(this->password, _password, needRefresh);
}

void HwCloudRASRBuilder::getDefaults(obs_data_t *settings)
{
	(void)settings;
}

void HwCloudRASRBuilder::refreshToken()
{
	if (username.isEmpty() || password.isEmpty() || domain_name.isEmpty() ||
	    region.isEmpty()) {
		return;
	}
	QUrl token_endpoint(QStringLiteral(HWCLOUD_IAM_TOKEN_API).arg(region));
	QNetworkRequest request(token_endpoint);
	request.setHeader(QNetworkRequest::ContentTypeHeader,
			  "application/json");
	request.setTransferTimeout(7000);
	QJsonObject json;
	json.insert(
		"auth",
		QJsonObject(
			{{"identity",
			  QJsonObject(
				  {{"methods", QJsonArray({"password"})},
				   {"password",
				    QJsonObject(
					    {{"user",
					      QJsonObject(
						      {{"name", username},
						       {"password", password},
						       {"domain",
							QJsonObject(
								{{"name",
								  domain_name}})}})}})}})},
			 {"scope",
			  QJsonObject({{"project",
					QJsonObject({{"name", region}})}})}}));
	QNetworkAccessManager nam;
	QEventLoop loop;
	QByteArray body = QJsonDocument(json).toJson(QJsonDocument::Compact);
	qDebug() << "request body:" << body;

	cpr::Response r =
		cpr::Post(cpr::Url{token_endpoint.toString().toStdString()},
			  cpr::Body{body.toStdString()},
			  cpr::Header{{"Content-Type", "application/json"}});

	if (r.status_code != 201) {
		if (r.status_code == 0) {
			blog(LOG_WARNING, "network error: %s",
			     r.error.message.c_str());
		} else {
			blog(LOG_WARNING, "error: %d, %s", (int)r.status_code,
			     r.text.c_str());
		}
		return;
	}
	token = QString::fromStdString(r.header["X-Subject-Token"]);

	qDebug() << "get token:" << token << QString::fromStdString(r.text);

	needRefresh = false;
}

ASRBase *HwCloudRASRBuilder::build()
{
	if (!needBuild && !needRefresh) {
		return nullptr;
	}
	if (needRefresh) {
		token = "";
		refreshToken();
	}
	if (token.isEmpty()) {
		return nullptr;
	}
	needBuild = false;
	auto asr = new HwCloudRASR(region, project_id, token);
	return asr;
}

static HwCloudRASRBuilder hwCloudRASRBuilder;
static ASRBuilderRegister register_hwcloud_asr(&hwCloudRASRBuilder,
					       HWCLOUD_PROVIDER_ID,
					       L_SP_HWCLOUD);
