#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "CommonObjects.h"

class GameLogic {
public:
    GameLogic(CommonObjects& common);

    void notifyGameInitialized();
    void notifyGuessProcessed(const GuessResult& result);
    void notifyApiError(const std::string& error);
    void notifyInvalidWord();

    bool submitGuess(const std::string& guess);
    bool isWaitingForApi() const;
    bool isGameOver() const;
    bool hasWon() const;
    const std::vector<GuessResult>& getGuessHistory() const;
    std::string getCurrentAnswer() const;

    void setOnGameStateChanged(std::function<void()> callback);
    void setOnErrorOccurred(std::function<void(const std::string&)> callback);
    void setOnInvalidWord(std::function<void()> callback);

private:
    bool ValidLength(const std::string& guess) const;
    void notifyGameStateChanged();
    void updateLetterStates(GuessResult& result) const;

    CommonObjects& common;
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::function<void()> onGameStateChanged;
    std::function<void(const std::string&)> onErrorOccurred;
    std::function<void()> onInvalidWord;
};

#endif // GAMELOGIC_H
