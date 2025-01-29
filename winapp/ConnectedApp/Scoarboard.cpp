#include "ScoreBoard.h"
#include "Colors.h"
#include <algorithm>

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

void ScoreBoard::renderHighScoresButton(ImVec2 displaySize) {
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - 150, 20));
    ImGui::Begin("##HighScores", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[4]);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent button hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0)); // Transparent button active

    if (ImGui::Button("k")) { // k represents the score icon in our customized font
        showScores = true;
    }
    ImGui::PopStyleColor(3); // Restore previous style
    ImGui::PopFont();
    ImGui::End();
}

void ScoreBoard::renderHighScoresPopup() {
    if (showScores) {
        ImGui::OpenPopup("High Scores");
        showScores = false;
    }

    if (ImGui::BeginPopupModal("High Scores", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // Dark background
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Dark border
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Use the medium font

        auto scores = getScores();
        ImGui::Separator();

        for (const auto& score : scores) {
            ImGui::Text("%s", score.name.c_str());
        }

        ImGui::PopFont();
        ImGui::PopStyleColor(3); // Restore previous styles

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
