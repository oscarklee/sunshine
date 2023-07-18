/**
 * @file src/timeoutmanager.cpp
 * @brief todo
 */

// local includes
#include "timeoutmanager.h"
#include "main.h"
#include "nvhttp.h"

namespace timeoutmanager {
    struct timeoutmanager_t {
        std::shared_ptr<nvhttp::client_t> client;
        std::thread thread;
        std::atomic<bool>* pause;

        timeoutmanager_t(std::shared_ptr<nvhttp::client_t> client, std::thread thread, std::atomic<bool>* pause)
            : client(client), thread(std::move(thread)), pause(pause) {}
    };

    std::unordered_map<std::string, timeoutmanager_t> managers;

    void timeoutmanagerthread(std::shared_ptr<nvhttp::client_t> client, std::atomic<bool>* pause) {
        while (client->seconds > 0) {
            while (!(*pause) && client->seconds > 0) {
                BOOST_LOG(info) << "ID " << client->fbp << ": " << client->seconds << " seconds left.";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                client->seconds--;
            }
        }
    }

    void start(std::shared_ptr<nvhttp::client_t> client) {
        auto it = managers.find(client->fbp);
        if (it != managers.end()) {
            *(it->second.pause) = false;
        } else {
            auto pause = new std::atomic<bool>(false);
            managers.emplace(client->fbp, timeoutmanager_t(client, std::thread(timeoutmanagerthread, client, pause), pause));
        }
    }

    void stop(std::shared_ptr<nvhttp::client_t> client) {
        auto it = managers.find(client->fbp);
        if (it != managers.end()) {
            *(it->second.pause) = true;
        }
    }
}  // namespace timeoutmanager
