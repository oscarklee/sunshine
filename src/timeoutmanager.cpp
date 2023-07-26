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
    std::function<void(std::shared_ptr<nvhttp::client_t>, int)> timeoutmanager_t::on_pause_time;

    void timeoutmanagerthread(std::shared_ptr<nvhttp::client_t> client, std::atomic<bool>* pause, std::atomic<bool>* stop) {
        auto pause_time = 0;
        while (!(*stop) && client->seconds > 0) {
            while (!(*stop) && !(*pause) && client->seconds > 0) {
                pause_time = 0;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                client->seconds--;

                if (timeoutmanager_t::on_time_decrease)
                    timeoutmanager_t::on_time_decrease(client);
            }

            pause_time++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (timeoutmanager_t::on_pause_time)
                timeoutmanager_t::on_pause_time(client, pause_time);
        }
    }

    void start(std::shared_ptr<nvhttp::client_t> client) {
        auto it = managers.find(client->fbp);
        if (it != managers.end() && it->second.thread.joinable()) {
            *(it->second.pause) = false;
            return;
        }

        auto pause = new std::atomic<bool>(false);
        auto stop = new std::atomic<bool>(false);
        managers.emplace(client->fbp, timeoutmanager_t(client, std::thread(timeoutmanagerthread, client, pause, stop), pause, stop));
    }

    void stop(std::shared_ptr<nvhttp::client_t> client) {
        auto it = managers.find(client->fbp);
        if (it != managers.end() && it->second.thread.joinable()) {
            *(it->second.pause) = true;
        }
    }

    void clear(std::shared_ptr<nvhttp::client_t> client) {
        auto it = managers.find(client->fbp);
        if (it != managers.end() && it->second.thread.joinable()) {
            *(it->second.stop) = true;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            it->second.thread.detach();
            managers.erase(it);
        }
    }
}  // namespace timeoutmanager
