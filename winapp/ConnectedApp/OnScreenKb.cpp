// OnScreenKb.cpp
#include "OnScreenKb.h"

void RenderOnScreenKeyboard(float offsetX, float offsetY, float gridWidth, float gridHeight, float cellSize, float spacing,
    std::string& currentGuess, size_t& currentCol, size_t& currentRow, GameLogic& game_logic) {
    static const std::vector<std::string> keyboardLayout = {
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };

    ImGui::SetCursorPos(ImVec2(offsetX, offsetY + gridHeight + 70.0f));
    for (size_t i = 0; i < keyboardLayout.size(); i++) {
        float rowWidth = keyboardLayout[i].length() * (cellSize + spacing) - spacing;
        float rowOffsetX = (gridWidth - rowWidth) / 2.0f;
        ImGui::SetCursorPosX(offsetX + rowOffsetX);

        for (char key : keyboardLayout[i]) {
            std::string label(1, key);
            if (ImGui::Button(label.c_str(), ImVec2(cellSize, cellSize))) {
                if (currentCol < 5 && currentRow < 6) {
                    currentGuess[currentCol] = key;
                    currentCol++;
                }
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    // Enter and Backspace buttons
    float controlRowWidth = 2 * (2 * cellSize + spacing) - spacing;
    float controlOffsetX = (gridWidth - controlRowWidth) / 2.0f;
    ImGui::SetCursorPosX(offsetX + controlOffsetX);

    if (ImGui::Button("Enter", ImVec2(2 * cellSize, cellSize))) {
        if (currentCol == 5) {
            if (game_logic.submitGuess(currentGuess)) {
                currentGuess = std::string(5, ' ');
                currentCol = 0;
                currentRow++;
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Backspace", ImVec2(2 * cellSize, cellSize))) {
        if (currentCol > 0) {
            currentCol--;
            currentGuess[currentCol] = ' ';
        }
    }
}
