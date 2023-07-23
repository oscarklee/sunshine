/**
 * @file src/publichttp.cpp
 * @brief todo
 */

#include "publichttp.h"
#include "config.h"
#include "main.h"

#include <Simple-Web-Server/crypto.hpp>
#include <Simple-Web-Server/server_https.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/algorithm/string.hpp>

using namespace std::literals;

namespace phttp {
    namespace pt = boost::property_tree;

    using https_server_t = SimpleWeb::Server<SimpleWeb::HTTPS>;

    using resp_https_t = std::shared_ptr<typename SimpleWeb::ServerBase<SimpleWeb::HTTPS>::Response>;
    using req_https_t = std::shared_ptr<typename SimpleWeb::ServerBase<SimpleWeb::HTTPS>::Request>;

    void
    print_req(const req_https_t &request) {
        BOOST_LOG(debug) << "METHOD :: "sv << request->method;
        BOOST_LOG(debug) << "DESTINATION :: "sv << request->path;

        for (auto &[name, val] : request->header) {
        BOOST_LOG(debug) << name << " -- " << (name == "Authorization" ? "CREDENTIALS REDACTED" : val);
        }

        BOOST_LOG(debug) << " [--] "sv;

        for (auto &[name, val] : request->parse_query_string()) {
        BOOST_LOG(debug) << name << " -- " << val;
        }

        BOOST_LOG(debug) << " [--] "sv;
    }

    void
    not_found(resp_https_t response, req_https_t request) {
        pt::ptree tree;
        tree.put("root.<xmlattr>.status_code", 404);

        std::ostringstream data;

        pt::write_xml(data, tree);
        response->write(data.str());

        *response << "HTTP/1.1 404 NOT FOUND\r\n"<< data.str();
    }

    void
    getIndexPage(resp_https_t response, req_https_t request) {
        print_req(request);

        std::string content = read_file(WEB_DIR "index.html");
        SimpleWeb::CaseInsensitiveMultimap headers;
        headers.emplace("Content-Type", "text/html; charset=utf-8");
        response->write(content, headers);
    }

    void
    start() {
        auto shutdown_event = mail::man->event<bool>(mail::shutdown);

        https_server_t server { config::nvhttp.cert, config::nvhttp.pkey };
        server.default_resource["GET"] = not_found;
        server.resource["^/$"]["GET"] = getIndexPage;

        auto accept_and_run = [&](auto *server) {
            try {
                server->start([](unsigned short port) {
                    BOOST_LOG(info) << "Public UI available at [https://localhost:"sv << port << "]";
                });
            }
            catch (boost::system::system_error &err) {
                // It's possible the exception gets thrown after calling server->stop() from a different thread
                if (shutdown_event->peek()) {
                    return;
                }

                BOOST_LOG(fatal) << "Couldn't start Configuration HTTPS server: "sv << err.what();
                shutdown_event->raise(true);
                return;
            }
        };
        std::thread tcp { accept_and_run, &server };

        // Wait for any event
        shutdown_event->view();

        server.stop();

        tcp.join();
    }
}
