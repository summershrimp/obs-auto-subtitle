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

#include "ASRBase.h"

ASRBase::ASRBase(QObject *parent) : QObject(parent)
{
	qRegisterMetaType<ResultCallback>("ResultCallback");
	connect(this, &ASRBase::start, this, &ASRBase::onStart);
	connect(this, &ASRBase::stop, this, &ASRBase::onStop);
}

void ASRBase::setParam(QString key, QString value)
{
	params[key] = value;
}

void ASRBase::setResultCallback(ASRBase::ResultCallback cb)
{
	this->resultCallback = std::move(cb);
}

void ASRBase::setConnectedCallback(ASRBase::ConnectedCallback cb)
{
	this->connectedCallback = std::move(cb);
}

void ASRBase::setDisconnectedCallback(ASRBase::DisconnectedCallback cb)
{
	this->disconnectedCallback = std::move(cb);
}

void ASRBase::setErrorCallback(ErrorCallback cb)
{
	this->errorCallback = std::move(cb);
}

ASRBase::ResultCallback ASRBase::getResultCallback()
{
	return resultCallback;
}

ASRBase::ConnectedCallback ASRBase::getConnectedCallback()
{
	return connectedCallback;
}

ASRBase::DisconnectedCallback ASRBase::getDisconnectedCallback()
{
	return disconnectedCallback;
}

ASRBase::ErrorCallback ASRBase::getErrorCallback()
{
	return errorCallback;
}