/**
 * @file src/timeoutmanager.cpp
 * @brief todo
 */

// local includes
#include "timeoutmanager.h"
#include "main.h"
#include "nvhttp.h"

namespace timeoutmanager {
    std::unordered_map<std::string, timeoutmanager_t> managers;
    std::function<void(std::shared_ptr<nvhttp::client_t>)> timeoutmanager_t::on_time_decrease;

    void timeoutmanagerthread(std::shared_ptr<nvhttp::client_t> client, std::atomic<bool>* pause) {
        while (client->seconds > 0) {
            while (!(*pause) && client->seconds > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                client->seconds--;

                if (timeoutmanager_t::on_time_decrease)
                    timeoutmanager_t::on_time_decrease(client);
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
