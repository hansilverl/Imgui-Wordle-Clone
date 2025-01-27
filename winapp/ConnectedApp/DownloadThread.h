#pragma once
#include "CommonObjects.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <string_view>
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include "GameLogic.h"

class DownloadThread {
public:
    DownloadThread(GameLogic& logic) : game_logic(logic) {}
    void operator()(CommonObjects& common);

private:
    GameLogic& game_logic;
    void initializeGame(CommonObjects& common);
    void processGuess(CommonObjects& common);
};