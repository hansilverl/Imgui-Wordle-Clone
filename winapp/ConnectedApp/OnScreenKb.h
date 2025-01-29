// OnScreenKb.h
#ifndef ONSCREENKB_H
#define ONSCREENKB_H

#include <string>
#include <unordered_map>
#include "GameLogic.h"
#include "../../shared/ImGuiSrc/imgui.h"

class OnScreenKeyboard {
public:
    void Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize);

private:
    void RenderRow(const char* keys, char* inputBuffer, bool& invalidWord, GameLogic& game_logic);
    void RenderEnterButton(char* inputBuffer, bool& invalidWord, GameLogic& game_logic);
    ImVec4 GetKeyColor(char key, const GameLogic& game_logic) const;
    
    // Default color for unused keys
    const ImVec4 DEFAULT_COLOR = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    // Color for letters not in word
    const ImVec4 WRONG_COLOR = ImVec4(58.0f / 255.0f, 58.0f / 255.0f, 60.0f / 255.0f, 1.0f);
    // Color for letters in wrong position
    const ImVec4 YELLOW_COLOR = ImVec4(181.0f / 255.0f, 159.0f / 255.0f, 59.0f / 255.0f, 1.0f);
    // Color for letters in correct position
    const ImVec4 GREEN_COLOR = ImVec4(83.0f / 255.0f, 141.0f / 255.0f, 78.0f / 255.0f, 1.0f);
};

#endif // ONSCREENKB_H