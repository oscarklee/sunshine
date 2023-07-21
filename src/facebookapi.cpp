/**
 * @file src/facebookapi.cpp
 * @brief todo
 */
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "facebookapi.h"

#include <iostream>
#include <cpp-httplib/httplib.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

namespace facebook {
    namespace pt = boost::property_tree;

    std::string
    getFacebookProfile(std::string accesstoken) {
        httplib::Client client("https://graph.facebook.com");
        auto response = client.Get("/me", {{"Authorization", std::string("Bearer ") + accesstoken}});

        if (response && response->status == 200) {
            pt::ptree json;
            std::stringstream jsonStream;

            jsonStream << response->body;
            pt::read_json(jsonStream, json);

            std::string id = json.get<std::string>("id");
            if (!id.empty()) {
                return id;
            }
        }

        throw std::runtime_error(response->body);
    }
}