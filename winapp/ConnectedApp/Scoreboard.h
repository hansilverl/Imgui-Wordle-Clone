// Scoreboard.h
#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

struct ScoreEntry {
    std::string name;
    int score;
};

class ScoreBoard {
public:
    ScoreBoard(const std::string& filePath);
    void addScore(const std::string& name, int score);
    std::vector<ScoreEntry> getScores() const;

private:
    std::string filePath;
    void saveToFile(const std::vector<ScoreEntry>& scores) const;
    std::vector<ScoreEntry> loadFromFile() const;
};

#endif // SCOREBOARD_H
