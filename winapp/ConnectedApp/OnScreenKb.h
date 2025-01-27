// OnScreenKb.h
#ifndef ONSCREENKB_H
#define ONSCREENKB_H

#include <string>
#include "GameLogic.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_win32.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_dx11.h"

class OnScreenKeyboard {
public:
	void Render(char* inputBuffer, bool& invalidWord, GameLogic& game_logic, ImVec2 boardSize);

private:
    void RenderRow(const char* keys, char* inputBuffer, bool& invalidWord, GameLogic& game_logic);
};

#endif // ONSCREENKB_H
