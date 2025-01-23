// GameLogic.h
#pragma once
#include "CommonObject.h"
#include <functional>
#include <mutex>
#include <condition_variable>

class GameLogic {
public:
    GameLogic(CommonObjects& common);

    // Interface for DownloadThread
    void notifyGameInitialized();
    void notifyGuessProcessed(const GuessResult& result);
    void notifyApiError(const std::string& error);
    void notifyInvalidWord();   // for words not in word list

    // Interface for GUI/Input thread
    bool submitGuess(const std::string& guess);
    bool isWaitingForApi() const;
    bool isGameOver() const;
    bool hasWon() const;
    const std::vector<GuessResult>& getGuessHistory() const;
    std::string getCurrentAnswer() const;

    // Register callbacks for GUI updates
    void setOnGameStateChanged(std::function<void()> callback);
    void setOnErrorOccurred(std::function<void(const std::string&)> callback);
    void setOnInvalidWord(std::function<void()> callback); // New callback for invalid words

private:
    CommonObjects& common;
    mutable std::mutex mutex;
    std::condition_variable cv;

    std::function<void()> onGameStateChanged;
    std::function<void(const std::string&)> onErrorOccurred;
    std::function<void()> onInvalidWord; // New callback for invalid words

    bool ValidLength(const std::string& guess) const;
    void notifyGameStateChanged();
};
