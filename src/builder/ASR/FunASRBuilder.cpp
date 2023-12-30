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

#include "FunASRBuilder.h"

#define FUNASR_PROVIDER_ID 0x0005U
#define L_SP_FUNASR "AutoSub.SP.FunASR"

#define PROP_FUNASR_ENDPOINT "autosub_filter_funasr_endpoint"
#define T_ENDPOINT obs_module_text("AutoSub.Endpoint")

void FunASRBuilder::getProperties(obs_properties_t *props)
{
	obs_properties_add_text(props, PROP_FUNASR_ENDPOINT, T_ENDPOINT,
				OBS_TEXT_DEFAULT);
}

void FunASRBuilder::showProperties(obs_properties_t *props)
{
	PROPERTY_SET_VISIBLE(props, PROP_FUNASR_ENDPOINT);
}

void FunASRBuilder::hideProperties(obs_properties_t *props)
{
	PROPERTY_SET_UNVISIBLE(props, PROP_FUNASR_ENDPOINT);
}

void FunASRBuilder::updateSettings(obs_data_t *settings)
{
	endpoint = obs_data_get_string(settings, PROP_FUNASR_ENDPOINT);
}

void FunASRBuilder::getDefaults(obs_data_t *settings)
{
	(void)settings;
}

ASRBase *FunASRBuilder::build()
{

	auto asr = new FunASR(endpoint);
	blog(LOG_INFO, "FunASRBuilder: build FunASR %s",
	     endpoint.toStdString().c_str());
	return asr;
}

static FunASRBuilder funASRBuilder;
static ASRBuilderRegister register_Alinls_asr(&funASRBuilder,
					      FUNASR_PROVIDER_ID, L_SP_FUNASR);
