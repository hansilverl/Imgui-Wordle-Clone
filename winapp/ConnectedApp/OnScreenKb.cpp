#include "OnScreenKb.h"
#include "GameLogic.h"

void OnScreenKeyboard::Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize) {
    const char* keys1 = "QWERTYUIOP";
    const char* keys2 = "ASDFGHJKL";
    const char* keys3 = "ZXCVBNM";

    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 600) * 0.5f, boardSize.y + 20));
    ImGui::Begin("##OnScreenKeyboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

    RenderRow(keys1, inputBuffer, invalidWord, game_logic);
    ImGui::Dummy(ImVec2(25, 0)); // Add some padding to center the row
    ImGui::SameLine();
    RenderRow(keys2, inputBuffer, invalidWord, game_logic);

    RenderEnterButton(inputBuffer, invalidWord, game_logic);
    ImGui::SameLine();
    RenderRow(keys3, inputBuffer, invalidWord, game_logic);
    ImGui::SameLine();
	if (ImGui::Button("BACKSPACE", ImVec2(100, 50))) {
        if (strlen(inputBuffer) > 0) {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }
    }

    ImGui::End();
}

void OnScreenKeyboard::RenderRow(const char* keys, char* inputBuffer, bool& invalidWord, GameLogic& game_logic) {
    for (int i = 0; i < strlen(keys); ++i) {
        if (ImGui::Button(std::string(1, keys[i]).c_str(), ImVec2(50, 50))) {
            if (strlen(inputBuffer) < 5) {
                inputBuffer[strlen(inputBuffer)] = keys[i];
            }
        }
        if (i < strlen(keys) - 1) ImGui::SameLine();
    }
}

void OnScreenKeyboard::RenderEnterButton(char* inputBuffer, bool& invalidWord, GameLogic& game_logic) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Use a smaller font for the "Enter" button
    if (ImGui::Button("ENTER", ImVec2(100, 50))) {
        if (strlen(inputBuffer) == 5) {
            if (!game_logic.submitGuess(inputBuffer)) {
                invalidWord = true;
            }
        }
    }
    ImGui::PopFont();
}
