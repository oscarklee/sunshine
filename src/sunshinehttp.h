/**
 * @file src/sunshinehttp.h
 * @brief todo
 */
#pragma once

// standard includes
#include <string>

// local includes
#include "thread_safe.h"

// lib includes
#include <Simple-Web-Server/server_https.hpp>

namespace sunshinehttp {
    class SunshineHttpsServer: public SimpleWeb::Server<SimpleWeb::HTTPS> {
  public:
    SunshineHttpsServer(const std::string &certification_file, const std::string &private_key_file):
        SimpleWeb::Server<SimpleWeb::HTTPS>::Server(certification_file, private_key_file) {}

    std::function<int(crypto::x509_t&)> verify;
    std::function<void(std::shared_ptr<Response>, std::shared_ptr<Request>)> on_verify_failed;

  protected:
    std::string 
    getCertificatePEM(const crypto::x509_t& cert) {
        BIO* bio = BIO_new(BIO_s_mem());
        PEM_write_bio_X509(bio, cert.get());
        char* buffer;
        long length = BIO_get_mem_data(bio, &buffer);
        std::string pemStr(buffer, length);
        BIO_free(bio);
        return pemStr;
    }

    void
    after_bind() override {
      SimpleWeb::Server<SimpleWeb::HTTPS>::after_bind();

      if (verify) {
        context.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::verify_fail_if_no_peer_cert | boost::asio::ssl::verify_client_once);
        context.set_verify_callback([](int verified, boost::asio::ssl::verify_context &ctx) {
          // To respond with an error message, a connection must be established
          return 1;
        });
      }
    }

    // This is Server<HTTPS>::accept() with SSL validation support added
    void
    accept() override {
      auto connection = create_connection(*io_service, context);

      acceptor->async_accept(connection->socket->lowest_layer(), [this, connection](const SimpleWeb::error_code &ec) {
        auto lock = connection->handler_runner->continue_lock();
        if (!lock)
          return;

        if (ec != SimpleWeb::error::operation_aborted)
          this->accept();

        auto session = std::make_shared<Session>(config.max_request_streambuf_size, connection);

        if (!ec) {
          boost::asio::ip::tcp::no_delay option(true);
          SimpleWeb::error_code ec;
          session->connection->socket->lowest_layer().set_option(option, ec);

          session->connection->set_timeout(config.timeout_request);
          session->connection->socket->async_handshake(boost::asio::ssl::stream_base::server, [this, session](const SimpleWeb::error_code &ec) {
            session->connection->cancel_timeout();
            auto lock = session->connection->handler_runner->continue_lock();
            if (!lock)
              return;
            if (!ec) {
              crypto::x509_t x509 = { SSL_get_peer_certificate(session->connection->socket->native_handle()) };
              session->request->header.emplace("cert", getCertificatePEM(x509));
              if (verify && !verify(x509)) {
                this->write(session, on_verify_failed);
              } else {
                this->read(session);
              }
            }
            else if (this->on_error)
              this->on_error(session->request, ec);
          });
        }
        else if (this->on_error)
          this->on_error(session->request, ec);
      });
    }
  };

}