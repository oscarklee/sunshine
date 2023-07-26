/**
 * @file src/timeoutmanager.h
 * @brief todo
 */
#pragma once

// standard includes
#include <thread>

// local includes
#include "nvhttp.h"

namespace timeoutmanager {
    struct timeoutmanager_t {
        static std::function<void(std::shared_ptr<nvhttp::client_t>)> on_time_decrease;
        static std::function<void(std::shared_ptr<nvhttp::client_t>, int)> on_pause_time;
        std::shared_ptr<nvhttp::client_t> client;
        std::thread thread;
        std::atomic<bool>* pause;
        std::atomic<bool>* stop;

        timeoutmanager_t(std::shared_ptr<nvhttp::client_t> client, std::thread thread, std::atomic<bool>* pause, std::atomic<bool>* stop)
            : client(client), thread(std::move(thread)), pause(pause), stop(stop) {}
    };
    
    // functions
    void
    start(std::shared_ptr<nvhttp::client_t> client);
    void
    stop(std::shared_ptr<nvhttp::client_t> client);
    void
    clear(std::shared_ptr<nvhttp::client_t> client);
}