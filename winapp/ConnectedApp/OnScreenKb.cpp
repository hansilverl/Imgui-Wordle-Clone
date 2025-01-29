#include "OnScreenKb.h"
#include "Colors.h"

ImVec4 OnScreenKeyboard::GetKeyColor(char key, const GameLogic& game_logic) const {
    bool found_green = false;
    bool found_yellow = false;
    bool found_wrong = false;

    // Check all guesses
    const auto& history = game_logic.getGuessHistory();
    for (const auto& guess : history) {
        for (const auto& letter : guess.letter_states) {
            if (letter.letter == key) {
                if (letter.correct_position) {
                    found_green = true;
                    break; // Green is the highest priority, we can stop checking
                }
                else if (letter.in_word) {
                    found_yellow = true;
                }
                else {
                    found_wrong = true;
                }
            }
        }
        if (found_green) break; // Exit outer loop if we found green
    }

    // Return the highest priority color
    if (found_green) return GREEN_COLOR;
    if (found_yellow) return YELLOW_COLOR;
    if (found_wrong) return WRONG_COLOR;
    return DEFAULT_COLOR;
}

void OnScreenKeyboard::Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize) {
    const char* keys1 = "QWERTYUIOP";
    const char* keys2 = "ASDFGHJKL";
    const char* keys3 = "ZXCVBNM";

    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 560) * 0.5f, boardSize.y + 20));
    ImGui::Begin("##OnScreenKeyboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

    // Set button style
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    RenderRow(keys1, inputBuffer, invalidWord, game_logic);
    ImGui::Dummy(ImVec2(25, 0)); // Add some padding to center the row
    ImGui::SameLine(0, 6.0f);
    RenderRow(keys2, inputBuffer, invalidWord, game_logic);

    RenderEnterButton(inputBuffer, invalidWord, game_logic);
    ImGui::SameLine(0, 6.0f);
    RenderRow(keys3, inputBuffer, invalidWord, game_logic);
    ImGui::SameLine(0, 6.0f);

    // Handle backspace button - using default color
    ImGui::PushStyleColor(ImGuiCol_Button, DEFAULT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
    if (ImGui::Button("x", ImVec2(75, 60))) { // x maps to backspace in our custom font
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
        ImVec4 keyColor = GetKeyColor(keys[i], game_logic);
        ImVec4 keyColorHovered = ImVec4(keyColor.x * 1.2f, keyColor.y * 1.2f, keyColor.z * 1.2f, keyColor.w);
        ImVec4 keyColorActive = ImVec4(keyColor.x * 0.8f, keyColor.y * 0.8f, keyColor.z * 0.8f, keyColor.w);

        ImGui::PushStyleColor(ImGuiCol_Button, keyColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, keyColorHovered); // Lighter color on hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, keyColorActive); // Darker color on active

        if (ImGui::Button(std::string(1, keys[i]).c_str(), ImVec2(45, 60))) {
            if (strlen(inputBuffer) < 5) {
                inputBuffer[strlen(inputBuffer)] = keys[i];
                inputBuffer[strlen(inputBuffer) + 1] = '\0';
            }
        }
        ImGui::PopStyleColor(3);
        // If not the last key in row, add spacing between buttons
        if (i < strlen(keys) - 1) ImGui::SameLine(0, 6.0f);
    }
    ImGui::PopFont();
}

void OnScreenKeyboard::RenderEnterButton(char* inputBuffer, bool& invalidWord, GameLogic& game_logic) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // Use a smaller font for the "Enter" button
    ImGui::PushStyleColor(ImGuiCol_Button, DEFAULT_COLOR);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

    if (ImGui::Button("ENTER", ImVec2(70, 60))) {
        if (strlen(inputBuffer) == 5) {
            if (!game_logic.submitGuess(inputBuffer)) {
                invalidWord = true;
            }
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::PopFont();
}
