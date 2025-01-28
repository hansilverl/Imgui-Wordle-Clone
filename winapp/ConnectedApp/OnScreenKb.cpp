#include "OnScreenKb.h"
#include "GameLogic.h"

void OnScreenKeyboard::Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize) {
    const char* keys1 = "QWERTYUIOP";
    const char* keys2 = "ASDFGHJKL";
    const char* keys3 = "ZXCVBNM";

    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 560) * 0.5f, boardSize.y + 20));
    ImGui::Begin("##OnScreenKeyboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

    // Set button style
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f); // Round buttons
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Grey color
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Slightly lighter grey when hovered
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); // Slightly darker grey when active

    RenderRow(keys1, inputBuffer, invalidWord, game_logic);
    ImGui::Dummy(ImVec2(25, 0)); // Add some padding to center the row
    ImGui::SameLine(0, 6.0f);
    RenderRow(keys2, inputBuffer, invalidWord, game_logic);

    RenderEnterButton(inputBuffer, invalidWord, game_logic);
    ImGui::SameLine(0, 6.0f);
    RenderRow(keys3, inputBuffer, invalidWord, game_logic);
    ImGui::SameLine(0, 6.0f);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
    if (ImGui::Button("⌫", ImVec2(75, 60))) { // Unicode Backspace character
        size_t len = strlen(inputBuffer);
        if (strlen(inputBuffer) > 0) {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }
    }
    ImGui::PopFont();

    // Restore button style
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImGui::End();
}

void OnScreenKeyboard::RenderRow(const char* keys, char* inputBuffer, bool& invalidWord, GameLogic& game_logic) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    for (int i = 0; i < strlen(keys); ++i) {
        if (ImGui::Button(std::string(1, keys[i]).c_str(), ImVec2(45, 60))) {
            if (strlen(inputBuffer) < 5) {
                inputBuffer[strlen(inputBuffer)] = keys[i];
                inputBuffer[strlen(inputBuffer) + 1] = '\0';
            }
        }
        if (i < strlen(keys) - 1) ImGui::SameLine(0, 6.0f); // Adjusted the second parameter to change the spacing
    }
    ImGui::PopFont();
}

void OnScreenKeyboard::RenderEnterButton(char* inputBuffer, bool& invalidWord, GameLogic& game_logic) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // Use a smaller font for the "Enter" button
    if (ImGui::Button("ENTER", ImVec2(70, 60))) {
        if (strlen(inputBuffer) == 5) {
            if (!game_logic.submitGuess(inputBuffer)) {
                invalidWord = true;
            }
        }
    }
    ImGui::PopFont();
}