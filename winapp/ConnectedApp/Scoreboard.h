#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "../../shared/ImGuiSrc/imgui.h"

struct ScoreEntry {
    std::string name;
    int score;
};

class ScoreBoard {
public:
    ScoreBoard(const std::string& filePath);
    void addScore(const std::string& name, int score);
    std::vector<ScoreEntry> getScores() const;
    void renderHighScoresButton(ImVec2 displaySize);
    void renderHighScoresPopup();

private:
    std::string filePath;
    bool showScores = false;
    mutable std::vector<ScoreEntry> scores;
    void appendToFile(const ScoreEntry& entry) const;
    void loadFromFile() const;
    void saveToFile() const;
};

#endif // SCOREBOARD_H