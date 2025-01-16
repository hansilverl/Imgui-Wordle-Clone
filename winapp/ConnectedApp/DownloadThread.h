#pragma once
#include "CommonObject.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <string_view>
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>

class DownloadThread {
public:
    void operator()(CommonObjects& common);

private:
    void initializeGame(CommonObjects& common);
    void processGuess(CommonObjects& common);
};