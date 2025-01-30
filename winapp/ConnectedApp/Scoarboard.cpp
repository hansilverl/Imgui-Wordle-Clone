#include "ScoreBoard.h"
#include "Colors.h"

ScoreBoard::ScoreBoard(const std::string& filePath) : filePath(filePath) {}

void ScoreBoard::addScore(const std::string& name, int score) {
    if (!name.empty()) {
        appendToFile({ name, score });
    }
}

std::vector<ScoreEntry> ScoreBoard::getScores() const {
    auto scores = loadFromFile();
    // Sort scores from highest to lowest
    std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
        });
    // Return only the top 5 scores
    if (scores.size() > 5) {
        scores.resize(5);
    }
    return scores;
}

void ScoreBoard::appendToFile(const ScoreEntry& entry) const {
    std::ofstream file(filePath, std::ios_base::app);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + filePath);
    }
    file << entry.name << "," << entry.score << "\n";
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

    ImGui::SetNextWindowSize(ImVec2(400, 300)); // Set a larger size for the window
    if (ImGui::BeginPopupModal("High Scores", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); // Dark background
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Dark border
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Use the medium font

        auto scores = getScores();
        ImGui::Text("High Scores");
        ImGui::NewLine();
        ImGui::Separator();

        // Render the scores in a table-like format
        ImGui::Columns(2, "scores", true);
        ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text("Score"); ImGui::NextColumn();
        ImGui::Separator();

        for (const auto& score : scores) {
            ImGui::Text("%s", score.name.c_str()); ImGui::NextColumn(); ImGui::Text("%d", score.score); ImGui::NextColumn();
        }
        ImGui::Columns(1);

        ImGui::PopFont();
        ImGui::PopStyleColor(3); // Restore previous styles
        // Set 'x' button at top right corner
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 40, 10));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Ensure the correct font is used
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set text color to white for contrast

        if (ImGui::Button("x", ImVec2(30, 30))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(); // Restore previous text color
        ImGui::PopFont(); // Restore previous font
        ImGui::EndPopup();
    }
}