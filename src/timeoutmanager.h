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
    // functions
    void
    start(std::shared_ptr<nvhttp::client_t> client);
    void
    stop(std::shared_ptr<nvhttp::client_t> client);
}