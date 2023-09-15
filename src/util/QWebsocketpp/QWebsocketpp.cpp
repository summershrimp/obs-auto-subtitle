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

#include "QWebsocketpp.h"

#include <string.h>
#ifdef _WINDOWS
int strcasecmp(const char *a, const char *b) {
    return stricmp(a, b);
}
#endif


QString default_cacert_path;

void setDefaultCACertPath(const QString &path) {
    default_cacert_path = path;
}

/// Verify that one of the subject alternative names matches the given hostname
bool verify_subject_alternative_name(const char * hostname, X509 * cert) {
    STACK_OF(GENERAL_NAME) * san_names = NULL;
    
    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }
    
    int san_names_count = sk_GENERAL_NAME_num(san_names);
    
    bool result = false;
    
    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);
        
        if (current_name->type != GEN_DNS) {
            continue;
        }
        
        char const * dns_name = (char const *) ASN1_STRING_get0_data(current_name->d.dNSName);
        
        // Make sure there isn't an embedded NUL character in the DNS name
        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
    
    return result;
}

/// Verify that the certificate common name matches the given hostname
bool verify_common_name(char const * hostname, X509 * cert) {
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }
    
    // Extract the CN field
    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }
    
    // Convert the CN field to a C string
    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }
    
    char const * common_name_str = (char const *) ASN1_STRING_get0_data(common_name_asn1);
    
    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }
    
    // Compare expected hostname with the CN
    return (strcasecmp(hostname, common_name_str) == 0);
}

/**
 * This code is derived from examples and documentation found ato00po
 * http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
 * and
 * https://github.com/iSECPartners/ssl-conservatory
 */
bool verify_certificate(const char * hostname, bool preverified, asio::ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // if we are on the final cert and everything else checks out, ensure that
    // the hostname is present on the list of SANs or the common name (CN).
    if (depth == 0 && preverified) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        
        if (verify_subject_alternative_name(hostname, cert)) {
            return true;
        } else if (verify_common_name(hostname, cert)) {
            return true;
        } else {
            return false;
        }
    }

    return preverified;
}

/// TLS Initialization handler
/**
 * WebSocket++ core and the Asio Transport do not handle TLS context creation
 * and setup. This callback is provided so that the end user can set up their
 * TLS context using whatever settings make sense for their application.
 *
 * As Asio and OpenSSL do not provide great documentation for the very common
 * case of connect and actually perform basic verification of server certs this
 * example includes a basic implementation (using Asio and OpenSSL) of the
 * following reasonable default settings and verification steps:
 *
 * - Disable SSLv2 and SSLv3
 * - Load trusted CA certificates and verify the server cert is trusted.
 * - Verify that the hostname matches either the common name or one of the
 *   subject alternative names on the certificate.
 *
 * This is not meant to be an exhaustive reference implimentation of a perfect
 * TLS client, but rather a reasonable starting point for building a secure
 * TLS encrypted WebSocket client.
 *
 * If any TLS, Asio, or OpenSSL experts feel that these settings are poor
 * defaults or there are critically missing steps please open a GitHub issue
 * or drop a line on the project mailing list.
 *
 * Note the bundled CA cert ca-chain.cert.pem is the CA cert that signed the
 * cert bundled with echo_server_tls. You can use print_client_tls with this
 * CA cert to connect to echo_server_tls as long as you use /etc/hosts or
 * something equivilent to spoof one of the names on that cert 
 * (websocketpp.org, for example).
 */
context_ptr on_tls_init(websocketpp::connection_hdl, std::string hostname, std::string cacert_path) {
    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
        ctx->set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::no_sslv3 |
                         asio::ssl::context::single_dh_use);

        if(cacert_path.empty()) {
            ctx->set_verify_mode(asio::ssl::verify_none);
        } else {
            ctx->set_verify_mode(asio::ssl::verify_peer);
            ctx->set_verify_callback(std::bind(&verify_certificate, hostname.c_str(), std::placeholders::_1, std::placeholders::_2));

            // Here we load the CA certificates of all CA's that this client trusts.
            ctx->load_verify_file(cacert_path);
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}


QWebsocketpp::QWebsocketpp(QObject *parent) : QObject(parent) {
    cacert_path = default_cacert_path;
}


qint64 QWebsocketpp::sendBinaryMessage(const QByteArray &data) {
    ws_client.send(hdl, data.toStdString(), websocketpp::frame::opcode::binary);
    return data.size();
}

qint64 QWebsocketpp::sendTextMessage(const QString &message) {
    ws_client.send(hdl, message.toStdString(), websocketpp::frame::opcode::text);
    return message.size();
}

QString	QWebsocketpp::errorString() const {
    return QString();
}

void QWebsocketpp::close(unsigned short closeCode, const QString &reason) {
    ws_client.close(hdl, closeCode, reason.toStdString());
}

void QWebsocketpp::ignoreSslErrors() {
    cacert_path = "";
}

void QWebsocketpp::open(const QUrl &url) {
    QNetworkRequest request(url);
    emit open(request);
}

void QWebsocketpp::onMessage(websocketpp::connection_hdl thdl, client::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        emit textMessageReceived(QString::fromStdString(msg->get_payload()));
    } else {
        emit binaryMessageReceived(QByteArray(msg->get_payload().c_str(), msg->get_payload().size()));
    }
}

void QWebsocketpp::open(const QNetworkRequest &request) {
    
    ws_client.init_asio();
    ws_client.set_tls_init_handler(std::bind(&on_tls_init, std::placeholders::_1, request.url().host().toStdString().c_str(), cacert_path.toStdString()));
    ws_client.set_message_handler(std::bind(&QWebsocketpp::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    ws_client.set_open_handler([this](websocketpp::connection_hdl hdl){
        emit connected();
    });

    ws_client.set_fail_handler([this](websocketpp::connection_hdl hdl){
        emit errorOccurred(QAbstractSocket::SocketError::ConnectionRefusedError);
    });

    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection(request.url().toString().toStdString(), ec);

    for(auto i : request.rawHeaderList()) {
        auto key = i.split(':').first();
        auto value = i.split(':').last();
        con->append_header(key.toStdString(), value.toStdString());
    }

    hdl = ws_client.connect(con);

    run_thread = std::thread([this](){ws_client.run();});
    run_thread.detach();
}
