/*
QWebsocketpp
 Copyright (C) 2019-2023 Yibai Zhang

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

#ifndef _QWEBSOCKETPP_H
#define _QWEBSOCKETPP_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QNetworkRequest>
#include <QAbstractSocket>
#include <memory>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/core_client.hpp>
#include <websocketpp/client.hpp>

#include <thread>

typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::client<websocketpp::config::asio_client> no_tls_client;
typedef websocketpp::config::core_client core_client;
typedef std::shared_ptr<asio::ssl::context> context_ptr;

class QWebsocketpp : public QObject {
	Q_OBJECT
public:
	QWebsocketpp(QObject *parent = nullptr);
	qint64 sendBinaryMessage(const QByteArray &data);
	qint64 sendTextMessage(const QString &message);
	QString errorString() const;
	static void setDefaultCACertPath(const QString &path);

public slots:
	void
	close(unsigned short closeCode = websocketpp::close::status::normal,
	      const QString &reason = QString());
	void ignoreSslErrors();
	void open(const QUrl &url);
	void open(const QNetworkRequest &request);

signals:
	void connected();
	void disconnected();
	void textMessageReceived(const QString &message);
	void binaryMessageReceived(const QByteArray &message);
	void errorOccurred(QAbstractSocket::SocketError error);

private:
	static QString default_cacert_path;
	void onMessage(websocketpp::connection_hdl hdl,
		       core_client::message_type::ptr msg);
	std::shared_ptr<tls_client> wss_client;
	std::shared_ptr<no_tls_client> ws_client;
	std::thread run_thread;
	int client_is_tls;
	websocketpp::connection_hdl hdl;
	QString cacert_path;
	bool running = false;
	std::mutex running_lck;
};

#endif
