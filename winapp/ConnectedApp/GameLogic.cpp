#include "GameLogic.h"
#include <algorithm>
#include <cctype>

GameLogic::GameLogic(CommonObjects& common) : common(common) {}

void GameLogic::notifyGameInitialized() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        common.game_initialized = true;
    }
    notifyGameStateChanged();
}

void GameLogic::notifyGuessProcessed(const GuessResult& result) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (result.is_valid_word) {
            common.guess_history.push_back(result);

            if (result.is_correct) {
                common.game_won = true;
                common.game_over = true;
            }
            else if (common.guess_history.size() >= 6) {
                common.game_over = true;
            }
        }
        else {
            notifyInvalidWord();
        }
        common.waiting_for_api = false;
    }
    cv.notify_all();
    notifyGameStateChanged();
}

void GameLogic::notifyApiError(const std::string& error) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        common.waiting_for_api = false;
    }
    cv.notify_all();
    if (onErrorOccurred) {
        onErrorOccurred(error);
    }
}

void GameLogic::notifyInvalidWord() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        common.waiting_for_api = false;
    }
    cv.notify_all();
    if (onInvalidWord) {
        onInvalidWord();
    }
}

bool GameLogic::submitGuess(const std::string& guess) {
    if (!ValidLength(guess)) {
        return false;
    }
    // The following block is scoped to ensure the mutex is locked only for the duration of the block.
    // This helps in limiting the critical section and avoids holding the lock longer than necessary.
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (common.waiting_for_api || common.game_over) {
            return false;
        }
        common.current_guess = guess;
        common.waiting_for_api = true;
        common.new_guess_available = true;
    }
    return true;
}

bool GameLogic::isWaitingForApi() const {
    std::lock_guard<std::mutex> lock(mutex);
    return common.waiting_for_api;
}

bool GameLogic::isGameOver() const {
    std::lock_guard<std::mutex> lock(mutex);
    return common.game_over;
}

bool GameLogic::hasWon() const {
    std::lock_guard<std::mutex> lock(mutex);
    return common.game_won;
}

const std::vector<GuessResult>& GameLogic::getGuessHistory() const {
    std::lock_guard<std::mutex> lock(mutex);
    return common.guess_history;
}

std::string GameLogic::getCurrentAnswer() const {
    std::lock_guard<std::mutex> lock(mutex);
    return common.current_answer;
}

void GameLogic::setOnGameStateChanged(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex);
    onGameStateChanged = std::move(callback);
}

void GameLogic::setOnErrorOccurred(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(mutex);
    onErrorOccurred = std::move(callback);
}

void GameLogic::setOnInvalidWord(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex);
    onInvalidWord = std::move(callback);
}

bool GameLogic::ValidLength(const std::string& guess) const {
    if (guess.length() != 5) return false;
    return true;
}

void GameLogic::notifyGameStateChanged() {
    if (onGameStateChanged) {
        onGameStateChanged();
    }
}
