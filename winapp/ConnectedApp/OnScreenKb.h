// OnScreenKb.h
#ifndef ONSCREENKB_H
#define ONSCREENKB_H

#include <string>
#include <unordered_map>
#include "GameLogic.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include "Colors.h"

class OnScreenKeyboard {
public:
    void Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize);

private:
    void RenderRow(const char* keys, char* inputBuffer, bool& invalidWord, GameLogic& game_logic);
    void RenderEnterButton(char* inputBuffer, bool& invalidWord, GameLogic& game_logic);
    ImVec4 GetKeyColor(char key, const GameLogic& game_logic) const;
   
};

#endif // ONSCREENKB_H