#include "ScoreBoard.h"

ScoreBoard::ScoreBoard(const std::string& filePath) : filePath(filePath) {}

void ScoreBoard::addScore(const std::string& name, int score) {
    auto scores = loadFromFile();
    scores.push_back({ name, score });
    saveToFile(scores);
}

std::vector<ScoreEntry> ScoreBoard::getScores() const {
    return loadFromFile();
}

void ScoreBoard::saveToFile(const std::vector<ScoreEntry>& scores) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filePath);
    }
    for (const auto& entry : scores) {
        file << entry.name << "," << entry.score << "\n";
    }
}

std::vector<ScoreEntry> ScoreBoard::loadFromFile() const {
    std::vector<ScoreEntry> scores;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return scores; // Return empty if the file doesn't exist
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string name;
        int score;
        if (std::getline(stream, name, ',') && (stream >> score)) {
            scores.push_back({ name, score });
        }
    }
    return scores;
}